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

from testing.thrift_types import easy as python_easy, hard as python_hard
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
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated
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
from thrift.python.types import Struct as PythonStruct
from thrift.test.terse_write.types import (
    EmptyStruct,
    FieldLevelTerseStruct,
    MyEnum,
    MyStruct,
    MyUnion,
)


class SerializerTestBase(unittest.TestCase):
    def thrift_serialization_round_robin(
        self, control: Union[Struct, PythonStruct], fixtures: Mapping[Protocol, bytes]
    ) -> None:
        for proto in Protocol:
            encoded = serialize(control, protocol=proto)
            self.assertIsInstance(encoded, bytes)
            decoded = deserialize(type(control), encoded, protocol=proto)
            self.assertIsInstance(decoded, type(control))
            self.assertEqual(control, decoded)
            self.assertEqual((proto, encoded), (proto, fixtures.get(proto)))

    def header_serialize_round_robin(
        self,
        control: Union[Struct, PythonStruct],
    ) -> None:
        for proto in Protocol:
            for transform in Transform:
                if (
                    proto == Protocol.DEPRECATED_VERBOSE_JSON
                    and Transform != Transform.NONE
                ):
                    continue
                buf = serialize_with_header(
                    control, protocol=proto, transform=transform
                )
                decoded = deserialize_from_header(type(control), buf)
                self.assertEqual(control, decoded)
                iobuf = serialize_with_header_iobuf(
                    control, protocol=proto, transform=transform
                )
                self.assertEqual(control, deserialize_from_header(type(control), iobuf))

    def with_length_round_robin(
        self,
        control: Union[Struct, PythonStruct],
    ) -> None:
        for proto in Protocol:
            encoded = serialize(control, protocol=proto)
            decoded, length = deserialize_with_length(
                type(control), encoded, protocol=proto
            )
            self.assertIsInstance(decoded, type(control))
            self.assertEqual(decoded, control)
            self.assertEqual(length, len(encoded))


