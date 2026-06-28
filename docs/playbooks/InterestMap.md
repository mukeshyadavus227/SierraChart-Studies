# InterestMap — Playbook

**Rank: #6 — high value, needs footprint + Time & Sales.**

## What it does (plain English)
Automatically finds and **draws the price levels that matter** (support/resistance)
and keeps them updated. It builds each level from real evidence: heavy
volume-at-price, two-way absorption, big "whale" trades on the tape, and whether
price came back and **defended** the level. It then **alerts** you when price
approaches a level or a whale trades right at one.

## Why it's worth running
- Hands you the key levels without manual drawing, and they **update and fade**
  as the market changes (old levels decay, defended ones get stronger).
- The **approach** and **whale-at-level** alerts tell you when to pay attention.
- Levels are colored by side (buy interest vs sell interest), so you know which
  way they're likely to act.

## What you need
- **Footprint / Volume-at-Price data** (turn on Numbers Bars / VAP).
- **Time & Sales** (for the whale detection).
- Works on any bar type; it rebuilds from chart history when loaded.

## How to read it
| Output | Looks like | Meaning → Action |
|---|---|---|
| **Blue band** | Horizontal zone below price | Buy interest (potential support) |
| **Red band** | Horizontal zone above price | Sell interest (potential resistance) |
| **Label** e.g. `S12 T3 D2` | Text in the band | Score 12, Touched 3 times, Defended 2 times — higher = stronger level |
| **Gray band** | Faded zone | A level that got broken; kept for context, fades away |
| **Approach alert** | Sound | Price is within N ticks of a live level |
| **Whale-at-level alert** | Sound | A big aggressor just traded inside a level |

## Key settings (starting points)
| Setting | Default | Plain meaning |
|---|---|---|
| Bucket Width | `4` ticks | How tightly levels are grouped (scale per instrument) |
| Baseline Length | `200` bars | Window used to decide what counts as "heavy" volume |
| Min Volume Percentile | `95%` | A level must rank in the top 5% of volume to qualify |
| Whale Min Size | `200` | Trade size (contracts) to count as a whale (`0` = off) |
| Decay Half-Life | `300` bars | How fast old, untested levels fade |
| Max Levels Shown | `8` | How many top levels to draw |
| Approach Alert (ticks) | `0` (off) | Set to e.g. `4` to alert when price nears a level |
| Whale-At-Level Alert | `On` | Alert when a whale hits a level |

*Tuning tip:* too many levels/clutter → raise **Min Volume Percentile** or lower
**Max Levels Shown**; adjust **Whale Min Size** to your market's typical big-trade size.

## Best use
- **Markets:** ES / NQ and other liquid futures with good footprint data.
- **Bar type / timeframe:** any; commonly footprint/volume or range bars.
- **Combine with:** take FlowConviction divergences and AbsorptionGradient
  trapped-trader dots **at these levels** — that's where reversals cluster.

## Gotchas
- Needs VAP **and** Time & Sales; without them it has little to work with.
- Set **Bucket Width** sensibly per instrument (too small = clutter, too big = blur).
- Whale history is rebuilt within a Sierra Chart session; a restart re-warms it.

**Bottom line:** Your auto-drawn, self-updating level map with alerts. Best used
as the "where" that the order-flow studies react around.
