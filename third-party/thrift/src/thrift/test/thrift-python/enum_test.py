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


class ThriftPython_EnumClass_Test(unittest.TestCase):
    """
    Tests thrift-python enum classes, as exposed directly by the `thrift_enums` module.

    This test covers method defined on the Enum class itself. For operations on the
    enum instances (i.e., members), which are the attributes of the Enum class, see
    below.
    """

    def test_members(self) -> None:
        """
        Enum classes have an attribute for each enum member, whose type is the enum
        class itself.
        """
        self.assertIsInstance(enums.Color.red, enums.Color)
        self.assertIsInstance(enums.Color.red, int)
        self.assertIs(type(enums.Color.red), enums.Color)

    def test_dunder_members(self) -> None:
        """
        Enum classes have a (officially supported) __members__attribute, which maps
        enum names to the corresponding member.

        See: https://docs.python.org/3/library/enum.html#supported-dunder-names
        """
        self.assertIs(enums.Color.__members__["red"], enums.Color.red)
        self.assertEqual(len(enums.Color.__members__), 3)

        with self.assertRaisesRegex(
            TypeError, "'mappingproxy' object does not support item assignment"
        ):
            enums.Color.__members__["foobar"] = None  # pyre-ignore[16]

        with self.assertRaisesRegex(
            TypeError, "'mappingproxy' object does not support item deletion"
        ):
            del enums.Color.__members__["red"]

    def test_getitem(self) -> None:
        """
        Enum classes can be indexed by enum name, but not their (integer) value.
        """

        self.assertIs(enums.Color["red"], enums.Color.red)

        # They cannot be indexed by enum value
        with self.assertRaises(KeyError):
            enums.Color[0]  # pyre-ignore[6]: Check invalid argument type on purpose.

    def test_contains(self) -> None:
        """
        Enum classes "contains" both enum members and enum (integer) values, but not
        their names.
        """
        self.assertIn(enums.Color.red, enums.Color)
        self.assertIn(0, enums.Color)
        self.assertNotIn(42, enums.Color)
        self.assertNotIn("red", enums.Color)

        # Under "normal" (non-Int) Python Enum semantics, a given Enum type cannot
        # contain a member of a different Enum type. For IntEnum types however, every
        # enum member is also a plain integer, which may be contained in another Enum
        # type.. Thrift enums follow IntEnum semantics, so the following test must pass.
        self.assertIn(enums.PositiveNumber.ONE, enums.Color)

    def test_len(self) -> None:
        """
        The length of an enum class is the number of enum members it contains.
        """
        self.assertEqual(len(enums.Color), 3)
        self.assertEqual(len(enums.PositiveNumber), 6)

    def test_iter(self) -> None:
        """
        Iterating over an enum class returns the enum members.
        """
        it = iter(enums.Color)
        self.assertIs(iter(it), it)

        self.assertIs(next(it), enums.Color.red)
        self.assertIs(next(it), enums.Color.blue)
        self.assertIs(next(it), enums.Color.green)

        with self.assertRaises(StopIteration):
            next(it)

        self.assertEqual(len(list(enums.Color)), 3)
        self.assertEqual(len(set(enums.Color)), 3)

    def test_reversed(self) -> None:
        reversed_it = reversed(enums.Color)
        self.assertIs(iter(reversed_it), reversed_it)

        self.assertIs(next(reversed_it), enums.Color.green)
        self.assertIs(next(reversed_it), enums.Color.blue)
        self.assertIs(next(reversed_it), enums.Color.red)

        with self.assertRaises(StopIteration):
            next(reversed_it)

    def test_set_or_del_attr(self) -> None:
        """
        Enum class attributes cannot be modified (set or deleted).
        """

        with self.assertRaisesRegex(
            AttributeError, r"Thrift enum type 'Color': cannot assign member \('foo'\)"
        ):
            enums.Color.foo = 42  # pyre-ignore[16]: Check runtime behavior

        with self.assertRaisesRegex(
            AttributeError, r"Thrift enum type 'Color': cannot assign member \('red'\)"
        ):
            enums.Color.red = 42  # pyre-ignore[41]: Check behavior despite final hint

        with self.assertRaisesRegex(
            AttributeError, r"Thrift enum type 'Color': cannot delete member \('red'\)"
        ):
            del enums.Color.red

    def test_call(self) -> None:
        """
        The enum class "call" operator (similar to object instatiation) actually returns
        enum members given their (integer) value.
        """
        self.assertIs(enums.Color(0), enums.Color.red)
        self.assertIs(enums.Color(1), enums.Color.blue)
        self.assertIs(enums.Color(enums.Color.red), enums.Color.red)

        # For reference, corresponding native Python error looks like:
        # "ValueError: 42 is not a valid Color"
        with self.assertRaisesRegex(
            ValueError, "Enum type Color has no attribute with value 42"
        ):
            enums.Color(42)

        # Similarly: "ValueError: 'BLUE' is not a valid Color"
        with self.assertRaisesRegex(
            ValueError, "Enum type Color has no attribute with value 'BLUE'"
        ):
            # pyre-ignore[6]: pass invalid argument (str) on purpose
            enums.Color("BLUE")

    def test_dir(self) -> None:
        self.assertIn("red", dir(enums.Color))
        self.assertIn("blue", dir(enums.Color))
        self.assertIn("green", dir(enums.Color))

        # Despite being a "dunder" field, __members__ is part of the public API and
        # typically the idiomatic way to check for the existence of an enum name.
        # (see https://docs.python.org/3/library/enum.html#supported-dunder-names).
        self.assertIn("__members__", dir(enums.Color))

    def test_get_thrift_name(self) -> None:
        self.assertEqual(enums.Color.__get_thrift_name__(), "enum_test.Color")

    def test_get_thrift_uri(self) -> None:
        self.assertEqual(
            enums.Color.__get_thrift_uri__(), "facebook.com/thrift/test/python/Color"
        )

    def test_get_metadata(self) -> None:
        self.assertIsNotNone(enums.Color.__get_metadata__())

    def test_subclass(self) -> None:
        # BAD: Subclassing a Thrift enum class should not be allowed. All of the
        # following should fail.
        sub_class = type("SubClass", (enums.Color,), {})
        self.assertTrue(issubclass(sub_class, enums.Color))
        self.assertIs(type(sub_class), type(enums.Color))
        self.assertIn("__members__", dir(sub_class))


