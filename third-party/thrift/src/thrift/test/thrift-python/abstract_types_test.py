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

import thrift.test.thrift_python.enum_test.thrift_abstract_types as enum_test_abstract_types

import thrift.test.thrift_python.enum_test.thrift_enums as enum_test_enums

import thrift.test.thrift_python.included.thrift_abstract_types

from parameterized import parameterized
from thrift.python.abstract_types import AbstractGeneratedError

from thrift.python.exceptions import Error, GeneratedError
from thrift.python.mutable_exceptions import MutableGeneratedError

from thrift.test.thrift_python.struct_test.thrift_abstract_types import (  # @manual=//thrift/test/thrift-python:struct_test_thrift-python-types
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesAbstract,
    TestExceptionCopy as TestExceptionCopyAbstract,
    TestStruct as TestStructAbstract,
    TestStructAdaptedTypes as TestStructAdaptedTypesAbstract,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesAbstract,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesAbstract,
    TestStructCopy as TestStructCopyAbstract,
    TestStructEmpty as TestStructEmptyAbstract,
    TestStructEmptyAlias as TestStructEmptyAliasAbstract,
    TestStructNested_0 as TestStructNested_0Abstract,
    TestStructNested_1 as TestStructNested_1Abstract,
    TestStructNested_2 as TestStructNested_2Abstract,
    TestStructWithExceptionField as TestStructWithExceptionFieldAbstract,
    TestStructWithInvariantField as TestStructWithInvariantFieldAbstract,
    TestStructWithNestedContainers as TestStructWithNestedContainersAbstract,
    TestStructWithTypedefField as TestStructWithTypedefFieldAbstract,
    TestStructWithUnionField as TestStructWithUnionFieldAbstract,
)

from thrift.test.thrift_python.struct_test.thrift_mutable_types import (
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesMutable,
    TestExceptionCopy as TestExceptionCopyMutable,
    TestStruct as TestStructMutable,
    TestStructAdaptedTypes as TestStructAdaptedTypesMutable,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesMutable,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesMutable,
    TestStructCopy as TestStructCopyMutable,
    TestStructNested_0 as TestStructNested_0Mutable,
    TestStructNested_1 as TestStructNested_1Mutable,
    TestStructWithExceptionField as TestStructWithExceptionFieldMutable,
    TestStructWithInvariantField as TestStructWithInvariantFieldMutable,
    TestStructWithNestedContainers as TestStructWithNestedContainersMutable,
    TestStructWithTypedefField as TestStructWithTypedefFieldMutable,
    TestStructWithUnionField as TestStructWithUnionFieldMutable,
)

from thrift.test.thrift_python.struct_test.thrift_types import (
    TestExceptionAllThriftPrimitiveTypes as TestExceptionAllThriftPrimitiveTypesImmutable,
    TestExceptionCopy as TestExceptionCopyImmutable,
    TestStruct as TestStructImmutable,
    TestStructAdaptedTypes as TestStructAdaptedTypesImmutable,
    TestStructAllThriftContainerTypes as TestStructAllThriftContainerTypesImmutable,
    TestStructAllThriftPrimitiveTypes as TestStructAllThriftPrimitiveTypesImmutable,
    TestStructCopy as TestStructCopyImmutable,
    TestStructNested_0 as TestStructNested_0Immutable,
    TestStructNested_1 as TestStructNested_1Immutable,
    TestStructWithExceptionField as TestStructWithExceptionFieldImmutable,
    TestStructWithInvariantField as TestStructWithInvariantFieldImmutable,
    TestStructWithNestedContainers as TestStructWithNestedContainersImmutable,
    TestStructWithTypedefField as TestStructWithTypedefFieldImmutable,
    TestStructWithUnionField as TestStructWithUnionFieldImmutable,
)