class SerializerTests(SerializerTestBase):
    def test_with_header(self) -> None:
        control = easy(val=5, val_list=[4, 3, 2, 1])
        self.header_serialize_round_robin(control)

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
        loop = asyncio.new_event_loop()
        # pyre-fixme[6]: For 2nd argument expected `(*(*asyncio.events._Ts)) -> _T`
        #  but got `(tstruct: sT, protocol: Protocol = ...) -> bytes`.
        coro = loop.run_in_executor(None, serialize, control)
        encoded = loop.run_until_complete(coro)
        # pyre-fixme[6]: For 2nd argument expected `(*(*asyncio.events._Ts)) -> _T` b...
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
            deserialize(easy, b"\x02\xde\xad\xbe\xef", protocol=Protocol.BINARY)
        with self.assertRaises(BufferError):
            deserialize_from_header(easy, b"\x02\xde\xad\xbe\xef")
        control = easy(val=5, val_list=[4, 3, 2, 1])
        buf = serialize_with_header(control, transform=Transform.ZSTD_TRANSFORM)
        newBytes = bytearray(buf)
        newBytes[4] += 1
        with self.assertRaises(Error):
            deserialize_from_header(easy, bytes(newBytes))

    def pickle_round_robin(
        self,
        control: Struct | Hashable,
    ) -> None:
        encoded = pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
        decoded = pickle.loads(encoded)
        self.assertIsInstance(decoded, type(control))
        self.assertEqual(control, decoded)

    # tests py3 auto-migrate backwards compatibility
    # bytes produced by pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
    # when run in normal mode (auto-migrate off)
    def assert_unpickle_compat(self, stored: bytes, control: Struct | Hashable) -> None:
        decoded = None
        try:
            decoded = pickle.loads(stored)
        except pickle.UnpicklingError:
            self.fail(
                f"failed to unpickle {stored=}"
                f"encoded control={pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)}"
            )
        self.assertIsInstance(decoded, type(control))
        self.assertEqual(
            control,
            decoded,
        )

    def test_serialize_easy_struct(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4])
        fixtures: Mapping[Protocol, bytes] = {
            Protocol.COMPACT: b"\x15\n\x19E\x02\x04\x06\x08,\x00\x16\x00\x00",
            Protocol.BINARY: b"\x08\x00\x01\x00\x00\x00\x05\x0f\x00\x02\x08\x00\x00\x00"
            b"\x04\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00"
            b"\x00\x00\x04\x0c\x00\x04\x00\n\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            Protocol.JSON: b'{"val":5,"val_list":[1,2,3,4],"an_int":{},"py3_hidden":0}',
            Protocol.DEPRECATED_VERBOSE_JSON: b'{"1":{"i32":5},"2":{"lst":["i32",4,1,2,3,4]},"4"'
            b':{"rec":{}},"5":{"i64":0}}',
        }
        self.thrift_serialization_round_robin(control, fixtures)

    def test_pickle_easy_struct(self) -> None:
        control = easy(val=0, val_list=[5, 6, 7])
        self.pickle_round_robin(control)

    def test_unpickle_stored_easy_struct(self) -> None:
        control = easy(val=0, val_list=[5, 6, 7])
        # string produced with pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
        # when run in normal mode (auto-migrate off)
        stored = (
            b"\x80\x05\x95U\x00\x00\x00\x00\x00\x00\x00\x8c\x15thrift.py3."
            b"serializer\x94\x8c\x0bdeserialize\x94\x93\x94\x8c\rtesting.types"
            b"\x94\x8c\x04easy\x94\x93\x94C\x0c\x15\x00\x195\n\x0c\x0e,\x00\x16"
            b"\x00\x00\x94\x86\x94R\x94."
        )
        self.assert_unpickle_compat(stored, control)

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
            Protocol.DEPRECATED_VERBOSE_JSON: b'{"1":{"i32":0},"2":{"lst":["i32",4,1,2,3,4]},"3":'
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
            Protocol.DEPRECATED_VERBOSE_JSON: b'{"3":{"i32":1337}}',
        }

        self.thrift_serialization_round_robin(control, fixtures)

    def test_pickle_Integers_union(self) -> None:
        control = Integers(large=2**32)
        self.pickle_round_robin(control)

    def test_unpickle_stored_Integers_union(self) -> None:
        control = Integers(large=2**32)
        stored = (
            b"\x80\x05\x95T\x00\x00\x00\x00\x00\x00\x00\x8c\x15thrift.py3."
            b"serializer\x94\x8c\x0bdeserialize\x94\x93\x94\x8c\rtesting.types"
            b"\x94\x8c\x08Integers\x94\x93\x94C\x07F\x80\x80\x80\x80 \x00\x94"
            b"\x86\x94R\x94."
        )
        self.assert_unpickle_compat(stored, control)

    def test_pickle_enum(self) -> None:
        control = MyEnum.ME1
        self.pickle_round_robin(control)

    def test_unpickle_stored_enum(self) -> None:
        control = MyEnum.ME1
        stored = (
            b"\x80\x04\x959\x00\x00\x00\x00\x00\x00\x00\x8c$thrift.test.terse_write."
            b"thrift_enums\x94\x8c\x06MyEnum\x94\x93\x94K\x01\x85\x94R\x94."
        )
        self.assert_unpickle_compat(stored, control)

    def test_pickle_sequence(self) -> None:
        control = I32List([1, 2, 3, 4])
        self.pickle_round_robin(control)

        digits = Digits(data=[Integers(tiny=1), Integers(tiny=2), Integers(large=0)])
        data = digits.data
        assert data
        self.pickle_round_robin(data)

    def test_unpickle_stored_sequence(self) -> None:
        control = I32List([1, 2, 3, 4])
        stored = (
            b"\x80\x05\x95/\x00\x00\x00\x00\x00\x00\x00\x8c\rtesting.types"
            b"\x94\x8c\tList__i32\x94\x93\x94]\x94(K\x01K\x02K\x03K\x04e"
            b"\x85\x94R\x94."
        )
        self.assert_unpickle_compat(stored, control)

    def test_pickle_set(self) -> None:
        control = SetI32({1, 2, 3, 4})
        self.pickle_round_robin(control)

    def test_unpickle_stored_set(self) -> None:
        control = SetI32({1, 2, 3, 4})
        stored = (
            b"\x80\x05\x95.\x00\x00\x00\x00\x00\x00\x00\x8c\rtesting.types"
            b"\x94\x8c\x08Set__i32\x94\x93\x94\x8f\x94(K\x01K\x02K\x03K\x04"
            b"\x90\x85\x94R\x94."
        )
        self.assert_unpickle_compat(stored, control)

    def test_pickle_mapping(self) -> None:
        control = StrStrMap({"test": "test", "foo": "bar"})
        self.pickle_round_robin(control)

    def test_unpickle_stored_mapping(self) -> None:
        control = StrStrMap({"test": "test", "foo": "bar"})
        stored = (
            b"\x80\x05\x95E\x00\x00\x00\x00\x00\x00\x00\x8c\rtesting.types"
            b"\x94\x8c\x12Map__string_string\x94\x93\x94}\x94(\x8c\x04test"
            b"\x94h\x04\x8c\x03foo\x94\x8c\x03bar\x94u\x85\x94R\x94."
        )
        self.assert_unpickle_compat(stored, control)

    def test_deserialize_with_length(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4, 5])
        self.with_length_round_robin(control)

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
            union_field=MyUnion(),
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
            # pyre-ignore[6]: isset typing is awful
            isset_map: Mapping[str, bool] = Struct.isset_DEPRECATED(decoded).__dict__
            self.assertEqual(len(isset_map), 15 if is_auto_migrated() else 0, isset_map)
            # terse fields not included in py3 isset
            if is_auto_migrated():
                non_union_isset = [
                    isset
                    for fld_name, isset in isset_map.items()
                    if fld_name != "union_field"
                ]
                self.assertTrue(all(non_union_isset), isset_map)
                self.assertFalse(isset_map["union_field"])

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
            union_field=MyUnion(),
        )
        for proto in Protocol:
            encoded = serialize(obj, protocol=proto)
            encoded_empty = serialize(empty, protocol=proto)
            self.assertEqual(encoded, encoded_empty)
            decoded = deserialize(type(obj), encoded, protocol=proto)

            # pyre-ignore[6]: isset typing is awful
            isset_map: Mapping[str, bool] = Struct.isset_DEPRECATED(decoded).__dict__
            # terse fields not included in py3 isset
            self.assertEqual(len(isset_map), 15 if is_auto_migrated() else 0, isset_map)
            self.assertFalse(any(isset_map.values()), isset_map)
            if not is_auto_migrated():
                self.assertNotIn("union_field", isset_map)


