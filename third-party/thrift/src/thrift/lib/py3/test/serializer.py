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

import asyncio
import pickle
import unittest
from typing import Hashable, Mapping, Union

from apache.thrift.test.terse_write.terse_write.types import (
    EmptyStruct,
    FieldLevelTerseStruct,
    MyEnum,
    MyStruct,
    MyStructWithCustomDefault,
    TerseStructWithCustomDefault,
)
from testing.types import (
    Digits,
    easy,
    hard,
    I32List,
    Integers,
    SetI32,
    StringBucket,
    StrStrMap,
)
from thrift.py3.common import Protocol
from thrift.py3.exceptions import Error
from thrift.py3.serializer import (
    deserialize,
    deserialize_from_header,
    deserialize_with_length,
    serialize,
    serialize_iobuf,
    serialize_with_header,
    serialize_with_header_iobuf,
    Transform,
)
from thrift.py3.types import Struct


class SerializerTests(unittest.TestCase):
    def test_with_header_bytes(self) -> None:
        control = easy(val=5, val_list=[4, 3, 2, 1])
        buf = serialize_with_header(control, transform=Transform.ZSTD_TRANSFORM)
        decoded = deserialize_from_header(easy, buf)
        self.assertEqual(control, decoded)

    def test_with_header_iobuf(self) -> None:
        control = easy(val=5, val_list=[4, 3, 2, 1])
        iobuf = serialize_with_header_iobuf(control, transform=Transform.ZSTD_TRANSFORM)
        decoded = deserialize_from_header(easy, iobuf)
        self.assertEqual(control, decoded)

    def test_with_header_iobuf_binary(self) -> None:
        control = easy(val=6, val_list=[5, 4, 3, 2, 1])
        iobuf = serialize_with_header_iobuf(
            control, protocol=Protocol.BINARY, transform=Transform.ZLIB_TRANSFORM
        )
        decoded = deserialize_from_header(easy, iobuf)
        self.assertEqual(control, decoded)

    def test_with_header_iobuf_json(self) -> None:
        control = easy(val=4, val_list=[3, 2, 1])
        iobuf = serialize_with_header_iobuf(control, protocol=Protocol.JSON)
        decoded = deserialize_from_header(easy, iobuf)
        self.assertEqual(control, decoded)

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `sT` for 1st param but got `None`.
            serialize(None, Protocol.JSON)

    def test_sanity(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `sT` for 1st param but got `int`.
            serialize(1, Protocol.COMPACT)

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Protocol` for 2nd param but got `None`.
            serialize(easy(), None)

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Type[Variable[thrift.py3.serializer.sT (bound
            #  to Struct)]]` for 1st param but got `Type[Protocol]`.
            deserialize(Protocol, b"")

        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Union[bytearray, bytes, folly.iobuf.IOBuf,
            #  memoryview]` for 2nd param but got `Type[Protocol]`.
            deserialize(easy, Protocol)

    def test_from_thread_pool(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4])
        loop = asyncio.get_event_loop()
        coro = loop.run_in_executor(None, serialize, control)
        encoded = loop.run_until_complete(coro)
        coro = loop.run_in_executor(None, deserialize, type(control), encoded)
        decoded = loop.run_until_complete(coro)
        self.assertEqual(control, decoded)

    def test_serialize_iobuf(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4, 5])
        iobuf = serialize_iobuf(control)
        decoded = deserialize(type(control), iobuf)
        self.assertEqual(control, decoded)

    def test_bad_deserialize(self) -> None:
        with self.assertRaises(Error):
            deserialize(easy, b"", protocol=Protocol.JSON)
        with self.assertRaises(Error):
            deserialize(easy, b"\x05AAAAAAAA")
        with self.assertRaises(Error):
            deserialize(easy, b"\x02\xDE\xAD\xBE\xEF", protocol=Protocol.BINARY)
        with self.assertRaises(BufferError):
            deserialize_from_header(easy, b"\x02\xDE\xAD\xBE\xEF")
        with self.assertRaises(Error):
            control = easy(val=5, val_list=[4, 3, 2, 1])
            buf = serialize_with_header(control, transform=Transform.ZSTD_TRANSFORM)
            newBytes = bytearray(buf)
            newBytes[4] += 1
            deserialize_from_header(easy, bytes(newBytes))

    def thrift_serialization_round_robin(
        self, control: Struct, fixtures: Mapping[Protocol, bytes]
    ) -> None:
        for proto in Protocol:
            encoded = serialize(control, protocol=proto)
            self.assertIsInstance(encoded, bytes)
            decoded = deserialize(type(control), encoded, protocol=proto)
            self.assertIsInstance(decoded, type(control))
            self.assertEqual(control, decoded)
            self.assertEqual((proto, encoded), (proto, fixtures.get(proto)))

    def pickle_round_robin(
        self,
        control: Union[Struct, Hashable],
    ) -> None:
        encoded = pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
        decoded = pickle.loads(encoded)
        self.assertIsInstance(decoded, type(control))
        self.assertEqual(control, decoded)

    def test_serialize_easy_struct(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4])
        fixtures: Mapping[Protocol, bytes] = {
            Protocol.COMPACT: b"\x15\n\x19E\x02\x04\x06\x08,\x00\x16\x00\x00",
            Protocol.BINARY: b"\x08\x00\x01\x00\x00\x00\x05\x0f\x00\x02\x08\x00\x00\x00"
            b"\x04\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00"
            b"\x00\x00\x04\x0c\x00\x04\x00\n\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            Protocol.JSON: b'{"val":5,"val_list":[1,2,3,4],"an_int":{},"py3_hidden":0}',
            Protocol.COMPACT_JSON: b'{"1":{"i32":5},"2":{"lst":["i32",4,1,2,3,4]},"4"'
            b':{"rec":{}},"5":{"i64":0}}',
        }
        self.thrift_serialization_round_robin(control, fixtures)

    def test_pickle_easy_struct(self) -> None:
        control = easy(val=0, val_list=[5, 6, 7])
        self.pickle_round_robin(control)

    def test_serialize_hard_struct(self) -> None:
        control = hard(
            val=0, val_list=[1, 2, 3, 4], name="foo", an_int=Integers(tiny=1)
        )
        fixtures: Mapping[Protocol, bytes] = {
            Protocol.COMPACT: b"\x15\x00\x19E\x02\x04\x06\x08\x18\x03foo\x1c\x13\x01"
            b"\x00\x18\x0csome default\x00",
            Protocol.BINARY: b"\x08\x00\x01\x00\x00\x00\x00\x0f\x00\x02\x08\x00\x00\x00"
            b"\x04\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00"
            b"\x00\x00\x04\x0b\x00\x03\x00\x00\x00\x03foo\x0c\x00\x04"
            b"\x03\x00\x01\x01\x00\x0b\x00\x05\x00\x00\x00\x0csome def"
            b"ault\x00",
            Protocol.JSON: b'{"val":0,"val_list":[1,2,3,4],"name":"foo","an_int":{"tiny'
            b'":1},"other":"some default"}',
            Protocol.COMPACT_JSON: b'{"1":{"i32":0},"2":{"lst":["i32",4,1,2,3,4]},"3":'
            b'{"str":"foo"},"4":{"rec":{"1":{"i8":1}}},"5":{"str":"some default"}}',
        }
        self.thrift_serialization_round_robin(control, fixtures)

    def test_pickle_hard_struct(self) -> None:
        control = hard(
            val=0, val_list=[1, 2, 3, 4], name="foo", an_int=Integers(tiny=1)
        )
        self.pickle_round_robin(control)

    def test_serialize_Integers_union(self) -> None:
        control = Integers(medium=1337)
        fixtures: Mapping[Protocol, bytes] = {
            Protocol.COMPACT: b"5\xf2\x14\x00",
            Protocol.BINARY: b"\x08\x00\x03\x00\x00\x059\x00",
            Protocol.JSON: b'{"medium":1337}',
            Protocol.COMPACT_JSON: b'{"3":{"i32":1337}}',
        }

        self.thrift_serialization_round_robin(control, fixtures)

    def test_pickle_Integers_union(self) -> None:
        control = Integers(large=2**32)
        self.pickle_round_robin(control)

    def test_pickle_sequence(self) -> None:
        control = I32List([1, 2, 3, 4])
        self.pickle_round_robin(control)

        digits = Digits(data=[Integers(tiny=1), Integers(tiny=2), Integers(large=0)])
        data = digits.data
        assert data
        self.pickle_round_robin(data)

    def test_pickle_set(self) -> None:
        control = SetI32({1, 2, 3, 4})
        self.pickle_round_robin(control)

    def test_pickle_mapping(self) -> None:
        control = StrStrMap({"test": "test", "foo": "bar"})
        self.pickle_round_robin(control)

    def test_deserialize_with_length(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4, 5])
        for proto in Protocol:
            encoded = serialize(control, protocol=proto)
            decoded, length = deserialize_with_length(
                type(control), encoded, protocol=proto
            )
            self.assertIsInstance(decoded, type(control))
            self.assertEqual(decoded, control)
            self.assertEqual(length, len(encoded))

    def test_string_with_non_utf8_data(self) -> None:
        encoded = b"\x0b\x00\x01\x00\x00\x00\x03foo\x00"
        sb = deserialize(StringBucket, encoded, protocol=Protocol.BINARY)
        self.assertEqual("foo", sb.one)

        encoded = b"\x0b\x00\x01\x00\x00\x00\x03\xfa\xf0\xef\x00"
        sb = deserialize(StringBucket, encoded, protocol=Protocol.BINARY)
        with self.assertRaises(UnicodeDecodeError):
            # Accessing the property is when the string is decoded as UTF-8.
            sb.one

    def test_field_level_terse_write(self) -> None:
        obj = FieldLevelTerseStruct(
            bool_field=True,
            byte_field=1,
            short_field=2,
            int_field=3,
            long_field=4,
            float_field=5,
            double_field=6,
            string_field="7",
            binary_field=b"8",
            enum_field=MyEnum.ME1,
            list_field=[1],
            set_field={1},
            map_field={1: 1},
            struct_field=MyStruct(field1=1),
        )
        empty = EmptyStruct()
        for proto in Protocol:
            encoded = serialize(obj, protocol=proto)
            decoded, length = deserialize_with_length(
                type(obj), encoded, protocol=proto
            )
            self.assertIsInstance(decoded, type(obj))
            self.assertEqual(decoded, obj)
            self.assertEqual(length, len(encoded))

        # Set fields to their intrinsic default.
        obj = FieldLevelTerseStruct(
            bool_field=False,
            byte_field=0,
            short_field=0,
            int_field=0,
            long_field=0,
            float_field=0,
            double_field=0,
            string_field="",
            binary_field=b"",
            enum_field=MyEnum.ME0,
            list_field=[],
            set_field=set(),
            map_field={},
            struct_field=MyStruct(field1=0),
        )
        for proto in Protocol:
            encoded = serialize(obj, protocol=proto)
            encoded_empty = serialize(empty, protocol=proto)
            self.assertEqual(encoded, encoded_empty)

    # Since empty serializd binary is deserialized, all terse fields should equal
    # to the intrinsic default.
    def test_terse_struct_with_custom_default(self) -> None:
        empty = EmptyStruct()
        for proto in Protocol:
            encoded_empty = serialize(empty, protocol=proto)
            decoded, length = deserialize_with_length(
                TerseStructWithCustomDefault, encoded_empty, protocol=proto
            )
            self.assertIsInstance(decoded, TerseStructWithCustomDefault)
            self.assertEqual(decoded.bool_field, False)
            self.assertEqual(decoded.byte_field, 0)
            self.assertEqual(decoded.short_field, 0)
            self.assertEqual(decoded.int_field, 0)
            self.assertEqual(decoded.long_field, 0)
            self.assertEqual(decoded.float_field, 0.0)
            self.assertEqual(decoded.double_field, 0.0)
            self.assertEqual(decoded.string_field, "")
            self.assertEqual(decoded.binary_field, b"")
            self.assertEqual(decoded.enum_field, MyEnum.ME0)
            self.assertEqual(decoded.list_field, [])
            self.assertEqual(decoded.set_field, set())
            self.assertEqual(decoded.map_field, {})
            self.assertEqual(decoded.struct_field, MyStructWithCustomDefault(field1=0))
