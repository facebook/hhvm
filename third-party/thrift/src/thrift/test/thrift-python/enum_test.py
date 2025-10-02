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

import thrift.test.thrift_python.enum_test.thrift_enums as enums

import thrift.test.thrift_python.enum_test.thrift_mutable_types as mutable_types
import thrift.test.thrift_python.enum_test.thrift_types as immutable_types

from parameterized import parameterized

from thrift.python.mutable_types import to_thrift_list


class ThriftPython_Enums_Test(unittest.TestCase):
    """
    Tests thrift-python enums, as exposed directly by the `thrift_enums` module.
    """

    def test_int_equality(self) -> None:
        self.assertEqual(0, enums.PositiveNumber.NONE)
        self.assertEqual(1, enums.PositiveNumber.ONE)
        self.assertEqual(2, enums.PositiveNumber.TWO)
        self.assertEqual(5, enums.PositiveNumber.FIVE)


class ThriftPython_TypesEnum_Test(unittest.TestCase):
    """
    Tests thrift-python enums exposed via the mutable and immutable type modules
    (i.e., `thrift_mutable_types` and `thrift_types`, respectively).
    """

    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    @parameterized.expand([("immutable", immutable_types), ("mutable", mutable_types)])
    def test_enum_default(self, _name: str, test_types: types.ModuleType) -> None:
        self.assertEqual(0, test_types.PositiveNumber.NONE)
        self.assertEqual(1, test_types.PositiveNumber.ONE)

        s = test_types.TestStruct()
        self.assertEqual(test_types.PositiveNumber.NONE, s.number)
        self.assertEqual([], s.number_list)
        self.assertEqual(0, test_types.Color.red)
        self.assertEqual(test_types.Color.red, s.color)
        self.assertEqual([], s.color_list)

    @parameterized.expand([("immutable", immutable_types), ("mutable", mutable_types)])
    def test_enum_initialize(self, name: str, test_types: types.ModuleType) -> None:
        is_mutable_run: bool = name == "mutable"
        number_list = [
            test_types.PositiveNumber.ONE,
            test_types.PositiveNumber.TWO,
            test_types.PositiveNumber.THREE,
        ]
        color_list = [test_types.Color.blue, test_types.Color.blue]
        s = test_types.TestStruct(
            number=test_types.PositiveNumber.TWO,
            number_list=to_thrift_list(number_list) if is_mutable_run else number_list,
            color=test_types.Color.green,
            color_list=to_thrift_list(color_list) if is_mutable_run else color_list,
        )

        self.assertEqual(test_types.PositiveNumber.TWO, s.number)
        self.assertEqual(2, s.number)
        self.assertEqual([1, 2, 3], s.number_list)
        self.assertEqual(test_types.Color.green, s.color)
        self.assertEqual(2, s.color)
        self.assertEqual([test_types.Color.blue, test_types.Color.blue], s.color_list)

    @parameterized.expand([("mutable", mutable_types)])
    def test_enum_update(self, _name: str, test_types: types.ModuleType) -> None:
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

        error_regex = r"value Color.green is not '<class '.+\.PositiveNumber'>'"
        with self.assertRaisesRegex(TypeError, error_regex):
            s.number_list.append(test_types.Color.green)
        self.assertEqual([2, 5], s.number_list)

    @parameterized.expand([("immutable", immutable_types), ("mutable", mutable_types)])
    def test_enum_identity(self, _name: str, test_types: types.ModuleType) -> None:
        self.assertIs(enums.PositiveNumber, test_types.PositiveNumber)

    @parameterized.expand([("immutable", immutable_types), ("mutable", mutable_types)])
    def test_enum_equality(self, _name: str, test_types: types.ModuleType) -> None:
        self.assertEqual(enums.PositiveNumber.ONE, test_types.PositiveNumber.ONE)
