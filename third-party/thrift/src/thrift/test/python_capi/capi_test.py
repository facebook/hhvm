#!/usr/bin/env fbpython
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

import unittest
from sys import getrefcount
from typing import Generator

import thrift.python_capi.fixture as fixture

from folly.iobuf import IOBuf
from thrift.python.serializer import Protocol, serialize, serialize_iobuf
from thrift.test.python_capi.module.thrift_types import (
    AdaptedFields,
    AnnoyingEnum,
    ComposeStruct,
    DoubledPair,
    EmptyStruct,
    ListStruct,
    MapStruct,
    MyDataItem,
    MyEnum,
    MyStruct,
    Onion as MyUnion,
    PrimitiveStruct,
    SetStruct,
    StringPair,
)
from thrift.test.python_capi.serialized_dep.thrift_types import (
    SerializedError,
    SerializedStruct,
    SerializedUnion,
)

from thrift.test.python_capi.thrift_dep.thrift_types import (
    DepEnum,
    DepStruct,
    SomeError,
)


class PythonCapiFixture(unittest.TestCase):
    def my_struct(self) -> MyStruct:
        return MyStruct(
            inty=1,
            stringy="hello",
            myItemy=MyDataItem(),
            myEnumy=MyEnum.MyValue1,
            booly=True,
            floatListy=[-1.0, 1.0, 2.0, 3.0],
            strMappy={b"hello": "world", b"-1": "-1"},
            intSetty={-1, 1, 2, 3, 5, 8},
        )

    def my_union(self) -> Generator[MyUnion, None, None]:
        yield MyUnion()
        yield MyUnion(myEnum=MyEnum.MyValue1)
        yield MyUnion(myStruct=self.primitive())
        yield MyUnion(myString="acef")
        yield MyUnion(doubleList=[1.0, 2.0, 3.0])
        yield MyUnion(strMap={b"key": "val", b"bytes": "str"})

    def primitive(self) -> PrimitiveStruct:
        return PrimitiveStruct(
            booly=True,
            charry=-9,
            shorty=2**15 - 1,
            inty=2**31 - 1,
            longy=2**63 - 1,
            floaty=-1.0,
            dubby=-1.0,
            stringy="€ to £ to ₹",
            bytey=b"bippity boppity boo",
            buffy=IOBuf(memoryview(b" the buffest buffer ")),
            pointbuffy=IOBuf(memoryview(b"the pointiest buffer")),
            patched_struct=self.my_struct(),
            empty_struct=EmptyStruct(),
            some_error=SomeError(msg="bad math"),
            fbstring=b"v fast string",
            managed_string_view="I'm an rpc string utility",
        )

    def primitive_unset(self) -> PrimitiveStruct:
        return PrimitiveStruct(
            booly=True,
            # charry left deliberately unset, should be 0
            shorty=0,
            inty=2**31 - 1,
            longy=2**63 - 1,
            # leave optional `floaty` `dubby`, `stringy`, `bytey` unset
        )

    def adapted_fields(self) -> AdaptedFields:
        return AdaptedFields(
            adapted_int=4247,
            list_adapted_int=[1, 1, 2, 3, 5, 8],
            set_adapted_int={2, 3, 5, 7, 11, 13},
            inline_adapted_int=47,
        )

    def list_struct(self) -> ListStruct:
        return ListStruct(
            boolz=[True, True, False, False, False, False, True, True, False, True],
            intz=[-1, -2, -1, 0, 1, 2, 2, 2, 2, 10],
            stringz=["wat", "", "-1", "-1", "lol", "loool"],
            encoded=[b"beep", b"boop", b"bop"],
            uidz=[-(2**63), -1, 0, 1, 2**63 - 1],
            matrix=[[4.0, 9.0, 2.0], [3.0, 5.0, 7.0], [8.0, 1.0, 6.0]],
            ucharz=[[2, 7, 6], [9, 5, 1], [4, 3, 8]],
            voxels=[
                [[2, 7, 6], [9, 5, 1], [4, 3, 8]],
                [[2, 7, 6], [9, 5, 1], [4, 3, 8]],
                [[2, 7, 6], [9, 5, 1], [4, 3, 8]],
            ],
            buf_ptrs=[IOBuf(memoryview(x)) for x in [b"abc", b"def", b"ghi"]],
        )

    def empty_lists(self) -> ListStruct:
        # optional fields left unset
        return ListStruct(
            boolz=[],
            encoded=[],
            uidz=[],
            matrix=[],
            ucharz=[[], [9, 5, 1], []],
            voxels=[[], [[]], [[], [3], []]],
        )

    def set_struct(self) -> SetStruct:
        return SetStruct(
            enumz={MyEnum.MyValue1, MyEnum.MyValue2},
            intz={1, 1, 2, 3, 5, 8, 13, 23, 42},
            binnaz={b"abcd", b"efgh", b"ijkl", b"mnop"},
            encoded={b"abcd", b"bcda", b"cdab", b"dabc"},
            uidz={0, 10, 100, 1000, 10000},
            charz={0, 1, 2, 4, 8, 16},
            setz=[{1, 2, 3}, {}, {2, 3}, {1, 2, 3}],
        )

    def empty_sets(self) -> SetStruct:
        return SetStruct(
            enumz={},
            intz={},
            binnaz={},
            encoded={},
            uidz={},
            charz={},
            setz=[{}],
        )

    def map_struct(self) -> MapStruct:
        return MapStruct(
            enumz={MyEnum.MyValue1: "V1", MyEnum.MyValue2: "V2"},
            intz={i: str(i) for i in range(-3, 3)},
            binnaz={b"a": self.primitive(), b"b": self.primitive()},
            encoded={"wdf": 3.1, "wef": 2.9},
            flotz={i: float(i) for i in range(5)},
            map_list=[{i: i**2 for i in range(j)} for j in range(2)],
            list_map={-1: [1, -2, 3, -5], 2: [4, -8, 16]},
            fast_list_map={1: [-1.0, 1.0], -1: [1.0, -1.0]},
            buf_map={x: IOBuf(memoryview(x)) for x in [b"qergq", b"", b"wefwi"]},
            unsigned_list_map={1: [1, 2, 3, 5], 2: [4, 8, 16]},
        )

    def empty_maps(self) -> MapStruct:
        return MapStruct(
            enumz={},
            encoded={},
            flotz={},
            map_list=[{}],
            list_map={},
            fast_list_map={},
        )

    def dep_struct(self) -> DepStruct:
        return DepStruct(
            s="blah",
            i=42,
        )

    def composed(self) -> ComposeStruct:
        return ComposeStruct(
            enum_=MyEnum.MyValue2,
            renamed_=AnnoyingEnum.FOO,
            primitive=self.primitive(),
            aliased=self.list_struct(),
            xenum=DepEnum.Arm2,
            xstruct=self.dep_struct(),
            friends=[self.dep_struct()] * 3,
            serial_struct=SerializedStruct(s="wefw", i=42),
            serial_union=SerializedUnion(i=47),
            serial_error=SerializedError(msg="tldr"),
        )


