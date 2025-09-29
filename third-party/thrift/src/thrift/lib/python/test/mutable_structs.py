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

import copy
import pickle
import types
import typing
import unittest
from datetime import datetime

import thrift.python.mutable_serializer as mutable_serializer

from parameterized import parameterized

from thrift.python.mutable_exceptions import MutableGeneratedError
from thrift.python.mutable_types import (
    _isset as mutable_isset,
    MutableStruct,
    MutableStructOrUnion,
    to_thrift_list,
    to_thrift_map,
    to_thrift_set,
)

from thrift.test.thrift_python.struct_test.thrift_mutable_types import (
    TestComplexContainersStruct,
    TestContainersWithStructElements as TestContainersWithStructElementsMutable,
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesMutable,
    TestStruct,
    TestStructAdaptedTypes as TestStructAdaptedTypesMutable,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesMutable,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesMutable,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable,
    TestStructAsListElement as TestStructAsListElementMutable,
    TestStructBool as TestStructBoolMutable,
    TestStructContainerAssignment as TestStructContainerAssignmentMutable,
    TestStructNested_0 as TestStructNested_0_Mutable,
    TestStructWithDefaultValues as TestStructWithDefaultValuesMutable,
    TestStructWithExceptionField as TestStructWithExceptionFieldMutable,
    TestStructWithFloatingPoint as TestStructWithFloatingPointMutable,
    TestStructWithUnionField as TestStructWithUnionFieldMutable,
)

from thrift.test.thrift_python.struct_test.thrift_types import (
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesImmutable,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesImmutable,
    TestStructNested_0 as TestStructNested_0_Immutable,
    TestStructNested_1 as TestStructNested_1_Immutable,
    TestStructNested_2 as TestStructNested_2_Immutable,
    TestStructWithNestedContainers as TestStructWithNestedContainersImmutable,
)

max_byte: int = 2**7 - 1
max_i16: int = 2**15 - 1
max_i32: int = 2**31 - 1
max_i64: int = 2**63 - 1


def _thrift_serialization_round_trip(
    test: unittest.TestCase,
    module: types.ModuleType,
    control: typing.Union[MutableStructOrUnion, MutableGeneratedError],
) -> None:
    for proto in module.Protocol:
        encoded = module.serialize(control, protocol=proto)
        test.assertIsInstance(encoded, bytes)

        decoded = module.deserialize(type(control), encoded, protocol=proto)
        test.assertIsInstance(decoded, type(control))
        test.assertEqual(control, decoded)


def _pickle_round_trip(
    test: unittest.TestCase,
    control: typing.Union[MutableStructOrUnion],
) -> None:
    pickled = pickle.dumps(control, protocol=pickle.HIGHEST_PROTOCOL)
    unpickled = pickle.loads(pickled)
    test.assertIsInstance(unpickled, type(control))
    test.assertEqual(control, unpickled)


