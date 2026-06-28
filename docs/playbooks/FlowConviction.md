# FlowConviction — Playbook

**Rank: 🥈 #2 — deploy first.**

## What it does (plain English)
Answers one question well: **is buying or selling actually in control right now,
or is this just noise?** It blends three order-flow measures into one colored
histogram and warns you when price and order flow disagree (a divergence).

## Why it's worth running
- Cuts through chop — the color only turns strongly green/red when conviction is
  real (it needs two bars to confirm, so it doesn't flicker).
- The **divergence warnings** (price makes a new high/low but flow doesn't back
  it) are some of the best reversal tells in the pack.
- Self-contained — no other studies needed.

## What you need
- **Order-flow data** (bid/ask volume). Run it on a chart that has order-flow
  data, e.g. a **Numbers Bars / footprint**-capable chart or a live feed.
- Built for **ES tick charts (100T / 200T)** per the author's notes.

## How to read it
| Output | Looks like | Meaning → Action |
|---|---|---|
| **Composite background** | Green / Red / Gray / Orange / Purple | **Green** = buyers in control. **Red** = sellers in control. **Gray** = no clear winner (stand aside). **Orange** = price up but flow is bearish → *bearish divergence*. **Purple** = price down but flow is bullish → *bullish divergence*. |
| **Sustained Aggression** | White line | Above 0 = net buying pressure, below 0 = net selling |
| **Delta Persistence** | Yellow line | Near +100 = buyers one-sided; near −100 = sellers one-sided |
| **Divergence Warning** | Orange / Cyan dots | **Cyan** = absorption at a low (possible bottom). **Orange** = distribution at a high (possible top). |

## Key settings (starting points)
| Setting | Default | Plain meaning |
|---|---|---|
| Large Print Size | `10` | Trades this size or bigger count as "big" |
| Large Print Weight | `3.0` | How much extra weight big trades get |
| Contested Zone Width | `15` | How close to zero stays "gray/neutral" — raise it to be more selective |
| Aggression Window | `20` bars | How many bars of buying/selling pressure to add up |
| Persistence Window | `20` bars | Window for the one-sidedness measure |
| Divergence Lookback | `10` bars | How far back to look for the high/low used in divergence |

*Tuning tip:* if you get too many weak color flips, **raise Contested Zone Width**.
If divergences are too rare, **lower the Divergence Aggression Threshold**.

## Best use
- **Markets:** ES first, then NQ / other liquid futures.
- **Bar type / timeframe:** ES 100T or 200T tick bars (or comparable volume bars).
- **Combine with:** OTFStateFilter (trade green signals only when trend = Up,
  red only when trend = Down). Use divergence dots near InterestMap levels.

## Gotchas
- Needs real order-flow data — on a plain time chart with no bid/ask volume it
  will read flat.
- Color needs two bars to flip (on purpose). Don't expect instant turns.

**Bottom line:** Your "who's really winning" gauge, plus a reliable divergence
early-warning. Pair it with the trend filter.
