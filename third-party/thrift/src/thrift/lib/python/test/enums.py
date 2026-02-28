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

import pickle
import sys
import types
import unittest
from enum import Flag as PyFlag, IntEnum as PyIntEnum
from typing import cast, Type, TypeVar

import python_test.enums.thrift_abstract_types as abstract_types
import python_test.enums.thrift_enums as enums_module
import python_test.enums.thrift_mutable_types as mutable_types
import python_test.enums.thrift_types as immutable_types
import testing.dependency.thrift_enums as dependency_enums
import testing.dependency.thrift_types as dependency_types
import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
from parameterized import parameterized_class
from python_test.enums.thrift_abstract_types import Color as AbstractColor
from python_test.enums.thrift_types import (
    BadMembers,
    Color,
    ColorGroups,
    ColorMap,
    File,
    Kind,
    OptionalColorGroups,
    OptionalFile,
    Perm,
)
from python_test.enums_typedef_only.thrift_abstract_types import ColorTypedef
from thrift.python.mutable_types import (
    _ThriftListWrapper,
    _ThriftMapWrapper,
    to_thrift_list,
    to_thrift_map,
)
from thrift.python.types import BadEnum, Enum, Flag


_E = TypeVar("_E", bound=Enum)
ListT = TypeVar("ListT")
MapKey = TypeVar("MapKey")
MapValue = TypeVar("MapValue")


