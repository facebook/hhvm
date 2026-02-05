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

import unittest
from unittest import mock

from parameterized import parameterized
from thrift.util import randomizer


def _create_mock_type_spec(type_name="TestType"):
    mock_spec = mock.MagicMock()
    mock_spec.get_type_name.return_value = type_name
    return mock_spec


class TestEnsureBinary(unittest.TestCase):
    @parameterized.expand(
        [
            ("str_to_bytes", "hello", b"hello"),
            ("bytes_passthrough", b"world", b"world"),
            ("empty_str", "", b""),
            ("unicode_str", "café", b"caf\xc3\xa9"),
        ]
    )
    def test_ensure_binary_valid_inputs(self, name, input_val, expected):
        # GIVEN
        # input_val and expected from parameters

        # WHEN
        actual = randomizer._ensure_binary(input_val)

        # THEN
        self.assertEqual(expected, actual)

    def test_ensure_binary_invalid_type_raises(self):
        # GIVEN
        invalid_input = 123

        # WHEN / THEN
        with self.assertRaises(TypeError) as ctx:
            randomizer._ensure_binary(invalid_input)

        self.assertIn("not expecting type", str(ctx.exception))


class TestDeepDictUpdate(unittest.TestCase):
    @parameterized.expand(
        [
            (
                "basic_update",
                {"a": 1},
                {"b": 2},
                {"a": 1, "b": 2},
            ),
            (
                "overwrite_simple_value",
                {"a": 1},
                {"a": 2},
                {"a": 2},
            ),
            (
                "nested_dict_merge",
                {"a": {"x": 1, "y": 2}},
                {"a": {"y": 3, "z": 4}},
                {"a": {"x": 1, "y": 3, "z": 4}},
            ),
            (
                "deeply_nested_merge",
                {"a": {"b": {"c": 1}}},
                {"a": {"b": {"d": 2}}},
                {"a": {"b": {"c": 1, "d": 2}}},
            ),
        ]
    )
    def test_deep_dict_update(self, name, base, update, expected):
        # GIVEN
        # base and update from parameters

        # WHEN
        randomizer.deep_dict_update(base, update)

        # THEN
        self.assertEqual(expected, base)

    def test_deep_dict_update_nondestructive_on_base_values(self):
        # GIVEN
        inner_dict = {"x": 1}
        base = {"a": inner_dict}
        update = {"a": {"y": 2}}

        # WHEN
        randomizer.deep_dict_update(base, update)

        # THEN
        self.assertEqual({"x": 1}, inner_dict)
        self.assertEqual({"a": {"x": 1, "y": 2}}, base)


class TestBoolRandomizer(unittest.TestCase):
    def test_randomize_returns_bool(self):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        bool_randomizer = randomizer.BoolRandomizer(type_spec, state, {})

        # WHEN
        result = bool_randomizer._randomize()

        # THEN
        self.assertIsInstance(result, bool)

    @parameterized.expand(
        [
            ("always_true", 1.0, True),
            ("always_false", 0.0, False),
        ]
    )
    def test_randomize_respects_p_true(self, name, p_true, expected):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        constraints = {"p_true": p_true}
        bool_randomizer = randomizer.BoolRandomizer(type_spec, state, constraints)

        # WHEN
        result = bool_randomizer._randomize()

        # THEN
        self.assertEqual(expected, result)

    @parameterized.expand(
        [
            ("bool_true", True, True),
            ("bool_false", False, False),
            ("int_1", 1, True),
            ("int_0", 0, False),
            ("str_true", "true", True),
            ("str_false", "false", False),
        ]
    )
    def test_eval_seed(self, name, seed, expected):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        bool_randomizer = randomizer.BoolRandomizer(type_spec, state, {})

        # WHEN
        result = bool_randomizer.eval_seed(seed)

        # THEN
        self.assertEqual(expected, result)


class TestI32Randomizer(unittest.TestCase):
    def test_randomize_returns_int_in_range(self):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        i32_randomizer = randomizer.I32Randomizer(type_spec, state, {})
        min_val = -(2**31)
        max_val = 2**31 - 1

        # WHEN
        result = i32_randomizer._randomize()

        # THEN
        self.assertIsInstance(result, int)
        self.assertGreaterEqual(result, min_val)
        self.assertLessEqual(result, max_val)

    def test_randomize_respects_range_constraint(self):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        constraints = {"range": [10, 20]}
        i32_randomizer = randomizer.I32Randomizer(type_spec, state, constraints)

        # WHEN
        result = i32_randomizer._randomize()

        # THEN
        self.assertGreaterEqual(result, 10)
        self.assertLessEqual(result, 20)

    def test_add_delta_clamps_to_max(self):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        constraints = {"fuzz_max_delta": 10}
        i32_randomizer = randomizer.I32Randomizer(type_spec, state, constraints)
        max_val = 2**31 - 1

        # WHEN
        result = i32_randomizer._add_delta(max_val)

        # THEN
        self.assertLessEqual(result, max_val)

    def test_add_delta_clamps_to_min(self):
        # GIVEN
        type_spec = _create_mock_type_spec()
        state = randomizer.RandomizerState()
        constraints = {"fuzz_max_delta": 10}
        i32_randomizer = randomizer.I32Randomizer(type_spec, state, constraints)
        min_val = -(2**31)

        # WHEN
        result = i32_randomizer._add_delta(min_val)

        # THEN
        self.assertGreaterEqual(result, min_val)
