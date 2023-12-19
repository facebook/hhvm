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

import itertools
import unittest

from folly.iobuf import IOBuf

from testing.thrift_types import (
    easy,
    EasyList,
    I32List,
    int_list,
    StringList,
    StrList2D,
)
from thrift.python.test.containers.thrift_types import Foo, Lists


class ListTests(unittest.TestCase):
    def test_negative_indexes(self) -> None:
        length = len(int_list)
        for i in range(length):
            self.assertEqual(int_list[i], int_list[i - length])

        with self.assertRaises(IndexError):
            int_list[-length - 1]

    def test_list_of_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            I32List([None, None, None])

    def test_list_creation_with_list_items(self) -> None:
        a = ["one", "two", "three"]
        b = ["cyan", "magenta", "yellow"]
        c = ["foo", "bar", "baz"]
        d = ["I", "II", "III"]
        StrList2D([a, b, c, d])
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            StrList2D([a, [None]])

    def test_list_add(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = int_list + other_list
        self.assertIsInstance(new_list, list)
        # Insure the items from both lists are in the new_list
        self.assertEqual(new_list, list(itertools.chain(int_list, other_list)))

    def test_list_radd(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = other_list + int_list
        self.assertIsInstance(new_list, list)
        self.assertEqual(new_list, list(itertools.chain(other_list, int_list)))
        # pyre-fixme[58]: special Thrift list supports concat with a tuple
        new_list = tuple(other_list) + int_list
        self.assertIsInstance(new_list, tuple)

    def test_list_creation(self) -> None:
        I32List(range(10))
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            I32List([1, "b", "c", "four"])

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
        y = I32List([1, 2, 3, 4, 5])
        z = I32List([1, 2, 3, 1, 10])

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

        x2 = I32List([1, 2, 3, 4])
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
        StringList(["hello", "world"])
        with self.assertRaises(TypeError):
            StringList("hello")

    def test_count(self) -> None:
        x = I32List([1, 2, 3, 4, 1, 2, 3, 4])
        y = list(x)
        self.assertEqual(x.count(2), y.count(2))
        self.assertEqual(x.count(5), y.count(5))

    def test_struct_list(self) -> None:
        a = EasyList([easy()])
        b = EasyList([easy(val=0)])
        c = EasyList([easy(val=1)])
        d = EasyList([easy(val_list=[])])
        e = EasyList([easy(val_list=[1])])
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

    def test_struct_with_list_fields(self) -> None:
        s = Lists(
            boolList=[True, False],
            byteList=[1, 2, 3],
            i16List=[4, 5, 6],
            i64List=[7, 8, 9],
            doubleList=[1.23, 4.56],
            floatList=[7.89, 10.11],
            stringList=["foo", "bar"],
            binaryList=[b"foo", b"bar"],
            iobufList=[IOBuf(b"foo"), IOBuf(b"bar")],
            structList=[Foo(value=1), Foo(value=2)],
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
        self.assertEqual(s.structList, [Foo(value=1), Foo(value=2)])
        # test reaccess the list element not recreating the struct
        self.assertIs(s.structList[0], s.structList[0])
        self.assertIs(s.structList[1], s.structList[1])
