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
import typing
import unittest

import thrift.test.thrift_python.enum_test.thrift_enums as enums

import thrift.test.thrift_python.enum_test.thrift_mutable_types as mutable_types
import thrift.test.thrift_python.enum_test.thrift_types as immutable_types

from parameterized import parameterized

from thrift.python.mutable_types import to_thrift_list


class ThriftPython_Enum(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_type_check_with_transitive_import(self) -> None:
        # This test only exists to ensure that type-check
        # on the code below succeeds.
        # ==================================================================================
        # The only approach so far that seems succeed type-check
        # is when thrift_types.pyi and thrift_mutable_types.pyi
        # import PositiveNumber from .thrift_enums like below
        #
        # from thrift.test.thrift_python.enum_test.thrift_enums import *
        # ===================================================================================
        # When thrift_types.pyi and thrift_mutable_types.pyi
        # import PositiveNumber from .thrift_enums like
        #
        # from thrift.test.thrift_python.enum_test.thrift_enums import _fbthrift_PositiveNumber
        # PositiveNumber = _fbthrift_PositiveNumber
        #
        # or like
        #
        # from thrift.test.thrift_python.enum_test.thrift_enums import PositiveNumber
        # from thrift.test.thrift_python.enum_test.thrift_enums import _fbthrift_PositiveNumber
        #
        # The code below fails type-check with the errors
        #
        # Invalid type [31]: Expression `immutable_types.PositiveNumber.TWO` is not a literal value.
        # Invalid type [31]: Expression `immutable_types.PositiveNumber.ONE` is not a literal value.
        # Undefined or invalid type [11]: Annotation `immutable_types.PositiveNumber` is not defined as a type.
        #
        # NOTES:
        #   - The latter approach above also causes the type-check error below at any call-site
        #   that references PositiveNumber. This comment only here because the context of this
        #   test is specifically for these type-check failures.
        #
        #   Undefined attribute [16]: Module `thrift.test.thrift_python.enum_test.thrift_types` has no attribute `PositiveNumber`.
        #
        #   - The error happens irrespective of whether the call-site uses immutable or mutable
        #   types. It also happens if parameterized on both. To avoid any question of whether
        #   the failure happens because of parameterization and for the sake of brevity,
        #   this test uses only immutable_types.
        # ==================================================================================
        def literal_return_type(
            choice: int,
        ) -> typing.Literal[
            None,
            immutable_types.PositiveNumber.ONE,
            immutable_types.PositiveNumber.TWO,
        ]:
            match choice:
                case 1:
                    return immutable_types.PositiveNumber.ONE
                case 2:
                    return immutable_types.PositiveNumber.TWO
                case _:
                    return None

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
        is_mutable_run: bool = test_types.__name__.endswith("mutable_types")
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

    def test_enum_identity(self) -> None:
        self.assertIs(enums.PositiveNumber, immutable_types.PositiveNumber)
        self.assertIs(enums.PositiveNumber, mutable_types.PositiveNumber)
        self.assertIs(immutable_types.PositiveNumber, mutable_types.PositiveNumber)

    def test_value(self) -> None:
        self.assertEqual(0, enums.PositiveNumber.NONE)
        self.assertEqual(1, enums.PositiveNumber.ONE)
        self.assertEqual(2, enums.PositiveNumber.TWO)
        self.assertEqual(5, enums.PositiveNumber.FIVE)

        self.assertEqual(
            immutable_types.PositiveNumber.NONE, mutable_types.PositiveNumber.NONE
        )
        self.assertEqual(enums.PositiveNumber.ONE, mutable_types.PositiveNumber.ONE)
        self.assertEqual(enums.PositiveNumber.TWO, immutable_types.PositiveNumber.TWO)
