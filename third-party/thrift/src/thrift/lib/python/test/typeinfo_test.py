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

import typing
import unittest

from parameterized import parameterized

from thrift.python.mutable_typeinfos import (
    MutableListTypeInfo,
    MutableMapTypeInfo,
    MutableSetTypeInfo,
    MutableStructTypeInfo,
)

from thrift.python.test.typeinfo_test import TypeInfoTests as CTests
from thrift.python.types import (
    AdaptedTypeInfo,
    EnumTypeInfo,
    IntegerTypeInfo,
    IOBufTypeInfo,
    ListTypeInfo,
    MapTypeInfo,
    SetTypeInfo,
    StringTypeInfo,
    StructTypeInfo,
    TypeInfo,
)

# This python file serves as a boilerplate code for executing tests written
# in a Cython module. Simply import the Cython module containing the tests,
# and call the appropriate test functions within the TestCase class.


class TypeInfoTests(unittest.TestCase):

    @parameterized.expand(
        [
            AdaptedTypeInfo,
            EnumTypeInfo,
            IntegerTypeInfo,
            IOBufTypeInfo,
            ListTypeInfo,
            MapTypeInfo,
            MutableListTypeInfo,
            MutableSetTypeInfo,
            MutableStructTypeInfo,
            MutableMapTypeInfo,
            SetTypeInfo,
            StringTypeInfo,
            StructTypeInfo,
            TypeInfo,
        ]
    )
    def test_TypeInfo_classes_are_final(self, base_type: typing.Type[object]) -> None:
        with self.assertRaisesRegex(
            TypeError, f"{base_type.__name__}' is not an acceptable base type"
        ):

            class Derived(base_type):
                pass

    def test_IntegerTypeInfo(self) -> None:
        CTests(self).test_IntegerTypeInfo()

    def test_StringTypeInfo(self) -> None:
        CTests(self).test_StringTypeInfo()

    def test_ListTypeInfo(self) -> None:
        CTests(self).test_ListTypeInfo()

    def test_ListTypeInfo_nested(self) -> None:
        CTests(self).test_ListTypeInfo_nested()

    def test_SetTypeInfo(self) -> None:
        CTests(self).test_SetTypeInfo()

    def test_SetTypeInfo_nested(self) -> None:
        CTests(self).test_SetTypeInfo_nested()

    def test_TypeInfo(self) -> None:
        CTests(self).test_TypeInfo()

    def test_StructTypeInfo(self) -> None:
        CTests(self).test_StructTypeInfo()

    def test_EnumTypeInfo(self) -> None:
        CTests(self).test_EnumTypeInfo()

    def test_AdaptedTypeInfo(self) -> None:
        CTests(self).test_AdaptedTypeInfo()

    def test_IOBufTypeInfo(self) -> None:
        CTests(self).test_IOBufTypeInfo()

    def test_MapTypeInfo(self) -> None:
        CTests(self).test_MapTypeInfo()

    def test_MapTypeInfo_nested(self) -> None:
        CTests(self).test_MapTypeInfo_nested()

    def test_MutableListTypeInfo(self) -> None:
        CTests(self).test_MutableListTypeInfo()

    def test_MutableListTypeInfo_nested(self) -> None:
        CTests(self).test_MutableListTypeInfo_nested()

    def test_MutableSetTypeInfo(self) -> None:
        CTests(self).test_MutableSetTypeInfo()

    def test_MutableMapTypeInfo(self) -> None:
        CTests(self).test_MutableMapTypeInfo()
