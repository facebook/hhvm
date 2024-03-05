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
import unittest
from typing import cast, Type, TypeVar

from testing.thrift_types import (
    BadMembers,
    Color,
    ColorGroups,
    File,
    Kind,
    OptionalColorGroups,
    OptionalFile,
    Perm,
)
from thrift.python.serializer import deserialize, Protocol, serialize_iobuf
from thrift.python.types import BadEnum, Enum, Flag

_E = TypeVar("_E", bound=Enum)


class EnumTests(unittest.TestCase):
    def test_bad_member_names(self) -> None:
        self.assertIsInstance(BadMembers.name_, BadMembers)
        self.assertIsInstance(BadMembers.value_, BadMembers)
        self.assertIn("name_", BadMembers.__members__)
        self.assertIn("value_", BadMembers.__members__)

    def test_normal_enum(self) -> None:
        with self.assertRaises(TypeError):
            # Enums are not ints
            # pyre-ignore[6]: for tests
            File(name="/etc/motd", type=8)
        x = File(name="/etc", type=Kind.DIR)
        self.assertIsInstance(x.type, Kind)
        self.assertEqual(x.type, Kind.DIR)
        self.assertNotEqual(x.type, Kind.SOCK)
        self.assertNotIsInstance(4, Kind, "Ints are not Enums")
        self.assertIsInstance(Kind.DIR, int, "Enums are Ints")
        self.assertIn(x.type, Kind)
        self.assertEqual(x.type.value, 4)

    def test_enum_value_rename(self) -> None:
        """The value name is None but we auto rename it to None_"""
        x = deserialize(File, b'{"name":"blah", "type":0}', Protocol.JSON)
        self.assertEqual(x.type, Kind.None_)

    def test_bad_enum_hash_same(self) -> None:
        x = deserialize(File, b'{"name": "something", "type": 64}', Protocol.JSON)
        y = deserialize(File, b'{"name": "something", "type": 64}', Protocol.JSON)
        self.assertEqual(hash(x), hash(y))
        self.assertEqual(hash(x.type), hash(y.type))
        self.assertFalse(x.type is y.type)
        self.assertEqual(x.type, y.type)
        self.assertFalse(x.type != y.type)

    def test_bad_enum_in_struct(self) -> None:
        to_serialize = OptionalFile(name="something", type=64)
        serialized = serialize_iobuf(to_serialize)
        x = deserialize(File, serialized)
        self.assertBadEnum(cast(BadEnum, x.type), Kind, 64)

    def test_bad_enum_in_list_index(self) -> None:
        x = deserialize(
            ColorGroups, serialize_iobuf(OptionalColorGroups(color_list=[1, 5, 0]))
        )
        self.assertEqual(len(x.color_list), 3)
        self.assertEqual(x.color_list[0], Color.blue)
        self.assertBadEnum(cast(BadEnum, x.color_list[1]), Color, 5)
        self.assertEqual(x.color_list[2], Color.red)

    def test_bad_enum_in_list_iter(self) -> None:
        x = deserialize(
            ColorGroups, serialize_iobuf(OptionalColorGroups(color_list=[1, 5, 0]))
        )
        for idx, v in enumerate(x.color_list):
            if idx == 0:
                self.assertEqual(v, Color.blue)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), Color, 5)
            else:
                self.assertEqual(v, Color.red)

    def test_bad_enum_in_list_reverse(self) -> None:
        x = deserialize(
            ColorGroups, serialize_iobuf(OptionalColorGroups(color_list=[1, 5, 0]))
        )
        for idx, v in enumerate(reversed(x.color_list)):
            if idx == 0:
                self.assertEqual(v, Color.red)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), Color, 5)
            else:
                self.assertEqual(v, Color.blue)

    def test_bad_enum_in_set_iter(self) -> None:
        x = deserialize(
            ColorGroups, serialize_iobuf(OptionalColorGroups(color_list=[1, 5, 0]))
        )
        for v in x.color_set:
            if v not in (Color.blue, Color.red):
                self.assertBadEnum(cast(BadEnum, v), Color, 5)

    def test_bad_enum_in_map_lookup(self) -> None:
        x = deserialize(
            ColorGroups,
            serialize_iobuf(OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})),
        )
        val = x.color_map[Color.red]
        self.assertBadEnum(cast(BadEnum, val), Color, 5)

    def test_bad_enum_in_map_iter(self) -> None:
        x = deserialize(
            ColorGroups,
            serialize_iobuf(OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})),
        )
        s = set()
        for k in x.color_map:
            s.add(k)
        self.assertEqual(len(s), 4)
        s.discard(Color.red)
        s.discard(Color.blue)
        lst = sorted(s, key=lambda e: cast(BadEnum, e).value)
        self.assertBadEnum(cast(BadEnum, lst[0]), Color, 6)
        self.assertBadEnum(cast(BadEnum, lst[1]), Color, 7)

    def test_bad_enum_in_map_values(self) -> None:
        x = deserialize(
            ColorGroups,
            serialize_iobuf(OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})),
        )
        s = set()
        for k in x.color_map.values():
            s.add(k)
        self.assertEqual(len(s), 4)
        s.discard(Color.green)
        s.discard(Color.blue)
        lst = sorted(s, key=lambda e: cast(BadEnum, e).value)
        self.assertBadEnum(cast(BadEnum, lst[0]), Color, 5)
        self.assertBadEnum(cast(BadEnum, lst[1]), Color, 8)

    def test_bad_enum_in_map_items(self) -> None:
        x = deserialize(
            ColorGroups,
            serialize_iobuf(OptionalColorGroups(color_map={1: 2, 0: 5, 6: 1, 7: 8})),
        )
        for k, v in x.color_map.items():
            if k == Color.blue:
                self.assertEqual(v, Color.green)
            elif k == Color.red:
                self.assertBadEnum(cast(BadEnum, v), Color, 5)
            else:
                ck = cast(BadEnum, k)
                if ck.value == 6:
                    self.assertEqual(v, Color.blue)
                else:
                    self.assertBadEnum(cast(BadEnum, v), Color, 8)

    def assertBadEnum(self, e: BadEnum, cls: Type[_E], val: int) -> None:
        self.assertIsInstance(e, BadEnum)
        self.assertEqual(e.value, val)
        self.assertEqual(e.enum, cls)
        self.assertEqual(int(e), val)

    def test_format(self) -> None:
        self.assertEqual(f"{Color.red}", "Color.red")

    def test_bool_of_class(self) -> None:
        self.assertTrue(bool(Color))

    def test_bool_of_members(self) -> None:
        self.assertTrue(Kind.None_)
        self.assertTrue(Color.red)

    def test_pickle(self) -> None:
        serialized = pickle.dumps(Color.green)
        green = pickle.loads(serialized)
        self.assertIs(green, Color.green)

    def test_adding_member(self) -> None:
        with self.assertRaises(AttributeError):
            # pyre-fixme[16]: `Type` has no attribute `black`.
            Color.black = 3

    def test_delete(self) -> None:
        with self.assertRaises(AttributeError):
            del Color.red

    def test_changing_member(self) -> None:
        with self.assertRaises(AttributeError):
            # pyre-fixme[8]: Attribute has type `Color`; used as `str`.
            Color.red = "lol"

    def test_contains(self) -> None:
        self.assertIn(Color.blue, Color)
        self.assertIn(1, Color)

    def test_equal(self) -> None:
        self.assertEqual(Color.blue, Color.blue)
        self.assertNotEqual(Color.blue, Color.green)
        self.assertEqual(Color.blue, 1)
        self.assertEqual(2, Color.green)
        self.assertNotEqual(Color.blue, Kind.FIFO)

    def test_hash(self) -> None:
        colors = {}
        colors[Color.red] = 0xFF0000
        colors[Color.blue] = 0x0000FF
        colors[Color.green] = 0x00FF00
        self.assertEqual(colors[Color.green], 0x00FF00)
        self.assertTrue(Color.blue in colors)
        self.assertTrue(Kind.CHAR not in colors)
        self.assertTrue(1 in colors)
        values_to_names = {v.value: v.name for v in Color}
        self.assertEqual(values_to_names[Color.red], "red")

    def test_enum_in_enum_out(self) -> None:
        self.assertIs(Color(Color.blue), Color.blue)

    def test_enum_value(self) -> None:
        self.assertEqual(Color.red.value, 0)

    def test_enum(self) -> None:
        lst = list(Color)
        self.assertEqual(len(lst), len(Color))
        self.assertEqual(len(Color), 3)
        self.assertEqual([Color.red, Color.blue, Color.green], lst)
        for i, color in enumerate("red blue green".split(), 0):
            e = Color(i)
            self.assertEqual(e, getattr(Color, color))
            self.assertEqual(e.value, i)
            self.assertEqual(e, i)
            self.assertEqual(e.name, color)
            self.assertIn(e, Color)
            self.assertIs(type(e), Color)
            self.assertIsInstance(e, Color)
            self.assertEqual(str(e), "Color." + color)
            self.assertEqual(int(e), i)
            self.assertEqual(repr(e), f"<Color.{color}: {i}>")

    def test_insinstance_Enum(self) -> None:
        self.assertIsInstance(Color.red, Enum)
        self.assertTrue(issubclass(Color, Enum))


