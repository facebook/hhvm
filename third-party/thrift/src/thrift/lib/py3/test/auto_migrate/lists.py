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

import itertools
import unittest

from testing.types import (
    binary_list,
    Color,
    ColorGroups,
    easy,
    I32List,
    int_list,
    List__i16,
    List__i32,
    List__string,
    ListTypes,
    StringList,
    StrList2D,
    Uint32List,
    unicode_list,
)
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import (
    brokenInAutoMigrate,
    is_auto_migrated,
)
from thrift.lib.python.test.testing_utils import Untruthy
from thrift.py3.types import Container, List as Py3List
from thrift.python.types import List as PythonList


class ListTests(unittest.TestCase):
    def test_negative_indexes(self) -> None:
        length = len(int_list)
        for i in range(length):
            self.assertEqual(int_list[i], int_list[i - length])

        with self.assertRaises(IndexError):
            int_list[-length - 1]

    def test_list_of_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Sequence[int]]` for 1st param
            #  but got `List[None]`.
            I32List([None, None, None])

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Sequence[int]]` for 1st param
            #  but got `List[None]`.
            List__i32([None, None, None])

    def test_string_const(self) -> None:
        self.assertEqual(unicode_list, ["Bulgaria", "Benin", "Saint Barthélemy"])
        self.assertEqual(binary_list, [b"Saint Barth\xc3\xa9lemy"])

    def test_isinstance(self) -> None:
        self.assertIsInstance(List__i32(range(5)), Py3List)
        self.assertTrue(issubclass(List__i32, Py3List))
        self.assertIsInstance(List__i32(range(5)), List__i32)
        self.assertIsInstance(easy(val_list=[1, 2, 3]).val_list, Py3List)
        self.assertTrue(
            issubclass(easy(val_list=[1, 2, 3]).val_list.__class__, Py3List)
        )
        self.assertIsInstance(easy(val_list=[1, 2, 3]).val_list, List__i32)
        self.assertIsInstance(I32List(range(5)), Py3List)
        self.assertIsInstance(I32List(range(5)), List__i32)

        if is_auto_migrated():
            self.assertIsInstance(List__i32(range(5)), PythonList)
            self.assertIsInstance(easy(val_list=[1, 2, 3]).val_list, PythonList)
            self.assertIsInstance(I32List(range(5)), PythonList)

        self.assertNotIsInstance(List__i16(range(5)), List__i32)
        self.assertNotIsInstance(
            List__string([f"str_{i}" for i in range(5)]), List__i32
        )

    def test_list_creation_with_list_items(self) -> None:
        a = ["one", "two", "three"]
        b = ["cyan", "magenta", "yellow"]
        c = ["foo", "bar", "baz"]
        d = ["I", "II", "III"]
        StrList2D([a, b, c, d])
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected
            #  `Optional[typing.Sequence[typing.Sequence[str]]]` for 1st param but got
            #  `List[typing.Union[typing.List[None], typing.List[str]]]`.
            StrList2D([a, [None]])

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            List__i32().__dict__

    def test_default_ctor(self) -> None:
        self.assertEqual(I32List(), [])
        self.assertEqual(List__i32(), [])

    @brokenInAutoMigrate()
    def test_list_add(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = int_list + other_list
        self.assertIsInstance(new_list, type(int_list))
        # Ensure the items from both lists are in the new_list
        self.assertEqual(new_list, list(itertools.chain(int_list, other_list)))

    def test_list_radd(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = other_list + int_list
        self.assertIsInstance(new_list, list)
        self.assertEqual(new_list, list(itertools.chain(other_list, int_list)))
        new_list = tuple(other_list) + int_list
        self.assertIsInstance(new_list, tuple)

    def test_list_creation(self) -> None:
        I32List(range(10))
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Sequence[int]]` for 1st param
            #  but got `List[typing.Union[int, str]]`.
            I32List([1, "b", "c", "four"])

    def test_create_untruthy_list(self) -> None:
        with self.assertRaisesRegex(ValueError, "Do not dare question my truth"):
            bool(Untruthy(5))

        self.assertEqual(I32List(Untruthy(5)), list(range(5)))
        self.assertEqual(ListTypes(second=Untruthy(5)).second, list(range(5)))

    def test_hashability(self) -> None:
        hash(easy().val_list)
        hash(I32List(range(10)))

    def test_index(self) -> None:
        x = I32List([1, 2, 3, 4, 1, 2, 3, 4])
        y = list(x)
        self.assertEqual(x.index(2), y.index(2))
        self.assertEqual(x.index(2, 3), y.index(2, 3))
        self.assertEqual(x.index(2, 0, 2), y.index(2, 0, 2))
        with self.assertRaises(ValueError):
            raise Exception(x.index(4, 0, 2))

        with self.assertRaises(ValueError):
            x.index("lol")

        with self.assertRaises(ValueError):
            y.index(4, 0, 2)
        self.assertEqual(x.index(4, -20, -2), y.index(4, -20, -2))

    def test_slicing(self) -> None:
        x = I32List([1, 2, 3, 4, 1, 2, 3, 4])
        y = list(x)
        self.assertEqual(x[2:], y[2:])
        self.assertEqual(x[:5], y[:5])
        self.assertEqual(x[:0], y[:0])
        self.assertEqual(x[-5:-1], y[-5:-1])
        self.assertEqual(x[::-1], y[::-1])
        self.assertEqual(x[::-3], y[::-3])
        self.assertEqual(x[-1:-5:-1], y[-1:-5:-1])

    def test_comparisons(self) -> None:
        x = I32List([1, 2, 3, 4])
        y = List__i32([1, 2, 3, 4, 5])
        z = I32List([1, 2, 3, 1, 10])

        # pyre-fixme[6]: For 2nd param expected `SupportsDunderGT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertLess(x, y)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderGT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertLess(z, x)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderGT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertLess(z, y)
        self.assertNotEqual(z, y)
        self.assertNotEqual(x, y)
        self.assertNotEqual(z, x)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderLT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertGreater(y, x)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderLT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertGreater(x, z)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderLE[Variable[_T]]` but
        #  got `List__i32`.
        self.assertGreaterEqual(x, z)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderGT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertLessEqual(x, y)

        x2 = I32List([1, 2, 3, 4])
        self.assertEqual(x, x2)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderGT[Variable[_T]]` but
        #  got `List__i32`.
        self.assertLessEqual(x, x2)
        # pyre-fixme[6]: For 2nd param expected `SupportsDunderLE[Variable[_T]]` but
        #  got `List__i32`.
        self.assertGreaterEqual(x, x2)

    def test_no_raise_on_type_error(self) -> None:
        t_list = I32List([1, 2, 3, 4])
        self.assertFalse(t_list == easy())
        self.assertFalse(easy() == t_list)
        self.assertNotEqual(t_list, easy())
        self.assertNotEqual(easy(), t_list)

    def test_list_from_iterable(self) -> None:
        colors = map(Color, range(3))
        self.assertEqual(
            # pyre-ignore[6]: deliberately incompatible parameter type
            ColorGroups(color_list=colors).color_list,
            [Color.red, Color.blue, Color.green],
        )

    def test_is_container(self) -> None:
        self.assertIsInstance(int_list, Container)
        self.assertIsInstance(I32List([1, 2, 3]), Container)
        self.assertIsInstance(StrList2D([["a", "b"], ["c", "d"]]), Container)

    def test_string_list(self) -> None:
        StringList(["hello", "world"])
        with self.assertRaises(TypeError):
            StringList("hello")

    def gen_colorgroups(self) -> ColorGroups:
        clist = [Color.red, Color.green, Color.red]
        return ColorGroups(
            color_list=clist,
            color_set=set(clist),
            color_map={c: c for c in clist},
        )

    def test_container_contains(self) -> None:
        groups = self.gen_colorgroups()
        self.assertIn(Color.red, groups.color_list)
        self.assertIn(Color.red, groups.color_set)
        self.assertIn(Color.red, groups.color_map)
        self.assertNotIn(Color.blue, groups.color_list)
        self.assertNotIn(Color.blue, groups.color_set)
        self.assertNotIn(Color.blue, groups.color_map)

    @brokenInAutoMigrate()
    def test_container_contains_type_mismatch(self) -> None:
        groups = self.gen_colorgroups()
        self.assertNotIn("str", groups.color_list)
        self.assertNotIn("str", groups.color_set)
        self.assertNotIn("str", groups.color_map)
        # in auto-migrate, thrift-python allows implicit enum conversion
        self.assertNotIn(0, groups.color_list)
        self.assertNotIn(0, groups.color_set)
        self.assertNotIn(0, groups.color_map)

    @brokenInAutoMigrate()
    def test_list_count(self) -> None:
        clist = [Color.red, Color.red, Color.blue]
        groups = ColorGroups(color_list=clist)

        self.assertEqual(groups.color_list.count(Color.red), 2)
        self.assertEqual(groups.color_list.count(Color.blue), 1)
        self.assertEqual(groups.color_list.count(Color.green), 0)

        self.assertEqual(groups.color_list.count("str"), 0)
        # in auto-migrate, thrift-python allows implicit enum conversion
        self.assertEqual(groups.color_list.count(int(Color.red)), 0)

    def test_custom_cpp_type_list(self) -> None:
        x = [1, 2, 3, 4]
        y = Uint32List(x)
        self.assertEqual(y[1], 2)
        self.assertEqual(list(y), x)

    # thrift-python doesn't believe in flat-name containers
    def test_list_module(self) -> None:
        # typedefs are ListTypeFactory in auto-migrate
        if not is_auto_migrated():
            self.assertEqual(I32List.__module__, "testing.types")
        # flat name containers are always .types
        self.assertEqual(List__i32.__module__, "testing.types")
