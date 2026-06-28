// =============================================================================
// OFCommon.h  —  Shared helpers for the SierraChart-Studies ACSIL suite
//
// Centralises the four boilerplate patterns that were previously copy-pasted
// across every study (see CLAUDE.md "ACSIL Core Patterns"):
//   1. Full-recalculation detection
//   2. Intrabar guard / new-bar gating
//   3. Settings fingerprint (structural input change detection)
//   4. Post-loop watermark alerting (HTF backfill-safe)
//
// USAGE:
//   #include "sierrachart.h"
//   #include "OFCommon.h"
//   ...
//   if (!OF::ShouldProcessBarClose(sc)) return;   // intrabar guard
//
// ADOPTION NOTE:
//   This header is NOT yet wired into the existing studies. It is provided so
//   new studies and incremental refactors can share one tested implementation
//   instead of re-deriving these patterns. Adopt it ONE study at a time and
//   recompile that study in Sierra Chart (F5) to confirm — the helpers mirror
//   the patterns already proven in OrderflowSignalV3 / LiquidityZones /
//   TrappedTraders, but ACSIL only compiles inside Sierra Chart.
//
// SLOT CONVENTIONS (must stay consistent with CLAUDE.md):
//   1     lastKnownBars            new-bar detection
//   2     alert watermark          newest alerted signal bar index
//   3     alert watermark          newest alerted confluence bar index
//   4-6   settings fingerprint     structural input change detection
//
// -----------------------------------------------------------------------------
// WORKED EXAMPLE — adopting the intrabar guard in OrderflowSignalV3.cpp
//
//   BEFORE (hand-rolled, ~7 lines):
//     int& lastKnownBars = sc.GetPersistentInt(1);
//     const bool isFullRecalc = (sc.UpdateStartIndex == 0);
//     const bool isNewBar     = (totalBars > lastKnownBars);
//     if (!isFullRecalc && !isNewBar)
//         return;
//     lastKnownBars = totalBars;
//
//   AFTER:
//     const bool isFullRecalc = OF::IsFullRecalc(sc);  // still used by the alert block
//     if (!OF::ShouldProcessBarClose(sc))
//         return;
//
// WORKED EXAMPLE — adopting the watermark alert (signal alert in V3.cpp)
//
//   The signal message embeds a conviction level, so use the primitive and
//   format the message yourself; the convenience wrapper ScanAndAlert covers
//   the simpler confluence-onset case.
//
//     const int sigBar = OF::NewestNewSignalBar(sc, OF::SLOT_ALERT_SIGNAL,
//         [&](int b){ return sg_Level1[b]!=0.0f || sg_Level2[b]!=0.0f || sg_Level3[b]!=0.0f; });
//     if (sigSound > 0 && sigBar >= 0) {
//         const int level = (sg_Level3[sigBar]!=0.0f) ? 3 : (sg_Level2[sigBar]!=0.0f ? 2 : 1);
//         SCString msg; msg.Format("Orderflow Signal L%d (%d bar(s) back)", level, (sc.ArraySize-1) - sigBar);
//         sc.SetAlert(sigSound, sc.ArraySize-1, msg);
//     }
//
//     OF::ScanAndAlert(sc, OF::SLOT_ALERT_CONFLUENCE, confSound,
//         [&](int b){ return sg_Confluence[b] > 0.5f && (b==0 || sg_Confluence[b-1] < 0.5f); },
//         "Orderflow Confluence Zone");
// =============================================================================
#pragma once

#include "sierrachart.h"

namespace OF
{
    // Persistent-int slot conventions shared across the suite.
    enum PersistSlot
    {
        SLOT_LAST_KNOWN_BARS  = 1,
        SLOT_ALERT_SIGNAL     = 2,
        SLOT_ALERT_CONFLUENCE = 3,
        SLOT_FINGERPRINT_0    = 4,
        SLOT_FINGERPRINT_1    = 5,
        SLOT_FINGERPRINT_2    = 6,
    };

    // -------------------------------------------------------------------------
    // 1. Full-recalculation detection.
    //    True when SC requests a full pass (first add, Recalculate, or a study
    //    reset queued via sc.ResetStudyToCalculateFromBarOne).
    // -------------------------------------------------------------------------
    inline bool IsFullRecalc(SCStudyInterfaceRef sc)
    {
        return sc.UpdateStartIndex == 0 || sc.IsFullRecalculation;
    }

