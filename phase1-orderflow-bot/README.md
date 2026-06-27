# Phase 1 — Order-Flow Bot: a fail-fast edge test

This folder is **not** a finished trading product. It is the smallest honest
experiment that can answer one question cheaply:

> Does a single order-flow trigger, gated by a single trend filter, with a real
> bracket, have **positive expectancy out-of-sample** on your instrument?

If the answer is no, you stop here having spent days, not months. That is the
point — it is designed to *fail fast and cheap*. It deliberately rejects the
TraderOracle/GoldBug pattern (8 correlated indicators, no exits, no validation).

## What's here

| File | What it is |
|---|---|
| `OrderFlowOCOBot.cpp` | A minimal ACSIL strategy: **one** order-flow trigger + **one** trend gate + a **real OCO bracket** + risk controls + train/test date windows. |
| `walkforward/walkforward.py` | Turns the Strategy Optimizer's in-sample grid search into an honest **walk-forward / out-of-sample** evaluation, and scores a trade list. No deps beyond Python 3 stdlib. |

## The strategy (intentionally tiny)

- **Trigger (order-flow, independent of price):** per-bar delta = `AskVol − BidVol`,
  converted to a session **z-score** (self-calibrating mean/σ via Welford, reset
  each trading day). Two selectable modes:
  - `Delta Thrust` — trade **with** a delta push beyond `k`σ (momentum).
  - `Absorption` — strong **opposing** delta that price refused (fade).
- **Gate (one filter):** sign of an EMA slope with a tick deadband. Optional.
- **Execution:** market-on-signal entry (no limit-at-passed-price fill fiction)
  with an attached **target + hard stop** in OCO group 1 — a true bracket.
- **Risk:** one position at a time, max trades/day, daily-loss circuit breaker,
  optional session window, data-integrity halt (no bid/ask volume → never trades).
- **No repaint / no look-ahead:** decisions only on **closed** bars; position
  state read from actual fills (`PositionData`), never from order return codes.

Every threshold (`k`, EMA length, slope deadband, stop ticks, target R, …) is an
**input**, so the optimizer can grid it and nothing is silently curve-fit.

> ⚠️ Safety defaults: `Enabled = No` and `Send Orders To Trade Service = No`.
> It will not place anything until you turn it on, and only on a Sim account
> until you explicitly flip it live.

## Install / build

1. Copy `OrderFlowOCOBot.cpp` to Sierra Chart's `ACS_Source` folder.
2. **Analysis → Build Custom Studies DLL** (F5).
3. Add **OrderFlow OCO Bot (Phase 1)** to a chart that carries **Bid/Ask volume**
   (futures with a proper feed). Confirm the *Delta Z* subgraph is non-zero — if
   it's flat, your chart has no bid/ask volume and the bot is inert by design.

## The Phase-1 protocol (run this, in order)

**Step 1 — pick rolling windows.** Choose N consecutive time windows (e.g. 4
months → 4 folds). The bot's `Trade Window Start/End (YYYYMMDD)` inputs let you
confine trading to one window at a time; the optimizer has no end-date, so this
is how you control the train/test split.

**Step 2 — grid-search each window (in-sample).** Using `SierraChartStrategyOptimizer`
(sibling repo), target `scsf_OrderFlowOCOBot` and grid the parameters you care
about (start with `k`, `emaLen`, `targetR`, `stopTicks`). Run it **once per
window**, each producing a `…summary.csv`. Label them in time order, e.g.
`f1.csv f2.csv f3.csv f4.csv`. **Configure commissions + slippage in the Sim
account first** — otherwise every number is optimistic.

**Step 3 — walk-forward evaluate (out-of-sample).**

```bash
python walkforward/walkforward.py walkforward \
    --windows f1.csv f2.csv f3.csv f4.csv \
    --param-cols k emaLen targetR stopTicks \
    --metric TotalPnL --trades-col Trades \
    --min-trades 30 --top-frac 0.20 --pass-frac 0.60
```

For each adjacent pair it picks a **robust parameter plateau** on the train fold
(not the single peak), then scores those exact params on the **next** fold (which
the selection never saw). It prints per-fold OOS results and a **GO / NO-GO**
verdict.

**Step 4 — if GO, forward-test (Phase 3).** Run live on Sim for a meaningful
window with the chosen params *fixed*. Export the trade list and check it:

```bash
python walkforward/walkforward.py tradelist --file sim_trades.csv \
    --pnl-col "Profit/Loss" --min-trades 30
```

Only proceed to tiny live size if the **out-of-sample** expectancy CI is entirely
above zero.

## The GO / NO-GO rule (explicit — decide *before* you look)

**GO** (continue to forward sim) requires **all** of:
- OOS-positive in **≥ 60%** of folds (`--pass-frac`),
- **summed** OOS metric > 0,
- average **OOS/IS degradation ≥ 0** (i.e. OOS doesn't flip negative; tighten to
  e.g. `--max-degrade 0.4` to demand it keep ≥40% of in-sample edge),
- each scored fold meets `--min-trades` (small samples don't count).

Anything else is **NO-GO → stop.** A NO-GO is the tool *working*: it just saved
you the months GoldBug would have wasted. Most first triggers fail here — expected.

## What this proves — and what it does not

- ✅ Proves: whether *this specific* trigger+gate+bracket survived an honest
  train/test split with costs, on your data.
- ❌ Does **not** prove a durable, all-weather edge. Out-of-sample success on a
  few folds is necessary, not sufficient — regimes change.
- The walk-forward harness fixes the optimizer's **biggest** flaw (in-sample-only,
  ranked by P/L → selection bias). It does not add commissions/slippage for you —
  that lives in your Sim account config.
- The reusable assets here (the **OCO execution skeleton** and the **walk-forward
  harness**) outlive this trigger. If it fails, swap in trigger #2 and re-run
  Steps 2–3 in days. That reusability is the real deliverable.

## Honest expectation

Building a *consistently* profitable automated futures bot is hard and most
attempts fail out-of-sample. This scaffold doesn't change those odds — it just
makes the answer **cheap, fast, and honest**, so you commit money only to
something that already survived scrutiny.
