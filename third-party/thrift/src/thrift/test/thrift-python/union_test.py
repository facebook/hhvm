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

import enum
import importlib
import unittest

from thrift.test.thrift_python.union_test.thrift_types import (
    TestStruct as TestStructImmutable,
    TestUnion as TestUnionImmutable,
    TestUnionAmbiguousFromValueBoolInt as TestUnionAmbiguousFromValueBoolIntImmutable,
    TestUnionAmbiguousFromValueFloatInt as TestUnionAmbiguousFromValueFloatIntImmutable,
    TestUnionAmbiguousFromValueIntBool as TestUnionAmbiguousFromValueIntBoolImmutable,
    TestUnionAmbiguousTypeFieldName as TestUnionAmbiguousTypeFieldNameImmutable,
    TestUnionAmbiguousValueFieldName as TestUnionAmbiguousValueFieldNameImmutable,
)


class ThriftPython_ImmutableUnion_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation_and_read(self) -> None:
        u = TestUnionImmutable()
        self.assertIs(u.type, TestUnionImmutable.Type.EMPTY)
        self.assertIsNone(u.value)

        u2 = TestUnionImmutable(string_field="Hello, world!")
        self.assertIs(u2.type, TestUnionImmutable.Type.string_field)
        self.assertEqual(u2.value, "Hello, world!")
        self.assertEqual(u2.string_field, "Hello, world!")
        with self.assertRaisesRegex(
            AttributeError, "Union contains a value of type string_field, not int_field"
        ):
            u2.int_field

    def test_class_field_enum(self) -> None:
        # The "Type" class attribute is an enumeration type
        self.assertTrue(hasattr(TestUnionImmutable, "Type"))
        fields_enum_type = TestUnionImmutable.Type
        self.assertTrue(issubclass(fields_enum_type, enum.Enum))
        self.assertIsInstance(fields_enum_type, enum.EnumMeta)

        # It has the same name as the union class itself:
        self.assertEqual(fields_enum_type.__name__, "TestUnion")

        # It holds members for each field, as well as an extra EMPTY one.
        self.assertCountEqual(
            [
                "string_field",
                "int_field",
                "struct_field",
                "EMPTY",
            ],
            fields_enum_type.__members__.keys(),
        )

        self.assertIs(fields_enum_type(0), fields_enum_type.EMPTY)

        names_and_values = {
            member.name: member.value
            for member in fields_enum_type.__members__.values()
        }

        self.assertEqual(
            names_and_values,
            {"EMPTY": 0, "string_field": 1, "int_field": 2, "struct_field": 3},
        )

    def test_from_value(self) -> None:
        self.assertEqual(
            TestUnionImmutable.fromValue("hello, world!"),
            TestUnionImmutable(string_field="hello, world!"),
        )
        self.assertEqual(
            TestUnionImmutable.fromValue(42), TestUnionImmutable(int_field=42)
        )
        self.assertEqual(
            TestUnionImmutable.fromValue(
                TestStructImmutable(unqualified_string="hello, world!")
            ),
            TestUnionImmutable(
                struct_field=TestStructImmutable(unqualified_string="hello, world!")
            ),
        )

    def test_from_value_none(self) -> None:
        # BAD: fromValue(None) results in a weird state, where u.type is None
        # rather than EMPTY.
        union_none = TestUnionImmutable.fromValue(None)
        self.assertIsNone(union_none.type)
        self.assertIsNone(union_none.value)

    def test_from_value_ambiguous_int_bool(self) -> None:
        union_int_bool_1 = TestUnionAmbiguousFromValueIntBoolImmutable.fromValue(1)
        self.assertIs(
            union_int_bool_1.type,
            TestUnionAmbiguousFromValueIntBoolImmutable.Type.int_field,
        )
        self.assertEqual(union_int_bool_1.value, 1)
        self.assertEqual(union_int_bool_1.int_field, 1)

        # BAD: fromValue(bool) populates an int field if it comes before bool.
        union_int_bool_2 = TestUnionAmbiguousFromValueIntBoolImmutable.fromValue(True)
        self.assertIs(
            union_int_bool_2.type,
            TestUnionAmbiguousFromValueIntBoolImmutable.Type.int_field,
        )
        self.assertEqual(union_int_bool_2.value, 1)
        self.assertEqual(union_int_bool_2.int_field, 1)

    def test_from_value_ambiguous_bool_int(self) -> None:
        # BAD: Unlike the previous test case, fromValue(int) does not populate
        # a bool field if seen first.
        union_bool_int_1 = TestUnionAmbiguousFromValueBoolIntImmutable.fromValue(1)
        self.assertIs(
            union_bool_int_1.type,
            TestUnionAmbiguousFromValueBoolIntImmutable.Type.int_field,
        )
        self.assertEqual(union_bool_int_1.value, 1)
        self.assertEqual(union_bool_int_1.int_field, 1)
        self.assertEqual(union_bool_int_1.int_field, True)

        union_bool_int_2 = TestUnionAmbiguousFromValueBoolIntImmutable.fromValue(True)
        self.assertIs(
            union_bool_int_2.type,
            TestUnionAmbiguousFromValueBoolIntImmutable.Type.bool_field,
        )
        self.assertEqual(union_bool_int_2.value, True)
        self.assertEqual(union_bool_int_2.value, 1)
        self.assertEqual(union_bool_int_2.bool_field, 1)

    def test_from_value_ambiguous_float_int(self) -> None:
        # BAD: fromValue(int) populated a float field if it comes before int.
        union_float_int_1 = TestUnionAmbiguousFromValueFloatIntImmutable.fromValue(1)
        self.assertIs(
            union_float_int_1.type,
            TestUnionAmbiguousFromValueFloatIntImmutable.Type.float_field,
        )
        self.assertEqual(union_float_int_1.value, 1.0)
        self.assertEqual(union_float_int_1.float_field, 1)

        union_float_int_2 = TestUnionAmbiguousFromValueFloatIntImmutable.fromValue(1.0)
        self.assertIs(
            union_float_int_2.type,
            TestUnionAmbiguousFromValueFloatIntImmutable.Type.float_field,
        )
        self.assertEqual(union_float_int_2.value, 1.0)
        self.assertEqual(union_float_int_2.float_field, 1)

    def test_field_name_conflict(self) -> None:
        # BAD: All statements below should actually pass, but are failing due
        # to the (undocumented) reserved attributes colliding with IDL field names.
        with self.assertRaises(TypeError):
            TestUnionAmbiguousTypeFieldNameImmutable()

        self.assertNotIsInstance(
            TestUnionAmbiguousTypeFieldNameImmutable.Type, enum.EnumMeta
        )

        u = TestUnionAmbiguousValueFieldNameImmutable(value=42)
        self.assertEqual(u.value, 42)
        with self.assertRaises(AttributeError):
            u.type

        u2 = TestUnionAmbiguousValueFieldNameImmutable(type=123)
        with self.assertRaises(AttributeError):
            u2.value

    def test_hash(self) -> None:
        hash(TestUnionImmutable())

    def test_equality(self) -> None:
        u1 = TestUnionImmutable(string_field="hello")
        u2 = TestUnionImmutable(string_field="hello")
        self.assertIsNot(u1, u2)
        self.assertEqual(u1, u2)

        u3 = TestUnionImmutable(string_field="world")
        self.assertIsNot(u1, u2)
        self.assertNotEqual(u1, u3)

    def test_ordering(self) -> None:
        self.assertLess(
            TestUnionImmutable(string_field="hello"),
            TestUnionImmutable(string_field="world"),
        )
        self.assertGreater(
            TestUnionImmutable(int_field=42),
            TestUnionImmutable(string_field="world"),
        )
        self.assertLess(
            TestUnionImmutable(int_field=41),
            TestUnionImmutable(int_field=42),
        )


class ThriftPython_MutableUnion_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_import_fails_not_implemented_yet(self) -> None:
        with self.assertRaises(NotImplementedError):
            importlib.import_module(
                "thrift.test.thrift_python.union_test.thrift_mutable_types"
            )
