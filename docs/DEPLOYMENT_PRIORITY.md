# Deployment Priority — Which Studies to Load First

A plain-English ranking of the studies in this repo, best to worst, for someone
deciding **what to put on a Sierra Chart layout first**.

> **Read this first — what these actually are.**
> These are **decision-support studies** (indicators, signals, alerts). They do
> **not** place orders by themselves — none of them is a hands-off "bot" that
> auto-trades your account. They tell you *what the order flow is doing* and
> *when conditions line up*; you (or a separate execution tool) still pull the
> trigger. All default settings below are **sensible starting points**, not
> magic numbers — expect to tune them per market.

---

## The ranking at a glance

| Rank | Study | What you get | Setup effort | Ready out of the box? |
|---|---|---|---|---|
| 🥇 1 | **OTFStateFilter** | Up / Down / Neutral trend state | None | ✅ Yes |
| 🥈 2 | **FlowConviction** | Is buying or selling really in control? + divergence warnings | Low | ✅ Yes (needs order-flow data) |
| 🥉 3 | **AbsorptionGradient** | Exhaustion + trapped-trader reversal markers | Low | ✅ Yes (needs order-flow data) |
| 4 | **OrderflowSignalV3** | Master signal engine — combines many signals into graded arrows | High (wiring) | ⚙️ After you wire triggers |
| 5 | **DOMReaderV2** | Order-book pull/stack + collapse & breakout triggers | Medium | ✅ Yes (needs live depth) |
| 6 | **InterestMap** | Auto-drawn key levels with approach + whale alerts | Medium | ✅ Yes (needs footprint + T&S) |
| 7 | MTFCloseFilter | Multi-timeframe trend backdrop (6 charts) | Medium | ⚙️ After you set chart numbers |
| 8 | BreadthCompositeOscillator | Whole-market up/down bias (NYSE breadth) | Medium | ⚙️ Needs $VOLD / $ADD |
| 9 | LiquidityZones | Supply/demand zones | Medium | ✅ Yes (needs footprint) |
| 10 | TrappedTraders | Trapped-trader zones | Medium | ✅ Yes (needs footprint) |
| 11 | OrderflowConfluence | Older bull/bear engine (no alerts) | High (wiring) | ⚙️ Superseded by #4 |
| 12 | ReconTapeV2 | Multi-tier tape reconstruction | Low | ✅ Reading tool |
| 13 | ReconTape | Tape bubbles + zones of interest | Low | ✅ Reading tool |
| 14 | BigTradesTape | Highlights large trades + saves history | Low | ✅ Reading tool |
| 15 | DeltaVelocityProfile | Delta momentum / acceleration | Low | ✅ Reading tool |
| 16 | DeltaReversalTrigger | Delta + sigma bands | Low | ✅ Reading tool |
| 17 | TapeReader | Raw aggression flow | Low | ✅ Reading tool |
| 18 | AVWAPRotation | Auto anchored-VWAP per swing | Low | ✅ Reading tool |

Full playbooks for the top 6 live in [`playbooks/`](playbooks/).

---

## Why this order

### Tier 1 — Deploy these first (high value, plug-and-play)
These are complete, self-contained, and give an edge immediately with almost no setup.

1. **[OTFStateFilter](playbooks/OTFStateFilter.md)** — The foundation. It tells
   you the trend regime (Up / Down / Neutral) with a simple, mechanical rule.
   Everything else trades better when you only take signals *in the direction of
   this state*. Zero configuration.
2. **[FlowConviction](playbooks/FlowConviction.md)** — Answers the core scalping
   question: *is one side actually in control, or is this noise?* Plus it warns
   when price and order flow disagree (divergence) — often the best reversal tell.
3. **[AbsorptionGradient](playbooks/AbsorptionGradient.md)** — Spots when a move
   is running out of gas (exhaustion) and flags **trapped traders**, which are
   high-quality reversal/continuation cues.

### Tier 2 — Deploy next (high value, but need data or wiring)
4. **[OrderflowSignalV3](playbooks/OrderflowSignalV3.md)** — The most powerful
   tool here: it fuses up to 50 other signals into clean Level 1/2/3 arrows. It's
   ranked below Tier 1 only because **you must wire it to your other studies
   first** (it ships empty on purpose). Once wired, it's the centerpiece.
5. **[DOMReaderV2](playbooks/DOMReaderV2.md)** — Reads the live order book for
   walls being pulled or stacked, and fires "collapse" and "breakout" triggers.
   Needs a **live market-depth feed**; it can't be backtested on history.
6. **[InterestMap](playbooks/InterestMap.md)** — Continuously draws the price
   levels that matter and alerts when price approaches them or a whale trades
   there. Needs **footprint (Volume-at-Price) data + Time & Sales**.

### Tier 3 — Confluence / context (run as a backdrop)
7–11. **MTFCloseFilter, BreadthCompositeOscillator, LiquidityZones,
TrappedTraders, OrderflowConfluence.** Useful supporting context (higher-timeframe
trend, whole-market bias, zones), but each needs more setup or overlaps with a
higher-ranked tool. OrderflowConfluence is the old engine — prefer #4.

### Tier 4 — Reading tools (situational, not signal generators)
12–18. Tape and delta visualizers. Great for *manual* tape reading and
confirmation, but they don't produce a clear "do X" signal, so they're lowest
priority for an automated/alert-driven workflow.

---

## Suggested rollout plan

1. **Week 1:** Load OTFStateFilter + FlowConviction + AbsorptionGradient on your
   main scalping chart. Learn to read them together (trend + conviction +
   exhaustion). No wiring needed.
2. **Week 2:** Add DOMReaderV2 (if you have a depth feed) and InterestMap (if you
   run footprint charts). These add "where" and "when the book breaks."
3. **Week 3:** Wire OrderflowSignalV3 to the studies above and let it grade the
   confluence into arrows. Turn on its alerts last, once you trust the inputs.
4. **Ongoing:** Layer Tier 3 context studies as needed; keep Tier 4 for manual
   tape reading.

---

## Best markets & timeframes (general guidance)

- **Markets:** These are order-flow tools, so they work best on **deeply liquid
  futures** — first choice **ES** (E-mini S&P 500), then **NQ** (Nasdaq), and
  other liquid contracts (CL, GC). Thin markets give noisy order flow.
- **Bar type:** Scalping order flow reads best on **tick, volume, or range bars**
  (e.g. ES 100T/200T or 200-volume), not large time bars — those blend too much
  flow together. Per-study specifics are in each playbook.
- **Data feed:** Depth-based tools (DOMReaderV2) need a feed that provides live
  market depth (e.g. CME via Denali/Teton or Rithmic). Footprint tools
  (InterestMap, LiquidityZones, TrappedTraders) need Volume-at-Price enabled.

> ⚠️ **Not financial advice.** Defaults are starting points. Test on a sim/replay
> account, confirm each study compiles (F5) and behaves on your data, and tune
> before risking money.
