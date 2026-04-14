#!/usr/bin/env python3
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

from __future__ import annotations

import json
import math
import unittest

import facebook.thrift.json5.json5_compatibility_test.thrift_types as compat_types
import facebook.thrift.json5.json5_negative_test.thrift_types as negative_types
import facebook.thrift.json5.json5_test.thrift_types as test_types
from thrift.python.exceptions import Error
from thrift.python.serializer import deserialize, Protocol, serialize


class Json5SerializerTest(unittest.TestCase):
    """Test JSON5 serialization using test cases from json5_test.thrift."""

    def test_serialize(self) -> None:
        for tc in test_types.testCases:
            with self.subTest(tc.name):
                json_str = serialize(tc.example, protocol=Protocol.JSON5).decode()
                self.assertEqual(json.loads(json_str), json.loads(tc.json))

    def test_deserialize_json(self) -> None:
        for tc in test_types.testCases:
            with self.subTest(tc.name):
                result = deserialize(
                    test_types.Example, tc.json.encode(), Protocol.JSON5
                )
                self.assertEqual(result, tc.example)

    def test_deserialize_json5(self) -> None:
        for tc in test_types.testCases:
            with self.subTest(tc.name):
                result = deserialize(
                    test_types.Example, tc.json5.encode(), Protocol.JSON5
                )
                self.assertEqual(result, tc.example)


class Json5EncoderExtraTest(unittest.TestCase):
    def test_serialize_bool(self) -> None:
        for value in [True, False]:
            with self.subTest(value=value):
                data = test_types.Example(boolValue=value)
                json_str = serialize(data, protocol=Protocol.JSON5).decode()
                parsed = json.loads(json_str)
                self.assertEqual(parsed["boolValue"], value)

    def test_serialize_int(self) -> None:
        for value in [0, 42, -42, 2147483647, -2147483648]:
            with self.subTest(value=value):
                data = test_types.Example(i32Value=value)
                json_str = serialize(data, protocol=Protocol.JSON5).decode()
                parsed = json.loads(json_str)
                self.assertEqual(parsed["i32Value"], value)

    def test_serialize_float(self) -> None:
        for value in [0.0, 1.5, -1.5, 0.1]:
            with self.subTest(value=value):
                data = test_types.Example(doubleValue=value)
                json_str = serialize(data, protocol=Protocol.JSON5).decode()
                parsed = json.loads(json_str)
                self.assertEqual(parsed["doubleValue"], value)

    def test_serialize_negative_zero(self) -> None:
        data = test_types.Example(doubleValue=-0.0)
        json_str = serialize(data, protocol=Protocol.JSON5).decode()
        parsed = json.loads(json_str)
        self.assertEqual(math.copysign(1.0, parsed["doubleValue"]), -1.0)

    def test_serialize_string(self) -> None:
        for value in [
            "",
            "Hello, world!",
            'Say "Hello"!',
            "Hello,\nworld!",
            "\U0001f44b",
        ]:
            with self.subTest(value=value):
                data = test_types.Example(stringValue=value)
                json_str = serialize(data, protocol=Protocol.JSON5).decode()
                parsed = json.loads(json_str)
                self.assertEqual(parsed["stringValue"], value)

    def test_serialize_nan_inf(self) -> None:
        data = test_types.Example(
            infValue=float("inf"),
            nanValue=float("nan"),
        )
        json_str = serialize(data, protocol=Protocol.JSON5).decode()
        parsed = json.loads(json_str)
        self.assertEqual(parsed["infValue"], "Infinity")
        self.assertEqual(parsed["nanValue"], "NaN")

    def test_serialize_negative_nan_inf(self) -> None:
        data = test_types.Example(
            infValue=float("-inf"),
            nanValue=-float("nan"),
        )
        json_str = serialize(data, protocol=Protocol.JSON5).decode()
        parsed = json.loads(json_str)
        self.assertEqual(parsed["infValue"], "-Infinity")
        self.assertEqual(parsed["nanValue"], "-NaN")


class Json5CompatibilityTest(unittest.TestCase):
    """Test JSON5 backward compatibility using test cases from
    json5_compatibility_test.thrift."""

    def test_compatibility(self) -> None:
        for tc in compat_types.compatibilityTestCases:
            with self.subTest(tc.name):
                for input_json in tc.inputs:
                    result = deserialize(
                        test_types.Example,
                        input_json.encode(),
                        Protocol.JSON5,
                    )
                    self.assertEqual(
                        result,
                        tc.output,
                        f"Failed for input: {input_json}",
                    )


class Json5CompatibilityExtraTest(unittest.TestCase):
    def test_positive_infinity_formats(self) -> None:
        for input_json in [
            '{"infValue": Infinity}',
            '{"infValue": "Infinity"}',
        ]:
            with self.subTest(input_json=input_json):
                result = deserialize(
                    test_types.Example,
                    input_json.encode(),
                    Protocol.JSON5,
                )
                value = result.infValue
                self.assertIsNotNone(value)
                self.assertTrue(math.isinf(value))
                self.assertGreater(value, 0)

    def test_negative_infinity_formats(self) -> None:
        for input_json in [
            '{"infValue": -Infinity}',
            '{"infValue": "-Infinity"}',
        ]:
            with self.subTest(input_json=input_json):
                result = deserialize(
                    test_types.Example,
                    input_json.encode(),
                    Protocol.JSON5,
                )
                value = result.infValue
                self.assertIsNotNone(value)
                self.assertTrue(math.isinf(value))
                self.assertLess(value, 0)

    def test_nan_formats(self) -> None:
        for input_json in [
            '{"nanValue": NaN}',
            '{"nanValue": "NaN"}',
        ]:
            with self.subTest(input_json=input_json):
                result = deserialize(
                    test_types.Example,
                    input_json.encode(),
                    Protocol.JSON5,
                )
                value = result.nanValue
                self.assertIsNotNone(value)
                self.assertTrue(math.isnan(value))


class Json5NegativeTest(unittest.TestCase):
    """Test that JSON5 decoder rejects invalid inputs."""

    def test_negative(self) -> None:
        cases = [
            ("enum", negative_types.enumValidationNegativeCases),
            ("type", negative_types.typeValidationNegativeCases),
            ("format", negative_types.formatValidationNegativeCases),
            ("overflow", negative_types.overflowValidationNegativeCases),
        ]
        for category, test_cases in cases:
            for tc in test_cases:
                with self.subTest(f"{category}_{tc.name}"):
                    with self.assertRaises(Error):
                        deserialize(
                            test_types.Example, tc.json.encode(), Protocol.JSON5
                        )
