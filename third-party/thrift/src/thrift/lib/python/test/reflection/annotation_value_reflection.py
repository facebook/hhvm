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
from typing import cast, Type

from python_test.reflection.annotation_value_reflection.thrift_clients import (
    AnnotationValueReflectionService,
)
from python_test.reflection.annotation_value_reflection.thrift_services import (
    AnnotationValueReflectionServiceInterface as ReflectionInterface,
)
from thrift.python.reflection.constants_reflection import ConstantStructSpec
from thrift.python.reflection.inspect_impl import inspect
from thrift.python.reflection.services_reflection import ServiceSpec
from thrift.python.types import ServiceInterface


class AnnotationValueReflectionTest(unittest.TestCase):
    """Regression test for the services-reflection annotation-value import bug.

    The service annotation's value has a type from a transitively-included
    program. The generated thrift_services_reflection.py referenced that
    module without importing it, so inspect() raised NameError instead of
    returning a ServiceSpec.
    """

    def test_inspect_does_not_raise_and_returns_spec(self) -> None:
        # Raised NameError before the fix.
        spec = inspect(cast(Type[ServiceInterface], AnnotationValueReflectionService))
        self.assertIsNotNone(
            spec, "inspect() returned None -- reflection module failed to build"
        )
        self.assertIsInstance(spec, ServiceSpec)

    def test_service_annotation_present(self) -> None:
        spec = inspect(cast(Type[ServiceInterface], AnnotationValueReflectionService))
        assert spec is not None
        self.assertIn(
            "annotation_value_def.WithNestedAnnotationValue",
            spec.structured_annotations,
        )

    def test_nested_annotation_value_resolves(self) -> None:
        spec = inspect(cast(Type[ServiceInterface], AnnotationValueReflectionService))
        assert spec is not None
        annotation = spec.structured_annotations[
            "annotation_value_def.WithNestedAnnotationValue"
        ]
        # Outer annotation and its transitively-typed nested value must resolve.
        outer = annotation.value
        assert isinstance(outer, ConstantStructSpec)
        nested = outer.fields["value"].value
        assert isinstance(nested, ConstantStructSpec)
        self.assertIsNotNone(nested.struct_type)

    def test_interface_and_client_agree(self) -> None:
        client_spec = inspect(
            cast(Type[ServiceInterface], AnnotationValueReflectionService)
        )
        iface_spec = inspect(ReflectionInterface)
        self.assertIsNotNone(client_spec)
        self.assertIsNotNone(iface_spec)
        assert client_spec is not None and iface_spec is not None
        self.assertEqual(
            list(client_spec.functions.keys()),
            list(iface_spec.functions.keys()),
        )
