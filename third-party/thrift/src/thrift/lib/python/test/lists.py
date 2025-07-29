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

import itertools
import unittest

from typing import List, Type, TypeVar

import python_test.containers.thrift_mutable_types as mutable_containers_types
import python_test.containers.thrift_types as immutable_containers_types

import python_test.lists.thrift_mutable_types as mutable_lists_types
import python_test.lists.thrift_types as immutable_lists_types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from python_test.containers.thrift_types import Color, Foo, Lists
from python_test.lists.thrift_types import (
    AtoIValueList,
    easy,
    EasyList,
    I32List,
    int_list,
    StringList,
    StrList2D,
)
from thrift.lib.python.test.testing_utils import Untruthy
from thrift.python.mutable_types import _ThriftListWrapper, to_thrift_list

ListT = TypeVar("ListT")


class ImmutableListTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(easy().val_list)
        hash(I32List(range(10)))

    def test_constant_list(self) -> None:
        self.assertEqual(int_list, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10])

        with self.assertRaisesRegex(
            TypeError,
            "'thrift.python.types.List' object does not support item assignment",
        ):
            # pyre-ignore[16]: `typing.Sequence` has no attribute `__setitem__`
            int_list[0] = 0

        with self.assertRaisesRegex(
            AttributeError,
            "'thrift.python.types.List' object has no attribute 'append'",
        ):
            # pyre-ignore[16]: `typing.Sequence` has no attribute `append`
            int_list.append(11)


