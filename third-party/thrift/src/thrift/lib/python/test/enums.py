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
# pyre-ignore-all-errors[16]: `EnumTests` has not attribute `tt`


from __future__ import annotations

import pickle
import unittest
from typing import cast, Type, TypeVar

import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
import thrift.python.test.enums.thrift_mutable_types as mutable_types
import thrift.python.test.enums.thrift_types as immutable_types

from parameterized import parameterized_class

from thrift.python.types import BadEnum, Enum, Flag


_E = TypeVar("_E", bound=Enum)


# tt = test_types, ser = serializer
@parameterized_class(
    ("tt", "ser"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class EnumTests(unittest.TestCase):
    def test_bad_member_names(self) -> None:
        self.assertIsInstance(self.tt.BadMembers.name_, self.tt.BadMembers)
        self.assertIsInstance(self.tt.BadMembers.value_, self.tt.BadMembers)
        self.assertIn("name_", self.tt.BadMembers.__members__)
        self.assertIn("value_", self.tt.BadMembers.__members__)

    def test_normal_enum(self) -> None:
        with self.assertRaises(TypeError):
            # Enums are not ints
            self.tt.File(name="/etc/motd", type=8)
        x = self.tt.File(name="/etc", type=self.tt.Kind.DIR)
        self.assertIsInstance(x.type, self.tt.Kind)
        self.assertEqual(x.type, self.tt.Kind.DIR)
        self.assertNotEqual(x.type, self.tt.Kind.SOCK)
        self.assertNotIsInstance(4, self.tt.Kind, "Ints are not Enums")
        self.assertIsInstance(self.tt.Kind.DIR, int, "Enums are Ints")
        self.assertIn(x.type, self.tt.Kind)
        self.assertEqual(x.type.value, 4)

    def test_enum_value_rename(self) -> None:
        """The value name is None but we auto rename it to None_"""
        x = self.ser.deserialize(
            self.tt.File, b'{"name":"blah", "type":0}', self.ser.Protocol.JSON
        )
        self.assertEqual(x.type, self.tt.Kind.None_)

    def test_protocol_int_conversion(self) -> None:
        self.assertEqual(self.ser.Protocol.BINARY.value, 0)
        self.assertEqual(self.ser.Protocol.DEPRECATED_VERBOSE_JSON.value, 1)
        self.assertEqual(self.ser.Protocol.COMPACT.value, 2)
        self.assertEqual(self.ser.Protocol.JSON.value, 5)

    def test_bad_enum_hash_same(self) -> None:
        x = self.ser.deserialize(
            self.tt.File,
            b'{"name": "something", "type": 64}',
            self.ser.Protocol.JSON,
        )
        y = self.ser.deserialize(
            self.tt.File, b'{"name": "something", "type": 64}', self.ser.Protocol.JSON
        )
        # Mutable types do not support hashing
        if self.tt.__name__.endswith("immutable_types"):
            self.assertEqual(hash(x), hash(y))
        self.assertEqual(hash(x.type), hash(y.type))
        self.assertFalse(x.type is y.type)
        self.assertEqual(x.type, y.type)
        self.assertFalse(x.type != y.type)

    def test_bad_enum_in_struct(self) -> None:
        to_serialize = self.tt.OptionalFile(name="something", type=64)
        serialized = self.ser.serialize_iobuf(to_serialize)
        x = self.ser.deserialize(self.tt.File, serialized)
        self.assertBadEnum(cast(BadEnum, x.type), self.tt.Kind, 64)

    def test_bad_enum_in_list_index(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(self.tt.OptionalColorGroups(color_list=[1, 5, 0])),
        )
        self.assertEqual(len(x.color_list), 3)
        self.assertEqual(x.color_list[0], self.tt.Color.blue)
        self.assertBadEnum(cast(BadEnum, x.color_list[1]), self.tt.Color, 5)
        self.assertEqual(x.color_list[2], self.tt.Color.red)

    def test_bad_enum_in_list_iter(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(self.tt.OptionalColorGroups(color_list=[1, 5, 0])),
        )
        for idx, v in enumerate(x.color_list):
            if idx == 0:
                self.assertEqual(v, self.tt.Color.blue)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), self.tt.Color, 5)
            else:
                self.assertEqual(v, self.tt.Color.red)

    def test_bad_enum_in_list_reverse(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(self.tt.OptionalColorGroups(color_list=[1, 5, 0])),
        )
        for idx, v in enumerate(reversed(x.color_list)):
            if idx == 0:
                self.assertEqual(v, self.tt.Color.red)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), self.tt.Color, 5)
            else:
                self.assertEqual(v, self.tt.Color.blue)

    def test_bad_enum_in_set_iter(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(self.tt.OptionalColorGroups(color_list=[1, 5, 0])),
        )
        for v in x.color_set:
            if v not in (self.tt.Color.blue, self.tt.Color.red):
                self.assertBadEnum(cast(BadEnum, v), self.tt.Color, 5)

    def test_bad_enum_in_map_lookup(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(
                self.tt.OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})
            ),
        )
        val = x.color_map[self.tt.Color.red]
        self.assertBadEnum(cast(BadEnum, val), self.tt.Color, 5)

    def test_bad_enum_in_map_iter(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(
                self.tt.OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})
            ),
        )
        s = set()
        for k in x.color_map:
            s.add(k)
        self.assertEqual(len(s), 4)
        s.discard(self.tt.Color.red)
        s.discard(self.tt.Color.blue)
        lst = sorted(s, key=lambda e: cast(BadEnum, e).value)
        self.assertBadEnum(cast(BadEnum, lst[0]), self.tt.Color, 6)
        self.assertBadEnum(cast(BadEnum, lst[1]), self.tt.Color, 7)

    def test_bad_enum_in_map_values(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(
                self.tt.OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})
            ),
        )
        s = set()
        for k in x.color_map.values():
            s.add(k)
        self.assertEqual(len(s), 4)
        s.discard(self.tt.Color.green)
        s.discard(self.tt.Color.blue)
        lst = sorted(s, key=lambda e: cast(BadEnum, e).value)
        self.assertBadEnum(cast(BadEnum, lst[0]), self.tt.Color, 5)
        self.assertBadEnum(cast(BadEnum, lst[1]), self.tt.Color, 8)

    def test_bad_enum_in_map_items(self) -> None:
        x = self.ser.deserialize(
            self.tt.ColorGroups,
            self.ser.serialize_iobuf(
                self.tt.OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})
            ),
        )
        for k, v in x.color_map.items():
            if k == self.tt.Color.blue:
                self.assertEqual(v, self.tt.Color.green)
            elif k == self.tt.Color.red:
                self.assertBadEnum(cast(BadEnum, v), self.tt.Color, 5)
            else:
                ck = cast(BadEnum, k)
                if ck.value == 6:
                    self.assertEqual(v, self.tt.Color.blue)
                else:
                    self.assertBadEnum(cast(BadEnum, v), self.tt.Color, 8)

    def assertBadEnum(self, e: BadEnum, cls: Type[_E], val: int) -> None:
        self.assertIsInstance(e, BadEnum)
        self.assertEqual(e.value, val)
        self.assertEqual(e.enum, cls)
        self.assertEqual(int(e), val)

    def test_format(self) -> None:
        self.assertEqual(f"{self.tt.Color.red}", "Color.red")

    def test_bool_of_class(self) -> None:
        self.assertTrue(bool(self.tt.Color))

    def test_bool_of_members(self) -> None:
        self.assertTrue(self.tt.Kind.None_)
        self.assertTrue(self.tt.Color.red)

    def test_pickle(self) -> None:
        serialized = pickle.dumps(self.tt.Color.green)
        green = pickle.loads(serialized)
        self.assertIs(green, self.tt.Color.green)

    def test_adding_member(self) -> None:
        with self.assertRaises(AttributeError):
            self.tt.Color.black = 3

    def test_delete(self) -> None:
        with self.assertRaises(AttributeError):
            del self.tt.Color.red

    def test_changing_member(self) -> None:
        with self.assertRaises(AttributeError):
            self.tt.Color.red = "lol"

    def test_contains(self) -> None:
        self.assertIn(self.tt.Color.blue, self.tt.Color)
        self.assertIn(1, self.tt.Color)

    def test_equal(self) -> None:
        self.assertEqual(self.tt.Color.blue, self.tt.Color.blue)
        self.assertNotEqual(self.tt.Color.blue, self.tt.Color.green)
        self.assertEqual(self.tt.Color.blue, 1)
        self.assertEqual(2, self.tt.Color.green)
        self.assertNotEqual(self.tt.Color.blue, self.tt.Kind.FIFO)

    def test_hash(self) -> None:
        colors = {}
        colors[self.tt.Color.red] = 0xFF0000
        colors[self.tt.Color.blue] = 0x0000FF
        colors[self.tt.Color.green] = 0x00FF00
        self.assertEqual(colors[self.tt.Color.green], 0x00FF00)
        self.assertTrue(self.tt.Color.blue in colors)
        self.assertTrue(self.tt.Kind.CHAR not in colors)
        self.assertTrue(1 in colors)
        values_to_names = {v.value: v.name for v in self.tt.Color}
        self.assertEqual(values_to_names[self.tt.Color.red], "red")

    def test_enum_in_enum_out(self) -> None:
        self.assertIs(self.tt.Color(self.tt.Color.blue), self.tt.Color.blue)

    def test_enum_value(self) -> None:
        self.assertEqual(self.tt.Color.red.value, 0)

    def test_enum(self) -> None:
        lst = list(self.tt.Color)
        self.assertEqual(len(lst), len(self.tt.Color))
        self.assertEqual(len(self.tt.Color), 3)
        self.assertEqual(
            [self.tt.Color.red, self.tt.Color.blue, self.tt.Color.green], lst
        )
        for i, color in enumerate("red blue green".split(), 0):
            e = self.tt.Color(i)
            self.assertEqual(e, getattr(self.tt.Color, color))
            self.assertEqual(e.value, i)
            self.assertEqual(e, i)
            self.assertEqual(e.name, color)
            self.assertIn(e, self.tt.Color)
            self.assertIs(type(e), self.tt.Color)
            self.assertIsInstance(e, self.tt.Color)
            self.assertEqual(str(e), "Color." + color)
            self.assertEqual(int(e), i)
            self.assertEqual(repr(e), f"<Color.{color}: {i}>")

    def test_insinstance_Enum(self) -> None:
        _ = list(self.tt.Color)
        self.assertIsInstance(self.tt.Color.red, Enum)
        self.assertTrue(issubclass(self.tt.Color, Enum))