class SerializerForwardCompat(SerializerTestBase):
    def test_with_header(self) -> None:
        control = python_easy(val=5, val_list=[4, 3, 2, 1])
        self.header_serialize_round_robin(control)

    def test_serialize_easy_struct_python_forward_compat(self) -> None:
        control = python_easy(val=5, val_list=[1, 2, 3, 4])
        fixtures: Mapping[Protocol, bytes] = {
            Protocol.COMPACT: b"\x15\n\x19E\x02\x04\x06\x08,\x00\x16\x00\x00",
            Protocol.BINARY: b"\x08\x00\x01\x00\x00\x00\x05\x0f\x00\x02\x08\x00\x00\x00"
            b"\x04\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00"
            b"\x00\x00\x04\x0c\x00\x04\x00\n\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            Protocol.JSON: b'{"val":5,"val_list":[1,2,3,4],"an_int":{},"py3_hidden":0}',
            Protocol.DEPRECATED_VERBOSE_JSON: b'{"1":{"i32":5},"2":{"lst":["i32",4,1,2,3,4]},"4"'
            b':{"rec":{}},"5":{"i64":0}}',
        }
        self.thrift_serialization_round_robin(control, fixtures)

    def test_serialize_hard_struct_python_forward_compat(self) -> None:
        # note the implicit conversion from py3 Integers to python integers on creation
        control = python_hard(
            val=0,
            val_list=[1, 2, 3, 4],
            name="foo",
            # pyre-fixme[6]: In call `python_hard.__init__`, for argument `an_int`, expected `Optional[thrift_types.Integer]` but got `Integers`.
            an_int=Integers(tiny=1),
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
            Protocol.DEPRECATED_VERBOSE_JSON: b'{"1":{"i32":0},"2":{"lst":["i32",4,1,2,3,4]},"3":'
            b'{"str":"foo"},"4":{"rec":{"1":{"i8":1}}},"5":{"str":"some default"}}',
        }
        self.thrift_serialization_round_robin(control, fixtures)

    def test_deserialize_with_length(self) -> None:
        control = python_easy(val=5, val_list=[1, 2, 3, 4, 5])
        self.with_length_round_robin(control)
