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

import python_test.maps.thrift_mutable_types as mutable_maps_types
import python_test.maps.thrift_types as immutable_maps_types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from python_test.containers.thrift_types import (
    Color as ColorType,
    Foo as FooType,
    Maps as MapsType,
)
from python_test.maps.thrift_types import (
    easy as easyType,
    F14MapFollyString as F14MapFollyStringType,
    StrEasyMap as StrEasyMapType,
    StrI32ListMap as StrI32ListMapType,
    StrIntMap as StrIntMapType,
    StrStrIntListMapMap as StrStrIntListMapMapType,
    StrStrMap as StrStrMapType,
)


class MyStringEnum(str, Enum):
    test = "test"


class ImmutableMapTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(immutable_maps_types.StrI32ListMap())
        x = immutable_maps_types.StrStrIntListMapMap(
            {"foo": immutable_maps_types.StrI32ListMap()}
        )
        hash(x["foo"])


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
        self.Color: Type[ColorType] = self.containers_types.Color
        self.is_mutable_run: bool = self.containers_types.__name__.endswith(
            "thrift_mutable_types"
        )

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

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            StrIntMapType().__dict__

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

    def test_contains_enum(self) -> None:
        cmap = self.Maps(colorMap={c: c for c in [self.Color.red, self.Color.blue]})
        self.assertIn(self.Color.red, cmap.colorMap)
        self.assertIn(self.Color.blue, cmap.colorMap)
        self.assertNotIn(self.Color.green, cmap.colorMap)

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
        px["foo"]["bar"] = px["foo"]["bar"] + [5, 7, 8]
        self.assertEqual(s["bar"], [0, 1])
        # Now turn this crazy mixed structure back to Cython
        cx = self.StrStrIntListMapMap(px)
        px["bar"] = {"lol": "TypeError"}
        with self.assertRaises(TypeError):
            self.StrStrIntListMapMap(px)
        self.assertNotIn("bar", cx)

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
                {}
                if self.is_mutable_run
                else {
                    self.Foo(value=1): self.Foo(value=1),
                    self.Foo(value=2): self.Foo(value=2),
                }
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
        if not self.is_mutable_run:
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


# TODO: Collapse these two test cases into parameterized test above
class MapImmutablePythonTests(unittest.TestCase):
    Color = immutable_containers_types.Color
    Maps = immutable_containers_types.Maps

    def test_contains_enum(self) -> None:
        cmap = self.Maps(colorMap={c: c for c in [self.Color.red, self.Color.blue]})
        self.assertNotIn("str", cmap.colorMap)

        # This behavior is more permissive than thrift-py3
        # which implicitly converts int to enum
        self.assertIn(0, cmap.colorMap)
        self.assertIn(1, cmap.colorMap)
        self.assertNotIn(2, cmap.colorMap)
        # gross
        self.assertEqual(cmap.colorMap[0], self.Color.red)
        self.assertEqual(cmap.colorMap[1], self.Color.blue)

        self.assertEqual(cmap.colorMap.get(0), self.Color.red)
        self.assertEqual(cmap.colorMap.get(1), self.Color.blue)
        self.assertEqual(cmap.colorMap.get(2), None)


# TODO: Collapse these two test cases into parameterized test above
class MapMutablePythonTests(unittest.TestCase):
    # pyre-ignore
    Color = mutable_containers_types.Color
    # pyre-ignore
    Maps = mutable_containers_types.Maps

    # this test case documents behavior divergences from thrift-python
    @unittest.expectedFailure
    def test_contains_enum(self) -> None:
        cmap = self.Maps(colorMap={c: c for c in [self.Color.red, self.Color.blue]})
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertNotIn("str", cmap.colorMap)
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertIn(0, cmap.colorMap)

        self.assertEqual(cmap.colorMap.get(0), self.Color.red)
        self.assertEqual(cmap.colorMap.get(1), self.Color.blue)
        self.assertEqual(cmap.colorMap.get(2), None)
