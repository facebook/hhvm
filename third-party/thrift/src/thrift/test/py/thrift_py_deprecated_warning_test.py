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
import unittest
import warnings

import thrift.Thrift

_TTYPES_MODULE = "thrift.test.py.PythonReservedKeywords.ttypes"


class TestThriftPyDeprecatedWarning(unittest.TestCase):
    def __evict_module(self, module_name: str) -> None:
        sys.modules.pop(module_name, None)
        parts = module_name.rsplit(".", 1)
        if len(parts) == 2:
            parent = sys.modules.get(parts[0])
            if parent is not None and hasattr(parent, parts[1]):
                delattr(parent, parts[1])

    def setUp(self) -> None:
        self.__evict_module(_TTYPES_MODULE)
        globals().pop("__warningregistry__", None)

    def tearDown(self) -> None:
        self.__evict_module(_TTYPES_MODULE)

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