class ThriftPython_EnumMembers_Test(unittest.TestCase):
    """
    Tests thrift-python enum members, as exposed directly by the `thrift_enums` module.
    """

    def test_int_equality(self) -> None:
        """
        Enum members are integers, and can be compared to other integers. However,
        comparing enum members of different types always returns False, even if their
        values are the same.
        """
        self.assertEqual(enums.Color.red, 0)
        self.assertEqual(0, enums.Color.red)
        self.assertEqual(enums.Color["red"], 0)

        self.assertNotEqual(enums.Color.green, 0)
        self.assertNotEqual(0, enums.Color.green)
        self.assertNotEqual(enums.Color["green"], 0)

        self.assertEqual(enums.PositiveNumber.NONE, 0)
        self.assertNotEqual(enums.Color.red, enums.PositiveNumber.NONE)

    def test_name_and_value(self) -> None:
        self.assertEqual(enums.Color.red.name, "red")
        self.assertEqual(enums.Color.red.value, 0)
        self.assertEqual(enums.Color.green.name, "green")
        self.assertEqual(enums.Color.green.value, 2)

    def test_hashable(self) -> None:
        """
        Enum members are hashable. They can be used as map keys and set members.
        """
        hash(enums.Color.red)
        self.assertEqual(
            len({enums.Color.red, enums.Color.green, enums.Color["green"]}), 2
        )

    def test_str(self) -> None:
        self.assertEqual(str(enums.Color.red), "Color.red")

    def test_repr(self) -> None:
        self.assertEqual(repr(enums.Color.red), "<Color.red: 0>")

    def test_thrift_converters(self) -> None:
        self.assertIs(enums.Color.red._to_python(), enums.Color.red)
        self.assertIs(enums.Color.red._to_py3(), enums.Color.red)

        py_deprecated_enum = enums.Color.red._to_py_deprecated()
        self.assertIs(type(py_deprecated_enum), int)
        self.assertEqual(py_deprecated_enum, 0)
        self.assertEqual(py_deprecated_enum, enums.Color.red.value)
        self.assertEqual(py_deprecated_enum, enums.Color.red)

    def test_dir(self) -> None:
        self.assertIn("name", dir(enums.Color.red))
        self.assertIn("value", dir(enums.Color.red))

        # Python doc explicitly states that __members__ is only available on the Enum
        # *class* (not enum members).
        self.assertNotIn("__members__", dir(enums.Color.red))


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
