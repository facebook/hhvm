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

import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile
import traceback
import typing
import unittest
from pathlib import Path

from thrift.compiler.test import fixture_utils


_THRIFT_BIN_PATH = fixture_utils.get_thrift_binary_path(thrift_bin_arg=None)
assert _THRIFT_BIN_PATH

_FIXTURES_ROOT_DIR_RELPATH = Path("thrift/compiler/test/fixtures")


def _gen_find_recursive_files(top: Path) -> typing.Generator[Path, None, None]:
    """Yields a Path for every file under `top`, relative to it."""

    for root, _, filenames in os.walk(top):
        root_path = Path(root)
        for filename in filenames:
            yield (root_path / filename).relative_to(top)


class FixtureTest(unittest.TestCase):

    MSG = " ".join(
        [
            "One or more fixtures are out of sync with the thrift compiler.",
            "To sync them, build thrift and then run:",
            "`thrift/compiler/test/build_fixtures <build-dir>`, where",
            "<build-dir> is a path where the program `thrift/compiler/thrift`",
            "may be found.",
        ]
    )

    def _compare_code(
        self,
        gen_code_path: Path,
        fixture_code_path: Path,
        cmd: typing.List[str],
        enable_dedicated_output_dir: bool,
    ) -> None:
        """
        Checks that the contents of the files under the two given paths are
        identical, and fails this test if that is not the case.
        """
        gen_file_relpaths: list[Path] = sorted(_gen_find_recursive_files(gen_code_path))

        # TODO: Remove the filtering logic of the non-output files in the source
        # fixture as soon as outputs are moved into a dedicated folder
        #  (eg ".../out/...") - i.e., as soon as `enable_dedicated_output_dir`
        # is always True.
        if enable_dedicated_output_dir:
            fixture_file_relpaths: list[Path] = sorted(
                _gen_find_recursive_files(fixture_code_path)
            )
        else:
            fixture_file_relpaths: list[Path] = sorted(
                file_relpath
                for file_relpath in _gen_find_recursive_files(fixture_code_path)
                if file_relpath.name != "cmd" and file_relpath.parts[0] != "src"
            )

        try:
            # Compare that the generated files are the same
            self.assertEqual(gen_file_relpaths, fixture_file_relpaths)

            for gen_file_relpath in gen_file_relpaths:
                gen_file_path = gen_code_path / gen_file_relpath
                fixture_file_path = fixture_code_path / gen_file_relpath
                gen_file_contents = gen_file_path.read_text()
                fixture_file_contents = fixture_file_path.read_text()
                if gen_file_contents == fixture_file_contents:
                    continue

                msg = [f"Difference found in {gen_file_relpath}:"]
                for line in difflib.unified_diff(
                    fixture_file_contents.splitlines(),
                    gen_file_contents.splitlines(),
                    str(fixture_file_path),
                    str(gen_file_path),
                    lineterm="",
                ):
                    msg.append(line)
                msg.append("Command: {}".format(cmd))
                self.fail("\n".join(msg))
        except Exception:
            print(self.MSG, file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            raise

    def setUp(self) -> None:
        tmp = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, tmp, True)
        self.tmp_dir_abspath = Path(tmp).resolve(strict=True)
        self.maxDiff = None

    def runTest(self, fixture_name: str) -> None:

        repo_root_dir_abspath = Path.cwd()
        fixture_dir_abspath = (
            repo_root_dir_abspath / _FIXTURES_ROOT_DIR_RELPATH / fixture_name
        )

        enable_dedicated_output_dir = fixture_utils.is_dedicated_output_dir_enabled(
            fixture_dir_abspath
        )

        fixture_output_root_dir_abspath = (
            self.tmp_dir_abspath / "out"
            if enable_dedicated_output_dir
            else self.tmp_dir_abspath
        )

        if enable_dedicated_output_dir or not fixture_output_root_dir_abspath.exists():
            fixture_output_root_dir_abspath.mkdir()

        fixture_cmds = fixture_utils.parse_fixture_cmds(
            repo_root_dir_abspath,
            fixture_name,
            fixture_dir_abspath,
            fixture_output_root_dir_abspath,
            _THRIFT_BIN_PATH,
            enable_dedicated_output_dir,
        )

        # Run thrift compiler and generate files
        for fixture_cmd in fixture_cmds:
            if enable_dedicated_output_dir:
                os.mkdir(fixture_output_root_dir_abspath / fixture_cmd.unique_name)

            subprocess.check_call(
                fixture_cmd.build_command_args,
                close_fds=True,
            )

        # Compare generated code to fixture code
        self._compare_code(
            fixture_output_root_dir_abspath,
            (
                fixture_dir_abspath / "out"
                if enable_dedicated_output_dir
                else fixture_dir_abspath
            ),
            fixture_cmd.build_command_args,
            enable_dedicated_output_dir,
        )


def _add_fixture(klazz, fixture_name: str) -> None:
    def test_method(self):
        self.runTest(fixture_name)

    test_method.__name__ = str("test_" + re.sub("[^0-9a-zA-Z]", "_", fixture_name))
    setattr(klazz, test_method.__name__, test_method)


for fixture_name in fixture_utils.get_all_fixture_names(_FIXTURES_ROOT_DIR_RELPATH):
    _add_fixture(FixtureTest, fixture_name)

if __name__ == "__main__":
    unittest.main()
