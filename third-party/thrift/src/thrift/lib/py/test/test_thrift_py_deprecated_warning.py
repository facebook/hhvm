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

import importlib
import sys
import tempfile
import textwrap
import unittest
import uuid
import warnings
from pathlib import Path

import thrift.Thrift
from parameterized import parameterized


class TestThriftPyDeprecatedWarning(unittest.TestCase):
    def test_warning_class_subclasses_DeprecationWarning(self) -> None:
        # GIVEN
        expected = True

        # WHEN
        actual = issubclass(
            thrift.Thrift.ThriftPyDeprecatedWarning,
            DeprecationWarning,
        )

        # THEN
        self.assertEqual(expected, actual)

    def test_warns_on_direct_helper_call(self) -> None:
        # GIVEN
        module_name = "foo.bar"
        expected = [thrift.Thrift.ThriftPyDeprecatedWarning]

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.simplefilter("always", thrift.Thrift.ThriftPyDeprecatedWarning)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
        actual = [warning.category for warning in recorded]

        # THEN
        self.assertEqual(expected, actual)

    def test_distinct_generated_modules_both_warn(self) -> None:
        # GIVEN
        package_name = f"thrift_py_deprecated_warning_{uuid.uuid4().hex}"
        first_module = f"{package_name}.first"
        second_module = f"{package_name}.second"
        expected = [
            thrift.Thrift.ThriftPyDeprecatedWarning,
            thrift.Thrift.ThriftPyDeprecatedWarning,
        ]
        module_source = textwrap.dedent(
            """\
            from thrift.Thrift import warn_thrift_py_deprecated

            warn_thrift_py_deprecated(__name__)
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir_str:
            temp_dir = Path(temp_dir_str)
            package_dir = temp_dir / package_name
            package_dir.mkdir()
            (package_dir / "__init__.py").write_text("", encoding="utf-8")
            (package_dir / "first.py").write_text(module_source, encoding="utf-8")
            (package_dir / "second.py").write_text(module_source, encoding="utf-8")

            sys.path.insert(0, temp_dir_str)
            try:
                # WHEN
                with warnings.catch_warnings(record=True) as recorded:
                    warnings.resetwarnings()
                    importlib.import_module(first_module)
                    importlib.import_module(second_module)
                actual = [warning.category for warning in recorded]

                # THEN
                self.assertEqual(expected, actual)
            finally:
                sys.path.remove(temp_dir_str)
                for module_name in list(sys.modules):
                    if module_name == package_name or module_name.startswith(
                        f"{package_name}."
                    ):
                        del sys.modules[module_name]

    @parameterized.expand(
        [
            (False, [thrift.Thrift.ThriftPyDeprecatedWarning]),
            (
                True,
                [
                    thrift.Thrift.ThriftPyDeprecatedWarning,
                    thrift.Thrift.ThriftPyDeprecatedWarning,
                ],
            ),
        ]
    )
    def test_generated_module_reexecution_respects_filter_policy(
        self,
        use_always_filter: bool,
        expected: list[type[Warning]],
    ) -> None:
        # GIVEN
        package_name = f"thrift_py_deprecated_warning_{uuid.uuid4().hex}"
        generated_module = f"{package_name}.generated"
        importer_module = f"{package_name}.importer"
        generated_source = textwrap.dedent(
            """\
            from thrift.Thrift import warn_thrift_py_deprecated

            warn_thrift_py_deprecated(__name__)
            """
        )
        importer_source = textwrap.dedent(
            f"""\
            import sys

            sys.modules.pop("{generated_module}", None)
            import {generated_module}
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir_str:
            temp_dir = Path(temp_dir_str)
            package_dir = temp_dir / package_name
            package_dir.mkdir()
            (package_dir / "__init__.py").write_text("", encoding="utf-8")
            (package_dir / "generated.py").write_text(
                generated_source, encoding="utf-8"
            )
            (package_dir / "importer.py").write_text(importer_source, encoding="utf-8")

            sys.path.insert(0, temp_dir_str)
            try:
                # WHEN
                with warnings.catch_warnings(record=True) as recorded:
                    warnings.resetwarnings()
                    if use_always_filter:
                        warnings.simplefilter(
                            "always",
                            thrift.Thrift.ThriftPyDeprecatedWarning,
                        )
                    importer = importlib.import_module(importer_module)
                    importlib.reload(importer)
                actual = [warning.category for warning in recorded]

                # THEN
                self.assertEqual(expected, actual)
            finally:
                sys.path.remove(temp_dir_str)
                for module_name in list(sys.modules):
                    if module_name == package_name or module_name.startswith(
                        f"{package_name}."
                    ):
                        del sys.modules[module_name]

    def test_message_matches_expected_format(self) -> None:
        # GIVEN
        module_name = "some.unique.module.name.for.test"
        expected = [
            "some.unique.module.name.for.test uses thrift-py-deprecated. "
            "Migrate to thrift-python. See https://fburl.com/thrift-python "
            "and https://fburl.com/wiki/jihy02dr. Future automatic "
            "thrift-py-deprecated code generation may stop for non-migrated "
            "targets: https://fburl.com/workplace/wer48s4m."
        ]

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.simplefilter("always", thrift.Thrift.ThriftPyDeprecatedWarning)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
        actual = [str(warning.message) for warning in recorded]

        # THEN
        self.assertEqual(expected, actual)

    def test_default_filter_makes_warning_visible_under_interpreter_defaults(
        self,
    ) -> None:
        # GIVEN
        module_name = "foo.bar"
        expected = [thrift.Thrift.ThriftPyDeprecatedWarning]
        # Mimic CPython's startup state: a benign `default` filter for
        # `DeprecationWarning` (which would normally hide it from non-__main__
        # code on the second emission). Our helper must still install its own
        # targeted `default` filter so the *first* warning is surfaced — the
        # interpreter's `default` action does not count as a user choice.

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.resetwarnings()
            warnings.filterwarnings("default", category=DeprecationWarning)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
        actual = [warning.category for warning in recorded]

        # THEN
        self.assertEqual(expected, actual)

    @parameterized.expand(
        [
            (True, thrift.Thrift.ThriftPyDeprecatedWarning),
            (False, None),
            (False, DeprecationWarning),
        ]
    )
    def test_ignore_filter_silences_warning(
        self,
        use_filterwarnings: bool,
        filter_category: type[Warning] | None,
    ) -> None:
        # GIVEN
        module_name = "some.module"
        expected = 0

        # WHEN
        with warnings.catch_warnings(record=True) as caught:
            warnings.resetwarnings()
            if use_filterwarnings:
                assert filter_category is not None
                warnings.filterwarnings("ignore", category=filter_category)
            elif filter_category is None:
                warnings.simplefilter("ignore")
            else:
                warnings.simplefilter("ignore", filter_category)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)

        actual = len(caught)

        # THEN
        self.assertEqual(expected, actual)

    def test_user_error_filter_raises(self) -> None:
        # GIVEN
        module_name = "foo.bar"
        expected_exception = thrift.Thrift.ThriftPyDeprecatedWarning

        # WHEN
        with warnings.catch_warnings():
            warnings.resetwarnings()
            warnings.filterwarnings("error", category=expected_exception)

            # THEN
            with self.assertRaises(expected_exception):
                thrift.Thrift.warn_thrift_py_deprecated(module_name)

    def test_stacklevel_blames_importer_of_generated_module(self) -> None:
        # GIVEN
        package_name = f"thrift_py_deprecated_warning_{uuid.uuid4().hex}"
        generated_module = f"{package_name}.generated"
        importer_module = f"{package_name}.importer"
        expected_warning_count = 1
        expected_importer_filename_match = True
        expected_generated_filename_match = False
        expected_runtime_filename_match = False
        generated_source = textwrap.dedent(
            """\
            from thrift.Thrift import warn_thrift_py_deprecated

            warn_thrift_py_deprecated(__name__)
            """
        )
        importer_source = textwrap.dedent(
            f"""\
            import {generated_module}
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir_str:
            temp_dir = Path(temp_dir_str)
            package_dir = temp_dir / package_name
            package_dir.mkdir()
            (package_dir / "__init__.py").write_text("", encoding="utf-8")
            (package_dir / "generated.py").write_text(
                generated_source, encoding="utf-8"
            )
            (package_dir / "importer.py").write_text(importer_source, encoding="utf-8")

            sys.path.insert(0, temp_dir_str)
            try:
                # WHEN
                with warnings.catch_warnings(record=True) as recorded:
                    warnings.resetwarnings()
                    importlib.import_module(importer_module)
                actual_warning_count = len(recorded)
                actual_importer_filename_match = recorded[0].filename.endswith(
                    "importer.py"
                )
                actual_generated_filename_match = recorded[0].filename.endswith(
                    "generated.py"
                )
                actual_runtime_filename_match = recorded[0].filename.endswith(
                    "Thrift.py"
                )

                # THEN
                self.assertEqual(expected_warning_count, actual_warning_count)
                self.assertEqual(
                    expected_importer_filename_match,
                    actual_importer_filename_match,
                )
                self.assertEqual(
                    expected_generated_filename_match,
                    actual_generated_filename_match,
                )
                self.assertEqual(
                    expected_runtime_filename_match,
                    actual_runtime_filename_match,
                )
            finally:
                sys.path.remove(temp_dir_str)
                for module_name in list(sys.modules):
                    if module_name == package_name or module_name.startswith(
                        f"{package_name}."
                    ):
                        del sys.modules[module_name]
