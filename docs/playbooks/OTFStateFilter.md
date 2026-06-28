# OTFStateFilter — Playbook

**Rank: 🥇 #1 — deploy first.**

## What it does (plain English)
Tells you whether the market is in an **Up**, **Down**, or **Neutral** trend, using
a simple mechanical rule based on the shape of recent bars. Think of it as your
"which way am I allowed to trade?" filter.

## Why it's worth running
- **Zero setup** — drop it on and it works.
- Keeps you out of countertrend trades. The simplest edge: only take other
  studies' long signals when this says **Up**, shorts when it says **Down**.
- Can read structure from a *different* chart (e.g. a Renko chart) while sitting
  on your fast scalping chart.

## The rule it uses
- **Goes Up** after two bars of rising lows (higher low, then higher low again).
- **Goes Down** after two bars of falling highs (lower high, then lower high).
- **Exits** the moment price breaks the running extreme since it entered (a
  built-in trailing stop on structure). Always passes through Neutral when
  flipping.

## What you need
- Just price (High/Low). **No** order flow, footprint, or depth required.
- Works on **any bar type** and any liquid market.

## How to read it
| Output | Looks like | Meaning → Action |
|---|---|---|
| **State line** | Stair-step, gray | `+1` = Up (favor longs), `0` = Neutral (stand aside), `-1` = Down (favor shorts) |
| **Up Context** | Green background | You are in an uptrend regime |
| **Down Context** | Red background | You are in a downtrend regime |

## Key setting
| Setting | Default | Plain meaning | Suggested |
|---|---|---|---|
| Source Chart | `0` (this chart) | Set to another chart's number to borrow its High/Low structure | Leave at `0` to start; point at a Renko/higher-TF chart once comfortable |

## Best use
- **Markets:** any liquid market (ES/NQ for scalping).
- **Bar type / timeframe:** any. For scalping, run it on a slightly slower chart
  (or via *Source Chart*) so the trend doesn't flip on every wiggle.
- **Combine with:** FlowConviction and AbsorptionGradient — take their signals
  **only in the direction of this state**.

## Gotchas
- It recalculates every tick; the current bar's state can change until the bar
  closes. Treat the **closed-bar** state as the confirmed one.
- It's a *structure* filter, not a momentum one — in fast V-reversals it will lag
  by design (it waits for two clean bars).

**Bottom line:** Your trade-direction gatekeeper. Load it first, respect it.
