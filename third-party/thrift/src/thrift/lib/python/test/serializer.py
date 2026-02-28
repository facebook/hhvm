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
import json
import pickle
import types
import unittest
from collections.abc import Sequence, Set
from typing import Any, Iterable, Mapping, Type, TypeVar, Union

import python_test.containers.thrift_mutable_types as mutable_containers_types
import python_test.containers.thrift_types as immutable_containers_types
import testing.thrift_mutable_types as mutable_types
import testing.thrift_types as immutable_types
import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
import thrift.test.terse_write.thrift_mutable_types as mutable_terse_types
import thrift.test.terse_write.thrift_types as immutable_terse_types
from folly.iobuf import IOBuf
from parameterized import parameterized_class
from python_test.containers.thrift_types import (
    Foo,
    Lists,
    Maps,
    Sets,
    UnicodeContainers,
)
from testing.thrift_mutable_types import (
    SortedMaps as MutableSortedMaps,
    SortedSets as MutableSortedSets,
)
from testing.thrift_types import (
    Color,
    ColorGroups,
    Complex,
    ComplexUnion,
    Digits,
    easy,
    EasyList,
    EasySet,
    hard,
    I32List,
    Integers,
    IOBufListStruct,
    Reserved,
    SetI32,
    SetI32Lists,
    SortedMaps,
    SortedSets,
    StrEasyMap,
    StrI32ListMap,
    StringBucket,
    StringList,
    StrList2D,
    StrStrMap,
)
from thrift.python.exceptions import Error
from thrift.python.mutable_types import (
    _ThriftListWrapper,
    _ThriftMapWrapper,
    _ThriftSetWrapper,
    MutableStructOrUnion,
    to_thrift_list,
    to_thrift_map,
    to_thrift_set,
)
from thrift.python.serializer import Protocol
from thrift.python.types import StructOrUnion
from thrift.test.terse_write.thrift_types import (
    EmptyStruct,
    FieldLevelTerseStruct,
    MyEnum,
    MyStruct,
    MyUnion,
    TerseSafePatch,
    TerseStructs,
    TerseStructs1,
    TerseStructs2,
    TerseStructs3,
)

ListT = TypeVar("ListT")
SetT = TypeVar("SetT")
MapKey = TypeVar("MapKey")
MapValue = TypeVar("MapValue")


def thrift_serialization_round_trip(
    self: unittest.TestCase,
    control: Union[StructOrUnion, MutableStructOrUnion],
    serializer: types.ModuleType,
) -> None:
    for proto in Protocol:
        encoded = serializer.serialize(control, protocol=proto)
        self.assertIsInstance(encoded, bytes)
        decoded = serializer.deserialize(type(control), encoded, protocol=proto)
        self.assertIsInstance(decoded, type(control))
        self.assertEqual(control, decoded)

        decoded_with_full_cache = serializer.deserialize(
            type(control), encoded, protocol=proto
        )
        self.assertIsInstance(decoded_with_full_cache, type(control))
        self.assertEqual(control, decoded_with_full_cache)


