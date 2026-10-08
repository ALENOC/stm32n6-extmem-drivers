#!/usr/bin/env python3
"""Summarise the cppcheck MISRA C:2012 addon results for the driver sources.

Usage: misra_report.py <results.jsonl>

The input holds one JSON object per line, as written by `misra.py --cli`.
Findings located in the host test harness (Tests/) are ignored and duplicates
coming from headers included by several sources are counted once. The report
lists the number of findings per rule; it does not fail the build: the
remaining findings are documented deviations (Docs/Static_Analysis.md).
"""
import collections
import json
import sys


def main(path):
    seen = set()
    per_rule = collections.Counter()
    with open(path) as results:
        for line in results:
            line = line.strip()
            if not line.startswith("{"):
                continue
            item = json.loads(line)
            if "Tests/" in item["file"]:
                continue
            key = (item["file"], item["linenr"], item["column"], item["errorId"])
            if key in seen:
                continue
            seen.add(key)
            per_rule[item["errorId"].replace("c2012-", "Rule ")] += 1
    print("MISRA C:2012 findings in the driver sources (cppcheck addon): %d" % sum(per_rule.values()))
    for rule, count in sorted(per_rule.items(), key=lambda kv: (-kv[1], kv[0])):
        print("  %-12s %4d" % (rule, count))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1]))