# tt = test_types, ser = serializer
@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class EnumTests(unittest.TestCase):
    def setUp(self) -> None:
        # pyre-ignore[16]: has no attribute `test_types`
        self.BadMembers: Type[BadMembers] = self.test_types.BadMembers
        self.File: Type[File] = self.test_types.File
        self.Kind: Type[Kind] = self.test_types.Kind
        self.OptionalFile: Type[OptionalFile] = self.test_types.OptionalFile
        self.ColorGroups: Type[ColorGroups] = self.test_types.ColorGroups
        self.Color: Type[Color] = self.test_types.Color
        self.Perm: Type[Perm] = self.test_types.Perm
        self.OptionalColorGroups: Type[OptionalColorGroups] = (
            self.test_types.OptionalColorGroups
        )
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def to_map(
        self, map_data: dict[MapKey, MapValue]
    ) -> dict[MapKey, MapValue] | _ThriftMapWrapper:
        return to_thrift_map(map_data) if self.is_mutable_run else map_data

    def test_bad_member_names(self) -> None:
        self.assertIsInstance(self.BadMembers.name_, self.BadMembers)
        self.assertIsInstance(self.BadMembers.value_, self.BadMembers)
        self.assertIn("name_", self.BadMembers.__members__)
        self.assertIn("value_", self.BadMembers.__members__)

    def test_normal_enum(self) -> None:
        with self.assertRaises(TypeError):
            # Enums are not ints
            # pyre-ignore[6]: for tests
            self.File(name="/etc/motd", type=8)
        x = self.File(name="/etc", type=self.Kind.DIR)
        self.assertIsInstance(x.type, self.Kind)
        self.assertEqual(x.type, self.Kind.DIR)
        self.assertNotEqual(x.type, self.Kind.SOCK)
        self.assertNotIsInstance(4, self.Kind, "Ints are not Enums")
        self.assertIsInstance(self.Kind.DIR, int, "Enums are Ints")
        self.assertIn(x.type, self.Kind)
        self.assertEqual(x.type.value, 4)
        self.assertRaises(ValueError, lambda: self.Kind(47))

    def test_enum_value_rename(self) -> None:
        """The value name is None but we auto rename it to None_"""
        x = self.serializer.deserialize(
            self.File, b'{"name":"blah", "type":0}', self.serializer.Protocol.JSON
        )
        self.assertEqual(x.type, self.Kind.None_)

    def test_protocol_int_conversion(self) -> None:
        self.assertEqual(self.serializer.Protocol.BINARY.value, 0)
        self.assertEqual(self.serializer.Protocol.DEPRECATED_VERBOSE_JSON.value, 1)
        self.assertEqual(self.serializer.Protocol.COMPACT.value, 2)
        self.assertEqual(self.serializer.Protocol.JSON.value, 5)

    def test_bad_enum_hash_same(self) -> None:
        x = self.serializer.deserialize(
            self.File,
            b'{"name": "something", "type": 64}',
            self.serializer.Protocol.JSON,
        )
        y = self.serializer.deserialize(
            self.File,
            b'{"name": "something", "type": 64}',
            self.serializer.Protocol.JSON,
        )
        # Mutable types do not support hashing
        if not self.is_mutable_run:
            self.assertEqual(hash(x), hash(y))
        self.assertEqual(hash(x.type), hash(y.type))
        self.assertFalse(x.type is y.type)
        self.assertEqual(x.type, y.type)
        self.assertFalse(x.type != y.type)

    def test_bad_enum_in_struct(self) -> None:
        to_serialize = self.OptionalFile(name="something", type=64)
        serialized = self.serializer.serialize_iobuf(to_serialize)
        x = self.serializer.deserialize(self.File, serialized)
        self.assertBadEnum(cast(BadEnum, x.type), self.Kind, 64)

    def test_bad_enum_in_list_index(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                # pyre-ignore[6]: TODO: Thrift-Container init
                self.OptionalColorGroups(color_list=self.to_list([1, 5, 0]))
            ),
        )
        self.assertEqual(len(x.color_list), 3)
        self.assertEqual(x.color_list[0], self.Color.blue)
        self.assertBadEnum(cast(BadEnum, x.color_list[1]), self.Color, 5)
        self.assertEqual(x.color_list[2], self.Color.red)

    def test_bad_enum_in_list_iter(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                # pyre-ignore[6]: TODO: Thrift-Container init
                self.OptionalColorGroups(color_list=self.to_list([1, 5, 0]))
            ),
        )
        for idx, v in enumerate(x.color_list):
            if idx == 0:
                self.assertEqual(v, self.Color.blue)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), self.Color, 5)
            else:
                self.assertEqual(v, self.Color.red)

    def test_bad_enum_in_list_reverse(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                # pyre-ignore[6]: TODO: Thrift-Container init
                self.OptionalColorGroups(color_list=self.to_list([1, 5, 0]))
            ),
        )
        for idx, v in enumerate(reversed(x.color_list)):
            if idx == 0:
                self.assertEqual(v, self.Color.red)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), self.Color, 5)
            else:
                self.assertEqual(v, self.Color.blue)

    def test_bad_enum_in_set_iter(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                # pyre-ignore[6]: TODO: Thrift-Container init
                self.OptionalColorGroups(color_list=self.to_list([1, 5, 0]))
            ),
        )
        for v in x.color_set:
            if v not in (self.Color.blue, self.Color.red):
                self.assertBadEnum(cast(BadEnum, v), self.Color, 5)

    def test_bad_enum_in_map_lookup(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                self.OptionalColorGroups(
                    # pyre-ignore[6]: TODO: Thrift-Container init
                    color_map=self.to_map({1: 2, 0: 5, 6: 1, 7: 8})
                )
            ),
        )
        val = x.color_map[self.Color.red]
        self.assertBadEnum(cast(BadEnum, val), self.Color, 5)

    def test_bad_enum_in_map_iter(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                self.OptionalColorGroups(
                    # pyre-ignore[6]: TODO: Thrift-Container init
                    color_map=self.to_map({1: 2, 0: 5, 6: 1, 7: 8})
                )
            ),
        )
        s = set()
        for k in x.color_map:
            s.add(k)
        self.assertEqual(len(s), 4)
        s.discard(self.Color.red)
        s.discard(self.Color.blue)
        lst = sorted(s, key=lambda e: cast(BadEnum, e).value)
        self.assertBadEnum(cast(BadEnum, lst[0]), self.Color, 6)
        self.assertBadEnum(cast(BadEnum, lst[1]), self.Color, 7)

    def test_bad_enum_in_map_values(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                self.OptionalColorGroups(
                    # pyre-ignore[6]: TODO: Thrift-Container init
                    color_map=self.to_map({1: 2, 0: 5, 6: 1, 7: 8})
                )
            ),
        )
        s = set()
        for k in x.color_map.values():
            s.add(k)
        self.assertEqual(len(s), 4)
        s.discard(self.Color.green)
        s.discard(self.Color.blue)
        lst = sorted(s, key=lambda e: cast(BadEnum, e).value)
        self.assertBadEnum(cast(BadEnum, lst[0]), self.Color, 5)
        self.assertBadEnum(cast(BadEnum, lst[1]), self.Color, 8)

    def test_bad_enum_in_map_items(self) -> None:
        x = self.serializer.deserialize(
            self.ColorGroups,
            self.serializer.serialize_iobuf(
                self.OptionalColorGroups(
                    # pyre-ignore[6]: TODO: Thrift-Container init
                    color_map=self.to_map({1: 2, 0: 5, 6: 1, 7: 8})
                )
            ),
        )
        for k, v in x.color_map.items():
            if k == self.Color.blue:
                self.assertEqual(v, self.Color.green)
            elif k == self.Color.red:
                self.assertBadEnum(cast(BadEnum, v), self.Color, 5)
            else:
                ck = cast(BadEnum, k)
                if ck.value == 6:
                    self.assertEqual(v, self.Color.blue)
                else:
                    self.assertBadEnum(cast(BadEnum, v), self.Color, 8)

    def assertBadEnum(self, e: BadEnum, cls: Type[_E], val: int) -> None:
        self.assertIsInstance(e, BadEnum)
        self.assertEqual(e.value, val)
        self.assertEqual(e.enum, cls)
        self.assertEqual(int(e), val)

    def test_format(self) -> None:
        self.assertEqual(f"{self.Color.red}", "Color.red")

    def test_bool_of_class(self) -> None:
        self.assertTrue(bool(self.Color))

    def test_bool_of_members(self) -> None:
        self.assertTrue(self.Kind.None_)
        self.assertTrue(self.Color.red)

    def test_pickle(self) -> None:
        serialized = pickle.dumps(self.Color.green)
        green = pickle.loads(serialized)
        self.assertIs(green, self.Color.green)

    def test_adding_member(self) -> None:
        with self.assertRaises(AttributeError):
            # pyre-fixme[16]: `Type` has no attribute `black`.
            self.Color.black = 3

    def test_delete(self) -> None:
        with self.assertRaises(AttributeError):
            del self.Color.red

    def test_changing_member(self) -> None:
        with self.assertRaises(AttributeError):
            # pyre-fixme[41]: Cannot reassign final attribute
            self.Color.red = "lol"

    def test_contains(self) -> None:
        self.assertIn(self.Color.blue, self.Color)
        self.assertIn(1, self.Color)

    def test_equal(self) -> None:
        self.assertEqual(self.Color.blue, self.Color.blue)
        self.assertNotEqual(self.Color.blue, self.Color.green)
        self.assertEqual(self.Color.blue, 1)
        self.assertEqual(2, self.Color.green)
        self.assertNotEqual(self.Color.blue, self.Kind.FIFO)

    def test_hash(self) -> None:
        colors = {}
        colors[self.Color.red] = 0xFF0000
        colors[self.Color.blue] = 0x0000FF
        colors[self.Color.green] = 0x00FF00
        self.assertEqual(colors[self.Color.green], 0x00FF00)
        self.assertTrue(self.Color.blue in colors)
        self.assertTrue(self.Kind.CHAR not in colors)
        self.assertTrue(1 in colors)
        values_to_names = {v.value: v.name for v in self.Color}
        self.assertEqual(values_to_names[self.Color.red], "red")

    def test_enum_in_enum_out(self) -> None:
        self.assertIs(self.Color(self.Color.blue), self.Color.blue)

    def test_enum_value(self) -> None:
        self.assertEqual(self.Color.red.value, 0)

    def test_enum(self) -> None:
        lst = list(self.Color)
        self.assertEqual(len(lst), len(self.Color))
        self.assertEqual(len(self.Color), 5)
        self.assertEqual(
            [
                self.Color.red,
                self.Color.blue,
                self.Color.green,
                self.Color._Color__pleurigloss,
                self.Color._Color__octarine,
            ],
            lst,
        )
        for i, color in enumerate("red blue green".split(), 0):
            e = self.Color(i)
            self.assertEqual(e, getattr(self.Color, color))
            self.assertEqual(e.value, i)
            self.assertEqual(e, i)
            self.assertEqual(e.name, color)
            self.assertIn(e, self.Color)
            self.assertIs(type(e), self.Color)
            self.assertIsInstance(e, self.Color)
            self.assertEqual(str(e), "Color." + color)
            self.assertEqual(int(e), i)
            self.assertEqual(repr(e), f"<Color.{color}: {i}>")

    def test_insinstance_Enum(self) -> None:
        _ = list(self.Color)
        self.assertIsInstance(self.Color.red, Enum)
        self.assertTrue(issubclass(self.Color, Enum))

    def test_callable(self) -> None:
        vals = range(len(self.Color))
        # this is done to verify pyre typestub more than anything
        colors = list(map(self.Color, vals))
        self.assertEqual(colors, list(self.Color))

    def test_class_mangled_color(self) -> None:
        self.assertEqual(self.Color._Color__pleurigloss, 3)
        self.assertEqual(self.Color._Color__octarine, 4)
        self.assertFalse(hasattr(self.Color, "__octarine"))

    def test_map_mangled_color_constant(self) -> None:
        for arm in self.Color:
            arm_name = arm.name.rsplit("__", 1)[1] if "__" in arm.name else arm.name
            # const ColorMap returns first character of arm name
            self.assertEqual(ColorMap[arm], arm_name[0])


