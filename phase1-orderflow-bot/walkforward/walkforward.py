#!/usr/bin/env python3
"""
walkforward.py — honest out-of-sample evaluation for Sierra Chart strategy runs.

WHY THIS EXISTS
    The Sierra Chart Strategy Optimizer does an IN-SAMPLE grid search only. It
    ranks every parameter combination by total P/L on ONE window and reports the
    single best. That single best is, almost by construction, the most overfit
    combination — picking it is the canonical selection-bias trap.

    This tool turns that grid search into a real WALK-FORWARD test:
      * You run the optimizer separately on several consecutive time windows
        ("folds"), each producing a summary CSV.
      * For each fold we choose ROBUST parameters on the TRAIN fold using
        PLATEAU selection (a parameter region that is good on average with its
        neighbours), NOT the single peak.
      * We then look those exact parameters up in the NEXT fold (the untouched
        TEST window) and record their OUT-OF-SAMPLE result.
      * The walk-forward verdict is the aggregate of those OOS results.

    Two modes:
      walkforward   — fold CSVs in, plateau-select on train, score on next (OOS)
      tradelist     — one SC-exported trade list in, expectancy/PF/maxDD + bootstrap CI

    Nothing here trades or touches your account. It only reads CSVs you produced.

USAGE
    python walkforward.py walkforward \
        --windows f1.csv f2.csv f3.csv f4.csv \
        --param-cols k emaLen targetR stopTicks \
        --metric TotalPnL --trades-col Trades \
        --min-trades 30 --top-frac 0.20

    python walkforward.py tradelist --file trades.csv \
        --pnl-col "Profit/Loss" --min-trades 30

GO/NO-GO RULE (defaults; tune in README): a fold's OOS result "passes" if it is
    net-positive after costs AND meets --min-trades. Overall GO requires
    >= --pass-frac of folds passing AND a positive summed OOS metric AND average
    OOS/IS degradation no worse than --max-degrade. Anything else = NO-GO = stop.
"""

import argparse
import csv
import math
import sys
from statistics import mean, median


# --------------------------------------------------------------------------- #
# CSV loading (schema-agnostic: we only need the param columns + a metric col) #
# --------------------------------------------------------------------------- #
def load_rows(path):
    with open(path, newline="") as fh:
        rows = list(csv.DictReader(fh))
    if not rows:
        raise ValueError(f"{path}: no data rows")
    return rows


def to_float(x):
    try:
        return float(str(x).replace(",", "").replace("$", "").strip())
    except (ValueError, AttributeError):
        return None


def param_key(row, param_cols):
    """A hashable, normalized identity for a parameter combination."""
    parts = []
    for c in param_cols:
        v = to_float(row.get(c))
        parts.append(round(v, 10) if v is not None else str(row.get(c)))
    return tuple(parts)


def grid_steps(rows, param_cols):
    """Infer the optimizer's grid step per parameter (smallest gap between
    distinct sorted values). Used to define a parameter's 'neighbourhood'."""
    steps = {}
    for c in param_cols:
        vals = sorted({to_float(r.get(c)) for r in rows if to_float(r.get(c)) is not None})
        gaps = [b - a for a, b in zip(vals, vals[1:]) if b - a > 1e-12]
        steps[c] = min(gaps) if gaps else 0.0
    return steps


# --------------------------------------------------------------------------- #
# Plateau selection: reward stable regions, not lucky spikes                   #
# --------------------------------------------------------------------------- #
def neighbourhood_score(target, rows, param_cols, metric, steps, radius=1):
    """Average metric over all rows within `radius` grid steps of `target` on
    every parameter axis (an axis with step 0 is treated as 'must match')."""
    vals = []
    for r in rows:
        ok = True
        for i, c in enumerate(param_cols):
            tv = target[i]
            rv = to_float(r.get(c))
            step = steps.get(c, 0.0)
            if not isinstance(tv, float) or rv is None:
                if str(r.get(c)) != str(tv):
                    ok = False
                    break
                continue
            tol = radius * step if step > 0 else 1e-9
            if abs(rv - tv) > tol + 1e-12:
                ok = False
                break
        if ok:
            m = to_float(r.get(metric))
            if m is not None:
                vals.append(m)
    return (mean(vals), len(vals)) if vals else (float("-inf"), 0)


