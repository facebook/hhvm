#!/usr/bin/env python3
# pyre-strict
"""Test driver for the fanout service.

The fanout service answers the question "given these changed symbols, which
files need to be re-typechecked"?

The test driver verifies that the list of files to re-typecheck is the expected
list.

TODO: Additionally, we perform the sanity check that the list of typechecking
errors produced from a full typecheck is the same as for an incremental
typecheck.
"""
import argparse
import glob
import json
import os
import os.path
import pprint
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from typing import Dict, List, Optional, cast


Path = str


def log(message: str) -> None:
    sys.stderr.write(message)
    sys.stderr.write("\n")


def copy(source: Path, dest: Path) -> None:
    log(f"Copying {source} to {dest}")
    shutil.copy(source, dest)


def exec(args: List[str]) -> str:
    log(f"Running: {' '.join(args)}")
    result = subprocess.check_output(args)
    return result.decode()


@dataclass
class Env:
    root_dir: Path
    hh_fanout_path: Path
    hh_server_path: Path


@dataclass
class SavedStateInfo:
    dep_table_path: Path
    naming_table_path: Path


def generate_saved_state(env: Env, target_dir: Path) -> SavedStateInfo:
    saved_state_path = os.path.join(target_dir, "saved_state")
    subprocess.check_call(
        [env.hh_server_path, env.root_dir, "--save-state", saved_state_path],
        # Don't include the "No errors!" output in the test output.
        stdout=sys.stderr,
    )
    return SavedStateInfo(
        dep_table_path=saved_state_path + ".sql", naming_table_path=saved_state_path
    )


def relativize_path(env: Env, path: str) -> str:
    """Convert an absolute path into a relative path for consistent test output."""
    return os.path.relpath(path, env.root_dir)


def run_hh_fanout(
    env: Env,
    saved_state_info: SavedStateInfo,
    changed_files: List[Path],
    args: List[str],
    cursor: Optional[str],
) -> Dict[str, object]:
    common_args = []
    common_args.extend(("--from", "integration-test"))
    common_args.extend(("--root", env.root_dir))
    common_args.extend(("--verbosity", "high"))
    common_args.extend(("--naming-table-path", saved_state_info.naming_table_path))
    common_args.extend(("--dep-table-path", saved_state_info.dep_table_path))
    common_args.extend(("--state-path", os.path.join(env.root_dir, "hh_fanout_state")))
    for changed_file in changed_files:
        common_args.extend(("--changed-file", changed_file))

    hh_fanout_args = list(common_args)
    if cursor is not None:
        hh_fanout_args.extend(["--cursor", cursor])
    result = exec([env.hh_fanout_path, "calculate", *hh_fanout_args, *args])
    result = json.loads(result)

    # Also include the debug output for when the test cases fail and need to be
    # debugged.
    debug_result = exec([env.hh_fanout_path, "debug", *common_args, *args])
    debug_result = json.loads(debug_result)
    result["debug"] = debug_result["debug"]

    return result


def sanitize_hh_fanout_result(env: Env, result: Dict[str, object]) -> None:
    result["files"] = [
        relativize_path(env, path) for path in cast(List[str], result["files"])
    ]

    debug_result = cast(Dict[str, object], result["debug"])
    relevant_dep_edges = cast(List[Dict[str, str]], debug_result["relevant_dep_edges"])
    for relevant_dep_edge in relevant_dep_edges:
        relevant_dep_edge["dependent_path"] = relativize_path(
            env, relevant_dep_edge["dependent_path"]
        )

    cursor = cast(str, result["cursor"])
    delimiter = ","
    assert delimiter in cursor
    parts = cursor.split(delimiter)
    parts[-1] = "<hash-redacted-for-test>"
    result["cursor"] = delimiter.join(parts)

    if "telemetry" in result:
        result["telemetry"] = "<telemetry-redacted-for-test>"


def run_test(
    env: Env,
    saved_state_info: SavedStateInfo,
    test_path: Path,
    changed_files: List[Path],
) -> None:
    result = run_hh_fanout(
        env=env,
        saved_state_info=saved_state_info,
        changed_files=changed_files,
        args=[test_path],
        cursor=None,
    )
    sanitize_hh_fanout_result(env, result)
    pprint.pprint(result)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("test_path", type=os.path.abspath)
    parser.add_argument("--hh-fanout", type=os.path.abspath)
    parser.add_argument("--hh-server", type=os.path.abspath)

    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as work_dir:
        test_path = os.path.join(work_dir, os.path.relpath(args.test_path, os.getcwd()))
        with open(os.path.join(work_dir, ".hhconfig"), "w") as f:
            f.write("")
        for source_file in glob.glob(
            os.path.join(os.path.dirname(args.test_path), "*.php")
        ):
            copy(source_file, work_dir)

        env = Env(
            root_dir=work_dir,
            hh_fanout_path=args.hh_fanout,
            hh_server_path=args.hh_server,
        )
        saved_state_info = generate_saved_state(env, work_dir)

        i = 0
        while True:
            i += 1
            new_files = glob.glob(
                os.path.join(os.path.dirname(args.test_path), f"*.new{i}")
            )
            if not new_files and i > 1:
                break

            changed_files = []
            for source_file in new_files:
                source_file = os.path.relpath(source_file, os.getcwd())
                (target_filename, _) = os.path.splitext(source_file)
                target_path = os.path.join(work_dir, target_filename)
                changed_files.append(target_path)
                copy(source_file, target_path)

            print(f"Change set #{i}")
            run_test(
                env=env,
                saved_state_info=saved_state_info,
                test_path=test_path,
                changed_files=changed_files,
            )


if __name__ == "__main__":
    main()
