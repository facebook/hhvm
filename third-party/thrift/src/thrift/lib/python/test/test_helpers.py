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

import math
import struct
import unittest
from typing import Any, Type

import testing.thrift_mutable_types as mutable_test_types
import testing.thrift_types as immutable_test_types
from parameterized import parameterized, parameterized_class
from thrift.python.test_helpers import (
    assert_thrift_almost_equal,
    round_thrift_float32_if_rollout,
    round_thrift_to_float32,
)


def to_float32(val: float) -> float:
    """Helper to convert a float64 value to float32 and back to float64."""
    return struct.unpack("f", struct.pack("f", val))[0]


class RoundToFloat32NonThriftTest(unittest.TestCase):
    @parameterized.expand(
        [
            ("float64_rounded", 1.234567890123456789, True),
            ("integer", 42, True),
            ("large_integer", 2**31 - 1, True),
            ("negative_float", -3.14159265358979, True),
            ("zero", 0, True),
        ]
    )
    def test_numeric_values_converted_to_float32(
        self, _name: str, val: float, should_convert: bool
    ) -> None:
        """Test that numeric values are converted to float32."""
        result = round_thrift_to_float32(val, convert_int=True)
        expected = to_float32(val)
        self.assertEqual(result, expected)
        self.assertIsInstance(result, float)

    @parameterized.expand(
        [
            ("string", "test_string"),
            ("bool_true", True),
            ("bool_false", False),
            ("none", None),
        ]
    )
    def test_non_numeric_scalars_returned_unchanged(
        self, _name: str, val: object
    ) -> None:
        """Test that non-numeric scalar values are returned unchanged."""
        result = round_thrift_to_float32(val)
        if val is None:
            self.assertIsNone(result)
        else:
            self.assertEqual(result, val)

    @parameterized.expand(
        [
            ("list_floats", [1.1, 2.2, 3.3], list),
            ("list_mixed", [1, 2.5, "test", True], list),
            ("tuple_floats", (1.1, 2.2), tuple),
            ("set_integers", {1, 2, 3}, set),
        ]
    )
    def test_containers_with_numeric_values_are_rounded(
        self, _name: str, val: object, expected_type: Type[Any]
    ) -> None:
        """Test that containers with numeric values are rounded to float32."""
        result = round_thrift_to_float32(val)
        expected: Any = None
        if isinstance(val, list):
            expected = [
                to_float32(x) if isinstance(x, (int, float)) else x for x in val
            ]
        elif isinstance(val, tuple):
            expected = tuple([to_float32(x) for x in val])
        elif isinstance(val, set):
            expected = {to_float32(x) for x in val}
        self.assertEqual(result, expected)
        self.assertIsInstance(result, expected_type)

    def test_dict_with_numeric_values_is_rounded(self) -> None:
        """Test that a dict with numeric values is rounded to float32."""
        val = {1: 1.234567890123456789, 2: 2.345678901234567890}
        result = round_thrift_to_float32(val)
        expected = {to_float32(k): to_float32(v) for k, v in val.items()}
        self.assertEqual(result, expected)
        self.assertIsInstance(result, dict)

    def test_dict_with_string_keys(self) -> None:
        """Test that a dict with string keys and numeric values is handled correctly."""
        val = {"a": 1.5, "b": 2.5}
        result = round_thrift_to_float32(val)
        expected = {"a": to_float32(1.5), "b": to_float32(2.5)}
        self.assertEqual(result, expected)

    def test_nested_list_is_rounded(self) -> None:
        """Test that nested lists are rounded to float32."""
        val = [[1.1, 2.2], [3.3, 4.4]]
        result = round_thrift_to_float32(val)
        expected = [
            [to_float32(1.1), to_float32(2.2)],
            [to_float32(3.3), to_float32(4.4)],
        ]
        self.assertEqual(result, expected)

    def test_complex_nested_structure(self) -> None:
        """Test that complex nested structures are rounded correctly."""
        val = {
            "list": [1.1, 2.2, {"inner": 3.3}],
            "dict": {"a": [4.4, 5.5], "b": 6.6},
            "float": 7.7,
        }
        result = round_thrift_to_float32(val)
        expected = {
            "list": [
                to_float32(1.1),
                to_float32(2.2),
                {"inner": to_float32(3.3)},
            ],
            "dict": {"a": [to_float32(4.4), to_float32(5.5)], "b": to_float32(6.6)},
            "float": to_float32(7.7),
        }
        self.assertEqual(result, expected)

    @parameterized.expand(
        [
            ("positive_infinity", float("inf"), lambda r: math.isinf(r) and r > 0),
            ("negative_infinity", float("-inf"), lambda r: math.isinf(r) and r < 0),
            ("nan", float("nan"), math.isnan),
        ]
    )
    def test_special_float_values(
        self, _name: str, val: float, validator: object
    ) -> None:
        """Test that special float values like inf, -inf, and nan are handled."""
        result = round_thrift_to_float32(val)
        # pyre-ignore[29]: validator is a callable
        self.assertTrue(validator(result))

    def test_conditional_float32_rounding(
        self,
    ) -> None:
        self.assertNotEqual(round_thrift_to_float32(1.1), 1.1)
        self.assertEqual(round_thrift_float32_if_rollout(1.1), 1.1)


