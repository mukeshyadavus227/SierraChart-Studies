// ============================================================================
// OrderFlowOCOBot.cpp  —  Phase-1 "fail-fast" order-flow strategy
// ----------------------------------------------------------------------------
// PURPOSE
//   A deliberately MINIMAL, honest automated strategy whose only job is to let
//   you answer ONE question cheaply: does a single order-flow trigger, gated by
//   a single trend filter, with a real bracket, have positive expectancy
//   OUT-OF-SAMPLE?  It is NOT a finished product — it is the smallest thing that
//   can be validated. If it fails out-of-sample, you stop here having spent the
//   least (see README.md, the GO/NO-GO rule).
//
// DESIGN PRINCIPLES (the opposite of the TraderOracle/GoldBug anti-patterns)
//   1. ONE trigger, ONE gate. No 8-indicator "confluence" of correlated price
//      MAs. The trigger is order-flow (bid/ask traded volume = delta), which is
//      genuinely independent of the price-derived trend gate.
//   2. REAL risk on every trade: a true OCO bracket (target + hard stop) via
//      attached orders, defined-R, one position at a time.
//   3. NO repaint / NO look-ahead: decisions are taken only on CLOSED bars;
//      no future indices are read; position state is driven by ACTUAL fills
//      (PositionData.PositionQuantity), never by order-submission return codes.
//   4. TRAIN/TEST window inputs so you can run a training window and a separate,
//      untouched test window — the manual walk-forward split the Strategy
//      Optimizer cannot do on its own.
//   5. Every magic number is an INPUT so the Strategy Optimizer can grid it, and
//      so nothing is silently curve-fit in the source.
//
// PERSISTENT SLOT USAGE (per project convention — document to avoid conflicts)
//   Int   0 : Welford sample count (delta stats, per session)
//   Int   1 : current trading-day date (YYYYMMDD); -1 = none yet
//   Int   2 : trades placed today
//   Int   3 : trading halted for today (daily-loss breaker tripped)  0/1
//   Int   4 : data-integrity warning emitted this session           0/1
//   Int   5 : highest bar index already folded into stats (fold-once watermark)
//   Float 0 : Welford running mean of bar delta
//   Float 1 : Welford running M2 (sum of squared deltas from mean)
//
// REQUIREMENTS
//   * Chart must carry Bid/Ask traded volume (SC_BIDVOL / SC_ASKVOL) — true for
//     futures with a proper data feed. If absent, the bot logs a warning and
//     never trades (data-integrity halt).
//   * Run on a Trade-Simulation account first. Replay with "Accurate Trading
//     System Back Test" mode for the optimizer.
// ============================================================================

#include "sierrachart.h"

SCDLLName("OrderFlow OCO Bot (Phase 1)")

// --- small online-variance helper (Welford); n incremented by caller --------
static void WelfordAdd(int n, double& mean, double& M2, double x)
{
    const double delta = x - mean;
    mean += delta / (double)n;
    M2   += delta * (x - mean);
}
static double WelfordSigma(double M2, int n)
{
    return (n > 1) ? sqrt(M2 / (double)(n - 1)) : 0.0;
}