# tt = test_types, ser = serializer
@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class FlagTests(unittest.TestCase):
    def setUp(self) -> None:
        # pyre-ignore[16]: has no attribute `test_types`
        self.BadMembers: Type[BadMembers] = self.test_types.BadMembers
        self.File: Type[File] = self.test_types.File
        self.Kind: Type[Kind] = self.test_types.Kind
        self.OptionalFile: Type[OptionalFile] = self.test_types.OptionalFile
        self.ColorGroups: Type[ColorGroups] = self.test_types.ColorGroups
        self.Color: Type[Color] = self.test_types.Color
        self.Perm: Type[Perm] = self.test_types.Perm
        self.OptionalColorGroups: Type[OptionalColorGroups] = (
            self.test_types.OptionalColorGroups
        )
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def test_flag_enum(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for tests
            self.File(name="/etc/motd", permissions=4)
        x = self.File(name="/bin/sh", permissions=self.Perm.read | self.Perm.execute)
        self.assertIsInstance(x.permissions, self.Perm)
        self.assertEqual(x.permissions, self.Perm.read | self.Perm.execute)
        self.assertTrue(x.permissions)
        self.assertNotIsInstance(2, self.Perm, "Flags are not ints")
        self.assertEqual(int(x.permissions), 5)
        self.assertEqual(x.permissions.value, 5)
        x = self.File(name="")
        self.assertFalse(x.permissions)
        self.assertIsInstance(x.permissions, self.Perm)
        self.assertEqual(f"{self.Perm.read}", "Perm.read")
        self.assertTrue(self.Perm.read in self.Perm.read | self.Perm.execute)

    def test_flag_enum_serialization_roundtrip(self) -> None:
        x = self.File(
            name="/dev/null",
            type=self.Kind.CHAR,
            permissions=self.Perm.read | self.Perm.write,
        )

        y = self.serializer.deserialize(self.File, self.serializer.serialize_iobuf(x))
        self.assertEqual(x, y)
        self.assertEqual(x.permissions, self.Perm.read | self.Perm.write)
        self.assertIsInstance(x.permissions, self.Perm)

    def test_zero(self) -> None:
        zero = self.Perm(0)
        # This is stdlib Python Behavior
        self.assertIn(zero, self.Perm)
        self.assertIsInstance(zero, self.Perm)

    def test_logical(self) -> None:
        self.assertEqual(self.Perm.read & self.Perm.write, self.Perm(0))
        self.assertEqual(self.Perm.read ^ self.Perm.write, self.Perm(6))
        self.assertEqual(~self.Perm.read, self.Perm(3))

    def test_combination(self) -> None:
        combo = self.Perm(self.Perm.read.value | self.Perm.execute.value)
        self.assertEqual(combo, self.Perm.read.value + self.Perm.execute.value)
        # This is stdlib Python Behavior
        self.assertIn(combo, self.Perm)
        self.assertIsInstance(combo, self.Perm)
        self.assertIs(combo, self.Perm.read | self.Perm.execute)

        # make sure it works when creating a combo in a struct
        x = self.File(name="/bin/sh", permissions=(self.Perm.read | self.Perm.execute))
        self.assertEqual(x.permissions, self.Perm.read.value + self.Perm.execute.value)
        self.assertEqual(x.permissions, 5)
        # This is stdlib Python Behavior
        self.assertIn(x.permissions, self.Perm)
        self.assertIsInstance(x.permissions, self.Perm)
        self.assertIs(x.permissions, self.Perm.read | self.Perm.execute)

    def test_is(self) -> None:
        allp = self.Perm(7)
        self.assertIs(allp, self.Perm(7))

    def test_invert(self) -> None:
        x = self.Perm(-2)
        self.assertIs(x, self.Perm.read | self.Perm.write)

    def test_insinstance_Flag(self) -> None:
        self.assertIsInstance(self.Perm.read, Flag)
        self.assertTrue(issubclass(self.Perm, Flag))
        self.assertIsInstance(self.Perm.read, Enum)
        self.assertTrue(issubclass(self.Perm, Enum))

    def test_combo_in_call(self) -> None:
        x = self.Perm(7)
        self.assertIs(x, self.Perm.read | self.Perm.write | self.Perm.execute)

    def test_combo_repr(self) -> None:
        x = self.Perm(7)
        self.assertEqual("<Perm.read|write|execute: 7>", repr(x))


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class EnumMetaTests(unittest.TestCase):
    def setUp(self) -> None:
        # pyre-ignore[16]: has no attribute `test_types`
        self.Color: Type[Color] = self.test_types.Color
        self.Kind: Type[Kind] = self.test_types.Kind
        self.Perm: Type[Perm] = self.test_types.Perm
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )

    def test_enum_metaclass_contains(self) -> None:
        self.assertIn(self.Color.red, self.Color)
        self.assertIn(self.Color.blue, self.Color)
        self.assertIn(self.Color.green, self.Color)
        self.assertNotIn("red", self.Color)
        self.assertNotIn(self.Perm.read, self.Color)

        self.assertNotIn(-1, self.Color)
        self.assertNotIn(len(Color), self.Color)

        # this is more lenient behavior than thrift-py3
        self.assertIn(0, self.Color)
        self.assertIn(1, self.Color)
        self.assertIn(2, self.Color)

    def test_enum_metaclass_dir(self) -> None:
        attrs = set(dir(self.Color))
        self.assertEqual(len(self.Color), 5)
        self.assertEqual(len(attrs), 4 + len(self.Color))
        self.assertIn("red", attrs)
        self.assertIn("blue", attrs)
        self.assertIn("green", attrs)
        self.assertIn("__class__", attrs)
        self.assertIn("__doc__", attrs)
        self.assertIn("__members__", attrs)
        self.assertIn("__module__", attrs)

    def test_changing_member(self) -> None:
        with self.assertRaises(AttributeError):
            # pyre-fixme[41]: Cannot reassign final attribute
            self.Color.red = "lol"

    def test_delete(self) -> None:
        with self.assertRaises(AttributeError):
            del self.Color.red