@parameterized_class(
    ("test_types", "container_types", "serializer_module"),
    [
        (immutable_types, immutable_containers_types, immutable_serializer),
        (mutable_types, mutable_containers_types, mutable_serializer),
    ],
)
class SerializerTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.easy: Type[easy] = self.test_types.easy
        self.hard: Type[hard] = self.test_types.hard
        self.Integers: Type[Integers] = self.test_types.Integers
        self.I32List: Type[I32List] = self.test_types.I32List
        self.StrList2D: Type[StrList2D] = self.test_types.StrList2D
        self.StringList: Type[StringList] = self.test_types.StringList
        self.Digits: Type[Digits] = self.test_types.Digits
        self.ColorGroups: Type[ColorGroups] = self.test_types.ColorGroups
        self.Color: Type[Color] = self.test_types.Color
        self.ComplexUnion: Type[ComplexUnion] = self.test_types.ComplexUnion
        self.Complex: Type[Complex] = self.test_types.Complex
        self.IOBufListStruct: Type[IOBufListStruct] = self.test_types.IOBufListStruct
        self.EasyList: Type[EasyList] = self.test_types.EasyList
        self.EasySet: Type[EasySet] = self.test_types.EasySet
        self.StrEasyMap: Type[StrEasyMap] = self.test_types.StrEasyMap
        self.SetI32: Type[SetI32] = self.test_types.SetI32
        self.SetI32Lists: Type[SetI32Lists] = self.test_types.SetI32Lists
        self.StrStrMap: Type[StrStrMap] = self.test_types.StrStrMap
        self.StrI32ListMap: Type[StrI32ListMap] = self.test_types.StrI32ListMap
        self.StringBucket: Type[StringBucket] = self.test_types.StringBucket
        self.Reserved: Type[Reserved] = self.test_types.Reserved
        # pyre-ignore[16]: has no attribute `container_types`
        self.Sets: Type[Sets] = self.container_types.Sets
        self.Lists: Type[Lists] = self.container_types.Lists
        self.Maps: Type[Maps] = self.container_types.Maps
        self.UnicodeContainers: Type[UnicodeContainers] = (
            self.container_types.UnicodeContainers
        )
        self.Foo: Type[Foo] = self.container_types.Foo
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module
        self.SortedSets: Type[SortedSets] = self.test_types.SortedSets
        self.SortedMaps: Type[SortedMaps] = self.test_types.SortedMaps

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def to_set(self, set_data: set[SetT]) -> set[SetT] | _ThriftSetWrapper:
        return to_thrift_set(set_data) if self.is_mutable_run else set_data

    def to_map(
        self, map_data: dict[MapKey, MapValue]
    ) -> dict[MapKey, MapValue] | _ThriftMapWrapper:
        return to_thrift_map(map_data) if self.is_mutable_run else map_data

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            self.serializer.serialize(None)

    def test_sanity(self) -> None:
        with self.assertRaises(TypeError):
            self.serializer.serialize(1, Protocol.COMPACT)

        with self.assertRaises(TypeError):
            self.serializer.serialize(self.easy(), None)

        with self.assertRaises(TypeError):
            self.serializer.deserialize(Protocol, b"")

    def test_from_thread_pool(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        control = self.easy(val=5, val_list=self.to_list([1, 2, 3, 4]))
        loop = asyncio.new_event_loop()
        try:
            coro = loop.run_in_executor(None, self.serializer.serialize, control)
            encoded = loop.run_until_complete(coro)
            coro = loop.run_in_executor(
                None, self.serializer.deserialize, type(control), encoded
            )
            decoded = loop.run_until_complete(coro)
            self.assertEqual(control, decoded)
        finally:
            loop.close()

    def test_serialize_iobuf(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        control = self.easy(val=5, val_list=self.to_list([1, 2, 3, 4, 5]))
        iobuf = self.serializer.serialize_iobuf(control)
        decoded = self.serializer.deserialize(type(control), iobuf)
        self.assertEqual(control, decoded)

    def test_bad_deserialize(self) -> None:
        with self.assertRaises(Error):
            self.serializer.deserialize(self.easy, b"")
        with self.assertRaises(Error):
            self.serializer.deserialize(self.easy, b"\x05AAAAAAAA")
        with self.assertRaises(Error):
            self.serializer.deserialize(
                self.easy, b"\x02\xde\xad\xbe\xef", protocol=Protocol.BINARY
            )

    def thrift_serialization_round_trip(
        self, control: Union[StructOrUnion, MutableStructOrUnion]
    ) -> None:
        thrift_serialization_round_trip(self, control, self.serializer)

    def pickle_round_trip(
        self,
        control: Union[
            StructOrUnion, MutableStructOrUnion, Sequence, Set, Mapping[Any, Any]
        ],
    ) -> None:
        encoded = pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
        decoded = pickle.loads(encoded)
        self.assertIsInstance(decoded, type(control))
        self.assertEqual(control, decoded)

    def test_serialize_easy_struct(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        control = self.easy(val=5, val_list=self.to_list([1, 2, 3, 4]))
        self.thrift_serialization_round_trip(control)

    def test_pickle_easy_struct(self) -> None:
        if self.is_mutable_run:
            # TODO: pickle for mutable types
            return

        val = self.easy(val=0, val_list=[5, 6, 7])
        self.pickle_round_trip(control=val)
        self.pickle_round_trip(control=self.EasyList([val]))
        self.pickle_round_trip(control=self.EasySet({val}))
        self.pickle_round_trip(control=self.StrEasyMap({"foo": val}))

    def test_serialize_hard_struct(self) -> None:
        control = self.hard(
            val=0,
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_list=self.to_list([1, 2, 3, 4]),
            name="foo",
            an_int=self.Integers(tiny=1),
        )
        self.thrift_serialization_round_trip(control)

    def test_pickle_hard_struct(self) -> None:
        if self.is_mutable_run:
            # TODO: pickle for mutable types
            return

        control = self.hard(
            val=0, val_list=[1, 2, 3, 4], name="foo", an_int=self.Integers(tiny=1)
        )
        self.pickle_round_trip(control)

    def test_serialize_Integers_union(self) -> None:
        control = self.Integers(medium=1337)

        self.thrift_serialization_round_trip(control)

    def test_pickle_Integers_union(self) -> None:
        if self.is_mutable_run:
            # TODO: pickle for mutable types
            return

        control = self.Integers(large=2**32)
        self.pickle_round_trip(control)

    def test_pickle_sequence(self) -> None:
        if self.is_mutable_run:
            # TODO: pickle for mutable types
            return

        self.pickle_round_trip(control=self.I32List([1, 2, 3, 4]))
        self.pickle_round_trip(
            control=self.StrList2D([self.StringList(["foo", "bar"])])
        )

        digits = self.Digits(
            data=[self.Integers(tiny=1), self.Integers(tiny=2), self.Integers(large=0)]
        )
        data = digits.data
        assert data
        self.pickle_round_trip(data)

    def test_pickle_set(self) -> None:
        if self.is_mutable_run:
            # TODO: pickle for mutable types
            return

        self.pickle_round_trip(control=self.SetI32({1, 2, 3, 4}))
        self.pickle_round_trip(
            control=self.SetI32Lists({self.I32List([1, 2]), self.I32List([3, 4])})
        )

    def test_pickle_mapping(self) -> None:
        if self.is_mutable_run:
            # TODO: pickle for mutable types
            return

        self.pickle_round_trip(control=self.StrStrMap({"test": "test", "foo": "bar"}))
        self.pickle_round_trip(control=self.StrI32ListMap({"a": self.I32List([1, 2])}))

    def test_serialize_Complex(self) -> None:
        control = self.Complex(
            val_bool=True,
            val_i32=42,
            val_i64=1 << 33,
            val_string="hello\u4e16\u754c",
            val_binary=b"\xe5\x92\x8c\xe5\b9\xb3",
            val_iobuf=IOBuf(b"\xe5\x9b\x9b\xe5\x8d\x81\xe4\xba\x8c"),
            val_enum=self.Color.green,
            val_union=self.ComplexUnion(double_val=1.234),
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_set=(
                {easy(val=42)}
                if not self.is_mutable_run
                else to_thrift_set(set())  # Mutable types are not hashable.
            ),
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_map=self.to_map({"foo": b"foovalue"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            val_complex_map=(
                {"bar": [{self.easy(val=42), self.easy(val_list=[1, 2, 3])}]}
                if not self.is_mutable_run
                else to_thrift_map({})  # Mutable types are not hashable.
            ),
            val_struct_with_containers=self.ColorGroups(
                # pyre-ignore[6]: TODO: Thrift-Container init
                color_list=self.to_list([self.Color.blue, self.Color.green]),
                # pyre-ignore[6]: TODO: Thrift-Container init
                color_set=self.to_set({self.Color.blue, self.Color.red}),
                # pyre-ignore[6]: TODO: Thrift-Container init
                color_map=self.to_map({self.Color.blue: self.Color.green}),
            ),
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_iobuf_list_struct(self) -> None:
        control = self.IOBufListStruct(
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufs=self.to_list([IOBuf(b"foo"), IOBuf(b"bar")])
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_lists_struct(self) -> None:
        control = self.Lists(
            # pyre-ignore[6]: TODO: Thrift-Container init
            boolList=self.to_list([True, False]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            byteList=self.to_list([1, 2, 3]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i16List=self.to_list([4, 5, 6]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i64List=self.to_list([7, 8, 9]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            doubleList=self.to_list([1.23, 4.56]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            floatList=self.to_list([7.0, 8.0]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            stringList=self.to_list(["foo", "bar"]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            binaryList=self.to_list([b"foo", b"bar"]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufList=self.to_list([IOBuf(b"foo"), IOBuf(b"bar")]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            structList=self.to_list([self.Foo(value=1), self.Foo(value=2)]),
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_set_struct(self) -> None:
        control = self.Sets(
            # pyre-ignore[6]: TODO: Thrift-Container init
            boolSet=self.to_set({True, False}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            byteSet=self.to_set({1, 2, 3}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i16Set=self.to_set({4, 5, 6}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i64Set=self.to_set({7, 8, 9}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            doubleSet=self.to_set({1.23, 4.56}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            floatSet=self.to_set({7, 8}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            stringSet=self.to_set({"foo", "bar"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            binarySet=self.to_set({b"foo", b"bar"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufSet=self.to_set({IOBuf(b"foo"), IOBuf(b"bar")}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            structSet=(
                {self.Foo(value=1), self.Foo(value=2)}
                if not self.is_mutable_run
                else to_thrift_set(set())  # Mutable types are not hashable.
            ),
        )
        self.thrift_serialization_round_trip(control)

    def test_serialize_map_struct(self) -> None:
        control = self.Maps(
            # pyre-ignore[6]: TODO: Thrift-Container init
            boolMap=self.to_map({True: 1, False: 0}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            byteMap=self.to_map({1: 1, 2: 2, 3: 3}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i16Map=self.to_map({4: 4, 5: 5, 6: 6}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i64Map=self.to_map({7: 7, 8: 8, 9: 9}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            doubleMap=self.to_map({1.23: 1.23, 4.56: 4.56}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            floatMap=self.to_map({7.0: 7.0, 8.0: 8.0}),
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
                {
                    self.Foo(value=1): self.Foo(value=1),
                    self.Foo(value=2): self.Foo(value=2),
                }
                if not self.is_mutable_run
                else to_thrift_map({})  # Mutable types are not hashable.
            ),
        )
        self.thrift_serialization_round_trip(control)

    def test_deserialize_with_length(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        control = self.easy(val=5, val_list=self.to_list([1, 2, 3, 4, 5]))
        for proto in Protocol:
            encoded = self.serializer.serialize(control, protocol=proto)
            decoded, length = self.serializer.deserialize_with_length(
                type(control), encoded, protocol=proto
            )
            self.assertIsInstance(decoded, type(control))
            self.assertEqual(decoded, control)
            self.assertEqual(length, len(encoded))

    def test_string_with_non_utf8_data(self) -> None:
        encoded = b"\x0b\x00\x01\x00\x00\x00\x03foo\x00"
        sb = self.serializer.deserialize(
            self.StringBucket, encoded, protocol=Protocol.BINARY
        )
        self.assertEqual("foo", sb.one)

        encoded = b"\x0b\x00\x01\x00\x00\x00\x03\xfa\xf0\xef\x00"
        sb = self.serializer.deserialize(
            self.StringBucket, encoded, protocol=Protocol.BINARY
        )
        with self.assertRaises(UnicodeDecodeError):
            # Accessing the property is when the string is decoded as UTF-8.
            sb.one

        # curiously, thrift-python can retain non-unicode string without
        # data loss on serialization roundtrip
        reserialized = self.serializer.serialize(sb, protocol=Protocol.BINARY)
        self.assertEqual(encoded, reserialized)

    def test_non_utf8_container_list(self) -> None:
        json_bytes = b'{"stringList":["good","\xc3\x28"],"stringSet":[],"stringMap":{}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        if self.is_mutable_run:
            # mutable thrift-python lazily converts elements when accessed
            self.assertEqual(s.stringList[0], "good")
            with self.assertRaises(UnicodeDecodeError):
                s.stringList[1]
        else:
            # immutable thrift-python eagerly converts all elements on field access
            with self.assertRaises(UnicodeDecodeError):
                s.stringList

        round_trip_bytes = self.serializer.serialize(s, Protocol.JSON)
        self.assertEqual(json_bytes, round_trip_bytes)

    def test_non_utf8_container_set(self) -> None:
        json_bytes = b'{"stringList":[],"stringSet":["good","\xc3\x28"],"stringMap":{}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        # Both mutable and immutable thrift-python now pass internal
        # set/frozenset directly for sets with string elements
        # Accessing the set field succeeds (no eager conversion)
        set_field = s.stringSet
        # Membership with valid string works
        self.assertIn("good", set_field)
        # Iteration triggers validation and raises on bad Unicode
        with self.assertRaises(UnicodeDecodeError):
            list(set_field)
        # validate it raises consistently
        with self.assertRaises(UnicodeDecodeError):
            list(set_field)
        # can't compare round-trip serialized set because ordering is random

    def test_non_utf8_container_map_unicode_error_bad_key(self) -> None:
        json_bytes = b'{"stringList":[],"stringSet":[],"stringMap":{"key":"val","\xc3\x28":"good"}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        # Both mutable and immutable thrift-python now pass internal
        # dict directly for maps with string keys
        # Accessing the map field succeeds (no eager conversion)
        map_field = s.stringMap
        # Accessing valid keys works
        self.assertEqual(map_field["key"], "val")
        # Iteration triggers conversion and raises on bad key
        with self.assertRaises(UnicodeDecodeError):
            list(map_field.items())
        # validate it raises consistently
        with self.assertRaises(UnicodeDecodeError):
            list(map_field.items())

        round_trip_bytes = self.serializer.serialize(s, Protocol.JSON)
        self.assertEqual(json_bytes, round_trip_bytes)

    def test_non_utf8_container_map_unicode_error_bad_val(self) -> None:
        json_bytes = b'{"stringList":[],"stringSet":[],"stringMap":{"key":"val","good":"\xc3\x28"}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        # Both mutable and immutable thrift-python now pass internal
        # dict directly for maps with string keys
        # Accessing the map field succeeds (no eager conversion)
        map_field = s.stringMap
        # Accessing valid keys works
        self.assertEqual(map_field["key"], "val")
        # Accessing the bad value triggers conversion and raises error
        with self.assertRaises(UnicodeDecodeError):
            map_field["good"]
        # validate it raises consistently
        with self.assertRaises(UnicodeDecodeError):
            map_field["good"]

        round_trip_bytes = self.serializer.serialize(s, Protocol.JSON)
        self.assertEqual(json_bytes, round_trip_bytes)

    def test_lazy_map_keyerror_non_existent_key(self) -> None:
        """Test that accessing non-existent keys raises KeyError in lazy mode."""
        json_bytes = b'{"stringList":[],"stringSet":[],"stringMap":{"key":"val","good":"\xc3\x28"}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        map_field = s.stringMap
        # Accessing non-existent key should raise KeyError even in lazy mode
        with self.assertRaises(KeyError):
            map_field["nonexistent"]
        # Map should still be in lazy mode - accessing bad value should
        # raise UnicodeDecodeError
        with self.assertRaises(UnicodeDecodeError):
            map_field["good"]

    def test_lazy_map_membership_no_conversion(self) -> None:
        """Test that membership operator works without triggering full conversion."""
        json_bytes = b'{"stringList":[],"stringSet":[],"stringMap":{"key":"val","good":"\xc3\x28"}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        map_field = s.stringMap
        # Membership testing should work without triggering conversion
        self.assertTrue("key" in map_field)
        self.assertTrue("good" in map_field)
        self.assertFalse("nonexistent" in map_field)
        # Map should still be in lazy mode - iteration should still raise
        with self.assertRaises(UnicodeDecodeError):
            list(map_field.items())

    def test_lazy_map_len_no_conversion(self) -> None:
        """Test that len() works without triggering full conversion."""
        json_bytes = b'{"stringList":[],"stringSet":[],"stringMap":{"key":"val","good":"\xc3\x28"}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        map_field = s.stringMap
        # len() should work without triggering conversion
        self.assertEqual(len(map_field), 2)
        # Map should still be in lazy mode - iteration should still raise
        with self.assertRaises(UnicodeDecodeError):
            list(map_field.items())

    def test_lazy_map_get_with_default(self) -> None:
        """Test that get() method works correctly in lazy mode."""
        json_bytes = b'{"stringList":[],"stringSet":[],"stringMap":{"key":"val","good":"\xc3\x28"}}'
        s = self.serializer.deserialize(
            self.UnicodeContainers, json_bytes, Protocol.JSON
        )
        map_field = s.stringMap
        # get() with existing key should work
        self.assertEqual(map_field.get("key"), "val")
        # get() with non-existent key should return None
        self.assertIsNone(map_field.get("nonexistent"))
        # get() with default should return default for non-existent key
        self.assertEqual(map_field.get("nonexistent", "default"), "default")
        # get() with bad value should raise UnicodeDecodeError
        with self.assertRaises(UnicodeDecodeError):
            map_field.get("good")

    # Test binary field is b64encoded in SimpleJSON protocol.
    def test_binary_serialization_simplejson(self) -> None:
        json_bytes = b'{"val_bool":false,"val_i32":0,"val_i64":0,"val_string":"abcdef","val_binary":"YWJjZGU","val_iobuf":"YWJjZGVm","val_enum":0,"val_union":{},"val_list":[],"val_map":{},"val_struct_with_containers":{"color_list":[],"color_set":[],"color_map":{}}}'
        s = self.Complex(
            val_string="abcdef",
            val_binary=b"abcde",
            val_iobuf=IOBuf(b"abcdef"),
        )
        self.assertEqual(
            self.serializer.serialize(s, protocol=Protocol.JSON), json_bytes
        )
        self.assertEqual(
            self.serializer.deserialize(
                self.Complex, json_bytes, protocol=Protocol.JSON
            ),
            s,
        )

    def test_json_deserialize_python_name(self) -> None:
        json_bytes = b'{"from":"fromVal","nonlocal":0,"ok":"ok","cpdef":true,"move":"","inst":"","changes":"","__mangled_str":"","__mangled_int":0}'
        r = self.Reserved(from_="fromVal", ok="ok", is_cpdef=True)
        print(self.serializer.serialize(r, protocol=Protocol.JSON))
        self.assertEqual(
            self.serializer.serialize(r, protocol=Protocol.JSON), json_bytes
        )
        self.assertEqual(
            self.serializer.deserialize(
                self.Reserved, json_bytes, protocol=Protocol.JSON
            ),
            r,
        )

    def test_serialize_sorted_set(self) -> None:
        if self.is_mutable_run:
            # mutable structs aren't hashable but empty set works
            easies = set()
        else:
            easies = {self.easy(val=i) for i in range(3, 0, -1)}
        ints = set(range(5, -6, -2))
        strings = {"foo", "bar", "baz"}
        colors = {self.Color.green, self.Color.red, self.Color.blue}
        if self.is_mutable_run:
            s = MutableSortedSets(
                easies=to_thrift_set(easies),
                ints=to_thrift_set(ints),
                strings=to_thrift_set(strings),
                colors=to_thrift_set(colors),
            )
        else:
            s = self.SortedSets(
                easies=easies, ints=ints, strings=strings, colors=colors
            )
        # assert basic serialization works across all protocols
        thrift_serialization_round_trip(self, s, self.serializer)

        # assert the set is sorted in json
        json_s = json.loads(self.serializer.serialize(s, Protocol.JSON).decode("utf-8"))
        self.assertEqual(json_s["ints"], sorted(ints))
        self.assertEqual(json_s["strings"], sorted(strings))
        self.assertEqual(json_s["colors"], sorted(colors))
        json_easy_vals = [e["val"] for e in json_s["easies"]]
        self.assertEqual(json_easy_vals, sorted(e.val for e in easies))

    def test_serialize_sorted_map(self) -> None:
        if self.is_mutable_run:
            # mutable structs aren't hashable but empty set works
            easies = {}
        else:
            easies = {self.easy(val=i): self.easy(val=-i) for i in range(3, 0, -1)}
        ints = {i: -i for i in range(5, -6, -2)}
        strings = {s: s + "_val" for s in ("foo", "bar", "baz")}
        colors = {
            c: self.Color((int(c) - 1) % 3) for c in (self.Color.green, self.Color.red)
        }
        if self.is_mutable_run:
            s = MutableSortedMaps(
                easies=to_thrift_map(easies),
                ints=to_thrift_map(ints),
                strings=to_thrift_map(strings),
                colors=to_thrift_map(colors),
            )
        else:
            s = self.SortedMaps(
                easies=easies, ints=ints, strings=strings, colors=colors
            )
        # assert basic serialization works across all protocols
        thrift_serialization_round_trip(self, s, self.serializer)

        # struct keys aren't supported by normal json parse, so drop them
        if self.is_mutable_run:
            assert isinstance(s, MutableSortedMaps)
            s.easies = to_thrift_map({})
        else:
            assert isinstance(s, self.SortedMaps)
            s = s(easies={})

        serialized_json = self.serializer.serialize(s, Protocol.JSON).decode("utf-8")
        json_s = json.loads(serialized_json)

        # assert the map is sorted in json
        def to_int_list(it: Iterable[str]) -> list[int]:
            return list(map(int, it))

        self.assertListEqual(to_int_list(json_s["ints"].keys()), sorted(ints.keys()))
        self.assertListEqual(list(json_s["strings"].keys()), sorted(strings.keys()))
        self.assertListEqual(
            to_int_list(json_s["colors"].keys()), sorted(map(int, colors.keys()))
        )
        # assert the values match the keys
        self.assertListEqual(
            to_int_list(json_s["ints"].values()),
            [-int(i) for i in json_s["ints"].keys()],
        )
        self.assertListEqual(
            list(json_s["strings"].values()),
            [s + "_val" for s in json_s["strings"].keys()],
        )
        self.assertListEqual(
            to_int_list(json_s["colors"].values()),
            [(int(c) - 1) % 3 for c in json_s["colors"].keys()],
        )


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_terse_types, immutable_serializer),
        (mutable_terse_types, mutable_serializer),
    ],
)
class SerializerTerseWriteTests(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        self.FieldLevelTerseStruct: Type[FieldLevelTerseStruct] = (
            # pyre-ignore[16]: has no attribute `test_types`
            self.test_types.FieldLevelTerseStruct
        )
        self.MyStruct: Type[MyStruct] = self.test_types.MyStruct
        self.MyUnion: Type[MyUnion] = self.test_types.MyUnion
        self.EmptyStruct: Type[EmptyStruct] = self.test_types.EmptyStruct
        self.MyEnum: Type[MyEnum] = self.test_types.MyEnum
        self.TerseStructs: Type[TerseStructs] = self.test_types.TerseStructs
        self.TerseStructs1: Type[TerseStructs1] = self.test_types.TerseStructs1
        self.TerseStructs2: Type[TerseStructs2] = self.test_types.TerseStructs2
        self.TerseStructs3: Type[TerseStructs3] = self.test_types.TerseStructs3
        self.TerseSafePatch: Type[TerseSafePatch] = self.test_types.TerseSafePatch
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def to_set(self, set_data: set[SetT]) -> set[SetT] | _ThriftSetWrapper:
        return to_thrift_set(set_data) if self.is_mutable_run else set_data

    def to_map(
        self, map_data: dict[MapKey, MapValue]
    ) -> dict[MapKey, MapValue] | _ThriftMapWrapper:
        return to_thrift_map(map_data) if self.is_mutable_run else map_data

    def thrift_serialization_round_trip(
        self, control: Union[StructOrUnion, MutableStructOrUnion]
    ) -> None:
        thrift_serialization_round_trip(self, control, self.serializer)

    def test_field_level_terse_write(self) -> None:
        obj = self.FieldLevelTerseStruct(
            bool_field=True,
            byte_field=1,
            short_field=2,
            int_field=3,
            long_field=4,
            float_field=5,
            double_field=6,
            string_field="7",
            binary_field=b"8",
            enum_field=self.MyEnum.ME1,
            # pyre-ignore[6]: TODO: Thrift-Container init
            list_field=self.to_list([1]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            set_field=self.to_set({1}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            map_field=self.to_map({1: 1}),
            struct_field=self.MyStruct(field1=1),
            union_field=self.MyUnion(struct_field=self.MyStruct(field1=1)),
        )
        empty = self.EmptyStruct()
        for proto in Protocol:
            encoded = self.serializer.serialize(obj, protocol=proto)
            decoded, length = self.serializer.deserialize_with_length(
                type(obj), encoded, protocol=proto
            )
            self.assertIsInstance(decoded, type(obj))
            self.assertEqual(decoded, obj)
            self.assertEqual(length, len(encoded))

        # Set fields to their intrinsic default.
        obj = self.FieldLevelTerseStruct()
        for proto in Protocol:
            encoded = self.serializer.serialize(obj, protocol=proto)
            encoded_empty = self.serializer.serialize(empty, protocol=proto)
            self.assertEqual(encoded, encoded_empty)

    def test_terse_structs_optimization(self) -> None:
        # empty
        empty = self.EmptyStruct()
        obj = self.TerseStructs(
            field1=self.MyStruct(field1=0),
            field2=self.MyStruct(field1=0),
            field3=self.MyStruct(field1=0),
        )
        for proto in Protocol:
            encoded_empty = self.serializer.serialize(empty, protocol=proto)
            encoded_obj = self.serializer.serialize(obj, protocol=proto)
            self.assertEqual(encoded_empty, encoded_obj)

        # field1 set
        obj = self.TerseStructs(
            field1=self.MyStruct(field1=1),
            field2=self.MyStruct(field1=0),
            field3=self.MyStruct(field1=0),
        )
        obj1 = self.TerseStructs1(field1=self.MyStruct(field1=1))
        for proto in Protocol:
            encoded_obj1 = self.serializer.serialize(obj1, protocol=proto)
            encoded_obj = self.serializer.serialize(obj, protocol=proto)
            self.assertEqual(encoded_obj1, encoded_obj)

        # field2 set
        obj = self.TerseStructs(
            field1=self.MyStruct(field1=0),
            field2=self.MyStruct(field1=1),
            field3=self.MyStruct(field1=0),
        )
        obj2 = self.TerseStructs2(field2=self.MyStruct(field1=1))
        for proto in Protocol:
            encoded_obj2 = self.serializer.serialize(obj2, protocol=proto)
            encoded_obj = self.serializer.serialize(obj, protocol=proto)
            self.assertEqual(encoded_obj2, encoded_obj)

        # field3 set
        obj = self.TerseStructs(
            field1=self.MyStruct(field1=0),
            field2=self.MyStruct(field1=0),
            field3=self.MyStruct(field1=1),
        )
        obj3 = self.TerseStructs3(field3=self.MyStruct(field1=1))
        for proto in Protocol:
            encoded_obj3 = self.serializer.serialize(obj3, protocol=proto)
            encoded_obj = self.serializer.serialize(obj, protocol=proto)
            self.assertEqual(encoded_obj3, encoded_obj)

    def test_terse_safe_patch(self) -> None:
        s = self.TerseSafePatch(version=1, data=IOBuf(b"abcdef"))
        self.thrift_serialization_round_trip(s)
