# OrderflowSignalV3 — Playbook

**Rank: #4 — the centerpiece, but needs wiring first.**

## What it does (plain English)
This is the **scoreboard**. It watches up to 50 other order-flow signals on your
chart, gives each one a weight (1–3 points), adds them up per bar, and prints
**graded arrows**: Level 1 (small), Level 2 (medium), Level 3 (big conviction).
It also shades a **confluence background** when lots of signals fire close
together. Add it **twice** — once wired to bullish signals, once to bearish.

## Why it's worth running
- Turns a messy wall of indicators into **one clean, ranked signal**.
- The strongest tool here once set up — it's how you combine everything else
  (DOMReaderV2 triggers, FlowConviction, InterestMap, etc.) into a single read.
- Smart on live data: it counts a higher-timeframe trigger **only once** (when it
  first fires), so you don't get fake "high conviction" from reprints.

## ⚠️ Setup required (this is why it's #4, not #1)
It **ships empty on purpose** — all 50 trigger slots are blank. You must wire it
to *your* chart's studies:
1. Add your order-flow studies to the chart (e.g. DOMReaderV2, FlowConviction).
2. Open this study's settings → for each Trigger, pick a **Study + Subgraph** and
   a **weight** (1/2/3 points).
3. Repeat on a second copy for the bearish side.

## What you need
- At least one other order-flow study on the chart to point at.
- Runs on bar close (skips mid-bar noise). No special data feed of its own.

## How to read it
| Output | Looks like | Meaning → Action |
|---|---|---|
| **Level 1 arrow** | Small yellow | Weak signal (score ≥ 3) — minor confirmation |
| **Level 2 arrow** | Orange | Solid signal (score ≥ 5) |
| **Level 3 arrow** | Big green | Strong, multi-source agreement (score ≥ 8) — your A+ setups |
| **Confluence background** | Light blue shade | A recent cluster of signals — a "hot zone," often near reversals/continuation |
| **Bar Score** | Histogram | This bar's raw point total (colored by the tier that fired) |
| **Debug: Connected / Firing** | Lines | Sanity check: how many triggers are wired and how many fired |

## Key settings (starting points)
| Setting | Default | Plain meaning |
|---|---|---|
| Level 1 / 2 / 3 Threshold | `3 / 5 / 8` | Points needed for each arrow tier — raise for fewer, higher-quality arrows |
| Lookback Window | `5` bars | Window for the confluence background |
| Confluence Threshold | `6` | Points within the window to light the background |
| Level 1/2/3 Window | `1` bar | Add scores over N bars; raise if signals spread across bars in fast moves |
| Signal / Confluence Alert | `0` (off) | Set a sound number to get alerts — turn on **last**, once you trust the inputs |
| Enable OTF Filter | `No` | Optionally only show arrows when a trend/condition study agrees |

## Best use
- **Markets:** ES / NQ (whatever your wired studies are tuned for).
- **Bar type / timeframe:** match the bars your trigger studies use (tick/volume
  for scalping).
- **Combine with:** wire OTFStateFilter into the **OTF Filter** slot so arrows
  only fire in the trend direction.

## Gotchas
- Empty out of the box — no wiring, no arrows. That's expected.
- After a Sierra Chart restart it may need one manual *Recalculate* if the trigger
  studies load after it (it tries to self-fix).
- Settings reference studies by ID, which are **chart-specific** — re-wire if you
  copy it to another chart.

**Bottom line:** The master signal engine. Invest 20 minutes wiring it; it pays
back by collapsing everything into one ranked arrow.
