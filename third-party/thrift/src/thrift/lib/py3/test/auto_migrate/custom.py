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

import unittest

from testing.types import customized
from thrift.py3.serializer import deserialize, serialize


# Tests for customized alternate implementations of the data structures:


class CustomTests(unittest.TestCase):
    def roundTrip(self, obj: customized) -> None:
        self.assertEqual(obj, deserialize(customized, serialize(obj)))

    def test_list_template(self) -> None:
        c = customized(list_template=[1, 2, 3])
        self.assertEqual(c.list_template, [1, 2, 3])
        self.assertEqual(list(c.list_template), [1, 2, 3])
        self.assertNotIsInstance(c.list_template, list)
        self.roundTrip(c)

    def test_set_template(self) -> None:
        c = customized(set_template={1, 2, 3})
        self.assertEqual(c.set_template, {1, 2, 3})
        self.assertEqual(set(c.set_template), {1, 2, 3})
        self.assertNotIsInstance(c.set_template, set)
        self.roundTrip(c)

    def test_map_template(self) -> None:
        c = customized(map_template={1: 2, 3: 6, 5: 10})
        self.assertEqual(c.map_template, {1: 2, 3: 6, 5: 10})
        self.assertEqual(dict(c.map_template.items()), {1: 2, 3: 6, 5: 10})
        self.assertNotIsInstance(c.map_template, dict)
        self.roundTrip(c)

    def test_list_type(self) -> None:
        c = customized(list_type=[1, 2, 3])
        self.assertEqual(c.list_type, [1, 2, 3])
        self.assertEqual(list(c.list_type), [1, 2, 3])
        self.assertNotIsInstance(c.list_type, list)
        self.roundTrip(c)

    def test_set_type(self) -> None:
        c = customized(set_type={1, 2, 3})
        self.assertEqual(c.set_type, {1, 2, 3})
        self.assertEqual(set(c.set_type), {1, 2, 3})
        self.assertNotIsInstance(c.set_type, set)
        self.roundTrip(c)

    def test_map_type(self) -> None:
        c = customized(map_type={1: 2, 3: 6, 5: 10})
        self.assertEqual(c.map_type, {1: 2, 3: 6, 5: 10})
        self.assertEqual(dict(c.map_type.items()), {1: 2, 3: 6, 5: 10})
        self.assertNotIsInstance(c.map_type, dict)
        self.roundTrip(c)

    def test_string_type(self) -> None:
        c = customized(string_type="hello")
        self.assertEqual(c.string_type, "hello")
        # For custom primitive and string types, we don't create new instance of type.
        self.roundTrip(c)

    def test_cpp_name(self) -> None:
        c = customized(foo=3)
        self.assertEqual(c.foo, 3)
        self.roundTrip(c)

    def test_list_of_uint32(self) -> None:
        c = customized(list_of_uint32=[1, 2, 3])
        self.assertEqual(c.list_of_uint32, [1, 2, 3])
        self.assertEqual(list(c.list_of_uint32), [1, 2, 3])
        self.assertNotIsInstance(c.list_of_uint32, list)
        self.roundTrip(c)

    def test_adapted_string(self) -> None:
        c = customized(adapted_string="hello")
        self.assertEqual(c.adapted_string, "hello")
        self.roundTrip(c)

    def test_adapted_binary(self) -> None:
        c = customized(adapted_binary=b"hello")
        self.assertEqual(c.adapted_binary, b"hello")
        self.roundTrip(c)

    def test_adapted_string_list(self) -> None:
        c = customized(adapted_string_list=["hello"])
        self.assertEqual(c.adapted_string_list, ["hello"])
        self.assertEqual(list(c.adapted_string_list), ["hello"])
        self.assertNotIsInstance(c.adapted_string_list, list)
        self.roundTrip(c)

    def test_adapted_string_set(self) -> None:
        c = customized(adapted_string_set={"1", "2", "3"})
        self.assertEqual(c.adapted_string_set, {"1", "2", "3"})
        self.assertEqual(set(c.adapted_string_set), {"1", "2", "3"})
        self.assertNotIsInstance(c.adapted_string_set, set)
        self.roundTrip(c)

    def test_adapted_string_key_map(self) -> None:
        c = customized(adapted_string_key_map={"1": "2", "3": "6", "5": "10"})
        self.assertEqual(c.adapted_string_key_map, {"1": "2", "3": "6", "5": "10"})
        self.assertEqual(
            dict(c.adapted_string_key_map.items()), {"1": "2", "3": "6", "5": "10"}
        )
        self.assertNotIsInstance(c.adapted_string_key_map, dict)
        self.roundTrip(c)

    def test_adapted_string_value_map(self) -> None:
        c = customized(adapted_string_value_map={"1": "2", "3": "6", "5": "10"})
        self.assertEqual(c.adapted_string_value_map, {"1": "2", "3": "6", "5": "10"})
        self.assertEqual(
            dict(c.adapted_string_value_map.items()), {"1": "2", "3": "6", "5": "10"}
        )
        self.assertNotIsInstance(c.adapted_string_value_map, dict)
        self.roundTrip(c)

    def test_adapted_string_map(self) -> None:
        c = customized(adapted_string_map={"1": "2", "3": "6", "5": "10"})
        self.assertEqual(c.adapted_string_map, {"1": "2", "3": "6", "5": "10"})
        self.assertEqual(
            dict(c.adapted_string_map.items()), {"1": "2", "3": "6", "5": "10"}
        )
        self.assertNotIsInstance(c.adapted_string_map, dict)
        self.roundTrip(c)
