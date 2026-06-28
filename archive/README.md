# Archived studies

Superseded versions kept for reference only. These are **not** part of the
active study set and are not maintained. Do not load them alongside their
current replacement — they share study/DLL names and behaviour with the live
version and only add confusion.

| File | Superseded by | Reason |
|---|---|---|
| `OrderflowSignalV2.cpp` | `OrderflowSignalV3.cpp` | V3 is a strict superset: same 50-trigger engine plus rising-edge trigger detection (fixes HTF multi-bar reprint inflating the confluence sum on live data) and the V3.2 post-loop watermark alert fix. V2 also still ships the old chart-specific hardcoded default trigger map. |

To restore an archived study, move it back to the repo root and recompile in
Sierra Chart (F5).
