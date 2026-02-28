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

import unittest
from typing import Dict, List

from testing.types import (
    Color,
    ColorGroups,
    F14MapFollyString,
    LocationMap,
    Map__Color_Color,
    Map__string_i64,
    Map__string_List__i32,
    StrI32ListMap,
    StrIntMap,
    StrStrIntListMapMap,
    StrStrMap,
)
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import (
    brokenInAutoMigrate,
    is_auto_migrated,
)
from thrift.py3.types import Container, Map as Py3Map
from thrift.python.types import Map as PythonMap


class MapTests(unittest.TestCase):
    def test_default_ctor(self) -> None:
        self.assertEqual(StrIntMap(), {})
        self.assertEqual(Map__string_i64(), {})

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

    def test_isinstance(self) -> None:
        str_int_map = {"foo": 5, "bar": 4}
        self.assertIsInstance(Map__string_i64(str_int_map), Py3Map)
        self.assertTrue(issubclass(Map__string_i64, Py3Map))
        self.assertIsInstance(Map__string_i64(str_int_map), Map__string_i64)
        color_map = {Color.red: Color.blue, Color.green: Color.red}
        self.assertIsInstance(ColorGroups(color_map=color_map).color_map, Py3Map)
        self.assertTrue(
            issubclass(ColorGroups(color_map=color_map).color_map.__class__, Py3Map)
        )
        self.assertIsInstance(
            ColorGroups(color_map=color_map).color_map, Map__Color_Color
        )
        self.assertIsInstance(StrIntMap(str_int_map), Py3Map)
        self.assertIsInstance(StrIntMap(str_int_map), Map__string_i64)

        if is_auto_migrated():
            self.assertIsInstance(Map__string_i64(str_int_map), PythonMap)
            self.assertIsInstance(ColorGroups(color_map=color_map).color_map, PythonMap)
            self.assertIsInstance(StrIntMap(str_int_map), PythonMap)

        self.assertNotIsInstance(StrIntMap(str_int_map), Map__Color_Color)
        self.assertNotIsInstance(StrIntMap(str_int_map), Map__string_List__i32)

    def test_map_views(self) -> None:
        str_int_map = StrIntMap({"foo": 5})
        self.assertEqual(len(str_int_map), 1)
        self.assertEqual(len(str_int_map.keys()), 1)
        self.assertEqual(len(str_int_map.values()), 1)
        self.assertEqual(len(str_int_map.items()), 1)

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
        # pyre-fixme[6]: For 1st argument expected `str` but got `int`.
        self.assertIs(x.get(5), None)
        # pyre-fixme[6]: For 1st argument expected `str` but got `Map__string_string`.
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

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            Map__Color_Color().__dict__

    def test_empty(self) -> None:
        StrIntMap()
        StrIntMap({})
        StrStrIntListMapMap({})
        StrStrIntListMapMap({"foo": {}})
        StrStrIntListMapMap({"foo": {"bar": []}})

    def test_mixed_construction(self) -> None:
        s = Map__string_List__i32({"bar": [0, 1]})
        x = StrStrIntListMapMap({"foo": s})
        px: Dict[str, Dict[str, List[int]]] = {}
        # pyre-fixme[6]: Expected `Dict[str, List[int]]` for 2nd param but got
        #  `Mapping[str, typing.Sequence[int]]`.
        px["foo"] = x["foo"]
        px["baz"] = {"wat": [4]}
        px["foo"] = dict(px["foo"])
        px["foo"]["bar"] = px["foo"]["bar"] + [5, 7, 8]
        self.assertEqual(s["bar"], [0, 1])
        # Now turn this crazy mixed structure back to Cython
        cx = StrStrIntListMapMap(px)
        # pyre-fixme[6]: Expected `Dict[str, List[int]]` for 2nd param but got
        #  `Dict[str, str]`.
        px["bar"] = {"lol": "TypeError"}
        with self.assertRaises(TypeError):
            StrStrIntListMapMap(px)
        self.assertNotIn("bar", cx)

    # in thrift-python, these are MapTypeFactory
    @brokenInAutoMigrate()
    def test_module_name(self) -> None:
        self.assertEqual(StrI32ListMap.__module__, "testing.types")
        self.assertEqual(StrIntMap.__module__, "testing.types")
        self.assertEqual(StrStrIntListMapMap.__module__, "testing.types")
        self.assertEqual(StrStrMap.__module__, "testing.types")

    def test_hashability(self) -> None:
        hash(StrI32ListMap())
        x = StrStrIntListMapMap({"foo": StrI32ListMap()})
        hash(x["foo"])

    def test_equality(self) -> None:
        x = StrIntMap({"foo": 5, "bar": 4})
        y = Map__string_i64({"foo": 4, "bar": 5})
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
