# DOMReaderV2 — Playbook

**Rank: #5 — high value, needs a live depth feed.**

## What it does (plain English)
Watches the **order book (DOM)** in real time and measures whether resting orders
are being **stacked** (added — conviction) or **pulled** (yanked — fear/spoofing).
On top of that it fires two trade triggers: a **Collapse/Vacuum** (a big wall on
one side suddenly vanishes) and a **Contested-then-Resolve** (both sides fight,
then one breaks).

## Why it's worth running
- Shows *intent* in the book that you can't see from price alone — walls building
  or evaporating.
- The **Collapse** and **Contested** triggers are concrete, bar-close signals with
  built-in cooldown so they don't spam.
- Tags each trigger by whether it agrees with the short-term price slope.

## ⚠️ Important limitation
- **Needs a LIVE market-depth feed.** Sierra Chart can't rebuild depth from
  history, so this works **forward-only** — historical bars before you load it
  stay blank, and you **can't backtest** the depth part on replay of old data.

## What you need
- A feed providing live DOM (e.g. CME via Denali/Teton, or Rithmic).
- Authored for **200-volume ES** bars.

## How to read it
| Output | Looks like | Meaning → Action |
|---|---|---|
| **Net Pull/Stack** | Blue/Red histogram | Blue = bids being stacked (bullish book), Red = asks being stacked (bearish book) |
| **Collapse – Bull** | Up arrow | A bearish wall evaporated → room to run **up** |
| **Collapse – Bear** | Down arrow | A bullish wall evaporated → room to drop |
| **Contested – Bull / Bear** | Purple arrow | Both sides were hot, then the book broke that way (breakout after a standoff) |
| Arrow brightness | Bright / muted / gray | **Bright** = agrees with price slope (stronger). **Muted** = against slope. **Gray** = slope neutral. |

## Key settings (starting points)
| Setting | Default | Plain meaning |
|---|---|---|
| Event Lookback (W) | `6` bars | Window to spot the wall peak before a collapse |
| Tall/Hot Sigma (k) | `2.0` | How unusually big a wall must be to count |
| Collapse Drop (M) | `2.0` | How hard it must drop to fire a collapse |
| Net Breakout (J) | `1.5` | How hard the book must break for a contested signal |
| Min Contested Bars | `2` | Both sides must be hot this many bars first |
| Cooldown | `5` bars | Quiet period after a trigger (anti-spam) |
| Warmup | `30` bars | Stats need this many bars before triggers turn on |
| SMA Length | `10` | Price trend used to brighten/mute markers |

*Tuning tip:* too many triggers → raise **k / M / J** or **Cooldown**; too few →
lower them.

## Best use
- **Markets:** ES first (deep book); NQ works too. Avoid thin books.
- **Bar type / timeframe:** volume bars (e.g. 200-volume ES) or tick bars.
- **Combine with:** feed its Collapse/Contested triggers into **OrderflowSignalV3**
  as weighted triggers; confirm with OTFStateFilter direction.

## Gotchas
- No depth feed = no signals. Confirm your feed actually provides DOM.
- Forward-only: load it and let it run; don't expect it to fill in the past.
- Start on **sim/replay live** to learn the triggers before trading them.

**Bottom line:** Your live order-book intent reader. The Collapse and Contested
triggers are the payoff — but only with a real depth feed.