class PyColor(PyIntEnum):
    red = 0
    blue = 1
    green = 2


class OtherPyColor(PyIntEnum):
    red = 0
    blue = 1
    green = 2


class PyPerm(PyFlag):
    read = 4
    write = 2
    execute = 1


class TestWithStdlibEnums(unittest.TestCase):
    def test_enum_behavior_matrix(self) -> None:
        if sys.version_info < (3, 12):
            # Testing Python 3.12 behavior only
            return
        for y in (PyColor, Color, OtherPyColor):
            for x in (PyColor, Color, OtherPyColor):
                with self.subTest(x=x, y=y):
                    self.assertIsInstance(x.red, int)
                    self.assertIn(x.red, y)
                    self.assertIn(0, x)
                    # A thrift-python flag can be found in a pure python enum
                    # Because they directly convert to ints and python flags do not.
                    # self.assertNotIn(Perm.write, x)
                    self.assertNotIn(PyPerm.write, x)

    def test_flag_behavior_matrix(self) -> None:
        if sys.version_info < (3, 12):
            # Testing Python 3.12 behavior only
            return
        for x in (PyPerm, Perm):
            with self.subTest(x=x):
                # pyre is being really weird here, I can't ignore these because it says the ignores are unused
                # But this cast makes pyre happy.
                x = cast(Type[Perm], x)
                combined = x.read | x.write
                self.assertIn(combined, x)
                self.assertIn(combined.value, x)
                self.assertIn(0, x)
                self.assertIn(x(0), x)


