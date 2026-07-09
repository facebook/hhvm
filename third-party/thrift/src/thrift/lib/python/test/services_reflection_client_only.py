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
from typing import Any

from parameterized import parameterized

# Deliberately depends ONLY on the -python-clients, not -python-services
from test_thrift.thrift_clients import TestingService
from test_thrift.thrift_types import Color, easy
from thrift.python.reflection import inspect
from thrift.python.reflection.services_reflection import ServiceSpec


def _client_spec() -> ServiceSpec:
    spec = inspect(TestingService)  # pyre-ignore[6]: client is inspectable
    assert isinstance(spec, ServiceSpec)
    return spec


class InspectClientOnlyTest(unittest.TestCase):
    def test_client_returns_service_spec(self) -> None:
        spec = _client_spec()
        self.assertEqual(spec.name, "TestingService")
        self.assertTrue(len(spec.functions) > 0)

    @parameterized.expand(
        [
            ("string", "getName", str),
            ("bool", "invert", bool),
            ("i32", "getPriority", int),
            ("float", "getRequestTimeout", float),
            ("struct", "echoStruct", easy),
            ("enum", "echoColor", Color),
        ]
    )
    def test_return_type_resolves_to_class(
        self, _name: str, func_name: str, expected_type: type[Any]
    ) -> None:
        func = _client_spec().get_function(func_name)
        assert func is not None, f"function {func_name!r} not found"
        self.assertIs(func.return_type, expected_type)

    @parameterized.expand(
        [
            ("string", "complex_action", "first", str),
            ("i64", "complex_action", "third", int),
            ("struct", "echoStruct", "what", easy),
            ("enum", "echoColor", "color", Color),
        ]
    )
    def test_argument_type_resolves_to_class(
        self, _name: str, func_name: str, arg_name: str, expected_type: type[Any]
    ) -> None:
        func = _client_spec().get_function(func_name)
        assert func is not None, f"function {func_name!r} not found"
        arg = func.get_argument(arg_name)
        assert arg is not None, f"argument {arg_name!r} not found"
        self.assertIs(arg.type, expected_type)