/*==========================================================================*/
SCSFExport scsf_OrderFlowOCOBot(SCStudyInterfaceRef sc)
{
    // ---- subgraphs (diagnostics; the bot trades, these just let you see it) -
    SCSubgraphRef sgEMA     = sc.Subgraph[0];   // trend-gate EMA
    SCSubgraphRef sgDeltaZ  = sc.Subgraph[1];   // delta z-score (trigger input)
    SCSubgraphRef sgLong    = sc.Subgraph[2];   // long-entry marker
    SCSubgraphRef sgShort   = sc.Subgraph[3];   // short-entry marker

    // ---- inputs -------------------------------------------------------------
    SCInputRef in_Enabled      = sc.Input[0];
    SCInputRef in_SendLive     = sc.Input[1];   // send to trade service vs sim
    SCInputRef in_TrigMode     = sc.Input[2];   // 0 Thrust (momentum), 1 Absorption (fade)
    SCInputRef in_DeltaK       = sc.Input[3];   // trigger sigma threshold k
    SCInputRef in_WarmupBars   = sc.Input[4];   // min delta samples before trading
    SCInputRef in_EMALen       = sc.Input[5];   // trend-gate EMA length
    SCInputRef in_SlopeLook    = sc.Input[6];   // bars back for EMA slope
    SCInputRef in_SlopeDeadTk   = sc.Input[7];   // slope deadband (ticks)
    SCInputRef in_UseGate      = sc.Input[8];   // apply trend gate? (Yes/No)
    SCInputRef in_Qty          = sc.Input[9];   // contracts per trade
    SCInputRef in_StopTicks    = sc.Input[10];  // hard stop distance (ticks)
    SCInputRef in_TargetR      = sc.Input[11];  // target = R * stop distance
    SCInputRef in_MaxTradesDay  = sc.Input[12];  // daily trade cap
    SCInputRef in_MaxDailyLoss  = sc.Input[13];  // daily loss breaker ($, 0 = off)
    SCInputRef in_SessStartHr   = sc.Input[14];  // session start hour (chart TZ)
    SCInputRef in_SessEndHr     = sc.Input[15];  // session end hour (chart TZ)
    SCInputRef in_StartDate    = sc.Input[16];  // trade window start YYYYMMDD (0 = no bound)
    SCInputRef in_EndDate      = sc.Input[17];  // trade window end   YYYYMMDD (0 = no bound)
    SCInputRef in_FlattenEOD     = sc.Input[18]; // flatten at session end?

    if (sc.SetDefaults)
    {
        sc.GraphName   = "OrderFlow OCO Bot (Phase 1)";
        sc.GraphRegion = 0;
        sc.AutoLoop    = 1;
        sc.ScaleRangeType = SCALE_SAMEASREGION;

        sgEMA.Name = "Trend Gate EMA";
        sgEMA.DrawStyle = DRAWSTYLE_LINE; sgEMA.PrimaryColor = RGB(120,160,220);
        sgEMA.DrawZeros = false;

        sgDeltaZ.Name = "Delta Z (internal)";
        sgDeltaZ.DrawStyle = DRAWSTYLE_IGNORE; sgDeltaZ.DrawZeros = true;

        sgLong.Name = "Long Entry"; sgLong.DrawStyle = DRAWSTYLE_ARROW_UP;
        sgLong.PrimaryColor = RGB(0,200,0); sgLong.LineWidth = 2; sgLong.DrawZeros = false;
        sgShort.Name = "Short Entry"; sgShort.DrawStyle = DRAWSTYLE_ARROW_DOWN;
        sgShort.PrimaryColor = RGB(220,40,40); sgShort.LineWidth = 2; sgShort.DrawZeros = false;

        in_Enabled.Name = "Enabled"; in_Enabled.SetYesNo(false);  // OFF by default — safety
        in_SendLive.Name = "Send Orders To Trade Service"; in_SendLive.SetYesNo(false);

        in_TrigMode.Name = "Trigger Mode";
        in_TrigMode.SetCustomInputStrings("Delta Thrust (momentum);Absorption (fade)");
        in_TrigMode.SetCustomInputIndex(0);

        in_DeltaK.Name = "Trigger Sigma (k)"; in_DeltaK.SetFloat(2.0f); in_DeltaK.SetFloatLimits(0.1f, 10.0f);
        in_WarmupBars.Name = "Warmup Bars (delta stats)"; in_WarmupBars.SetInt(50); in_WarmupBars.SetIntLimits(5, 5000);

        in_EMALen.Name = "Trend EMA Length"; in_EMALen.SetInt(50); in_EMALen.SetIntLimits(2, 1000);
        in_SlopeLook.Name = "Trend Slope Lookback (bars)"; in_SlopeLook.SetInt(10); in_SlopeLook.SetIntLimits(1, 500);
        in_SlopeDeadTk.Name = "Trend Slope Deadband (ticks)"; in_SlopeDeadTk.SetFloat(2.0f); in_SlopeDeadTk.SetFloatLimits(0.0f, 1000.0f);
        in_UseGate.Name = "Apply Trend Gate"; in_UseGate.SetYesNo(true);

        in_Qty.Name = "Contracts Per Trade"; in_Qty.SetInt(1); in_Qty.SetIntLimits(1, 1000);
        in_StopTicks.Name = "Hard Stop (ticks)"; in_StopTicks.SetInt(40); in_StopTicks.SetIntLimits(1, 100000);
        in_TargetR.Name = "Target (R multiple of stop)"; in_TargetR.SetFloat(1.5f); in_TargetR.SetFloatLimits(0.1f, 100.0f);

        in_MaxTradesDay.Name = "Max Trades Per Day"; in_MaxTradesDay.SetInt(10); in_MaxTradesDay.SetIntLimits(1, 10000);
        in_MaxDailyLoss.Name = "Max Daily Loss ($, 0=off)"; in_MaxDailyLoss.SetFloat(0.0f); in_MaxDailyLoss.SetFloatLimits(0.0f, 1e9f);

        in_SessStartHr.Name = "Session Start Hour (chart TZ, -1=off)"; in_SessStartHr.SetInt(-1); in_SessStartHr.SetIntLimits(-1, 23);
        in_SessEndHr.Name = "Session End Hour (chart TZ, -1=off)"; in_SessEndHr.SetInt(-1); in_SessEndHr.SetIntLimits(-1, 24);

        in_StartDate.Name = "Trade Window Start (YYYYMMDD, 0=off)"; in_StartDate.SetInt(0); in_StartDate.SetIntLimits(0, 99999999);
        in_EndDate.Name = "Trade Window End (YYYYMMDD, 0=off)"; in_EndDate.SetInt(0); in_EndDate.SetIntLimits(0, 99999999);
        in_FlattenEOD.Name = "Flatten At Session End"; in_FlattenEOD.SetYesNo(true);

        // --- trade settings: one position, real attached OCO, no pyramiding ---
        sc.AllowMultipleEntriesInSameDirection = false;
        sc.MaximumPositionAllowed = 1;
        sc.SupportReversals = false;
        sc.AllowOppositeEntryWithOpposingPositionOrOrders = false;
        sc.SupportAttachedOrdersForTrading = true;   // attached target/stop -> OCO
        sc.UseGUIAttachedOrderSetting = false;
        sc.CancelAllOrdersOnEntriesAndReversals = false;
        sc.AllowEntryWithWorkingOrders = false;
        sc.AllowOnlyOneTradePerBar = true;
        sc.CancelAllWorkingOrdersOnExit = true;
        sc.MaintainTradeStatisticsAndTradesData = true;

        return;
    }

    // route orders to sim unless explicitly told to go live
    sc.SendOrdersToTradeService = in_SendLive.GetYesNo();

    if (sc.ChartIsDownloadingHistoricalData(sc.ChartNumber)) return;
    if (sc.LastCallToFunction) return;

    const int idx = sc.Index;

    // ---- persistent state ---------------------------------------------------
    int&    wN            = sc.GetPersistentInt(0);
    int&    sessionDate   = sc.GetPersistentInt(1);
    int&    tradesToday   = sc.GetPersistentInt(2);
    int&    haltedToday   = sc.GetPersistentInt(3);
    int&    dataWarned    = sc.GetPersistentInt(4);
    int&    lastFoldedBar = sc.GetPersistentInt(5);
    double& dMean         = sc.GetPersistentFloat(0);
    double& dM2           = sc.GetPersistentFloat(1);

    // Start of a fresh full recalculation: re-fold all history into the stats.
    if (sc.UpdateStartIndex == 0 && idx == 0)
    {
        lastFoldedBar = -1;
        wN = 0; dMean = 0.0; dM2 = 0.0; sessionDate = -1;
    }
    if (idx < 2) return;

    // ---- trend-gate EMA (one filter, computed every bar) --------------------
    const int   emaLen    = in_EMALen.GetInt();
    sc.MovingAverage(sc.BaseData[SC_LAST], sgEMA, MOVAVGTYPE_EXPONENTIAL, emaLen);

    // ---- bar timing / session / window gates --------------------------------
    const SCDateTime bt = sc.BaseDateTimeIn[idx];
    const int barDate  = bt.GetDate();                 // YYYYMMDD
    const int barHour  = bt.GetHour();
    const int dayDate  = (int)sc.GetTradingDayDate(bt);

    // new trading day -> reset session stats + daily counters
    if (dayDate != sessionDate)
    {
        sessionDate = dayDate;
        wN = 0; dMean = 0.0; dM2 = 0.0;
        tradesToday = 0; haltedToday = 0; dataWarned = 0;
    }

    const int startDate = in_StartDate.GetInt();
    const int endDate   = in_EndDate.GetInt();
    const bool inDateWindow = (startDate == 0 || barDate >= startDate) &&
                              (endDate   == 0 || barDate <= endDate);

    const int sH = in_SessStartHr.GetInt(), eH = in_SessEndHr.GetInt();
    const bool inSession = (sH < 0 || eH < 0) ? true : (barHour >= sH && barHour < eH);

    // ---- order-flow trigger: per-bar delta -> session z-score ---------------
    const double askV = sc.BaseData[SC_ASKVOL][idx];
    const double bidV = sc.BaseData[SC_BIDVOL][idx];
    const double deltaBar = askV - bidV;

    // data-integrity guard: no bid/ask volume on this feed -> never trade
    if (askV == 0.0 && bidV == 0.0 && sc.Volume[idx] > 0.0)
    {
        if (!dataWarned && !sc.IsFullRecalculation)
        {
            sc.AddMessageToLog("OrderFlowOCOBot: chart has NO bid/ask volume "
                "(SC_ASKVOL/SC_BIDVOL == 0). Trigger is inert; bot will not trade. "
                "Use a feed/chart with Bid/Ask volume.", 1);
            dataWarned = 1;
        }
    }

    // act only on a CLOSED bar (no intrabar repaint), and not during full recalc
    const bool barClosed = (sc.GetBarHasClosedStatus(idx) == BHCS_BAR_HAS_CLOSED);
    const bool liveBar    = (!sc.IsFullRecalculation) && (sc.UpdateStartIndex > 0);

    // z-score the current bar against stats SO FAR (before folding it in, so a
    // bar never sets its own threshold), then fold the closed bar exactly once.
    const double sigma = (wN > 1) ? WelfordSigma(dM2, wN) : 0.0;
    const double z = (sigma > 1e-9) ? (deltaBar - dMean) / sigma : 0.0;
    sgDeltaZ[idx] = z;

    if (barClosed && idx > lastFoldedBar)   // guard against AutoLoop re-processing
    {
        wN += 1;
        WelfordAdd(wN, dMean, dM2, deltaBar);
        lastFoldedBar = idx;
    }

    // ---- evaluate gate + trigger -------------------------------------------
    const double tick = (sc.TickSize > 0.0) ? sc.TickSize : 0.25;
    const int slopeLook = in_SlopeLook.GetInt();
    const double slope = sgEMA[idx] - sgEMA[max(0, idx - slopeLook)];
    const double deadband = in_SlopeDeadTk.GetFloat() * tick;
    int gate = 0;                                   // +1 up, -1 down, 0 neutral
    if (slope >  deadband) gate = +1;
    else if (slope < -deadband) gate = -1;

    const double k = in_DeltaK.GetFloat();
    const bool warm = (wN >= in_WarmupBars.GetInt());
    const bool useGate = in_UseGate.GetYesNo();
    const int  mode = in_TrigMode.GetIndex();       // 0 thrust, 1 absorption

    const double barClose = sc.Close[idx];
    const double barOpen  = sc.Open[idx];

    bool wantLong = false, wantShort = false;
    if (warm && barClosed)
    {
        if (mode == 0)   // DELTA THRUST: trade WITH a strong delta push
        {
            wantLong  = (z >=  k) && (!useGate || gate > 0);
            wantShort = (z <= -k) && (!useGate || gate < 0);
        }
        else             // ABSORPTION: strong opposing delta that price refused
        {                // heavy selling (z<=-k) but bar closed up = buyers absorbed
            wantLong  = (z <= -k) && (barClose >  barOpen) && (!useGate || gate >= 0);
            wantShort = (z >=  k) && (barClose <  barOpen) && (!useGate || gate <= 0);
        }
    }

    // ---- position / risk state ---------------------------------------------
    s_SCPositionData pos;
    sc.GetTradePosition(pos);
    const bool flat = (pos.PositionQuantity == 0);

    // daily-loss circuit breaker (uses SC's realized+open daily P/L)
    const double maxDailyLoss = in_MaxDailyLoss.GetFloat();
    if (maxDailyLoss > 0.0 && !haltedToday && liveBar)
    {
        const double dayPnL = pos.DailyProfitLoss + pos.OpenProfitLoss;
        if (dayPnL <= -maxDailyLoss)
        {
            haltedToday = 1;
            sc.FlattenAndCancelAllOrders();
            sc.AddMessageToLog("OrderFlowOCOBot: daily loss breaker tripped — flat for the day.", 1);
        }
    }

    // flatten at session end if requested
    if (in_FlattenEOD.GetYesNo() && !inSession && !flat && liveBar)
        sc.FlattenAndCancelAllOrders();

    // ---- gating for new entries --------------------------------------------
    const bool tradingAllowed =
        in_Enabled.GetYesNo() && liveBar && barClosed &&
        inDateWindow && inSession && !haltedToday &&
        flat &&
        (pos.WorkingOrdersExist == 0) &&
        (tradesToday < in_MaxTradesDay.GetInt());

    sgLong[idx] = 0.0; sgShort[idx] = 0.0;
    if (!tradingAllowed || (!wantLong && !wantShort)) return;
    if (wantLong && wantShort) return;              // ambiguous — skip

    // ---- build the OCO bracket ---------------------------------------------
    const int    qty       = in_Qty.GetInt();
    const double stopDist  = in_StopTicks.GetInt() * tick;
    const double targetDist = stopDist * in_TargetR.GetFloat();
    if (stopDist <= 0.0) return;

    // Setting Target1Offset + Stop1Offset on a BuyEntry/SellEntry attaches a
    // target (limit) and a hard stop in OCO group 1 -> a real bracket. Offsets
    // are actual price distances from the fill (ticks * sc.TickSize).
    s_SCNewOrder order;
    order.OrderQuantity = qty;
    order.OrderType = SCT_ORDERTYPE_MARKET;         // market-on-signal: no fill fiction
    order.TimeInForce = SCT_TIF_DAY;
    order.Target1Offset = targetDist;               // OCO target as offset from fill
    order.Stop1Offset   = stopDist;                 // OCO stop  as offset from fill

    int result = 0;
    if (wantLong)  result = (int)sc.BuyEntry(order);
    else           result = (int)sc.SellEntry(order);

    if (result > 0)                                 // order accepted by SC
    {
        // NOTE: acceptance != fill. tradesToday counts submissions; position
        // state is read from PositionData on subsequent bars, never assumed here.
        tradesToday += 1;
        if (wantLong)  sgLong[idx]  = sc.Low[idx]  - 2 * tick;
        else           sgShort[idx] = sc.High[idx] + 2 * tick;

        SCString m;
        m.Format("%s entry @bar %d  z=%.2f gate=%d  qty=%d stop=%.2f target=%.2f (mode %d)",
                 wantLong ? "LONG" : "SHORT", idx, z, gate, qty, stopDist, targetDist, mode);
        sc.AddMessageToLog(m, 0);
    }
}
