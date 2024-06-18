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

from typing import List, Type

import thrift.python.test.containers.thrift_mutable_types as mutable_containers_types
import thrift.python.test.containers.thrift_types as immutable_containers_types

import thrift.python.test.lists.thrift_mutable_types as immutable_lists_types
import thrift.python.test.lists.thrift_types as mutable_lists_types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from thrift.python.test.containers.thrift_types import Foo, Lists
from thrift.python.test.lists.thrift_types import (
    easy,
    EasyList,
    I32List,
    int_list,
    StringList,
    StrList2D,
)


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
        # pyre-ignore[16]: has no attribute `containers_types`
        self.Foo: Type[Foo] = self.containers_types.Foo
        self.Lists: Type[Lists] = self.containers_types.Lists

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
        self.StrList2D([a, b, c, d])
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.StrList2D([a, [None]])

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
        self.I32List(range(10))
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.I32List([1, "b", "c", "four"])

    def test_hashability(self) -> None:
        # Mutable types do not support hashing
        # pyre-ignore[16]: has no attribute `lists_types`
        if self.lists_types.__name__.endswith("immutable_types"):
            hash(self.easy().val_list)
            hash(self.I32List(range(10)))

    def test_index(self) -> None:
        x = self.I32List([1, 2, 3, 4, 1, 2, 3, 4])
        y = list(x)
        self.assertEqual(x.index(2), y.index(2))
        self.assertEqual(x.index(2, 3), y.index(2, 3))
        self.assertEqual(x.index(2, 0, 2), y.index(2, 0, 2))
        with self.assertRaises(ValueError):
            raise Exception(x.index(4, 0, 2))

        with self.assertRaises(ValueError):
            y.index(4, 0, 2)
        self.assertEqual(x.index(4, -20, -2), y.index(4, -20, -2))

    def test_slicing(self) -> None:
        x = self.I32List([1, 2, 3, 4, 1, 2, 3, 4])
        y = list(x)
        self.assertEqual(x[2:], y[2:])
        self.assertEqual(x[:5], y[:5])
        self.assertEqual(x[:0], y[:0])
        self.assertEqual(x[-5:-1], y[-5:-1])
        self.assertEqual(x[::-1], y[::-1])
        self.assertEqual(x[::-3], y[::-3])
        self.assertEqual(x[-1:-5:-1], y[-1:-5:-1])

    def test_comparisons(self) -> None:
        x = self.I32List([1, 2, 3, 4])
        y = self.I32List([1, 2, 3, 4, 5])
        z = self.I32List([1, 2, 3, 1, 10])

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

        x2 = self.I32List([1, 2, 3, 4])
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

    def test_string_list(self) -> None:
        self.StringList(["hello", "world"])
        with self.assertRaises(TypeError):
            self.StringList("hello")

    def test_count(self) -> None:
        x = self.I32List([1, 2, 3, 4, 1, 2, 3, 4])
        y = list(x)
        self.assertEqual(x.count(2), y.count(2))
        self.assertEqual(x.count(5), y.count(5))

    def test_struct_list(self) -> None:
        a = self.EasyList([self.easy()])
        b = self.EasyList([self.easy(val=0)])
        c = self.EasyList([self.easy(val=1)])
        d = self.EasyList([self.easy(val_list=[])])
        e = self.EasyList([self.easy(val_list=[1])])
        self.assertEqual(a, b)
        self.assertEqual(a, d)
        self.assertNotEqual(a, c)
        # DO_BEFORE(alperyoney,20240701): Implement __lt__, __le__ for MutableStruct
        # pyre-ignore[16]: has no attribute `containers_types`
        if self.containers_types.__name__.endswith("immutable_types"):
            self.assertLess(a, c)
            self.assertGreater(c, e)
            self.assertLess(d, e)
            self.assertGreater(e, d)
            self.assertLessEqual(a, b)
            self.assertGreaterEqual(a, d)
            self.assertLessEqual(a, c)
            self.assertGreaterEqual(c, e)

    def test_struct_with_list_fields(self) -> None:
        s = self.Lists(
            boolList=[True, False],
            byteList=[1, 2, 3],
            i16List=[4, 5, 6],
            i64List=[7, 8, 9],
            doubleList=[1.23, 4.56],
            floatList=[7.89, 10.11],
            stringList=["foo", "bar"],
            binaryList=[b"foo", b"bar"],
            iobufList=[IOBuf(b"foo"), IOBuf(b"bar")],
            structList=[self.Foo(value=1), self.Foo(value=2)],
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
        # test reaccess the list element not recreating the struct
        # DO_BEFORE(alperyoney,20240701): Figure out whether mutable containers
        # should cache the instance.
        # pyre-ignore[16]: has no attribute `containers_types`
        if self.containers_types.__name__.endswith("immutable_types"):
            self.assertIs(s.structList[0], s.structList[0])
            self.assertIs(s.structList[1], s.structList[1])
