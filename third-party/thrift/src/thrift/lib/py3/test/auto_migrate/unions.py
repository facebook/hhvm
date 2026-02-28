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

import enum
import types
import unittest

from folly.iobuf import IOBuf
from testing.types import (
    _UnderscoreUnion,
    Color,
    ComplexUnion,
    easy,
    Integers,
    IOBufUnion,
    Misordered,
    ReservedUnion,
    ValueOrError,
)
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import (
    brokenInAutoMigrate,
    is_auto_migrated,
)
from thrift.py3.common import Protocol
from thrift.py3.serializer import deserialize
from thrift.py3.types import Struct, Union


class UnionTests(unittest.TestCase):
    def test_hashability(self) -> None:
        hash(Integers())

    def test_union_dir(self) -> None:
        expected = (
            [
                "digits",
            ]
            + (
                # thrift-python added these to reduce the disparity between
                # immutable and mutable union APIs.
                ["fbthrift_current_field", "fbthrift_current_value"]
                if is_auto_migrated()
                else []
            )
            + [
                "large",
                "medium",
                "name_",
                "small",
                "tiny",
                "type",
                "unbounded",
                "value",
            ]
        )
        self.assertEqual(expected, dir(Integers()))
        self.assertEqual(expected, dir(Integers))

    def test_union_enum_dir(self) -> None:
        contents = dir(Integers.Type)
        # 7 variants + EMPTY
        self.assertEqual(len(Integers.Type), 8, list(Integers.Type))
        for arm in Integers.Type:
            self.assertTrue(arm.name in contents)
        self.assertIn("__module__", contents)
        self.assertIn("__class__", contents)
        self.assertIn("__doc__", contents)
        self.assertIn("__members__", contents)
        for itype in iter(Integers.Type):
            self.assertTrue(itype.name in contents)

    def test_union_type_enum_name(self) -> None:
        type_enum = Integers.Type
        if is_auto_migrated():
            self.assertEqual(type_enum.__name__, "Integers")
        else:
            self.assertEqual(type_enum.__name__, "__IntegersType")

        self.assertIsInstance(Integers().type, Integers.Type)

    def test_union_enum_members(self) -> None:
        members = Integers.Type.__members__
        # Alias can't happen in this enum so these should always equal
        self.assertEqual(len(members), len(Integers.Type))
        for type in Integers.Type:
            self.assertIn(type.name, members)
            self.assertIs(type, members[type.name])

    def test_union_subclass(self) -> None:
        self.assertIsInstance(Integers(tiny=2).type, enum.Enum)
        self.assertTrue(issubclass(Integers.Type, enum.Enum))

    def test_deserialize_empty(self) -> None:
        x = deserialize(Integers, b"{}", Protocol.JSON)
        self.assertEqual(x.type, Integers.Type.EMPTY)

    def test_union_module_name(self) -> None:
        variant_prefix = "thrift_" if is_auto_migrated() else ""
        expected = f"testing.{variant_prefix}types"

        self.assertEqual(Integers.__module__, expected)
        self.assertEqual(Integers().__class__.__module__, expected)
        self.assertEqual(ComplexUnion.__module__, expected)
        self.assertEqual(ComplexUnion().__class__.__module__, expected)

    def test_union_usage(self) -> None:
        value = hash("i64")
        x = Integers(large=value)
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, x.get_type())
        self.assertEqual(x.type, Integers.Type.large)
        self.assertEqual(x.value, value)
        # Hashing Works
        s = {x}
        self.assertIn(x, s)
        # Repr is useful
        rx = repr(x)
        self.assertIn(str(value), rx)
        self.assertIn(x.type.name, rx)

    def test_multiple_values(self) -> None:
        with self.assertRaises(TypeError):
            Integers(small=1, large=2)

    def test_wrong_type(self) -> None:
        x = Integers(small=1)
        with self.assertRaises(AttributeError):
            x.large
        x.small

    def test_integers_fromValue(self) -> None:
        tiny = 2**7 - 1
        small = 2**15 - 1
        medium = 2**31 - 1
        large = 2**63 - 1
        union = Integers.fromValue(tiny)
        self.assertEqual(union.type, Integers.Type.tiny)
        union = Integers.fromValue(small)
        self.assertEqual(union.type, Integers.Type.small)
        union = Integers.fromValue(medium)
        self.assertEqual(union.type, Integers.Type.medium)
        union = Integers.fromValue(large)
        self.assertEqual(union.type, Integers.Type.large)

    def test_complexunion_fromValue(self) -> None:
        tiny = 2**7 - 1
        large = 2**63 - 1
        adouble = 3.14159265358
        # afloat is adouble rounded to 32-bit float
        afloat = 3.1415927410125732
        union = ComplexUnion.fromValue(tiny)
        self.assertEqual(union.type, ComplexUnion.Type.tiny)
        union = ComplexUnion.fromValue(large)
        self.assertEqual(union.type, ComplexUnion.Type.large)
        union = ComplexUnion.fromValue(afloat)
        self.assertEqual(union.value, afloat)
        self.assertEqual(union.type, ComplexUnion.Type.float_val)
        union = ComplexUnion.fromValue(adouble)
        # thrift-python has no mechanism to distinguish between float and double
        # TODO(T243911644): resolve thrift-py3 vs thrift-python behavior
        u_arm, u_val = (
            (ComplexUnion.Type.float_val, afloat)
            if is_auto_migrated()
            else (ComplexUnion.Type.double_val, adouble)
        )
        self.assertEqual(union.value, u_val)
        self.assertEqual(union.type, u_arm)
        union = ComplexUnion.fromValue(Color.red)
        self.assertEqual(union.type, ComplexUnion.Type.color)
        union = ComplexUnion.fromValue(easy())
        self.assertEqual(union.type, ComplexUnion.Type.easy_struct)
        union = ComplexUnion.fromValue("foo")
        self.assertEqual(union.type, ComplexUnion.Type.text)
        union = ComplexUnion.fromValue(b"ar")
        self.assertEqual(union.type, ComplexUnion.Type.raw)
        union = ComplexUnion.fromValue(True)
        self.assertEqual(union.type, ComplexUnion.Type.truthy)

    def test_misordered_fromValue(self) -> None:
        u = Misordered.fromValue(31)
        # BAD: thrift-python uses key order, thrift-py3 uses declaration order
        # this is a risk for auto-migration
        if is_auto_migrated():
            self.assertEqual(u.type, Misordered.Type.val64)
            self.assertEqual(u.val64, 31)
        else:
            self.assertEqual(u.type, Misordered.Type.val32)
            self.assertEqual(u.val32, 31)

        u = Misordered.fromValue("31")
        if is_auto_migrated():
            self.assertEqual(u.type, Misordered.Type.s2)
            self.assertEqual(u.s2, "31")
        else:
            self.assertEqual(u.type, Misordered.Type.s1)
            self.assertEqual(u.s1, "31")

    def test_float32_field(self) -> None:
        # thrift-py3 rounds to float32 via cython. We want to eventually
        # remove this behavior and update tests that expect float32 rounding.
        self.assertNotEqual(ComplexUnion(float_val=1.1).float_val, 1.1)
        self.assertEqual(ComplexUnion(float_val=1.1).float_val, 1.100000023841858)

        doubles = [1.1, 2.2, 3.3]
        floats = [1.100000023841858, 2.200000047683716, 3.299999952316284]
        u = ComplexUnion(float_list=doubles)
        self.assertNotEqual(u.float_list, doubles)
        self.assertEqual(u.float_list, floats)

        double_set = set(doubles)
        u = ComplexUnion(float_set=double_set)
        self.assertNotEqual(u.float_set, double_set)
        self.assertEqual(u.float_set, set(floats))

        double_map = {x: x for x in doubles}
        u = ComplexUnion(float_map=double_map)
        float_map = {x: x for x in floats}
        self.assertNotEqual(u.float_map, double_map)
        self.assertEqual(u.float_map, float_map)

    def test_iobuf_union(self) -> None:
        abuf = IOBuf(b"3.141592025756836")
        union = IOBufUnion.fromValue(abuf)
        self.assertEqual(union.type, IOBufUnion.Type.buf)
        self.assertEqual(bytes(union.buf), bytes(abuf))

    def test_reserved_union(self) -> None:
        x = ReservedUnion(from_="foo")
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, ReservedUnion.Type.from_)
        self.assertEqual(x.value, "foo")
        self.assertEqual(x.from_, "foo")

        x = ReservedUnion(nonlocal_=3)
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, ReservedUnion.Type.nonlocal_)
        self.assertEqual(x.value, 3)
        self.assertEqual(x.nonlocal_, 3)

        x = ReservedUnion(ok="bar")
        self.assertIsInstance(x, Union)
        self.assertEqual(x.type, ReservedUnion.Type.ok)
        self.assertEqual(x.value, "bar")
        self.assertEqual(x.ok, "bar")

    def test_underscore_union(self) -> None:
        x = _UnderscoreUnion(_a="foo")
        self.assertEqual(x.type, _UnderscoreUnion.Type._a)
        self.assertEqual(x._a, "foo")

        x = _UnderscoreUnion(_b=31)
        self.assertEqual(x.type, _UnderscoreUnion.Type._b)
        self.assertEqual(x._b, 31)

        if is_auto_migrated():
            self.assertEqual(x.Type.__name__, "_UnderscoreUnion")
        else:
            self.assertEqual(x.Type.__name__, "___UnderscoreUnionType")

    def test_instance_base_class(self) -> None:
        self.assertIsInstance(ComplexUnion(tiny=1), Union)
        self.assertIsInstance(ComplexUnion(tiny=1), Struct)
        self.assertIsInstance(ComplexUnion(tiny=1), ComplexUnion)
        self.assertNotIsInstance(ComplexUnion(tiny=1), ReservedUnion)
        self.assertNotIsInstance(3, Union)
        self.assertNotIsInstance(3, ComplexUnion)
        self.assertTrue(issubclass(ComplexUnion, Union))
        self.assertTrue(issubclass(ComplexUnion, Struct))
        self.assertFalse(issubclass(int, Union))
        self.assertFalse(issubclass(int, ComplexUnion))
        self.assertFalse(issubclass(Union, ComplexUnion))
        self.assertFalse(issubclass(Struct, ComplexUnion))
        self.assertFalse(issubclass(ComplexUnion, ReservedUnion))

    def test_subclass_not_allow_inheritance(self) -> None:
        thrift_python_err = r"Inheritance from generated thrift union .+ is deprecated. Please use composition."
        cython_err = (
            r"type '.+' is not an acceptable base type"
            if not hasattr(ValueOrError, "_FBTHRIFT__PYTHON_CLASS")
            else r"Inheritance of thrift-generated .+ from TestSubclass is deprecated."
        )
        err_regex = thrift_python_err if is_auto_migrated() else cython_err

        with self.assertRaisesRegex(TypeError, err_regex):
            types.new_class(
                "TestSubclass",
                bases=(ValueOrError,),
            )
