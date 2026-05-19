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

from __future__ import annotations

import errno
import importlib.resources
import os
import pty
import re
import subprocess
import textwrap
import unittest
from dataclasses import dataclass

from parameterized import parameterized
from thrift.test.py import (
    thrift_py_deprecated_warning_e2e_test_parameters as parameters,
)


@dataclass(frozen=True)
class ProcessResult:
    returncode: int
    stdout: str
    stderr: str


class TestThriftPyDeprecatedWarningE2E(unittest.TestCase):
    def __binary_path(self) -> str:
        return str(
            importlib.resources.files("thrift.test.py").joinpath(
                "thrift_py_deprecated_warning_e2e_cli"
            )
        )

    def __read_pty_output(self, master_fd: int) -> str:
        chunks: list[str] = []
        while True:
            try:
                chunk = os.read(master_fd, 4096)
            except OSError as error:
                if error.errno == errno.EIO:
                    break
                raise
            if not chunk:
                break
            chunks.append(chunk.decode())
        return "".join(chunks).replace("\r\n", "\n")

    def __run_with_tty_stderr(self, warning_mode: str, scenario: str) -> ProcessResult:
        master_fd, slave_fd = pty.openpty()
        slave_fd_open = True
        try:
            process = subprocess.Popen(
                [self.__binary_path(), warning_mode, scenario],
                stdout=subprocess.PIPE,
                stderr=slave_fd,
                text=True,
            )
            os.close(slave_fd)
            slave_fd_open = False
            stderr = self.__read_pty_output(master_fd)
            stdout, _stderr = process.communicate()
            returncode = process.returncode
            assert returncode is not None
            return ProcessResult(
                returncode=returncode,
                stdout=stdout,
                stderr=stderr,
            )
        finally:
            os.close(master_fd)
            if slave_fd_open:
                os.close(slave_fd)

    def __normalize_stderr(self, stderr: str) -> str:
        stderr = re.sub(
            r'File ".*?thrift_py_deprecated_warning_e2e_cli#link-tree/',
            'File "<e2e_cli>#link-tree/',
            stderr,
        )
        stderr = re.sub(
            r'File "/usr/local/fbcode/[^/]+/lib/python3\.\d+/',
            'File "<python-runtime>/',
            stderr,
        )
        return stderr

    def __abridge_error_stderr(self, stderr: str) -> str:
        exception_match = re.search(
            r"^thrift\.Thrift\.ThriftPyDeprecatedWarning: .*$",
            stderr,
            flags=re.MULTILINE,
        )
        if (
            not stderr.startswith("Traceback (most recent call last):\n")
            or exception_match is None
        ):
            return stderr
        return f"Traceback (most recent call last):\n{exception_match.group(0)}\n"

    def __run_with_piped_stderr(
        self, warning_mode: str, scenario: str
    ) -> ProcessResult:
        completed = subprocess.run(
            [self.__binary_path(), warning_mode, scenario],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            check=False,
        )
        stderr = self.__normalize_stderr(completed.stderr)
        if completed.returncode != 0:
            abridged_stderr = self.__abridge_error_stderr(stderr)
        else:
            abridged_stderr = stderr
        return ProcessResult(
            returncode=completed.returncode,
            stdout=completed.stdout,
            stderr=abridged_stderr,
        )

    # pyre-ignore[56]: Pyre cannot infer parameterized tuple type.
    @parameterized.expand(parameters.tty_cli_summary_cases())
    def test_tty_cli_summary_output(
        self,
        _name: str,
        warning_mode: str,
        scenario: str,
        _execution_surface: str,
        expected_returncode: int,
        expected_stdout: str,
        expected_stderr: str,
    ) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=expected_returncode,
            stdout=expected_stdout,
            stderr=expected_stderr,
        )

        # WHEN
        actual = self.__run_with_tty_stderr(warning_mode, scenario)

        # THEN
        self.assertEqual(expected, actual)

    def test_force_env_keeps_detailed_warning_on_tty(self) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=0,
            stdout="",
            stderr=textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:18: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                Hello Thrift!!!
                """
            ),
        )
        original_warning_env = os.environ.get("THRIFT_PY_DEPRECATED_WARNING")
        os.environ["THRIFT_PY_DEPRECATED_WARNING"] = "force"

        try:
            # WHEN
            actual = self.__run_with_tty_stderr("helper-default", "source-a")
        finally:
            if original_warning_env is None:
                os.environ.pop("THRIFT_PY_DEPRECATED_WARNING", None)
            else:
                os.environ["THRIFT_PY_DEPRECATED_WARNING"] = original_warning_env

        # THEN
        self.assertEqual(expected, actual)

    # pyre-ignore[56]: Pyre cannot infer parameterized tuple type.
    @parameterized.expand(parameters.non_tty_filter_exit_cases())
    def test_non_tty_filter_exit_behavior(
        self,
        _name: str,
        warning_mode: str,
        scenario: str,
        _execution_surface: str,
        expected_returncode: int,
        expected_stdout: str,
        expected_stderr: str,
    ) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=expected_returncode,
            stdout=expected_stdout,
            stderr=expected_stderr,
        )

        # WHEN
        actual = self.__run_with_piped_stderr(warning_mode, scenario)

        # THEN
        self.assertEqual(expected, actual)

    # pyre-ignore[56]: Pyre cannot infer parameterized tuple type.
    @parameterized.expand(parameters.lazy_first_use_warning_cases())
    def test_lazy_first_use_warning_shape(
        self,
        _name: str,
        warning_mode: str,
        scenario: str,
        _execution_surface: str,
        expected_returncode: int,
        expected_stdout: str,
        expected_stderr: str,
    ) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=expected_returncode,
            stdout=expected_stdout,
            stderr=expected_stderr,
        )

        # WHEN
        actual = self.__run_with_piped_stderr(warning_mode, scenario)

        # THEN
        self.assertEqual(expected, actual)

    # pyre-ignore[56]: Pyre cannot infer parameterized tuple type.
    @parameterized.expand(parameters.non_tty_detailed_warning_cases())
    def test_non_tty_detailed_warning_output(
        self,
        _name: str,
        warning_mode: str,
        scenario: str,
        _execution_surface: str,
        expected_returncode: int,
        expected_stdout: str,
        expected_stderr: str,
    ) -> None:
        # GIVEN
        expected = ProcessResult(
            returncode=expected_returncode,
            stdout=expected_stdout,
            stderr=expected_stderr,
        )

        # WHEN
        actual = self.__run_with_piped_stderr(warning_mode, scenario)

        # THEN
        self.assertEqual(expected, actual)
