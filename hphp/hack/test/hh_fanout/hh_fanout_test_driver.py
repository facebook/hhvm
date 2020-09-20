#!/usr/bin/env python3
# pyre-strict
"""Test driver for the fanout service.

The fanout service answers the question "given these changed symbols, which
files need to be re-typechecked"?

The test cases are in files called `test.txt` in the subdirectories of this
directory. They are written in a small DSL (detailed later in this file).

Additionally, we perform the sanity check that the list of typechecking
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
import textwrap
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


DEFAULT_EXEC_ENV = {
    # Local configuration may be different on local vs. CI machines, so just
    # don't use one for test runs.
    "HH_LOCALCONF_PATH": "/tmp/nonexistent"
}


def exec(args: List[str], *, raise_on_error: bool = True) -> str:
    command_line = " ".join(args)
    log(f"Running: {command_line}")
    result = subprocess.run(
        args, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=DEFAULT_EXEC_ENV
    )
    stdout = result.stdout.decode()
    if raise_on_error and result.returncode != 0:
        # stderr is pretty noisy ordinarily, and causes the logs to be
        # truncated with its length, so only surface it in cases of error.
        stderr = result.stderr.decode()
        raise RuntimeError(
            f"Command {command_line} failed with return code {result.returncode}.\n"
            + f"Stdout: {stdout}\n"
            + f"Stderr: {stderr}\n"
        )
    elif DEBUGGING:
        stderr = result.stderr.decode()
        log(f"Stderr: {stderr}")
    return stdout


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

    # Used to force a lazy init, without reporting typechecking errors (which
    # would otherwise cause the process to terminate with error).
    lazy_init_args = ["--config", "lazy_init2=true", "--config", "lazy_parse=true"]
    exec(
        [
            env.hh_server_path,
            "--check",
            env.root_dir,
            "--save-naming",
            naming_table_path,
            *lazy_init_args,
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
    common_args.extend(("--detail-level", "high"))
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
    if len(args) == 1:
        debug_result = exec([env.hh_fanout_path, "debug", *common_args, *args])
        debug_result = json.loads(debug_result)
        result["debug"] = debug_result["debug"]
    return result


def run_hh_fanout_calculate_errors(
    env: Env, saved_state_info: SavedStateInfo, cursor: Cursor
) -> Dict[str, object]:
    args = []
    args.extend(("--from", "integration-test"))
    args.extend(("--root", env.root_dir))
    args.extend(("--detail-level", "high"))
    args.extend(("--naming-table-path", saved_state_info.naming_table_path))
    args.extend(("--dep-table-path", saved_state_info.dep_table_path))
    args.extend(("--state-path", os.path.join(env.root_dir, "hh_fanout_state")))
    args.extend(("--cursor", cursor))

    result = exec([env.hh_fanout_path, "calculate-errors", *args])
    result = json.loads(result)
    return result


def run_hh_fanout_calculate_errors_pretty_print(
    env: Env, saved_state_info: SavedStateInfo, cursor: Cursor
) -> str:
    args = []
    args.extend(("--from", "integration-test"))
    args.extend(("--root", env.root_dir))
    args.extend(("--detail-level", "high"))
    args.extend(("--naming-table-path", saved_state_info.naming_table_path))
    args.extend(("--dep-table-path", saved_state_info.dep_table_path))
    args.extend(("--state-path", os.path.join(env.root_dir, "hh_fanout_state")))
    args.extend(("--cursor", cursor))
    args.append("--pretty-print")

    result = exec([env.hh_fanout_path, "calculate-errors", *args])
    result = result.replace(env.root_dir, "")
    result = result.strip()
    return result


def run_hh_fanout_status(env: Env, cursor: Cursor) -> str:
    args = []
    args.extend(("--from", "integration-test"))
    args.extend(("--root", env.root_dir))
    args.extend(("--state-path", os.path.join(env.root_dir, "hh_fanout_state")))
    args.extend(("--cursor", cursor))

    result = exec([env.hh_fanout_path, "status", *args])
    result = result.replace(env.root_dir, "")
    result = result.strip()
    return result


def run_hh_server_check(env: Env) -> str:
    result = exec(
        [env.hh_server_path, "--check", env.root_dir, "--no-load"],
        # Returns 1 when there's typechecking errors.
        raise_on_error=False,
    )
    result = result.replace(env.root_dir, "")
    result = result.strip()
    return result


def sanitize_hh_fanout_result(env: Env, result: Dict[str, object]) -> None:
    result["files"] = [
        relativize_path(env, path) for path in cast(List[str], result["files"])
    ]

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


def run_typecheck_test(
    env: Env, saved_state_info: SavedStateInfo, work_dir: Path, cursor: Cursor
) -> None:
    hh_fanout_result = run_hh_fanout_calculate_errors_pretty_print(
        env=env, saved_state_info=saved_state_info, cursor=cursor
    )
    hh_server_result = run_hh_server_check(env=env)
    print(hh_fanout_result)

    if hh_fanout_result == hh_server_result:
        print("(Additionally, hh_fanout errors matched hh_server errors.)")
    else:
        nocommit = "\x40nocommit"
        print(f"{nocommit} -- hh_fanout did NOT match hh_server output!")
        print("hh_server errors:")
        print(hh_server_result)


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

        with open(args.test_path) as f:
            test_commands = f.readlines()

        saved_state_info: Optional[SavedStateInfo] = None
        is_first = True
        changed_files: List[Path] = []
        cursor: Optional[Cursor] = None
        i = -1
        while i + 1 < len(test_commands):
            i += 1
            test_command = test_commands[i]
            test_command = test_command.strip()
            if test_command.startswith("#") or test_command == "":
                continue
            [command, *command_args] = test_command.split()

            # `write` writes the given file contents to disk, and queues it up
            # as a "changed file" for the next `calculate-fanout` query.
            if command == "write":
                content_lines = []
                while i + 1 < len(test_commands) and test_commands[i + 1][0].isspace():
                    content_lines.append(test_commands[i + 1])
                    i += 1
                # Note that the lines already have `\n` at their ends, no need
                # to add newlines.
                content = "".join(content_line for content_line in content_lines)
                content = textwrap.dedent(content)

                [target_filename] = command_args
                target_path = os.path.join(work_dir, target_filename)
                changed_files.append(target_path)
                with open(target_path, "w") as f:
                    f.write(content)

            # `generate-saved-state` generates a saved-state for use in
            # subsequent commands. Can only be called once.
            elif command == "generate-saved-state":
                assert (
                    saved_state_info is None
                ), "Cannot call `generate-saved-state` more than once"
                saved_state_info = generate_saved_state(env, work_dir)
                changed_files = []

            # `calculate-fanout` calculates the fanout for the arguments and
            # prints it to the test output for comparison against the `.exp`
            # file.
            elif command == "calculate-fanout":
                if is_first:
                    is_first = False
                else:
                    print()
                print(f"Fanout calculation on line {i + 1}")

                assert (
                    saved_state_info is not None
                ), f"Must call `generate-saved-state` before `calculate-fanout` on line {i + 1}"
                cursor = run_fanout_test(
                    env=env,
                    saved_state_info=saved_state_info,
                    args=command_args,
                    changed_files=changed_files,
                    cursor=cursor,
                )
                changed_files = []

            # `calculate-errors` calculates the set of typechecking errors in
            # the codebase and prints it to the test output for comparison
            # against the `.exp` file.
            elif command == "calculate-errors":
                if is_first:
                    is_first = False
                else:
                    print()
                print(f"Typecheck for change set on line {i + 1}")

                # We require a cursor to calculate errors, but we won't have a
                # non-`None` one on the first iteration of this loop. Do an
                # `hh_fanout` query to bring us up to date and to ensure we
                # have a cursor.
                assert (
                    saved_state_info is not None
                ), f"Must call `generate-saved-state` before `calculate-errors` on line {i + 1}"
                cursor = cast(
                    Optional[Cursor],
                    run_hh_fanout(
                        env=env,
                        saved_state_info=saved_state_info,
                        changed_files=changed_files,
                        args=[],
                        cursor=cursor,
                    )["cursor"],
                )
                changed_files = []
                assert (
                    cursor is not None
                ), "Cursor should be available since we queried fanout"

                assert (
                    saved_state_info is not None
                ), f"Must call `generate-saved-state` before `calculate-errors` on line {i + 1}"
                run_typecheck_test(
                    env=env,
                    saved_state_info=saved_state_info,
                    work_dir=work_dir,
                    cursor=cursor,
                )

            else:
                raise ValueError(f"Unrecognized test command: {command}")

        if DEBUGGING:
            debug_dir = "/tmp/hh_fanout_debug"
            shutil.rmtree(debug_dir, ignore_errors=True)
            shutil.copytree(work_dir, debug_dir)
            sys.stderr.write(
                "DEBUGGING is enabled. "
                + f"You can examine the saved states at: {debug_dir}\n"
            )


if __name__ == "__main__":
    main()
