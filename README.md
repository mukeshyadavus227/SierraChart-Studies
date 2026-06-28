# SierraChart Studies

A collection of Sierra Chart ACSIL (C++) custom studies for ES futures scalping, built around order-flow confluence gated by trend-state filters.

## Studies

| File | Description |
|---|---|
| `OrderflowConfluence.cpp` | 25 bull + 25 bear order-flow triggers with shared or independent scoring modes |
| `OrderflowSignalV3.cpp` | **Canonical** single-direction engine: 50 weighted triggers, 3 intensity levels (`AutoLoop = 0`), rising-edge trigger detection, post-loop watermark alerts. Supersedes V2 (archived). |
| `OTFStateFilter.cpp` | Trend state machine (Higher-Lows / Lower-Highs) producing a +1 / 0 / −1 state subgraph |
| `MTFCloseFilter.cpp` | Multi-timeframe close-breakout state across six charts with a colored status bar |
| `MTFCloseFilter.pine` | TradingView Pine v5 port of MTFCloseFilter |
| `BreadthCompositeOscillator.cpp` | NYSE $VOLD + $ADD composite oscillator with dual Z-score normalization — macro bias filter |
| `TapeReader.cpp` | Tape reading study |
| `ReconTape.cpp` | Reconstructed tape: bubbles + Zones-of-Interest rectangles, Z-score or percentile significance |
| `ReconTapeV2.cpp` | Multi-tier reconstructed tape (distinct variant — 3 tiers, session-scoped baseline; not a superset of V1) |
| `BigTradesTape.cpp` | Highlights large trades on the tape |
| `DOMReaderV2.cpp` | **Canonical** DOM pull/stack reader: V1 oscillator preserved byte-for-byte on SG1, plus atoms, conviction butterfly, and bar-close triggers. Supersedes V1 (archived). |
| `DeltaVelocityProfile.cpp` | Delta velocity profiling |
| `DeltaReversalTrigger.cpp` | Delta reversal trigger detection |
| `AbsorptionGradient.cpp` | Absorption gradient visualization |
| `FlowConviction.cpp` | Order-flow conviction scoring |
| `InterestMap.cpp` | Open interest mapping |
| `LiquidityZones.cpp` | Liquidity zone detection |
| `TrappedTraders.cpp` | Trapped trader detection |
| `AVWAPRotation.cpp` | Anchored VWAP rotation study |
| `OFCommon.h` | Shared helpers for the common ACSIL patterns (intrabar guard, settings fingerprint, watermark alerts). Opt-in; adopt per study. |

Superseded versions live in [`archive/`](archive/) and are not maintained.
