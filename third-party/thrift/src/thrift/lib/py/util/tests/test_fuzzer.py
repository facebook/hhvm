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

"""Tests for fuzzer constraints with py-deprecated and thrift-python types."""

import math
import unittest

from fuzz.ttypes import (
    BTree as BTreePyDeprecated,
    Color as ColorPyDeprecated,
    IntUnion as IntUnionPyDeprecated,
    SimpleStruct as SimpleStructPyDeprecated,
    StructWithOptionals as StructWithOptionalsPyDeprecated,
)
from parameterized import parameterized
from thrift import Thrift
from thrift.util import randomizer, type_inspect


def get_py_deprecated_struct_spec_args(cls):
    """Create spec_args tuple for py-deprecated types."""
    thrift_spec = cls.thrift_spec
    is_union = cls.isUnion() if hasattr(cls, "isUnion") else False
    return (cls, thrift_spec, is_union)


def get_valid_enum_values(enum_cls):
    """Get valid enum values for py-deprecated enums."""
    return set(enum_cls._VALUES_TO_NAMES.keys())


class TestFuzzer(unittest.TestCase):
    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_bool_p_true_always_true(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        bool_spec = inspect_module.get_spec(Thrift.TType.BOOL, None)
        gen = randomizer.BoolRandomizer(bool_spec, state, {"p_true": 1.0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertTrue(result)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_bool_p_true_always_false(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        bool_spec = inspect_module.get_spec(Thrift.TType.BOOL, None)
        gen = randomizer.BoolRandomizer(bool_spec, state, {"p_true": 0.0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertFalse(result)

    @parameterized.expand(
        [
            ("py_deprecated", ColorPyDeprecated, type_inspect),
        ]
    )
    def test_enum_p_invalid_zero(self, name, enum_cls, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        enum_spec = inspect_module.get_spec(Thrift.TType.I32, enum_cls)
        gen = randomizer.EnumRandomizer(enum_spec, state, {"p_invalid": 0})
        gen.preprocess()
        valid_values = get_valid_enum_values(enum_cls)

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertIn(result, valid_values)

    @parameterized.expand(
        [
            ("py_deprecated", ColorPyDeprecated, type_inspect),
        ]
    )
    def test_enum_p_invalid_one(self, name, enum_cls, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        enum_spec = inspect_module.get_spec(Thrift.TType.I32, enum_cls)
        gen = randomizer.EnumRandomizer(enum_spec, state, {"p_invalid": 1.0})
        gen.preprocess()
        valid_values = get_valid_enum_values(enum_cls)

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertNotIn(result, valid_values)
            self.assertIsInstance(result, int)

    @parameterized.expand(
        [
            ("py_deprecated", ColorPyDeprecated, type_inspect),
        ]
    )
    def test_enum_choices(self, name, enum_cls, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        enum_spec = inspect_module.get_spec(Thrift.TType.I32, enum_cls)
        allowed_choices = [16711680, 255]  # RED and BLUE
        gen = randomizer.EnumRandomizer(
            enum_spec, state, {"choices": allowed_choices, "p_invalid": 0}
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertIn(result, allowed_choices)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_i32_range(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        i32_spec = inspect_module.get_spec(Thrift.TType.I32, None)
        gen = randomizer.I32Randomizer(i32_spec, state, {"range": [100, 200]})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertGreaterEqual(result, 100)
            self.assertLessEqual(result, 200)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_i32_choices(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        i32_spec = inspect_module.get_spec(Thrift.TType.I32, None)
        allowed_choices = [1, 2, 3]
        gen = randomizer.I32Randomizer(i32_spec, state, {"choices": allowed_choices})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertIn(result, allowed_choices)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_double_p_zero_one(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        double_spec = inspect_module.get_spec(Thrift.TType.DOUBLE, None)
        gen = randomizer.DoublePrecisionFloatRandomizer(
            double_spec, state, {"p_zero": 1.0, "p_unreal": 0}
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertEqual(result, 0.0)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_double_p_unreal_one(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        double_spec = inspect_module.get_spec(Thrift.TType.DOUBLE, None)
        gen = randomizer.DoublePrecisionFloatRandomizer(
            double_spec, state, {"p_unreal": 1.0}
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            is_unreal = math.isnan(result) or math.isinf(result)
            self.assertTrue(is_unreal, f"Expected nan/inf/-inf, got {result}")

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_double_mean_std_deviation(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        double_spec = inspect_module.get_spec(Thrift.TType.DOUBLE, None)
        mean = 100.0
        std_dev = 1.0
        gen = randomizer.DoublePrecisionFloatRandomizer(
            double_spec,
            state,
            {"mean": mean, "std_deviation": std_dev, "p_zero": 0, "p_unreal": 0},
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertGreater(result, mean - 10 * std_dev)
            self.assertLess(result, mean + 10 * std_dev)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_string_mean_length_zero(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        string_spec = inspect_module.get_spec(Thrift.TType.STRING, None)
        gen = randomizer.StringRandomizer(string_spec, state, {"mean_length": 0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertEqual(result, "")

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_list_mean_length_zero(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        list_spec = inspect_module.get_spec(
            Thrift.TType.LIST, (Thrift.TType.BOOL, None)
        )
        gen = randomizer.ListRandomizer(list_spec, state, {"mean_length": 0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertEqual(len(result), 0)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_list_max_length(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        list_spec = inspect_module.get_spec(Thrift.TType.LIST, (Thrift.TType.I32, None))
        gen = randomizer.ListRandomizer(
            list_spec, state, {"mean_length": 100, "max_length": 5}
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertLessEqual(len(result), 5)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_list_element_constraints(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        list_spec = inspect_module.get_spec(Thrift.TType.LIST, (Thrift.TType.I32, None))
        gen = randomizer.ListRandomizer(
            list_spec,
            state,
            {"mean_length": 5, "max_length": 10, "element": {"range": [1, 10]}},
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            for elem in result:
                self.assertGreaterEqual(elem, 1)
                self.assertLessEqual(elem, 10)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_set_mean_length_zero(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        set_spec = inspect_module.get_spec(Thrift.TType.SET, (Thrift.TType.I32, None))
        gen = randomizer.SetRandomizer(set_spec, state, {"mean_length": 0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertEqual(len(result), 0)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_set_max_length(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        set_spec = inspect_module.get_spec(Thrift.TType.SET, (Thrift.TType.I32, None))
        gen = randomizer.SetRandomizer(
            set_spec, state, {"mean_length": 100, "max_length": 5}
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertLessEqual(len(result), 5)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_set_element_constraints(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        set_spec = inspect_module.get_spec(Thrift.TType.SET, (Thrift.TType.I32, None))
        gen = randomizer.SetRandomizer(
            set_spec,
            state,
            {"mean_length": 5, "max_length": 10, "element": {"range": [1, 10]}},
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            for elem in result:
                self.assertGreaterEqual(elem, 1)
                self.assertLessEqual(elem, 10)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_map_mean_length_zero(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        map_spec = inspect_module.get_spec(
            Thrift.TType.MAP, (Thrift.TType.I32, None, Thrift.TType.STRING, None)
        )
        gen = randomizer.MapRandomizer(map_spec, state, {"mean_length": 0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertEqual(len(result), 0)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_map_key_constraints(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        map_spec = inspect_module.get_spec(
            Thrift.TType.MAP, (Thrift.TType.I32, None, Thrift.TType.STRING, None)
        )
        gen = randomizer.MapRandomizer(
            map_spec,
            state,
            {"mean_length": 5, "max_length": 10, "key": {"range": [1, 100]}},
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            for key in result.keys():
                self.assertGreaterEqual(key, 1)
                self.assertLessEqual(key, 100)

    @parameterized.expand(
        [
            ("py_deprecated", type_inspect),
        ]
    )
    def test_map_value_constraints(self, name, inspect_module):
        # GIVEN
        state = randomizer.RandomizerState()
        map_spec = inspect_module.get_spec(
            Thrift.TType.MAP, (Thrift.TType.I32, None, Thrift.TType.I32, None)
        )
        gen = randomizer.MapRandomizer(
            map_spec,
            state,
            {"mean_length": 5, "max_length": 10, "value": {"choices": [42, 100]}},
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            for val in result.values():
                self.assertIn(val, [42, 100])

    @parameterized.expand(
        [
            (
                "py_deprecated",
                StructWithOptionalsPyDeprecated,
                type_inspect,
                get_py_deprecated_struct_spec_args,
            ),
        ]
    )
    def test_struct_p_include_zero(
        self, name, struct_cls, inspect_module, get_spec_args_fn
    ):
        # GIVEN
        state = randomizer.RandomizerState()
        spec_args = get_spec_args_fn(struct_cls)
        struct_spec = inspect_module.get_spec(Thrift.TType.STRUCT, spec_args)
        gen = randomizer.StructRandomizer(struct_spec, state, {"p_include": 0.0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertIsInstance(result, struct_cls)
            self.assertIsNone(result.a)
            self.assertIsNone(result.b)
            self.assertIsNone(result.c)

    @parameterized.expand(
        [
            (
                "py_deprecated",
                StructWithOptionalsPyDeprecated,
                type_inspect,
                get_py_deprecated_struct_spec_args,
            ),
        ]
    )
    def test_struct_p_include_one(
        self, name, struct_cls, inspect_module, get_spec_args_fn
    ):
        # GIVEN
        state = randomizer.RandomizerState()
        spec_args = get_spec_args_fn(struct_cls)
        struct_spec = inspect_module.get_spec(Thrift.TType.STRUCT, spec_args)
        gen = randomizer.StructRandomizer(struct_spec, state, {"p_include": 1.0})
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertIsInstance(result, struct_cls)
            self.assertIsNotNone(result.a)
            self.assertIsNotNone(result.b)
            self.assertIsNotNone(result.c)

    @parameterized.expand(
        [
            (
                "py_deprecated",
                StructWithOptionalsPyDeprecated,
                type_inspect,
                get_py_deprecated_struct_spec_args,
            ),
        ]
    )
    def test_struct_per_field(self, name, struct_cls, inspect_module, get_spec_args_fn):
        # GIVEN
        state = randomizer.RandomizerState()
        spec_args = get_spec_args_fn(struct_cls)
        struct_spec = inspect_module.get_spec(Thrift.TType.STRUCT, spec_args)
        gen = randomizer.StructRandomizer(
            struct_spec,
            state,
            {
                "p_include": 0.0,
                "per_field": {"a": {"p_include": 1.0}, "b": {"p_include": 0.0}},
            },
        )
        gen.preprocess()

        # WHEN
        results = [gen.generate() for _ in range(10)]

        # THEN
        for result in results:
            self.assertIsInstance(result, struct_cls)
            self.assertIsNotNone(result.a)
            self.assertIsNone(result.b)

    @parameterized.expand(
        [
            (
                "py_deprecated",
                BTreePyDeprecated,
                type_inspect,
                get_py_deprecated_struct_spec_args,
            ),
        ]
    )
    def test_struct_max_recursion_depth(
        self, name, struct_cls, inspect_module, get_spec_args_fn
    ):
        # GIVEN
        state = randomizer.RandomizerState()
        spec_args = get_spec_args_fn(struct_cls)
        struct_spec = inspect_module.get_spec(Thrift.TType.STRUCT, spec_args)
        gen = randomizer.StructRandomizer(
            struct_spec, state, {"p_include": 1.0, "max_recursion_depth": 1}
        )
        gen.preprocess()

        # WHEN
        result = gen.generate()

        # THEN
        self.assertIsNotNone(result)
        self.assertIsInstance(result, struct_cls)

    @parameterized.expand(
        [
            (
                "py_deprecated",
                SimpleStructPyDeprecated,
                type_inspect,
                get_py_deprecated_struct_spec_args,
            ),
        ]
    )
    def test_fuzz_struct(self, name, struct_cls, inspect_module, get_spec_args_fn):
        # GIVEN
        state = randomizer.RandomizerState()
        spec_args = get_spec_args_fn(struct_cls)
        type_spec = inspect_module.get_spec(Thrift.TType.STRUCT, spec_args)
        gen = randomizer.StructRandomizer(type_spec, state, {"p_include": 1.0})
        gen.preprocess()

        # WHEN
        result = gen.generate()

        # THEN
        self.assertIsInstance(result, struct_cls)
        self.assertIsNotNone(result.a)
        self.assertIsNotNone(result.b)

    @parameterized.expand(
        [
            (
                "py_deprecated",
                IntUnionPyDeprecated,
                type_inspect,
                get_py_deprecated_struct_spec_args,
            ),
        ]
    )
    def test_fuzz_union(self, name, union_cls, inspect_module, get_spec_args_fn):
        # GIVEN
        state = randomizer.RandomizerState()
        spec_args = get_spec_args_fn(union_cls)
        type_spec = inspect_module.get_spec(Thrift.TType.STRUCT, spec_args)
        gen = randomizer.StructRandomizer(type_spec, state, {"p_include": 1.0})
        gen.preprocess()

        # WHEN
        result = gen.generate()

        # THEN
        self.assertIsInstance(result, union_cls)


if __name__ == "__main__":
    unittest.main()
