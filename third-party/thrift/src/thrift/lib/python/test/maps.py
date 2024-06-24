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

import unittest

from enum import Enum
from typing import Dict, Type

import python_test.containers.thrift_mutable_types as mutable_containers_types
import python_test.containers.thrift_types as immutable_containers_types

import python_test.maps.thrift_mutable_types as immutable_maps_types
import python_test.maps.thrift_types as mutable_maps_types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from python_test.containers.thrift_types import Foo as FooType, Maps as MapsType
from python_test.maps.thrift_types import (
    easy as easyType,
    F14MapFollyString as F14MapFollyStringType,
    LocationMap,
    StrEasyMap as StrEasyMapType,
    StrI32ListMap as StrI32ListMapType,
    StrIntMap as StrIntMapType,
    StrStrIntListMapMap as StrStrIntListMapMapType,
    StrStrMap as StrStrMapType,
)


class MyStringEnum(str, Enum):
    test = "test"


@parameterized_class(
    ("containers_types", "maps_types"),
    [
        (immutable_containers_types, immutable_maps_types),
        (mutable_containers_types, mutable_maps_types),
    ],
)
class MapTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `containers_types` and `maps_types`.
        """
        # pyre-ignore[16]: has no attribute `sets_types`
        self.LocationMap: Dict[int, Dict[int, int]] = self.maps_types.LocationMap
        self.StrIntMap: Type[StrIntMapType] = self.maps_types.StrIntMap
        self.StrStrIntListMapMap: Type[StrStrIntListMapMapType] = (
            self.maps_types.StrStrIntListMapMap
        )
        self.StrStrMap: Type[StrStrMapType] = self.maps_types.StrStrMap
        self.StrI32ListMap: Type[StrI32ListMapType] = self.maps_types.StrI32ListMap
        self.F14MapFollyString: Type[F14MapFollyStringType] = (
            self.maps_types.F14MapFollyString
        )
        self.StrEasyMap: Type[StrEasyMapType] = self.maps_types.StrEasyMap
        self.easy: Type[easyType] = self.maps_types.easy
        # pyre-ignore[16]: has no attribute `containers_types`
        self.Foo: Type[FooType] = self.containers_types.Foo
        self.Maps: Type[MapsType] = self.containers_types.Maps

    def test_recursive_const_map(self) -> None:
        self.assertEqual(self.LocationMap[1][1], 1)

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.StrIntMap({None: 5})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.StrIntMap({"foo": None})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.StrStrIntListMapMap({"bar": {"foo": [None, None]}})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.StrStrIntListMapMap({"bar": {"foo": None}})

    def test_getitem(self) -> None:
        x = self.StrStrMap({"test": "value"})
        self.assertEqual(x["test"], "value")
        self.assertEqual(x[MyStringEnum.test], "value")
        with self.assertRaises(KeyError):
            x[5]
        with self.assertRaises(KeyError):
            x[x]

    def test_get(self) -> None:
        x = self.StrStrMap({"test": "value"})
        self.assertEqual(x.get("test"), "value")
        self.assertIs(x.get(5), None)
        self.assertIs(x.get(x), None)

    def test_contains(self) -> None:
        x = self.StrStrMap({"test": "value"})
        self.assertIn("test", x)
        self.assertNotIn(5, x)
        self.assertNotIn(x, x)

    def test_items_values(self) -> None:
        x = {"test": "value"}
        tx = self.StrStrMap(x)
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))

    def test_empty(self) -> None:
        self.StrIntMap()
        self.StrIntMap({})
        self.StrStrIntListMapMap({})
        self.StrStrIntListMapMap({"foo": {}})
        self.StrStrIntListMapMap({"foo": {"bar": []}})

    def test_mixed_construction(self) -> None:
        s = self.StrI32ListMap({"bar": [0, 1]})
        x = self.StrStrIntListMapMap({"foo": s})
        px = {}
        px["foo"] = x["foo"]
        px["baz"] = {"wat": [4]}
        px["foo"] = dict(px["foo"])
        # DO_BEFORE(alperyoney,20240801): Implement '+' for `MutableList`
        # pyre-ignore[16]: has no attribute `maps_types`
        if self.maps_types.__name__.endswith("immutable_types"):
            px["foo"]["bar"] = px["foo"]["bar"] + [5, 7, 8]
        self.assertEqual(s["bar"], [0, 1])
        # Now turn this crazy mixed structure back to Cython
        cx = self.StrStrIntListMapMap(px)
        px["bar"] = {"lol": "TypeError"}
        with self.assertRaises(TypeError):
            self.StrStrIntListMapMap(px)
        self.assertNotIn("bar", cx)

    def test_hashability(self) -> None:
        # Mutable types do not support hashing
        # pyre-ignore[16]: has no attribute `lists_types`
        if self.maps_types.__name__.endswith("immutable_types"):
            hash(self.StrI32ListMap())
            x = self.StrStrIntListMapMap({"foo": self.StrI32ListMap()})
            hash(x["foo"])

    def test_equality(self) -> None:
        x = self.StrIntMap({"foo": 5, "bar": 4})
        y = self.StrIntMap({"foo": 4, "bar": 5})
        self.assertNotEqual(x, y)
        y = self.StrIntMap({"foo": 5, "bar": 4})
        self.assertEqual(x, y)
        self.assertEqual(x, x)
        self.assertEqual(y, y)

    def test_custom_cpp_type(self) -> None:
        x = {"foo": "foo_value"}
        tx = self.F14MapFollyString(x)
        self.assertEqual(x["foo"], tx["foo"])
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))

    def test_struct_in_map(self) -> None:
        a = self.StrEasyMap({"a": self.easy()})
        b = self.StrEasyMap({"a": self.easy(val=0)})
        c = self.StrEasyMap({"a": self.easy(val=1)})
        d = self.StrEasyMap({"a": self.easy(val_list=[])})
        self.assertEqual(a, b)
        self.assertEqual(a, d)
        self.assertNotEqual(a, c)

    def test_struct_with_map_fields(self) -> None:
        # pyre-ignore[16]: has no attribute `lists_types`
        is_immutable = self.maps_types.__name__.endswith("immutable_types")

        s = self.Maps(
            boolMap={True: True, False: False},
            byteMap={1: 1, 2: 2, 3: 3},
            i16Map={4: 4, 5: 5, 6: 6},
            i64Map={7: 7, 8: 8, 9: 9},
            doubleMap={1.23: 1.23, 4.56: 4.56},
            floatMap={7.89: 7.89, 10.11: 10.11},
            stringMap={"foo": "foo", "bar": "bar"},
            binaryMap={b"foo": b"foo", b"bar": b"bar"},
            iobufMap={IOBuf(b"foo"): IOBuf(b"foo"), IOBuf(b"bar"): IOBuf(b"bar")},
            structMap=(
                {
                    self.Foo(value=1): self.Foo(value=1),
                    self.Foo(value=2): self.Foo(value=2),
                }
                if is_immutable
                else {}
            ),
        )
        self.assertEqual(s.boolMap, {True: True, False: False})
        self.assertEqual(s.byteMap, {1: 1, 2: 2, 3: 3})
        self.assertEqual(s.i16Map, {4: 4, 5: 5, 6: 6})
        self.assertEqual(s.i64Map, {7: 7, 8: 8, 9: 9})
        self.assertEqual(s.doubleMap, {1.23: 1.23, 4.56: 4.56})
        self.assertEqual(s.floatMap, {7.89: 7.89, 10.11: 10.11})
        self.assertEqual(s.stringMap, {"foo": "foo", "bar": "bar"})
        self.assertEqual(s.binaryMap, {b"foo": b"foo", b"bar": b"bar"})
        self.assertEqual(
            s.iobufMap, {IOBuf(b"foo"): IOBuf(b"foo"), IOBuf(b"bar"): IOBuf(b"bar")}
        )
        if is_immutable:
            self.assertEqual(
                s.structMap,
                {
                    self.Foo(value=1): self.Foo(value=1),
                    self.Foo(value=2): self.Foo(value=2),
                },
            )
            # test reaccess the map element won't have to recreating the struct
            self.assertIs(
                s.structMap[self.Foo(value=1)], s.structMap[self.Foo(value=1)]
            )
            self.assertIs(
                s.structMap[self.Foo(value=2)], s.structMap[self.Foo(value=2)]
            )