    // -------------------------------------------------------------------------
    // 2. Intrabar guard.
    //    Returns true only on a bar close (ArraySize grew) or a full recalc,
    //    and advances the lastKnownBars watermark. Returns false on intrabar
    //    ticks so the caller can early-return. Standard pattern for studies
    //    that should compute once per finished bar.
    // -------------------------------------------------------------------------
    inline bool ShouldProcessBarClose(SCStudyInterfaceRef sc,
                                      int slot = SLOT_LAST_KNOWN_BARS)
    {
        int& lastKnownBars = sc.GetPersistentInt(slot);
        const bool fullRecalc = (sc.UpdateStartIndex == 0);
        const bool newBar     = (sc.ArraySize > lastKnownBars);

        if (!fullRecalc && !newBar)
            return false;

        lastKnownBars = sc.ArraySize;
        return true;
    }

    // -------------------------------------------------------------------------
    // 3. Settings fingerprint.
    //    Returns true if `fingerprint` differs from the value stored in `slot`
    //    since the last call, and stores the new value. Use one slot per
    //    fingerprint word (4, 5, 6...). Feed it only STRUCTURAL inputs
    //    (lookbacks, thresholds, mode selectors); never display-only inputs
    //    (colors, transparency, widths).
    //
    //    Typical use: force a full recalc when structural settings change.
    //        if (OF::SettingsChanged(sc, fp)) needFullRecalc = true;
    // -------------------------------------------------------------------------
    inline bool SettingsChanged(SCStudyInterfaceRef sc, int fingerprint,
                                int slot = SLOT_FINGERPRINT_0)
    {
        int& stored = sc.GetPersistentInt(slot);
        const bool changed = (stored != fingerprint);
        stored = fingerprint;
        return changed;
    }

    // -------------------------------------------------------------------------
    // 4a. Watermark scan primitive (HTF backfill-safe alerting).
    //
    //    HTF triggers backfill onto historical bars, so the rising edge — and
    //    therefore the signal — can land 2-3 bars behind the live bar.
    //    sc.SetAlert() with that stale index is suppressed during real-time
    //    updates. This primitive finds the newest signalled bar that is NEWER
    //    than the stored watermark (within the last `scanBars`), advances the
    //    watermark to it, and returns its index. The CALLER then formats a
    //    message and raises ONE alert anchored to the current last bar.
    //
    //    On full recalc / first run the watermark is fast-forwarded to the last
    //    bar and -1 is returned (no alert) so chart loads never alert-storm.
    //
    //    isSignalAtBar: callable bool(int barIndex).
    //    Returns the signalled bar index, or -1 if none.
    // -------------------------------------------------------------------------
    template <typename SignalFn>
    inline int NewestNewSignalBar(SCStudyInterfaceRef sc, int watermarkSlot,
                                  SignalFn isSignalAtBar, int scanBars = 10)
    {
        const int lastBar = sc.ArraySize - 1;
        if (lastBar < 0)
            return -1;

        int& watermark = sc.GetPersistentInt(watermarkSlot);

        if (IsFullRecalc(sc) || watermark == 0)
        {
            watermark = lastBar;   // never alert on historical data / chart load
            return -1;
        }

        const int scanFloor = (lastBar - scanBars + 1 > 0) ? (lastBar - scanBars + 1) : 0;
        const int start     = (watermark + 1 > scanFloor) ? (watermark + 1) : scanFloor;

        for (int b = lastBar; b >= start; --b)
        {
            if (isSignalAtBar(b))
            {
                watermark = b;
                return b;
            }
        }
        return -1;
    }

    // -------------------------------------------------------------------------
    // 4b. Convenience wrapper: scan + raise a standard alert.
    //    Builds "<label> (<N> bar(s) back)" and raises one alert anchored to
    //    the current last bar. No-op when soundID <= 0. For richer messages
    //    (e.g. including a conviction level) call NewestNewSignalBar directly
    //    and format the message yourself.
    // -------------------------------------------------------------------------
    template <typename SignalFn>
    inline void ScanAndAlert(SCStudyInterfaceRef sc, int watermarkSlot,
                             int soundID, SignalFn isSignalAtBar,
                             const char* label, int scanBars = 10)
    {
        if (soundID <= 0)
        {
            // Keep the watermark current so enabling the alert later does not
            // replay historical signals.
            int& watermark = sc.GetPersistentInt(watermarkSlot);
            watermark = sc.ArraySize - 1;
            return;
        }

        const int bar = NewestNewSignalBar(sc, watermarkSlot, isSignalAtBar, scanBars);
        if (bar < 0)
            return;

        const int lastBar = sc.ArraySize - 1;
        SCString msg;
        msg.Format("%s (%d bar(s) back)", label, lastBar - bar);
        sc.SetAlert(soundID, lastBar, msg);
    }

} // namespace OF
