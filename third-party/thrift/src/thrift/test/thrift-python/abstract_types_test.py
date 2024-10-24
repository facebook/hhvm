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

from thrift.test.thrift_python.struct_test.thrift_abstract_types import (  # @manual=//thrift/test/thrift-python:struct_test_thrift-python-types
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesAbstract,
    TestExceptionCopy as TestExceptionCopyAbstract,
    TestStruct as TestStructAbstract,
    # TODO: Uncomment when adapted types work correctly.
    # TestStructAdaptedTypes as TestStructAdaptedTypesAbstract,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesAbstract,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesAbstract,
    TestStructAllThriftPrimitiveTypesWithDefaultValues as TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract,
    TestStructCopy as TestStructCopyAbstract,
    TestStructEmpty as TestStructEmptyAbstract,
    TestStructEmptyAlias as TestStructEmptyAliasAbstract,
    TestStructNested_0 as TestStructNested_0Abstract,
    TestStructNested_1 as TestStructNested_1Abstract,
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


class ThriftPythonAbstractTypesTest(unittest.TestCase):
    @staticmethod
    # pyre-ignore[2]
    # pyre-ignore[3]
    def _get_property_type(property_descriptor) -> typing.Type[typing.Any]:
        import inspect

        signature = inspect.signature(property_descriptor.fget)

        # TODO(T204911681): This mechanism should return the correct type.
        # However, it generates the error "typing.Final[int] is not a valid type argument."
        # type_hints = typing.get_type_hints(property_descriptor.fget)

        # return type_hints["return"]

        return signature.return_annotation

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
                TestStructWithDefaultValuesAbstract.unqualified_integer,
                # For some reason, these appear as strings, which
                # indicates a forward reference, and not as
                # the type.
                # For now this satisfies the needs of this test.
                "int",
            ),
            (
                TestStructWithDefaultValuesAbstract.optional_integer,
                "_typing.Optional[int]",
            ),
            (
                TestStructWithDefaultValuesAbstract.unqualified_struct,
                "TestStruct",
            ),
            (
                TestStructWithDefaultValuesAbstract.optional_struct,
                "_typing.Optional[TestStruct]",
            ),
            (
                TestStructWithDefaultValuesAbstract.unqualified_struct_intrinsic_default,
                "TestStruct",
            ),
            (
                TestStructWithDefaultValuesAbstract.optional_struct_intrinsic_default,
                "_typing.Optional[TestStruct]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_bool,
                "bool",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_bool,
                "_typing.Optional[bool]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_byte,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_byte,
                "_typing.Optional[int]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_i16,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_i16,
                "_typing.Optional[int]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_i32,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_i32,
                "_typing.Optional[int]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_i64,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_i64,
                "_typing.Optional[int]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_float,
                "float",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_float,
                "_typing.Optional[float]",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.unqualified_double,
                "float",
            ),
            (
                TestStructAllThriftPrimitiveTypesAbstract.optional_double,
                "_typing.Optional[float]",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_bool,
                "bool",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_byte,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i16,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i32,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_i64,
                "int",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_float,
                "float",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_double,
                "float",
            ),
            (
                TestStructAllThriftPrimitiveTypesWithDefaultValuesAbstract.unqualified_string,
                "str",
            ),
            (
                TestStructAllThriftContainerTypesAbstract.unqualified_list_i32,
                "_typing.Sequence[int]",
            ),
            (
                TestStructAllThriftContainerTypesAbstract.optional_list_i32,
                "_typing.Optional[_typing.Sequence[int]]",
            ),
            (
                TestStructAllThriftContainerTypesAbstract.unqualified_set_string,
                "_typing.AbstractSet[str]",
            ),
            (
                TestStructAllThriftContainerTypesAbstract.optional_set_string,
                "_typing.Optional[_typing.AbstractSet[str]]",
            ),
            (
                TestStructAllThriftContainerTypesAbstract.unqualified_map_string_i32,
                "_typing.Mapping[str, int]",
            ),
            (
                TestStructAllThriftContainerTypesAbstract.optional_map_string_i32,
                "_typing.Optional[_typing.Mapping[str, int]]",
            ),
            (
                TestStructWithTypedefFieldAbstract.empty_struct,
                "TestStructEmpty",
            ),
            (
                TestStructWithTypedefFieldAbstract.empty_struct_alias,
                "TestStructEmpty",
            ),
            (
                TestStructNested_0Abstract.nested_1,
                "TestStructNested_1",
            ),
            (
                TestStructNested_1Abstract.nested_2,
                "TestStructNested_2",
            ),
            (
                TestStructWithUnionFieldAbstract.union_field,
                "TestUnion",
            ),
            (
                TestStructWithUnionFieldAbstract.union_field_from_included,
                "thrift.test.thrift_python.included.thrift_abstract_types.TestUnion",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_bool,
                "bool",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_bool,
                "_typing.Optional[bool]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_byte,
                "int",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_byte,
                "_typing.Optional[int]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i16,
                "int",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_i16,
                "_typing.Optional[int]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i32,
                "int",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_i32,
                "_typing.Optional[int]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_i64,
                "int",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_i64,
                "_typing.Optional[int]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_float,
                "float",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_float,
                "_typing.Optional[float]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_double,
                "float",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_double,
                "_typing.Optional[float]",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.unqualified_string,
                "str",
            ),
            (
                TestExceptionAllThriftPrimitiveTypesAbstract.optional_string,
                "_typing.Optional[str]",
            ),
            (
                TestStructWithExceptionFieldAbstract.i32_field,
                "int",
            ),
            (
                TestStructWithExceptionFieldAbstract.exception_field,
                "TestExceptionAllThriftPrimitiveTypes",
            ),
            (
                TestStructCopyAbstract.unqualified_i32,
                "int",
            ),
            (
                TestStructCopyAbstract.optional_i32,
                "_typing.Optional[int]",
            ),
            (
                TestStructCopyAbstract.unqualified_string,
                "str",
            ),
            (
                TestStructCopyAbstract.optional_string,
                "_typing.Optional[str]",
            ),
            (
                TestStructCopyAbstract.unqualified_list_i32,
                "_typing.Sequence[int]",
            ),
            (
                TestStructCopyAbstract.unqualified_set_string,
                "_typing.AbstractSet[str]",
            ),
            (
                TestStructCopyAbstract.unqualified_map_string_i32,
                "_typing.Mapping[str, int]",
            ),
            (
                TestStructCopyAbstract.recursive_struct,
                "_typing.Optional[TestStructCopy]",
            ),
            (
                TestStructCopyAbstract.unqualified_binary,
                "_fbthrift_iobuf.IOBuf",
            ),
            (
                TestExceptionCopyAbstract.unqualified_i32,
                "int",
            ),
            (
                TestExceptionCopyAbstract.optional_i32,
                "_typing.Optional[int]",
            ),
            (
                TestExceptionCopyAbstract.unqualified_string,
                "str",
            ),
            (
                TestExceptionCopyAbstract.optional_string,
                "_typing.Optional[str]",
            ),
            (
                TestExceptionCopyAbstract.unqualified_list_i32,
                "_typing.Sequence[int]",
            ),
            (
                TestExceptionCopyAbstract.optional_list_i32,
                "_typing.Optional[_typing.Sequence[int]]",
            ),
            (
                TestExceptionCopyAbstract.unqualified_set_string,
                "_typing.AbstractSet[str]",
            ),
            (
                TestExceptionCopyAbstract.optional_set_string,
                "_typing.Optional[_typing.AbstractSet[str]]",
            ),
            (
                TestExceptionCopyAbstract.unqualified_map_string_i32,
                "_typing.Mapping[str, int]",
            ),
            (
                TestExceptionCopyAbstract.recursive_exception,
                "_typing.Optional[TestExceptionCopy]",
            ),
            (
                TestUnionAbstract.string_field,
                "str",
            ),
            (
                TestUnionAbstract.int_field,
                "int",
            ),
            (
                TestUnionAbstract.struct_field,
                "TestStruct",
            ),
            (TestUnionAmbiguousTypeFieldNameAbstract.Type, "int"),
        ]
    )
    # pyre-ignore[2]: Ignore types for tests.
    def test_property_type_hints(self, interface_property, expected_type) -> None:
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
