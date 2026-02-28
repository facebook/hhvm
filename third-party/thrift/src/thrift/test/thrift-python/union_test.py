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
import typing
import unittest
from datetime import datetime

import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
from thrift.python.mutable_types import MutableStructOrUnion, to_thrift_list
from thrift.python.types import StructOrUnion as ImmutableStructOrUnion
from thrift.test.thrift_python.union_test.thrift_mutable_types import (
    DefaultFieldUnion as DefaultFieldUnionMutable,
    TestStruct as TestStructMutable,
    TestUnion as TestUnionMutable,
    TestUnionAdaptedTypes as TestUnionAdaptedTypesMutable,
)
from thrift.test.thrift_python.union_test.thrift_types import (
    DefaultFieldUnion as DefaultFieldUnionImmutable,
    TestStruct as TestStructImmutable,
    TestUnion as TestUnionImmutable,
    TestUnionAdaptedTypes as TestUnionAdaptedTypesImmutable,
    TestUnionAmbiguousFromValueBoolInt as TestUnionAmbiguousFromValueBoolIntImmutable,
    TestUnionAmbiguousFromValueFloatInt as TestUnionAmbiguousFromValueFloatIntImmutable,
    TestUnionAmbiguousFromValueIntBool as TestUnionAmbiguousFromValueIntBoolImmutable,
    TestUnionAmbiguousTypeFieldName as TestUnionAmbiguousTypeFieldNameImmutable,
    TestUnionAmbiguousValueFieldName as TestUnionAmbiguousValueFieldNameImmutable,
)


def _assert_serialization_round_trip(
    test: unittest.TestCase,
    serializer_module: types.ModuleType,
    thrift_object: typing.Union[MutableStructOrUnion, ImmutableStructOrUnion],
) -> None:
    for proto in serializer_module.Protocol:
        encoded = serializer_module.serialize(thrift_object, protocol=proto)
        test.assertIsInstance(encoded, bytes)

        decoded = serializer_module.deserialize(
            type(thrift_object), encoded, protocol=proto
        )
        test.assertIsInstance(decoded, type(thrift_object))
        test.assertEqual(thrift_object, decoded)


