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

import typing
import unittest

from apache.thrift.type.standard.thrift_types import TypeName, Void
from apache.thrift.type.type.thrift_types import Type
from folly.iobuf import IOBuf
from testing.thrift_types import Color
from thrift.python.any.serializer import (
    deserialize_list,
    deserialize_map,
    deserialize_primitive,
    deserialize_set,
    serialize_list,
    serialize_map,
    serialize_primitive,
    serialize_set,
)
from thrift.python.any.typestub import PrimitiveType, SerializableType, TKey, TValue

# @manual=//thrift/test/testset:testset-python-types
from thrift.test.testset import thrift_types


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

    def _test_list_round_trip(
        self,
        value: typing.Sequence[SerializableType],
    ) -> None:
        iobuf = serialize_list(value)
        decoded = deserialize_list(
            type(value[0]) if value else str,
            iobuf,
        )
        self.assertEqual(value, decoded)

    def test_empty_list_round_trip(self) -> None:
        self._test_list_round_trip([])

    def test_list_of_ints_round_trip(self) -> None:
        self._test_list_round_trip([1, 1, 2, 3, 5, 8])

    def test_list_of_structs_round_trip(self) -> None:
        self._test_list_round_trip(
            [
                thrift_types.struct_map_string_i32(field_1={"one": 1}),
                thrift_types.struct_map_string_i32(field_1={"two": 2}),
            ]
        )

    def test_list_of_unions_round_trip(self) -> None:
        self._test_list_round_trip(
            [
                thrift_types.union_map_string_string(field_2={"foo": "bar"}),
                thrift_types.union_map_string_string(field_2={"hello": "world"}),
            ]
        )

    def test_list_of_exceptions_round_trip(self) -> None:
        self._test_list_round_trip(
            [
                thrift_types.exception_map_string_i64(field_1={"code": 400}),
                thrift_types.exception_map_string_i64(field_1={"code": 404}),
            ]
        )

    def test_thrift_list_round_trip(self) -> None:
        self._test_list_round_trip(
            thrift_types.struct_list_i32(field_1=[1, 2, 3, 4]).field_1
        )

    def _test_set_round_trip(
        self,
        value: typing.AbstractSet[SerializableType],
    ) -> None:
        iobuf = serialize_set(value)
        decoded = deserialize_set(
            type(next(iter(value))) if value else bytes,  # doesn't matter for empty set
            iobuf,
        )
        self.assertEqual(value, decoded)

    def test_empty_set_round_trip(self) -> None:
        self._test_set_round_trip(set())

    def test_set_of_ints_round_trip(self) -> None:
        self._test_set_round_trip({1, 1, 2, 3, 5, 8})

    def test_set_of_structs_round_trip(self) -> None:
        self._test_set_round_trip(
            {
                thrift_types.struct_map_string_i32(field_1={"one": 1}),
                thrift_types.struct_map_string_i32(field_1={"two": 2}),
            }
        )

    def test_thrift_set_round_trip(self) -> None:
        self._test_set_round_trip(
            thrift_types.struct_set_i64(field_1={1, 2, 3, 4}).field_1
        )

    def _test_map_round_trip(
        self,
        original: typing.Mapping[TKey, TValue],
    ) -> None:
        iobuf = serialize_map(original)
        if original:
            k, v = next(iter(original.items()))
            key_cls = type(k)
            value_cls = type(v)
        else:
            key_cls = bool  # doesn't matter for empty dict
            value_cls = bool  # doesn't matter for empty dict
        decoded = deserialize_map(
            key_cls,
            value_cls,
            iobuf,
        )
        self.assertEqual(original, decoded)

    def test_empty_map_round_trip(self) -> None:
        self._test_map_round_trip({})

    def test_int_to_str_map_round_trip(self) -> None:
        self._test_map_round_trip({1: "one", 2: "two"})

    def test_str_to_struct_map_round_trip(self) -> None:
        self._test_map_round_trip(
            {
                "one": thrift_types.struct_map_string_i32(field_1={"one": 1}),
                "two": thrift_types.struct_map_string_i32(field_1={"two": 2}),
            }
        )

    def test_thrift_map_round_trip(self) -> None:
        self._test_map_round_trip(
            thrift_types.struct_map_string_i32(field_1={"one": 1}).field_1
        )
