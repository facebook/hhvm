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

from testing.types import easy, I32List, int_list, StringList, StrList2D, Uint32List
from thrift.py3.types import Container


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

    def test_list_add(self) -> None:
        other_list = [99, 88, 77, 66, 55]
        new_list = int_list + other_list
        self.assertIsInstance(new_list, type(int_list))
        # Insure the items from both lists are in the new_list
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

    def test_is_container(self) -> None:
        self.assertIsInstance(int_list, Container)
        self.assertIsInstance(I32List([1, 2, 3]), Container)
        self.assertIsInstance(StrList2D([["a", "b"], ["c", "d"]]), Container)

    def test_string_list(self) -> None:
        StringList(["hello", "world"])
        with self.assertRaises(TypeError):
            StringList("hello")

    def test_custom_cpp_type_list(self) -> None:
        x = [1, 2, 3, 4]
        y = Uint32List(x)
        self.assertEqual(y[1], 2)
        self.assertEqual(list(y), x)
