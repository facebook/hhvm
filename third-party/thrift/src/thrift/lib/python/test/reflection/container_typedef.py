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

from python_test.reflection.container_typedef.thrift_clients import (
    ReflectionReproService,
)
from python_test.reflection.container_typedef.thrift_services import (
    ReflectionReproServiceInterface as ReproInterface,
)
from thrift.python.reflection.inspect_impl import inspect
from thrift.python.reflection.services_reflection import ServiceSpec


class ReflectionReproTest(unittest.TestCase):
    """Regression test for the reflection-codegen module-qualification bug.

    A service method whose arg/return is a named container typedef defined in an
    *included* file made the generated thrift_services_reflection.py reference the
    typedef under the enclosing program's module instead of the defining module.
    That AttributeError was swallowed by __get_reflection__, so inspect()
    returned None for the whole service. This must return a full ServiceSpec.
    """

    def test_inspect_client_is_not_none(self) -> None:
        spec = inspect(ReflectionReproService)
        self.assertIsNotNone(
            spec, "inspect() returned None -- reflection module failed to build"
        )
        self.assertIsInstance(spec, ServiceSpec)

    def test_all_functions_present(self) -> None:
        spec = inspect(ReflectionReproService)
        assert spec is not None
        self.assertIn("echo_map", spec.functions)
        self.assertIn("echo_same", spec.functions)
        self.assertIn("echo_hidden", spec.functions)

    def test_cross_file_typedef_arg_and_return(self) -> None:
        spec = inspect(ReflectionReproService)
        assert spec is not None
        echo_map = spec.get_function("echo_map")
        self.assertIsNotNone(echo_map)
        assert echo_map is not None
        # arg and return resolve to a real class (the included ReproDepMap)
        self.assertIsNotNone(echo_map.return_type)
        self.assertEqual(len(echo_map.arguments), 1)
        self.assertIsNotNone(echo_map.arguments[0].type)

    def test_same_file_typedef_still_works(self) -> None:
        spec = inspect(ReflectionReproService)
        assert spec is not None
        echo_same = spec.get_function("echo_same")
        self.assertIsNotNone(echo_same)
        assert echo_same is not None
        self.assertIsNotNone(echo_same.return_type)

    def test_py3hidden_typedef(self) -> None:
        spec = inspect(ReflectionReproService)
        assert spec is not None
        echo_hidden = spec.get_function("echo_hidden")
        self.assertIsNotNone(echo_hidden)
        assert echo_hidden is not None
        self.assertIsNotNone(echo_hidden.return_type)

    def test_interface_and_client_agree(self) -> None:
        client_spec = inspect(ReflectionReproService)
        iface_spec = inspect(ReproInterface)
        self.assertIsNotNone(iface_spec)
        assert client_spec is not None and iface_spec is not None
        self.assertEqual(
            list(client_spec.functions.keys()),
            list(iface_spec.functions.keys()),
        )