@parameterized_class(
    ("test_types",),
    [
        (immutable_test_types,),
        (mutable_test_types,),
    ],
)
class RoundThriftToFloat32Test(unittest.TestCase):
    def setUp(self) -> None:
        self.test_types: Any = self.test_types
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )

    def test_enum_is_returned_unchanged(self) -> None:
        """Test that Enum values are returned unchanged."""
        val = self.test_types.Color.red
        result = round_thrift_to_float32(val)
        self.assertEqual(result, val)
        self.assertIs(result, val)

    def test_struct_with_numeric_fields_is_rounded(self) -> None:
        """Test that a struct with numeric fields is rounded to float32."""
        val = self.test_types.LatLon(lat=51.4769, lon=0.0005)
        result = round_thrift_to_float32(val)
        self.assertEqual(result.lat, to_float32(51.4769))
        self.assertEqual(result.lon, to_float32(0.0005))
        self.assertIsInstance(result, self.test_types.LatLon)

    def test_struct_with_containers_is_rounded(self) -> None:
        """Test that a struct with container fields is rounded to float32."""
        if self.is_mutable_run:
            return  # mutable containers not yet implemented
        val = self.test_types.numerical(
            int_val=42,
            float_val=3.14159265358979,
            int_list=[1, 2, 3],
            float_list=[1.1, 2.2, 3.3],
            i64_val=1000000,
        )
        result = round_thrift_to_float32(val)
        self.assertEqual(result.int_val, to_float32(42))
        self.assertEqual(result.float_val, to_float32(3.14159265358979))
        self.assertEqual(result.int_list, [to_float32(1), to_float32(2), to_float32(3)])
        self.assertEqual(
            result.float_list, [to_float32(1.1), to_float32(2.2), to_float32(3.3)]
        )
        self.assertEqual(result.i64_val, to_float32(1000000))

    @parameterized.expand(
        [
            ("float_value", "float_val", 3.14159265358979),
            ("double_value", "double_val", 2.718281828459045),
            ("integer_value", "tiny", 42),
        ]
    )
    def test_union_with_numeric_values_is_rounded(
        self, _name: str, field_name: str, value: float
    ) -> None:
        """Test that unions with numeric values are rounded to float32."""
        val = self.test_types.ComplexUnion(**{field_name: value})
        result = round_thrift_to_float32(val)
        expected = to_float32(value)
        if self.is_mutable_run:
            self.assertEqual(result.fbthrift_current_value, expected)
        else:
            self.assertEqual(result.value, expected)
        self.assertIsInstance(result, self.test_types.ComplexUnion)

    @parameterized.expand(
        [
            ("enum_value", "color", lambda self: self.test_types.Color.red),
            ("string_value", "text", lambda _: "test_string"),
        ]
    )
    def test_union_with_non_numeric_values_unchanged(
        self, _name: str, field_name: str, value_fn: object
    ) -> None:
        """Test that unions with non-numeric values are unchanged."""
        # pyre-ignore[29]: value_fn is a callable
        value = value_fn(self)
        val = self.test_types.ComplexUnion(**{field_name: value})
        result = round_thrift_to_float32(val)
        if self.is_mutable_run:
            self.assertEqual(result.fbthrift_current_value, value)
        else:
            self.assertEqual(result.value, value)

    def test_union_with_list_value_is_rounded(self) -> None:
        if self.is_mutable_run:
            return  # mutable containers not yet implemented
        """Test that a union with a list of floats is rounded to float32."""
        val = self.test_types.ComplexUnion(float_list=[1.1, 2.2, 3.3])
        result = round_thrift_to_float32(val)
        expected = [to_float32(1.1), to_float32(2.2), to_float32(3.3)]
        self.assertEqual(list(result.fbthrift_current_value), expected)

    def test_empty_union_is_returned_unchanged(self) -> None:
        """Test that an empty union is returned unchanged."""
        val = self.test_types.ComplexUnion()
        result = round_thrift_to_float32(val)
        self.assertIsNone(result.fbthrift_current_value)


