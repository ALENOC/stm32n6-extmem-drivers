#!/usr/bin/env python3
"""Summarise gcov line and branch coverage for the driver sources.

Usage: coverage_report.py <source.c> [...]
Reads the <file>.c.gcov reports produced by `gcov -b` in the current directory,
prints per-file line / branch coverage with the uncovered line numbers, and
exits with status 1 when any driver line is not executed.
"""

import os
import re
import sys


def parse(gcov_path):
    lines_total = lines_hit = 0
    br_total = br_taken = 0
    missed = []
    current = 0
    with open(gcov_path, encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            m = re.match(r"\s*([^:]+):\s*(\d+):", raw)
            if m:
                count, current = m.group(1).strip(), int(m.group(2))
                if count == "-":
                    continue
                lines_total += 1
                if count.startswith("#####") or count.startswith("====="):
                    missed.append(current)
                else:
                    lines_hit += 1
                continue
            b = re.match(r"branch\s+\d+\s+(taken\s+(\d+)|never executed)", raw)
            if b:
                br_total += 1
                if b.group(2) is not None and int(b.group(2)) > 0:
                    br_taken += 1
    return lines_total, lines_hit, br_total, br_taken, missed


def main(sources):
    ok = True
    sum_l = sum_lh = sum_b = sum_bt = 0
    print("\n%-58s %14s %16s" % ("Driver source", "Lines", "Branches"))
    for src in sources:
        report = os.path.basename(src) + ".gcov"
        if not os.path.exists(report):
            print("%-58s  no coverage data" % src)
            ok = False
            continue
        lt, lh, bt, bh, missed = parse(report)
        sum_l += lt; sum_lh += lh; sum_b += bt; sum_bt += bh
        lp = 100.0 * lh / lt if lt else 100.0
        bp = 100.0 * bh / bt if bt else 100.0
        print("%-58s %6.1f%% %3d/%-3d %6.1f%% %4d/%-4d" % (src, lp, lh, lt, bp, bh, bt))
        if missed:
            ok = False
            print("    uncovered lines: " + ", ".join(str(n) for n in missed))
    lp = 100.0 * sum_lh / sum_l if sum_l else 100.0
    bp = 100.0 * sum_bt / sum_b if sum_b else 100.0
    print("%-58s %6.1f%% %5d/%-5d %5.1f%% %5d/%-5d" % ("TOTAL", lp, sum_lh, sum_l, bp, sum_bt, sum_b))
    if not ok:
        print("\nCoverage check FAILED: every driver line must be executed by the test suite.")
        return 1
    print("\nCoverage check passed: 100% of driver lines executed.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
