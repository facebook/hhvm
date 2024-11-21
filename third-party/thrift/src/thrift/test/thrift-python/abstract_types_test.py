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

import datetime
import typing
import unittest

import thrift.test.thrift_python.included.thrift_abstract_types

from folly import iobuf

from parameterized import parameterized

from thrift.test.thrift_python.struct_test.thrift_abstract_types import (  # @manual=//thrift/test/thrift-python:struct_test_thrift-python-types
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesAbstract,
    TestExceptionCopy as TestExceptionCopyAbstract,
    TestStruct as TestStructAbstract,
    TestStructAdaptedTypes as TestStructAdaptedTypesAbstract,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesAbstract,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesAbstract,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract,
    TestStructCopy as TestStructCopyAbstract,
    TestStructEmpty as TestStructEmptyAbstract,
    TestStructEmptyAlias as TestStructEmptyAliasAbstract,
    TestStructNested_0 as TestStructNested_0Abstract,
    TestStructNested_1 as TestStructNested_1Abstract,
    TestStructNested_2 as TestStructNested_2Abstract,
    TestStructWithDefaultValues as TestStructWithDefaultValuesAbstract,
    TestStructWithExceptionField as TestStructWithExceptionFieldAbstract,
    TestStructWithTypedefField as TestStructWithTypedefFieldAbstract,
    TestStructWithUnionField as TestStructWithUnionFieldAbstract,
)

from thrift.test.thrift_python.struct_test.thrift_mutable_types import (
    TestStruct as TestStructMutable,
)

from thrift.test.thrift_python.struct_test.thrift_types import (
    TestStruct as TestStructImmutable,
)

from thrift.test.thrift_python.union_test.thrift_abstract_types import (  # @manual=//thrift/test/thrift-python:union_test_thrift-python-types
    TestUnion as TestUnionAbstract,
    TestUnionAmbiguousTypeFieldName as TestUnionAmbiguousTypeFieldNameAbstract,
    # TODO: Uncomment when adapted types work correctly.
    # TestUnionAdaptedTypes as TestUnionAdaptedTypesAbstract,
)

from thrift.test.thrift_python.union_test.thrift_mutable_types import (
    TestUnion as TestUnionMutable,
)

from thrift.test.thrift_python.union_test.thrift_types import (
    TestUnion as TestUnionImmutable,
)


