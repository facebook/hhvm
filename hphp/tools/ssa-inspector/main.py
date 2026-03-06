# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

"""
CLI entry point for the SSA Graph Inspector.

Subcommands:
  list-runs   Show available tcprint runs in Hive
  top         Show top N hottest translations by profCount
  inspect     Detailed view of specific function/translation
"""

from __future__ import annotations

import argparse
import json
import sys

from hphp.tools.ssa_inspector import data_fetcher
from hphp.tools.ssa_inspector.formatter import format_translation
from hphp.tools.ssa_inspector.ir_parser import parse_json_blob
from hphp.tools.ssa_inspector.xenon_fetcher import fetch_gcpu_for_functions


def cmd_list_runs(args: argparse.Namespace) -> None:
    runs = data_fetcher.list_runs(days=args.days)
    if not runs:
        print("No runs found.")
        return
    print(f"{'Date':<14} {'Run UUID'}")
    print("-" * 60)
    for r in runs:
        print(f"{r.get('ds', '?'):<14} {r.get('run_uuid', '?')}")


def cmd_top(args: argparse.Namespace) -> None:
    rows = data_fetcher.fetch_top_translations(
        top_n=args.top_n,
        ds=args.date,
        run_uuid=args.run_uuid,
        tenant=args.tenant,
        partition=args.partition,
    )
    if not rows:
        print("No translations found.")
        return

    # Fetch xenon gCPU data if requested
    gcpu_map: dict[str, float] = {}
    if args.xenon_hours > 0:
        func_names = list({r.get("func_name", "") for r in rows if r.get("func_name")})
        gcpu_map = fetch_gcpu_for_functions(func_names, args.xenon_hours)

    print(f"Top {len(rows)} translations by profCount:\n")
    for i, r in enumerate(rows, 1):
        pc = r.get("prof_count", 0)
        func_name = r.get("func_name", "?")
        gcpu_str = ""
        if args.xenon_hours > 0:
            gcpu_pct = gcpu_map.get(func_name)
            gcpu_str = f"  gCPU={gcpu_pct:>5.2f}%" if gcpu_pct else "  gCPU=    -"
        print(
            f"  {i:3d}. trans_id={r['trans_id']:<8}"
            f"  profCount={pc:>12,}"
            f"{gcpu_str}"
            f"  kind={r.get('kind', '?'):<18}"
            f"  {func_name}"
        )

    if args.format != "list":
        # Also show formatted output for each
        print("\n" + "=" * 70)
        for r in rows:
            translation = parse_json_blob(r["data"])
            print(
                format_translation(
                    translation, fmt=args.format, show_disasm=args.disasm
                )
            )
            print()


def _extract_func_names(rows: list[dict]) -> list[str]:
    """Extract function names from translation data rows."""
    func_names: list[str] = []
    for r in rows:
        try:
            data = json.loads(r["data"])
            name = data.get("translation", {}).get("funcName", "")
            if name:
                func_names.append(name)
        except (json.JSONDecodeError, KeyError):
            pass
    return func_names


def cmd_inspect(args: argparse.Namespace) -> None:
    if args.local_file:
        rows = data_fetcher.load_from_local_file(
            path=args.local_file,
            function_filter=args.function,
            trans_id_filter=args.trans_id,
        )
    elif args.trans_id is not None:
        result = data_fetcher.fetch_translation_by_id(
            trans_id=args.trans_id,
            ds=args.date,
            run_uuid=args.run_uuid,
            tenant=args.tenant,
            partition=args.partition,
        )
        rows = [result] if result else []
    elif args.function:
        rows = data_fetcher.fetch_translations_by_function(
            function_name=args.function,
            ds=args.date,
            run_uuid=args.run_uuid,
            tenant=args.tenant,
            partition=args.partition,
            kind=args.kind,
        )
    else:
        print(
            "Error: specify --function, --trans-id, or --local-file",
            file=sys.stderr,
        )
        sys.exit(1)

    if not rows:
        print("No translations found.")
        return

    # Extract function names for xenon lookup
    func_names = _extract_func_names(rows)

    # Fetch xenon gCPU data if requested
    gcpu_map: dict[str, float] = {}
    if args.xenon_hours > 0 and func_names:
        gcpu_map = fetch_gcpu_for_functions(list(set(func_names)), args.xenon_hours)

    print(f"Found {len(rows)} translation(s).")

    if args.xenon_hours > 0 and func_names:
        # Show gCPU for the primary function
        gcpu_pct = gcpu_map.get(func_names[0])
        if gcpu_pct is not None:
            print(
                f"Xenon gCPU: {gcpu_pct:.2f}% of total CPU "
                f"(last {args.xenon_hours}h, exclusive)"
            )
        else:
            print(f"Xenon gCPU: not in top 500 functions (last {args.xenon_hours}h)")

    print()
    for r in rows:
        translation = parse_json_blob(r["data"])
        print(format_translation(translation, fmt=args.format, show_disasm=args.disasm))
        print()


def main() -> None:
    parser = argparse.ArgumentParser(
        description="HHVM tcprint SSA Graph Inspector",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    # list-runs
    p_list = subparsers.add_parser(
        "list-runs", help="Show available tcprint runs in Hive"
    )
    p_list.add_argument(
        "--days", type=int, default=7, help="Look back N days (default: 7)"
    )
    p_list.set_defaults(func=cmd_list_runs)

    # top
    p_top = subparsers.add_parser(
        "top", help="Show top N hottest translations by profCount"
    )
    _add_partition_args(p_top)
    p_top.add_argument(
        "--top-n", type=int, default=10, help="Number of translations (default: 10)"
    )
    p_top.add_argument(
        "--format",
        choices=["list", "summary", "detail"],
        default="list",
        help="Output format (default: list)",
    )
    p_top.add_argument(
        "--disasm",
        action="store_true",
        help="Include machine code disassembly",
    )
    p_top.add_argument(
        "--xenon-hours",
        type=int,
        default=4,
        help="Xenon lookback window in hours for gCPU %% (0 to skip, default: 4)",
    )
    p_top.set_defaults(func=cmd_top)

    # inspect
    p_inspect = subparsers.add_parser(
        "inspect", help="Detailed view of function/translation"
    )
    _add_partition_args(p_inspect)
    p_inspect.add_argument("--function", help="Function name (substring match)")
    p_inspect.add_argument("--trans-id", type=int, help="Translation ID")
    p_inspect.add_argument(
        "--local-file", help="Load from local trace file instead of Hive"
    )
    p_inspect.add_argument(
        "--kind",
        help="Filter by TransKind (e.g. TransOptimize)",
    )
    p_inspect.add_argument(
        "--format",
        choices=["summary", "detail"],
        default="detail",
        help="Output format (default: detail)",
    )
    p_inspect.add_argument(
        "--disasm",
        action="store_true",
        help="Include machine code disassembly",
    )
    p_inspect.add_argument(
        "--xenon-hours",
        type=int,
        default=4,
        help="Xenon lookback window in hours for gCPU %% (0 to skip, default: 4)",
    )
    p_inspect.set_defaults(func=cmd_inspect)

    args = parser.parse_args()
    args.func(args)


def _add_partition_args(parser: argparse.ArgumentParser) -> None:
    """Add common partition-filtering arguments to a subparser."""
    parser.add_argument("--tenant", help="Tenant name (e.g. web, instagram)")
    parser.add_argument("--partition", type=int, help="Partition number")
    parser.add_argument("--date", help="Date partition (YYYY-MM-DD)")
    parser.add_argument("--run-uuid", help="Exact run UUID")


if __name__ == "__main__":
    main()
