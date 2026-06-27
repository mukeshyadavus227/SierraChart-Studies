# Sierra Chart Studies — Comprehensive Screening & Profit-Potential Shortlist

> **Scope:** 6 repositories. The 4 with strategy source — `SierraChart-Studies`, `SierraChartStudies`, `sierrachart-absorption-study`, `SierraChart-TraderOracle` — were screened study-by-study below. `SierraChartStrategyOptimizer` (a backtest harness) and `sierrachart-acsil-docs` (Sierra Chart reference docs + vendor example code) are infrastructure/reference and are assessed as context, not scored as studies.
>
> **Coverage:** All **44** candidate study source files (~24,500 LOC) were read byte-by-byte and adversarially audited. `DanBracketMaster.cpp` and `TraderOracle.cpp` (the 2,774-line flagship) were recovered in a second pass and folded in below.

## Executive summary

**44 studies** across four repositories were profiled and adversarially audited (thesis, tradeability, repaint/look-ahead, overfit, completeness), then cross-analyzed in five thematic clusters and against the backtest-optimizer infrastructure.

The big picture is stark and consistent:

- **`SierraChart-Studies`** (20 studies) is a **discretionary order-flow / context DASHBOARD toolkit**, not a trading system. Every member is a visualization, a paint-only signal layer, or a regime filter. **Zero** of them call `sc.BuyEntry/SellEntry/SubmitOrder` (grep-verified: 0 of 20 files place orders). None can produce P&L on their own.
- **`SierraChartStudies`** (the "Danijel" repo) is the only place with **genuinely live auto-traders** that place real bracketed orders. Grep-verified, exactly **6 files** place orders: `DanZoneDeltaReversalTrader`, `DanBracketBot`, `DanScalpMaster`, `RangeDetector`, `SC_Range_Bounce_Auto`, `EUSupportBounce`. The rest (`DanZoneMaster`, `DanBracketMaster`, `DanTheDumbTrader`, `DanStopLock`, `DanDeltaWizWorkerSlave`) are painters/stop-managers despite README billing. The auto-traders are mechanically real but riddled with edge-killing bugs and overfit.
- **`SierraChart-TraderOracle`** is the flagship-painter `TraderOracle.cpp`/"Olympus" (a no-order multi-indicator confluence signal with confirmed repaint) + mostly **broken line-drawing utilities**, plus `Renko_GOAT` (paints) and `TORobots` (the only order-placer here). **Half the files do not even compile** (`TORobots`, `MarketMaker`, `VolImbRenko`).
- **`sierrachart-absorption-study`** contributes one repainting single-histogram indicator.

**Headline finding: there is no proven, deployable money-maker in any repo.** Not a single study has `edgeSurvives = true`. The closest things to "tradeable mechanics" are the live auto-traders, but each fails adversarial scrutiny on look-ahead, overfit, or live-capital bugs.

**Shortlist (best-engineered, worth fixing — NOT yet profitable):**
1. `SC_Range_Bounce_Auto.cpp` — only auto-trader with closed-bar gating **and** a hard 1.5R-capped OCO stop.
2. `DanZoneDeltaReversalTrader.cpp` — most complete live auto-trader with a coherent thesis (delta-spike reversal at zones).
3. `DOMReaderV2.cpp` — best-architected *signal* engine (closed-bar Welford sigma triggers), but places no orders and is unbacktestable.

Everything else is context, supporting indicator, or reject.

---

## Scoring methodology

Each study is rated **1–5 (adjusted)** against five criteria. Profit-making potential requires *all five*, not just "looks good on a chart":

1. **Thesis** — economically plausible, mechanized in code (not just prose in a header).
2. **Tradeability** — actionable **entries AND exits/risk** (stops, targets, sizing). A pure indicator with no order/exit logic cannot score above 2 regardless of how clean it is.
3. **No repaint / no look-ahead** — closed-bar decisions; no forming-bar reads used as final; no future indices; no HTF backfill; no full-recalc-vs-live divergence.
4. **No overfit** — no hardcoded prices/dates, no instrument-baked tick/contract constants, thresholds derived (sigma/ATR/percentile) rather than eyeballed to one symbol/period.
5. **Completeness** — compiles, no dead-code core, no stubs masquerading as features.