class AssertAlmostEqualTestNonThrift(unittest.TestCase):
    """Test suite for assert_thrift_almost_equal helper function."""

    def test_equal_floats_pass(self) -> None:
        """Test that nearly equal floats pass the assertion."""
        assert_thrift_almost_equal(self, 1.00000001, 1.0)

    def test_unequal_floats_fail_with_context(self) -> None:
        """Test that unequal floats fail and include field_context in error."""
        with self.assertRaises(AssertionError):
            assert_thrift_almost_equal(self, 1.5, 2.0)

    def test_unequal_floats_fail_with_msg_kwarg(self) -> None:
        """Test that msg= kwarg is included in the AssertionError."""
        # The field_context should be passed as msg= to assertAlmostEqual
        # Error format: "1.5 != 2.0 within 5 places (0.5 difference) : myfield"
        with self.assertRaisesRegex(AssertionError, r"1\.5 != 2\.0.*: myfield"):
            assert_thrift_almost_equal(
                self,
                1.5,
                2.0,
                field_context="myfield",
                places=5,
            )

    def test_equal_integers_pass(self) -> None:
        """Test that equal integers pass the assertion."""
        assert_thrift_almost_equal(self, 42, 42)

    def test_unequal_integers_fail_with_context(self) -> None:
        """Test that unequal integers fail and include field_context in error."""
        with self.assertRaisesRegex(AssertionError, "42 != 43"):
            assert_thrift_almost_equal(self, 42, 43)

    def test_strings(self) -> None:
        """Test that equal strings pass the assertion."""
        assert_thrift_almost_equal(self, "hello", "hello")
        with self.assertRaisesRegex(AssertionError, "'hello' != 'world'"):
            assert_thrift_almost_equal(self, "hello", "world")

    def test_equal_lists_pass(self) -> None:
        """Test that equal lists pass the assertion."""
        list1 = [1.0, 2.0, 3.0]
        list2 = [1.0, 2.0, 3.0]
        assert_thrift_almost_equal(self, list1, list2)

    def test_unequal_lists_fail_with_index_context(self) -> None:
        """Test that unequal lists fail and include index in context."""
        list1 = [1.0, 2.0, 3.0]
        list2 = [1.0, 2.5, 3.0]
        # Index should be in context
        with self.assertRaisesRegex(AssertionError, r"list\[1\]"):
            assert_thrift_almost_equal(self, list1, list2)

    def test_unequal_list_lengths_fail_with_context(self) -> None:
        """Test that lists of different lengths fail with proper context."""
        list1 = [1.0, 2.0]
        list2 = [1.0, 2.0, 3.0]
        with self.assertRaisesRegex(AssertionError, r"list\.__len__"):
            assert_thrift_almost_equal(self, list1, list2)

    def test_equal_dicts_pass(self) -> None:
        """Test that equal dicts pass the assertion."""
        dict1 = {"a": 1.0, "b": 2.0}
        dict2 = {"a": 1.0, "b": 2.0}
        assert_thrift_almost_equal(self, dict1, dict2)

    def test_unequal_dict_values_fail_with_key_context(self) -> None:
        """Test that unequal dict values fail and include key in context."""
        dict1 = {"a": 1.0, "b": 2.0}
        dict2 = {"a": 1.0, "b": 2.5}
        # Key should be in context
        with self.assertRaisesRegex(AssertionError, r"dict\[b\]"):
            assert_thrift_almost_equal(self, dict1, dict2)

    def test_dict_missing_keys_fail_with_context(self) -> None:
        """Test that dicts with different keys fail with proper context."""
        dict1 = {"a": 1.0, "b": 2.0}
        dict2 = {"a": 1.0, "c": 3.0}
        # Set order is not guaranteed, so just assert not 'a'
        with self.assertRaisesRegex(
            AssertionError,
            r"result and expected have non-empty symmetric difference between key sets: \{[^a]+\}; dict",
        ):
            assert_thrift_almost_equal(self, dict1, dict2)

    def test_equal_sets_pass(self) -> None:
        """Test that equal sets pass the assertion."""
        set1 = {1, 2, 3}
        set2 = {1, 2, 3}
        assert_thrift_almost_equal(self, set1, set2)

    def test_unequal_sets_fail_with_context(self) -> None:
        """Test that unequal sets fail and include field_context in error."""
        set1 = {1, 2, 3}
        set2 = {1, 2, 4}
        with self.assertRaisesRegex(AssertionError, "set"):
            assert_thrift_almost_equal(self, set1, set2)

    def test_places_kwarg_is_respected(self) -> None:
        """Test that the places= kwarg is properly passed to assertAlmostEqual."""
        # With places=1, these should be equal
        assert_thrift_almost_equal(self, 1.15, 1.14, places=1)

        # With places=2, these should not be equal
        with self.assertRaises(AssertionError):
            assert_thrift_almost_equal(self, 1.15, 1.14, places=2)

    def test_delta_kwarg_is_respected(self) -> None:
        """Test that the delta= kwarg is properly passed to assertAlmostEqual."""
        # With delta=0.1, these should be equal
        assert_thrift_almost_equal(self, 1.15, 1.14, delta=0.1)

        # With delta=0.001, these should not be equal
        with self.assertRaises(AssertionError):
            assert_thrift_almost_equal(self, 1.15, 1.14, delta=0.001)