class ThriftPython_MutableStruct_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def _assert_field_behavior(
        self,
        struct: MutableStruct,
        field_name: str,
        expected_default_value: object,
        value: object,
        invalid_value: object,
        overflow_value: typing.Optional[object] = None,
    ) -> None:
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
        # If `expected_default_value` is `None`, field is optional
        is_optional = expected_default_value is None

        # Check for the `expected_default_value`. The unqualified field should
        # never be `None`
        if not is_optional:
            self.assertIsNotNone(getattr(struct, field_name))
            self.assertEqual(expected_default_value, getattr(struct, field_name))
            self.assertFalse(mutable_isset(struct)[field_name])
        else:
            self.assertIsNone(getattr(struct, field_name))
            self.assertFalse(mutable_isset(struct)[field_name])

        # Set the `value`, read it back
        setattr(struct, field_name, value)
        self.assertEqual(value, getattr(struct, field_name))
        self.assertTrue(mutable_isset(struct)[field_name])

        # pyre-ignore[16]: internal, could be remove/replaced later
        struct._fbthrift_internal_resetFieldToStandardDefault(field_name)
        if not is_optional:
            self.assertIsNotNone(getattr(struct, field_name))
            self.assertEqual(expected_default_value, getattr(struct, field_name))
            self.assertTrue(mutable_isset(struct)[field_name])
        else:
            self.assertIsNone(getattr(struct, field_name))
            self.assertFalse(mutable_isset(struct)[field_name])

        # `del struct.field_name` raises a `AttributeError`
        with self.assertRaises(AttributeError):
            delattr(struct, field_name)

        # Assigning `None` raises a `TypeError` for non-optional fields
        if not is_optional:
            with self.assertRaises(TypeError):
                setattr(struct, field_name, None)
        else:
            # For optional fields, assigning `None` resets the field
            setattr(struct, field_name, value)
            self.assertTrue(mutable_isset(struct)[field_name])
            setattr(struct, field_name, None)
            self.assertFalse(mutable_isset(struct)[field_name])

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
        self,
        field_name: str,
        expected_default_value: object,
        value: object,
        invalid_value: object,
        overflow_value: object,
    ) -> None:
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
        self,
        field_name: str,
        expected_default_value: object,
        value: object,
        invalid_value: object,
        overflow_value: object,
    ) -> None:
        s = TestStructAllThriftPrimitiveTypesWithDefaultValuesMutable()
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
            (TestStructAllThriftContainerTypesMutable(),),
            (
                TestStructAllThriftContainerTypesMutable(
                    unqualified_list_i32=to_thrift_list([1, 2, 3])
                ),
            ),
            (
                TestStructAllThriftContainerTypesMutable(
                    optional_list_i32=to_thrift_list([11, 22, 33])
                ),
            ),
            (
                TestStructAllThriftContainerTypesMutable(
                    unqualified_list_i32=to_thrift_list([1, 2]),
                    optional_list_i32=to_thrift_list([3]),
                ),
            ),
            (
                TestStructAllThriftContainerTypesMutable(
                    unqualified_set_string=to_thrift_set({"1", "2", "3"})
                ),
            ),
            (
                TestStructAllThriftContainerTypesMutable(
                    optional_set_string=to_thrift_set({"11", "22", "33"})
                ),
            ),
            (
                TestStructAllThriftContainerTypesMutable(
                    unqualified_set_string=to_thrift_set({"1", "2", "3"}),
                    optional_set_string=to_thrift_set({"11", "22", "33"}),
                ),
            ),
        ]
    )
    def test_container_serialization_round_trip(self, struct: MutableStruct) -> None:
        _thrift_serialization_round_trip(self, mutable_serializer, struct)
        _pickle_round_trip(self, struct)

    @parameterized.expand(
        [
            (
                TestStructAdaptedTypesMutable(
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400)
                ),
            ),
            (
                TestStructAdaptedTypesMutable(
                    unqualified_adapted_i32_to_datetime=datetime.fromtimestamp(0),
                    optional_adapted_i32_to_datetime=datetime.fromtimestamp(86400),
                ),
            ),
        ]
    )
    def test_adapter_serialization_round_trip(self, struct: MutableStruct) -> None:
        _thrift_serialization_round_trip(self, mutable_serializer, struct)
        _pickle_round_trip(self, struct)

    def test_create_for_struct_with_union_field(self) -> None:
        _ = TestStructWithUnionFieldMutable()

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
    def test_exception_serialization_round_trip(
        self, struct_or_exception: typing.Union[MutableStruct, MutableGeneratedError]
    ) -> None:
        _thrift_serialization_round_trip(self, mutable_serializer, struct_or_exception)

    def test_struct_repr(self) -> None:
        TestStructAllThriftPrimitiveTypes = TestStructAllThriftPrimitiveTypesMutable
        mutable = TestStructAllThriftPrimitiveTypes()
        self.assertEqual(mutable, eval(repr(mutable)))
        self.assertEqual(
            repr(mutable),
            "TestStructAllThriftPrimitiveTypes(unqualified_bool=False, optional_bool=None, unqualified_byte=0, optional_byte=None, unqualified_i16=0, optional_i16=None, unqualified_i32=0, optional_i32=None, unqualified_i64=0, optional_i64=None, unqualified_float=0.0, optional_float=None, unqualified_double=0.0, optional_double=None, unqualified_string='', optional_string=None)",
        )

        TestStructAllThriftContainerTypes = TestStructAllThriftContainerTypesMutable
        mutable = TestStructAllThriftContainerTypes()
        self.assertEqual(mutable, eval(repr(mutable)))
        self.assertEqual(
            repr(mutable),
            "TestStructAllThriftContainerTypes(unqualified_list_i32=to_thrift_list([]), optional_list_i32=None, unqualified_set_string=to_thrift_set(set()), optional_set_string=None, unqualified_map_string_i32=to_thrift_map({}), optional_map_string_i32=None)",
        )

    def test_complex_container_struct_repr(self) -> None:
        mutable_struct = TestComplexContainersStruct(
            list_int=to_thrift_list([]),
            set_int=to_thrift_set(set()),
            map_int_to_string=to_thrift_map({}),
            list_of_list=to_thrift_list([[]]),
            list_of_set=to_thrift_list([set()]),
            set_of_list=to_thrift_set(set()),
            list_map_int_to_set=to_thrift_list([{0: set()}]),
            map_int_to_list_of_set=to_thrift_map({0: [set()]}),
            list_of_struct=to_thrift_list([TestStruct()]),
        )
        self.assertEqual(mutable_struct, eval(repr(mutable_struct)))

        mutable_struct = TestComplexContainersStruct(
            list_int=to_thrift_list([1, 2, 3]),
            set_int=to_thrift_set({1, 2, 3}),
            map_int_to_string=to_thrift_map({1: "abc", 2: "xyz"}),
            list_of_list=to_thrift_list([[1], [2]]),
            list_of_set=to_thrift_list([set()]),
            set_of_list=to_thrift_set(set()),
            list_map_int_to_set=to_thrift_list([{0: {1, 2, 3}}]),
            map_int_to_list_of_set=to_thrift_map({0: [{1, 2, 3}]}),
            list_of_struct=to_thrift_list([TestStruct(unqualified_string="abc")]),
        )
        self.assertEqual(mutable_struct, eval(repr(mutable_struct)))

    def test_struct_default_value_cache_primitives(self) -> None:
        # Immutable-types uses a default-value cache and initialize the fields
        # with the same cached Python object repeatedly. However, this approach
        # doesn't work well with mutable types when there is no copy-on-write
        # mechanism in place. The issue is particullarly problematic with the
        # container types but this test checks it for primitive types.
        mutable_s1 = TestStructAllThriftPrimitiveTypesMutable()
        mutable_s2 = TestStructAllThriftPrimitiveTypesMutable()

        # Update the fields from `s1_mutable`. This update should affect only
        # `s1_mutable` and not the default value cache or any other struct.
        mutable_s1.unqualified_string = "Hello world!"
        mutable_s1.unqualified_i32 = 11

        self.assertEqual("Hello world!", mutable_s1.unqualified_string)
        self.assertEqual(11, mutable_s1.unqualified_i32)

        # Check that changes to `mutable_s1` do not affect `mutable_s2`.
        self.assertEqual("", mutable_s2.unqualified_string)
        self.assertEqual(0, mutable_s2.unqualified_i32)

        # Check that a new struct, potentially utilizing the default value
        # cache, initializes with the correct default value.
        mutable_s3 = TestStructAllThriftPrimitiveTypesMutable()
        self.assertEqual("", mutable_s3.unqualified_string)
        self.assertEqual(0, mutable_s3.unqualified_i32)

        # Check for immutable struct
        immutable_s1 = TestStructAllThriftPrimitiveTypesImmutable()
        self.assertEqual("", immutable_s1.unqualified_string)
        self.assertEqual(0, immutable_s1.unqualified_i32)

    def test_struct_default_value_cache_container(self) -> None:
        # Immutable types use a default-value cache and initialize the fields
        # with the same cached Python object repeatedly. However, this approach
        # doesn't work well with mutable types when there is no copy-on-write
        # mechanism in place.
        #
        # For example: If a cached empty Python list is used to populate a list
        # field in different structs, but a mutable struct can append to that
        # list, which will update the value for all other structs as well as
        # the list object in the default value cache.
        mutable_s1 = TestStructAllThriftContainerTypesMutable()
        mutable_s2 = TestStructAllThriftContainerTypesMutable()

        # Update the container from `s1_mutable`. This update should affect only
        # `s1_mutable` and not the default value cache or any other struct.
        mutable_s1.unqualified_list_i32.append(1)

        self.assertEqual([1], mutable_s1.unqualified_list_i32)

        # Check that changes to `mutable_s1` do not affect `mutable_s2`.
        self.assertEqual([], mutable_s2.unqualified_list_i32)

        # Check that a new struct, potentially utilizing the default value
        # cache, initializes with the correct default value.
        mutable_s3 = TestStructAllThriftContainerTypesMutable()
        self.assertEqual([], mutable_s3.unqualified_list_i32)

        # Check for immutable struct
        immutable_s1 = TestStructAllThriftContainerTypesImmutable()
        self.assertEqual([], immutable_s1.unqualified_list_i32)

    def test_struct_user_default_value(self) -> None:
        """
        struct TestStructWithDefaultValues {
          1: i32 unqualified_integer = 42;

          2: optional i32 optional_integer = 43;

          3: TestStruct unqualified_struct = TestStruct{unqualified_string = "hello"};

          4: optional TestStruct optional_struct = TestStruct{
            unqualified_string = "world",
          };

          5: TestStruct unqualified_struct_intrinsic_default;

          6: optional TestStruct optional_struct_intrinsic_default;

          7: list<i32> unqualified_list_i32 = [1, 2, 3];
        }
        """
        mutable_s1 = TestStructWithDefaultValuesMutable()
        self.assertEqual(42, mutable_s1.unqualified_integer)
        self.assertEqual("hello", mutable_s1.unqualified_struct.unqualified_string)
        self.assertEqual([1, 2, 3], mutable_s1.unqualified_list_i32)

        mutable_s1.unqualified_integer = 11
        mutable_s1.unqualified_struct.unqualified_string = "world"
        mutable_s1.unqualified_list_i32.append(4)

        self.assertEqual(11, mutable_s1.unqualified_integer)
        self.assertEqual("world", mutable_s1.unqualified_struct.unqualified_string)
        self.assertEqual([1, 2, 3, 4], mutable_s1.unqualified_list_i32)

        # Updating the values in a mutable struct instance should not update
        # the default values. Verify that a new instance is created with
        # user-specified default values.
        mutable_s2 = TestStructWithDefaultValuesMutable()
        self.assertEqual(42, mutable_s2.unqualified_integer)
        self.assertEqual("hello", mutable_s2.unqualified_struct.unqualified_string)
        self.assertEqual([1, 2, 3], mutable_s2.unqualified_list_i32)

    def test_struct_attribute_cache(self) -> None:
        """
        struct TestStructAsListElement {
          1: string string_field;
          2: list<i32> list_int;
        }

        struct TestStructContainerAssignment {
          ...
          5: list<TestStructAsListElement> list_struct;
          ...
        }
        """
        s_elem = TestStructAsListElementMutable(
            string_field="elem", list_int=to_thrift_list([1])
        )

        s = TestStructContainerAssignmentMutable(
            list_struct=to_thrift_list([s_elem, s_elem])
        )

        # `to_thrift_list()` always copies the containers inside it. However,
        # if the container element is a Thrift struct, union or exception,
        # assignment uses reference semantics for them.
        self.assertEqual(s_elem, s.list_struct[0])
        self.assertEqual(s_elem, s.list_struct[1])

        # `s_elem`, `s.list_struct[0]` and  `s.list_struct[1]` are the "same"
        # structs

        # Demonstrate that they are the "same" structs by updating the `list_int`.
        self.assertEqual([1], s_elem.list_int)
        self.assertEqual([1], s.list_struct[0].list_int)
        self.assertEqual([1], s.list_struct[1].list_int)
        s.list_struct[0].list_int.append(2)
        self.assertEqual([1, 2], s_elem.list_int)
        self.assertEqual([1, 2], s.list_struct[0].list_int)
        self.assertEqual([1, 2], s.list_struct[1].list_int)

        # Demonstrate that they are the "same" structs by updating the `string_field`.
        # String fields are unique because when they are first accessed, they are
        # read from `struct._fbthrift_data`, converted to a Python type, and cached.
        # On subsequent accesses, the underlying `struct._fbthrift_data` is not
        # accessed again; the cached value is used.
        self.assertEqual("elem", s_elem.string_field)
        self.assertEqual("elem", s.list_struct[0].string_field)
        self.assertEqual("elem", s.list_struct[1].string_field)
        s.list_struct[1].string_field = "updated"
        self.assertEqual("updated", s_elem.string_field)
        self.assertEqual("updated", s.list_struct[0].string_field)
        self.assertEqual("updated", s.list_struct[1].string_field)

    def test_copy_from_immutable_struct(self) -> None:
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
        s_immutable = TestStructNested_0_Immutable(
            i32_field=42,
            nested_1=TestStructNested_1_Immutable(
                i32_field=41, nested_2=TestStructNested_2_Immutable(i32_field=40)
            ),
        )
        s_mutable = TestStructNested_0_Mutable(i32_field=0)

        # Assignment from immutable struct fields to mutable structs fields
        # would not work for all field types, for example, for struct fields
        with self.assertRaisesRegex(
            TypeError,
            "is not a <class '.*thrift_mutable_types.TestStructNested_1'>, "
            "is actually of type <.*thrift_types.TestStructNested_1'>.",
        ):
            for field_name, field_value in s_immutable:
                setattr(s_mutable, field_name, field_value)

        # with `._to_mutable_python()`
        for field_name, field_value in s_immutable._to_mutable_python():
            setattr(s_mutable, field_name, field_value)

        self.assertEqual(42, s_mutable.i32_field)
        self.assertEqual(41, s_mutable.nested_1.i32_field)
        self.assertEqual(40, s_mutable.nested_1.nested_2.i32_field)

    def test_conversion_nested_containers(self) -> None:
        s = TestStructWithNestedContainersImmutable(
            list_list_i32=[[1]],
            list_set_i32=[{2}],
            list_map_string_i32=[{"a": 1}],
            list_map_string_list_i32=[{"b": [2]}],
            list_map_string_set_i32=[{"c": {3}}],
            map_i32_list_i32={1: [1]},
            map_i32_set_i32={2: {2}},
            map_i32_map_string_i32={3: {"a": 1}},
            map_i32_map_string_list_i32={4: {"b": [2]}},
            map_i32_map_string_set_i32={5: {"c": {3}}},
            many_nested=[[{1: [{"a": [{2}]}]}]],
        )

        mutable_struct = s._to_mutable_python()
        self.assertEqual(s, mutable_struct._to_python())

    def test_container_with_struct_elements(self) -> None:
        s1 = TestContainersWithStructElementsMutable()
        s2 = TestContainersWithStructElementsMutable()

        self.assertEqual(s1, s2)

        s1.map_fld["1"] = TestStructBoolMutable()
        s2.map_fld["1"] = TestStructBoolMutable()

        self.assertEqual(s1, s2)

        s1.map_fld["1"].is_set = False

        self.assertEqual(s1, s2)

    def test_deepcopy_floating_point(self) -> None:
        p = 3.14159265359
        s1 = TestStructWithFloatingPointMutable(float_field=p, double_field=p)

        self.assertEqual(s1.float_field, p)
        self.assertEqual(s1.double_field, p)

        s2 = copy.deepcopy(s1)  # or s1()

        self.assertEqual(s2.double_field, p)

        # In Python, runtime floating-point numbers are double precision;
        # however, during a deepcopy, the object goes through a serialization
        # cycle, and the Thrift float field is truncated to its 'real' value
        # that can be held by a single precision floating field
        self.assertNotEqual(s2.float_field, p)
