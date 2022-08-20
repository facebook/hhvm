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

import asyncio
import types
import typing
import unittest

from testing.example.services import TestServiceInterface

from testing.example.types import (
    Color,
    ErrorWithEnum,
    ErrorWithMessageAnnotation,
    SimpleError,
    TestNestedStruct,
    TestStruct,
    TestStructSimple,
    TestStructWithBoxAnnotation,
    TestStructWithList,
    TestStructWithMixin,
    TestStructWithRefAnnotation,
    TestStructWithSet,
    TestUnion,
)
from thrift.py3.common import Protocol
from thrift.py3.serializer import deserialize
from thrift.py3.server import getServiceName


class NoLegacyTest(unittest.TestCase):
    def test_enum(self) -> None:
        color = Color.red
        self.assertEqual(color, Color.red)

    def test_simple_exception(self) -> None:
        try:
            raise SimpleError(errortext="testerror", retcode=-1)
        except SimpleError as exception:
            self.assertEqual(exception.errortext, "testerror")
            self.assertEqual(exception.retcode, -1)
        else:
            self.assertFalse("Should not get here")

    def test_exception_with_enum(self) -> None:
        try:
            raise ErrorWithEnum(color=Color.green, retcode=0)
        except ErrorWithEnum as exception:
            self.assertEqual(exception.color, Color.green)
            self.assertEqual(exception.retcode, 0)
        else:
            self.assertFalse("Should not get here")

    def test_exception_without_message_annotation(self) -> None:
        try:
            raise SimpleError(errortext="test message", retcode=-1)
        except SimpleError as exception:
            message = str(exception)
            self.assertEqual(message, "('test message', -1)")
        else:
            self.assertFalse("Should not get here")

    def test_exception_with_message_annotation(self) -> None:
        try:
            raise ErrorWithMessageAnnotation(errortext="test message", retcode=-1)
        except ErrorWithMessageAnnotation as exception:
            message = str(exception)
            self.assertEqual(message, "test message")
        else:
            self.assertFalse("Should not get here")

    def test_struct_with_all_fields_set(self) -> None:
        testStruct = TestStruct(
            field1="field1", field2="field2", field3=Color.blue, field4=Color.green
        )
        self.assertEqual(testStruct.field1, "field1")
        self.assertEqual(testStruct.field2, "field2")
        self.assertEqual(testStruct.field3, Color.blue)
        self.assertEqual(testStruct.field4, Color.green)

    def test_struct_with_optional_not_set(self) -> None:
        testStruct = TestStruct(field1="field1", field3=Color.green)
        self.assertEqual(testStruct.field1, "field1")
        self.assertIsNone(testStruct.field2)
        self.assertEqual(testStruct.field3, Color.green)
        self.assertIsNone(testStruct.field4)

    def test_struct_with_default_args(self) -> None:
        testStruct = TestStruct()
        self.assertEqual(testStruct.field1, "")
        self.assertIsNone(testStruct.field2)
        self.assertEqual(testStruct.field3, Color.red)
        self.assertIsNone(testStruct.field4)

    def test_struct_with_list(self) -> None:
        testStruct = TestStructWithList(numbers=[1, 2, 3])
        self.assertEquals(testStruct.numbers, [1, 2, 3])

    def test_struct_with_set(self) -> None:
        testStruct = TestStructWithSet(numbers={1, 2, 3})
        self.assertEqual(len(testStruct.numbers), 3)
        for i in [1, 2, 3]:
            self.assertTrue(i in testStruct.numbers)

    def test_nested_struct(self) -> None:
        testStruct = TestNestedStruct(nested=TestStructWithList(numbers=[1, 2, 3]))
        self.assertEqual(testStruct.nested.numbers, [1, 2, 3])

    def test_mixin_struct(self) -> None:
        testStruct = TestStructWithMixin(
            field3="field3", field4=TestStructSimple(field1="field1", field2=5)
        )
        self.assertEqual(testStruct.field3, "field3")
        self.assertEqual(testStruct.field1, "field1")
        self.assertEqual(testStruct.field2, 5)

    def test_struct_with_empty_box_data(self) -> None:
        testStruct = TestStructWithBoxAnnotation()
        self.assertIsNone(testStruct.data)

    def test_struct_with_valid_box_data(self) -> None:
        testStruct = TestStructWithBoxAnnotation(data=[1, 2, 3])
        self.assertEqual(testStruct.data, [1, 2, 3])

    def test_struct_with_empty_ref_data(self) -> None:
        testStruct = TestStructWithRefAnnotation()
        self.assertIsNone(testStruct.data)

    def test_struct_with_valid_ref_data(self) -> None:
        testStruct = TestStructWithRefAnnotation(data=[1, 2, 3])
        self.assertEqual(testStruct.data, [1, 2, 3])

    def test_empty_union(self) -> None:
        testUnion = TestUnion()
        self.assertEqual(testUnion.type, TestUnion.Type.EMPTY)

    def test_valid_union(self) -> None:
        tiny = 2**7 - 1
        union = TestUnion.fromValue(tiny)
        self.assertEqual(union.type, TestUnion.Type.tiny)
        self.assertEqual(union.tiny, tiny)

        small = 2**15 - 1
        union = TestUnion.fromValue(small)
        self.assertEqual(union.type, TestUnion.Type.small)
        self.assertEqual(union.small, small)

        medium = 2**31 - 1
        union = TestUnion.fromValue(medium)
        self.assertEqual(union.type, TestUnion.Type.medium)
        self.assertEqual(union.medium, medium)

        large = 2**63 - 1
        union = TestUnion.fromValue(large)
        self.assertEqual(union.type, TestUnion.Type.large)
        self.assertEqual(union.large, large)

        str_val = "Test string"
        union = TestUnion.fromValue(str_val)
        self.assertEqual(union.type, TestUnion.Type.strval)
        self.assertEqual(union.strval, str_val)

    def test_valid_union_with_ref_field_set(self) -> None:
        # Thrift-py3 union does not support python object ctors with the cpp.ref
        # Hence using deserialization to check that union cpp refs are supported
        testUnion = deserialize(
            TestUnion, b'{"dataptr":[1,2,3]}', protocol=Protocol.JSON
        )
        self.assertEqual(testUnion.type, TestUnion.Type.dataptr)
        self.assertEqual(testUnion.dataptr, [1, 2, 3])


