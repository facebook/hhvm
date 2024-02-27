#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import asyncio
import multiprocessing
import os
import re
import shlex
import shutil
import subprocess
import sys
import typing

import pkg_resources

from thrift.compiler.test import fixture_utils

"""
* Invoke from the `/fbsource/fbcode/` directory using `buck run`:

    buck run thrift/compiler/test:build_fixtures

will build all fixtures under `thrift/compiler/test/fixtures`. Or

    buck run thrift/compiler/test:build_fixtures -- --fixture-names [$FIXTURENAMES]

will only build selected fixtures under `thrift/compiler/test/fixtures`.

$FIXTURENAMES is a space-separated list of fixture names to build specifically
(default is to build all fixtures).
"""
DEFAULT_FIXTURE_ROOT = "."


def parsed_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--thrift-bin",
        dest="thrift_bin",
        help=(
            "Absolute path or relative path to thrift compiler executable. "
            "For relative path, we will search script's path and all parents"
        ),
        type=str,
        default=None,
    )
    parser.add_argument(
        "--fixture-root",
        dest="fixture_root",
        help=(
            "$FIXTUREROOT/thrift/compiler/test/fixtures is where the"
            " fixtures are located, defaults to the current working dir"
        ),
        type=str,
        default=DEFAULT_FIXTURE_ROOT,
    )
    parser.add_argument(
        "--fixture-names",
        dest="fixture_names",
        help="Name of the fixture to build, default is to build all fixtures",
        type=str,
        nargs="*",
        default=None,
    )
    return parser.parse_args()


# Will be set to true if any fixture-generating subprocess fails.
has_errors = False


async def run_subprocess(sem: asyncio.Semaphore, cmd: list[str], *, cwd: str) -> None:
    """Runs a subprocess for the given `cmd`.

    If the subprocess fails (i.e., returns a non-0 code), sets the global
    variable `has_errors` to `True`.

    Args:
        sem: Semaphore to limit the number of concurrent subprocesses
        cmd: Commands to run.
        cwd: Current working directory.
    """
    async with sem:
        p = await asyncio.create_subprocess_exec(
            *cmd,
            cwd=cwd,
            close_fds=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        out, err = await p.communicate()
        sys.stdout.write(out.decode(sys.stdout.encoding))
        if p.returncode != 0:
            global has_errors
            has_errors = True
            sys.stderr.write(err.decode(sys.stderr.encoding))


def _add_processes_for_fixture(
    fixture_name: str,
    fixture_root: str,
    fixture_dir: str,
    thrift_bin: str,
    subprocess_semaphore,
    processes: list,
) -> None:
    """
    Handles the generation of the given fixture, adding any pending coroutine
    to the given `processes` list.
    """
    # FIXTURE_ROOT/thrift/compiler/test/fixtures/FIXTURE_NAME
    # eg. './thrift/compiler/test/fixtures/adapter'
    fixture_src = os.path.join(fixture_dir, fixture_name)

    # Delete all existing fixture files (except "cmd", "src", ".*")
    for existing_fixture_filename in set(os.listdir(fixture_src)) - {"cmd", "src"}:
        if existing_fixture_filename.startswith("."):
            continue
        shutil.rmtree(os.path.join(fixture_src, existing_fixture_filename))

    for fixture_cmd in fixture_utils.parse_fixture_cmds(
        fixture_root, fixture_name, fixture_src, thrift_bin
    ):
        processes.append(
            run_subprocess(
                subprocess_semaphore, fixture_cmd.build_command_args, cwd=fixture_root
            )
        )


async def main() -> int:
    args = parsed_args()

    thrift_bin: typing.Optional[str] = fixture_utils.get_thrift_binary_path(
        args.thrift_bin
    )
    if thrift_bin is None:
        sys.stderr.write(
            "error: cannot find the Thrift compiler ({})\n".format(args.thrift_bin)
        )
        return 1

    fixture_root: str = args.fixture_root
    # Directory that contains all fixture directories (one fixture per sub-dir).
    fixture_dir: str = os.path.join(fixture_root, "thrift/compiler/test/fixtures")

    fixture_names = (
        args.fixture_names
        if args.fixture_names is not None
        else sorted(
            f
            for f in os.listdir(fixture_dir)
            if os.path.isfile(os.path.join(fixture_dir, f, "cmd"))
        )
    )

    # Semaphore to limit the number of concurrent fixture builds.
    # Otherwise, a swarm of compiler processes results in too much
    # CPU contention, consistenly killing people's devservers.
    subprocess_semaphore = asyncio.Semaphore(value=multiprocessing.cpu_count())

    build_progress_msg_format = "Building fixture {{0:>{w}}}/{{1}}: {{2}}".format(
        w=len(str(len(fixture_names)))
    )

    processes = []
    for index, fixture_name in enumerate(fixture_names):
        # eg: 'Building fixture   1/117: adapter'
        msg = build_progress_msg_format.format(
            index + 1, len(fixture_names), fixture_name
        )
        print(msg, file=sys.stderr)

        _add_processes_for_fixture(
            fixture_name,
            fixture_root,
            fixture_dir,
            thrift_bin,
            subprocess_semaphore,
            processes,
        )

    await asyncio.gather(*processes)
    return int(has_errors)


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
