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


from __future__ import annotations

import unittest
from enum import Enum
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
from thrift.python.serializer import deserialize, serialize_iobuf
from thrift.python.types import BadEnum

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
        self.assertNotEqual(x.type, 4, "Enums are not Ints")
        self.assertNotIsInstance(4, Kind, "Enums are not Ints")
        self.assertIn(x.type, Kind)
        self.assertEqual(x.type.value, 4)

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


class FlagTests(unittest.TestCase):
    def test_flag_enum(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: for tests
            File(name="/etc/motd", permissions=4)
        x = File(name="/bin/sh", permissions=Perm.read | Perm.execute)
        self.assertIsInstance(x.permissions, Perm)
        self.assertEqual(x.permissions, Perm.read | Perm.execute)
        self.assertNotIsInstance(2, Perm, "Flags are not ints")
        self.assertEqual(x.permissions.value, 5)
        x = File(name="")
        self.assertFalse(x.permissions)
        self.assertIsInstance(x.permissions, Perm)

    def test_flag_enum_serialization_roundtrip(self) -> None:
        x = File(name="/dev/null", type=Kind.CHAR, permissions=Perm.read | Perm.write)

        y = deserialize(File, serialize_iobuf(x))
        self.assertEqual(x, y)
        self.assertEqual(x.permissions, Perm.read | Perm.write)
        self.assertIsInstance(x.permissions, Perm)
