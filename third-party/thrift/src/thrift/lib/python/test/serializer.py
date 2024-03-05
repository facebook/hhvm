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

import asyncio
import pickle
import unittest
from collections.abc import Sequence, Set
from typing import Any, Mapping, Union

from apache.thrift.test.terse_write.terse_write.thrift_types import (
    EmptyStruct,
    FieldLevelTerseStruct,
    MyEnum,
    MyStruct,
    MyStructWithCustomDefault,
    MyUnion,
    TerseStructs,
    TerseStructs1,
    TerseStructs2,
    TerseStructs3,
    TerseStructWithCustomDefault,
)
from folly.iobuf import IOBuf
from testing.thrift_types import (
    Color,
    ColorGroups,
    Complex,
    ComplexUnion,
    Digits,
    easy,
    hard,
    I32List,
    Integers,
    IOBufListStruct,
    SetI32,
    StringBucket,
    StrStrMap,
)
from thrift.python.exceptions import Error
from thrift.python.serializer import (
    deserialize,
    deserialize_with_length,
    Protocol,
    serialize,
    serialize_iobuf,
)
from thrift.python.test.containers.thrift_types import Foo, Lists, Maps, Sets
from thrift.python.types import StructOrUnion


class SerializerTests(unittest.TestCase):
    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: intentionally introduced for testing
            serialize(None)

    def test_sanity(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: intentionally introduced for testing
            serialize(1, Protocol.COMPACT)

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: intentionally introduced for testing
            serialize(easy(), None)

        with self.assertRaises(TypeError):
            # pyre-ignore[6]: intentionally introduced for testing
            deserialize(Protocol, b"")

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
            deserialize(easy, b"")
        with self.assertRaises(Error):
            deserialize(easy, b"\x05AAAAAAAA")
        with self.assertRaises(Error):
            deserialize(easy, b"\x02\xDE\xAD\xBE\xEF", protocol=Protocol.BINARY)

    def thrift_serialization_round_trip(self, control: StructOrUnion) -> None:
        for proto in Protocol:
            encoded = serialize(control, protocol=proto)
            self.assertIsInstance(encoded, bytes)
            decoded = deserialize(type(control), encoded, protocol=proto)
            self.assertIsInstance(decoded, type(control))
            self.assertEqual(control, decoded)

            decoded_with_full_cache = deserialize(
                type(control), encoded, protocol=proto, fully_populate_cache=True
            )
            self.assertIsInstance(decoded_with_full_cache, type(control))
            self.assertEqual(control, decoded_with_full_cache)

    def pickle_round_trip(
        self,
        # pyre-ignore[2]
        control: Union[StructOrUnion, Sequence, Set, Mapping[Any, Any]],
    ) -> None:
        encoded = pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
        decoded = pickle.loads(encoded)
        self.assertIsInstance(decoded, type(control))
        self.assertEqual(control, decoded)

    def test_serialize_easy_struct(self) -> None:
        control = easy(val=5, val_list=[1, 2, 3, 4])
        self.thrift_serialization_round_trip(control)

    def test_pickle_easy_struct(self) -> None:
        control = easy(val=0, val_list=[5, 6, 7])
        self.pickle_round_trip(control)

    def test_serialize_hard_struct(self) -> None:
        control = hard(
            val=0, val_list=[1, 2, 3, 4], name="foo", an_int=Integers(tiny=1)
        )
        self.thrift_serialization_round_trip(control)

    def test_pickle_hard_struct(self) -> None:
        control = hard(
            val=0, val_list=[1, 2, 3, 4], name="foo", an_int=Integers(tiny=1)
        )
        self.pickle_round_trip(control)

    def test_serialize_Integers_union(self) -> None:
        control = Integers(medium=1337)

        self.thrift_serialization_round_trip(control)

    def test_pickle_Integers_union(self) -> None:
        control = Integers(large=2**32)
        self.pickle_round_trip(control)

    @unittest.skip("Pickling containers doesn't work yet")
    def test_pickle_sequence(self) -> None:
        control = I32List([1, 2, 3, 4])
        self.pickle_round_trip(control)

        digits = Digits(data=[Integers(tiny=1), Integers(tiny=2), Integers(large=0)])
        data = digits.data
        assert data
        self.pickle_round_trip(data)

    @unittest.skip("Pickling containers doesn't work yet")
    def test_pickle_set(self) -> None:
        control = SetI32({1, 2, 3, 4})
        self.pickle_round_trip(control)

    @unittest.skip("Pickling containers doesn't work yet")
    def test_pickle_mapping(self) -> None:
        control = StrStrMap({"test": "test", "foo": "bar"})
        self.pickle_round_trip(control)

    def test_serialize_Complex(self) -> None:
        control = Complex(
            val_bool=True,
            val_i32=42,
            val_i64=1 << 33,
            val_string="hello\u4e16\u754c",
            val_binary=b"\xe5\x92\x8c\xe5\b9\xb3",
            val_iobuf=IOBuf(b"\xe5\x9b\x9b\xe5\x8d\x81\xe4\xba\x8c"),
            val_enum=Color.green,
            val_union=ComplexUnion(double_val=1.234),
            val_set={easy(val=42)},
            val_map={"foo": b"foovalue"},
            val_complex_map={"bar": [{easy(val=42), easy(val_list=[1, 2, 3])}]},
            val_struct_with_containers=ColorGroups(
                color_list=[Color.blue, Color.green],
                color_set={Color.blue, Color.red},
                color_map={Color.blue: Color.green},
            ),
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_iobuf_list_struct(self) -> None:
        control = IOBufListStruct(iobufs=[IOBuf(b"foo"), IOBuf(b"bar")])
        self.thrift_serialization_round_trip(control)

    def test_serialize_lists_struct(self) -> None:
        control = Lists(
            boolList=[True, False],
            byteList=[1, 2, 3],
            i16List=[4, 5, 6],
            i64List=[7, 8, 9],
            doubleList=[1.23, 4.56],
            floatList=[7.0, 8.0],
            stringList=["foo", "bar"],
            binaryList=[b"foo", b"bar"],
            iobufList=[IOBuf(b"foo"), IOBuf(b"bar")],
            structList=[Foo(value=1), Foo(value=2)],
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_set_struct(self) -> None:
        control = Sets(
            boolSet={True, False},
            byteSet={1, 2, 3},
            i16Set={4, 5, 6},
            i64Set={7, 8, 9},
            doubleSet={1.23, 4.56},
            floatSet={7, 8},
            stringSet={"foo", "bar"},
            binarySet={b"foo", b"bar"},
            iobufSet={IOBuf(b"foo"), IOBuf(b"bar")},
            structSet={Foo(value=1), Foo(value=2)},
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_map_struct(self) -> None:
        control = Maps(
            boolMap={True: 1, False: 0},
            byteMap={1: 1, 2: 2, 3: 3},
            i16Map={4: 4, 5: 5, 6: 6},
            i64Map={7: 7, 8: 8, 9: 9},
            doubleMap={1.23: 1.23, 4.56: 4.56},
            floatMap={7.0: 7.0, 8.0: 8.0},
            stringMap={"foo": "foo", "bar": "bar"},
            binaryMap={b"foo": b"foo", b"bar": b"bar"},
            iobufMap={IOBuf(b"foo"): IOBuf(b"foo"), IOBuf(b"bar"): IOBuf(b"bar")},
            structMap={Foo(value=1): Foo(value=1), Foo(value=2): Foo(value=2)},
        )
        self.thrift_serialization_round_trip(control)

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
            union_field=MyUnion(struct_field=MyStruct(field1=1)),
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
        obj = FieldLevelTerseStruct()
        for proto in Protocol:
            encoded = serialize(obj, protocol=proto)
            encoded_empty = serialize(empty, protocol=proto)
            self.assertEqual(encoded, encoded_empty)

    # Since empty serializd binary is deserialized, all terse fields should equal
    # to their intrinsic default values.
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

    def test_terse_structs_optimization(self) -> None:
        # empty
        empty = EmptyStruct()
        obj = TerseStructs(
            field1=MyStruct(field1=0),
            field2=MyStruct(field1=0),
            field3=MyStruct(field1=0),
        )
        for proto in Protocol:
            encoded_empty = serialize(empty, protocol=proto)
            encoded_obj = serialize(obj, protocol=proto)
            self.assertEqual(encoded_empty, encoded_obj)

        # field1 set
        obj = TerseStructs(
            field1=MyStruct(field1=1),
            field2=MyStruct(field1=0),
            field3=MyStruct(field1=0),
        )
        obj1 = TerseStructs1(field1=MyStruct(field1=1))
        for proto in Protocol:
            encoded_obj1 = serialize(obj1, protocol=proto)
            encoded_obj = serialize(obj, protocol=proto)
            self.assertEqual(encoded_obj1, encoded_obj)

        # field2 set
        obj = TerseStructs(
            field1=MyStruct(field1=0),
            field2=MyStruct(field1=1),
            field3=MyStruct(field1=0),
        )
        obj2 = TerseStructs2(field2=MyStruct(field1=1))
        for proto in Protocol:
            encoded_obj2 = serialize(obj2, protocol=proto)
            encoded_obj = serialize(obj, protocol=proto)
            self.assertEqual(encoded_obj2, encoded_obj)

        # field3 set
        obj = TerseStructs(
            field1=MyStruct(field1=0),
            field2=MyStruct(field1=0),
            field3=MyStruct(field1=1),
        )
        obj3 = TerseStructs3(field3=MyStruct(field1=1))
        for proto in Protocol:
            encoded_obj3 = serialize(obj3, protocol=proto)
            encoded_obj = serialize(obj, protocol=proto)
            self.assertEqual(encoded_obj3, encoded_obj)

    # Test binary field is b64encoded in SimpleJSON protocol.
    def test_binary_serialization_simplejson(self) -> None:
        json_bytes = b'{"val_bool":false,"val_i32":0,"val_i64":0,"val_string":"abcdef","val_binary":"YWJjZGU","val_iobuf":"YWJjZGVm","val_enum":0,"val_union":{},"val_list":[],"val_map":{},"val_struct_with_containers":{"color_list":[],"color_set":[],"color_map":{}}}'
        s = Complex(
            val_string="abcdef",
            val_binary=b"abcde",
            val_iobuf=IOBuf(b"abcdef"),
        )
        self.assertEqual(serialize(s, protocol=Protocol.JSON), json_bytes)
        self.assertEqual(deserialize(Complex, json_bytes, protocol=Protocol.JSON), s)
