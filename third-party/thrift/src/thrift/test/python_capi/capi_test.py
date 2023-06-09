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
    MyDataItem,
    MyEnum,
    MyStruct,
    MyStructPatch,  # this import breaks autodeps w/o manual
    MyUnion,
)


class PythonCapiTest(unittest.TestCase):
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

    def struct_patch(self) -> MyStructPatch:
        return MyStructPatch(
            assign=self.my_struct(),
        )

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

    def test_roundtrip_TypeError(self) -> None:
        with self.assertRaises(TypeError):
            fixture.roundtrip_MyDataItem(MyEnum.MyValue1)
        with self.assertRaises(TypeError):
            fixture.roundtrip_MyUnion(MyEnum.MyValue1)
        with self.assertRaises(AttributeError):
            fixture.roundtrip_MyEnum(self.my_struct())

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
