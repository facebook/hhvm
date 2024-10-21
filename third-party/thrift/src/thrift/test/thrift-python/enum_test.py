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

import types
import unittest

import thrift.test.thrift_python.enum_test.thrift_mutable_types as mutable_types  # @manual=//thrift/test/thrift-python:enum_test_thrift-python-types
import thrift.test.thrift_python.enum_test.thrift_types as immutable_types

from parameterized import parameterized


class ThriftPython_Enum(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    @parameterized.expand([immutable_types, mutable_types])
    def test_enum_default(self, test_types: types.ModuleType) -> None:
        self.assertEqual(0, test_types.PositiveNumber.NONE)
        self.assertEqual(1, test_types.PositiveNumber.ONE)

        s = test_types.TestStruct()
        self.assertEqual(test_types.PositiveNumber.NONE, s.number)
        self.assertEqual([], s.number_list)
        self.assertEqual(0, test_types.Color.red)
        self.assertEqual(test_types.Color.red, s.color)
        self.assertEqual([], s.color_list)

    @parameterized.expand([immutable_types, mutable_types])
    def test_enum_initialize(self, test_types: types.ModuleType) -> None:
        s = test_types.TestStruct(
            number=test_types.PositiveNumber.TWO,
            number_list=[
                test_types.PositiveNumber.ONE,
                test_types.PositiveNumber.TWO,
                test_types.PositiveNumber.THREE,
            ],
            color=test_types.Color.green,
            color_list=[test_types.Color.blue, test_types.Color.blue],
        )

        self.assertEqual(test_types.PositiveNumber.TWO, s.number)
        self.assertEqual(2, s.number)
        self.assertEqual([1, 2, 3], s.number_list)
        self.assertEqual(test_types.Color.green, s.color)
        self.assertEqual(2, s.color)
        self.assertEqual([test_types.Color.blue, test_types.Color.blue], s.color_list)

    @parameterized.expand([mutable_types])
    def test_enum_update(self, test_types: types.ModuleType) -> None:
        s = test_types.TestStruct()

        self.assertEqual(test_types.PositiveNumber.NONE, s.number)
        s.number = test_types.PositiveNumber.TWO
        self.assertEqual(test_types.PositiveNumber.TWO, s.number)

        with self.assertRaisesRegex(TypeError, "3 is not '.*PositiveNumber'"):
            s.number = 3
        self.assertEqual(test_types.PositiveNumber.TWO, s.number)

        self.assertEqual([], s.number_list)
        s.number_list.append(test_types.PositiveNumber.TWO)
        s.number_list.append(test_types.PositiveNumber.FIVE)
        self.assertEqual(
            [test_types.PositiveNumber.TWO, test_types.PositiveNumber.FIVE],
            s.number_list,
        )

        with self.assertRaisesRegex(TypeError, "Color.green is not '.*PositiveNumber'"):
            s.number_list.append(test_types.Color.green)
        self.assertEqual([2, 5], s.number_list)
