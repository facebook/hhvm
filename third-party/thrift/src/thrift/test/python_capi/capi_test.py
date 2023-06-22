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

import thrift.python_capi.fixture as fixture

from thrift.test.python_capi.module.thrift_types import (  # @manual=:test_module-python-types
    DoubledPair,
    EmptyStruct,
    ListStruct,
    MyDataItem,
    MyEnum,
    MyStruct,
    MyStructPatch,  # this import breaks autodeps w/o manual
    MyUnion,
    PrimitiveStruct,
    StringPair,
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

    def my_union(self) -> MyUnion:
        return MyUnion(myStruct=self.my_struct())

    def primitive(self) -> PrimitiveStruct:
        return PrimitiveStruct(
            booly=True,
            charry=-9,
            shorty=-1,
            inty=2**31 - 1,
            longy=-(2**63),
            floaty=-1.0,
            dubby=-1.0,
            stringy="€ to £ to ₹",
            bytey=b"bippity boppity boo",
        )

    def primitive_unset(self) -> PrimitiveStruct:
        return PrimitiveStruct(
            booly=True,
            # charry leave deliberately unset, should be 0
            shorty=-1,
            inty=2**31 - 1,
            longy=-(2**63),
            # leave optional `floaty` `dubby`, `stringy`, `bytey` unset
        )

    def struct_patch(self) -> MyStructPatch:
        return MyStructPatch(
            assign=self.my_struct(),
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


class PythonCapiRoundtrip(PythonCapiFixture):
    def test_roundtrip_struct(self) -> None:
        i = MyDataItem()
        empty = MyStruct()
        s = self.my_struct()
        self.assertEqual(i, fixture.roundtrip_MyDataItem(i))
        self.assertEqual(empty, fixture.roundtrip_MyStruct(empty))
        self.assertEqual(s, fixture.roundtrip_MyStruct(s))

    def test_roundtrip_union(self) -> None:
        self.assertEqual(self.my_union(), fixture.roundtrip_MyUnion(self.my_union()))

    def test_roundtrip_enum(self) -> None:
        self.assertEqual(MyEnum.MyValue1, fixture.roundtrip_MyEnum(MyEnum.MyValue1))
        self.assertEqual(MyEnum.MyValue2, fixture.roundtrip_MyEnum(MyEnum.MyValue2))

    def test_roundtrip_struct_patch(self) -> None:
        self.assertEqual(
            self.struct_patch(), fixture.roundtrip_MyStructPatch(self.struct_patch())
        )
        empty_patch = MyStructPatch(assign=MyStruct())
        self.assertEqual(empty_patch, fixture.roundtrip_MyStructPatch(empty_patch))

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
        with self.assertRaises(AttributeError):
            fixture.roundtrip_MyEnum(self.my_struct())

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


class PythonCapiTypeCheck(PythonCapiFixture):
    def test_typeCheck_struct(self) -> None:
        i = MyDataItem()
        s = self.my_struct()
        self.assertTrue(fixture.check_MyDataItem(i))
        self.assertFalse(fixture.check_MyDataItem(s))
        self.assertTrue(fixture.check_MyStruct(s))
        self.assertFalse(fixture.check_MyStruct(i))

    def test_typeCheck_union(self) -> None:
        self.assertTrue(fixture.check_MyUnion(self.my_union()))
        self.assertFalse(fixture.check_MyUnion(self.my_struct()))
        self.assertFalse(fixture.check_MyUnion(MyEnum.MyValue1))

    def test_typeCheck_struct_patch(self) -> None:
        self.assertTrue(fixture.check_MyStructPatch(self.struct_patch()))
        self.assertFalse(fixture.check_MyStructPatch(self.my_struct()))
        self.assertFalse(fixture.check_MyStructPatch(MyEnum.MyValue1))

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