class PythonCapiRoundtrip(PythonCapiFixture):
    def test_roundtrip_struct(self) -> None:
        i = MyDataItem()
        empty = MyStruct()
        s = self.my_struct()
        self.assertEqual(i, fixture.roundtrip_MyDataItem(i))
        self.assertEqual(empty, fixture.roundtrip_MyStruct(empty))
        self.assertEqual(s, fixture.roundtrip_MyStruct(s))

    def test_roundtrip_union(self) -> None:
        for u in self.my_union():
            self.assertEqual(u, fixture.roundtrip_MyUnion(u))

    def test_roundtrip_enum(self) -> None:
        self.assertEqual(MyEnum.MyValue1, fixture.roundtrip_MyEnum(MyEnum.MyValue1))
        self.assertEqual(MyEnum.MyValue2, fixture.roundtrip_MyEnum(MyEnum.MyValue2))

    def test_roundtrip_field_adapted(self) -> None:
        a, b = ("TacosSalad", "DaLassoCat")
        s = StringPair(normal=a, doubled=b)
        self.assertEqual(s, fixture.roundtrip_StringPair(s)),

    def test_roundtrip_type_adapted(self) -> None:
        s = DoubledPair(s="TacosSalad", x=42)
        self.assertEqual(s, fixture.roundtrip_DoubledPair(s))

    def test_roundtrip_marshal_EmptyStruct(self) -> None:
        self.assertEqual(EmptyStruct(), fixture.roundtrip_EmptyStruct(EmptyStruct()))
        with self.assertRaises(TypeError):
            fixture.roundtrip_EmptyStruct(MyStruct())

    def test_roundtrip_TypeError(self) -> None:
        with self.assertRaises(TypeError):
            fixture.roundtrip_MyDataItem(MyEnum.MyValue1)
        with self.assertRaises(TypeError):
            fixture.roundtrip_MyUnion(MyEnum.MyValue1)
        with self.assertRaises(TypeError):
            fixture.roundtrip_MyEnum(self.my_struct())

    def test_roundtrip_OverflowError(self) -> None:
        ## Failures on extraction to cpp
        negative_msg = "can't convert negative"
        with self.assertRaisesRegex(OverflowError, negative_msg):
            fixture.roundtrip_PrimitiveStruct(PrimitiveStruct(shorty=-1))
        with self.assertRaisesRegex(OverflowError, negative_msg):
            fixture.roundtrip_PrimitiveStruct(PrimitiveStruct(longy=-1))
        with self.assertRaisesRegex(OverflowError, negative_msg):
            fixture.roundtrip_MapStruct(MapStruct(unsigned_list_map={1: [1, -1]}))
        with self.assertRaisesRegex(OverflowError, negative_msg):
            fixture.roundtrip_MapStruct(MapStruct(unsigned_list_map={-1: [1, 3]}))

        ## Failure on creation of thrift-python object (existing behavior)
        with self.assertRaises(OverflowError):
            fixture.roundtrip_PrimitiveStruct(PrimitiveStruct(shorty=2**15))

    def test_roundtrip_marshal_PrimitiveStruct(self) -> None:
        self.assertEqual(
            PrimitiveStruct(), fixture.roundtrip_PrimitiveStruct(PrimitiveStruct())
        )
        self.assertEqual(
            self.primitive(), fixture.roundtrip_PrimitiveStruct(self.primitive())
        )
        self.assertEqual(
            self.primitive_unset(),
            fixture.roundtrip_PrimitiveStruct(self.primitive_unset()),
        )
        unset_primitive = fixture.roundtrip_PrimitiveStruct(self.primitive_unset())
        self.assertIsNone(unset_primitive.floaty)
        self.assertIsNone(unset_primitive.dubby)
        self.assertIsNone(unset_primitive.stringy)
        self.assertIsNone(unset_primitive.bytey)
        with self.assertRaises(TypeError):
            fixture.roundtrip_PrimitiveStruct(self.my_struct())

    def test_memleak_primitive(self) -> None:
        # Use non-singleton objects to avoid noise from runtime
        short = 9001
        f = 9001.0
        bytes_ = b"bippity boppity boo"

        def make_primitive():
            return PrimitiveStruct(
                shorty=short,
                inty=short,
                longy=short,
                floaty=f,
                dubby=f,
                bytey=bytes_,
            )

        primitive = make_primitive()
        # This test works to detect leaks of primitives only because they are
        # placed directly into struct internal data without conversion.
        # Non-primitives can be leaked, but not detectable by this test.
        self.assertIs(primitive.shorty, short)
        self.assertIs(primitive.inty, short)
        self.assertIs(primitive.longy, short)
        self.assertIs(primitive.floaty, f)
        self.assertIs(primitive.dubby, f)
        self.assertIs(primitive.bytey, bytes_)

        short_refcount = getrefcount(short)
        f_refcount = getrefcount(f)
        bytes_refcount = getrefcount(bytes_)

        for _ in range(10):
            fixture.roundtrip_PrimitiveStruct(make_primitive())

        # These all fail if there is a leak in Extractor<PrimitiveStruct>
        self.assertEqual(bytes_refcount, getrefcount(bytes_))
        self.assertEqual(f_refcount, getrefcount(f))
        self.assertEqual(short_refcount, getrefcount(short))

    def test_roundtrip_marshal_ListStruct(self) -> None:
        self.assertEqual(ListStruct(), fixture.roundtrip_ListStruct(ListStruct()))
        self.assertEqual(
            self.list_struct(), fixture.roundtrip_ListStruct(self.list_struct())
        )
        self.assertEqual(
            self.empty_lists(), fixture.roundtrip_ListStruct(self.empty_lists())
        )
        self.assertIsNone(fixture.roundtrip_ListStruct(self.empty_lists()).intz)
        self.assertIsNone(fixture.roundtrip_ListStruct(self.empty_lists()).stringz)

    def test_roundtrip_marshal_SetStruct(self) -> None:
        self.assertEqual(SetStruct(), fixture.roundtrip_SetStruct(SetStruct()))
        self.assertEqual(
            self.empty_sets(), fixture.roundtrip_SetStruct(self.empty_sets())
        )
        expected = self.set_struct()
        actual = fixture.roundtrip_SetStruct(self.set_struct())
        # sets are serialized in a non-sorted order, so compare field by field
        for f in ["enumz", "intz", "binnaz", "encoded", "uidz", "charz", "setz"]:
            self.assertEqual(getattr(expected, f), getattr(actual, f), f)

    def test_roundtrip_marshal_MapStruct(self) -> None:
        self.assertEqual(MapStruct(), fixture.roundtrip_MapStruct(MapStruct()))
        self.assertEqual(
            self.empty_maps(), fixture.roundtrip_MapStruct(self.empty_maps())
        )
        expected = self.map_struct()
        actual = fixture.roundtrip_MapStruct(self.map_struct())
        for f in [
            "enumz",
            "intz",
            "binnaz",
            "encoded",
            "flotz",
            "map_list",
            "list_map",
            "fast_list_map",
        ]:
            self.assertEqual(getattr(expected, f), getattr(actual, f), f)

    def test_roundtrip_marshal_ComposeStruct(self) -> None:
        self.assertEqual(
            ComposeStruct(), fixture.roundtrip_ComposeStruct(ComposeStruct())
        )
        self.assertEqual(
            self.composed(), fixture.roundtrip_ComposeStruct(self.composed())
        )

    def test_roundtrip_marshal_AdaptedFields(self) -> None:
        self.assertEqual(
            AdaptedFields(), fixture.roundtrip_AdaptedFields(AdaptedFields())
        )
        self.assertEqual(
            self.adapted_fields(),
            fixture.roundtrip_AdaptedFields(self.adapted_fields()),
        )


