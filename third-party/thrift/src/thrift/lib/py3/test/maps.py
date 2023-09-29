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

import unittest
from typing import Dict, List

from testing.types import (
    F14MapFollyString,
    LocationMap,
    StrI32ListMap,
    StrIntMap,
    StrStrIntListMapMap,
    StrStrMap,
)
from thrift.py3.types import Container


class MapTests(unittest.TestCase):
    def test_recursive_const_map(self) -> None:
        self.assertEqual(LocationMap[1][1], 1)

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Mapping[str, int]]` for 1st
            #  param but got `Dict[None, int]`.
            StrIntMap({None: 5})
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Mapping[str, int]]` for 1st
            #  param but got `Dict[str, None]`.
            StrIntMap({"foo": None})
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Mapping[str,
            #  typing.Mapping[str, typing.Sequence[int]]]]` for 1st param but got
            #  `Dict[str, Dict[str, List[None]]]`.
            StrStrIntListMapMap({"bar": {"foo": [None, None]}})
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[typing.Mapping[str,
            #  typing.Mapping[str, typing.Sequence[int]]]]` for 1st param but got
            #  `Dict[str, Dict[str, None]]`.
            StrStrIntListMapMap({"bar": {"foo": None}})

    def test_getitem(self) -> None:
        x = StrStrMap({"test": "value"})
        self.assertEqual(x["test"], "value")
        with self.assertRaises(KeyError):
            # pyre-fixme[6]: Expected `str` for 1st param but got `int`.
            x[5]
        with self.assertRaises(KeyError):
            # pyre-fixme[6]: Expected `str` for 1st param but got `Map__string_string`.
            x[x]

    def test_get(self) -> None:
        x = StrStrMap({"test": "value"})
        self.assertEqual(x.get("test"), "value")
        self.assertIs(x.get(5), None)
        self.assertIs(x.get(x), None)

    def test_contains(self) -> None:
        x = StrStrMap({"test": "value"})
        self.assertIn("test", x)
        self.assertNotIn(5, x)
        self.assertNotIn(x, x)

    def test_items_values(self) -> None:
        x = {"test": "value"}
        tx = StrStrMap(x)
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))

    def test_empty(self) -> None:
        StrIntMap()
        StrIntMap({})
        StrStrIntListMapMap({})
        StrStrIntListMapMap({"foo": {}})
        StrStrIntListMapMap({"foo": {"bar": []}})

    def test_mixed_construction(self) -> None:
        s = StrI32ListMap({"bar": [0, 1]})
        x = StrStrIntListMapMap({"foo": s})
        px: Dict[str, Dict[str, List[int]]] = {}
        # pyre-fixme[6]: Expected `Dict[str, List[int]]` for 2nd param but got
        #  `Mapping[str, typing.Sequence[int]]`.
        px["foo"] = x["foo"]
        px["baz"] = {"wat": [4]}
        px["foo"] = dict(px["foo"])
        px["foo"]["bar"] = px["foo"]["bar"] + [5, 7, 8]
        self.assertEquals(s["bar"], [0, 1])
        # Now turn this crazy mixed structure back to Cython
        cx = StrStrIntListMapMap(px)
        # pyre-fixme[6]: Expected `Dict[str, List[int]]` for 2nd param but got
        #  `Dict[str, str]`.
        px["bar"] = {"lol": "TypeError"}
        with self.assertRaises(TypeError):
            StrStrIntListMapMap(px)
        self.assertNotIn("bar", cx)

    def test_hashability(self) -> None:
        hash(StrI32ListMap())
        x = StrStrIntListMapMap({"foo": StrI32ListMap()})
        hash(x["foo"])

    def test_equality(self) -> None:
        x = StrIntMap({"foo": 5, "bar": 4})
        y = StrIntMap({"foo": 4, "bar": 5})
        self.assertNotEqual(x, y)
        y = StrIntMap({"foo": 5, "bar": 4})
        self.assertEqual(x, y)
        self.assertEqual(x, x)
        self.assertEqual(y, y)

    def test_is_container(self) -> None:
        self.assertIsInstance(LocationMap, Container)
        self.assertIsInstance(StrI32ListMap(), Container)
        self.assertIsInstance(StrIntMap(), Container)
        self.assertIsInstance(StrStrIntListMapMap(), Container)
        self.assertIsInstance(StrStrMap(), Container)

    def test_custom_cpp_type(self) -> None:
        x = {"foo": "foo_value"}
        tx = F14MapFollyString(x)
        self.assertEqual(x["foo"], tx["foo"])
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))
