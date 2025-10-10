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

# pyre-unsafe

import atexit
import os
import re
import shlex
import textwrap
import typing
from contextlib import ExitStack
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Optional

import importlib_resources


@dataclass
class FixtureCmd:
    unique_name: str
    build_command_args: typing.List[str]
    output_directory_arg_pos: typing.Final[typing.Optional[int]]


def _ascend_find_exe(path: Path, target: str) -> typing.Optional[Path]:
    """
    Returns path to an executable file with the `target` name, in `path` or
    any of its parent directories.

    If no such executable is found, returns None.
    """
    if not path.is_dir():
        path = path.parent
    while True:
        candidate_path = path / target
        if os.access(candidate_path, os.X_OK):
            return candidate_path

        parent_path = path.parent
        if parent_path.samefile(path):
            return None
        path = parent_path


def read_lines(path: Path) -> list[str]:
    with path.open() as f:
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
        or "-ast" in generator_spec
    )


def parse_fixture_cmds(
    repo_root_dir_abspath: Path,
    fixture_name: str,
    fixture_dir_abspath: Path,
    fixture_output_root_dir_abspath: Path,
    thrift_bin_path: Path,
    thrift2ast_bin_path: Optional[Path],
) -> list[FixtureCmd]:
    assert repo_root_dir_abspath.is_absolute()
    assert fixture_dir_abspath.is_absolute()
    assert fixture_output_root_dir_abspath.is_absolute()
    assert fixture_output_root_dir_abspath.is_dir()

    fixture_cmds = {}

    cmds = read_lines(fixture_dir_abspath / "cmd")

    for cmd in cmds:
        fixture_cmd = _parse_fixture_cmd(
            cmd,
            repo_root_dir_abspath,
            fixture_name,
            fixture_dir_abspath,
            fixture_output_root_dir_abspath,
            thrift_bin_path,
            thrift2ast_bin_path,
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
    cmd: str,
    repo_root_dir_abspath: Path,
    fixture_name: str,
    fixture_dir_abspath: Path,
    fixture_output_root_dir_abspath: Path,
    thrift_bin_path: Path,
    thrift2ast_bin_path: Optional[Path],
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
        (cmd_name_prefix, generator_spec, target_file_name) = shlex.split(cmd.strip())

        cmd_name_matcher = _FIXTURE_CMD_NAME_PREFIX_PATTERN.match(cmd_name_prefix)
        if not cmd_name_matcher:
            raise RuntimeError(
                f"Invalid cmd name (must match regex '{_FIXTURE_CMD_NAME_PREFIX_PATTERN.pattern}'): '{cmd_name_prefix}'."
            )
        cmd_name = cmd_name_matcher.group(1)

        # target_file_name: "src/module.thrift" ->
        # target_file_relpath: "thrift/compiler/test/fixtures/foobar/src/module.thrift"
        target_file_relpath = (fixture_dir_abspath / target_file_name).relative_to(
            repo_root_dir_abspath
        )

        base_args = [
            thrift_bin_path,
            "-I",
            fixture_dir_abspath,
            "-I",
            repo_root_dir_abspath,
        ]
        if _should_build_included_files_recursively(generator_spec):
            base_args.append("-r")

        base_args += [
            "-o",
            fixture_output_root_dir_abspath / cmd_name,
            "--gen",
        ]
        # Output directory is before '--gen' which is at current_len - 1
        output_directory_arg_pos = len(base_args) - 2

        # Add include_prefix for mstch_cpp* generators
        if (
            "mstch_cpp" in generator_spec
            or "mstch_py3" in generator_spec
            or "mstch_python_capi" in generator_spec
        ):
            generator_spec = _add_option_to_generator_spec(
                generator_spec,
                "include_prefix",
                str(PurePosixPath("thrift/compiler/test/fixtures") / fixture_name),
            )

        if generator_spec.startswith("thrift2ast-"):
            if thrift2ast_bin_path is None:
                raise RuntimeError("No path to `thrift2ast` binary provided.")
            generator_spec = generator_spec.removeprefix("thrift2ast-")
            base_args[0] = thrift2ast_bin_path
            base_args[-1:-1] = ["--inject-schema-const"]

        if generator_spec.startswith("streamdev-"):
            generator_spec = generator_spec.removeprefix("streamdev-")
            base_args[-1:-1] = ["--allow-unreleased-streaming"]

        return FixtureCmd(
            unique_name=cmd_name,
            build_command_args=[
                str(x) for x in base_args + [generator_spec, target_file_relpath]
            ],
            output_directory_arg_pos=output_directory_arg_pos,
        )
    except Exception as err:
        raise RuntimeError(
            f"Error parsing command for fixture '{fixture_name}': '{cmd}'."
        ) from err


def _get_binary(filename: str) -> Optional[Path]:
    """
    Returns path to the binary, or None if not be found.
    """
    file_manager = ExitStack()
    atexit.register(file_manager.close)
    resource = importlib_resources.files(__name__) / filename
    path = file_manager.enter_context(importlib_resources.as_file(resource))
    return path if path.is_file() else None


def get_thrift_binary_path(
    thrift_bin_arg: typing.Optional[str],
) -> typing.Optional[Path]:
    """
    Returns path to the Thrift compiler binary, or None if not be found.

    Args:
        thrift_bin_arg: Name of Thrift compiler binary provided by user on the
          command line, if any. If this is not None, will try to find an
          executable with this name in the current working directory or any of
          its parents.
    """
    if thrift_bin_arg is None:
        return _get_binary("thrift")

    return _ascend_find_exe(Path.cwd(), thrift_bin_arg)


def get_thrift2ast_binary_path() -> typing.Optional[Path]:
    """
    Returns path to the Thrift compiler binary, or None if not be found.

    Args:
        thrift_bin_arg: Name of Thrift compiler binary provided by user on the
          command line, if any. If this is not None, will try to find an
          executable with this name in the current working directory or any of
          its parents.
    """
    return _get_binary("thrift2ast")


def get_all_fixture_names(fixtures_root_dir_path: Path) -> list[str]:
    """ "
    Returns the (simple) names of all the fixture directories in the given
    directory (which should typically correspond to the
    `thrift/compiler/test/fixtures/` directory in a given repository).

    The returned list contains the sorted "base names" of the fixture
    directories, without any parent path, eg:

    ```
    [ 'any', 'basic', ...]
    ```
    """

    validate_that_root_path_is_a_dir(fixtures_root_dir_path)

    fixture_dirs = (
        fixture_dir
        for fixture_dir in fixtures_root_dir_path.iterdir()
        if (fixture_dir / "cmd").is_file()
    )

    return sorted((fixture_dir.name for fixture_dir in fixture_dirs))


def validate_that_root_path_is_a_dir(root_path: Path) -> None:
    if not root_path.is_dir():
        raise RuntimeError(
            textwrap.dedent(
                f"""\
                Expected {root_path} to be a directory.
                This usually means either an incorrect current directory
                or an incorrect `--fixture-root` option.

                For complete help, run:
                buck run xplat/thrift/compiler/test:build_fixtures -- --help
                """
            )
        )


def apply_postprocessing(
    fixture_output_root_dir_abspath: Path,
) -> None:
    """
    Walks through the output directory and applies postprocessing to each file.

    Current steps:
      - Parts of lines between `@fbthrift_strip_from_fixtures` are replaced with `<truncated>`.

    Args:
        fixture_output_root_dir_abspath: The root directory of the fixture outputs.
    """
    DELIMITER = "@fbthrift_strip_from_fixtures"

    for root, _, files in os.walk(fixture_output_root_dir_abspath):
        for file in files:
            file_path = Path(root) / file
            with open(file_path, "r+", encoding="utf-8") as f:
                changed = False
                content = f.readlines()
                for i, line in enumerate(content):
                    if DELIMITER in line:
                        while line.count(DELIMITER) < 2:
                            line = line + content[i + 1]
                            del content[i + 1]
                        parts = line.split(DELIMITER)
                        content[i] = parts[0] + "<truncated>" + parts[2]
                        changed = True
                if changed:
                    f.seek(0)
                    f.writelines(content)
                    f.truncate()