class PythonCapiTypeCheck(PythonCapiFixture):
    def test_typeCheck_struct(self) -> None:
        i = MyDataItem()
        s = self.my_struct()
        self.assertTrue(fixture.check_MyDataItem(i))
        self.assertFalse(fixture.check_MyDataItem(s))
        self.assertTrue(fixture.check_MyStruct(s))
        self.assertFalse(fixture.check_MyStruct(i))

    def test_typeCheck_union(self) -> None:
        for u in self.my_union():
            self.assertTrue(fixture.check_MyUnion(u))
        self.assertFalse(fixture.check_MyUnion(self.my_struct()))
        self.assertFalse(fixture.check_MyUnion(MyEnum.MyValue1))

    def test_typeCheck_enum(self) -> None:
        self.assertTrue(fixture.check_MyEnum(MyEnum.MyValue1))
        self.assertTrue(fixture.check_MyEnum(MyEnum.MyValue2))
        self.assertFalse(fixture.check_MyEnum(self.my_struct()))

    def test_roundtrip_field_adapted(self) -> None:
        a, b = ("TacosSalad", "DaLassoCat")
        self.assertTrue(fixture.check_StringPair(StringPair(normal=a, doubled=b)))
        self.assertFalse(fixture.check_StringPair(MyEnum.MyValue1))

    def test_roundtrip_type_adapted(self) -> None:
        self.assertTrue(
            fixture.check_DoubledPair(DoubledPair(s="TacosSalad" * 2, x=42))
        )
        self.assertFalse(fixture.check_DoubledPair(MyEnum.MyValue1))

    def test_typeCheck_PrimitiveStruct(self) -> None:
        self.assertTrue(fixture.check_PrimitiveStruct(self.primitive()))
        self.assertTrue(fixture.check_PrimitiveStruct(PrimitiveStruct()))
        self.assertFalse(fixture.check_PrimitiveStruct(MyEnum.MyValue1))
        self.assertFalse(fixture.check_PrimitiveStruct(self.my_struct()))

    def test_typeCheck_ListStruct(self) -> None:
        self.assertTrue(fixture.check_ListStruct(self.list_struct()))
        self.assertTrue(fixture.check_ListStruct(self.empty_lists()))
        self.assertTrue(fixture.check_ListStruct(ListStruct()))
        self.assertFalse(fixture.check_ListStruct(MyEnum.MyValue1))
        self.assertFalse(fixture.check_ListStruct(self.my_struct()))

    def test_typeCheck_SetStruct(self) -> None:
        self.assertTrue(fixture.check_SetStruct(self.set_struct()))
        self.assertTrue(fixture.check_SetStruct(self.empty_sets()))
        self.assertTrue(fixture.check_SetStruct(SetStruct()))
        self.assertFalse(fixture.check_SetStruct(MyEnum.MyValue1))
        self.assertFalse(fixture.check_SetStruct(self.my_struct()))

    def test_typeCheck_MapStruct(self) -> None:
        self.assertTrue(fixture.check_MapStruct(self.map_struct()))
        self.assertTrue(fixture.check_MapStruct(self.empty_maps()))
        self.assertTrue(fixture.check_MapStruct(MapStruct()))
        self.assertFalse(fixture.check_MapStruct(MyEnum.MyValue1))
        self.assertFalse(fixture.check_MapStruct(self.my_struct()))

    def test_typeCheck_ComposeStruct(self) -> None:
        self.assertTrue(fixture.check_ComposeStruct(self.composed()))
        self.assertTrue(fixture.check_ComposeStruct(ComposeStruct()))
        self.assertFalse(fixture.check_ComposeStruct(MyEnum.MyValue1))
        self.assertFalse(fixture.check_ComposeStruct(self.my_struct()))


