# AbsorptionGradient — Playbook

**Rank: 🥉 #3 — deploy first.**

## What it does (plain English)
Measures **effort vs. result**: how much order flow (effort) it's taking to move
price (result). When lots of volume produces little price movement, the market is
"absorbing" — often a sign a move is running out of gas. It also flags **trapped
traders**, which are great reversal cues.

## Why it's worth running
- Tells you when a trend is **getting tired** before price actually turns.
- The **trapped-trader markers** are clean, actionable reversal signals.
- Works on any bar type and needs no other studies.

## What you need
- **Order-flow data** (bid/ask volume).
- Works on **any bar type** (time, range, volume, tick, Renko). It can auto-detect
  range bars and adjust how it measures price change.

## How to read it
| Output | Looks like | Meaning → Action |
|---|---|---|
| **Absorption Gradient** | Yellow line, 0–100 | Near **100** = exhausted (effort piling up, price stuck) → watch for a turn. Near **0** = efficient, healthy move. |
| **Efficiency Ratio** | Blue line, 0–100 | The mirror image: high = clean directional move, low = choppy/absorbing. |
| **Gradient Slope** | Green/Red histogram | **Green** = absorption getting worse (exhaustion building). **Red** = energy returning. |
| **Trapped Bars** | Cyan / Orange dots | **Cyan** = price went up but delta was net selling → **shorts trapped** (bullish). **Orange** = price went down but delta was net buying → **longs trapped** (bearish). |

## Key settings (starting points)
| Setting | Default | Plain meaning |
|---|---|---|
| Window Length | `15` bars | How many bars to measure effort/result over |
| Price Change Mode | `1` (Close-to-Close) | How "result" is measured; `2` = auto-detect range bars |
| Tick Size | `0.25` | Your contract's tick (ES = 0.25, NQ = 0.25, set per market) |
| Trapped Delta Threshold | `0.30` | Lower = more trapped-trader signals (more sensitive) |
| Gradient Slope Lookback | `4` bars | How far back to measure the exhaustion slope |

*Tuning tip:* too many trapped dots → **raise** the Trapped Delta Threshold; too
few → **lower** it. Set **Tick Size** correctly for your instrument.

## Best use
- **Markets:** ES / NQ and other liquid futures.
- **Bar type / timeframe:** any; for scalping use tick/range/volume bars.
- **Combine with:** FlowConviction (exhaustion + divergence together is strong)
  and OTFStateFilter (a trapped-trader dot *against* the trend = fade setup; *with*
  the trend = pullback entry).

## Gotchas
- Set **Tick Size** to your contract or the absorption scale will be off.
- Needs order-flow data; flat without bid/ask volume.
- The first ~15–20 bars are warmup and won't plot.

**Bottom line:** Your exhaustion + trapped-trader radar. Best near support/
resistance and after extended moves.
