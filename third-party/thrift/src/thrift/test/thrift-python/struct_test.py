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

import dataclasses
import importlib
import types
import typing
import unittest
from datetime import datetime

import thrift.python.mutable_serializer as mutable_serializer

import thrift.python.serializer as immutable_serializer

from parameterized import parameterized
from thrift.python.exceptions import Error
from thrift.python.mutable_containers import MutableList, MutableMap, MutableSet

from thrift.python.mutable_exceptions import MutableGeneratedError
from thrift.python.mutable_types import (
    MutableStruct,
    MutableStructMeta,
    MutableStructOrUnion,
)
from thrift.python.types import (
    Struct as ImmutableStruct,
    StructMeta as ImmutableStructMeta,
    StructOrUnion as ImmutableStructOrUnion,
)

from thrift.test.thrift_python.struct_test.thrift_mutable_types import (  # @manual=//thrift/test/thrift-python:struct_test_thrift-python-types
    bool_constant,
    byte_constant,
    double_constant,
    float_constant,
    i16_constant,
    i32_constant,
    i64_constant,
    list_constant,
    map_constant,
    set_constant,
    string_constant,
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesMutable,
    TestStruct as TestStructMutable,
    TestStructAdaptedTypes as MutableTestStructAdaptedTypes,
    TestStructAllThriftContainerTypes as MutableTestStructAllThriftContainerTypes,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesMutable,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable,
    TestStructEmpty as TestStructEmptyMutable,
    TestStructEmptyAlias as TestStructEmptyAliasMutable,
    TestStructNested_0 as TestStructNested_0_Mutable,
    TestStructNested_1 as TestStructNested_1_Mutable,
    TestStructNested_2 as TestStructNested_2_Mutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesMutable,
    TestStructWithExceptionField as TestStructWithExceptionFieldMutable,
    TestStructWithTypedefField as TestStructWithTypedefFieldMutable,
    TestStructWithUnionField as TestStructWithUnionFieldMutable,
)

from thrift.test.thrift_python.struct_test.thrift_types import (
    TestStruct as TestStructImmutable,
    TestStructAdaptedTypes as ImmutableTestStructAdaptedTypes,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesImmutable,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesImmutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesImmutable,
)

max_byte = 2**7 - 1
max_i16 = 2**15 - 1
max_i32 = 2**31 - 1
max_i64 = 2**63 - 1


def _thrift_serialization_round_trip(
    test, module, control: typing.Union[MutableStructOrUnion, ImmutableStructOrUnion]
) -> None:
    for proto in module.Protocol:
        encoded = module.serialize(control, protocol=proto)
        test.assertIsInstance(encoded, bytes)

        decoded = module.deserialize(type(control), encoded, protocol=proto)
        test.assertIsInstance(decoded, type(control))
        test.assertEqual(control, decoded)


