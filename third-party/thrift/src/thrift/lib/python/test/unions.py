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

from testing.thrift_types import ComplexUnion, Digits, Integers, ReservedUnion
from thrift.python.serializer import deserialize, serialize_iobuf
from thrift.python.types import Union


class UnionTests(unittest.TestCase):
    def test_constructor(self) -> None:
        self.assertEqual(Integers(small=2).type, Integers.Type.small)
        self.assertEqual(Integers(unbounded="123").type, Integers.Type.unbounded)
        self.assertEqual(
            Integers(
                digits=Digits(
                    data=[Integers(tiny=1), Integers(small=2), Integers(large=3)]
                )
            ).type,
            Integers.Type.digits,
        )

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
        x = deserialize(Integers, serialize_iobuf(Integers()))
        self.assertEqual(x.type, Integers.Type.EMPTY)

    def test_deserialize_nonempty(self) -> None:
        x = deserialize(Integers, serialize_iobuf(Integers(tiny=42)))
        self.assertEqual(x.type, Integers.Type.tiny)

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
        string = "123"
        union = Integers.fromValue(tiny)
        self.assertEqual(union.type, Integers.Type.tiny)
        union = Integers.fromValue(small)
        self.assertEqual(union.type, Integers.Type.small)
        union = Integers.fromValue(medium)
        self.assertEqual(union.type, Integers.Type.medium)
        union = Integers.fromValue(large)
        self.assertEqual(union.type, Integers.Type.large)
        union = Integers.fromValue(string)
        self.assertEqual(union.type, Integers.Type.unbounded)
        union = Integers.fromValue(
            Digits(data=[Integers(tiny=1), Integers(unbounded="123")])
        )
        self.assertEqual(union.type, Integers.Type.digits)

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

    def test_union_ordering(self) -> None:
        x = Integers(tiny=4)
        y = ComplexUnion(tiny=1)
        with self.assertRaises(TypeError):
            # flake8: noqa: B015 intentionally introduced for test
            x < y
        # same type, compare value
        y = Integers(tiny=2)
        self.assertGreater(x, y)
        self.assertLess(y, x)
        # different type
        y = Integers(small=2)
        self.assertLess(x, y)
        self.assertGreater(y, x)
        # equality
        y = Integers(tiny=4)
        self.assertEqual(x, y)