def select_plateau(rows, param_cols, metric, trades_col, min_trades, top_frac, radius):
    """Pick robust params on a TRAIN fold:
       1. keep rows meeting min-trades,
       2. restrict to the top `top_frac` by raw metric (candidate good region),
       3. among candidates, choose the one whose NEIGHBOURHOOD mean metric is
          highest (i.e. it sits inside a broad good plateau, not on a spike)."""
    elig = [r for r in rows
            if trades_col is None or (to_float(r.get(trades_col)) or 0) >= min_trades]
    if not elig:
        return None
    elig.sort(key=lambda r: to_float(r.get(metric)) or float("-inf"), reverse=True)
    n_top = max(1, int(round(len(elig) * top_frac)))
    candidates = elig[:n_top]
    steps = grid_steps(rows, param_cols)

    best = None
    for r in candidates:
        key = param_key(r, param_cols)
        nb_mean, nb_n = neighbourhood_score(key, rows, param_cols, metric, steps, radius)
        raw = to_float(r.get(metric))
        # rank by neighbourhood mean, tie-break by support size then raw metric
        score = (nb_mean, nb_n, raw)
        if best is None or score > best[0]:
            best = (score, key, r, nb_mean, nb_n)
    _, key, row, nb_mean, nb_n = best
    return {"key": key, "row": row, "is_metric": to_float(row.get(metric)),
            "plateau_mean": nb_mean, "plateau_support": nb_n}


def lookup(rows, param_cols, key, metric, trades_col):
    for r in rows:
        if param_key(r, param_cols) == key:
            return {"metric": to_float(r.get(metric)),
                    "trades": to_float(r.get(trades_col)) if trades_col else None}
    return None


# --------------------------------------------------------------------------- #
# Mode 1: walk-forward across fold CSVs                                        #
# --------------------------------------------------------------------------- #
def run_walkforward(a):
    folds = [(p, load_rows(p)) for p in a.windows]
    if len(folds) < 2:
        sys.exit("Need at least 2 window CSVs (train -> next-as-test).")

    print(f"\nWalk-forward over {len(folds)} folds "
          f"({len(folds) - 1} train->test pairs)")
    print(f"params={a.param_cols}  metric={a.metric}  "
          f"min_trades={a.min_trades}  top_frac={a.top_frac}\n")

    results = []
    for i in range(len(folds) - 1):
        train_path, train = folds[i]
        test_path, test = folds[i + 1]
        sel = select_plateau(train, a.param_cols, a.metric, a.trades_col,
                             a.min_trades, a.top_frac, a.radius)
        if sel is None:
            print(f"  fold {i+1}: no train rows meet min-trades — SKIP")
            continue
        oos = lookup(test, a.param_cols, sel["key"], a.metric, a.trades_col)
        params = dict(zip(a.param_cols, sel["key"]))
        if oos is None or oos["metric"] is None:
            print(f"  fold {i+1}: selected params {params} not found in test fold — SKIP")
            continue
        is_m = sel["is_metric"]
        oos_m = oos["metric"]
        oos_tr = oos["trades"]
        degrade = (oos_m / is_m) if (is_m not in (None, 0)) else float("nan")
        passed = (oos_m > 0) and (oos_tr is None or oos_tr >= a.min_trades)
        results.append({"fold": i + 1, "params": params, "is": is_m, "oos": oos_m,
                        "oos_trades": oos_tr, "degrade": degrade, "pass": passed,
                        "plateau_support": sel["plateau_support"]})
        print(f"  fold {i+1}: {train_path} -> {test_path}")
        print(f"      params       : {params}")
        print(f"      train metric : {is_m:.2f}   (plateau support {sel['plateau_support']} combos)")
        print(f"      OOS metric   : {oos_m:.2f}   trades={oos_tr}   "
              f"OOS/IS={degrade:.2f}   {'PASS' if passed else 'fail'}")

    if not results:
        sys.exit("\nNo evaluable folds. Check --param-cols / --metric names match the CSV headers.")

    n = len(results)
    n_pass = sum(r["pass"] for r in results)
    oos_sum = sum(r["oos"] for r in results)
    degr = [r["degrade"] for r in results if not math.isnan(r["degrade"])]
    avg_degrade = mean(degr) if degr else float("nan")
    pass_frac = n_pass / n

    print("\n" + "=" * 60)
    print("WALK-FORWARD SUMMARY")
    print(f"  evaluable folds        : {n}")
    print(f"  OOS-positive folds     : {n_pass}/{n}  ({pass_frac:.0%})")
    print(f"  summed OOS metric      : {oos_sum:.2f}")
    print(f"  median OOS metric      : {median(r['oos'] for r in results):.2f}")
    print(f"  avg OOS/IS degradation : {avg_degrade:.2f}  (1.0 = no decay, <0 = flips sign)")

    go = (pass_frac >= a.pass_frac) and (oos_sum > 0) and \
         (math.isnan(avg_degrade) or avg_degrade >= a.max_degrade)
    print("\n  VERDICT: " + ("GO  — edge survives out-of-sample; proceed to Phase 3 (forward sim)."
                             if go else
                             "NO-GO — does not survive out-of-sample. STOP here (this is the system working)."))
    print(f"  (rule: pass_frac>={a.pass_frac:.0%}, summed OOS>0, avg OOS/IS>={a.max_degrade})")
    print("=" * 60 + "\n")
    return 0 if go else 1