@parameterized_class(
    ("test_types",),
    [
        (immutable_test_types,),
        (mutable_test_types,),
    ],
)
class AssertThriftAlmostEqualTest(unittest.TestCase):
    """Test suite for assert_thrift_almost_equal helper function."""

    def setUp(self) -> None:
        self.test_types: Any = self.test_types
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )

    def test_equal_structs_pass(self) -> None:
        """Test that equal structs pass the assertion."""
        struct1 = self.test_types.LatLon(lat=51.4769, lon=0.0005)
        struct2 = self.test_types.LatLon(lat=51.4769, lon=0.0005)
        assert_thrift_almost_equal(self, struct1, struct2)

    def test_unequal_struct_types_fail_with_context(self) -> None:
        """Test that structs of different types fail with proper context."""
        struct1 = self.test_types.LatLon(lat=51.4769, lon=0.0005)
        struct2 = self.test_types.numerical(int_val=42)
        with self.assertRaisesRegex(AssertionError, r"LatLon\.__name__"):
            assert_thrift_almost_equal(self, struct1, struct2)

    def test_unequal_struct_fields_fail_with_field_context(self) -> None:
        """Test that unequal struct fields fail and include full field path."""
        struct1 = self.test_types.LatLon(lat=51.4769, lon=0.0005)
        struct2 = self.test_types.LatLon(lat=51.4769, lon=0.0006)
        # Should include both the parent context and field name
        with self.assertRaisesRegex(AssertionError, r"LatLon\.lon"):
            assert_thrift_almost_equal(self, struct1, struct2, places=5)

    def test_equal_unions_pass(self) -> None:
        """Test that equal unions pass the assertion."""
        union1 = self.test_types.ComplexUnion(float_val=3.14)
        union2 = self.test_types.ComplexUnion(float_val=3.14)
        assert_thrift_almost_equal(self, union1, union2)

    def test_unions_different_fields_fail_with_context(self) -> None:
        """Test that unions with different active fields fail with proper context."""
        union1 = self.test_types.ComplexUnion(float_val=3.14)
        union2 = self.test_types.ComplexUnion(text="hello")
        with self.assertRaisesRegex(AssertionError, "fbthrift_current_field"):
            assert_thrift_almost_equal(self, union1, union2)

    def test_unequal_union_values_fail_with_field_context(self) -> None:
        """Test that unions with different values fail and include full field path."""
        union1 = self.test_types.ComplexUnion(float_val=3.14)
        union2 = self.test_types.ComplexUnion(float_val=2.71)
        with self.assertRaisesRegex(AssertionError, r"ComplexUnion\.float_val"):
            assert_thrift_almost_equal(self, union1, union2, places=2)

    def test_nested_struct_with_floats_pass(self) -> None:
        """Test that nested structs with nearly equal floats pass."""
        struct1 = self.test_types.LatLon(lat=51.47690001, lon=0.00050001)
        struct2 = self.test_types.LatLon(lat=51.4769, lon=0.0005)
        assert_thrift_almost_equal(self, struct1, struct2, places=5)

    def test_rel_tol_kwarg_is_respected(self) -> None:
        """Test that the rel_tol= kwarg is properly passed to assertAlmostEqual."""
        # With rel_tol=1e-6, these values should be equal (relative difference is ~1e-9)
        struct1 = self.test_types.LatLon(lat=1000000.0, lon=2000000.0)
        struct2 = self.test_types.LatLon(lat=1000000.001, lon=2000000.002)
        assert_thrift_almost_equal(self, struct1, struct2, rel_tol=1e-6)

        # With rel_tol=1e-9, these should not be equal (relative difference is ~1e-9)
        with self.assertRaises(AssertionError):
            assert_thrift_almost_equal(self, struct1, struct2, rel_tol=1e-9)

    def test_deeply_nested_structure_with_context(self) -> None:
        """Test that deeply nested structures provide full context path on failure."""
        if self.is_mutable_run:
            return  # Skip for mutable types due to container limitations

        struct1 = self.test_types.numerical(
            int_val=42,
            float_val=3.14,
            float_list=[1.0, 2.0, 3.0],
        )
        struct2 = self.test_types.numerical(
            int_val=42,
            float_val=3.14,
            float_list=[1.0, 2.5, 3.0],
        )
        # Should show full path: result->numerical.float_list->List[1]
        with self.assertRaisesRegex(
            AssertionError, r"numerical\.float_list->List\[1\]"
        ):
            assert_thrift_almost_equal(self, struct1, struct2)