**Rating bands:** 5 = proven tradeable edge; 4 = tradeable, plausible edge, validation pending; 3 = tradeable but flawed; 2 = clean component / context, no standalone edge; 1 = broken, inert, overfit, or pure annotation.

---

## Master comparison table

Sorted by adjusted rating (desc), then by tradeability.

| Study | Repo | Type | Tradeable? | Repaint risk | Overfit risk | Completeness | Adj. | One-line verdict |
|---|---|---|---|---|---|---|---|---|
| DOMReaderV2.cpp | SC-Studies | SignalEngine | No (markers) | Low | Low | Usable | 2 | Best-architected closed-bar Welford trigger engine, but no orders, 1-bar lag, unbacktestable DOM feed. |
| TapeReader.cpp | SC-Studies | Indicator | No | None | None | Production | 1 | Cleanest/honest tape viz (no repaint, no overfit) but literally zero signal/edge. |
| FlowConviction.cpp | SC-Studies | SignalEngine | No (paint) | Low | Low | Usable | 2 | Thoughtful EWMA flow oscillator; live-bar color repaints intrabar; discretionary overlay only. |
| OrderflowSignalV3.cpp | SC-Studies | SignalEngine | No (arrows) | High | High | Usable | 2 | Direction-blind confluence aggregator; self-admitted repaint; hardcoded 33-study ensemble. |
| ReconTape.cpp | SC-Studies | Indicator | No | High(default) | None | Production | 2 | Production VAP bubble viz, trailing score (no look-ahead); Live default repaints forming bar. |
| BigTradesTape.cpp | SC-Studies | Indicator | No | Low | Low | Production | 2 | Well-built T&S sweep viz; weak "big print = follow-through" thesis; alert only. |
| InterestMap.cpp | SC-Studies | Indicator | No | High | Low | Production | 2 | Multi-evidence level book; bands repaint on recalc; whale data lost on reload. |
| TrappedTraders.cpp | SC-Studies | Indicator | No | Medium | Medium | Usable | 2 | Footprint S/R zone painter; retroactive merge/erase repaint; ES-tuned delta thresholds. |
| LiquidityZones.cpp | SC-Studies | Indicator | No | Medium | Low | Usable | 2 | Reads `Open[bi+1]` (look-ahead) + destructive intrabar zone invalidation; confirmed repaint. |
| DeltaVelocityProfile.cpp | SC-Studies | Indicator | No | Low | Low | Production | 2 | Plausible delta-momentum viz; fade cue corrupted by unguarded persistent-scalar-in-AutoLoop bug. |
| AbsorptionGradient.cpp | SC-Studies | Indicator | No | Low | Low | Usable | 2 | Effort-vs-result oscillator; trapped marker repaints; double min-max self-norm = noise. |
| BreadthCompositeOscillator.cpp | SC-Studies | Indicator (filter) | No | Medium | Medium | Production | 2 | Breadth color; intrabar repaint of $VOLD/$ADD; US-RTH-locked; inverted dynamic weighting. |
| AVWAPRotation.cpp | SC-Studies | Indicator | No | High | Low | Production | 2 | Pure AVWAP/midline plotter; legs anchor retroactively to prior swing extremes. |
| MTFCloseFilter.cpp | SC-Studies | Filter | No | High | Low | Usable | 2 | Clean MTF close-Donchian dashboard; reads forming bar; no historical fill = un-backtestable. |
| DOMReader.cpp | SC-Studies | Indicator | No | None | None | Production | 2 | Honest non-repaint pull/stack histogram; spoofable input; superseded by V2. |
| OrderflowConfluence.cpp | SC-Studies | SignalEngine | No | Medium | None | Usable | 2 | Empty directionless scoreboard; inert out of box; intrabar repaint. |
| OrderflowSignalV2.cpp | SC-Studies | SignalEngine | No | Medium | High | Usable | 2 | Direction-blind aggregator; broken stale-index alerts; superseded by V3. |
| TraderOracle.cpp (Olympus) | TraderOracle | SignalEngine | No (arrows) | High | Medium | Partial | 2 | Flagship multi-indicator AND-stack painter; places NO orders, broken Fisher/Supertrend internals, repaints on reload, dead/debug code. |
| DanBracketMaster.cpp | SCStudies | Indicator | No | Low | None | Usable | 2 | Bracket/zone painter + panic-flatten kill-switch; no entries/exits; README bills it "auto-detect" but it places no orders. |
| ReconTapeV2.cpp | SC-Studies | Indicator | No | Medium | Low | Production | 1 | Multi-tier VAP bubbles; coarse percentile + color-bar bug; Live default repaints. |
| DeltaReversalTrigger.cpp | SC-Studies | Indicator | No | Low | None | Partial | 1 | Misnamed "trigger" — computes bands+accel but never combines them; no signal at all. |
| OTFStateFilter.cpp | SC-Studies | Filter | No | High | None | Partial | 1 | Cross-chart mode paints all history with latest regime = textbook look-ahead. |
| DanZoneDeltaReversalTrader.cpp | SCStudies | AutoTrader | **Yes** | Low | Medium | Usable | 2 | Live OCO trader, coherent zone+delta thesis; edge outsourced + repaint-contaminated; capital bugs. |
| DanBracketBot.cpp | SCStudies | AutoTrader | **Yes** | None | Medium | Partial | 2 | Live trend-flip trader; 30:1 unreachable target/tight stop; dead Enabled toggle; orphaned-OCO. |
| RangeDetector.cpp | SCStudies | AutoTrader | **Yes** | High | High | Usable | 2 | Range-fade trader; forming-bar market entries, NO hard stop; curve-fit ATR gate. |
| SC_Range_Bounce_Auto.cpp | SCStudies | AutoTrader | **Yes** | Low | High | Partial | 2 | Best risk architecture (closed-bar + 1.5R stop); limit-at-passed-price backtest fiction. |
| EUSupportBounce.cpp | SCStudies | AutoTrader | **Yes** | Medium | High | Usable | 2 | Round-number bounce; `abs()`-on-float corrupts core filter; hour-early flatten; long-only. |
| DISCStudies.cpp (ATR2Risk) | SCStudies | Utility | No | None | None | Usable | 2 | ATR position-sizing helper; div-by-zero on ATR==0; min-1 clamp breaks risk cap. |
| DanZoneMaster.cpp | SCStudies | Utility | No | Low | None | Partial | 1 | Zone-recolor plumbing; zero orders; destructive color repaint on recalc. |
| DanDeltaWizWorkerSlave.cpp | SCStudies | Utility | No | Low | None | Usable | 1 | Remote loosen-only stop-mover; full recalc disarms it mid-trade. |
| DanTheDumbTrader.cpp | SCStudies | Utility | No | Low | None | Partial | 1 | Superseded near-duplicate of ZoneMaster; zero orders. |
| DanStopLock.cpp | SCStudies | Utility | No | None | None | Partial | 1 | Stop-ratchet; every-bar ModifyOrder spam; reverts on flat account; dead Enabled. |
| DanScalpMaster.cpp | SCStudies | AutoTrader | **Yes** | Low | Medium | Stub | 1 | Naked-market entries; "bottom" delta band mis-located near HIGH; real strategy never called. |
| NQ_Breakout_Ranges_Jan2025.cpp | SCStudies | Indicator | No | High | High | Partial | 1 | 292 hardcoded Jan-2025 NQ ranges drawn on recalc; look-ahead + maximal overfit; inert live. |
| Renko_GOAT.cpp | TraderOracle | SignalEngine | No (arrows) | High | Medium | Partial | 1 | 8-indicator confluence painter; 1-bar visual look-ahead; unreachable dead BUY alert. |
| TORobots.cpp (GoldBug) | TraderOracle | AutoTrader | Yes (broken) | High | Low | Stub | 1 | DOES NOT COMPILE (line 94); no exits; pyramids; uninitialized SuperTrend flags. |
| Killpips.cpp | TraderOracle | Indicator | No | None | Medium | Usable | 1 | Pure line-drawing util; stale baked-in levels; no signal. |
| mancini.cpp | TraderOracle | Indicator | No | None | High | Partial | 1 | Draws pasted Mancini ES levels; ES-locked; Left(4) truncation mis-draws decimals. |
| LRS.cpp | TraderOracle | Indicator | No | Low | Low | Usable | 1 | LinReg slope viz; zero-line/slope aliasing + off-by-one color bugs; no exported signal. |
| ManciniPlusConverter.cpp | TraderOracle | Utility | No | Low | High | Partial | 1 | Conversion feature is dead code; draws every line at price 0 by default. |
| CDVolumeAbsorption.cpp | absorption-study | Indicator | No | High | None | Partial | 1 | Full-day min-max norm recomputed every tick = look-ahead within session; no inputs/signal. |
| TraderSmarts_Unofficial.cpp | TraderOracle | Indicator | No | Low | None | Partial | 1 | Draws external vendor file levels across full history; memory leak; no error handling. |
| MarketMaker.cpp | TraderOracle | Indicator | No | None | High | Stub | 1 | DOES NOT COMPILE (undeclared identifiers); half-renamed Killpips clone. |
| VolImbRenko.cpp | TraderOracle | SignalEngine | No | High | Low | Stub | 1 | DOES NOT COMPILE (dup declarations); look-ahead by design (reads 4 future bars). |

