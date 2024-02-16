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

"""
* Invoke from the `/fbsource/fbcode/` directory using `buck run`:

    buck run thrift/compiler/test:build_fixtures

will build all fixtures under `thrift/compiler/test/fixtures`. Or

    buck run thrift/compiler/test:build_fixtures -- --fixture-names [$FIXTURENAMES]

will only build selected fixtures under `thrift/compiler/test/fixtures`.

* Invoke directly (if buck is not available):

    thrift/compiler/test/build_fixtures.py \
            --thrift-bin [$THRIFTBIN]      \
            --fixture-root [$FIXTUREROOT]  \
            --fixture-names [$FIXTURENAMES]

$THRIFTBIN is the absolute or relative path to thrift compiler executable.
$FIXTUREROOT/thrift/compiler/test/fixtures is the fixture dir (defaults
to the current working directory) and $FIXTURENAMES is a list of
fixture names to build specifically (default is to build all fixtures).
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


def ascend_find_exe(path: str, target: str) -> typing.Optional[str]:
    """
    Returns path to an executable file with the `target` name, in `path` or
    any of its parent directories.

    If no such executable is found, returns None.
    """
    if not os.path.isdir(path):
        path = os.path.dirname(path)
    while True:
        test = os.path.join(path, target)
        if os.access(test, os.X_OK):
            return test
        parent = os.path.dirname(path)
        if os.path.samefile(parent, path):
            return None
        path = parent


def read_lines(path: str) -> list[str]:
    with open(path, "r") as f:
        return f.readlines()


def _add_option_to_generator_spec(
    generator_spec: str, option_key: str, option_value: typing.Optional[str] = None
) -> str:
    """
    Returns a new generator spec obtained by adding the given option to
    `generator_spec`.

    Args:
        generator_spec: The original generator spec string, on which to add the
          given option. Should be in the form expected by the thrift compiler
          `--gen` flag, i.e.: `language[:key1=val1[,key2,[key3=val3]]]`
    """
    option_string = option_key
    if option_value:
        option_string += f"={option_value}"

    if ":" in generator_spec:
        # generator_spec already has an option
        return f"{generator_spec},{option_string}"

    # this is the first option of generator_spec
    return f"{generator_spec}:{option_string}"


def _should_build_included_files_recursively(generator_spec: str) -> bool:
    """
    Returns whether the thrift compiler should build all included thrift files
    recursively, for the given generator spec string.
    """
    # TODO: (yuhanhao) T41937765 When using generators that use
    # `mstch_objects` in recursive mode, if included thrift file
    # contains const structs or const union, generator will attempt to
    # de-reference a nullptr in `mstch_const_value::const_struct()`.
    # This is a hack before this is resolved.
    return not (
        "cpp2" in generator_spec
        or "schema" in generator_spec
        or "mstch_java" in generator_spec
        or "mstch_python" in generator_spec
    )


def _parse_cmd(
    cmd: str, fixture_root: str, fixture_name: str, fixture_src: str, thrift_bin: str
) -> typing.Optional[list[str]]:
    """
    Parses the given line from a `cmd` file and returns the command arguments
    to run (or None if n/a).

    Args:
        cmd: a single line from the `cmd` file of a thrift compiler fixture.
          It is expected to have the following format:

          ```
          GENERATOR_SPEC INPUT_FILE
          ```

          where:
          `GENERATOR_SPEC` is a "generator spec" string suitable for the `--gen`
          option of the thrift compiler (see `_add_option_to_generator_spec()`).

          INPUT_FILE is the relative path to the Thrift IDL file that will be
          passed to the thrift compiler.

          Example:

          ```
          hack:json,typedef,shapes=1 src/module.thrift
          ```
    """
    # Skip commented lines
    if re.match(r"^\s*#", cmd):
        return None

    (generator_spec, target_filename) = shlex.split(cmd.strip())

    # Relative path from --fixture_root to target file, eg:
    # 'thrift/compiler/test/fixtures/adapter/src/module.thrift'
    target_filename = os.path.relpath(
        os.path.join(fixture_src, target_filename), fixture_root
    )

    base_args = [
        thrift_bin,
        "-I",
        os.path.abspath(fixture_src),
        "-I",
        os.path.abspath(fixture_root),
    ]
    if _should_build_included_files_recursively(generator_spec):
        base_args.append("-r")

    base_args += [
        "-o",
        os.path.abspath(fixture_src),
        "--gen",
    ]

    # Add include_prefix for mstch_cpp* generators
    if "mstch_cpp" in generator_spec:
        generator_spec = _add_option_to_generator_spec(
            generator_spec,
            "include_prefix",
            os.path.join("thrift/compiler/test/fixtures", fixture_name),
        )

    return base_args + [generator_spec, target_filename]


def _get_thrift_binary_path(
    thrift_bin_arg: typing.Optional[str],
) -> typing.Optional[str]:
    """
    Returns path to the Thrift compiler binary, or None if not be found.

    Args:
        thrift_bin_arg: Name of Thrift compiler binary provided by user on the
          command line, if any. If this is not None, will try to find an
          executable with this name in the current working directory or any of
          its parents.
    """
    if thrift_bin_arg is None:
        return pkg_resources.resource_filename(__name__, "thrift")

    exe: str = os.path.join(os.getcwd(), sys.argv[0])
    return ascend_find_exe(exe, thrift_bin_arg)


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

    # Read commands from "cmd" file. See `_parse_cmd()` for the expected format.
    cmds = read_lines(os.path.join(fixture_src, "cmd"))

    for cmd in cmds:
        xargs = _parse_cmd(cmd, fixture_root, fixture_name, fixture_src, thrift_bin)
        if not xargs:
            continue

        processes.append(run_subprocess(subprocess_semaphore, xargs, cwd=fixture_root))


async def main():
    args = parsed_args()

    thrift_bin: typing.Optional[str] = _get_thrift_binary_path(args.thrift_bin)
    if thrift_bin is None:
        sys.stderr.write(
            "error: cannot find the Thrift compiler ({})\n".format(args.thrift_bin)
        )
        sys.exit(1)

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


asyncio.run(main())

sys.exit(int(has_errors))
