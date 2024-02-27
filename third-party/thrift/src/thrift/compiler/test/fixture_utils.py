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

import os
import re
import shlex
import sys
import typing
from dataclasses import dataclass

import pkg_resources


@dataclass
class FixtureCmd:
    unique_name: str
    build_command_args: typing.List[str]


def _ascend_find_exe(path: str, target: str) -> typing.Optional[str]:
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


def parse_fixture_cmds(
    fixture_root: str, fixture_name: str, fixture_src: str, thrift_bin: str
) -> list[FixtureCmd]:
    fixture_cmds = {}

    cmds = read_lines(os.path.join(fixture_src, "cmd"))

    for cmd in cmds:
        fixture_cmd = _parse_fixture_cmd(
            cmd, fixture_root, fixture_name, fixture_src, thrift_bin
        )
        if fixture_cmd is None:
            continue

        if fixture_cmd.unique_name in fixture_cmds:
            raise ValueError(
                f"Duplicate cmd name for fixture '{fixture_name}': '{fixture_cmd.unique_name}'."
            )

        fixture_cmds[fixture_cmd.unique_name] = fixture_cmd

    return list(fixture_cmds.values())


# A letter followed by 0 or more letters, digits or underscores, ending with
# a colon. All lowercase. The part preceding the colon is meant to be the
# unique (within a fixture) name for this cmd line. It is captured in group 1.
_FIXTURE_CMD_NAME_PREFIX_PATTERN = re.compile(r"^([a-z][a-z0-9_]*):$")


def _parse_fixture_cmd(
    cmd: str, fixture_root: str, fixture_name: str, fixture_src: str, thrift_bin: str
) -> typing.Optional[FixtureCmd]:
    """
    Parses the given line from a `cmd` file and returns the command arguments
    to run (or None if n/a).

    Args:
        cmd: a single line from the `cmd` file of a thrift compiler fixture.
          It is expected to have the following format:

          ```
          UNIQUE_NAME: GENERATOR_SPEC INPUT_FILE
          ```

          where:
          `UNIQUE_NAME` is a unique (within this folder) identifier for this
          command. It is primarily used to separate the output of each command
          (to avoid clobbering of generated content).

          `GENERATOR_SPEC` is a "generator spec" string suitable for the `--gen`
          option of the thrift compiler (see `_add_option_to_generator_spec()`).

          INPUT_FILE is the relative path to the Thrift IDL file that will be
          passed to the thrift compiler.

          Example:

          ```
          hack: hack:json,typedef,shapes=1 src/module.thrift
          ```
    """
    # Skip commented lines
    if re.match(r"^\s*#", cmd):
        return None

    try:
        (cmd_name_prefix, generator_spec, target_filename) = shlex.split(cmd.strip())

        cmd_name_matcher = _FIXTURE_CMD_NAME_PREFIX_PATTERN.match(cmd_name_prefix)
        if not cmd_name_matcher:
            raise RuntimeError(
                f"Invalid cmd name (must match regex '{_FIXTURE_CMD_NAME_PREFIX_PATTERN.pattern}'): '{cmd_name_prefix}'."
            )
        cmd_name = cmd_name_matcher.group(1)

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

        return FixtureCmd(
            unique_name=cmd_name,
            build_command_args=base_args + [generator_spec, target_filename],
        )
    except Exception as err:
        raise RuntimeError(
            f"Error parsing command for fixture '{fixture_name}': '{cmd}'."
        ) from err


def get_thrift_binary_path(
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
    return _ascend_find_exe(exe, thrift_bin_arg)