class Handler(TestServiceInterface):
    async def getName(self) -> str:
        return "TestServiceName"

    async def invert(self, value: bool) -> bool:
        return not value

    async def processCollection(
        self, values: typing.Sequence[int], addValue: int, doThrow: bool
    ) -> typing.Sequence[int]:
        if doThrow:
            raise ErrorWithEnum(color=Color.red, retcode=-1)

        return [v + addValue for v in values]

    async def renamedMethod(self, ret: bool) -> bool:
        return ret


class NoLegacyServiceTests(unittest.TestCase):
    def test_get_service_name(self) -> None:
        h = Handler()
        self.assertEqual(getServiceName(h), "TestService")

    def test_annotations(self) -> None:
        annotations = TestServiceInterface.annotations
        self.assertIsInstance(annotations, types.MappingProxyType)
        self.assertFalse(annotations.get("NotAnAnnotation"))
        self.assertEqual(annotations["fun_times"], "yes")

    def test_get_name(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        ret = loop.run_until_complete(h.getName())
        self.assertEqual(ret, "TestServiceName")

    def test_invert(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        ret = loop.run_until_complete(h.invert(True))
        self.assertFalse(ret)

    def test_process_collection_no_throw(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        values = [1, 2, 3]
        ret = loop.run_until_complete(h.processCollection(values, 1, False))
        self.assertEqual(ret, [2, 3, 4])
        self.assertEqual(values, [1, 2, 3])

    def test_process_collection_throw(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        with self.assertRaises(ErrorWithEnum):
            loop.run_until_complete(h.processCollection([], 0, True))

    def test_renamed_method(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        ret = loop.run_until_complete(h.renamedMethod(True))
        self.assertTrue(ret)