# tt = test_types, ser = serializer
@parameterized_class(
    ("tt", "ser"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class FlagTests(unittest.TestCase):
    def test_flag_enum(self) -> None:
        with self.assertRaises(TypeError):
            self.tt.File(name="/etc/motd", permissions=4)
        x = self.tt.File(
            name="/bin/sh", permissions=self.tt.Perm.read | self.tt.Perm.execute
        )
        self.assertIsInstance(x.permissions, self.tt.Perm)
        self.assertEqual(x.permissions, self.tt.Perm.read | self.tt.Perm.execute)
        self.assertTrue(x.permissions)
        self.assertNotIsInstance(2, self.tt.Perm, "Flags are not ints")
        self.assertEqual(x.permissions.value, 5)
        x = self.tt.File(name="")
        self.assertFalse(x.permissions)
        self.assertIsInstance(x.permissions, self.tt.Perm)
        self.assertEqual(f"{self.tt.Perm.read}", "Perm.read")
        self.assertTrue(self.tt.Perm.read in self.tt.Perm.read | self.tt.Perm.execute)

    def test_flag_enum_serialization_roundtrip(self) -> None:
        x = self.tt.File(
            name="/dev/null",
            type=self.tt.Kind.CHAR,
            permissions=self.tt.Perm.read | self.tt.Perm.write,
        )

        y = self.ser.deserialize(self.tt.File, self.ser.serialize_iobuf(x))
        self.assertEqual(x, y)
        self.assertEqual(x.permissions, self.tt.Perm.read | self.tt.Perm.write)
        self.assertIsInstance(x.permissions, self.tt.Perm)

    def test_zero(self) -> None:
        zero = self.tt.Perm(0)
        self.assertNotIn(zero, self.tt.Perm)
        self.assertIsInstance(zero, self.tt.Perm)

    def test_logical(self) -> None:
        self.assertEqual(self.tt.Perm.read & self.tt.Perm.write, self.tt.Perm(0))
        self.assertEqual(self.tt.Perm.read ^ self.tt.Perm.write, self.tt.Perm(6))
        self.assertEqual(~self.tt.Perm.read, self.tt.Perm(3))

    def test_combination(self) -> None:
        combo = self.tt.Perm(self.tt.Perm.read.value | self.tt.Perm.execute.value)
        self.assertNotIn(combo, self.tt.Perm)
        self.assertIsInstance(combo, self.tt.Perm)
        self.assertIs(combo, self.tt.Perm.read | self.tt.Perm.execute)

    def test_is(self) -> None:
        allp = self.tt.Perm(7)
        self.assertIs(allp, self.tt.Perm(7))

    def test_invert(self) -> None:
        x = self.tt.Perm(-2)
        self.assertIs(x, self.tt.Perm.read | self.tt.Perm.write)

    def test_insinstance_Flag(self) -> None:
        self.assertIsInstance(self.tt.Perm.read, Flag)
        self.assertTrue(issubclass(self.tt.Perm, Flag))
        self.assertIsInstance(self.tt.Perm.read, Enum)
        self.assertTrue(issubclass(self.tt.Perm, Enum))

    def test_combo_in_call(self) -> None:
        x = self.tt.Perm(7)
        self.assertIs(x, self.tt.Perm.read | self.tt.Perm.write | self.tt.Perm.execute)

    def test_combo_repr(self) -> None:
        x = self.tt.Perm(7)
        self.assertEqual("<Perm.read|write|execute: 7>", repr(x))