# --------------------------------------------------------------------------- #
# Mode 2: single trade-list statistics + bootstrap CI                         #
# --------------------------------------------------------------------------- #
def bootstrap_ci(xs, iters=5000, lo=2.5, hi=97.5, seed=12345):
    """Deterministic LCG bootstrap (no numpy dependency) for the mean P/L CI."""
    n = len(xs)
    if n == 0:
        return (float("nan"), float("nan"))
    state = seed & 0xFFFFFFFF
    means = []
    for _ in range(iters):
        s = 0.0
        for _ in range(n):
            state = (1103515245 * state + 12345) & 0x7FFFFFFF
            s += xs[state % n]
        means.append(s / n)
    means.sort()
    return (means[int(lo / 100 * iters)], means[int(hi / 100 * iters)])


def run_tradelist(a):
    rows = load_rows(a.file)
    pnls = [to_float(r.get(a.pnl_col)) for r in rows]
    pnls = [p for p in pnls if p is not None]
    n = len(pnls)
    if n < a.min_trades:
        print(f"\nOnly {n} trades (< --min-trades {a.min_trades}): sample too small "
              f"to judge. Gather more before trusting any statistic.\n")
    if n == 0:
        sys.exit(f"No numeric values in column '{a.pnl_col}'.")

    wins = [p for p in pnls if p > 0]
    losses = [p for p in pnls if p < 0]
    gross_win = sum(wins)
    gross_loss = -sum(losses)
    total = sum(pnls)
    expectancy = total / n
    win_rate = len(wins) / n
    pf = (gross_win / gross_loss) if gross_loss > 0 else float("inf")

    # equity curve max drawdown
    eq = 0.0
    peak = 0.0
    maxdd = 0.0
    for p in pnls:
        eq += p
        peak = max(peak, eq)
        maxdd = max(maxdd, peak - eq)

    ci = bootstrap_ci(pnls)
    print(f"\nTrade-list statistics  ({a.file})")
    print(f"  trades            : {n}")
    print(f"  net P/L           : {total:.2f}")
    print(f"  expectancy/trade  : {expectancy:.2f}   95% CI [{ci[0]:.2f}, {ci[1]:.2f}]")
    print(f"  win rate          : {win_rate:.1%}  ({len(wins)}W / {len(losses)}L)")
    print(f"  profit factor     : {pf:.2f}")
    print(f"  max drawdown      : {maxdd:.2f}")

    edge = ci[0] > 0
    print("\n  READ: " + ("expectancy CI is entirely ABOVE zero — a real positive edge in THIS sample."
                          if edge else
                          "expectancy CI includes zero — NOT distinguishable from no edge. Treat as no-go."))
    print("  (CI is in-sample to this trade list; only an OUT-OF-SAMPLE window's trades count as validation.)\n")
    return 0 if edge else 1


# --------------------------------------------------------------------------- #
def build_parser():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="mode", required=True)

    wf = sub.add_parser("walkforward", help="OOS evaluation across fold CSVs")
    wf.add_argument("--windows", nargs="+", required=True,
                    help="optimizer summary CSVs, in chronological order (>=2)")
    wf.add_argument("--param-cols", nargs="+", required=True,
                    help="CSV columns identifying a parameter combination")
    wf.add_argument("--metric", default="TotalPnL", help="performance column (default TotalPnL)")
    wf.add_argument("--trades-col", default=None, help="trade-count column (for --min-trades)")
    wf.add_argument("--min-trades", type=int, default=30)
    wf.add_argument("--top-frac", type=float, default=0.20,
                    help="fraction of best train rows considered candidates (default 0.20)")
    wf.add_argument("--radius", type=int, default=1, help="plateau neighbourhood radius in grid steps")
    wf.add_argument("--pass-frac", type=float, default=0.60, help="GO needs this fraction of folds OOS-positive")
    wf.add_argument("--max-degrade", type=float, default=0.0,
                    help="GO needs avg OOS/IS ratio >= this (0.0 = OOS just needs to stay positive)")
    wf.set_defaults(func=run_walkforward)

    tl = sub.add_parser("tradelist", help="expectancy/PF/maxDD + bootstrap CI for one trade list")
    tl.add_argument("--file", required=True, help="SC-exported trade list CSV")
    tl.add_argument("--pnl-col", default="Profit/Loss", help="per-trade P/L column")
    tl.add_argument("--min-trades", type=int, default=30)
    tl.set_defaults(func=run_tradelist)
    return p


def main(argv=None):
    args = build_parser().parse_args(argv)
    return args.func(args)


if __name__ == "__main__":
    sys.exit(main())