class ThriftPython_ImmutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation(self) -> None:
        # Field initialization at instantiation time
        w_new = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new.unqualified_string, "hello, world!")

    def test_creation_with_None(self) -> None:
        self.assertEqual(
            TestStructImmutable(),
            TestStructImmutable(unqualified_string=None, optional_string=None),
        )

        self.assertEqual(
            TestStructImmutable(unqualified_string="hello, world!"),
            TestStructImmutable(
                unqualified_string="hello, world!", optional_string=None
            ),
        )

    def test_call(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")

        # Call operator: create a copy, with new values.
        # Original instance is unchanged.
        w2 = w(unqualified_string="foobar")
        self.assertIsNot(w, w2)
        self.assertEqual(w2.unqualified_string, "foobar")
        self.assertEqual(w.unqualified_string, "hello, world!")
        self.assertNotEqual(w, w2)

        # Call operator with no values given: returns self
        # Note: this is equivalent to returning a copy of self in the immutable
        # domain (since their contents cannot change).
        w3 = w()
        self.assertIs(w, w3)

        # Call operator with invalid type
        with self.assertRaisesRegex(
            TypeError,
            (
                "error updating Thrift struct field 'unqualified_string': Cannot create "
                "internal string data representation. Expected type <class 'str'>, got: "
                "<class 'int'>."
            ),
        ):
            w(unqualified_string=42)

    def test_call_with_None(self) -> None:
        w = TestStructImmutable(unqualified_string="hello", optional_string="world")

        # TEST: Updating a value with "None" means resetting to its default value.
        self.assertEqual(
            w(unqualified_string=None),
            TestStructImmutable(unqualified_string="", optional_string="world"),
        )
        w1 = w(optional_string=None)
        self.assertIsNone(w1.optional_string)
        self.assertEqual(w1, TestStructImmutable(unqualified_string="hello"))
        self.assertEqual(
            w1, TestStructImmutable(unqualified_string="hello", optional_string=None)
        )

        w2 = w(unqualified_string=None, optional_string=None)
        self.assertEqual(w2, TestStructImmutable())
        self.assertIsNone(w2.optional_string)

    def test_default_values(self) -> None:
        # Custom default values:
        # Newly created instance has custom default values for non-optional
        # fields, but custom default values for optional fields are ignored.
        self.assertEqual(
            TestStructWithDefaultValuesImmutable(),
            TestStructWithDefaultValuesImmutable(
                unqualified_integer=42,
                optional_integer=None,
                unqualified_struct=TestStructImmutable(unqualified_string="hello"),
                optional_struct=None,
            ),
        )

        # Intrinsic default values:
        # optional struct field is None
        self.assertIsNone(
            TestStructWithDefaultValuesImmutable().optional_struct_intrinsic_default
        )

        # unqualified struct field is default-initialized
        self.assertEqual(
            TestStructImmutable(),
            TestStructWithDefaultValuesImmutable().unqualified_struct_intrinsic_default,
        )

    def test_type_safety(self) -> None:
        # Field type is validated on instantiation
        with self.assertRaisesRegex(
            TypeError,
            (
                "error initializing Thrift struct field 'unqualified_string': Cannot "
                "create internal string data representation. Expected type <class 'str'>, "
                "got: <class 'int'>"
            ),
        ):
            TestStructImmutable(unqualified_string=42)

    def test_equality_and_hashability(self) -> None:
        # Equality
        w_new = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new, w_new)
        w_new2 = TestStructImmutable(unqualified_string="hello, world!")
        self.assertEqual(w_new, w_new2)
        self.assertIsNot(w_new, w_new2)

        # Immutable types are hashable, with proper semantics.
        self.assertEqual(hash(w_new), hash(w_new2))
        self.assertIn(w_new, {w_new2})

        mapping = {w_new: 123}
        self.assertIn(w_new, mapping)
        self.assertIn(w_new2, mapping)
        self.assertEqual(mapping[w_new], 123)
        self.assertEqual(mapping[w_new2], 123)

        mapping[w_new2] = 456
        self.assertEqual(mapping[w_new], 456)
        self.assertEqual(mapping[w_new2], 456)

    def test_ordering(self) -> None:
        self.assertLess(
            TestStructImmutable(unqualified_string="a"),
            TestStructImmutable(unqualified_string="b"),
        )
        self.assertLess(
            TestStructImmutable(unqualified_string="a", optional_string="z"),
            TestStructImmutable(unqualified_string="b", optional_string="a"),
        )
        self.assertGreater(
            TestStructImmutable(unqualified_string="b", optional_string="z"),
            TestStructImmutable(unqualified_string="b", optional_string="a"),
        )

    def test_subclass(self) -> None:
        types.new_class(
            "TestImmutableSubclass",
            bases=(TestStructImmutable,),
            exec_body=lambda ns: ns.update(_fbthrift_SPEC=()),
        )

    def test_base_classes(self) -> None:
        self.assertIsInstance(TestStructImmutable(), ImmutableStruct)
        self.assertIsInstance(TestStructImmutable(), ImmutableStructOrUnion)

    def test_type(self) -> None:
        self.assertEqual(type(TestStructImmutable), ImmutableStructMeta)
        self.assertEqual(type(TestStructImmutable()), TestStructImmutable)

    def test_iteration(self) -> None:
        # Iterating over the class yields tuples of (field_name, None).
        self.assertSetEqual(
            set(TestStructImmutable),
            {("unqualified_string", None), ("optional_string", None)},
        )

        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            set(TestStructImmutable(unqualified_string="hello")),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )

    def test_del_attribute(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")

        # Attributes of immutable types cannot be deleted.
        #
        # Note the interesting (and somewhat inconsistent) current behavior:
        # Calling `del` prior to accessing an attribute raises an AttributeError
        # (at the cinder level), but doing so after accessing it is a silent
        # no-op.
        with self.assertRaisesRegex(AttributeError, "unqualified_string"):
            del w.unqualified_string
        self.assertEqual(w.unqualified_string, "hello, world!")
        del w.unqualified_string  # silent no-op
        self.assertEqual(w.unqualified_string, "hello, world!")

        # However, a new instance of the object can be created with a specific
        # attribute "deleted", by explicitly assigning it the "None" value:
        w_cleared = w(unqualified_string=None)
        self.assertEqual(w_cleared.unqualified_string, "")

        # "Deleting" a field (by creating a new instance) resets it to its
        # *standard default value*, i.e. the custom default value if provided
        # (see thrift/doc/idl/index.md#default-values).
        w_default_values = TestStructWithDefaultValuesImmutable(unqualified_integer=123)
        self.assertEqual(w_default_values.unqualified_integer, 123)
        w_default_values_cleared = w_default_values(unqualified_integer=None)
        self.assertEqual(w_default_values_cleared.unqualified_integer, 42)

    def test_type_hints(self) -> None:
        # thrift-python immutable structs do not include type hints directly
        # (although a separate .pyi interface is generated to allow tooling to
        # do static type checking)
        self.assertEqual(typing.get_type_hints(TestStructImmutable), {})

    def test_serialization_round_trip(self) -> None:
        s = TestStructAllThriftPrimitiveTypesImmutable(
            unqualified_string="Hello world!",
            optional_string="Hello optional!",
            unqualified_i32=19,
            optional_i32=23,
            unqualified_double=2.1,
            optional_double=1.3,
            unqualified_bool=True,
            optional_bool=False,
        )
        _thrift_serialization_round_trip(self, immutable_serializer, s)

        s_default_value = TestStructAllThriftPrimitiveTypesWithDefaultValuesImmutable()
        _thrift_serialization_round_trip(self, immutable_serializer, s_default_value)

    def test_adapted_types(self) -> None:
        s = ImmutableTestStructAdaptedTypes()
        # standard default value for i32 is 0, therefore `fromtimestamp(0)`
        self.assertEqual(
            s.unqualified_adapted_i32_to_datetime, datetime.fromtimestamp(0)
        )
        self.assertEqual(s.unqualified_adapted_string_to_i32, 123)

        new_date = datetime(2024, 5, 6, 12, 0, 0)
        s = s(unqualified_adapted_i32_to_datetime=new_date)
        self.assertEqual(s.unqualified_adapted_i32_to_datetime, new_date)

        with self.assertRaisesRegex(
            AttributeError,
            (
                "error updating Thrift struct field "
                "'unqualified_adapted_i32_to_datetime': 'int' object has no attribute "
                "'timestamp'"
            ),
        ):
            # Thrift simply passes the value to the adapter class. All type
            # checking is performed within the adapter class.
            s = s(unqualified_adapted_i32_to_datetime=123)

        s = s(unqualified_adapted_string_to_i32=999)
        self.assertEqual(s.unqualified_adapted_string_to_i32, 999)

    @parameterized.expand(
        [
            (
                ImmutableTestStructAdaptedTypes(
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400)
                ),
            ),
            (
                ImmutableTestStructAdaptedTypes(
                    unqualified_adapted_i32_to_datetime=datetime.fromtimestamp(0),
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400),
                ),
            ),
        ]
    )
    def test_adapter_serialization_round_trip(self, struct) -> None:
        _thrift_serialization_round_trip(self, immutable_serializer, struct)

    def test_to_immutable_python(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")
        self.assertIs(w, w._to_python())

    def test_to_mutable_python(self) -> None:
        w = TestStructImmutable(unqualified_string="hello, world!")
        with self.assertRaisesRegex(
            AttributeError, "'TestStruct' object has no attribute '_to_mutable_python'"
        ):
            w._to_mutable_python()


class ThriftPython_MutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_creation_and_assignment(self) -> None:
        w = TestStructMutable()
        self.assertEqual(w.unqualified_string, "")
        w.unqualified_string = "hello, world!"
        self.assertEqual(w.unqualified_string, "hello, world!")

        w2 = TestStructMutable(unqualified_string="hello")
        self.assertEqual(w2.unqualified_string, "hello")
        w2.unqualified_string += ", world!"
        self.assertEqual(w2.unqualified_string, "hello, world!")

    def test_creation_with_None(self) -> None:
        # DO_BEFORE(aristidis,20240811)): Note the inconsistency between the (current)
        # mutable implementation and the immutable version: specifying 'None' for an
        # unqualified field is an error for the former, but not the latter. Figure out
        # the desired behavior and update accordingly.
        with self.assertRaisesRegex(
            TypeError,
            (
                "error setting Thrift struct field 'unqualified_string': Cannot create "
                "internal string data representation. Expected type <class 'str'>, "
                "got: <class 'NoneType'>"
            ),
        ):
            self.assertEqual(
                TestStructMutable(),
                TestStructMutable(unqualified_string=None, optional_string=None),
            )

        # DO_BEFORE(aristidis,20240812)): Specifying None for an optional field should
        # not raise an error.
        with self.assertRaisesRegex(
            TypeError,
            (
                "error setting Thrift struct field 'optional_string': Cannot create "
                "internal string data representation. Expected type <class 'str'>, "
                "got: <class 'NoneType'>."
            ),
        ):
            self.assertEqual(
                TestStructMutable(unqualified_string="hello, world!"),
                TestStructMutable(
                    unqualified_string="hello, world!", optional_string=None
                ),
            )

    def test_call(self) -> None:
        # DO_BEFORE(aristidis,20240520): Support call operator for mutable types
        with self.assertRaises(TypeError):
            w = TestStructMutable(unqualified_string="hello, world!")

            # Call operator: create a (deep) copy, with new values.
            # Original instance is unchanged.
            w2 = w(unqualified_string="foobar")
            self.assertIsNot(w, w2)
            self.assertEqual(w2.unqualified_string, "foobar")
            self.assertEqual(w.unqualified_string, "hello, world!")
            self.assertNotEqual(w, w2)

            # Call operator with no values given: returns (deep) copy of self
            # Note the difference with immutable types (which return self).
            w3 = w()
            self.assertIsNot(w, w3)
            self.assertEqual(w, w3)

    def test_default_values(self) -> None:
        # Intrinsic default values:
        # optional struct field is None
        self.assertIsNone(
            TestStructWithDefaultValuesMutable().optional_struct_intrinsic_default
        )

        self.assertEqual(
            TestStructMutable(),
            TestStructWithDefaultValuesMutable().unqualified_struct_intrinsic_default,
        )

    def test_equality_and_hashability(self) -> None:
        # Equality
        w_mutable = TestStructMutable(unqualified_string="hello, world!")
        self.assertEqual(w_mutable, w_mutable)

        w_mutable2 = TestStructMutable(unqualified_string="hello, world!")
        self.assertEqual(w_mutable.unqualified_string, w_mutable2.unqualified_string)
        self.assertEqual(w_mutable, w_mutable2)

        # Instances are not hashable
        with self.assertRaisesRegex(TypeError, "unhashable type: 'TestStruct'"):
            hash(w_mutable)

        with self.assertRaisesRegex(TypeError, "unhashable type: 'TestStruct'"):
            {w_mutable}

        # List and Tuple membership tests use equality (not hashing).
        self.assertIn(w_mutable, [w_mutable2])
        self.assertIn(w_mutable, (w_mutable2,))

        w_mutable2.unqualified_string = "changed value"
        self.assertNotIn(w_mutable, [w_mutable2])
        self.assertNotIn(w_mutable, (w_mutable2,))

    def test_ordering(self) -> None:
        self.assertLess(
            TestStructMutable(unqualified_string="a"),
            TestStructMutable(unqualified_string="b"),
        )
        self.assertLess(
            TestStructMutable(unqualified_string="a", optional_string="z"),
            TestStructMutable(unqualified_string="b", optional_string="a"),
        )
        self.assertGreater(
            TestStructMutable(unqualified_string="b", optional_string="z"),
            TestStructMutable(unqualified_string="b", optional_string="a"),
        )

    def test_subclass(self) -> None:
        with self.assertRaisesRegex(
            TypeError, "Inheriting from thrift-python data types is forbidden:"
        ):
            types.new_class("TestSubclass2", bases=(TestStructMutable,))

    def test_base_classes(self) -> None:
        self.assertIsInstance(TestStructMutable(), MutableStruct)
        self.assertIsInstance(TestStructMutable(), MutableStructOrUnion)

    def test_type(self) -> None:
        self.assertEqual(type(TestStructMutable), MutableStructMeta)
        self.assertEqual(type(TestStructMutable()), TestStructMutable)

    def test_iteration(self) -> None:
        # Iterating over the class yields tuples of (field_name, None).
        self.assertSetEqual(
            set(TestStructMutable),
            {("unqualified_string", None), ("optional_string", None)},
        )

        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            set(TestStructMutable(unqualified_string="hello")),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )

    def test_del_attribute(self) -> None:
        w = TestStructMutable(unqualified_string="hello", optional_string="world")

        # Deleting an attribute on a (mutable) thrift-python instance raises
        # `AttributeError`
        with self.assertRaises(AttributeError):
            del w.unqualified_string

        with self.assertRaises(AttributeError):
            del w.optional_string

        self.assertEqual(w.unqualified_string, "hello")
        self.assertEqual(w.optional_string, "world")

        with self.assertRaises(AttributeError):
            del w.unqualified_string

        with self.assertRaises(AttributeError):
            del w.optional_string

    def test_type_hints(self) -> None:
        # Similar to thrift-python immutable structs, mutable structs do not
        # include type hints directly
        self.assertEqual(typing.get_type_hints(TestStructMutable), {})

    def test_to_immutable_python(self) -> None:
        w_mutable = TestStructMutable(unqualified_string="hello")
        w_immutable = w_mutable._to_python()
        self.assertIsNot(w_mutable, w_immutable)

        # Even though their contents are the same, the mutable and immutable
        # instance are not "equal":
        self.assertEqual(w_mutable.unqualified_string, w_immutable.unqualified_string)
        # Remember: set(<struct>) returns a set of (field name, value) tuples - see
        # test_iteration() above.
        self.assertSetEqual(set(w_mutable), set(w_immutable))
        self.assertNotEqual(w_mutable, w_immutable)

        # The newly obtained immutable object however is equal to a new
        # TestStructImmutable instance (with the same contents)
        self.assertEqual(w_immutable, TestStructImmutable(unqualified_string="hello"))

        w_mutable.unqualified_string = "hello, world!"
        self.assertNotEqual(
            w_mutable.unqualified_string, w_immutable.unqualified_string
        )

    def test_to_mutable_python(self) -> None:
        w_mutable = TestStructMutable(unqualified_string="hello")
        self.assertIs(w_mutable, w_mutable._to_mutable_python())

    def _assert_field_behavior(
        self,
        struct,
        field_name: str,
        expected_default_value,
        value,
        invalid_value,
        overflow_value=None,
    ):
        """
        This function is a helper function used to assert the behavior of a
        specific field in a structure.
            field_name (str): The name of the field to be tested.
            expected_default_value: The expected default value of the field.
            value: The value to be set for the field.
            invalid_value: A value of an incorrect type that, when set,
                should raise a TypeError.
            overflow_value (optional): A value that, when set, should raise
                an `OverflowError`. This is typically used for integral types.
        """
        # Check for the `expected_default_value`. The unqualified field should
        # never be `None`
        if expected_default_value is not None:
            self.assertIsNotNone(getattr(struct, field_name))
            self.assertEqual(expected_default_value, getattr(struct, field_name))
        else:  # OPTIONAL
            self.assertIsNone(getattr(struct, field_name))

        # Set the `value`, read it back
        setattr(struct, field_name, value)
        self.assertEqual(value, getattr(struct, field_name))

        # TODO: How to reset field to standard default value?
        struct._do_not_use_resetFieldToStandardDefault(field_name)
        if expected_default_value is not None:
            self.assertIsNotNone(getattr(struct, field_name))
            self.assertEqual(expected_default_value, getattr(struct, field_name))
        else:  # OPTIONAL
            self.assertIsNone(getattr(struct, field_name))

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            delattr(struct, field_name)

        # Assigning `None` raises a `TypeError`
        with self.assertRaises(TypeError):
            setattr(struct, field_name, None)

        # Value with wrong type raises `TypeError`
        with self.assertRaises(TypeError):
            setattr(struct, field_name, invalid_value)

        # For integral types check `OverflowError`
        if overflow_value is not None:
            with self.assertRaises(OverflowError):
                setattr(struct, field_name, overflow_value)

    @parameterized.expand(
        [
            # (field_name, expected_default_value, value, invalid_value, overflow_value")
            ("unqualified_bool", False, True, "Not Bool", None),
            ("optional_bool", None, True, "Not Bool", None),
            ("unqualified_byte", 0, max_byte, "Not Byte", max_byte + 1),
            ("optional_byte", None, max_byte, "Not Byte", max_byte + 1),
            ("unqualified_i16", 0, max_i16, "Not i16", max_i16 + 1),
            ("optional_i16", None, max_i16, "Not i16", max_i16 + 1),
            ("unqualified_i32", 0, max_i32, "Not i32", max_i32 + 1),
            ("optional_i32", None, max_i32, "Not i32", max_i32 + 1),
            ("unqualified_i64", 0, max_i64, "Not i64", max_i64 + 1),
            ("optional_i64", None, max_i64, "Not i64", max_i64 + 1),
            ("unqualified_float", 0.0, 1.0, "Not float", None),
            ("optional_float", None, 1.0, "Not float", None),
            ("unqualified_double", 0.0, 99.12, "Not double", None),
            ("optional_double", None, 99.12, "Not double", None),
            ("unqualified_string", "", "str-value", 999, None),
            ("optional_string", None, "str-value", 999, None),
        ]
    )
    def test_create_and_assign_for_all_primitive_types(
        self, field_name, expected_default_value, value, invalid_value, overflow_value
    ):
        s = TestStructAllThriftPrimitiveTypesMutable()
        self._assert_field_behavior(
            s,
            field_name=field_name,
            expected_default_value=expected_default_value,
            value=value,
            invalid_value=invalid_value,
            overflow_value=overflow_value,
        )

    @parameterized.expand(
        [
            # (field_name, expected_default_value, value, invalid_value, overflow_value")
            # `expected_default_value` is from IDL
            ("unqualified_bool", True, True, "Not Bool", None),
            ("unqualified_byte", 32, max_byte, "Not Byte", max_byte + 1),
            ("unqualified_i16", 512, max_i16, "Not i16", max_i16 + 1),
            ("unqualified_i32", 2048, max_i32, "Not i32", max_i32 + 1),
            ("unqualified_i64", 999, max_i64, "Not i64", max_i64 + 1),
            ("unqualified_float", 1.0, 1.0, "Not float", None),
            ("unqualified_double", 1.231, 99.12, "Not double", None),
            ("unqualified_string", "thrift-python", "str-value", 999, None),
        ]
    )
    def test_create_and_assign_for_all_primitive_types_with_default_values(
        self, field_name, expected_default_value, value, invalid_value, overflow_value
    ):
        s = TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable()
        self._assert_field_behavior(
            s,
            field_name=field_name,
            expected_default_value=expected_default_value,
            value=value,
            invalid_value=invalid_value,
            overflow_value=overflow_value,
        )

    def test_create_and_assign_for_i32(self) -> None:
        # This is the singular version of `test_create_and_assign_for_all_types`
        # for the i32 type. It's more readable since it doesn't use the
        # `verify_{qualified,optional}_helper` functions.
        s = TestStructAllThriftPrimitiveTypesMutable(unqualified_i32=11)

        # Check the value assigned during initialization
        self.assertEqual(11, s.unqualified_i32)

        # Set the value and read it back
        s.unqualified_i32 = 23
        self.assertEqual(23, s.unqualified_i32)

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            del s.unqualified_i32

        # Assigning `None` raises a `TypeError`
        with self.assertRaises(TypeError):
            s.unqualified_i32 = None

        # Assigning a value of the wrong type raises a `TypeError`
        with self.assertRaises(TypeError):
            s.unqualified_i32 = "This is not an integer"

        # Boundary check for integral types
        with self.assertRaises(OverflowError):
            s.unqualified_i32 = 2**31

        s = TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable()
        # from IDL: i32 unqualified_i32 = 2048;

        # Check the value from IDL during initialization
        self.assertEqual(2048, s.unqualified_i32)

        # Set the value and read it back
        s.unqualified_i32 = 32
        self.assertEqual(32, s.unqualified_i32)

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            del s.unqualified_i32

        # Assigning `None` raises a `TypeError`
        with self.assertRaises(TypeError):
            s.unqualified_i32 = None

        # Assigning a value of the wrong type raises a `TypeError`
        with self.assertRaises(TypeError):
            s.unqualified_i32 = "This is not an integer"

        # Boundary check for integral types
        with self.assertRaises(OverflowError):
            s.unqualified_i32 = -(2**31 + 1)

    def test_serialization_round_trip(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()
        s.unqualified_string = "Hello world!"
        s.optional_string = "Hello optional!"
        s.unqualified_i32 = 19
        s.optional_i32 = 23
        s.unqualified_double = 2.1
        s.optional_double = 1.3
        s.unqualified_bool = True
        s.optional_bool = False
        _thrift_serialization_round_trip(self, mutable_serializer, s)

        s_default_value = TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable()
        _thrift_serialization_round_trip(self, mutable_serializer, s_default_value)

    def test_create_and_assign_for_list(self) -> None:
        s = MutableTestStructAllThriftContainerTypes(unqualified_list_i32=[1, 2, 3])

        self.assertEqual(3, len(s.unqualified_list_i32))
        self.assertEqual([1, 2, 3], s.unqualified_list_i32)

        # Assigning to a list field
        s.unqualified_list_i32 = [1, 2, 3, 4, 5]
        self.assertEqual([1, 2, 3, 4, 5], s.unqualified_list_i32)
        s.unqualified_list_i32 = [1, 2, 3]
        self.assertEqual([1, 2, 3], s.unqualified_list_i32)

        s.unqualified_list_i32[0] = 2
        self.assertEqual([2, 2, 3], s.unqualified_list_i32)

        with self.assertRaisesRegex(IndexError, "list index out of range"):
            s.unqualified_list_i32[4]

        with self.assertRaisesRegex(IndexError, "list assignment index out of range"):
            s.unqualified_list_i32[4] = 2

        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s.unqualified_list_i32[4] = "Not integer"

        self.assertEqual([2, 2, 3], s.unqualified_list_i32)

        lst1 = s.unqualified_list_i32
        lst2 = s.unqualified_list_i32

        # lists are instance of thrift.python.mutable_containers.MutableList
        self.assertTrue(isinstance(lst1, MutableList))
        self.assertTrue(isinstance(lst2, MutableList))

        # lst1 and lst2 are the same instances
        self.assertTrue(lst1 is lst2)

        # Update on any variable is reflected on others
        self.assertEqual([2, 2, 3], s.unqualified_list_i32)
        self.assertEqual([2, 2, 3], lst1)
        self.assertEqual([2, 2, 3], lst2)

        lst2[1] = 10

        self.assertEqual([2, 10, 3], s.unqualified_list_i32)
        self.assertEqual([2, 10, 3], lst1)
        self.assertEqual([2, 10, 3], lst2)

        lst1.append(101)

        self.assertEqual([2, 10, 3, 101], s.unqualified_list_i32)
        self.assertEqual([2, 10, 3, 101], lst1)
        self.assertEqual([2, 10, 3, 101], lst2)

        self.assertEqual(101, s.unqualified_list_i32.pop())

        self.assertEqual([2, 10, 3], s.unqualified_list_i32)
        self.assertEqual([2, 10, 3], lst1)
        self.assertEqual([2, 10, 3], lst2)

        lst2.clear()

        self.assertEqual([], s.unqualified_list_i32)
        self.assertEqual([], lst1)
        self.assertEqual([], lst2)

        lst1.extend([11, 12, 13])

        self.assertEqual([11, 12, 13], s.unqualified_list_i32)
        self.assertEqual([11, 12, 13], lst1)
        self.assertEqual([11, 12, 13], lst2)

        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            lst2.extend([14, 15, "16", 17])

        # basic exception safety
        self.assertEqual([11, 12, 13, 14, 15], s.unqualified_list_i32)
        self.assertEqual([11, 12, 13, 14, 15], lst1)
        self.assertEqual([11, 12, 13, 14, 15], lst2)

    def test_assign_for_list(self) -> None:
        s1 = MutableTestStructAllThriftContainerTypes(unqualified_list_i32=[1, 2, 3])

        # It is possible to assign any value that supports `len()` and iteration
        s1.unqualified_list_i32 = [1, 2, 3]
        self.assertEqual([1, 2, 3], s1.unqualified_list_i32)
        s1.unqualified_list_i32 = {11, 12, 13}
        self.assertEqual([11, 12, 13], s1.unqualified_list_i32)
        s1.unqualified_list_i32 = (21, 22, 23)
        self.assertEqual([21, 22, 23], s1.unqualified_list_i32)
        s1.unqualified_list_i32 = []
        self.assertEqual([], s1.unqualified_list_i32)

        s2 = MutableTestStructAllThriftContainerTypes(unqualified_list_i32=[])
        # my_list and s2.unqualified_list_i32 are different lists
        my_list = [1, 2, 3]
        s2.unqualified_list_i32 = my_list
        self.assertEqual([1, 2, 3], s2.unqualified_list_i32)
        my_list[0] = 11
        self.assertEqual(1, s2.unqualified_list_i32[0])

        s3 = MutableTestStructAllThriftContainerTypes(unqualified_list_i32=[1, 2, 3])
        # Strong exception safety
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s3.unqualified_list_i32 = [11, 12, 13, "Not an Integer"]
        self.assertEqual([1, 2, 3], s3.unqualified_list_i32)

    def test_assign_for_set(self) -> None:
        s1 = MutableTestStructAllThriftContainerTypes(
            unqualified_set_string=["a", "b", "c"]
        )

        # It is possible to assign any value that supports iteration
        s1.unqualified_set_string = ["a", "b", "c"]
        self.assertEqual({"a", "b", "c"}, s1.unqualified_set_string)
        s1.unqualified_set_string = {"aa", "bb", "cc"}
        self.assertEqual({"aa", "bb", "cc"}, s1.unqualified_set_string)
        s1.unqualified_set_string = ("aaa", "bbb", "ccc")
        self.assertEqual({"aaa", "bbb", "ccc"}, s1.unqualified_set_string)

        # even from iterator, this is not possible for list field because of
        # `len()` requirement
        my_iter = iter(["x", "y", "z"])
        s1.unqualified_set_string = my_iter
        self.assertEqual({"x", "y", "z"}, s1.unqualified_set_string)

        s2 = MutableTestStructAllThriftContainerTypes(unqualified_set_string=[])
        # my_set and s2.unqualified_set_string are different sets
        my_set = {"a", "b", "c"}
        s2.unqualified_set_string = my_set
        self.assertEqual({"a", "b", "c"}, s2.unqualified_set_string)
        my_set.add("d")
        self.assertEqual(4, len(my_set))
        self.assertEqual(3, len(s2.unqualified_set_string))

        s3 = MutableTestStructAllThriftContainerTypes(
            unqualified_set_string=["a", "b", "c"]
        )
        # Strong exception safety
        with self.assertRaisesRegex(TypeError, "Expected type <class 'str'>"):
            s3.unqualified_set_string = ["aa", "bb", "cc", 999]
        self.assertEqual({"a", "b", "c"}, s3.unqualified_set_string)

    def test_assign_for_map(self) -> None:
        s1 = MutableTestStructAllThriftContainerTypes(
            unqualified_map_string_i32={"a": 1, "b": 2}
        )

        # It is possible to assign any mapping value that implements `items()`
        s1.unqualified_map_string_i32 = {"x": 1, "y": 2}
        self.assertEqual({"x": 1, "y": 2}, s1.unqualified_map_string_i32)

        class MyMapping:
            def items(self):
                return (("aa", 11), ("bb", 22))

        s1.unqualified_map_string_i32 = MyMapping()
        self.assertEqual({"aa": 11, "bb": 22}, s1.unqualified_map_string_i32)

        s2 = MutableTestStructAllThriftContainerTypes(unqualified_map_string_i32={})
        # my_map and s2.unqualified_map_string_i32 are different maps
        my_map = {"a": 1, "b": 2}
        s2.unqualified_map_string_i32 = my_map
        my_map["c"] = 3
        self.assertEqual(3, len(my_map))
        self.assertEqual(2, len(s2.unqualified_map_string_i32))

        s3 = MutableTestStructAllThriftContainerTypes(
            unqualified_map_string_i32={"a": 1, "b": 2}
        )
        # Strong exception safety
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            s3.unqualified_map_string_i32 = {"x": 1, "y": "Not an Integer"}
        self.assertEqual({"a": 1, "b": 2}, s3.unqualified_map_string_i32)

    @parameterized.expand(
        [
            (MutableTestStructAllThriftContainerTypes(),),
            (MutableTestStructAllThriftContainerTypes(unqualified_list_i32=[1, 2, 3]),),
            (MutableTestStructAllThriftContainerTypes(optional_list_i32=[11, 22, 33]),),
            (
                MutableTestStructAllThriftContainerTypes(
                    unqualified_list_i32=[1, 2], optional_list_i32=[3]
                ),
            ),
            (
                MutableTestStructAllThriftContainerTypes(
                    unqualified_set_string=["1", "2", "3"]
                ),
            ),
            (
                MutableTestStructAllThriftContainerTypes(
                    optional_set_string=["11", "22", "33"]
                ),
            ),
            (
                MutableTestStructAllThriftContainerTypes(
                    unqualified_set_string=["1", "2", "3"],
                    optional_set_string=["11", "22", "33"],
                ),
            ),
        ]
    )
    def test_container_serialization_round_trip(self, struct) -> None:
        _thrift_serialization_round_trip(self, mutable_serializer, struct)

    def test_adapted_types(self) -> None:
        s = MutableTestStructAdaptedTypes()
        # standard default value for i32 is 0, therefore `fromtimestamp(0)`
        self.assertEqual(
            s.unqualified_adapted_i32_to_datetime, datetime.fromtimestamp(0)
        )
        # from IDL, 3: `string unqualified_adapted_string_to_i32 = "123";`
        self.assertEqual(s.unqualified_adapted_string_to_i32, 123)

        new_date = datetime(2024, 5, 6, 12, 0, 0)
        s.unqualified_adapted_i32_to_datetime = new_date
        self.assertEqual(s.unqualified_adapted_i32_to_datetime, new_date)

        # Thrift simply passes the value to the adapter class. All type
        # checking is performed within the adapter class.
        with self.assertRaisesRegex(
            AttributeError, "'int' object has no attribute 'timestamp'"
        ):
            s.unqualified_adapted_i32_to_datetime = 123

        s.unqualified_adapted_string_to_i32 = 999
        self.assertEqual(s.unqualified_adapted_string_to_i32, 999)

    @parameterized.expand(
        [
            (
                MutableTestStructAdaptedTypes(
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400)
                ),
            ),
            (
                MutableTestStructAdaptedTypes(
                    unqualified_adapted_i32_to_datetime=datetime.fromtimestamp(0),
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400),
                ),
            ),
        ]
    )
    def test_adapter_serialization_round_trip(self, struct) -> None:
        _thrift_serialization_round_trip(self, mutable_serializer, struct)

    def test_typedef_simple(self) -> None:
        empty = TestStructEmptyMutable()
        empty_alias = TestStructEmptyAliasMutable()
        self.assertEqual(empty, empty_alias)

        struct = TestStructWithTypedefFieldMutable()
        struct.empty_struct = empty_alias
        struct.empty_struct_alias = empty

        _thrift_serialization_round_trip(self, mutable_serializer, struct)

    def test_create_and_init_for_set(self) -> None:
        # Initializing the `set` member with an iterable that contains duplicate
        # elements is fine. Thrift removes the duplicates.
        s = MutableTestStructAllThriftContainerTypes(
            unqualified_set_string=["1", "2", "2", "3", "3"]
        )
        self.assertEqual(3, len(s.unqualified_set_string))
        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)

        # Initializing the `set` member with an iterable that contains elements
        # with wrong type raises `TypeError`.
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s = MutableTestStructAllThriftContainerTypes(
                unqualified_set_string=["1", "2", "2", 9999, "3", "3"]
            )

    def test_create_and_assign_for_set(self) -> None:
        s = MutableTestStructAllThriftContainerTypes(
            unqualified_set_string=["1", "2", "3"]
        )

        self.assertEqual(3, len(s.unqualified_set_string))
        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)

        # Assigning to a set field
        s.unqualified_set_string = {"9", "8", "7"}
        self.assertEqual({"9", "8", "7"}, s.unqualified_set_string)
        s.unqualified_set_string = {"1", "2", "3"}
        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)

        # `__contains__()`
        self.assertIn("1", s.unqualified_set_string)
        self.assertIn("2", s.unqualified_set_string)
        self.assertNotIn("11", s.unqualified_set_string)
        self.assertNotIn("12", s.unqualified_set_string)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            self.assertIn(1, s.unqualified_set_string)

        # `add()`
        s.unqualified_set_string.add("4")
        self.assertEqual({"1", "2", "3", "4"}, s.unqualified_set_string)

        # uniqueness
        s.unqualified_set_string.add("3")
        s.unqualified_set_string.add("4")
        self.assertEqual({"1", "2", "3", "4"}, s.unqualified_set_string)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s.unqualified_set_string.add(999)

        # `remove()`
        s.unqualified_set_string.remove("1")
        self.assertEqual({"2", "3", "4"}, s.unqualified_set_string)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s.unqualified_set_string.remove(111)

        # `remove()` raises a `KeyError` if key is absent
        with self.assertRaisesRegex(KeyError, "111"):
            s.unqualified_set_string.remove("111")

        # `discard()`
        s.unqualified_set_string.discard("4")
        self.assertEqual({"2", "3"}, s.unqualified_set_string)

        # `discard()` does not raises a `KeyError` or `TypeError`
        s.unqualified_set_string.discard("111")
        s.unqualified_set_string.discard(111)

        set1 = s.unqualified_set_string
        set2 = s.unqualified_set_string

        # sets are instance of thrift.python.mutable_containers.MutableSet
        self.assertTrue(isinstance(set1, MutableSet))
        self.assertTrue(isinstance(set2, MutableSet))

        # set1 and set2 are the same instances
        self.assertIs(set1, set2)

        # Update on any variable is reflected on others
        self.assertEqual({"2", "3"}, s.unqualified_set_string)
        self.assertEqual({"2", "3"}, set1)
        self.assertEqual({"2", "3"}, set2)

        set1.add("1")

        self.assertEqual({"1", "2", "3"}, s.unqualified_set_string)
        self.assertEqual({"1", "2", "3"}, set1)
        self.assertEqual({"1", "2", "3"}, set2)

        # `isdisjoint(iterable)`
        self.assertTrue(set1.isdisjoint(["4", "5"]))
        self.assertTrue(set2.isdisjoint({"4", "5"}))
        self.assertFalse(set1.isdisjoint(["3", "4"]))
        self.assertFalse(set2.isdisjoint({"3", "4"}))

        other = MutableTestStructAllThriftContainerTypes(
            unqualified_set_string=["2", "3", "4"]
        )
        other_set = other.unqualified_set_string
        self.assertEqual({"1", "2", "3"}, set1)

        # `__and__()` (both sides are MutableSet)
        result = set1 & other_set
        self.assertIsInstance(result, MutableSet)
        self.assertIsNot(result, set1)
        self.assertIsNot(result, other_set)
        self.assertEqual({"1", "2", "3"}, set1)
        self.assertEqual({"2", "3", "4"}, other_set)
        self.assertEqual({"2", "3"}, result)

        # `__and__()` (left hand side is MutableSet)
        result = set1 & {"2", "3", "4"}
        self.assertIsInstance(result, MutableSet)
        self.assertEqual({"2", "3"}, result)

        # `__rand__() Not implemented yet` (right hand side is MutableSet)
        with self.assertRaisesRegex(TypeError, r"unsupported operand type\(s\) for \&"):
            result = {"2", "3", "4"} & set1

        # `__or__()` (both sides are MutableSet)
        result = set1 | other_set
        self.assertIsInstance(result, MutableSet)
        self.assertIsNot(result, set1)
        self.assertIsNot(result, other_set)
        self.assertEqual({"1", "2", "3"}, set1)
        self.assertEqual({"2", "3", "4"}, other_set)
        self.assertEqual({"1", "2", "3", "4"}, result)

        # `__or__()` (left hand side is MutableSet)
        result = set1 | {"2", "3", "4"}
        self.assertIsInstance(result, MutableSet)
        self.assertEqual({"1", "2", "3", "4"}, result)

        # `__ror__() Not implemented yet` (right hand side is MutableSet)
        with self.assertRaisesRegex(TypeError, r"unsupported operand type\(s\) for \|"):
            result = {"2", "3", "4"} | set1

        # `MutableSet` instances are not hashable
        with self.assertRaisesRegex(
            TypeError, "unhashable type: 'thrift.python.mutable_containers.MutableSet'"
        ):
            hash(set1)

        _thrift_serialization_round_trip(self, mutable_serializer, s)
        _thrift_serialization_round_trip(self, mutable_serializer, other)

        # `clear()`
        set2.clear()
        self.assertEqual(set(), s.unqualified_set_string)
        self.assertEqual(set(), set1)
        self.assertEqual(set(), set2)

    def test_create_and_assign_for_map(self) -> None:
        s = MutableTestStructAllThriftContainerTypes(
            unqualified_map_string_i32={"a": 1, "b": 2}
        )

        self.assertEqual(2, len(s.unqualified_map_string_i32))
        self.assertEqual({"a": 1, "b": 2}, s.unqualified_map_string_i32)

        # Assigning to a map field
        s.unqualified_map_string_i32 = {"x": 1, "y": 2}
        self.assertEqual({"x": 1, "y": 2}, s.unqualified_map_string_i32)
        s.unqualified_map_string_i32 = {"a": 1, "b": 2}
        self.assertEqual({"a": 1, "b": 2}, s.unqualified_map_string_i32)

        map1 = s.unqualified_map_string_i32
        map2 = s.unqualified_map_string_i32

        # maps are instance of thrift.python.mutable_containers.MutableMap
        self.assertTrue(isinstance(map1, MutableMap))
        self.assertTrue(isinstance(map2, MutableMap))

        # map1 and map2 are the same instances
        self.assertIs(map1, map2)

        # Update on any variable is reflected on others
        map1["c"] = 3
        self.assertEqual({"a": 1, "b": 2, "c": 3}, s.unqualified_map_string_i32)
        self.assertEqual({"a": 1, "b": 2, "c": 3}, map1)
        self.assertEqual({"a": 1, "b": 2, "c": 3}, map2)

        # `__contains__()`
        self.assertIn("a", s.unqualified_map_string_i32)
        self.assertIn("b", s.unqualified_map_string_i32)
        self.assertNotIn("x", s.unqualified_map_string_i32)
        self.assertNotIn("y", s.unqualified_map_string_i32)

        # `__contains__()` is type checked
        self.assertIn("a", s.unqualified_map_string_i32)
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            self.assertIn(1, s.unqualified_map_string_i32)

        # `__getitem__()`
        self.assertEqual(1, s.unqualified_map_string_i32["a"])
        self.assertEqual(2, s.unqualified_map_string_i32["b"])
        self.assertEqual(3, s.unqualified_map_string_i32["c"])

        with self.assertRaises(KeyError):
            s.unqualified_map_string_i32["Not Exists"]

        # `__getitem__()` is type checked
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s.unqualified_map_string_i32[999]

        # `get()`
        self.assertEqual(1, s.unqualified_map_string_i32.get("a"))
        # `get(, default=None)`
        self.assertEqual(None, s.unqualified_map_string_i32.get("Not Exists"))
        self.assertEqual(
            "MyDefaultValue",
            s.unqualified_map_string_i32.get("Not Exists", "MyDefaultValue"),
        )

        # `get()` is type checked
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s.unqualified_map_string_i32.get(999, "MyDefaultValue")

        # `__setitem__()`
        self.assertEqual(1, s.unqualified_map_string_i32["a"])
        s.unqualified_map_string_i32["a"] = 11
        self.assertEqual(11, s.unqualified_map_string_i32["a"])
        s.unqualified_map_string_i32["a"] = 1
        self.assertEqual(1, s.unqualified_map_string_i32["a"])

        # `__setitem__()` is type checked for both key and value
        with self.assertRaisesRegex(
            TypeError, "not a <class 'int'>, is actually of type <class 'str'>"
        ):
            s.unqualified_map_string_i32["a"] = "Not an integer"

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            s.unqualified_map_string_i32[999] = 11

        # `__iter__()`
        python_set = {"a", "b", "c"}
        for key in s.unqualified_map_string_i32:
            # `remove()` raises a `KeyError` if key is absent
            python_set.remove(key)

        self.assertEqual(0, len(python_set))

        # `keys()`
        self.assertEqual(3, len(s.unqualified_map_string_i32.keys()))

        python_set = {"a", "b", "c"}
        for key in s.unqualified_map_string_i32.keys():
            # `remove()` raises a `KeyError` if key is absent
            python_set.remove(key)

        # `keys()` returns a view
        keys = s.unqualified_map_string_i32.keys()
        self.assertEqual(3, len(keys))
        self.assertEqual({"a", "b", "c"}, set(keys))

        s.unqualified_map_string_i32["d"] = 4

        self.assertEqual(4, len(keys))
        self.assertEqual({"a", "b", "c", "d"}, set(keys))

        # `values()`
        self.assertEqual(4, len(s.unqualified_map_string_i32.values()))

        python_list = [1, 2, 3, 4]
        for value in s.unqualified_map_string_i32.values():
            python_list.remove(value)

        # `values()` returns a view
        values = s.unqualified_map_string_i32.values()
        self.assertEqual(4, len(values))
        self.assertEqual([1, 2, 3, 4], sorted(values))

        s.unqualified_map_string_i32["e"] = 5

        self.assertEqual(5, len(values))
        self.assertEqual([1, 2, 3, 4, 5], sorted(values))

        # `items()`
        self.assertEqual(5, len(s.unqualified_map_string_i32.items()))
        self.assertEqual(
            [("a", 1), ("b", 2), ("c", 3), ("d", 4), ("e", 5)],
            sorted(s.unqualified_map_string_i32.items()),
        )

        # `items()` returns a view
        items = s.unqualified_map_string_i32.items()
        self.assertEqual(5, len(items))
        self.assertEqual(
            [("a", 1), ("b", 2), ("c", 3), ("d", 4), ("e", 5)],
            sorted(items),
        )

        s.unqualified_map_string_i32["f"] = 6

        self.assertEqual(6, len(items))
        self.assertEqual(
            [("a", 1), ("b", 2), ("c", 3), ("d", 4), ("e", 5), ("f", 6)],
            sorted(items),
        )

        # `MutableMap` instances are not hashable
        with self.assertRaisesRegex(
            TypeError, "unhashable type: 'thrift.python.mutable_containers.MutableMap'"
        ):
            hash(s.unqualified_map_string_i32)

        _thrift_serialization_round_trip(self, mutable_serializer, s)

        # `clear()`
        s.unqualified_map_string_i32.clear()
        self.assertEqual({}, s.unqualified_map_string_i32)
        self.assertEqual({}, map1)
        self.assertEqual({}, map2)

    def test_constants(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()

        self.assertEqual(False, s.unqualified_bool)
        s.unqualified_bool = bool_constant
        self.assertEqual(True, s.unqualified_bool)

        self.assertEqual(0, s.unqualified_byte)
        s.unqualified_byte = byte_constant
        self.assertEqual(-10, s.unqualified_byte)

        self.assertEqual(0, s.unqualified_i16)
        s.unqualified_i16 = i16_constant
        self.assertEqual(200, s.unqualified_i16)

        self.assertEqual(0, s.unqualified_i32)
        s.unqualified_i32 = i32_constant
        self.assertEqual(0xFA12EE, s.unqualified_i32)

        self.assertEqual(0, s.unqualified_i64)
        s.unqualified_i64 = i64_constant
        self.assertEqual(0xFFFFFFFFFF, s.unqualified_i64)

        self.assertEqual(0, s.unqualified_float)
        s.unqualified_float = float_constant
        self.assertEqual(2.718281828459, s.unqualified_float)

        self.assertEqual(0, s.unqualified_double)
        s.unqualified_double = double_constant
        self.assertEqual(2.718281828459, s.unqualified_double)

        self.assertEqual("", s.unqualified_string)
        s.unqualified_string = string_constant
        self.assertEqual("June 28, 2017", s.unqualified_string)

        s = MutableTestStructAllThriftContainerTypes()

        self.assertEqual([], s.unqualified_list_i32)
        s.unqualified_list_i32.extend(list_constant)
        self.assertEqual([2, 3, 5, 7], s.unqualified_list_i32)

        s = MutableTestStructAllThriftContainerTypes(
            unqualified_set_string=set_constant
        )
        self.assertEqual({"foo", "bar", "baz"}, s.unqualified_set_string)

        s = MutableTestStructAllThriftContainerTypes(
            unqualified_map_string_i32=map_constant
        )
        self.assertEqual({"foo": 1, "bar": 2}, s.unqualified_map_string_i32)

    def test_nested_structs_init(self) -> None:
        """
        struct TestStructNested_2 {
          1: i32 i32_field;
        }

        struct TestStructNested_1 {
          1: i32 i32_field;
          2: TestStructNested_2 nested_2;
        }

        struct TestStructNested_0 {
          1: i32 i32_field;
          2: TestStructNested_1 nested_1;
        }
        """
        s2 = TestStructNested_2_Mutable(i32_field=2)
        s1 = TestStructNested_1_Mutable(i32_field=3, nested_2=s2)
        s0 = TestStructNested_0_Mutable(i32_field=5, nested_1=s1)

        self.assertEqual(s2, s0.nested_1.nested_2)
        self.assertEqual(s1, s0.nested_1)

        self.assertEqual(2, s0.nested_1.nested_2.i32_field)
        self.assertEqual(3, s0.nested_1.i32_field)

        # Update on `s2` updates both `s1` and `s2`
        self.assertEqual(2, s0.nested_1.nested_2.i32_field)
        self.assertEqual(2, s1.nested_2.i32_field)
        s2.i32_field = 7
        self.assertEqual(7, s0.nested_1.nested_2.i32_field)
        self.assertEqual(7, s1.nested_2.i32_field)

        # Accessing the same field returns the same instances
        my_s2_var1 = s0.nested_1.nested_2
        my_s2_var2 = s0.nested_1.nested_2

        self.assertIs(my_s2_var1, my_s2_var2)

        # Update on `my_s2_var1` or `my_s2_var2` updates all references
        my_s2_var1.i32_field = 11
        self.assertEqual(11, s0.nested_1.nested_2.i32_field)
        self.assertEqual(11, s1.nested_2.i32_field)
        self.assertEqual(11, s2.i32_field)

        # Unfortunately, this is not intuitive, `my_s2_var1` is not `s1`
        self.assertIs(my_s2_var1, my_s2_var2)
        self.assertIsNot(s1, my_s2_var2)

    def test_create_for_struct_with_union_field(self) -> None:
        _ = TestStructWithUnionFieldMutable()

    def test_exception(self) -> None:
        s = TestStructAllThriftPrimitiveTypesMutable()
        self.assertIsInstance(s, MutableStruct)
        self.assertNotIsInstance(s, MutableGeneratedError)

        # `Error` <- `MutableGeneratedError`
        # is an independent type hierarchy than
        # `MutableStructOrUnion` <- `MutableStruct`
        e = TestExceptionAllThriftPrimitiveTypesMutable()
        self.assertNotIsInstance(e, MutableStructOrUnion)
        self.assertNotIsInstance(e, MutableStruct)
        self.assertIsInstance(e, Error)
        self.assertIsInstance(e, MutableGeneratedError)

    def test_create_for_struct_with_exception_field(self) -> None:
        _ = TestStructWithExceptionFieldMutable()

    @parameterized.expand(
        [
            (TestExceptionAllThriftPrimitiveTypesMutable(),),
            (TestExceptionAllThriftPrimitiveTypesMutable(unqualified_i32=2),),
            (TestStructWithExceptionFieldMutable(),),
            (TestStructWithExceptionFieldMutable(i32_field=3),),
            (
                TestStructWithExceptionFieldMutable(
                    i32_field=5,
                    exception_field=TestExceptionAllThriftPrimitiveTypesMutable(
                        unqualified_string="Hello World!"
                    ),
                ),
            ),
        ]
    )
    def test_exception_serialization_round_trip(self, struct_or_exception) -> None:
        _thrift_serialization_round_trip(self, mutable_serializer, struct_or_exception)