class ThriftPythonAbstractTypesTest(unittest.TestCase):
    @staticmethod
    # pyre-ignore[2]
    # pyre-ignore[3]
    def _get_property_type(property_descriptor) -> typing.Type[typing.Any]:
        type_hints = typing.get_type_hints(property_descriptor.fget)

        return type_hints["return"]

    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_fn_call_with_read_only_abstract_base_class_with_immutable(self) -> None:
        """
        type-check will fail with the error below if
        TestStructImmutable does not inherit from TestStructAbstract.

        Incompatible parameter type [6]: In call `ThriftPythonAbstractTypesTest.test_fn_call_with_read_only_abstract_base_class_with_immutable.library_fn`, for 1st positional argument, expected `TestStructAbstract` but got `TestStructImmutable`.
        """

        # GIVEN
        def library_fn(ts: TestStructAbstract) -> None:
            # THEN
            self.assertEqual(ts.unqualified_string, "hello")
            self.assertIsNone(ts.optional_string)

        # WHEN
        library_fn(TestStructImmutable(unqualified_string="hello"))

    def test_type_hint_with_read_only_abstract_base_class_with_immutable(self) -> None:
        """
        type-check will fail with the error below if
        TestStructImmutable does not inherit from TestStructAbstract.

        Incompatible variable type [9]: ts is declared to have type `TestStructAbstract` but is used as type `TestStructImmutable`.
        """
        # GIVEN
        # TestStructAbstract
        # TestStructImmutable

        # WHEN
        ts: TestStructAbstract = TestStructImmutable(unqualified_string="hello")

        # THEN
        self.assertEqual(ts.unqualified_string, "hello")
        self.assertIsNone(ts.optional_string)

    def test_isinstance_with_read_only_abstract_base_class_with_immutable(self) -> None:
        """ """
        # GIVEN
        # TestStructAbstract
        # TestStructImmutable

        # WHEN
        # This assignment is here to validate type-checking.
        ts: TestStructAbstract = TestStructImmutable(unqualified_string="hello")

        # THEN
        self.assertIsInstance(ts, TestStructAbstract)

    def test_issubclass_with_read_only_abstract_base_class_with_immutable(self) -> None:
        """ """
        self.assertTrue(issubclass(TestStructImmutable, TestStructAbstract))

    def test_fn_call_with_read_only_abstract_base_class_with_mutable(self) -> None:
        """
        type-check will fail with the error below if
        TestStructMutable does not inherit from TestStructAbstract.

        Incompatible parameter type [6]: In call `ThriftPythonAbstractTypesTest.test_fn_call_with_read_only_abstract_base_class_with_mutable.library_fn`, for 1st positional argument, expected `TestStructAbstract` but got `TestStructMutable`.
        """

        # GIVEN
        def library_fn(ts: TestStructAbstract) -> None:
            # THEN
            self.assertEqual(ts.unqualified_string, "hello")
            self.assertIsNone(ts.optional_string)

        # WHEN
        library_fn(TestStructMutable(unqualified_string="hello"))

    def test_type_hint_with_read_only_abstract_base_class_with_mutable(self) -> None:
        """
        type-check will fail with the error below if
        TestStructMutable does not inherit from TestStructAbstract.

        Incompatible variable type [9]: ts is declared to have type `TestStructAbstract` but is used as type `TestStructMutable`.
        """
        # GIVEN
        # TestStructAbstract
        # TestStructMutable

        # WHEN
        ts: TestStructAbstract = TestStructMutable(unqualified_string="hello")

        # THEN
        self.assertEqual(ts.unqualified_string, "hello")
        self.assertIsNone(ts.optional_string)

    def test_isinstance_with_read_only_abstract_base_class_with_mutable(self) -> None:
        """ """
        # GIVEN
        # TestStructAbstract
        # TestStructMutable

        # WHEN
        ts: TestStructAbstract = TestStructMutable(unqualified_string="hello")

        # THEN
        self.assertIsInstance(ts, TestStructAbstract)

    def test_issubclass_with_read_only_abstract_base_class_with_mutable(self) -> None:
        """ """
        self.assertTrue(issubclass(TestStructMutable, TestStructAbstract))

    @parameterized.expand(
        [
            (
                "TestStructAdaptedTypesAbstract.unqualified_adapted_i32_to_datetime",
                TestStructAdaptedTypesAbstract.unqualified_adapted_i32_to_datetime,
                datetime.datetime,
            ),
            (
                "TestStructAdaptedTypesAbstract.optional_adapted_i32_to_datetime",
                TestStructAdaptedTypesAbstract.optional_adapted_i32_to_datetime,
                typing.Optional[datetime.datetime],
            ),
            (
                "TestStructAdaptedTypesAbstract.unqualified_adapted_datetime_to_i32",
                TestStructAdaptedTypesAbstract.unqualified_adapted_string_to_i32,
                int,
            ),
            (
                "TestStructWithDefaultValuesAbstract.unqualified_integer",
                TestStructWithDefaultValuesAbstract.unqualified_integer,
                int,
            ),
            (
                "TestStructWithDefaultValuesAbstract.optional_integer",
                TestStructWithDefaultValuesAbstract.optional_integer,
                typing.Optional[int],
            ),
            (
                "TestStructWithDefaultValuesAbstract.unqualified_string",
                TestStructWithDefaultValuesAbstract.unqualified_struct,
                TestStructAbstract,
            ),
            (
                "TestStructWithDefaultValuesAbstract.optional_struct",
                TestStructWithDefaultValuesAbstract.optional_struct,
                typing.Optional[TestStructAbstract],
            ),
            (
                "TestStructWithDefaultValuesAbstract.unqualified_struct_intrinsic_default",
                TestStructWithDefaultValuesAbstract.unqualified_struct_intrinsic_default,
                TestStructAbstract,
            ),
            (
                "TestStructWithDefaultValuesAbstract.optional_struct_intrinsic_default",
                TestStructWithDefaultValuesAbstract.optional_struct_intrinsic_default,
                typing.Optional[TestStructAbstract],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_bool",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_bool,
                bool,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_bool",
                TestStructAllThriftPrimitiveTypesAbstract.optional_bool,
                typing.Optional[bool],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_byte",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_byte,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_byte",
                TestStructAllThriftPrimitiveTypesAbstract.optional_byte,
                typing.Optional[int],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_i16",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_i16,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_i16",
                TestStructAllThriftPrimitiveTypesAbstract.optional_i16,
                typing.Optional[int],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_i32",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_i32,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_i32",
                TestStructAllThriftPrimitiveTypesAbstract.optional_i32,
                typing.Optional[int],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_i64",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_i64,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_i64",
                TestStructAllThriftPrimitiveTypesAbstract.optional_i64,
                typing.Optional[int],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_float",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_float,
                float,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_float",
                TestStructAllThriftPrimitiveTypesAbstract.optional_float,
                typing.Optional[float],
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.unqualified_double",
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_double,
                float,
            ),
            (
                "TestStructAllThriftPrimitiveTypesAbstract.optional_double",
                TestStructAllThriftPrimitiveTypesAbstract.optional_double,
                typing.Optional[float],
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_bool",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_bool,
                bool,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_byte",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_byte,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i16",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i16,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i32",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i32,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i64",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i64,
                int,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_float",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_float,
                float,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_double",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_double,
                float,
            ),
            (
                "TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_string",
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_string,
                str,
            ),
            (
                "TestStructAllThriftContainerTypesAbstract.unqualified_list_i32",
                TestStructAllThriftContainerTypesAbstract.unqualified_list_i32,
                typing.Sequence[int],
            ),
            (
                "TestStructAllThriftContainerTypesAbstract.optional_list_i32",
                TestStructAllThriftContainerTypesAbstract.optional_list_i32,
                typing.Optional[typing.Sequence[int]],
            ),
            (
                "TestStructAllThriftContainerTypesAbstract.unqualified_set_string",
                TestStructAllThriftContainerTypesAbstract.unqualified_set_string,
                typing.AbstractSet[str],
            ),
            (
                "TestStructAllThriftContainerTypesAbstract.optional_set_string",
                TestStructAllThriftContainerTypesAbstract.optional_set_string,
                typing.Optional[typing.AbstractSet[str]],
            ),
            (
                "TestStructAllThriftContainerTypesAbstract.unqualified_map_string_i32",
                TestStructAllThriftContainerTypesAbstract.unqualified_map_string_i32,
                typing.Mapping[str, int],
            ),
            (
                "TestStructAllThriftContainerTypesAbstract.optional_map_string_i32",
                TestStructAllThriftContainerTypesAbstract.optional_map_string_i32,
                typing.Optional[typing.Mapping[str, int]],
            ),
            (
                "TestStructWithTypedefFieldAbstract.empty_struct",
                TestStructWithTypedefFieldAbstract.empty_struct,
                TestStructEmptyAbstract,
            ),
            (
                "TestStructWithTypedefFieldAbstract.empty_struct_alias",
                TestStructWithTypedefFieldAbstract.empty_struct_alias,
                TestStructEmptyAbstract,
            ),
            (
                "TestStructNested_0Abstract.nested_1",
                TestStructNested_0Abstract.nested_1,
                TestStructNested_1Abstract,
            ),
            (
                "TestStructNested_1Abstract.nested_2",
                TestStructNested_1Abstract.nested_2,
                TestStructNested_2Abstract,
            ),
            (
                "TestStructWithUnionFieldAbstract.union_field",
                TestStructWithUnionFieldAbstract.union_field,
                thrift.test.thrift_python.struct_test.thrift_abstract_types.TestUnion,
            ),
            (
                "TestStructWithUnionFieldAbstract.union_field_from_included",
                TestStructWithUnionFieldAbstract.union_field_from_included,
                thrift.test.thrift_python.included.thrift_abstract_types.TestUnion,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_bool",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_bool,
                bool,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_bool",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_bool,
                typing.Optional[bool],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_byte",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_byte,
                int,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_byte",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_byte,
                typing.Optional[int],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i16",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i16,
                int,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_i16",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_i16,
                typing.Optional[int],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i32",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i32,
                int,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_i32",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_i32,
                typing.Optional[int],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i64",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i64,
                int,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_i64",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_i64,
                typing.Optional[int],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_float",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_float,
                float,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_float",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_float,
                typing.Optional[float],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_double",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_double,
                float,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_double",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_double,
                typing.Optional[float],
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_string",
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_string,
                str,
            ),
            (
                "TestExceptionAllThriftPrimitiveTypesAbstract.optional_string",
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_string,
                typing.Optional[str],
            ),
            (
                "TestStructWithExceptionFieldAbstract.i32_field",
                TestStructWithExceptionFieldAbstract.i32_field,
                int,
            ),
            (
                "TestStructWithExceptionFieldAbstract.exception_field",
                TestStructWithExceptionFieldAbstract.exception_field,
                TestExceptionAllThriftPrimitiveTypesAbstract,
            ),
            (
                "TestStructCopyAbstract.unqualified_i32",
                TestStructCopyAbstract.unqualified_i32,
                int,
            ),
            (
                "TestStructCopyAbstract.optional_i32",
                TestStructCopyAbstract.optional_i32,
                typing.Optional[int],
            ),
            (
                "TestStructCopyAbstract.unqualified_string",
                TestStructCopyAbstract.unqualified_string,
                str,
            ),
            (
                "TestStructCopyAbstract.optional_string",
                TestStructCopyAbstract.optional_string,
                typing.Optional[str],
            ),
            (
                "TestStructCopyAbstract.unqualified_list_i32",
                TestStructCopyAbstract.unqualified_list_i32,
                typing.Sequence[int],
            ),
            (
                "TestStructCopyAbstract.unqualified_set_string",
                TestStructCopyAbstract.unqualified_set_string,
                typing.AbstractSet[str],
            ),
            (
                "TestStructCopyAbstract.unqualified_map_string_i32",
                TestStructCopyAbstract.unqualified_map_string_i32,
                typing.Mapping[str, int],
            ),
            (
                "TestStructCopyAbstract.recursive_struct",
                TestStructCopyAbstract.recursive_struct,
                typing.Optional[TestStructCopyAbstract],
            ),
            (
                "TestStructCopyAbstract.unqualified_binary",
                TestStructCopyAbstract.unqualified_binary,
                iobuf.IOBuf,
            ),
            (
                "TestExceptionCopyAbstract.unqualified_i32",
                TestExceptionCopyAbstract.unqualified_i32,
                int,
            ),
            (
                "TestExceptionCopyAbstract.optional_i32",
                TestExceptionCopyAbstract.optional_i32,
                typing.Optional[int],
            ),
            (
                "TestExceptionCopyAbstract.unqualified_string",
                TestExceptionCopyAbstract.unqualified_string,
                str,
            ),
            (
                "TestExceptionCopyAbstract.optional_string",
                TestExceptionCopyAbstract.optional_string,
                typing.Optional[str],
            ),
            (
                "TestExceptionCopyAbstract.unqualified_list_i32",
                TestExceptionCopyAbstract.unqualified_list_i32,
                typing.Sequence[int],
            ),
            (
                "TestExceptionCopyAbstract.optional_list_i32",
                TestExceptionCopyAbstract.optional_list_i32,
                typing.Optional[typing.Sequence[int]],
            ),
            (
                "TestExceptionCopyAbstract.unqualified_set_string",
                TestExceptionCopyAbstract.unqualified_set_string,
                typing.AbstractSet[str],
            ),
            (
                "TestExceptionCopyAbstract.optional_set_string",
                TestExceptionCopyAbstract.optional_set_string,
                typing.Optional[typing.AbstractSet[str]],
            ),
            (
                "TestExceptionCopyAbstract.unqualified_map_string_i32",
                TestExceptionCopyAbstract.unqualified_map_string_i32,
                typing.Mapping[str, int],
            ),
            (
                "TestExceptionCopyAbstract.recursive_exception",
                TestExceptionCopyAbstract.recursive_exception,
                typing.Optional[TestExceptionCopyAbstract],
            ),
            (
                "TestUnionAbstract.string_field",
                TestUnionAbstract.string_field,
                str,
            ),
            (
                "TestUnionAbstract.int_field",
                TestUnionAbstract.int_field,
                int,
            ),
            (
                "TestUnionAbstract.struct_field",
                TestUnionAbstract.struct_field,
                thrift.test.thrift_python.union_test.thrift_abstract_types.TestStruct,
            ),
            (
                "TestUnionAmbiguousTypeFieldNameAbstract.Type",
                TestUnionAmbiguousTypeFieldNameAbstract.Type,
                int,
            ),
        ]
    )
    def test_property_type_hints(
        self,
        test_name: str,
        # pyre-ignore[2]: Ignore types for tests.
        interface_property,
        # pyre-ignore[2]: Ignore types for tests.
        expected_type,
    ) -> None:
        # GIVEN
        expected = expected_type

        # WHEN
        actual = self._get_property_type(
            interface_property,
        )

        # THEN
        self.assertEqual(expected, actual)

    def test_typedef(self) -> None:
        # GIVEN
        # typedef TestStructEmpty TestStructEmptyAlias
        expected = TestStructEmptyAbstract

        # WHEN
        actual = TestStructEmptyAliasAbstract

        # THEN
        self.assertIs(expected, actual)

    @parameterized.expand(
        [
            (
                # provide a test name for clarity, especially with failures.
                "immutable_struct",
                TestStructImmutable(unqualified_string="hello"),
            ),
            (
                "mutable_struct",
                TestStructMutable(unqualified_string="hello"),
            ),
        ],
    )
    def test_iteration(self, test_name: str, ts: TestStructAbstract) -> None:
        # Iterating over an instance yields (field_name, field_value) tuples.
        self.assertSetEqual(
            # If TestStructAbstract does not have an __iter__ method,
            # then the line below will fail with the error:
            # Incompatible parameter type [6]: In call `set.__init__`,
            # for 1st positional argument, expected `Iterable[Variable[_T]]`
            # but got `TestStructAbstract`
            set(ts),
            {("unqualified_string", "hello"), ("optional_string", None)},
        )

    # Construct immutable and mutable structs to ensure that
    # the type-check for this file in isolation checks the ability to construct
    # these types in the face of abstract methods in the abstract base class.
    @parameterized.expand(
        [
            (
                "immutable_struct",
                TestStructImmutable(),
            ),
            (
                "mutable_struct",
                TestStructMutable(),
            ),
        ],
    )
    def test_structs_to_immutable_python(
        self, test_name: str, ts: TestStructAbstract
    ) -> None:
        # If abstract class does not have a _to_python method,
        # then the line below will fail with the error:
        #   Undefined attribute [16]: `TestStructAbstract` has no attribute `_to_python`.
        ts._to_python()

    # Construct immutable and mutable unions to ensure that
    # the type-check for this file in isolation checks the ability to construct
    # these types in the face of abstract methods in the abstract base class.
    @parameterized.expand(
        [
            (
                "immutable_union",
                TestUnionImmutable(),
            ),
            (
                "mutable_union",
                TestUnionMutable(),
            ),
        ],
    )
    def test_unions_to_immutable_python(
        self, test_name: str, tu: TestUnionAbstract
    ) -> None:
        # If abstract class does not have a _to_python method,
        # then the line below will fail with the error:
        #   Undefined attribute [16]: `TestUnionAbstract` has no attribute `_to_python`.
        tu._to_python()

    # Construct immutable and mutable structs to ensure that
    # the type-check for this file in isolation checks the ability to construct
    # these types in the face of abstract methods in the abstract base class.
    @parameterized.expand(
        [
            (
                "immutable_struct",
                TestStructImmutable(),
            ),
            (
                "mutable_struct",
                TestStructMutable(),
            ),
        ],
    )
    def test_structs_to_mutable_python(
        self, test_name: str, ts: TestStructAbstract
    ) -> None:
        # If abstract class does not have a _to_mutable_python method,
        # then the line below will fail with the error:
        #   Undefined attribute [16]: `TestStructAbstract` has no attribute `_to_mutable_python`.
        ts._to_mutable_python()

    # Construct immutable and mutable unions to ensure that
    # the type-check for this file in isolation checks the ability to construct
    # these types in the face of abstract methods in the abstract base class.
    @parameterized.expand(
        [
            (
                "immutable_union",
                TestUnionImmutable(),
            ),
            (
                "mutable_union",
                TestUnionMutable(),
            ),
        ],
    )
    def test_unions_to_mutable_python(
        self, test_name: str, tu: TestUnionAbstract
    ) -> None:
        # If abstract class does not have a _to_mutable_python method,
        # then the line below will fail with the error:
        #   Undefined attribute [16]: `TestUnionAbstract` has no attribute `_to_mutable_python`.
        tu._to_mutable_python()

    def test_to_py3(self) -> None:
        # Rather than pull in py3 types, just check that the method exists.
        self.assertTrue(hasattr(TestStructAbstract, "_to_py3"))
        self.assertTrue(hasattr(TestUnionAbstract, "_to_py3"))

    def test_to_py_deprecated(self) -> None:
        # Rather than pull in py-deprecated types, just check that the method exists.
        self.assertTrue(hasattr(TestStructAbstract, "_to_py_deprecated"))
        self.assertTrue(hasattr(TestUnionAbstract, "_to_py_deprecated"))

    @parameterized.expand(
        [
            (
                "immutable_union",
                TestUnionImmutable(),
                TestUnionImmutable(string_field="Hello, world!"),
            ),
            (
                "mutable_union",
                TestUnionMutable(),
                TestUnionMutable(string_field="Hello, world!"),
            ),
        ],
    )
    def test_create_union(
        self,
        test_name: str,
        empty_union: TestUnionAbstract,
        string_field_union: TestUnionAbstract,
    ) -> None:
        self.assertEqual(
            empty_union.fbthrift_current_field,
            TestUnionAbstract.FbThriftUnionFieldEnum.EMPTY,
        )
        self.assertIsNone(empty_union.fbthrift_current_value)
        with self.assertRaises(AttributeError):
            empty_union.string_field

        self.assertIs(
            string_field_union.fbthrift_current_field,
            TestUnionMutable.FbThriftUnionFieldEnum.string_field,
        )
        self.assertEqual(string_field_union.fbthrift_current_value, "Hello, world!")
        self.assertEqual(string_field_union.string_field, "Hello, world!")
        # Trying to access any other field should raise an error.
        with self.assertRaises(
            AttributeError,
        ):
            string_field_union.int_field
