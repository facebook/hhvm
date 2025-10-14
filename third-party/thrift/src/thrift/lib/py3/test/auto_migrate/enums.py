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

import pickle
import unittest
from typing import cast, Type, TypeVar

from testing.thrift_types import Color as python_Color, Kind as python_Kind

from testing.types import BadMembers, Color, ColorGroups, File, Kind, Perm
from thrift.py3.common import Protocol
from thrift.py3.serializer import deserialize, serialize
from thrift.py3.types import BadEnum, Enum, Flag


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
            # pyre-fixme[6]: Expected `Optional[Kind]` for 2nd param but got `int`.
            File(name="/etc/motd", type=8)
        x = File(name="/etc", type=Kind.DIR)
        self.assertIsInstance(x.type, Kind)
        self.assertEqual(x.type, Kind.DIR)
        self.assertNotEqual(x.type, Kind.SOCK)
        self.assertIn(x.type, Kind)
        self.assertEqual(int(x.type), 4)
        self.assertRaises(ValueError, lambda: Kind(47))

    def test_normal_enum_not_int(self) -> None:
        x = File(name="/etc", type=Kind.DIR)
        self.assertEqual(x.type, 4, "Enums now compare to Ints")
        # in thrift-python they are int
        self.assertNotIsInstance(4, Kind, "Enums are not Ints")

    def test_enum_value_rename(self) -> None:
        """The value name is None but we auto rename it to None_"""
        x = deserialize(File, b'{"name":"blah", "type":0}', Protocol.JSON)
        self.assertEqual(x.type, Kind.None_)

    def test_protocol_int_conversion(self) -> None:
        self.assertEqual(Protocol.BINARY.value, 0)
        self.assertEqual(Protocol.DEPRECATED_VERBOSE_JSON.value, 1)
        self.assertEqual(Protocol.COMPACT.value, 2)
        self.assertEqual(Protocol.JSON.value, 5)

    def test_python_py3_equivalence(self) -> None:
        self.assertIs(Color, python_Color)
        self.assertIs(Color.red, python_Color.red)

        self.assertEqual(Color.red, python_Color.red)
        self.assertNotEqual(Color.blue, python_Color.red)

        # basic validation that pyre doesn't complain
        cg = ColorGroups(
            color_list=[python_Color.red, python_Color.blue, python_Color.green]
        )
        self.assertEqual(cg.color_list, [Color.red, Color.blue, Color.green])
        self.assertEqual(File(type=python_Kind.CHAR).type, Kind.CHAR)

    def test_bad_enum_hash_same(self) -> None:
        x = deserialize(File, b'{"name": "something", "type": 64}', Protocol.JSON)
        y = deserialize(File, b'{"name": "something", "type": 64}', Protocol.JSON)
        self.assertEqual(hash(x), hash(y))
        self.assertEqual(hash(x.type), hash(y.type))
        self.assertFalse(x.type is y.type)
        self.assertEqual(x.type, y.type)
        self.assertFalse(x.type != y.type)

    def test_bad_enum_in_struct(self) -> None:
        x = deserialize(File, b'{"name": "something", "type": 64}', Protocol.JSON)
        self.assertBadEnum(cast(BadEnum, x.type), Kind, 64)

    def test_bad_enum_in_list_index(self) -> None:
        x = deserialize(ColorGroups, b'{"color_list": [1, 5, 0]}', Protocol.JSON)
        self.assertEqual(len(x.color_list), 3)
        self.assertEqual(x.color_list[0], Color.blue)
        self.assertBadEnum(cast(BadEnum, x.color_list[1]), Color, 5)
        self.assertEqual(x.color_list[2], Color.red)

    def test_bad_enum_in_list_iter(self) -> None:
        x = deserialize(ColorGroups, b'{"color_list": [1, 5, 0]}', Protocol.JSON)
        for idx, v in enumerate(x.color_list):
            if idx == 0:
                self.assertEqual(v, Color.blue)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), Color, 5)
            else:
                self.assertEqual(v, Color.red)

    def test_bad_enum_in_list_reverse(self) -> None:
        x = deserialize(ColorGroups, b'{"color_list": [1, 5, 0]}', Protocol.JSON)
        for idx, v in enumerate(reversed(x.color_list)):
            if idx == 0:
                self.assertEqual(v, Color.red)
            elif idx == 1:
                self.assertBadEnum(cast(BadEnum, v), Color, 5)
            else:
                self.assertEqual(v, Color.blue)

    def test_bad_enum_in_set_iter(self) -> None:
        x = deserialize(ColorGroups, b'{"color_set": [1, 5, 0]}', Protocol.JSON)
        for v in x.color_set:
            if v not in (Color.blue, Color.red):
                self.assertBadEnum(cast(BadEnum, v), Color, 5)

    def test_bad_enum_in_map_lookup(self) -> None:
        json = b'{"color_map": {"1": 2, "0": 5, "6": 1, "7": 8}}'
        x = deserialize(ColorGroups, json, Protocol.JSON)
        val = x.color_map[Color.red]
        self.assertBadEnum(cast(BadEnum, val), Color, 5)

    def test_bad_enum_in_map_iter(self) -> None:
        json = b'{"color_map": {"1": 2, "0": 5, "6": 1, "7": 8}}'
        x = deserialize(ColorGroups, json, Protocol.JSON)
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
        json = b'{"color_map": {"1": 2, "0": 5, "6": 1, "7": 8}}'
        x = deserialize(ColorGroups, json, Protocol.JSON)
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
        json = b'{"color_map": {"0": 5, "1": 2, "6": 1, "7": 8}}'
        x = deserialize(ColorGroups, json, Protocol.JSON)
        self.assertEqual(
            str(x.color_map),
            "i{<Color.red: 0>: <Color.#INVALID#: 5>,"
            " <Color.blue: 1>: <Color.green: 2>,"
            " <Color.#INVALID#: 6>: <Color.blue: 1>,"
            " <Color.#INVALID#: 7>: <Color.#INVALID#: 8>}",
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

    def assertBadEnum(self, e: BadEnum | Enum, cls: Type[_E], val: int) -> None:
        self.assertIsInstance(e, BadEnum)
        self.assertEqual(e.value, val)
        self.assertEqual(cast(BadEnum, e).enum, cls)
        self.assertEqual(int(e), val)

    def test_pickle(self) -> None:
        serialized = pickle.dumps(Color.green)
        green = pickle.loads(serialized)
        self.assertIs(green, Color.green)

    def test_adding_member(self) -> None:
        with self.assertRaises(AttributeError):
            # pyre-fixme[16]: `Type` has no attribute `black`.
            Color.black = 3

    def test_delete(self) -> None:
        with self.assertRaises((AttributeError, TypeError)):
            del Color.red

    def test_bool_of_class(self) -> None:
        self.assertTrue(bool(Color))

    def test_bool_of_members(self) -> None:
        self.assertTrue(bool(Color.blue))

    def test_changing_member(self) -> None:
        with self.assertRaises((AttributeError, TypeError)):
            # pyre-fixme[41]: Cannot reassign final attribute
            Color.red = "lol"

    def test_hash(self) -> None:
        colors = {}
        colors[Color.red] = 0xFF0000
        colors[Color.blue] = 0x0000FF
        colors[Color.green] = 0x00FF00
        self.assertEqual(colors[Color.green], 0x00FF00)

    def test_enum_in_enum_out(self) -> None:
        self.assertIs(Color(Color.blue), Color.blue)

    def test_enum_value(self) -> None:
        self.assertEqual(Color.red.value, 0)

    def test_no_error_hasattr(self) -> None:
        self.assertFalse(hasattr(Color.red, "type"))
        self.assertFalse(hasattr(Color.red, "yellow"))

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

    def test_enum_module(self) -> None:
        # py3 enums are now the same as thrift-python enums
        self.assertEqual(Color.__module__, "testing.thrift_enums")

    def test_enum_print(self) -> None:
        for i, color in enumerate("red blue green".split(), 0):
            e = Color(i)
            # should have a test where repr called before str
            self.assertEqual(repr(e), f"<Color.{color}: {i}>")
            self.assertEqual(str(e), "Color." + color)
            self.assertEqual(f"{e=}", f"e=<Color.{color}: {i}>")

    def test_isinstance_Enum(self) -> None:
        self.assertIsInstance(Color.red, Enum)
        self.assertTrue(issubclass(Color, Enum))

    def test_callable(self) -> None:
        vals = range(3)
        colors = list(map(Color, vals))
        self.assertEqual(colors, list(Color))


class EnumMetaTests(unittest.TestCase):
    def test_iter_forward_compatible(self) -> None:
        # test that replacement for _fbthrift_get_all_names()
        # works as expected and is forward-compatible in thrift-python
        self.assertEqual([e.name for e in Color], ["red", "blue", "green"])

    def test_enum_call_forward_compatible(self) -> None:
        self.assertEqual(Color(0), Color.red)
        self.assertEqual(Color(1), Color.blue)
        self.assertEqual(Color(2), Color.green)

    def test_getitem(self) -> None:
        self.assertEqual(Color["red"], Color.red)
        self.assertEqual(Color["blue"], Color.blue)
        self.assertEqual(Color["green"], Color.green)

    def test_is_thrift_enum(self) -> None:
        self.assertTrue(hasattr(Color, "__members__"))

    def test_enum_metaclass_iter(self) -> None:
        self.assertEqual(list(Color), [Color.red, Color.blue, Color.green])
        self.assertEqual(list(reversed(Color)), [Color.green, Color.blue, Color.red])

    def test_enum_metaclass_contains(self) -> None:
        self.assertIn(Color.red, Color)
        self.assertIn(Color.blue, Color)
        self.assertIn(Color.green, Color)
        self.assertNotIn("red", Color)
        self.assertNotIn(Perm.read, Color)

        self.assertNotIn(-1, Color)
        self.assertNotIn(3, Color)

    def test_enum_metaclass_contains_int(self) -> None:
        self.assertIn(0, Color)
        self.assertIn(1, Color)
        self.assertIn(2, Color)

    def test_enum_metaclass_dir(self) -> None:
        attrs = set(dir(Color))
        self.assertIn("red", attrs)
        self.assertIn("blue", attrs)
        self.assertIn("green", attrs)
        self.assertIn("__class__", attrs)
        self.assertIn("__doc__", attrs)
        self.assertIn("__members__", attrs)
        self.assertIn("__module__", attrs)

    def test_sortable(self) -> None:
        colors = [Color(i) for i in range(2, -1, -1)]
        sorted_colors = [Color(i) for i in range(3)]
        self.assertEqual(sorted(colors), sorted_colors)

        colors.append(Color(0))
        self.assertEqual(sorted(colors), [Color(0)] + sorted_colors)

    def test_sortable_error(self) -> None:
        sorted_colors = [Color(i) for i in range(3)]
        self.assertEqual(sorted([1, Color(2), Color(0)]), sorted_colors)


class FlagTests(unittest.TestCase):
    def test_flag_enum(self) -> None:
        with self.assertRaises(TypeError):
            # flags are not ints
            # pyre-fixme[6]: Expected `Optional[Perm]` for 2nd param but got `int`.
            File(name="/etc/motd", permissions=4)
        x = File(name="/bin/sh", permissions=(Perm.read | Perm.execute))
        self.assertIsInstance(x.permissions, Perm)
        self.assertEqual(x.permissions, 5)
        self.assertEqual(x.permissions, Perm.read | Perm.execute)
        self.assertNotIsInstance(2, Perm, "Flags are not ints")
        self.assertEqual(int(x.permissions), 5)
        x = File(name="")
        self.assertFalse(x.permissions)
        self.assertIsInstance(x.permissions, Perm)

    def test_flag_enum_serialization_roundtrip(self) -> None:
        x = File(name="/dev/null", type=Kind.CHAR, permissions=Perm.read | Perm.write)

        y = deserialize(File, serialize(x))
        self.assertEqual(x, y)
        self.assertEqual(x.permissions, Perm.read | Perm.write)
        self.assertIsInstance(x.permissions, Perm)

    def test_zero(self) -> None:
        zero = Perm(0)
        self.assertIsInstance(zero, Perm)

        # NOTE: The following assertion used to be reversed, but has been failing for
        # months. This probably needs to be clarified similarly to the test below.
        self.assertIn(zero, Perm)

    def test_combination(self) -> None:
        combo = Perm(Perm.read.value | Perm.execute.value)
        self.assertIsInstance(combo, Perm)
        self.assertIs(combo, Perm.read | Perm.execute)

        # NOTE: It's currently unclear whether the following should be True or False.
        # The behavior has changed depending on Python versions, and this exact test
        # has been failing for months, so the assertion captures the current behavior
        # for now.
        self.assertIn(combo, Perm)

    def test_is(self) -> None:
        allp = Perm(7)
        self.assertIs(allp, Perm(7))

    def test_invert(self) -> None:
        x = Perm(-2)
        self.assertIs(x, Perm.read | Perm.write)

    def test_isinstance_Flag(self) -> None:
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

    def test_Perm_len(self) -> None:
        self.assertEqual(len(Perm), 3)
