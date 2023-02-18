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

import unittest

from apache.thrift.type.standard.thrift_types import TypeName, Void
from apache.thrift.type.type.thrift_types import Type

from thrift.python.any.serializer import deserialize_primitive, serialize_primitive


class SerializerTests(unittest.TestCase):
    def test_int_round_trip(self) -> None:
        i = 42
        iobuf = serialize_primitive(i)
        decoded = deserialize_primitive(int, iobuf)
        self.assertEqual(i, decoded)

    def test_int_round_trip_with_type_name(self) -> None:
        i = 42
        for type_name in [
            TypeName(i16Type=Void.Unused),
            TypeName(i32Type=Void.Unused),
            TypeName(i64Type=Void.Unused),
        ]:
            with self.subTest(type_name=type_name):
                iobuf = serialize_primitive(i, thrift_type=Type(name=type_name))
                decoded = deserialize_primitive(
                    int, iobuf, thrift_type=Type(name=type_name)
                )
                self.assertEqual(i, decoded)

    def test_float_round_trip(self) -> None:
        f = 123456.789
        iobuf = serialize_primitive(f)
        decoded = deserialize_primitive(float, iobuf)
        self.assertAlmostEqual(f, decoded, delta=0.001)

    def test_float_round_trip_with_type_name(self) -> None:
        f = 123456.789
        for type_name in [
            TypeName(floatType=Void.Unused),
            TypeName(doubleType=Void.Unused),
        ]:
            with self.subTest(type_name=type_name):
                iobuf = serialize_primitive(f, thrift_type=Type(name=type_name))
                decoded = deserialize_primitive(
                    float, iobuf, thrift_type=Type(name=type_name)
                )
                self.assertAlmostEqual(f, decoded, delta=0.001)
