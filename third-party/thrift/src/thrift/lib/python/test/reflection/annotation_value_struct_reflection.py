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

from python_test.reflection.annotation_value_struct_reflection.thrift_types import (
    AnnotationValueReflectionStruct,
)
from thrift.python.reflection.constants_reflection import ConstantStructSpec
from thrift.python.reflection.inspect_impl import inspect
from thrift.python.reflection.types_reflection import StructSpec


class AnnotationValueStructReflectionTest(unittest.TestCase):
    """Regression test for the struct-reflection annotation-value import bug.

    A struct- and field-level annotation's value has a type from a
    transitively-included program. The generated thrift_reflection.py
    referenced that module without importing it, so inspect() raised NameError
    instead of returning a StructSpec.
    """

    def test_inspect_does_not_raise_and_returns_spec(self) -> None:
        # Raised NameError before the fix.
        spec = inspect(AnnotationValueReflectionStruct)
        self.assertIsInstance(spec, StructSpec)

    def test_struct_annotation_nested_value_resolves(self) -> None:
        spec = inspect(AnnotationValueReflectionStruct)
        assert isinstance(spec, StructSpec)
        outer = spec.structured_annotations[
            "annotation_value_def.WithNestedAnnotationValue"
        ].value
        # Outer annotation and its transitively-typed nested value must resolve.
        assert isinstance(outer, ConstantStructSpec)
        nested = outer.fields["value"].value
        assert isinstance(nested, ConstantStructSpec)
        self.assertIsNotNone(nested.struct_type)

    def test_field_annotation_nested_value_resolves(self) -> None:
        spec = inspect(AnnotationValueReflectionStruct)
        assert isinstance(spec, StructSpec)
        field = spec.get_field("annotated_field")
        assert field is not None
        outer = field.structured_annotations[
            "annotation_value_def.WithNestedAnnotationValue"
        ].value
        assert isinstance(outer, ConstantStructSpec)
        nested = outer.fields["value"].value
        assert isinstance(nested, ConstantStructSpec)
        self.assertIsNotNone(nested.struct_type)
