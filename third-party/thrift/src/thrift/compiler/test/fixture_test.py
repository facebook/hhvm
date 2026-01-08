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

import re
import subprocess
import sys
import tempfile
import traceback
import unittest
from pathlib import Path

from xplat.thrift.compiler.test import fixture_utils


_THRIFT_BIN_PATH = fixture_utils.get_thrift_binary_path(thrift_bin_arg=None)
_THRIFT2AST_BIN_PATH = fixture_utils.get_thrift2ast_binary_path()
assert _THRIFT_BIN_PATH

_FIXTURES_ROOT_DIR_RELPATH = Path("thrift/compiler/test/fixtures")


class FixtureTest(unittest.TestCase):
    MSG = (
        "One or more fixtures are out of sync with the thrift compiler. "
        "To sync them, build thrift and then run: "
        "`thrift/compiler/test/build_fixtures <build-dir>`, where "
        "<build-dir> is a path where the program `thrift/compiler/thrift` "
        "may be found."
    )

    def setUp(self) -> None:
        tmp: str = self.enterContext(tempfile.TemporaryDirectory())
        self.tmp_dir_abspath = Path(tmp).resolve(strict=True)
        self.maxDiff = None

    def runTest(
        self,
        working_dir: Path,
        fixture_dir_abspath: Path,
        fixture_cmd: fixture_utils.FixtureCmd,
    ) -> None:
        outdir = self.tmp_dir_abspath / "out" / fixture_cmd.unique_name
        outdir.mkdir(parents=True, exist_ok=False)

        if fixture_cmd.output_directory_arg_pos is not None:
            fixture_cmd.build_command_args[fixture_cmd.output_directory_arg_pos] = str(
                outdir
            )

        subprocess.check_call(
            fixture_cmd.build_command_args,
            close_fds=True,
            cwd=working_dir,
        )

        fixture_utils.apply_postprocessing(outdir)

        # Compare generated code to fixture code
        try:
            fixture_utils.assert_identical_output(
                test=self,
                expected_dir=fixture_dir_abspath / "out" / fixture_cmd.unique_name,
                actual_dir=outdir,
                working_dir=str(working_dir),
                command=" ".join(fixture_cmd.build_command_args),
            )
        except Exception:
            print(self.MSG, file=sys.stderr)
            traceback.print_exc(file=sys.stderr)
            raise


def _add_fixture(klazz, fixture_name: str) -> None:
    repo_root_dir_abspath = Path.cwd() / "xplat"
    fixture_dir_abspath = (
        repo_root_dir_abspath / _FIXTURES_ROOT_DIR_RELPATH / fixture_name
    )
    fixture_cmds = fixture_utils.parse_fixture_cmds(
        repo_root_dir_abspath=repo_root_dir_abspath,
        fixture_name=fixture_name,
        fixture_dir_abspath=fixture_dir_abspath,
        # Use temp dir as output path during test discovery
        # Replaced with an ephemeral directory by runTest during test execution
        fixture_output_root_dir_abspath=Path(tempfile.gettempdir()),
        thrift_bin_path=_THRIFT_BIN_PATH,
        thrift2ast_bin_path=_THRIFT2AST_BIN_PATH,
    )

    for fixture_cmd in fixture_cmds:

        def test_method(self: FixtureTest, cmd=fixture_cmd):
            self.runTest(
                working_dir=repo_root_dir_abspath,
                fixture_dir_abspath=fixture_dir_abspath,
                fixture_cmd=cmd,
            )

        test_method.__name__ = "test_{fixture}_{cmd_name}".format(
            fixture=re.sub("[^0-9a-zA-Z_]", "_", fixture_name),
            cmd_name=re.sub("[^0-9a-zA-Z_]", "_", fixture_cmd.unique_name),
        )
        setattr(klazz, test_method.__name__, test_method)


for fixture_name in fixture_utils.get_all_fixture_names(
    "xplat" / _FIXTURES_ROOT_DIR_RELPATH
):
    _add_fixture(FixtureTest, fixture_name)

if __name__ == "__main__":
    unittest.main()