---

## SHORTLIST — real / solid / profit-capable

**Critical disclaimer:** No study in this corpus has a *proven* edge (`edgeSurvives = true` is empty across all 44). This shortlist is the set of **genuinely tradeable systems** (real orders) or **best-architected signal engines** that are *worth fixing and validating* — they have the right bones. Each is currently **unproven** or carries edge-killing defects.

### 1. `SC_Range_Bounce_Auto.cpp` — Range-boundary mean reversion (best risk architecture)

- **Edge:** Fade a persisted intraday range at its boundary, target the opposite boundary, gated to a medium-ATR band and EU session. Limit BUY at range low / limit SELL at range high.
- **Why it could be profitable:** It is the *only* auto-trader in the corpus that does two things right at once — (a) gates order submission to **closed bars** (`BHCS_BAR_HAS_CLOSED` + `UpdateStartIndex>0`, lines 282-283), and (b) submits a real **OCO bracket with a hard stop capped at 1.5R** (`StopAllPrice = RangeLow − min(stopBeyond, rangeSize*maxLossR)`, line 429). Defined, bounded risk is the single property that gives any mean-reversion system survivable expectancy.
- **What must still be validated (and how):**
  - **Fix the entry first.** The fatal flaw is a *backtest-fill fiction*: after the bar closes it places a LIMIT order **at the already-passed boundary price** (lines 436-446). A large fraction of fills the Python backtest assumed will never fill live. Change to **market-on-touch** (or a stop-limit straddling the boundary) before any test is meaningful.
  - Fix `InPosition` being set on order **acceptance**, not fill (lines 447-448), and the Close-based stop-vs-target exit **misclassification** (lines 305-328).
  - Then run the **Strategy Optimizer** *only as an in-sample grid search* over a **train** window (it has no `endDate`, so control the test split via how much chart history is loaded). Pick a **parameter plateau**, not the single peak (the optimizer's `ResultAnalyzer` ranks by Total P/L only — selection bias trap). Re-run a **separate later untouched window** with those params fixed (increment=0) and confirm the edge survives. Impose a minimum-trade count and verify commissions/slippage are configured in the Sim account.
- **Main risk:** Admitted single-dataset curve-fit (`"EXACT MATCH TO PYTHON BACKTEST"`, razor-precise ATR gate 0.82–7.23, 10/6/10-pt thresholds baked to NQ scale, EU session). Mean reversion at a band with a stop ~1.5× the range is structurally short the breakout tails. The edge does **not** currently survive.

### 2. `DanZoneDeltaReversalTrader.cpp` — Delta-spike reversal at supply/demand zones (most complete live trader)

- **Edge:** When price re-enters a demand/supply zone (published by the companion `DanZoneMaster`) **and** there is a large opposing delta spike (sellers exhausted at demand → long; buyers exhausted at supply → short), expect a reversal off the zone. The most economically coherent thesis in the corpus.
- **Why it could be profitable:** Fully-wired live trader: real `sc.BuyOrder/SellOrder` + OCO target/stop + **risk-based position sizing** (`calcMaxContractsForRisk`) + session auto-enable + replay-pause. Order path is **closed-bar gated** (`BHCS_BAR_HAS_CLOSED`), so minimal real-time look-ahead in live forward trading.
- **What must still be validated (and how):**
  - The actual alpha (where zones get drawn) lives in `DanZoneMaster`, which **rebuilds zones with hindsight on full recalc** — so any backtest/replay is **repaint-contaminated** and optimistic. You cannot validate this with the in-sample optimizer until zone publication is made causal/persisted per-bar. This is the dominant blocker.
  - Fix the live-capital bugs before risking money: wrong-sign MonsterMove short target (`barClose + 300`, lines 1126/1224 — puts the short TP *above* entry), always-true trailing-stop guard (line 589 uses the constant `ORDER_TYPE_BUY` as a condition), hardcoded short slave stop ignoring config (line 1250), and **multi-zone order stacking in one bar** with `MaximumPositionAllowed = 1e8` because `areWeInPosition()` reads only the last async fill.
- **Main risk:** Edge is outsourced + repaint-contaminated; instrument/regime-specific magic delta thresholds (35/80 raw contracts) and EU/DAX session windows. Mechanically real, edge unproven.

### 3. `DanBracketBot.cpp` — Structural trend-flip auto-trader (clean execution, broken economics)

- **Edge:** Track a reference bar + regime; when a candle closes through the reference bar, flip position (trend-flip / structure break).
- **Why it's on the list (narrowly):** It is the only live trader with **no look-ahead and no repaint** — acts strictly on closed bars (`BHCS_BAR_HAS_CLOSED`, AutoLoop=0). The execution plumbing is causal.
- **What must still be validated:** The economics are dominated by a **tight ~10-tick stop vs an effectively-unreachable ~300-tick target (30:1)**, so net behavior reduces to "tight stop + exit-on-flip" — it bleeds in chop. Before testing: expose the 300/10/2-tick + qty-3/6 constants as inputs (they are instrument-baked), fix the **non-functional `Enabled` kill switch** (Input[0] is read but never gates trading), and fix the **orphaned-OCO-on-flip** hazard (manual qty-6 reversal leaves the prior side's OCO live). Then grid-search target/stop on train, validate on test.
- **Main risk:** High whipsaw; no demonstrated edge; unsafe as-is.

> **Honorable mention (signal-research substrate, not tradeable): `DOMReaderV2.cpp`.** The best-*architected* engine in the whole corpus — closed-bar Welford session-stat triggers with **sigma-derived (not baked) thresholds**, cooldown, slope alignment, no intrabar repaint, low overfit. It is excluded from the tradeable shortlist because it **places no orders, lags one bar, and DOM depth is forward-only → structurally unbacktestable.** Worth keeping as a signal-research input, not a system.

---

## Strong supporting / context studies

These have **no standalone edge** but are the cleanest building blocks and could *materially improve* a tradeable system as filters/confirmation. None should be traded alone.

| Study | Role | Why it helps | Caveat |
|---|---|---|---|
| `TapeReader.cpp` | Flow/urgency reference | The **only** study with `repaint=none` AND `overfit=none` — strictly backward-looking flow histogram + self-adaptive (mean+K·std) urgency gate; full-recalc == realtime. | Zero signal; pure sanity/reference layer. |
| `FlowConviction.cpp` | Order-flow confirmation | Most thoughtful composite (EWMA-normalized aggression/velocity/persistence, divergence points, Welford-on-closed-bars). | Live-bar color repaints intrabar; ES-tick-coupled large-print proxy; mis-scaled blend (`n_pers` fed raw). |
| `MTFCloseFilter.cpp` | MTF trend-state gate | Instrument-agnostic close-only Donchian alignment with **no future-index look-ahead** (window correctly excludes current bar). Best of the OTF/MTF pair. | Reads forming bar (`cl[last]`) → flickers intrabar; stores no history → not backtestable as a gate; Mode 1 is a dead duplicate. |
| `DOMReaderV2.cpp` | Liquidity-event signal | Causal closed-bar collapse/vacuum + contested-resolve markers. | Forward-only DOM → unbacktestable; 1-bar lag. |
| `BreadthCompositeOscillator.cpp` | Directional bias (US index futures, RTH only) | Independent internals-based confirmation ($VOLD/$ADD) — orthogonal to price-based signals. | US-cash-equity-RTH hardcoded (390 min, 09:30–16:00); intrabar repaint; inverted dynamic weighting over-weights a dead stream. |
| `DISCStudies.cpp` (ATR2Risk) | Position sizing | Instrument-agnostic constant-dollar ATR sizing — useful glue for any of the auto-traders. | Div-by-zero on ATR==0 (line 103); min-1 clamp can exceed the risk cap; never places the implied stop. |
| `InterestMap` / `TrappedTraders` / `LiquidityZones` | Reaction-level context | Different ways to mark S/R reaction zones for a discretionary or zone-based trader. | All repaint on recalc; zone studies use ES-tuned un-normalized delta thresholds; LiquidityZones additionally has `Open[bi+1]` look-ahead. |

---

## Rejected / not profit-capable

### Pure visualization — no entries/exits/orders (cannot produce P&L)
`BigTradesTape`, `ReconTape`, `ReconTapeV2`, `DeltaVelocityProfile`, `AbsorptionGradient`, `AVWAPRotation`, `TraderOracle`/Olympus (paints arrows only — flagship of its repo, but no orders), `Killpips`, `mancini`, `LRS`, `MarketMaker`, `TraderSmarts_Unofficial`, `TapeReader` (no signal), `DanZoneMaster`/`DanTheDumbTrader` (recolor only), `DanBracketMaster` (bracket painter + manual panic-flatten of externally-owned positions — despite its README billing as "auto-detect consolidation zones", it places no orders of its own). The entire `SierraChart-Studies` repo is in this bucket functionally — grep-confirmed **zero** order calls across all 20 files.

### Repaint / look-ahead (signal differs live vs history)
- `OTFStateFilter` — cross-chart mode paints **all history with the latest regime** (lines 201-223/231); textbook look-ahead.
- `LiquidityZones` — reads `sc.Open[bi+1]` (lines 762/816) + destructive intrabar zone invalidation.
- `CDVolumeAbsorption` — full-day min-max norm recomputed every tick → look-ahead *within the session*.
- `VolImbRenko` — reads 4 future bars by design (and doesn't compile).
- `Renko_GOAT` / `TORobots` / `TraderOracle` (Olympus) — treat the forming bar as closed (`BarCloseStatus` is a "not-last-bar" flag); `TORobots` even aliases `close` to the bar OPEN. Olympus additionally has recursive Supertrend/Fisher and an intrabar-mutated squeeze state that are not new-bar-guarded → historical paint differs between live and reload (and the Fisher feedback array is never written, so that filter is silently broken).
- `OrderflowConfluence/V2/V3` — last-bar `i-1` leniency makes live diverge from recalc.

### Overfit / hardcoded
- `NQ_Breakout_Ranges_Jan2025` — **maximally overfit**: 292 hardcoded Jan-2025 NQ ranges, period in the filename/SCDLLName/GraphName; inert live; look-ahead.
- `EUSupportBounce`, `RangeDetector`, `SC_Range_Bounce_Auto` — instrument/session-baked constants, ATR gate 0.82–7.23, NQ-point thresholds, Cyprus/EU session hours.
- `mancini`/`ManciniPlusConverter`/`Killpips`/`MarketMaker` — ES-locked, stale snapshot levels, `Left(4)` price truncation.

### Incomplete / stub / does-not-compile
- **Will not compile:** `TORobots` (undefined `ordertype`, line 94), `MarketMaker` (undeclared `Input_RecalcInterval`/`Input_6_Lines`), `VolImbRenko` (duplicate local declarations).
- **Half-built:** `DeltaReversalTrigger` (computes ingredients, never forms a trigger), `DanScalpMaster` (sophisticated bracketed strategy fully written but **never called**; only a mis-coded naked-entry path runs), `ManciniPlusConverter` (conversion feature dead; draws lines at price 0).

### Redundant duplicates (keep the winner, retire the rest)
| Retire | Keep | Reason |
|---|---|---|
| `DOMReader` | `DOMReaderV2` | V2 adds the statistical baseline + closed-bar triggers. |
| `OrderflowSignalV2` | `OrderflowSignalV3` | V3 adds the retroactive-alert watermark; V2 has stale-index alerts + Input off-by-one. |
| `OrderflowConfluence` | `OrderflowSignalV3` | The bare inert ancestor of the same aggregator. |
| `ReconTapeV2` | `ReconTape` | V1 has a correct trailing percentile; V2 added a coarse percentile + color-bar bug. |
| `DanTheDumbTrader` | `DanZoneMaster` | Near-duplicate; ZoneMaster is the one wired into DeltaWiz. |
| `DanStopLock` | `DanDeltaWizWorkerSlave` | Both stop-ratchets; WorkerSlave is ensemble-native and less dangerous. |
| `MarketMaker` | `Killpips` | MarketMaker is a non-compiling clone. |
| `ManciniPlusConverter` | `mancini` | Converter's headline feature is dead code. |
| `TORobots` (trade) | `Renko_GOAT` (paint) | Same 8-indicator stack; TORobots doesn't compile. |

---

## Correlations & how to combine

### The intended order-flow pipeline (producer → consumer)
`SierraChart-Studies` is architected as a stack: single-thesis producers expose subgraphs → `OrderflowSignalV3` aggregates up to 50 of them into a weighted confluence score (its hardcoded ~33-study table is literally a list of these siblings' IDs). Intended flow:

```
DeltaVelocityProfile + DeltaReversalTrigger + AbsorptionGradient
 + TrappedTraders + LiquidityZones + DOMReaderV2 + BigTradesTape/ReconTape
        → OrderflowSignalV3 (aggregator)  → [human discretion]
```

**Why this stack is broken as-is:**
- **Direction-blind aggregator:** V3 fires on *any* non-zero value with all-ARROWUP subgraphs — it sums bullish and bearish triggers identically. You cannot derive long vs short.
- **Correlated, not independent, votes:** `DeltaReversalTrigger`, `DeltaVelocityProfile`, `FlowConviction`, `TapeReader`, `AbsorptionGradient` all re-express the same `SC_ASKVOL − SC_BIDVOL` signed-delta idea. "Confluence" over them is **double-counting one signal**, silently breaking the aggregator's independence assumption.
- **Fragile coupling:** the 33-study table is keyed to chart-instance Study IDs and silently breaks on any other layout.

### The TraderOracle intended stack (from chartbook reverse-engineering)
`Olympus` (multi-confluence signal, ID1) + `Delta_Intensity` (ID2) + `WaddahExplosion` confirmation → `TradingSystemBasedOnAlertCondition` studies translate spreadsheet booleans into orders → `GoldBug` (TORobots) executes 4 setups. Context from VWAP / 200-EMA / KAMA / Bollinger / Pivots. Instrument: **NQ 2000-trades-per-bar, Sim account.** This is a coherent *design* — but every load-bearing piece is defective. The signal core, **`Olympus` (read directly from `TraderOracle.cpp`)**, is an AND-stack of ~8 classic trend filters that **emits arrows only — it places no orders** — and its own internals are broken (Supertrend lower-band index bug at line 827, Fisher recursion that reads a never-written feedback array, intrabar squeeze state). The execution module (`TORobots`) doesn't compile (undefined `ordertype`, line 94), has no exits, and pyramids; and the line-drawers feed nothing programmatically (no exported signals). So the "system" is a paint-only signal whose orders depend on a non-compiling executor.

### Combinations that actually make sense
1. **A fixed tradeable core + orthogonal filters:** Take *one* fixed signal source (e.g. `DOMReaderV2` markers, or the `DanZoneDeltaReversalTrader` zone+delta rule) → gate with **`MTFCloseFilter`** (trend alignment, instrument-agnostic, no look-ahead) → bias with **`BreadthCompositeOscillator`** (only on US index futures, RTH) → size with **`DISCStudies` ATR2Risk**. These four are genuinely *orthogonal* (book churn vs MTF structure vs internals vs sizing).
2. **Do NOT stack the delta oscillators** (`FlowConviction` + `DeltaVelocity` + `AbsorptionGradient` + `DeltaReversalTrigger`) — they are the same signal; pick one.
3. **Do NOT run two zone painters** (`DanZoneMaster` vs `DanTheDumbTrader`) or two stop managers (`DanStopLock` vs `WorkerSlave`) or two range-faders (`RangeDetector` vs `SC_Range_Bounce_Auto`) — they conflict/duplicate.

### Where signals conflict
- Running `DanBracketBot`/`DanScalpMaster` on the same chart as the DeltaWiz triad → duplicate/stacked orders and fighting OCO brackets (they share `areWeInPosition()` and the same SC trade settings).
- Filters that read forming bars (`MTFCloseFilter`, `BreadthComposite`, `OTFStateFilter` native) flicker intrabar — stacking them *multiplies* the intrabar-flicker problem rather than reducing it. Confirm on bar close only.

---

## Caveats

1. **Nothing here is proven profitable.** Across all 44 studies, **zero** have `edgeSurvives = true`. The shortlist is "best bones / worth fixing," not "deploy this."
2. **The Strategy Optimizer is an in-sample grid-search runner only.** Its plumbing is correct (drives SC replay via `StartChartReplayNew`, replayMode=2 accurate fills, harvests `GetTradeStatisticsForSymbolV2`), but it has **no walk-forward, no out-of-sample, no holdout, no cross-validation, no robustness penalty**. `ResultAnalyzer` ranks strictly by Total P/L and surfaces the single highest in-sample combo as "best" — the canonical selection-bias / overfit trap. **Optimizer output is a hypothesis, not validation.**
3. **You must split train/test manually.** There is no `endDate` parameter — every replay runs to the end of loaded data, so the only way to define a test window is by controlling how much history the chart has loaded. Optimize on train, pick a **parameter plateau (not the peak)**, then re-run a *separate untouched later window* with params fixed (increment=0).
4. **Realism depends on your Sim account.** The harness does not enforce commissions/slippage — verify they're configured, or in-sample P/L is optimistic.
5. **Most "studies" must be converted before backtesting at all.** The optimizer measures realized trade P/L; signal/visualization studies (the entire `SierraChart-Studies` repo) produce **zero trades** and a degenerate report. They need real `sc.BuyEntry/SellEntry` + target/stop/enable inputs added first.
6. **DOM- and tape-based members are unbacktestable by construction.** Depth and T&S are forward-only and not persisted (`DOMReaderV2`, `DOMReader`, `ReconTape/V2`, `InterestMap` whale evidence) — the *best-architected* signal in the corpus cannot be validated out-of-sample on this platform.
7. **Fix the listed live-capital bugs before any real-money or even sim test of the auto-traders** — wrong-sign short targets, always-true trailing guards, order stacking with `MaximumPositionAllowed = 1e8`, non-functional `Enabled` toggles, and last-fill-only position detection are present in the tradeable candidates and will distort or invalidate results.