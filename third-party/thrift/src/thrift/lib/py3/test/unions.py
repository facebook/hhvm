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

from folly.iobuf import IOBuf
from testing.types import Color, ComplexUnion, easy, Integers, IOBufUnion, ReservedUnion
from thrift.py3.common import Protocol
from thrift.py3.serializer import deserialize
from thrift.py3.types import Union


class UnionTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(Integers())

    def test_union_dir(self) -> None:
        expected = [
            "digits",
            "large",
            "medium",
            "name_",
            "small",
            "tiny",
            "type",
            "unbounded",
            "value",
        ]
        self.assertEqual(expected, dir(Integers()))
        self.assertEqual(expected, dir(Integers))

    def test_union_enum_dir(self) -> None:
        contents = dir(Integers.Type)
        self.assertEqual(len(contents), 4 + len(Integers.Type))
        self.assertIn("__module__", contents)
        self.assertIn("__class__", contents)
        self.assertIn("__doc__", contents)
        self.assertIn("__members__", contents)
        for itype in iter(Integers.Type):
            self.assertTrue(itype.name in contents)

    def test_union_enum_members(self) -> None:
        members = Integers.Type.__members__
        # Alias can't happen in this enum so these should always equal
        self.assertEqual(len(members), len(Integers.Type))
        for type in Integers.Type:
            self.assertIn(type.name, members)
            self.assertIs(type, members[type.name])

    def test_deserialize_empty(self) -> None:
        x = deserialize(Integers, b"{}", Protocol.JSON)
        self.assertEqual(x.type, Integers.Type.EMPTY)

    def test_union_usage(self) -> None:
        value = hash("i64")
        x = Integers(large=value)
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, x.get_type())
        self.assertEqual(x.type, Integers.Type.large)
        self.assertEqual(x.value, value)
        # Hashing Works
        s = {x}
        self.assertIn(x, s)
        # Repr is useful
        rx = repr(x)
        self.assertIn(str(value), rx)
        self.assertIn(x.type.name, rx)

    def test_multiple_values(self) -> None:
        with self.assertRaises(TypeError):
            Integers(small=1, large=2)

    def test_wrong_type(self) -> None:
        x = Integers(small=1)
        with self.assertRaises(AttributeError):
            x.large
        x.small

    def test_integers_fromValue(self) -> None:
        tiny = 2**7 - 1
        small = 2**15 - 1
        medium = 2**31 - 1
        large = 2**63 - 1
        union = Integers.fromValue(tiny)
        self.assertEqual(union.type, Integers.Type.tiny)
        union = Integers.fromValue(small)
        self.assertEqual(union.type, Integers.Type.small)
        union = Integers.fromValue(medium)
        self.assertEqual(union.type, Integers.Type.medium)
        union = Integers.fromValue(large)
        self.assertEqual(union.type, Integers.Type.large)

    def test_complexunion_fromValue(self) -> None:
        tiny = 2**7 - 1
        large = 2**63 - 1
        afloat = 3.141592025756836  # Hand crafted to be representable as float
        adouble = 3.14159265358
        union = ComplexUnion.fromValue(tiny)
        self.assertEqual(union.type, ComplexUnion.Type.tiny)
        union = ComplexUnion.fromValue(large)
        self.assertEqual(union.type, ComplexUnion.Type.large)
        union = ComplexUnion.fromValue(afloat)
        self.assertEqual(union.value, afloat)
        self.assertEqual(union.type, ComplexUnion.Type.float_val)
        union = ComplexUnion.fromValue(adouble)
        self.assertEqual(union.value, adouble)
        self.assertEqual(union.type, ComplexUnion.Type.double_val)
        union = ComplexUnion.fromValue(Color.red)
        self.assertEqual(union.type, ComplexUnion.Type.color)
        union = ComplexUnion.fromValue(easy())
        self.assertEqual(union.type, ComplexUnion.Type.easy_struct)
        union = ComplexUnion.fromValue("foo")
        self.assertEqual(union.type, ComplexUnion.Type.text)
        union = ComplexUnion.fromValue(b"ar")
        self.assertEqual(union.type, ComplexUnion.Type.raw)
        union = ComplexUnion.fromValue(True)
        self.assertEqual(union.type, ComplexUnion.Type.truthy)

    def test_iobuf_union(self) -> None:
        abuf = IOBuf(b"3.141592025756836")
        union = IOBufUnion.fromValue(abuf)
        self.assertEqual(union.type, IOBufUnion.Type.buf)
        self.assertEqual(bytes(union.buf), bytes(abuf))

    def test_reserved_union(self) -> None:
        x = ReservedUnion(from_="foo")
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, ReservedUnion.Type.from_)
        self.assertEqual(x.value, "foo")
        self.assertEqual(x.from_, "foo")

        x = ReservedUnion(nonlocal_=3)
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, ReservedUnion.Type.nonlocal_)
        self.assertEqual(x.value, 3)
        self.assertEqual(x.nonlocal_, 3)

        x = ReservedUnion(ok="bar")
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, ReservedUnion.Type.ok)
        self.assertEqual(x.value, "bar")
        self.assertEqual(x.ok, "bar")
