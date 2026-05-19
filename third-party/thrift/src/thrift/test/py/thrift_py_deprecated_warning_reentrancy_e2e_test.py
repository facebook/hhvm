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

# pyre-strict

import importlib.resources
import subprocess
import textwrap
import unittest
from dataclasses import dataclass


@dataclass(frozen=True)
class ProcessResult:
    returncode: int
    stdout: str
    stderr: str


class TestThriftPyDeprecatedWarningReentrancyE2E(unittest.TestCase):
    def __binary_path(self) -> str:
        return str(
            importlib.resources.files("thrift.test.py").joinpath(
                "thrift_py_deprecated_warning_reentrancy_e2e_cli"
            )
        )

    def __run_repro(self, scenario: str) -> ProcessResult:
        completed = subprocess.run(
            [self.__binary_path(), scenario],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        return ProcessResult(
            returncode=completed.returncode,
            stdout=completed.stdout,
            stderr=completed.stderr,
        )

    def test_capturewarnings_filter_reentry_is_gated_in_subprocess(self) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=0,
            stdout="filter calls: 1\n",
            stderr=textwrap.dedent(
                """\
                __main__:43: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.

                """
            ),
        )

        # WHEN
        actual = self.__run_repro("default")

        # THEN
        self.assertEqual(expected, actual)

    def test_unproven_cpython_hooks_still_use_reentrancy_guard_outside_lazy_import(
        self,
    ) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=0,
            stdout="filter calls: 1\n",
            stderr=textwrap.dedent(
                """\
                __main__:43: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.

                """
            ),
        )

        # WHEN
        actual = self.__run_repro("missing-cpython-formatwarning-original")

        # THEN
        self.assertEqual(expected, actual)

    def test_missing_cpython_originals_use_fallback_during_lazy_import(self) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=0,
            stdout="filter calls: 0\n",
            stderr=textwrap.dedent(
                """\
                __main__:43: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        )

        # WHEN
        actual = self.__run_repro("lazy-missing-cpython-originals")

        # THEN
        self.assertEqual(expected, actual)
