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

import unittest

import testing.types as _types
from testing.builders import (
    ColorGroups_Builder,
    Digits_Builder,
    easy_Builder,
    File_Builder,
    HardError_Builder,
    Integers_Builder,
    numerical_Builder,
    Reserved_Builder,
    ValueOrError_Builder,
)


class BuilderTest(unittest.TestCase):
    def test_defaults(self) -> None:
        easy_builder = easy_Builder()
        self.assertIsNone(easy_builder.val)
        self.assertIsNone(easy_builder.val_list)
        self.assertIsNone(easy_builder.name)
        self.assertIsNone(easy_builder.an_int)
        file_builder = File_Builder()
        self.assertIsNone(file_builder.name)
        self.assertIsNone(file_builder.permissions)
        self.assertIsNone(file_builder.type)
        numerical_builder = numerical_Builder()
        self.assertIsNone(numerical_builder.int_val)
        self.assertIsNone(numerical_builder.float_val)
        self.assertIsNone(numerical_builder.int_list)
        self.assertIsNone(numerical_builder.float_list)
        self.assertIsNone(numerical_builder.i64_val)

    def test_build(self) -> None:
        easy_builder = easy_Builder()
        easy_builder.val = 42
        easy_builder.val_list = [123, 456]
        easy_builder.name = "foo"
        easy_builder.an_int = _types.Integers.fromValue(123)
        easy_obj = easy_builder()
        self.assertIsInstance(easy_obj, _types.easy)
        self.assertEqual(easy_obj.val, 42)
        self.assertEqual(easy_obj.val_list, [123, 456])
        self.assertEqual(easy_obj.name, "foo")
        self.assertEqual(easy_obj.an_int.value, 123)
        error_builder = HardError_Builder()
        error_builder.errortext = "oh no!"
        error_builder.code = 404
        error_obj = error_builder()
        self.assertIsInstance(error_obj, _types.HardError)
        self.assertEqual(error_obj.errortext, "oh no!")
        self.assertEqual(error_obj.code, 404)

    def test_build_with_builder_fields(self) -> None:
        easy_builder = easy_Builder()
        integers_builder = Integers_Builder()
        digits_builder = Digits_Builder()
        digits_builder.data = [
            _types.Integers.fromValue(123),
            _types.Integers.fromValue(456),
        ]
        integers_builder.digits = digits_builder
        easy_builder.an_int = integers_builder
        easy_obj = easy_builder()
        self.assertIsInstance(easy_obj, _types.easy)
        an_int_value = easy_obj.an_int.value
        self.assertIsInstance(an_int_value, _types.Digits)
        self.assertEqual(len(an_int_value.data), 2)
        v1, v2 = an_int_value.data
        self.assertEqual(v1.value, 123)
        self.assertEqual(v2.value, 456)

    def test_build_with_wrong_type_field(self) -> None:
        easy_builder = easy_Builder()
        with self.assertRaises(TypeError):
            # pyre-fixme[8]: Attribute has type `Optional[int]`; used as `str`.
            easy_builder.val = "123"
        easy_builder = easy_Builder()
        easy_builder.val_list = [
            "123",
            456,
        ]  # OK now as builder doesn't check list element types
        with self.assertRaises(TypeError):
            easy_builder()  # caught at build step
        value_or_error_builder = ValueOrError_Builder()
        value_or_error_builder.error = (
            "wrong!"  # OK now as builder takes any type for struct field
        )
        with self.assertRaises(TypeError):
            value_or_error_builder()  # caught at build step

    def test_reserved_names(self) -> None:
        builder = Reserved_Builder()
        builder.from_ = "from"
        builder.nonlocal_ = 42
        builder.is_cpdef = True
        obj = builder()
        self.assertEqual(obj.from_, "from")
        self.assertEqual(obj.nonlocal_, 42)
        self.assertTrue(obj.is_cpdef)

    def test_build_map_field(self) -> None:
        builder = ColorGroups_Builder()
        builder.color_map = {_types.Color.red: _types.Color.blue}
        obj = builder()
        self.assertEqual(obj.color_map[_types.Color.red], _types.Color.blue)