class FlagTests(unittest.TestCase):
    def test_flag_enum(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for tests
            File(name="/etc/motd", permissions=4)
        x = File(name="/bin/sh", permissions=Perm.read | Perm.execute)
        self.assertIsInstance(x.permissions, Perm)
        self.assertEqual(x.permissions, Perm.read | Perm.execute)
        self.assertTrue(x.permissions)
        self.assertNotIsInstance(2, Perm, "Flags are not ints")
        self.assertEqual(x.permissions.value, 5)
        x = File(name="")
        self.assertFalse(x.permissions)
        self.assertIsInstance(x.permissions, Perm)
        self.assertEqual(f"{Perm.read}", "Perm.read")
        self.assertTrue(Perm.read in Perm.read | Perm.execute)

    def test_flag_enum_serialization_roundtrip(self) -> None:
        x = File(name="/dev/null", type=Kind.CHAR, permissions=Perm.read | Perm.write)

        y = deserialize(File, serialize_iobuf(x))
        self.assertEqual(x, y)
        self.assertEqual(x.permissions, Perm.read | Perm.write)
        self.assertIsInstance(x.permissions, Perm)

    def test_zero(self) -> None:
        zero = Perm(0)
        self.assertNotIn(zero, Perm)
        self.assertIsInstance(zero, Perm)

    def test_logical(self) -> None:
        self.assertEqual(Perm.read & Perm.write, Perm(0))
        self.assertEqual(Perm.read ^ Perm.write, Perm(6))
        self.assertEqual(~Perm.read, Perm(3))

    def test_combination(self) -> None:
        combo = Perm(Perm.read.value | Perm.execute.value)
        self.assertNotIn(combo, Perm)
        self.assertIsInstance(combo, Perm)
        self.assertIs(combo, Perm.read | Perm.execute)

    def test_is(self) -> None:
        allp = Perm(7)
        self.assertIs(allp, Perm(7))

    def test_invert(self) -> None:
        x = Perm(-2)
        self.assertIs(x, Perm.read | Perm.write)

    def test_insinstance_Flag(self) -> None:
        self.assertIsInstance(Perm.read, Flag)
        self.assertTrue(issubclass(Perm, Flag))
        self.assertIsInstance(Perm.read, Enum)
        self.assertTrue(issubclass(Perm, Enum))

    def test_combo_in_call(self) -> None:
        x = Perm(7)
        self.assertIs(x, Perm.read | Perm.write | Perm.execute)

    def test_combo_repr(self) -> None:
        x = Perm(7)
        self.assertEqual("<Perm.read|write|execute: 7>", repr(x))
