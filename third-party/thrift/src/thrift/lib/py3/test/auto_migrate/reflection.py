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

import unittest

import testing.types

from testing.clients import TestingService
from testing.services import TestingServiceInterface
from testing.types import (
    Color,
    easy,
    HardError,
    I32List,
    Integers,
    Messy,
    Runtime,
    SimpleError,
    StrI32ListMap,
    StrStrIntListMapMap,
)
from thrift.lib.py3.test.auto_migrate_util import brokenInAutoMigrate
from thrift.py3.reflection import (
    inspect,
    inspectable,
    NumberType,
    Qualifier,
    StructType,
)


class ReflectionTests(unittest.TestCase):
    # Fails due to containers not working
    @brokenInAutoMigrate()
    def test_struct(self) -> None:
        x = easy(val=1, an_int=Integers(small=300), name="foo", val_list=[1, 2, 3, 4])
        self.assertTrue(inspectable(x))
        self.assertTrue(inspectable(easy))
        r = inspect(x)
        self.assertEqual(r.name, "easy")
        self.assertIsInstance(r.kind, StructType)
        self.assertEqual(r.kind, StructType.STRUCT)
        self.assertEqual(r.annotations, {"anno1": "foo", "bar": "1"})

    def test_struct_fields(self) -> None:
        r = inspect(Messy)

        self.assertEqual(len(r.fields), 3)

        self.assertEqual(r.fields[0].id, 1)
        self.assertEqual(r.fields[0].name, "opt_field")
        self.assertEqual(r.fields[0].type, str)
        self.assertEqual(r.fields[0].kind, NumberType.NOT_A_NUMBER)
        self.assertEqual(r.fields[0].qualifier, Qualifier.OPTIONAL)
        self.assertEqual(r.fields[0].default, None)
        self.assertEqual(
            r.fields[0].annotations, {"some": "annotation", "a.b.c": "d.e.f"}
        )

        self.assertEqual(r.fields[1].id, 3)
        self.assertEqual(r.fields[1].name, "unq_field")
        self.assertEqual(r.fields[1].type, str)
        self.assertEqual(r.fields[1].kind, NumberType.NOT_A_NUMBER)
        self.assertEqual(r.fields[1].qualifier, Qualifier.UNQUALIFIED)
        self.assertEqual(r.fields[1].default, "xyzzy")
        self.assertEqual(r.fields[1].annotations, {})

        self.assertEqual(r.fields[2].id, 4)
        self.assertEqual(r.fields[2].name, "struct_field")
        self.assertEqual(r.fields[2].type, Runtime)
        self.assertEqual(r.fields[2].kind, NumberType.NOT_A_NUMBER)
        self.assertEqual(r.fields[2].qualifier, Qualifier.UNQUALIFIED)
        self.assertEqual(
            r.fields[2].default,
            Runtime(bool_val=True, enum_val=Color.blue, int_list_val=[10, 20, 30]),
        )
        self.assertEqual(r.fields[2].annotations, {})

    def test_union(self) -> None:
        x = Integers(large=100)
        self.assertTrue(inspectable(x))
        self.assertTrue(inspectable(Integers))
        r = inspect(x)
        self.assertEqual(r.name, "Integers")
        self.assertEqual(r.kind, StructType.UNION)
        self.assertEqual(r.annotations, {})

    def test_exception(self) -> None:
        x = SimpleError(color=Color.red)
        self.assertTrue(inspectable(x))
        self.assertTrue(inspectable(SimpleError))
        r = inspect(x)
        self.assertEqual(r.name, "SimpleError")
        self.assertEqual(r.kind, StructType.EXCEPTION)
        self.assertEqual(r.annotations, {})

    @brokenInAutoMigrate()
    def test_list_element(self) -> None:
        x = testing.types.List__i32([1, 2, 3])
        self.assertTrue(inspectable(x))
        self.assertTrue(inspectable(testing.types.List__i32))
        r = inspect(x)
        self.assertEqual(r.value, int)
        self.assertEqual(r.kind, NumberType.I32)

    @brokenInAutoMigrate()
    def test_set_element(self) -> None:
        x = testing.types.Set__Color({Color.red, Color.blue})
        self.assertTrue(inspectable(x))
        self.assertTrue(inspectable(testing.types.Set__Color))
        r = inspect(x)
        self.assertEqual(r.value, Color)
        self.assertEqual(r.kind, NumberType.NOT_A_NUMBER)

    # Fails because thrift python list values are not inspectible
    @brokenInAutoMigrate()
    def test_map_key_value(self) -> None:
        x = StrStrIntListMapMap({"a": StrI32ListMap({"b": I32List([7, 8, 9])})})
        self.assertTrue(inspectable(x))
        self.assertTrue(inspectable(StrStrIntListMapMap))
        r = inspect(x)
        self.assertEqual(r.key, str)
        self.assertEqual(r.value, StrI32ListMap)
        self.assertEqual(r.key_kind, NumberType.NOT_A_NUMBER)
        self.assertEqual(r.value_kind, NumberType.NOT_A_NUMBER)

    def test_other(self) -> None:
        x = None
        self.assertFalse(inspectable(x))
        self.assertFalse(inspectable(type(x)))
        x = 3
        self.assertFalse(inspectable(x))
        self.assertFalse(inspectable(type(x)))
