#!/usr/bin/env python3
# pyre-strict
"""Test driver for the fanout service.

The fanout service answers the question "given these changed symbols, which
files need to be re-typechecked"?

The test cases are in files called `test.txt` in the subdirectories of this
directory. They are written in a small DSL (detailed later in this file).

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


DEBUGGING = False

Path = str
Cursor = str


def log(message: str) -> None:
    sys.stderr.write(message)
    sys.stderr.write("\n")


def copy(source: Path, dest: Path) -> None:
    log(f"Copying {source} to {dest}")
    shutil.copy(source, dest)


def exec(args: List[str], *, raise_on_error: bool = True) -> str:
    command_line = " ".join(args)
    log(f"Running: {command_line}")
    result = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout = result.stdout
    if raise_on_error and result.returncode != 0:
        # stderr is pretty noisy ordinarily, and causes the logs to be
        # truncated with its length, so only surface it in cases of error.
        stderr = result.stderr.decode()
        raise RuntimeError(
            f"Command {command_line} failed with return code {result.returncode}.\n"
            + f"Stderr: {stderr}\n"
        )
    return stdout.decode()


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
    saved_state_path = os.path.join(target_dir, "dep_table")
    naming_table_path = os.path.join(target_dir, "naming_table.sqlite")
    exec(
        [
            env.hh_server_path,
            env.root_dir,
            "--save-state",
            saved_state_path,
            "--gen-saved-ignore-type-errors",
        ]
    )
    exec(
        [
            env.hh_server_path,
            "--check",
            env.root_dir,
            "--save-naming",
            naming_table_path,
        ]
    )
    return SavedStateInfo(
        dep_table_path=saved_state_path + ".sql", naming_table_path=naming_table_path
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

    cursor = cast(Cursor, result["cursor"])
    delimiter = ","
    assert delimiter in cursor
    parts = cursor.split(delimiter)
    parts[-1] = "<hash-redacted-for-test>"
    result["cursor"] = delimiter.join(parts)

    if "telemetry" in result:
        result["telemetry"] = "<telemetry-redacted-for-test>"


def run_fanout_test(
    env: Env,
    saved_state_info: SavedStateInfo,
    args: List[Path],
    changed_files: List[Path],
    cursor: Optional[Cursor],
) -> Cursor:
    result = run_hh_fanout(
        env=env,
        saved_state_info=saved_state_info,
        changed_files=changed_files,
        args=args,
        cursor=cursor,
    )
    cursor = cast(Cursor, result["cursor"])
    sanitize_hh_fanout_result(env, result)
    pprint.pprint(result)
    return cursor


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("test_path", type=os.path.abspath)
    parser.add_argument("--hh-fanout", type=os.path.abspath)
    parser.add_argument("--hh-server", type=os.path.abspath)

    args = parser.parse_args()
    source_dir = os.path.dirname(args.test_path)
    with tempfile.TemporaryDirectory() as work_dir:
        with open(os.path.join(work_dir, ".hhconfig"), "w") as f:
            f.write("")
        for source_file in glob.glob(os.path.join(source_dir, "*.php")):
            copy(source_file, work_dir)

        env = Env(
            root_dir=work_dir,
            hh_fanout_path=args.hh_fanout,
            hh_server_path=args.hh_server,
        )
        saved_state_info = generate_saved_state(env, work_dir)

        with open(args.test_path) as f:
            test_commands = f.readlines()

        # The test proceeds through a series of timestamps. The first timestamp
        # is 0, which corresponds to the initial state of the repository,
        # copied from the test source directory. A saved-state is generated
        # using that set of files, and fed into `hh_fanout` commands.
        #
        # When we move to the next timestamp `N`, we copy any files matching
        # the pattern `*.php.newN` into the working directory (simulating a
        # file change on disk).
        timestamp = 0
        is_first = True
        changed_files: List[Path] = []
        cursor: Optional[Cursor] = None
        for test_command in test_commands:
            test_command = test_command.strip()
            if test_command.startswith("#") or test_command == "":
                continue
            [command, *command_args] = test_command.split()

            # `advance-time` advances the timestamp to the next value, and
            # copies any files for that timestamp into the test working
            # directory.
            if command == "advance-time":
                timestamp += 1
                new_files = glob.glob(os.path.join(source_dir, f"*.new{timestamp}"))
                changed_files = []
                for source_file in new_files:
                    source_file = os.path.relpath(source_file, os.getcwd())
                    (target_filename, _) = os.path.splitext(source_file)
                    target_path = os.path.join(work_dir, target_filename)
                    changed_files.append(target_path)
                    copy(source_file, target_path)

            # `calculate-fanout` calculates the fanout for the arguments and
            # prints it to the test output for comparison against the `.exp`
            # file.
            elif command == "calculate-fanout":
                if is_first:
                    is_first = False
                else:
                    print()
                print(f"Fanout calculation at time #{timestamp}")

                cursor = run_fanout_test(
                    env=env,
                    saved_state_info=saved_state_info,
                    args=command_args,
                    changed_files=changed_files,
                    cursor=cursor,
                )
                changed_files = []

            else:
                raise ValueError(f"Unrecognized test command: {command}")

        if DEBUGGING:
            debug_dir = "/tmp/hh_fanout_debug"
            shutil.rmtree(debug_dir, ignore_errors=True)
            shutil.copytree(work_dir, debug_dir)
            sys.stderr.write(
                f"DEBUGGING is enabled. "
                + f"You can examine the saved states at: {debug_dir}\n"
            )


if __name__ == "__main__":
    main()
