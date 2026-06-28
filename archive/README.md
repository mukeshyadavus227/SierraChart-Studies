# Archived studies

Superseded versions kept for reference only. These are **not** part of the
active study set and are not maintained. Do not load them alongside their
current replacement — they share study/DLL names and behaviour with the live
version and only add confusion.

| File | Superseded by | Reason |
|---|---|---|
| `OrderflowSignalV2.cpp` | `OrderflowSignalV3.cpp` | V3 is a strict superset: same 50-trigger engine plus rising-edge trigger detection (fixes HTF multi-bar reprint inflating the confluence sum on live data) and the V3.2 post-loop watermark alert fix. V2 also still ships the old chart-specific hardcoded default trigger map. |
| `DOMReader.cpp` | `DOMReaderV2.cpp` | V2 preserves V1's Pull/Stack oscillator byte-for-byte on SG1 (Net Pull/Stack) and layers analytics on top (atoms, conviction butterfly, forward Welford stats, bar-close triggers). Nothing in V1 is lost by using V2. |

> Note: `ReconTape.cpp` (V1) is **not** archived. `ReconTapeV2.cpp` is a distinct
> multi-tier variant, not a superset — V1 keeps Z-score significance mode, Zones
> of Interest rectangles, and reaction measurement that V2 does not have. Both
> are intentionally maintained.

To restore an archived study, move it back to the repo root and recompile in
Sierra Chart (F5).