class PythonCapiSerializeParity(PythonCapiFixture):
    def serialize(self, s: object) -> IOBuf:
        return serialize_iobuf(s, protocol=Protocol.BINARY)

    def test_PrimitiveStruct_extract(self) -> None:
        self.assertEqual(
            bytes(fixture.extract_and_serialize_PrimitiveStruct(PrimitiveStruct())),
            serialize(PrimitiveStruct(), protocol=Protocol.BINARY),
        )
        # need to actually create a thrift-cpp2 struct with both methods
        # to ensure consistent ordering of map and set fields
        self.assertEqual(
            fixture.extract_and_serialize_PrimitiveStruct(self.primitive()),
            fixture.deserialize_and_serialize_PrimitiveStruct(
                self.serialize(self.primitive())
            ),
        )

    def test_MyStruct_extract(self) -> None:
        self.assertEqual(
            fixture.extract_and_serialize_MyStruct(self.my_struct()),
            fixture.deserialize_and_serialize_MyStruct(
                self.serialize(self.my_struct())
            ),
        )

    def test_AdaptedFields_extract(self) -> None:
        self.assertEqual(
            fixture.extract_and_serialize_AdaptedFields(self.adapted_fields()),
            fixture.deserialize_and_serialize_AdaptedFields(
                self.serialize(self.adapted_fields())
            ),
        )

    def test_ListStruct_extract(self) -> None:
        self.assertEqual(
            fixture.extract_and_serialize_ListStruct(self.list_struct()),
            fixture.deserialize_and_serialize_ListStruct(
                self.serialize(self.list_struct())
            ),
        )

    def test_SetStruct_extract(self) -> None:
        self.assertEqual(
            fixture.extract_and_serialize_SetStruct(self.set_struct()),
            fixture.deserialize_and_serialize_SetStruct(
                self.serialize(self.set_struct())
            ),
        )

    def test_MapStruct_extract(self) -> None:
        self.assertEqual(
            fixture.extract_and_serialize_MapStruct(self.map_struct()),
            fixture.deserialize_and_serialize_MapStruct(
                self.serialize(self.map_struct())
            ),
        )

    def test_ComposeStruct_extract(self) -> None:
        self.assertEqual(
            fixture.extract_and_serialize_ComposeStruct(self.composed()),
            fixture.deserialize_and_serialize_ComposeStruct(
                self.serialize(self.composed())
            ),
        )