class ThriftPython_ImmutableUnion_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation(self) -> None:
        # Without any arguments, creates empty union (with value None).
        u = TestUnionImmutable()
        self.assertIs(
            u.fbthrift_current_field, TestUnionImmutable.FbThriftUnionFieldEnum.EMPTY
        )
        self.assertIs(u.type, TestUnionImmutable.Type.EMPTY)
        # Type is an alias for FbThriftUnionFieldEnum
        self.assertIs(u.fbthrift_current_field, TestUnionImmutable.Type.EMPTY)
        self.assertIsNone(u.value)
        with self.assertRaisesRegex(
            AttributeError, "Union contains a value of type EMPTY, not string_field"
        ):
            u.string_field
        _assert_serialization_round_trip(self, immutable_serializer, u)

        # Specifying exactly one keyword argument whose name corresponds to that of a
        # field for this Union, and a non-None value whose type is valid for that field,
        # should create a new instance with that field (and value) set.
        u2 = TestUnionImmutable(string_field="Hello, world!")
        self.assertIs(
            u2.fbthrift_current_field,
            TestUnionImmutable.FbThriftUnionFieldEnum.string_field,
        )
        self.assertIs(u2.type, TestUnionImmutable.Type.string_field)
        self.assertEqual(u2.value, "Hello, world!")
        self.assertEqual(u2.string_field, "Hello, world!")
        # Trying to access any other field should raise an error.
        with self.assertRaisesRegex(
            AttributeError, "Union contains a value of type string_field, not int_field"
        ):
            u2.int_field
        _assert_serialization_round_trip(self, immutable_serializer, u2)

        # Attempts to initialize an instance with a keyword argument whose name does
        # not match that of a field should raise an error.
        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\): unknown field "
                r"\(field_does_not_exist\)."
            ),
        ):
            # pyre-ignore[28]: Intentional for test
            TestUnionImmutable(field_does_not_exist=123)

        # Attempts to initialize an instance with more than one (valid, non-None)
        # keyword arguments raise an Error.
        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\) with more than one "
                r"keyword argument \(got non-None value for int_field, but already "
                r"had one for string_field\)."
            ),
        ):
            TestUnionImmutable(string_field="hello", int_field=42)

        # Attempts to initialize an instance with an invalid type raise an error.
        with self.assertRaisesRegex(
            TypeError,
            (
                "error initializing Thrift union with field 'int_field': value hello! "
                "is not a <class 'int'>, is actually of type <class 'str'>"
            ),
        ):
            # pyre-ignore[6]: Intentional for test
            TestUnionImmutable(int_field="hello!")

        self.assertEqual(TestUnionImmutable(int_field=None), TestUnionImmutable())
        self.assertEqual(
            TestUnionImmutable(string_field=None, int_field=None), TestUnionImmutable()
        )

        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\): unknown field "
                r"\(field_does_not_exist\)."
            ),
        ):
            # pyre-ignore[28]: Intentional for test
            TestUnionImmutable(field_does_not_exist=None)

        # Initialization with multiple keywords arguments, only one of which is not None
        # (and has a valid name and type).
        self.assertEqual(
            TestUnionImmutable(string_field=None, int_field=42),
            TestUnionImmutable(int_field=42),
        )
        self.assertEqual(
            TestUnionImmutable(int_field=42, string_field=None),
            TestUnionImmutable(int_field=42),
        )

        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\): unknown field "
                r"\(field_does_not_exist\)."
            ),
        ):
            # pyre-ignore[28]: Intentional for test
            TestUnionImmutable(string_field="hello", field_does_not_exist=None)

        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\): unknown field "
                r"\(field_does_not_exist\)."
            ),
        ):
            # pyre-ignore[28]: Intentional for test
            TestUnionImmutable(field_does_not_exist=None, string_field="hello")

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

        enum_names_and_values = {
            member.name: member.value
            for member in fields_enum_type.__members__.values()
        }

        self.assertEqual(
            enum_names_and_values,
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
        union_none = TestUnionImmutable.fromValue(None)
        self.assertEqual(union_none.type, union_none.Type.EMPTY)
        self.assertIsNone(union_none.value)

    def test_from_value_ambiguous_int_bool(self) -> None:
        union_int_bool_1 = TestUnionAmbiguousFromValueIntBoolImmutable.fromValue(1)
        self.assertIs(
            union_int_bool_1.fbthrift_current_field,
            TestUnionAmbiguousFromValueIntBoolImmutable.FbThriftUnionFieldEnum.int_field,
        )
        self.assertIs(
            union_int_bool_1.type,
            TestUnionAmbiguousFromValueIntBoolImmutable.Type.int_field,
        )
        self.assertEqual(union_int_bool_1.value, 1)
        self.assertEqual(union_int_bool_1.int_field, 1)
        _assert_serialization_round_trip(self, immutable_serializer, union_int_bool_1)

        # FIXED: fromValue(bool) used to populate int field ordered before bool field.
        union_int_bool_2 = TestUnionAmbiguousFromValueIntBoolImmutable.fromValue(True)
        self.assertIs(
            union_int_bool_2.fbthrift_current_field,
            TestUnionAmbiguousFromValueIntBoolImmutable.FbThriftUnionFieldEnum.bool_field,
        )
        self.assertIs(
            union_int_bool_2.type,
            TestUnionAmbiguousFromValueIntBoolImmutable.Type.bool_field,
        )
        self.assertEqual(union_int_bool_2.value, 1)
        self.assertEqual(union_int_bool_2.bool_field, True)
        _assert_serialization_round_trip(self, immutable_serializer, union_int_bool_2)

    def test_from_value_ambiguous_bool_int(self) -> None:
        # BAD: Unlike the previous test case, fromValue(int) does not populate
        # a bool field if seen first.
        union_bool_int_1 = TestUnionAmbiguousFromValueBoolIntImmutable.fromValue(1)
        self.assertIs(
            union_bool_int_1.fbthrift_current_field,
            TestUnionAmbiguousFromValueBoolIntImmutable.FbThriftUnionFieldEnum.int_field,
        )
        self.assertIs(
            union_bool_int_1.type,
            TestUnionAmbiguousFromValueBoolIntImmutable.Type.int_field,
        )
        self.assertEqual(union_bool_int_1.value, 1)
        self.assertEqual(union_bool_int_1.int_field, 1)
        self.assertEqual(union_bool_int_1.int_field, True)
        _assert_serialization_round_trip(self, immutable_serializer, union_bool_int_1)

        union_bool_int_2 = TestUnionAmbiguousFromValueBoolIntImmutable.fromValue(True)
        self.assertIs(
            union_bool_int_2.fbthrift_current_field,
            TestUnionAmbiguousFromValueBoolIntImmutable.FbThriftUnionFieldEnum.bool_field,
        )
        self.assertIs(
            union_bool_int_2.type,
            TestUnionAmbiguousFromValueBoolIntImmutable.Type.bool_field,
        )
        self.assertEqual(union_bool_int_2.value, True)
        self.assertEqual(union_bool_int_2.value, 1)
        self.assertEqual(union_bool_int_2.bool_field, 1)
        _assert_serialization_round_trip(self, immutable_serializer, union_bool_int_2)

    def test_from_value_ambiguous_float_int(self) -> None:
        # FIXED: fromValue(int) used to populate a float field ordered before int field
        union_float_int_1 = TestUnionAmbiguousFromValueFloatIntImmutable.fromValue(1)
        self.assertIs(
            union_float_int_1.fbthrift_current_field,
            TestUnionAmbiguousFromValueFloatIntImmutable.FbThriftUnionFieldEnum.int_field,
        )
        self.assertIs(
            union_float_int_1.type,
            TestUnionAmbiguousFromValueFloatIntImmutable.Type.int_field,
        )
        self.assertEqual(union_float_int_1.value, 1.0)
        self.assertEqual(union_float_int_1.int_field, 1)
        _assert_serialization_round_trip(self, immutable_serializer, union_float_int_1)

        union_float_int_2 = TestUnionAmbiguousFromValueFloatIntImmutable.fromValue(1.0)
        self.assertIs(
            union_float_int_2.fbthrift_current_field,
            TestUnionAmbiguousFromValueFloatIntImmutable.FbThriftUnionFieldEnum.float_field,
        )
        self.assertIs(
            union_float_int_2.type,
            TestUnionAmbiguousFromValueFloatIntImmutable.Type.float_field,
        )
        self.assertEqual(union_float_int_2.value, 1.0)
        self.assertEqual(union_float_int_2.float_field, 1)
        _assert_serialization_round_trip(self, immutable_serializer, union_float_int_2)

    def test_field_name_conflict(self) -> None:
        # By setting class type `Type` attr after field attrs, we get the desired behavior
        self.assertEqual(
            TestUnionAmbiguousTypeFieldNameImmutable().fbthrift_current_field,
            TestUnionAmbiguousTypeFieldNameImmutable().FbThriftUnionFieldEnum.EMPTY,
        )
        self.assertEqual(
            TestUnionAmbiguousTypeFieldNameImmutable().type,
            TestUnionAmbiguousTypeFieldNameImmutable().Type.EMPTY,
        )

        self.assertEqual(
            TestUnionAmbiguousTypeFieldNameImmutable(Type_=3).fbthrift_current_field,
            TestUnionAmbiguousTypeFieldNameImmutable.FbThriftUnionFieldEnum.Type_,
        )
        self.assertEqual(
            TestUnionAmbiguousTypeFieldNameImmutable(Type_=3).type,
            TestUnionAmbiguousTypeFieldNameImmutable.Type.Type_,
        )
        self.assertIsInstance(
            TestUnionAmbiguousTypeFieldNameImmutable.Type, enum.EnumMeta
        )
        type_union = TestUnionAmbiguousTypeFieldNameImmutable()
        with self.assertRaisesRegex(
            AttributeError, "object attribute 'Type' is read-only"
        ):
            # pyre-ignore[8, 41]: Intentional for test
            type_union.Type = 1
        _assert_serialization_round_trip(self, immutable_serializer, type_union)

        u = TestUnionAmbiguousValueFieldNameImmutable(value_=42)
        self.assertEqual(u.value_, 42)
        # this is now the catch-all accessor
        self.assertEqual(u.value, 42)
        with self.assertRaises(AttributeError):
            u.type_
        _assert_serialization_round_trip(self, immutable_serializer, u)

        u2 = TestUnionAmbiguousValueFieldNameImmutable(type_=123)
        with self.assertRaises(AttributeError):
            u2.value_
        _assert_serialization_round_trip(self, immutable_serializer, u2)

    def test_hash(self) -> None:
        hash(TestUnionImmutable())

    def test_equality(self) -> None:
        u1 = TestUnionImmutable(string_field="hello")
        u2 = TestUnionImmutable(string_field="hello")
        self.assertIsNot(u1, u2)
        self.assertEqual(u1, u2)
        _assert_serialization_round_trip(self, immutable_serializer, u1)
        _assert_serialization_round_trip(self, immutable_serializer, u2)

        u3 = TestUnionImmutable(string_field="world")
        self.assertIsNot(u1, u3)
        self.assertNotEqual(u1, u3)
        _assert_serialization_round_trip(self, immutable_serializer, u3)

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

    def test_adapted_types(self) -> None:
        u1 = TestUnionAdaptedTypesImmutable()
        self.assertIs(
            u1.fbthrift_current_field,
            TestUnionAdaptedTypesImmutable.FbThriftUnionFieldEnum.EMPTY,
        )
        self.assertIs(u1.type, TestUnionAdaptedTypesImmutable.Type.EMPTY)
        self.assertIsNone(u1.value)
        _assert_serialization_round_trip(self, immutable_serializer, u1)

        with self.assertRaisesRegex(
            AttributeError,
            (
                "error initializing Thrift union with field 'adapted_i32_to_datetime': "
                "'int' object has no attribute 'timestamp'"
            ),
        ):
            # pyre-ignore[6]: Intentional for test
            TestUnionAdaptedTypesImmutable(adapted_i32_to_datetime=123)

        u2 = TestUnionAdaptedTypesImmutable(
            adapted_i32_to_datetime=datetime.fromtimestamp(1718728839)
        )
        self.assertIs(
            u2.fbthrift_current_field,
            TestUnionAdaptedTypesImmutable.FbThriftUnionFieldEnum.adapted_i32_to_datetime,
        )
        self.assertIs(
            u2.type, TestUnionAdaptedTypesImmutable.Type.adapted_i32_to_datetime
        )

        # BAD: The following assertions demonstrate the fact that (immutable) Thrift
        # unions do not properly convert adapted fields on access. Indeed, u2.value
        # should be a datetime instance, but instead is the underlying (int) value.
        self.assertNotIsInstance(u2.value, datetime)
        self.assertNotEqual(u2.value, datetime.fromtimestamp(1718728839))
        self.assertIsInstance(u2.value, int)
        self.assertEqual(u2.value, 1718728839)
        self.assertIsNot(u2.value, u2.adapted_i32_to_datetime)

        # Note that the behavior above differs when the field is accessed directly
        # (instead of the auto-provided "value" attribute): the field value is then
        # adapted (i.e., datetime instead of the underlying int value).
        self.assertIsInstance(u2.adapted_i32_to_datetime, datetime)
        self.assertEqual(u2.adapted_i32_to_datetime, datetime.fromtimestamp(1718728839))
        self.assertNotIsInstance(u2.adapted_i32_to_datetime, int)
        self.assertNotEqual(u2.adapted_i32_to_datetime, 1718728839)

        # BAD: more "fromValue()" ambiguity: calling it with a datetime sets the
        # adapted field, but calling it with an integer (or string) raises an error
        # instead of setting one of the other fields.
        self.assertEqual(
            TestUnionAdaptedTypesImmutable.fromValue(
                # pyre-ignore[6]: Intentional for test
                datetime.fromtimestamp(1718728839)
            ),
            u2,
        )
        with self.assertRaisesRegex(
            AttributeError, "'int' object has no attribute 'timestamp'"
        ):
            TestUnionAdaptedTypesImmutable.fromValue(1718728839)
        with self.assertRaisesRegex(
            AttributeError, "'str' object has no attribute 'timestamp'"
        ):
            TestUnionAdaptedTypesImmutable.fromValue("1718728839")
        _assert_serialization_round_trip(self, immutable_serializer, u2)

        u3 = TestUnionAdaptedTypesImmutable(non_adapted_i32=1718728839)
        self.assertIs(
            u3.fbthrift_current_field,
            TestUnionAdaptedTypesImmutable.FbThriftUnionFieldEnum.non_adapted_i32,
        )
        self.assertIs(u3.type, TestUnionAdaptedTypesImmutable.Type.non_adapted_i32)
        self.assertIs(u3.value, u3.non_adapted_i32)
        self.assertEqual(u3.non_adapted_i32, 1718728839)
        _assert_serialization_round_trip(self, immutable_serializer, u3)

    def test_to_immutable_python(self) -> None:
        union_immutable = TestUnionImmutable(string_field="hello")
        self.assertIs(union_immutable, union_immutable._to_python())

    def test_to_mutable_python(self) -> None:
        union_immutable = TestUnionImmutable(string_field="hello")
        union_mutable = union_immutable._to_mutable_python()
        self.assertNotEqual(union_immutable, union_mutable)
        self.assertEqual(union_mutable, TestUnionMutable(string_field="hello"))

    def test_set_nonexistent_field(self) -> None:
        u = TestUnionImmutable(string_field="Hello!")
        with self.assertRaisesRegex(
            AttributeError, "object has no attribute 'non_existent_field'"
        ):
            # pyre-ignore[16]: Intentional for test
            u.non_existent_field = 999

    def test_union_field_enum_type_annotations(self) -> None:
        u = TestUnionImmutable(string_field="Hello!")
        u_fbthrift_current_field: TestUnionImmutable.FbThriftUnionFieldEnum = (
            u.fbthrift_current_field
        )
        self.assertEqual(
            u_fbthrift_current_field,
            TestUnionImmutable.FbThriftUnionFieldEnum.string_field,
        )
        u_type: TestUnionImmutable.Type = u.type

        u_value: typing.Union[None, TestStructImmutable, int, str] = u.value
        u_fbthrift_current_value: typing.Union[None, TestStructImmutable, int, str] = (
            u.fbthrift_current_value
        )

        self.assertEqual(u_type, TestUnionImmutable.Type.string_field)
        self.assertEqual(u_type, u_fbthrift_current_field)
        self.assertEqual(u_value, "Hello!")
        self.assertEqual(u_fbthrift_current_value, "Hello!")

    def test_match(self) -> None:
        # Canonical case: union with a field set matches Class pattern with
        # corresponding field keyword argument (but not a different keyword argument):
        match TestUnionImmutable(string_field="Hello!"):
            case TestUnionImmutable(int_field=int_field):
                self.fail(f"Unexpected match: {int_field}")
            case TestUnionImmutable(string_field=string_field):
                self.assertEqual(string_field, "Hello!")
            case _:
                self.fail("Expected match, got none.")

        # A non-empty union will match a Class pattern with no argument (the
        # specification explicitly says this checks isinstance() only).
        match TestUnionImmutable(string_field="Hello!"):
            case TestUnionImmutable():
                pass  # Expected
            case _:
                self.fail("Expected match, got none.")

        # Positional parameters in class pattern are not supported
        with self.assertRaisesRegex(
            TypeError, r"TestUnion\(\) accepts 0 positional sub-patterns \(1 given\)"
        ):
            match TestUnionImmutable(string_field="Hello!"):
                # pyre-ignore[16]: Intentional for test
                case TestUnionImmutable(string_field):
                    pass  # Should not be reached

        # Matching with multiple (field) keywords arguments ???
        match TestUnionImmutable(string_field="Hello!"):
            case TestUnionImmutable(string_field=string_field, int_field=int_field):
                self.fail(f"Unexpected match: {string_field}, {int_field}")
            case TestUnionImmutable(string_field=string_field, int_field=None):
                self.fail(f"Unexpected match: {string_field}")
            case _:
                pass  # Expected

        # Matching with special attributes
        match TestUnionImmutable(string_field="Hello!"):
            case TestUnionImmutable(
                fbthrift_current_field=x,
                fbthrift_current_value=y,
                string_field=string_field,
            ):
                self.assertEqual(
                    x, TestUnionImmutable.FbThriftUnionFieldEnum.string_field
                )
                self.assertEqual(y, "Hello!")
                self.assertEqual(string_field, "Hello!")
            case _:
                self.fail("Expected match, got none.")

        # Matching empty union (using fbthrift_current_value)
        match TestUnionImmutable():
            case TestUnionImmutable(fbthrift_current_value=None):
                pass
            case _:
                self.fail("Expected match, got none.")

        # Matching empty union (using fbthrift_current_field)
        match TestUnionImmutable():
            case TestUnionImmutable(
                fbthrift_current_field=TestUnionImmutable.FbThriftUnionFieldEnum.EMPTY
            ):
                pass
            case _:
                self.fail("Expected match, got none.")

        # Match nested struct data
        match TestUnionImmutable(
            struct_field=TestStructImmutable(unqualified_string="Hello!")
        ):
            case TestUnionImmutable(string_field=string_field):
                self.fail(f"Unexpected match: {string_field}")
            case TestUnionImmutable(fbthrift_current_value=None):
                self.fail("Unexpected match: (union is not empty)")
            case TestUnionImmutable(
                struct_field=TestStructImmutable(
                    unqualified_string=unqualified_string,
                    optional_string=optional_string,
                )
            ):
                self.assertEqual(unqualified_string, "Hello!")
                self.assertIsNone(optional_string)
            case _:
                self.fail("Expected match, got none.")

        # Match fields with adapted types
        match TestUnionAdaptedTypesImmutable(
            adapted_i32_to_datetime=datetime.fromtimestamp(1733507988)
        ):
            case TestUnionAdaptedTypesImmutable(adapted_i32_to_datetime=x):
                self.assertIsInstance(x, datetime)
                self.assertEqual(x, datetime.fromtimestamp(1733507988))
            case _:
                self.fail("Expected match, got none.")

        # Match adapted types via fbthrift_current_value returns the underlying value!
        # (see test_adapted_types() above).
        match TestUnionAdaptedTypesImmutable(
            adapted_i32_to_datetime=datetime.fromtimestamp(1733507988)
        ):
            case TestUnionAdaptedTypesImmutable(
                fbthrift_current_field=TestUnionAdaptedTypesImmutable.FbThriftUnionFieldEnum.adapted_i32_to_datetime,
                fbthrift_current_value=x,
            ):
                self.assertNotIsInstance(x, datetime)
                self.assertEqual(x, 1733507988)
            case _:
                self.fail("Expected match, got none.")

    def test_subclass(self) -> None:
        bases = (TestUnionImmutable,)
        err = (
            f"Inheritance from generated thrift union {bases[0].__name__} is deprecated."
            " Please use composition."
        )
        with self.assertRaisesRegex(TypeError, err):
            types.new_class(
                "TestImmutableSubclass",
                bases=bases,
                exec_body=lambda ns: ns.update(_fbthrift_SPEC=()),
            )

    def test_union_default(self) -> None:
        def assert_roundtrip(
            u: DefaultFieldUnionImmutable, which: DefaultFieldUnionImmutable.Type
        ) -> None:
            roundtrip = immutable_serializer.deserialize(
                DefaultFieldUnionImmutable, immutable_serializer.serialize(u)
            )
            self.assertEqual(u, roundtrip)
            self.assertEqual(roundtrip.type, which)

        assert_roundtrip(
            DefaultFieldUnionImmutable(), DefaultFieldUnionImmutable.Type.EMPTY
        )
        assert_roundtrip(
            DefaultFieldUnionImmutable(default_int=1),
            DefaultFieldUnionImmutable.Type.default_int,
        )
        assert_roundtrip(
            DefaultFieldUnionImmutable(useless_int_default=0),
            DefaultFieldUnionImmutable.Type.useless_int_default,
        )
        assert_roundtrip(
            DefaultFieldUnionImmutable(default_list=[1]),
            DefaultFieldUnionImmutable.Type.default_list,
        )
        assert_roundtrip(
            DefaultFieldUnionImmutable(useless_list_default=[1]),
            DefaultFieldUnionImmutable.Type.useless_list_default,
        )


class ThriftPython_MutableUnion_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation(self) -> None:
        # Without any arguments, creates empty union (with value None).
        u = TestUnionMutable()
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.EMPTY,
        )
        self.assertIsNone(u.fbthrift_current_value)
        with self.assertRaisesRegex(
            AttributeError,
            (
                r"Error retrieving Thrift union \(TestUnion\) field: requested "
                r"'string_field', but currently holds 'EMPTY'."
            ),
        ):
            u.string_field

        # Specifying exactly one keyword argument whose name corresponds to that of a
        # field for this Union, and a non-None value whose type is valid for that field,
        # should create a new instance with that field (and value) set.
        u2 = TestUnionMutable(string_field="Hello, world!")
        self.assertIs(
            u2.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.string_field,
        )
        self.assertEqual(u2.fbthrift_current_value, "Hello, world!")
        self.assertEqual(u2.string_field, "Hello, world!")
        # Trying to access any other field should raise an error.
        with self.assertRaisesRegex(
            AttributeError,
            (
                r"Error retrieving Thrift union \(TestUnion\) field: requested "
                "'int_field', but currently holds 'string_field'."
            ),
        ):
            u2.int_field

        # Attempts to initialize an instance with a keyword argument whose name does
        # not match that of a field should raise an error.
        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\): unknown field "
                r"\(field_does_not_exist\)."
            ),
        ):
            # pyre-ignore[28]: Intentional for test
            TestUnionMutable(field_does_not_exist=123)

        # Attempts to initialize an instance with more than one (valid, non-None)
        # keyword arguments raise an Error.
        with self.assertRaisesRegex(
            TypeError,
            (
                r"Cannot initialize Thrift union \(TestUnion\) with more than one "
                r"keyword argument \(got non-None value for int_field, but already "
                r"had one for string_field\)."
            ),
        ):
            TestUnionMutable(string_field="hello", int_field=42)

        # Attempts to initialize an instance with an invalid type raise an error.
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            TestUnionMutable(int_field="hello!")

        self.assertEqual(TestUnionMutable(int_field=None), TestUnionMutable())
        self.assertEqual(
            TestUnionMutable(string_field=None, int_field=None), TestUnionMutable()
        )

        with self.assertRaises(TypeError):
            # pyre-ignore[28]: Intentional for test
            TestUnionMutable(field_does_not_exist=None)

        # Initialization with multiple keywords arguments, only one of which is not None
        # (and has a valid name and type).
        u6 = TestUnionMutable(string_field=None, int_field=42)
        self.assertIs(
            u6.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.int_field,
        )
        self.assertEqual(u6.fbthrift_current_value, 42)
        self.assertEqual(u6.int_field, 42)

        u7 = TestUnionMutable(int_field=42, string_field=None)
        self.assertIs(
            u7.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.int_field,
        )
        self.assertEqual(u7.fbthrift_current_value, 42)
        self.assertEqual(u7.int_field, 42)

        with self.assertRaises(TypeError):
            # pyre-ignore[28]: Intentional for test
            TestUnionMutable(string_field="hello", field_does_not_exist=None)

        with self.assertRaises(TypeError):
            # pyre-ignore[28]: Intentional for test
            TestUnionMutable(field_does_not_exist=None, string_field="hello")

    def test_class_field_enum(self) -> None:
        # NOTE: in the immutable version, this attribute is using the
        # (unreserved) "Type" name.
        self.assertTrue(hasattr(TestUnionMutable, "FbThriftUnionFieldEnum"))
        fields_enum_type = TestUnionMutable.FbThriftUnionFieldEnum
        self.assertTrue(issubclass(fields_enum_type, enum.Enum))
        self.assertIsInstance(fields_enum_type, enum.EnumMeta)

        # Even though the name of the union class itself is not
        # the best name for the FbThriftUnionFieldEnum,
        # to ensure consistency between the immutable and mutable versions,
        # it must have that name:
        self.assertEqual(fields_enum_type.__name__, "TestUnion")

        # NOTE: "empty" value for mutable is different from immutable.
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

        enum_names_and_values = {
            member.name: member.value
            for member in fields_enum_type.__members__.values()
        }

        self.assertEqual(
            enum_names_and_values,
            {
                "EMPTY": 0,
                "string_field": 1,
                "int_field": 2,
                "struct_field": 3,
            },
        )

    def test_adapted_types(self) -> None:
        u1 = TestUnionAdaptedTypesMutable()
        self.assertIs(
            u1.fbthrift_current_field,
            TestUnionAdaptedTypesMutable.FbThriftUnionFieldEnum.EMPTY,
        )
        self.assertIsNone(u1.fbthrift_current_value)

        with self.assertRaisesRegex(
            AttributeError, "'int' object has no attribute 'timestamp'"
        ):
            # pyre-ignore[6]: Intentional for test
            TestUnionAdaptedTypesMutable(adapted_i32_to_datetime=123)

        u2 = TestUnionAdaptedTypesMutable(
            adapted_i32_to_datetime=datetime.fromtimestamp(1718728839)
        )
        self.assertIs(
            u2.fbthrift_current_field,
            TestUnionAdaptedTypesMutable.FbThriftUnionFieldEnum.adapted_i32_to_datetime,
        )

        # BAD: The following assertions demonstrate the fact that (immutable) Thrift
        # unions do not properly convert adapted fields on access. Indeed, u2.fbthrift_current_value
        # should be a datetime instance, but instead is the underlying (int) value.
        self.assertNotIsInstance(u2.fbthrift_current_value, datetime)
        self.assertNotEqual(
            u2.fbthrift_current_value, datetime.fromtimestamp(1718728839)
        )
        self.assertIsInstance(u2.fbthrift_current_value, int)
        self.assertEqual(u2.fbthrift_current_value, 1718728839)
        self.assertIsNot(u2.fbthrift_current_value, u2.adapted_i32_to_datetime)

        # Note that the behavior above differs when the field is accessed directly
        # (instead of the auto-provided "value" attribute): the field value is then
        # adapted (i.e., datetime instead of the underlying int value).
        self.assertIsInstance(u2.adapted_i32_to_datetime, datetime)
        self.assertEqual(u2.adapted_i32_to_datetime, datetime.fromtimestamp(1718728839))
        self.assertNotIsInstance(u2.adapted_i32_to_datetime, int)
        self.assertNotEqual(u2.adapted_i32_to_datetime, 1718728839)

    def test_adapted_types_assignment(self) -> None:
        # TEST: Assigning a (adapted) field takes in an adapted type and sets the field
        # correctly.
        u1 = TestUnionAdaptedTypesMutable()
        u1.adapted_i32_to_datetime = datetime.fromtimestamp(1718728839)
        self.assertIs(
            u1.fbthrift_current_field,
            TestUnionAdaptedTypesMutable.FbThriftUnionFieldEnum.adapted_i32_to_datetime,
        )
        self.assertEqual(u1.adapted_i32_to_datetime, datetime.fromtimestamp(1718728839))
        # BAD: Note that, as described above, the fbthrift_current_value exposes the
        # underlying (non-adapted) Thrift type, as opposed to the adapted type..
        self.assertEqual(u1.fbthrift_current_value, 1718728839)

        # TEST: We can assign to a different field (adapted or not):
        # At first, trying to access unset field raises Error:
        with self.assertRaisesRegex(
            AttributeError,
            (
                r"Error retrieving Thrift union \(TestUnionAdaptedTypes\) field: "
                "requested 'non_adapted_i32', but currently holds "
                "'adapted_i32_to_datetime'"
            ),
        ):
            _ = u1.non_adapted_i32
        # Setting an (unadapted) field makes it available:
        u1.non_adapted_i32 = 42
        self.assertEqual(u1.non_adapted_i32, 42)
        self.assertIs(
            u1.fbthrift_current_field,
            TestUnionAdaptedTypesMutable.FbThriftUnionFieldEnum.non_adapted_i32,
        )
        self.assertEqual(u1.fbthrift_current_value, 42)
        # Setting another adapted field:
        u1.adapted_string_to_i32 = 123
        self.assertEqual(u1.adapted_string_to_i32, 123)
        self.assertIs(
            u1.fbthrift_current_field,
            TestUnionAdaptedTypesMutable.FbThriftUnionFieldEnum.adapted_string_to_i32,
        )
        self.assertEqual(u1.fbthrift_current_value, "123")

    def test_set_field(self) -> None:
        u = TestUnionMutable()
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.EMPTY,
        )
        self.assertIsNone(u.fbthrift_current_value)

        u.string_field = "Hello, world!"
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.string_field,
        )
        self.assertEqual(u.fbthrift_current_value, "Hello, world!")
        self.assertIs(u.fbthrift_current_value, u.string_field)
        with self.assertRaisesRegex(
            AttributeError,
            (
                r"Error retrieving Thrift union \(TestUnion\) field: requested "
                r"'int_field', but currently holds 'string_field'."
            ),
        ):
            u.int_field

        u.int_field = 42
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.int_field,
        )
        self.assertEqual(u.fbthrift_current_value, 42)
        self.assertIs(u.fbthrift_current_value, u.int_field)

        u.int_field = 43
        self.assertEqual(u.int_field, 43)

    def test_set_nonexistent_field(self) -> None:
        u = TestUnionMutable(string_field="Hello!")
        with self.assertRaisesRegex(
            AttributeError, "object has no attribute 'non_existent_field'"
        ):
            # pyre-ignore[16]: Intentional for test
            u.non_existent_field = 999

    def test_reset(self) -> None:
        u = TestUnionMutable(string_field="Hello!")
        self.assertEqual(u.fbthrift_current_value, "Hello!")
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.string_field,
        )

        # Resetting union makes it empty
        u.fbthrift_reset()
        self.assertIsNone(u.fbthrift_current_value)
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.EMPTY,
        )

        # Union can be reset again, it's a no-op
        u.fbthrift_reset()
        self.assertIsNone(u.fbthrift_current_value)

        # Union can then be set:
        u.int_field = 42
        self.assertEqual(u.fbthrift_current_value, 42)
        self.assertIs(
            u.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.int_field,
        )

        # One last reset
        u.fbthrift_reset()
        self.assertIsNone(u.fbthrift_current_value)

    def test_del_field(self) -> None:
        u = TestUnionMutable(string_field="Hello!")
        self.assertEqual(u.string_field, "Hello!")
        with self.assertRaisesRegex(NotImplementedError, "__delete__"):
            del u.string_field

    def test_hash(self) -> None:
        with self.assertRaisesRegex(TypeError, "unhashable type: 'TestUnion'"):
            hash(TestUnionMutable())

    def test_equality(self) -> None:
        u1 = TestUnionMutable(string_field="hello")
        u2 = TestUnionMutable(string_field="hello")
        self.assertIsNot(u1, u2)
        self.assertEqual(u1, u2)

        u3 = TestUnionMutable(string_field="world")
        self.assertIsNot(u1, u3)
        self.assertNotEqual(u1, u3)
        u3.string_field = "hello"
        self.assertIsNot(u1, u3)
        self.assertEqual(u1, u3)

    def test_ordering(self) -> None:
        lhs = TestUnionMutable(string_field="hello")
        rhs = TestUnionMutable(string_field="world")

        self.assertLess(lhs, rhs)
        self.assertLessEqual(lhs, rhs)

        lhs.int_field = 42
        self.assertGreater(lhs, rhs)
        self.assertGreaterEqual(lhs, rhs)

        rhs.int_field = 43
        self.assertLess(lhs, rhs)
        self.assertLessEqual(lhs, rhs)

        rhs.int_field = 42
        self.assertLessEqual(lhs, rhs)
        self.assertGreaterEqual(lhs, rhs)

        # TEST: Empty union is smaller than all others, except another empty union
        self.assertLess(TestUnionMutable(), TestUnionMutable(string_field=""))
        self.assertLessEqual(TestUnionMutable(), TestUnionMutable())
        self.assertGreaterEqual(TestUnionMutable(), TestUnionMutable())

    def test_to_immutable_python(self) -> None:
        union_mutable = TestUnionMutable(string_field="hello")
        union_immutable = union_mutable._to_python()

        # TEST: The immutable instance is not the same, but has the same contents:
        self.assertIsNot(union_mutable, union_immutable)
        self.assertEqual(union_immutable, TestUnionImmutable(string_field="hello"))

        # TEST: changing the mutable instance does not affect the immutable one
        union_mutable.int_field = 42
        self.assertEqual(union_immutable, TestUnionImmutable(string_field="hello"))

        self.assertEqual(
            hash(union_immutable), hash(TestUnionImmutable(string_field="hello"))
        )

    def test_to_mutable_python(self) -> None:
        union_mutable = TestUnionMutable(string_field="hello")
        self.assertIs(union_mutable, union_mutable._to_mutable_python())

    def test_serialize(self) -> None:
        mutable_serializer.serialize(TestUnionMutable())

        with self.assertRaisesRegex(Exception, "underflow"):
            mutable_serializer.deserialize(TestUnionMutable, b"")

    def test_union_field_enum_type_annotations(self) -> None:
        u = TestUnionMutable(string_field="Hello!")
        u_fbthrift_current_field: TestUnionMutable.FbThriftUnionFieldEnum = (
            u.fbthrift_current_field
        )
        self.assertEqual(
            u_fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.string_field,
        )

        u_fbthrift_current_value: typing.Union[None, TestStructMutable, int, str] = (
            u.fbthrift_current_value
        )

        self.assertEqual(u_fbthrift_current_value, "Hello!")

    def test_match(self) -> None:
        # Canonical case: union with a field set matches Class pattern with
        # corresponding field keyword argument (but not a different keyword argument):
        match TestUnionMutable(string_field="Hello!"):
            case TestUnionMutable(int_field=int_field):
                self.fail(f"Unexpected match: {int_field}")
            case TestUnionMutable(string_field=string_field):
                self.assertEqual(string_field, "Hello!")
            case _:
                self.fail("Expected match, got none.")

        # A non-empty union will match a Class pattern with no argument (the
        # specification explicitly says this checks isinstance() only).
        match TestUnionMutable(string_field="Hello!"):
            case TestUnionMutable():
                pass  # Expected
            case _:
                self.fail("Expected match, got none.")

        # Positional parameters in class pattern are not supported
        with self.assertRaisesRegex(
            TypeError, r"TestUnion\(\) accepts 0 positional sub-patterns \(1 given\)"
        ):
            match TestUnionMutable(string_field="Hello!"):
                # pyre-ignore[16]: Intentional for test
                case TestUnionMutable(string_field):
                    pass  # Should not be reached

        # Matching with multiple (field) keywords arguments ???
        match TestUnionMutable(string_field="Hello!"):
            case TestUnionMutable(string_field=string_field, int_field=int_field):
                self.fail(f"Unexpected match: {string_field}, {int_field}")
            case TestUnionMutable(string_field=string_field, int_field=None):
                self.fail(f"Unexpected match: {string_field}")
            case _:
                pass  # Expected

        # Matching with special attributes
        match TestUnionMutable(string_field="Hello!"):
            case TestUnionMutable(
                fbthrift_current_field=x,
                fbthrift_current_value=y,
                string_field=string_field,
            ):
                self.assertEqual(
                    x, TestUnionMutable.FbThriftUnionFieldEnum.string_field
                )
                self.assertEqual(y, "Hello!")
                self.assertEqual(string_field, "Hello!")
            case _:
                self.fail("Expected match, got none.")

        # Matching empty union (using fbthrift_current_value)
        match TestUnionMutable():
            case TestUnionMutable(fbthrift_current_value=None):
                pass
            case _:
                self.fail("Expected match, got none.")

        # Matching empty union (using fbthrift_current_field)
        match TestUnionMutable():
            case TestUnionMutable(
                fbthrift_current_field=TestUnionMutable.FbThriftUnionFieldEnum.EMPTY
            ):
                pass
            case _:
                self.fail("Expected match, got none.")

        # Match nested struct data
        match TestUnionMutable(
            struct_field=TestStructMutable(unqualified_string="Hello!")
        ):
            case TestUnionMutable(string_field=string_field):
                self.fail(f"Unexpected match: {string_field}")
            case TestUnionMutable(fbthrift_current_value=None):
                self.fail("Unexpected match: (union is not empty)")
            case TestUnionMutable(
                struct_field=TestStructMutable(
                    unqualified_string=unqualified_string,
                    optional_string=optional_string,
                )
            ):
                self.assertEqual(unqualified_string, "Hello!")
                self.assertIsNone(optional_string)
            case _:
                self.fail("Expected match, got none.")

        # Match fields with adapted types
        match TestUnionAdaptedTypesMutable(
            adapted_i32_to_datetime=datetime.fromtimestamp(1733507988)
        ):
            case TestUnionAdaptedTypesMutable(adapted_i32_to_datetime=x):
                self.assertIsInstance(x, datetime)
                self.assertEqual(x, datetime.fromtimestamp(1733507988))
            case _:
                self.fail("Expected match, got none.")

        # Match adapted types via fbthrift_current_value returns the underlying value!
        # (see test_adapted_types() above).
        match TestUnionAdaptedTypesMutable(
            adapted_i32_to_datetime=datetime.fromtimestamp(1733507988)
        ):
            case TestUnionAdaptedTypesMutable(
                fbthrift_current_field=TestUnionAdaptedTypesMutable.FbThriftUnionFieldEnum.adapted_i32_to_datetime,
                fbthrift_current_value=x,
            ):
                self.assertNotIsInstance(x, datetime)
                self.assertEqual(x, 1733507988)
            case _:
                self.fail("Expected match, got none.")

        # Cases above are similar to immutable union. Checking with mutations next...

        # Similar to canonical case above, but field is changed after initialization.
        u = TestUnionMutable(string_field="Hello!")
        u.int_field = 42
        match u:
            case TestUnionMutable(string_field=string_field):
                self.fail(f"Unexpected match: {string_field}")
            case TestUnionMutable(int_field=int_field):
                self.assertEqual(int_field, 42)
            case _:
                self.fail("Expected match, got none.")

        # Match after resetting
        u.fbthrift_reset()
        match u:
            case TestUnionMutable(fbthrift_current_value=None):
                pass  # Expected
            case _:
                self.fail("Expected match, got none.")

        # Re-assign different field
        u.string_field = "world!"
        match u:
            case TestUnionMutable(fbthrift_current_value=None):
                self.fail("Unexpected match")
            case TestUnionMutable(string_field=string_field):
                self.assertEqual(string_field, "world!")
            case _:
                self.fail("Expected match, got none.")

    def test_union_default(self) -> None:
        def assert_roundtrip(
            u: DefaultFieldUnionMutable,
            which: DefaultFieldUnionMutable.FbThriftUnionFieldEnum,
        ) -> None:
            roundtrip = mutable_serializer.deserialize(
                DefaultFieldUnionMutable, mutable_serializer.serialize(u)
            )
            self.assertEqual(u, roundtrip)
            self.assertEqual(roundtrip.fbthrift_current_field, which)

        assert_roundtrip(
            DefaultFieldUnionMutable(),
            DefaultFieldUnionMutable.FbThriftUnionFieldEnum.EMPTY,
        )
        assert_roundtrip(
            DefaultFieldUnionMutable(default_int=1),
            DefaultFieldUnionMutable.FbThriftUnionFieldEnum.default_int,
        )
        assert_roundtrip(
            DefaultFieldUnionMutable(useless_int_default=0),
            DefaultFieldUnionMutable.FbThriftUnionFieldEnum.useless_int_default,
        )
        assert_roundtrip(
            DefaultFieldUnionMutable(default_list=to_thrift_list([1])),
            DefaultFieldUnionMutable.FbThriftUnionFieldEnum.default_list,
        )
        assert_roundtrip(
            DefaultFieldUnionMutable(useless_list_default=to_thrift_list([1])),
            DefaultFieldUnionMutable.FbThriftUnionFieldEnum.useless_list_default,
        )