from thrift.test.thrift_python.union_test.thrift_abstract_types import (  # @manual=//thrift/test/thrift-python:union_test_thrift-python-types
    TestUnion as TestUnionAbstract,
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
    def _get_property_type(property_descriptor) -> typing.Type[typing.Any]:
        type_hints = typing.get_type_hints(property_descriptor.fget)

        return type_hints["return"]

    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_fn_call_with_read_only_abstract_base_class_with_immutable_struct(
        self,
    ) -> None:
        """
        type-check will fail with the error below if
        TestStructImmutable does not inherit from TestStructAbstract.

        Incompatible parameter type [6]: In call `ThriftPythonAbstractTypesTest.test_fn_call_with_read_only_abstract_base_class_with_immutable.library_fn`, for 1st positional argument, expected `TestStructAbstract` but got `TestStructImmutable`.
        """

        # GIVEN
        def library_fn(ts: TestStructAbstract) -> None:
            # THEN
            self.assertIsInstance(ts, TestStructAbstract)

        # WHEN
        library_fn(TestStructImmutable(unqualified_string="hello"))
        self.assertTrue(issubclass(TestStructImmutable, TestStructAbstract))

    def test_fn_call_with_read_only_abstract_base_class_with_mutable_struct(
        self,
    ) -> None:
        """
        type-check will fail with the error below if
        TestStructMutable does not inherit from TestStructAbstract.

        Incompatible parameter type [6]: In call `ThriftPythonAbstractTypesTest.test_fn_call_with_read_only_abstract_base_class_with_mutable.library_fn`, for 1st positional argument, expected `TestStructAbstract` but got `TestStructMutable`.
        """

        # GIVEN
        def library_fn(ts: TestStructAbstract) -> None:
            # THEN
            self.assertIsInstance(ts, TestStructAbstract)

        # WHEN
        library_fn(TestStructMutable(unqualified_string="hello"))
        self.assertTrue(issubclass(TestStructMutable, TestStructAbstract))

    def test_fn_call_with_read_only_abstract_base_class_with_immutable_union(
        self,
    ) -> None:
        """
        type-check will fail with the error below if
        TestUnionImmutable does not inherit from TestUnionAbstract.

        Incompatible parameter type [6]: In call `ThriftPythonAbstractTypesTest.test_fn_call_with_read_only_abstract_base_class_with_immutable.library_fn`, for 1st positional argument, expected `TestUnionAbstract` but got `TestUnionImmutable`.
        """

        # GIVEN
        def library_fn(ts: TestUnionAbstract) -> None:
            # THEN
            self.assertIsInstance(ts, TestUnionAbstract)

        # WHEN
        library_fn(TestUnionImmutable(string_field="hello"))
        self.assertTrue(issubclass(TestUnionImmutable, TestUnionAbstract))

    def test_union_current_field_immutable(self) -> None:
        def make_abstract(u: TestUnionAbstract) -> TestUnionAbstract:
            return u

        # GIVEN
        u = make_abstract(TestUnionImmutable(string_field="hello"))
        # This fixme is required due to the Pyre limitation that it does not recognize nested
        # type aliases as types for type annotations.
        # pyre-fixme[9]: expected has type `TestUnionAbstract.FbThriftUnionFieldEnum`; used as
        #  `TestUnionImmutable.FbThriftUnionFieldEnum`.
        expected: TestUnionAbstract.FbThriftUnionFieldEnum = (
            TestUnionImmutable.FbThriftUnionFieldEnum.string_field
        )

        # WHEN
        actual: TestUnionAbstract.FbThriftUnionFieldEnum = u.fbthrift_current_field
        self.assertEqual(expected, actual)

    def test_fn_call_with_read_only_abstract_base_class_with_mutable_union(
        self,
    ) -> None:
        """
        type-check will fail with the error below if
        TestUnionMutable does not inherit from TestUnionAbstract.

        Incompatible parameter type [6]: In call `ThriftPythonAbstractTypesTest.test_fn_call_with_read_only_abstract_base_class_with_mutable.library_fn`, for 1st positional argument, expected `TestUnionAbstract` but got `TestUnionMutable`.
        """

        # GIVEN
        def library_fn(ts: TestUnionAbstract) -> None:
            # THEN
            self.assertIsInstance(ts, TestUnionAbstract)

        # WHEN
        library_fn(TestUnionMutable(string_field="hello"))
        self.assertTrue(issubclass(TestUnionMutable, TestUnionAbstract))

    def test_union_current_field_mutable(self) -> None:
        def make_abstract(u: TestUnionAbstract) -> TestUnionAbstract:
            return u

        # GIVEN
        u = make_abstract(TestUnionMutable(string_field="hello"))
        # This fixme is required due to the Pyre limitation that it does not recognize nested
        # type aliases as types for type annotations.
        # pyre-fixme[9]: expected has type `TestUnionAbstract.FbThriftUnionFieldEnum`; used as
        #  `TestUnionMutable.FbThriftUnionFieldEnum`.
        expected: TestUnionAbstract.FbThriftUnionFieldEnum = (
            TestUnionMutable.FbThriftUnionFieldEnum.string_field
        )

        # WHEN
        actual: TestUnionAbstract.FbThriftUnionFieldEnum = u.fbthrift_current_field
        self.assertEqual(expected, actual)

    @parameterized.expand(
        [
            (
                "raise_immutable_catch_abstract",
                TestExceptionAllThriftPrimitiveTypesImmutable,
                TestExceptionAllThriftPrimitiveTypesAbstract,
            ),
            (
                "raise_immutable_catch_abstract_generated_error",
                TestExceptionAllThriftPrimitiveTypesImmutable,
                AbstractGeneratedError,
            ),
            (
                "raise_immutable_catch_generated_error",
                TestExceptionAllThriftPrimitiveTypesImmutable,
                GeneratedError,
            ),
            (
                "raise_immutable_catch_error",
                TestExceptionAllThriftPrimitiveTypesImmutable,
                Error,
            ),
            (
                "raise_mutable_catch_abstract",
                TestExceptionAllThriftPrimitiveTypesMutable,
                TestExceptionAllThriftPrimitiveTypesAbstract,
            ),
            (
                "raise_mutable_catch_abstract_generated_error",
                TestExceptionAllThriftPrimitiveTypesMutable,
                AbstractGeneratedError,
            ),
            (
                "raise_mutable_catch_generated_error",
                TestExceptionAllThriftPrimitiveTypesMutable,
                MutableGeneratedError,
            ),
            (
                "raise_mutable_catch_error",
                TestExceptionAllThriftPrimitiveTypesMutable,
                Error,
            ),
        ]
    )
    def test_validate_that_exception_to_raise_derives_from_exception_to_catch(
        self,
        test_name: str,
        exception_to_raise: typing.Type[TestExceptionAllThriftPrimitiveTypesAbstract],
        exception_to_catch: typing.Type[Error],
    ) -> None:
        """
        This test checks that the raised exception (exception_to_raise) derives from the exception to catch (exception_to_catch).
        """
        try:
            raise exception_to_raise
        except exception_to_catch as ex:
            self.assertEqual(
                len(ex.args),
                16,
            )
        except Exception as ex:
            self.fail(f"{type(ex)} does not derive from {exception_to_catch}")

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

    @parameterized.expand(
        [
            (
                # provide a test name for clarity, especially with failures.
                "immutable_struct",
                TestStructWithInvariantFieldImmutable(unqualified_string="hello"),
            ),
            (
                "mutable_struct",
                TestStructWithInvariantFieldMutable(unqualified_string="hello"),
            ),
        ],
    )
    def test_iteration_with_invariant_field(
        self, test_name: str, ts: TestStructWithInvariantFieldAbstract
    ) -> None:
        self.assertEqual(
            # If the struct has an invariant field, `__iter__` is not generated.
            # TestStructWithInvariantFieldAbstract does not have an `__iter__`
            # method, so the line below will fail and needs a pyre-fixme.
            # pyre-fixme[6]: For 1st argument expected `Iterable[_T]` but got
            #  `TestStructWithInvariantFieldAbstract`.
            sorted(ts),
            [
                ("unqualified_i32", 0),
                ("unqualified_map_struct_i32", {}),
                ("unqualified_string", "hello"),
            ],
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

    def test_enum_identity(self) -> None:
        self.assertIs(
            enum_test_enums.PositiveNumber, enum_test_abstract_types.PositiveNumber
        )

    def test_enum_value(self) -> None:
        self.assertEqual(
            enum_test_enums.PositiveNumber.THREE,
            enum_test_abstract_types.PositiveNumber.THREE,
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructAllThriftPrimitiveTypesImmutable(),
            ),
            (
                "mutable",
                TestStructAllThriftPrimitiveTypesMutable(),
            ),
        ]
    )
    def test_all_thrift_primitive_property_types_test(
        self,
        test_case: str,
        test_struct_all_thrift_primitive_types_abstract: TestStructAllThriftPrimitiveTypesAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignoress will flag these as unused.
        # pyre-ignore[9]: unqualified_bool_incorrect_type has type `incorrect_type`;
        #  used as `bool`.
        unqualified_bool_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_bool
        )
        unqualified_bool: bool = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_bool
        )
        # pyre-ignore[9]: optional_bool_incorrect_type has type `incorrect_type`;
        #  used as `Optional[bool]`.
        optional_bool_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_bool
        )
        optional_bool: typing.Optional[bool] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_bool
        )

        # pyre-ignore[9]: unqualified_byte_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_byte_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_byte
        )
        unqualified_byte: int = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_byte
        )

        # pyre-ignore[9]: optional_byte_incorrect_type has type `incorrect_type`;
        #  used as `Optional[int]`.
        optional_byte_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_byte
        )
        optional_byte: typing.Optional[int] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_byte
        )

        # pyre-ignore[9]: unqualified_i16_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_i16_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_i16
        )
        unqualified_i16: int = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_i16
        )

        # pyre-ignore[9]: optional_i16_incorrect_type has type `incorrect_type`;
        #  used as `Optional[int]`.
        optional_i16_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_i16
        )
        optional_i16: typing.Optional[int] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_i16
        )

        # pyre-ignore[9]: unqualified_i32_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_i32
        )
        unqualified_i32: int = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_i32
        )

        # pyre-ignore[9]: optional_i32_incorrect_type has type `incorrect_type`;
        #  used as `Optional[int]`.
        optional_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_i32
        )
        optional_i32: typing.Optional[int] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_i32
        )

        # pyre-ignore[9]: unqualified_i64_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_i64_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_i64
        )
        unqualified_i64: int = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_i64
        )

        # pyre-ignore[9]: optional_i64_incorrect_type has type `incorrect_type`;
        #  used as `Optional[int]`.
        optional_i64_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_i64
        )
        optional_i64: typing.Optional[int] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_i64
        )

        # pyre-ignore[9]: unqualified_float_incorrect_type has type `incorrect_type`;
        #  used as `float`.
        unqualified_float_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_float
        )
        unqualified_float: float = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_float
        )

        # pyre-ignore[9]: optional_float_incorrect_type has type `incorrect_type`;
        #  used as `Optional[float]`.
        optional_float_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_float
        )
        optional_float: typing.Optional[float] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_float
        )

        # pyre-ignore[9]: unqualified_double_incorrect_type has type `incorrect_type`;
        #  used as `float`.
        unqualified_double_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_double
        )
        unqualified_double: float = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_double
        )

        # pyre-ignore[9]: optional_double_incorrect_type has type `incorrect_type`;
        #  used as `Optional[float]`.
        optional_double_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_double
        )
        optional_double: typing.Optional[float] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_double
        )

        # pyre-ignore[9]: unqualified_string_incorrect_type has type `incorrect_type`;
        #  used as `str`.
        unqualified_string_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_string
        )
        unqualified_string: str = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.unqualified_string
        )

        # pyre-ignore[9]: optional_string_incorrect_type has type `incorrect_type`;
        #  used as `Optional[str]`.
        optional_string_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_string
        )
        optional_string: typing.Optional[str] = (  # noqa F841
            test_struct_all_thrift_primitive_types_abstract.optional_string
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructAllThriftContainerTypesImmutable(),
            ),
            (
                "mutable",
                TestStructAllThriftContainerTypesMutable(),
            ),
        ]
    )
    def test_struct_all_thrift_container_property_types_test(
        self,
        test_case: str,
        test_struct_all_thrift_container_types_abstract: TestStructAllThriftContainerTypesAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignoress will flag these as unused.
        # pyre-ignore[9]: unqualified_list_i32_incorrect_type has type
        #  `incorrect_type`; used as `Sequence[int]`.
        unqualified_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.unqualified_list_i32
        )
        unqualified_list_i32: typing.Sequence[int] = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.unqualified_list_i32
        )
        # pyre-ignore[9]: optional_list_i32_incorrect_type has type `incorrect_type`;
        #  used as `Optional[Sequence[int]]`.
        optional_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.optional_list_i32
        )
        optional_list_i32: typing.Optional[typing.Sequence[int]] = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.optional_list_i32
        )

        # pyre-ignore[9]: unqualified_set_string_incorrect_type has type
        #  `incorrect_type`; used as `AbstractSet[str]`.
        unqualified_set_string_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.unqualified_set_string
        )
        unqualified_set_string: typing.AbstractSet[str] = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.unqualified_set_string
        )

        # pyre-ignore[9]: optional_set_string_incorrect_type has type
        #  `incorrect_type`; used as `Optional[AbstractSet[str]]`.
        optional_set_string_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.optional_set_string
        )
        optional_set_string: typing.Optional[typing.AbstractSet[str]] = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.optional_set_string
        )

        # pyre-ignore[9]: unqualified_map_string_i32_incorrect_type has type
        #  `incorrect_type`; used as `Mapping[str, int]`.
        unqualified_map_string_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.unqualified_map_string_i32
        )
        unqualified_map_string_i32: typing.Mapping[str, int] = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.unqualified_map_string_i32
        )

        # pyre-ignore[9]: optional_map_string_i32_incorrect_type has type
        #  `incorrect_type`; used as `Optional[Mapping[str, int]]`.
        optional_map_string_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.optional_map_string_i32
        )
        optional_map_string_i32: typing.Optional[typing.Mapping[str, int]] = (  # noqa F841
            test_struct_all_thrift_container_types_abstract.optional_map_string_i32
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructWithTypedefFieldImmutable(),
            ),
            (
                "mutable",
                TestStructWithTypedefFieldMutable(),
            ),
        ]
    )
    def test_struct_with_typedef_field_property_types_test(
        self,
        test_case: str,
        test_struct_with_typedef_field_abstract: TestStructWithTypedefFieldAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignores will flag these as unused.
        # pyre-ignore[9]: empty_struct_incorrect_type has type `incorrect_type`; used
        #  as `TestStructEmptyAbstract`.
        empty_struct_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_typedef_field_abstract.empty_struct
        )
        empty_struct: TestStructEmptyAbstract = (  # noqa F841
            test_struct_with_typedef_field_abstract.empty_struct
        )

        # pyre-ignore[9]: empty_struct_alias_incorrect_type has type
        #  `incorrect_type`; used as `TestStructEmptyAbstract`.
        empty_struct_alias_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_typedef_field_abstract.empty_struct_alias
        )
        empty_struct_alias: TestStructEmptyAbstract = (  # noqa F841
            test_struct_with_typedef_field_abstract.empty_struct_alias
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructNested_0Immutable(),
                TestStructNested_1Immutable(),
                TestStructCopyImmutable(),
                TestExceptionCopyImmutable(),
            ),
            (
                "mutable",
                TestStructNested_0Mutable(),
                TestStructNested_1Mutable(),
                TestStructCopyMutable(),
                TestExceptionCopyMutable(),
            ),
        ]
    )
    def test_nested_and_recursive_property_types(
        self,
        test_case: str,
        test_struct_nested_0_abstract: TestStructNested_0Abstract,
        test_struct_nested_1_abstract: TestStructNested_1Abstract,
        test_struct_copy_abstract: TestStructCopyAbstract,
        test_exception_copy_abstract: TestExceptionCopyAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignores will flag these as unused.
        # pyre-ignore[9]: nested_0_incorrect_type has type `incorrect_type`; used as
        #  `TestStructNested_1Abstract`.
        nested_0_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_nested_0_abstract.nested_1
        )
        nested_0: TestStructNested_1Abstract = (  # noqa F841
            test_struct_nested_0_abstract.nested_1
        )

        # pyre-ignore[9]: nested_1_incorrect_type has type `incorrect_type`; used as
        #  `TestStructNested_2Abstract`.
        nested_1_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_nested_1_abstract.nested_2
        )
        nested_1: TestStructNested_2Abstract = (  # noqa F841
            test_struct_nested_1_abstract.nested_2
        )

        # pyre-ignore[9]: recursive_struct_incorrect_type has type `incorrect_type`;
        #  used as `Optional[TestStructCopyAbstract]`.
        recursive_struct_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_copy_abstract.recursive_struct
        )
        recursive_struct: typing.Optional[TestStructCopyAbstract] = (  # noqa F841
            test_struct_copy_abstract.recursive_struct
        )

        # pyre-ignore[9]: recursive_exception_incorrect_type has type
        #  `incorrect_type`; used as `Optional[TestExceptionCopyAbstract]`.
        recursive_exception_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_copy_abstract.recursive_exception
        )
        recursive_exception: typing.Optional[TestExceptionCopyAbstract] = (  # noqa F841
            test_exception_copy_abstract.recursive_exception
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructWithNestedContainersImmutable(),
            ),
            (
                "mutable",
                TestStructWithNestedContainersMutable(),
            ),
        ]
    )
    def test_struct_with_nested_containers_property_types_test(
        self,
        test_case: str,
        test_struct_with_nested_containers_abstract: TestStructWithNestedContainersAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignores will flag these as unused.
        # pyre-ignore[9]: list_list_i32_incorrect_type has type `incorrect_type`;
        #  used as `Sequence[Sequence[int]]`.
        list_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_list_i32
        )
        list_list_i32: typing.Sequence[typing.Sequence[int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_list_i32
        )

        # pyre-ignore[9]: list_set_i32_incorrect_type has type `incorrect_type`; used
        #  as `Sequence[AbstractSet[int]]`.
        list_set_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_set_i32
        )
        list_set_i32: typing.Sequence[typing.AbstractSet[int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_set_i32
        )

        # pyre-ignore[9]: list_map_string_i32_incorrect_type has type
        #  `incorrect_type`; used as `Sequence[Mapping[str, int]]`.
        list_map_string_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_map_string_i32
        )
        list_map_string_i32: typing.Sequence[typing.Mapping[str, int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_map_string_i32
        )

        # pyre-ignore[9]: list_map_string_list_i32_incorrect_type has type
        #  `incorrect_type`; used as `Sequence[Mapping[str, Sequence[int]]]`.
        list_map_string_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_map_string_list_i32
        )
        list_map_string_list_i32: typing.Sequence[  # noqa F841
            typing.Mapping[str, typing.Sequence[int]]
        ] = test_struct_with_nested_containers_abstract.list_map_string_list_i32

        # pyre-ignore[9]: list_map_string_set_i32_incorrect_type has type
        #  `incorrect_type`; used as `Sequence[Mapping[str, AbstractSet[int]]]`.
        list_map_string_set_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.list_map_string_set_i32
        )
        list_map_string_set_i32: typing.Sequence[  # noqa F841
            typing.Mapping[str, typing.AbstractSet[int]]
        ] = test_struct_with_nested_containers_abstract.list_map_string_set_i32

        # pyre-ignore[9]: set_list_i32_incorrect_type has type `incorrect_type`; used
        #  as `AbstractSet[Sequence[int]]`.
        set_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.set_list_i32
        )
        set_list_i32: typing.AbstractSet[typing.Sequence[int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.set_list_i32
        )

        # pyre-ignore[9]: set_set_i32_incorrect_type has type `incorrect_type`; used
        #  as `AbstractSet[AbstractSet[int]]`.
        set_set_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.set_set_i32
        )
        set_set_i32: typing.AbstractSet[typing.AbstractSet[int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.set_set_i32
        )

        # pyre-ignore[9]: map_i32_list_i32_incorrect_type has type `incorrect_type`;
        #  used as `Mapping[int, Sequence[int]]`.
        map_i32_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_list_i32
        )
        map_i32_list_i32: typing.Mapping[int, typing.Sequence[int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_list_i32
        )

        # pyre-ignore[9]: map_i32_set_i32_incorrect_type has type `incorrect_type`;
        #  used as `Mapping[int, AbstractSet[int]]`.
        map_i32_set_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_set_i32
        )
        map_i32_set_i32: typing.Mapping[int, typing.AbstractSet[int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_set_i32
        )

        # pyre-ignore[9]: map_i32_map_string_i32_incorrect_type has type
        #  `incorrect_type`; used as `Mapping[int, Mapping[str, int]]`.
        map_i32_map_string_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_map_string_i32
        )
        map_i32_map_string_i32: typing.Mapping[int, typing.Mapping[str, int]] = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_map_string_i32
        )

        # pyre-ignore[9]: map_i32_map_string_list_i32_incorrect_type has type
        #  `incorrect_type`; used as `Mapping[int, Mapping[str, Sequence[int]]]`.
        map_i32_map_string_list_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_map_string_list_i32
        )
        map_i32_map_string_list_i32: typing.Mapping[  # noqa F841
            int, typing.Mapping[str, typing.Sequence[int]]
        ] = test_struct_with_nested_containers_abstract.map_i32_map_string_list_i32

        # pyre-ignore[9]: map_i32_map_string_set_i32_incorrect_type has type
        #  `incorrect_type`; used as `Mapping[int, Mapping[str, AbstractSet[int]]]`.
        map_i32_map_string_set_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.map_i32_map_string_set_i32
        )
        map_i32_map_string_set_i32: typing.Mapping[  # noqa F841
            int, typing.Mapping[str, typing.AbstractSet[int]]
        ] = test_struct_with_nested_containers_abstract.map_i32_map_string_set_i32

        # pyre-ignore[9]: many_nested_incorrect_type has type `incorrect_type`; used
        #  as `Sequence[Sequence[Mapping[int, Sequence[Mapping[str,
        #  Sequence[AbstractSet[int]]]]]]]`.
        many_nested_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_nested_containers_abstract.many_nested
        )
        many_nested: typing.Sequence[  # noqa F841
            typing.Sequence[
                typing.Mapping[
                    int,
                    typing.Sequence[
                        typing.Mapping[str, typing.Sequence[typing.AbstractSet[int]]]
                    ],
                ]
            ]
        ] = test_struct_with_nested_containers_abstract.many_nested

    @parameterized.expand(
        [
            (
                "immutable",
                TestExceptionAllThriftPrimitiveTypesImmutable(),
            ),
            (
                "mutable",
                TestExceptionAllThriftPrimitiveTypesMutable(),
            ),
        ]
    )
    def test_exception_all_thrift_primitive_property_types_test(
        self,
        test_case: str,
        test_exception_all_thrift_primitive_types_abstract: TestExceptionAllThriftPrimitiveTypesAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignores will flag these as unused.
        # pyre-ignore[9]: unqualified_bool_incorrect_type has type `incorrect_type`;
        #  used as `bool`.
        unqualified_bool_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_bool
        )
        unqualified_bool: bool = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_bool
        )
        # pyre-ignore[9]: optional_bool_incorrect_type has type `incorrect_type`;
        #  used as `Optional[bool]`.
        optional_bool_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_bool
        )
        optional_bool: typing.Optional[bool] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_bool
        )

        # pyre-ignore[9]: unqualified_byte_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_byte_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_byte
        )
        unqualified_byte: int = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_byte
        )

        # pyre-ignore[9]: optional_byte_incorrect_type has type `incorrect_type`;
        #  used as `Optional[int]`.
        optional_byte_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_byte
        )
        optional_byte: typing.Optional[int] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_byte
        )

        # pyre-ignore[9]: unqualified_i16_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_i16_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_i16
        )
        unqualified_i16: int = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_i16
        )

        # pyre-ignore[9]: optional_i16_incorrect_type has type `incorrect_type`; used
        #  as `Optional[int]`.
        optional_i16_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_i16
        )
        optional_i16: typing.Optional[int] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_i16
        )

        # pyre-ignore[9]: unqualified_i32_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_i32
        )
        unqualified_i32: int = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_i32
        )

        # pyre-ignore[9]: optional_i32_incorrect_type has type `incorrect_type`; used
        #  as `Optional[int]`.
        optional_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_i32
        )
        optional_i32: typing.Optional[int] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_i32
        )

        # pyre-ignore[9]: unqualified_i64_incorrect_type has type `incorrect_type`;
        #  used as `int`.
        unqualified_i64_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_i64
        )
        unqualified_i64: int = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_i64
        )

        # pyre-ignore[9]: optional_i64_incorrect_type has type `incorrect_type`; used
        #  as `Optional[int]`.
        optional_i64_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_i64
        )
        optional_i64: typing.Optional[int] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_i64
        )

        # pyre-ignore[9]: unqualified_float_incorrect_type has type `incorrect_type`;
        #  used as `float`.
        unqualified_float_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_float
        )
        unqualified_float: float = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_float
        )

        # pyre-ignore[9]: optional_float_incorrect_type has type `incorrect_type`;
        #  used as `Optional[float]`.
        optional_float_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_float
        )
        optional_float: typing.Optional[float] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_float
        )
        # pyre-ignore[9]: unqualified_double_incorrect_type has type
        #  `incorrect_type`; used as `float`.
        unqualified_double_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_double
        )
        unqualified_double: float = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_double
        )

        # pyre-ignore[9]: optional_double_incorrect_type has type `incorrect_type`;
        #  used as `Optional[float]`.
        optional_double_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_double
        )
        optional_double: typing.Optional[float] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_double
        )

        # pyre-ignore[9]: unqualified_string_incorrect_type has type
        #  `incorrect_type`; used as `str`.
        unqualified_string_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_string
        )
        unqualified_string: str = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.unqualified_string
        )

        # pyre-ignore[9]: optional_string_incorrect_type has type `incorrect_type`;
        #  used as `Optional[str]`.
        optional_string_incorrect_type: incorrect_type = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_string
        )
        optional_string: typing.Optional[str] = (  # noqa F841
            test_exception_all_thrift_primitive_types_abstract.optional_string
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructWithExceptionFieldImmutable(),
                TestStructWithUnionFieldImmutable(),
            ),
            (
                "mutable",
                TestStructWithExceptionFieldMutable(),
                TestStructWithUnionFieldMutable(),
            ),
        ]
    )
    def test_struct_with_union_field_property_types_test(
        self,
        test_case: str,
        test_struct_with_exception_field_abstract: TestStructWithExceptionFieldAbstract,
        test_struct_with_union_field_abstract: TestStructWithUnionFieldAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # pyre-ignore[9]: exception_field_incorrect_type has type `incorrect_type`; used
        #  as `TestExceptionAllThriftPrimitiveTypesAbstract`.
        exception_field_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_exception_field_abstract.exception_field
        )
        exception_field: TestExceptionAllThriftPrimitiveTypesAbstract = (  # noqa F841
            test_struct_with_exception_field_abstract.exception_field
        )

        # pyre-ignore[9]: union_field_incorrect_type has type `incorrect_type`; used as
        #  `TestUnion`.
        union_field_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_union_field_abstract.union_field
        )
        union_field: thrift.test.thrift_python.struct_test.thrift_abstract_types.TestUnion = (  # noqa F841
            test_struct_with_union_field_abstract.union_field
        )

        # pyre-ignore[9]: union_field_from_included_incorrect_type has type
        #  `incorrect_type`; used as `TestUnion`.
        union_field_from_included_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_with_union_field_abstract.union_field_from_included
        )
        union_field_from_included: thrift.test.thrift_python.included.thrift_abstract_types.TestUnion = (  # noqa F841
            test_struct_with_union_field_abstract.union_field_from_included
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestUnionImmutable(string_field="test"),
                TestUnionImmutable(int_field=42),
                TestUnionImmutable(
                    struct_field=thrift.test.thrift_python.union_test.thrift_types.TestStruct()
                ),
            ),
            (
                "mutable",
                TestUnionMutable(string_field="test"),
                TestUnionMutable(int_field=42),
                TestUnionMutable(
                    struct_field=thrift.test.thrift_python.union_test.thrift_mutable_types.TestStruct()
                ),
            ),
        ]
    )
    def test_union_fields(
        self,
        test_name: str,
        string_union: TestUnionAbstract,
        int_union: TestUnionAbstract,
        struct_union: TestUnionAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # Test string field
        self.assertEqual(
            string_union.fbthrift_current_field,
            TestUnionAbstract.FbThriftUnionFieldEnum.string_field,
        )
        # pyre-ignore[9]: string_field_incorrect_type has type `incorrect_type`; used as
        #  `str`.
        string_field_incorrect_type: incorrect_type = (  # noqa F841
            string_union.string_field
        )
        string_field: str = (  # noqa F841
            string_union.string_field
        )

        # Test int field
        self.assertEqual(
            int_union.fbthrift_current_field,
            TestUnionAbstract.FbThriftUnionFieldEnum.int_field,
        )
        # pyre-ignore[9]: int_field_incorrect_type has type `incorrect_type`; used as `int`.
        int_field_incorrect_type: incorrect_type = (  # noqa F841
            int_union.int_field
        )
        int_field: int = (  # noqa F841
            int_union.int_field
        )

        # Test struct field
        self.assertEqual(
            struct_union.fbthrift_current_field,
            TestUnionAbstract.FbThriftUnionFieldEnum.struct_field,
        )
        # pyre-ignore[9]: struct_field_incorrect_type has type `incorrect_type`; used as
        #  `TestStruct`.
        struct_field_incorrect_type: incorrect_type = (  # noqa F841
            struct_union.struct_field
        )
        struct_field: thrift.test.thrift_python.union_test.thrift_abstract_types.TestStruct = (  # noqa F841
            struct_union.struct_field
        )

    @parameterized.expand(
        [
            (
                "immutable",
                TestStructAdaptedTypesImmutable(
                    unqualified_adapted_i32_to_datetime=datetime.datetime(
                        day=1, month=1, year=2025
                    ),
                    optional_adapted_i32_to_datetime=datetime.datetime(
                        day=1, month=1, year=2025
                    ),
                    unqualified_adapted_string_to_i32=42,
                ),
            ),
            (
                "mutable",
                TestStructAdaptedTypesMutable(
                    unqualified_adapted_i32_to_datetime=datetime.datetime(
                        day=1, month=1, year=2025
                    ),
                    optional_adapted_i32_to_datetime=datetime.datetime(
                        day=1, month=1, year=2025
                    ),
                    unqualified_adapted_string_to_i32=42,
                ),
            ),
        ]
    )
    def test_adapted_types(
        self,
        test_case: str,
        test_struct_adapted_types: TestStructAdaptedTypesAbstract,
    ) -> None:
        # GIVEN
        class incorrect_type:
            pass

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignoress will flag these as unused.
        # pyre-ignore[9]: unqualified_adapted_i32_to_datetime_incorrect_type has type `incorrect_type`;
        #  used as `datetime`.
        unqualified_adapted_i32_to_datetime_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_adapted_types.unqualified_adapted_i32_to_datetime
        )
        unqualified_adapted_i32_to_datetime: datetime.datetime = (  # noqa F841
            test_struct_adapted_types.unqualified_adapted_i32_to_datetime
        )

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignoress will flag these as unused.
        # pyre-ignore[9]: optional_adapted_i32_to_datetime_incorrect_type has type `incorrect_type`;
        #  used as `Optional[datetime]`.
        optional_adapted_i32_to_datetime_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_adapted_types.optional_adapted_i32_to_datetime
        )
        optional_adapted_i32_to_datetime: typing.Optional[datetime.datetime] = (  # noqa F841
            test_struct_adapted_types.optional_adapted_i32_to_datetime
        )

        # If type-checks don't detect a type mismatch, then the CI check for
        # unused ignoress will flag these as unused.
        # pyre-ignore[9]: unqualified_adapted_string_to_i32_incorrect_type is declared to have type `incorrect_type`
        # but is used as type `int`
        unqualified_adapted_string_to_i32_incorrect_type: incorrect_type = (  # noqa F841
            test_struct_adapted_types.unqualified_adapted_string_to_i32
        )
        unqualified_adapted_string_to_i32: int = (  # noqa F841
            test_struct_adapted_types.unqualified_adapted_string_to_i32
        )