@parameterized_class(
    ("test_types",),
    [
        (abstract_types,),
        (immutable_types,),
        (mutable_types,),
    ],
)
class TypedefedEnumsTest(unittest.TestCase):
    """
    Tests that enum typedefs are available from the correct modules.
    """

    def setUp(self) -> None:
        # pyre-ignore[16]: has no attribute `test_types`
        self.Color: Type[AbstractColor] = self.test_types.Color
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )

    def test_enum_typedef_available_from_thrift_types(self) -> None:
        # GIVEN / WHEN
        # pyre-ignore[16]: has no attribute `ColorTypedef`
        typedef = self.test_types.ColorTypedef

        # THEN
        self.assertIs(typedef, self.Color)

    def test_enum_typedef_available_from_enums(self) -> None:
        # GIVEN / WHEN
        typedef = enums_module.ColorTypedef

        # THEN
        self.assertIs(typedef, enums_module.Color)

    def test_cross_file_enum_typedef_available_from_thrift_types(self) -> None:
        # GIVEN / WHEN
        # pyre-ignore[16]: has no attribute `StatusTypedef`
        typedef = self.test_types.StatusTypedef

        # THEN
        self.assertIs(typedef, dependency_types.Status)

    def test_cross_file_enum_typedef_available_from_enums(self) -> None:
        # GIVEN / WHEN
        typedef = enums_module.StatusTypedef

        # THEN
        self.assertIs(typedef, dependency_enums.Status)

    def test_typedef_of_typedef_available_from_enums(self) -> None:
        # GIVEN / WHEN
        typedef = enums_module.ColourTypedefOfTypedef

        # THEN
        # Should resolve to IncludedColour from sub_dependency
        self.assertIs(typedef, dependency_types.ColourAlias)

    def test_enum_typedef_only(self) -> None:
        """Test that enum typedefs can be imported from thrift_abstract_types.

        A thrift file with enum typedefs must generate valid Python import syntax,
        even when the file contains no direct enum definitions.
        """
        # THEN
        self.assertIs(ColorTypedef, enums_module.Color)
