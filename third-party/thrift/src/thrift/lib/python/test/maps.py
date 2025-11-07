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

from collections.abc import ItemsView, KeysView, ValuesView

from enum import Enum, IntEnum
from typing import Dict, Type, TypeVar

import python_test.containers.thrift_mutable_types as mutable_containers_types
import python_test.containers.thrift_types as immutable_containers_types

import python_test.maps.thrift_mutable_types as mutable_maps_types
import python_test.maps.thrift_types as immutable_maps_types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from python_test.containers.thrift_types import (
    Color as ColorType,
    Foo as FooType,
    Maps as ImmutableMaps,
    Maps as MapsType,
)
from python_test.maps.thrift_types import (
    constant_map,
    easy as easyType,
    F14MapFollyString as F14MapFollyStringType,
    StrAtoIValueMap as StrAtoIValueMapType,
    StrEasyMap as StrEasyMapType,
    StrI32ListMap as StrI32ListMapType,
    StrIntMap as StrIntMapType,
    StrStrIntListMapMap as StrStrIntListMapMapType,
    StrStrMap as StrStrMapType,
)
from thrift.python.mutable_types import (
    _ThriftListWrapper,
    _ThriftMapWrapper,
    to_thrift_list,
    to_thrift_map,
)

ListT = TypeVar("ListT")
MapKey = TypeVar("MapKey")
MapValue = TypeVar("MapValue")


class MyStringEnum(str, Enum):
    test = "test"


class ImmutableMapTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(immutable_maps_types.StrI32ListMap())
        x = immutable_maps_types.StrStrIntListMapMap(
            {"foo": immutable_maps_types.StrI32ListMap()}
        )
        hash(x["foo"])

    def test_constant_map(self) -> None:
        self.assertEqual(constant_map["1"], 1)
        self.assertEqual(constant_map["2"], 2)
        self.assertEqual(constant_map["3"], 3)

        with self.assertRaisesRegex(
            TypeError,
            "'thrift.python.types.Map' object does not support item assignment",
        ):
            # pyre-ignore[16]: `typing.Mapping` has no attribute `__setitem__`
            constant_map["4"] = 4


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
        self.StrAtoIValueMap: Type[StrAtoIValueMapType] = (
            self.maps_types.StrAtoIValueMap
        )
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

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def to_map(
        self, map_data: dict[MapKey, MapValue]
    ) -> dict[MapKey, MapValue] | _ThriftMapWrapper:
        return to_thrift_map(map_data) if self.is_mutable_run else map_data

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
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.StrStrMap(self.to_map({"test": "value"}))
        self.assertEqual(x["test"], "value")
        self.assertEqual(x[MyStringEnum.test.value], "value")
        with self.assertRaises(TypeError if self.is_mutable_run else KeyError):
            # pyre-ignore[6]: Intentional for test
            x[5]
        with self.assertRaises(TypeError if self.is_mutable_run else KeyError):
            # pyre-ignore[6]: Intentional for test
            x[x]

    def test_get(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.StrStrMap(self.to_map({"test": "value"}))
        self.assertEqual(x.get("test"), "value")
        if not self.is_mutable_run:
            # pyre-ignore[6]: Intentional for test
            self.assertIs(x.get(5), None)
            # pyre-ignore[6]: Intentional for test
            self.assertIs(x.get(x), None)

    def test_contains(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.StrStrMap(self.to_map({"test": "value"}))
        self.assertIn("test", x)
        self.assertNotIn(5, x)
        self.assertNotIn(x, x)

    def test_contains_enum(self) -> None:
        cmap = self.Maps(
            # pyre-ignore[6]: TODO: Thrift-Container init
            colorMap=self.to_map({c: c for c in [self.Color.red, self.Color.blue]})
        )
        self.assertIn(self.Color.red, cmap.colorMap)
        self.assertIn(self.Color.blue, cmap.colorMap)
        self.assertNotIn(self.Color.green, cmap.colorMap)

    def test_dict_views(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        dmap = self.StrStrMap(self.to_map({"test": "value"}))
        dmap_keys = dmap.keys()
        dmap_values = dmap.values()
        dmap_items = dmap.items()
        self.assertEqual(len(dmap_keys), 1)
        self.assertEqual(len(dmap_values), 1)
        self.assertEqual(len(dmap_items), 1)
        self.assertIsInstance(dmap_keys, KeysView)
        self.assertIsInstance(dmap_values, ValuesView)
        self.assertIsInstance(dmap_items, ItemsView)

    def test_items_values(self) -> None:
        x = {"test": "value"}
        # pyre-ignore[6]: TODO: Thrift-Container init
        tx = self.StrStrMap(self.to_map(x))
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))

    def test_empty(self) -> None:
        self.StrIntMap()
        # pyre-ignore[6]: TODO: Thrift-Container init
        self.StrIntMap(self.to_map({}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        self.StrStrIntListMapMap(self.to_map({}))
        # pyre-ignore[6]: Fixme: type error to be addressed later
        self.StrStrIntListMapMap(self.to_map({"foo": {}}))
        # pyre-ignore[6]: Fixme: type error to be addressed later
        self.StrStrIntListMapMap(self.to_map({"foo": {"bar": []}}))

    def test_mixed_construction(self) -> None:
        if self.is_mutable_run:
            # TODO: remove after implementing `to_thrift_map()`
            return

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
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.StrIntMap(self.to_map({"foo": 5, "bar": 4}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.StrIntMap(self.to_map({"foo": 4, "bar": 5}))
        self.assertNotEqual(x, y)
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.StrIntMap(self.to_map({"foo": 5, "bar": 4}))
        self.assertEqual(x, y)
        self.assertEqual(x, x)
        self.assertEqual(y, y)

    def test_custom_cpp_type(self) -> None:
        x = {"foo": "foo_value"}
        # pyre-ignore[6]: TODO: Thrift-Container init
        tx = self.F14MapFollyString(self.to_map(x))
        self.assertEqual(x["foo"], tx["foo"])
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))

    def test_struct_in_map(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        b = self.StrEasyMap(self.to_map({"a": self.easy(val=0)}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        a = self.StrEasyMap(self.to_map({"a": self.easy()}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        c = self.StrEasyMap(self.to_map({"a": self.easy(val=1)}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        d = self.StrEasyMap(self.to_map({"a": self.easy(val_list=self.to_list([]))}))
        self.assertEqual(a, b)
        self.assertEqual(a, d)
        self.assertNotEqual(a, c)

    def test_struct_with_map_fields(self) -> None:
        s = self.Maps(
            # pyre-ignore[6]: TODO: Thrift-Container init
            boolMap=self.to_map({True: True, False: False}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            byteMap=self.to_map({1: 1, 2: 2, 3: 3}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i16Map=self.to_map({4: 4, 5: 5, 6: 6}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i64Map=self.to_map({7: 7, 8: 8, 9: 9}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            doubleMap=self.to_map({1.23: 1.23, 4.56: 4.56}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            floatMap=self.to_map({7.89: 7.89, 10.11: 10.11}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            stringMap=self.to_map({"foo": "foo", "bar": "bar"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            binaryMap=self.to_map({b"foo": b"foo", b"bar": b"bar"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufMap=self.to_map(
                {IOBuf(b"foo"): IOBuf(b"foo"), IOBuf(b"bar"): IOBuf(b"bar")}
            ),
            # pyre-ignore[6]: TODO: Thrift-Container init
            structMap=(
                to_thrift_map({})
                if self.is_mutable_run
                else {
                    self.Foo(value=1): self.Foo(value=1),
                    self.Foo(value=2): self.Foo(value=2),
                }
            ),
        )
        self.assertEqual(s.boolMap, {True: True, False: False})
        self.assertEqual(s.i16Map, {4: 4, 5: 5, 6: 6})
        self.assertEqual(s.i64Map, {7: 7, 8: 8, 9: 9})
        self.assertEqual(s.doubleMap, {1.23: 1.23, 4.56: 4.56})
        self.assertEqual(s.floatMap, {7.89: 7.89, 10.11: 10.11})
        self.assertEqual(s.stringMap, {"foo": "foo", "bar": "bar"})
        self.assertEqual(s.byteMap, {1: 1, 2: 2, 3: 3})
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

    def test_adapted_maps(self) -> None:
        the_map = {"foo": 1, "bar": 2}

        # pyre-ignore[6]: TODO: Thrift-Container init
        self.assertEqual(the_map, self.StrAtoIValueMap(self.to_map(the_map)))

    def test_map_module_name(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        easy_map = self.StrEasyMap(self.to_map({"a": self.easy()}))
        if self.is_mutable_run:
            self.assertEqual(
                easy_map.__class__.__module__, "thrift.python.mutable_containers"
            )
        else:
            self.assertEqual(easy_map.__class__.__module__, "thrift.python.types")


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
        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap[0], self.Color.red)
        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap[1], self.Color.blue)

        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap.get(0), self.Color.red)
        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap.get(1), self.Color.blue)
        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap.get(2), None)

    def test_lazy_map_str_str(self) -> None:
        # map<str, str>
        s = ImmutableMaps(stringMap={"1": "1", "2": "2", "3": "3"})

        class StringEnum(str, Enum):
            one = "1"
            two = "2"

        self.assertEqual(s.stringMap[StringEnum.one], "1")
        self.assertEqual(s.stringMap["1"], "1")
        self.assertEqual(s.stringMap[StringEnum.two], "2")
        self.assertEqual(s.stringMap["2"], "2")
        for key, value in s.stringMap.items():
            self.assertIs(type(key), str)
            self.assertIs(type(value), str)

    def test_lazy_map_i32_str(self) -> None:
        # map<i32, str>
        s = ImmutableMaps(i32_string_map={1: "1", 2: "2", 3: "3"})

        class MyIntEnum(IntEnum):
            one = 1
            two = 2

        self.assertEqual(s.i32_string_map[MyIntEnum.one], "1")
        self.assertEqual(s.i32_string_map[1], "1")
        self.assertEqual(s.i32_string_map[MyIntEnum.two], "2")
        self.assertEqual(s.i32_string_map[2], "2")

        for key, value in s.i32_string_map.items():
            self.assertIs(type(key), int)
            self.assertIs(type(value), str)

    def test_lazy_map_float_str(self) -> None:
        # map<float, str>
        s = ImmutableMaps(float_string_map={1.0: "1", 2.0: "2", 3.0: "3"})

        class MyFloatType(float):
            def __init__(self, value):
                self.value = float(value)

            def __float__(self):
                return float(self.value)

        self.assertEqual(s.float_string_map[MyFloatType(1.0)], "1")
        self.assertEqual(s.float_string_map[1.0], "1")
        self.assertEqual(s.float_string_map[MyFloatType(2.0)], "2")
        self.assertEqual(s.float_string_map[2.0], "2")

        for key, value in s.float_string_map.items():
            self.assertIs(type(key), float)
            self.assertIs(type(value), str)

    def test_lazy_map_double_str(self) -> None:
        # map<double, str>
        s = ImmutableMaps(double_string_map={1.0: "1", 2.0: "2", 3.0: "3"})

        class MyFloat(float):
            def __init__(self, value):
                self.value = float(value)

            def __float__(self):
                return float(self.value)

        self.assertEqual(s.double_string_map[MyFloat(1.0)], "1")
        self.assertEqual(s.double_string_map[1.0], "1")
        self.assertEqual(s.double_string_map[MyFloat(2.0)], "2")
        self.assertEqual(s.double_string_map[2.0], "2")

        for key, value in s.double_string_map.items():
            self.assertIs(type(key), float)
            self.assertIs(type(value), str)


# TODO: Collapse these two test cases into parameterized test above
class MapMutablePythonTests(unittest.TestCase):
    Color = mutable_containers_types.Color
    Maps = mutable_containers_types.Maps

    # this test case documents behavior divergences from thrift-python
    @unittest.expectedFailure
    def test_contains_enum(self) -> None:
        # pyre-ignore[6]: Fixme: type error to be addressed later
        cmap = self.Maps(colorMap={c: c for c in [self.Color.red, self.Color.blue]})
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertNotIn("str", cmap.colorMap)
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertIn(0, cmap.colorMap)

        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap.get(0), self.Color.red)
        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap.get(1), self.Color.blue)
        # pyre-ignore[6]: Intentional for test
        self.assertEqual(cmap.colorMap.get(2), None)
