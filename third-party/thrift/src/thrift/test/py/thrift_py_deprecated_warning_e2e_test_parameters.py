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


import textwrap


def tty_warning_cases() -> list[tuple[str, str, str, int, str, str, int, str, str]]:
    return [
        (
            "helper_default_source_a",
            "helper-default",
            "source-a",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                Hello Thrift!!!
                """
            ),
            0,
            "",
            "Hello Thrift!!!\n",
        ),
        (
            "helper_default_two_source_modules",
            "helper-default",
            "two-source-modules",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                Hello Thrift!!!
                """
            ),
            0,
            "",
            "Hello Thrift!!!\n",
        ),
    ]


def non_tty_detailed_warning_cases() -> list[
    tuple[str, str, str, str, int, str, str, int, str, str]
]:
    return [
        (
            "helper_default_source_a",
            "helper-default",
            "source-a",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            "",
        ),
        (
            "helper_default_source_b",
            "helper-default",
            "source-b",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            "",
        ),
        (
            "helper_default_two_source_modules",
            "helper-default",
            "two-source-modules",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            "",
        ),
        (
            "python_default_source_a",
            "python-default",
            "source-a",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        ),
        (
            "module_two_source_modules",
            "module",
            "two-source-modules",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        ),
        (
            "once_two_source_modules",
            "once",
            "two-source-modules",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        ),
        (
            "generic_error_source_a",
            "generic-error",
            "source-a",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            "",
        ),
        (
            "always_source_a",
            "always",
            "source-a",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        ),
        (
            "always_two_source_modules",
            "always",
            "two-source-modules",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_a:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                thrift.test.py.thrift_py_deprecated_warning_e2e_source_b:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        ),
    ]


def lazy_first_use_warning_cases() -> list[
    tuple[str, str, str, str, int, str, str, int, str, str]
]:
    return [
        (
            "helper_default_lazy_first_use",
            "helper-default",
            "lazy-first-use",
            "pipe",
            0,
            "",
            textwrap.dedent(
                """\
                thrift.test.py.thrift_py_deprecated_warning_e2e_lazy_source:<LINE>: ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            0,
            "",
            "",
        ),
    ]


def non_tty_filter_exit_cases() -> list[
    tuple[str, str, str, str, int, str, str, int, str, str]
]:
    return [
        (
            "ignore_source_a",
            "ignore",
            "source-a",
            "pipe",
            0,
            "completed ignore source-a\n",
            "",
            0,
            "completed ignore source-a\n",
            "",
        ),
        (
            "error_source_a",
            "error",
            "source-a",
            "pipe",
            1,
            "",
            textwrap.dedent(
                """\
                Traceback (most recent call last):
                thrift.Thrift.ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
            1,
            "",
            textwrap.dedent(
                """\
                Traceback (most recent call last):
                thrift.Thrift.ThriftPyDeprecatedWarning: Uses thrift-py-deprecated. Migrate to thrift-python. See https://fburl.com/thrift-python and https://fburl.com/wiki/jihy02dr. Future automatic thrift-py-deprecated code generation may stop for non-migrated targets: https://fburl.com/workplace/wer48s4m.
                """
            ),
        ),
    ]
