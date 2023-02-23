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

import typing
import unittest

from apache.thrift.type.standard.thrift_types import TypeName, Void
from apache.thrift.type.type.thrift_types import Type
from folly.iobuf import IOBuf
from testing.thrift_types import Color

from thrift.python.any.serializer import deserialize_primitive, serialize_primitive


if typing.TYPE_CHECKING:
    from thrift.python.any.serializer import PrimitiveType


class SerializerTests(unittest.TestCase):
    def _test_round_trip(
        self, value: PrimitiveType, thrift_type: typing.Optional[Type] = None
    ) -> None:
        iobuf = serialize_primitive(value, thrift_type=thrift_type)
        decoded = deserialize_primitive(type(value), iobuf, thrift_type=thrift_type)
        self.assertIs(type(value), type(decoded))
        if isinstance(value, float):
            assert isinstance(decoded, float)
            self.assertAlmostEqual(float(value), float(decoded), places=3)
        else:
            self.assertEqual(value, decoded)

    def test_bool_round_trip(self) -> None:
        self._test_round_trip(True)

    def test_int_round_trip(self) -> None:
        self._test_round_trip(42)

    def test_float_round_trip(self) -> None:
        self._test_round_trip(123456.789)

    def test_str_round_trip(self) -> None:
        self._test_round_trip("thrift-python")

    def test_bytes_round_trip(self) -> None:
        self._test_round_trip(b"raw bytes")

    def test_iobuf_round_trip(self) -> None:
        self._test_round_trip(IOBuf(b"iobuf"))

    def test_enum_round_trip(self) -> None:
        self._test_round_trip(Color.green)

    def _test_round_trip_with_type_names(
        self, value: PrimitiveType, type_names: typing.Sequence[TypeName]
    ) -> None:
        for type_name in type_names:
            with self.subTest(type_name=type_name):
                self._test_round_trip(value, thrift_type=Type(name=type_name))

    def test_int_round_trip_with_type_name(self) -> None:
        self._test_round_trip_with_type_names(
            42,
            [
                TypeName(byteType=Void.Unused),
                TypeName(i16Type=Void.Unused),
                TypeName(i32Type=Void.Unused),
                TypeName(i64Type=Void.Unused),
            ],
        )

    def test_float_round_trip_with_type_name(self) -> None:
        self._test_round_trip_with_type_names(
            123456.789,
            [
                TypeName(floatType=Void.Unused),
                TypeName(doubleType=Void.Unused),
            ],
        )
