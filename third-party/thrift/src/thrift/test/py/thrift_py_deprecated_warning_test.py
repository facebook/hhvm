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

_TTYPES_MODULE = "thrift.test.py.PythonReservedKeywords.ttypes"
_CONSTANTS_MODULE = "thrift.test.py.PythonReservedKeywords.constants"
_SERVICE_MODULE = "thrift.test.contextprop_test.py.testing.TestingService"
_THIS_MODULE = "thrift.test.py.thrift_py_deprecated_warning_test"

_REAL_GENERATED_MODULES: list[tuple[str, str]] = [
    ("ttypes", _TTYPES_MODULE),
    ("constants", _CONSTANTS_MODULE),
    ("service", _SERVICE_MODULE),
]


class TestThriftPyDeprecatedWarning(unittest.TestCase):
    def __evict_module(self, module_name: str) -> None:
        sys.modules.pop(module_name, None)
        parts = module_name.rsplit(".", 1)
        if len(parts) == 2:
            parent = sys.modules.get(parts[0])
            if parent is not None and hasattr(parent, parts[1]):
                delattr(parent, parts[1])

    def __evict_real_generated_modules(self) -> None:
        for _, mod in _REAL_GENERATED_MODULES:
            self.__evict_module(mod)

    def __evict_and_reimport(self, module_name: str) -> None:
        self.__evict_module(module_name)
        importlib.import_module(module_name)

    def setUp(self) -> None:
        self.__evict_real_generated_modules()
        globals().pop("__warningregistry__", None)

    def tearDown(self) -> None:
        self.__evict_real_generated_modules()

    @parameterized.expand(_REAL_GENERATED_MODULES)
    def test_warning_emitted_on_generated_module_import(
        self,
        _label: str,
        generated_module: str,
    ) -> None:
        # GIVEN
        expected_min_count = 1

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.resetwarnings()
            importlib.import_module(generated_module)
        actual_count = sum(
            1
            for w in recorded
            if issubclass(w.category, thrift.Thrift.ThriftPyDeprecatedWarning)
        )

        # THEN
        self.assertGreaterEqual(actual_count, expected_min_count)

    @parameterized.expand(_REAL_GENERATED_MODULES)
    def test_warning_attributes_to_handwritten_caller(
        self,
        _label: str,
        generated_module: str,
    ) -> None:
        # GIVEN
        expected_filename = _THIS_MODULE

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.resetwarnings()
            importlib.import_module(generated_module)
        actual_filenames = [
            w.filename
            for w in recorded
            if issubclass(w.category, thrift.Thrift.ThriftPyDeprecatedWarning)
        ]

        # THEN
        self.assertGreaterEqual(len(actual_filenames), 1)
        self.assertEqual(
            [expected_filename] * len(actual_filenames),
            actual_filenames,
        )

    def test_no_warning_when_helper_symbol_is_missing(self) -> None:
        # SEV reference: https://chat.google.com/u/0/app/chat/AAQAckcQ4dM
        # GIVEN
        generated_module = _TTYPES_MODULE
        expected_warning_count = 0

        original_helper = thrift.Thrift.warn_thrift_py_deprecated
        del thrift.Thrift.warn_thrift_py_deprecated
        try:
            # WHEN
            with warnings.catch_warnings(record=True) as recorded:
                warnings.resetwarnings()
                importlib.import_module(generated_module)
            actual_warning_count = len(recorded)
        finally:
            thrift.Thrift.warn_thrift_py_deprecated = original_helper

        # THEN
        self.assertEqual(expected_warning_count, actual_warning_count)

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
        actual = [w.category for w in recorded]

        # THEN
        self.assertEqual(expected, actual)

    def test_message_matches_expected_format(self) -> None:
        # GIVEN
        subject_module_name = "some.unique.generated.module.name.for.test.ttypes"
        expected = [
            "Uses thrift-py-deprecated. "
            "Migrate to thrift-python. See https://fburl.com/thrift-python "
            "and https://fburl.com/wiki/jihy02dr. Future automatic "
            "thrift-py-deprecated code generation may stop for non-migrated "
            "targets: https://fburl.com/workplace/wer48s4m."
        ]

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.simplefilter("always", thrift.Thrift.ThriftPyDeprecatedWarning)
            thrift.Thrift.warn_thrift_py_deprecated(subject_module_name)
        actual = [str(w.message) for w in recorded]

        # THEN
        self.assertEqual(expected, actual)

    def test_default_filter_makes_warning_visible_under_interpreter_defaults(
        self,
    ) -> None:
        # GIVEN
        module_name = "foo.bar"
        expected = [thrift.Thrift.ThriftPyDeprecatedWarning]

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.resetwarnings()
            warnings.filterwarnings("default", category=DeprecationWarning)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
        actual = [w.category for w in recorded]

        # THEN
        self.assertEqual(expected, actual)

    def test_warning_visible_under_cpython_startup_filters(self) -> None:
        # CPython installs these filters at startup:
        #   ('default', None, DeprecationWarning, '__main__', 0)
        #   ('ignore', None, DeprecationWarning, None, 0)
        #   ('ignore', None, PendingDeprecationWarning, None, 0)
        #   ('ignore', None, ImportWarning, None, 0)
        #   ('ignore', None, ResourceWarning, None, 0)
        # The ignore::DeprecationWarning filter must not prevent the
        # helper from surfacing ThriftPyDeprecatedWarning.
        # GIVEN
        module_name = "foo.bar"
        expected = [thrift.Thrift.ThriftPyDeprecatedWarning]

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.resetwarnings()
            warnings.filterwarnings(
                "default", category=DeprecationWarning, module="__main__"
            )
            warnings.filterwarnings("ignore", category=DeprecationWarning)
            warnings.filterwarnings("ignore", category=PendingDeprecationWarning)
            warnings.filterwarnings("ignore", category=ImportWarning)
            warnings.filterwarnings("ignore", category=ResourceWarning)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
        actual = [w.category for w in recorded]

        # THEN
        self.assertEqual(expected, actual)

    @parameterized.expand(
        [
            ("exact_category", thrift.Thrift.ThriftPyDeprecatedWarning),
            ("all_warnings", Warning),
            ("deprecation_superclass", DeprecationWarning),
        ]
    )
    def test_ignore_filter_silences_warning(
        self,
        _label: str,
        filter_category: type[Warning],
    ) -> None:
        # GIVEN
        # User-installed ignore filters take precedence when prepended
        # after the helper's default filter — first-match-wins.
        module_name = "some.module"

        # WHEN
        with warnings.catch_warnings(record=True) as caught:
            warnings.resetwarnings()
            warnings.simplefilter("always", thrift.Thrift.ThriftPyDeprecatedWarning)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
            count_before_ignore = len(caught)
            warnings.filterwarnings("ignore", category=filter_category)
            thrift.Thrift.warn_thrift_py_deprecated(module_name)
            count_after_ignore = len(caught)

        # THEN
        expected_before = 1
        expected_after = 1
        self.assertEqual(expected_before, count_before_ignore)
        self.assertEqual(expected_after, count_after_ignore)

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

    @parameterized.expand(
        [
            ("default_dedup", False),
            ("always_reemits", True),
        ]
    )
    def test_generated_module_reexecution_respects_filter_policy(
        self,
        _label: str,
        use_always_filter: bool,
    ) -> None:
        # GIVEN
        generated_module = _TTYPES_MODULE
        expected_min_first_count = 1

        # WHEN
        with warnings.catch_warnings(record=True) as recorded:
            warnings.resetwarnings()
            if use_always_filter:
                warnings.simplefilter(
                    "always",
                    thrift.Thrift.ThriftPyDeprecatedWarning,
                )
            self.__evict_and_reimport(generated_module)
            first_count = sum(
                1
                for w in recorded
                if issubclass(w.category, thrift.Thrift.ThriftPyDeprecatedWarning)
            )
            self.__evict_and_reimport(generated_module)
            total_count = sum(
                1
                for w in recorded
                if issubclass(w.category, thrift.Thrift.ThriftPyDeprecatedWarning)
            )

        # THEN
        self.assertGreaterEqual(first_count, expected_min_first_count)
        if use_always_filter:
            self.assertLess(first_count, total_count)
        else:
            self.assertEqual(first_count, total_count)

    def test_warning_dedup_under_default_filter_per_handwritten_caller(
        self,
    ) -> None:
        # GIVEN
        package_name = f"thrift_py_deprecated_warning_{uuid.uuid4().hex}"
        first_importer_module = f"{package_name}.first_importer"
        second_importer_module = f"{package_name}.second_importer"
        expected_filenames = [first_importer_module, second_importer_module]
        importer_source_template = textwrap.dedent(
            f"""\
            import sys
            sys.modules.pop("{_TTYPES_MODULE}", None)
            import {_TTYPES_MODULE}  # noqa: F401,E402
            """
        )

        with tempfile.TemporaryDirectory() as temp_dir_str:
            temp_dir = Path(temp_dir_str)
            package_dir = temp_dir / package_name
            package_dir.mkdir()
            (package_dir / "__init__.py").write_text("", encoding="utf-8")
            (package_dir / "first_importer.py").write_text(
                importer_source_template, encoding="utf-8"
            )
            (package_dir / "second_importer.py").write_text(
                importer_source_template, encoding="utf-8"
            )

            sys.path.insert(0, temp_dir_str)
            try:
                self.__evict_module(_TTYPES_MODULE)

                # WHEN
                with warnings.catch_warnings(record=True) as recorded:
                    warnings.resetwarnings()
                    importlib.import_module(first_importer_module)
                    importlib.import_module(second_importer_module)
                actual_filenames = [
                    w.filename
                    for w in recorded
                    if issubclass(w.category, thrift.Thrift.ThriftPyDeprecatedWarning)
                ]

                # THEN
                self.assertEqual(expected_filenames, actual_filenames)
            finally:
                sys.path.remove(temp_dir_str)
                for module_name in list(sys.modules):
                    if module_name == package_name or module_name.startswith(
                        f"{package_name}."
                    ):
                        del sys.modules[module_name]