# ct = containers type, lt = lists type
@parameterized_class(
    ("containers_types", "lists_types"),
    [
        (immutable_containers_types, immutable_lists_types),
        (mutable_containers_types, mutable_lists_types),
    ],
)
class ListTests(unittest.TestCase):
    def setUp(self) -> None:
        # pyre-ignore[16]: has no attribute `lists_types`
        self.easy: Type[easy] = self.lists_types.easy
        self.EasyList: Type[EasyList] = self.lists_types.EasyList
        self.int_list: List[int] = self.lists_types.int_list
        self.I32List: Type[I32List] = self.lists_types.I32List
        self.StrList2D: Type[StrList2D] = self.lists_types.StrList2D
        self.StringList: Type[StringList] = self.lists_types.StringList
        self.AtoIValueList: Type[AtoIValueList] = self.lists_types.AtoIValueList
        # pyre-ignore[16]: has no attribute `containers_types`
        self.Foo: Type[Foo] = self.containers_types.Foo
        self.Lists: Type[Lists] = self.containers_types.Lists
        self.Color: Type[Color] = self.containers_types.Color
        self.is_mutable_run: bool = self.containers_types.__name__.endswith(
            "thrift_mutable_types"
        )
        self.unicode_list: list[str] = self.lists_types.unicode_list

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def test_negative_indexes(self) -> None:
        length = len(self.int_list)
        for i in range(length):
            self.assertEqual(self.int_list[i], self.int_list[i - length])

        with self.assertRaises(IndexError):
            self.int_list[-length - 1]

    def test_list_of_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.I32List([None, None, None])

    def test_list_creation_with_list_items(self) -> None:
        a = ["one", "two", "three"]
        b = ["cyan", "magenta", "yellow"]
        c = ["foo", "bar", "baz"]
        d = ["I", "II", "III"]
        # pyre-ignore[6]: TODO: Thrift-Container init
        self.StrList2D(self.to_list([a, b, c, d]))
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.StrList2D(self.to_list([a, [None]]))

    def test_list_add(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = self.int_list + other_list
        self.assertIsInstance(new_list, list)
        # Insure the items from both lists are in the new_list
        self.assertEqual(new_list, list(itertools.chain(self.int_list, other_list)))

    def test_list_radd(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = other_list + self.int_list
        self.assertIsInstance(new_list, list)
        self.assertEqual(new_list, list(itertools.chain(other_list, self.int_list)))
        # pyre-fixme[58]: special Thrift list supports concat with a tuple
        new_list = tuple(other_list) + self.int_list
        self.assertIsInstance(new_list, tuple)

    def test_list_creation(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        self.I32List(self.to_list(range(10)))
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.I32List([1, "b", "c", "four"])

    def test_create_untruthy_list(self) -> None:
        with self.assertRaises(ValueError):
            bool(Untruthy(5))

        self.assertEqual(I32List(Untruthy(5)), list(range(5)))
        self.assertEqual(Lists(i32List=Untruthy(5)).i32List, list(range(5)))

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            I32List().__dict__

    def test_index(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.I32List(self.to_list([1, 2, 3, 4, 1, 2, 3, 4]))
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
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.I32List(self.to_list([1, 2, 3, 4, 1, 2, 3, 4]))
        y = list(x)
        self.assertEqual(x[2:], y[2:])
        self.assertEqual(x[:5], y[:5])
        self.assertEqual(x[:0], y[:0])
        self.assertEqual(x[-5:-1], y[-5:-1])
        self.assertEqual(x[::-1], y[::-1])
        self.assertEqual(x[::-3], y[::-3])
        self.assertEqual(x[-1:-5:-1], y[-1:-5:-1])

    def test_comparisons(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.I32List(self.to_list([1, 2, 3, 4]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.I32List(self.to_list([1, 2, 3, 4, 5]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        z = self.I32List(self.to_list([1, 2, 3, 1, 10]))

        self.assertLess(x, y)
        self.assertLess(z, x)
        self.assertLess(z, y)
        self.assertNotEqual(z, y)
        self.assertNotEqual(x, y)
        self.assertNotEqual(z, x)
        self.assertGreater(y, x)
        self.assertGreater(x, z)
        self.assertGreaterEqual(x, z)
        self.assertLessEqual(x, y)

        # pyre-ignore[6]: TODO: Thrift-Container init
        x2 = self.I32List(self.to_list([1, 2, 3, 4]))
        self.assertEqual(x, x2)
        self.assertLessEqual(x, x2)
        self.assertGreaterEqual(x, x2)

        x3 = [1, 2, 3, 4]
        self.assertEqual(x, x3)
        self.assertLessEqual(x, x3)
        self.assertGreaterEqual(x, x3)
        self.assertLess(x3, y)
        self.assertNotEqual(z, x3)
        self.assertGreater(y, x3)
        self.assertLessEqual(x3, y)

    def test_no_raise_on_type_error(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        t_list = self.I32List(self.to_list([1, 2, 3, 4]))
        self.assertFalse(t_list == self.easy())
        self.assertFalse(self.easy() == t_list)
        self.assertNotEqual(t_list, self.easy())
        self.assertNotEqual(self.easy(), t_list)

    def test_string_list(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        self.StringList(self.to_list(["hello", "world"]))
        with self.assertRaises(TypeError):
            self.StringList("hello")

    def test_count(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.I32List(self.to_list([1, 2, 3, 4, 1, 2, 3, 4]))
        y = list(x)
        self.assertEqual(x.count(2), y.count(2))
        self.assertEqual(x.count(5), y.count(5))
        # pyre-ignore[6]: deliberate type mismatch
        self.assertEqual(x.count("str"), 0)

    def test_contains_enum(self) -> None:
        clist = self.Lists(
            # pyre-ignore[6]: TODO: Thrift-Container init
            colorList=self.to_list([self.Color.red, self.Color.red, self.Color.blue])
        )
        self.assertIn(self.Color.red, clist.colorList)
        self.assertIn(self.Color.blue, clist.colorList)
        self.assertNotIn(self.Color.green, clist.colorList)
        self.assertNotIn("str", clist.colorList)
        if self.is_mutable_run:
            self.assertNotIn(0, clist.colorList)
        else:
            self.assertIn(0, clist.colorList)

        # This behavior is more permissive than thrift-py3
        # which implicitly converts int to enum
        if self.is_mutable_run:
            self.assertNotIn(0, clist.colorList)
            self.assertNotIn(1, clist.colorList)

            # Python converts between integers and enums, but mutable containers
            # don't do it right now. This could be addressed in `EnumTypeInfo`
            # that will change the behavior in other places.
            self.assertEqual(0, self.Color.red)
            self.assertEqual(clist.colorList.index(self.Color.red), 0)
            with self.assertRaises(ValueError):
                clist.colorList.index(0)
        else:
            self.assertIn(0, clist.colorList)
            self.assertIn(1, clist.colorList)
            self.assertEqual(clist.colorList.index(0), 0)
            self.assertEqual(clist.colorList.index(1), 2)

        with self.assertRaises(ValueError):
            clist.colorList.index(2)

        if not self.is_mutable_run:
            # gross
            self.assertEqual(clist.colorList[clist.colorList.index(0)], self.Color.red)
            self.assertEqual(clist.colorList[clist.colorList.index(1)], self.Color.blue)

    def test_struct_list(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        a = self.EasyList(self.to_list([self.easy()]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        b = self.EasyList(self.to_list([self.easy(val=0)]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        c = self.EasyList(self.to_list([self.easy(val=1)]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        d = self.EasyList(self.to_list([self.easy(val_list=self.to_list([]))]))
        # pyre-ignore[6]: TODO: Thrift-Container init
        e = self.EasyList(self.to_list([self.easy(val_list=self.to_list([1]))]))
        self.assertEqual(a, b)
        self.assertEqual(a, d)
        self.assertNotEqual(a, c)
        self.assertLess(a, c)
        self.assertGreater(c, e)
        self.assertLess(d, e)
        self.assertGreater(e, d)
        self.assertLessEqual(a, b)
        self.assertGreaterEqual(a, d)
        self.assertLessEqual(a, c)
        self.assertGreaterEqual(c, e)

    def test_list_module_name(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        easy_list = self.EasyList(self.to_list([self.easy()]))
        if self.is_mutable_run:
            self.assertEqual(
                easy_list.__class__.__module__, "thrift.python.mutable_containers"
            )
        else:
            self.assertEqual(easy_list.__class__.__module__, "thrift.python.types")

    def test_struct_with_list_fields(self) -> None:
        s = self.Lists(
            # pyre-ignore[6]: TODO: Thrift-Container init
            boolList=self.to_list([True, False]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            byteList=self.to_list([1, 2, 3]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i16List=self.to_list([4, 5, 6]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i64List=self.to_list([7, 8, 9]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            doubleList=self.to_list([1.23, 4.56]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            floatList=self.to_list([7.89, 10.11]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            stringList=self.to_list(["foo", "bar"]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            binaryList=self.to_list([b"foo", b"bar"]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufList=self.to_list([IOBuf(b"foo"), IOBuf(b"bar")]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            structList=self.to_list([self.Foo(value=1), self.Foo(value=2)]),
        )
        self.assertEqual(s.boolList, [True, False])
        self.assertEqual(s.byteList, [1, 2, 3])
        self.assertEqual(s.i16List, [4, 5, 6])
        self.assertEqual(s.i64List, [7, 8, 9])
        self.assertEqual(s.doubleList, [1.23, 4.56])
        self.assertEqual(s.floatList, [7.89, 10.11])
        self.assertEqual(s.stringList, ["foo", "bar"])
        self.assertEqual(s.binaryList, [b"foo", b"bar"])
        self.assertEqual(s.iobufList, [IOBuf(b"foo"), IOBuf(b"bar")])
        self.assertEqual(s.structList, [self.Foo(value=1), self.Foo(value=2)])
        self.assertIs(s.structList[0], s.structList[0])
        self.assertIs(s.structList[1], s.structList[1])

    def test_count_enum(self) -> None:
        clist = self.Lists(
            # pyre-ignore[6]: TODO: Thrift-Container init
            colorList=self.to_list([self.Color.red, self.Color.red, self.Color.blue])
        )
        self.assertEqual(clist.colorList.count(self.Color.red), 2)
        self.assertEqual(clist.colorList.count(self.Color.blue), 1)
        self.assertEqual(clist.colorList.count(self.Color.green), 0)
        if self.is_mutable_run:
            self.assertEqual(clist.colorList.count(0), 0)
            self.assertEqual(clist.colorList.count(1), 0)
            self.assertEqual(clist.colorList.count(2), 0)
        else:
            # gross
            self.assertEqual(clist.colorList.count(0), 2)
            self.assertEqual(clist.colorList.count(1), 1)
            self.assertEqual(clist.colorList.count(2), 0)

    def test_adapted_lists(self) -> None:
        int_list = [1, 2, 3]

        # pyre-ignore[6]: TODO: Thrift-Container init
        self.assertEqual(int_list, self.AtoIValueList(self.to_list(int_list)))

    def test_unicode_const(self) -> None:
        self.assertEqual(self.unicode_list, ["Bulgaria", "Benin", "Saint Barthélemy"])
