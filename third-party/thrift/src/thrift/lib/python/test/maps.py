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

from folly.iobuf import IOBuf

from testing.thrift_types import (
    easy,
    F14MapFollyString,
    LocationMap,
    StrEasyMap,
    StrI32ListMap,
    StrIntMap,
    StrStrIntListMapMap,
    StrStrMap,
)
from thrift.python.test.containers.thrift_types import Foo, Maps


class MyStringEnum(str, Enum):
    test = "test"


class MapTests(unittest.TestCase):
    def test_recursive_const_map(self) -> None:
        self.assertEqual(LocationMap[1][1], 1)

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            StrIntMap({None: 5})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            StrIntMap({"foo": None})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            StrStrIntListMapMap({"bar": {"foo": [None, None]}})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            StrStrIntListMapMap({"bar": {"foo": None}})

    def test_getitem(self) -> None:
        x = StrStrMap({"test": "value"})
        self.assertEqual(x["test"], "value")
        self.assertEqual(x[MyStringEnum.test], "value")
        with self.assertRaises(KeyError):
            x[5]
        with self.assertRaises(KeyError):
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
        px = {}
        px["foo"] = x["foo"]
        px["baz"] = {"wat": [4]}
        px["foo"] = dict(px["foo"])
        px["foo"]["bar"] = px["foo"]["bar"] + [5, 7, 8]
        self.assertEqual(s["bar"], [0, 1])
        # Now turn this crazy mixed structure back to Cython
        cx = StrStrIntListMapMap(px)
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

    def test_custom_cpp_type(self) -> None:
        x = {"foo": "foo_value"}
        tx = F14MapFollyString(x)
        self.assertEqual(x["foo"], tx["foo"])
        self.assertEqual(list(x.values()), list(tx.values()))
        self.assertEqual(list(x.keys()), list(tx.keys()))
        self.assertEqual(list(x.items()), list(tx.items()))

    def test_struct_in_map(self) -> None:
        a = StrEasyMap({"a": easy()})
        b = StrEasyMap({"a": easy(val=0)})
        c = StrEasyMap({"a": easy(val=1)})
        d = StrEasyMap({"a": easy(val_list=[])})
        self.assertEqual(a, b)
        self.assertEqual(a, d)
        self.assertNotEqual(a, c)

    def test_struct_with_map_fields(self) -> None:
        s = Maps(
            boolMap={True: True, False: False},
            byteMap={1: 1, 2: 2, 3: 3},
            i16Map={4: 4, 5: 5, 6: 6},
            i64Map={7: 7, 8: 8, 9: 9},
            doubleMap={1.23: 1.23, 4.56: 4.56},
            floatMap={7.89: 7.89, 10.11: 10.11},
            stringMap={"foo": "foo", "bar": "bar"},
            binaryMap={b"foo": b"foo", b"bar": b"bar"},
            iobufMap={IOBuf(b"foo"): IOBuf(b"foo"), IOBuf(b"bar"): IOBuf(b"bar")},
            structMap={Foo(value=1): Foo(value=1), Foo(value=2): Foo(value=2)},
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
        self.assertEqual(
            s.structMap, {Foo(value=1): Foo(value=1), Foo(value=2): Foo(value=2)}
        )
        # test reaccess the map element won't have to recreating the struct
        self.assertIs(s.structMap[Foo(value=1)], s.structMap[Foo(value=1)])
        self.assertIs(s.structMap[Foo(value=2)], s.structMap[Foo(value=2)])
