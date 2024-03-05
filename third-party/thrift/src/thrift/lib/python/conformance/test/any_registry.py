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

import unittest

from thrift.conformance.protocol.thrift_types import StandardProtocol
from thrift.python.conformance.any_registry import AnyRegistry

# @manual=//thrift/test/testset:testset-python-types
from thrift.test.testset import thrift_types


class AnyRegistryTest(unittest.TestCase):
    def test_register_type(self) -> None:
        registry = AnyRegistry()
        self.assertTrue(registry.register_type(thrift_types.struct_empty))
        self.assertFalse(registry.register_type(thrift_types.struct_empty))

    def test_round_trip(self) -> None:
        def _test_for_protocol(protocol: StandardProtocol) -> None:
            registry = AnyRegistry()
            registry.register_module(thrift_types)
            original = thrift_types.struct_map_string_i32(
                field_1={
                    "Answer to the Ultimate Question of Life, the Universe, and Everything.": 42
                }
            )
            any_obj = registry.store(original, protocol)
            self.assertIsNotNone(any_obj.typeHashPrefixSha2_256)
            self.assertEqual(protocol, any_obj.protocol)
            loaded = registry.load(any_obj)
            self.assertEqual(original, loaded)

        for prot in (
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ):
            _test_for_protocol(prot)

    def test_absent_protocol(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)
        original = thrift_types.struct_map_string_i32(
            field_1={
                "Answer to the Ultimate Question of Life, the Universe, and Everything.": 42
            }
        )
        any_obj = registry.store(original)
        self.assertIsNotNone(any_obj.typeHashPrefixSha2_256)
        self.assertIsNone(any_obj.protocol)
        loaded = registry.load(any_obj)
        self.assertEqual(original, loaded)
