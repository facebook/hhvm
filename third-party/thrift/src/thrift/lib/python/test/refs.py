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

import types
import unittest
from typing import Type, TypeVar

import python_test.refs.thrift_mutable_types as mutable_types
import python_test.refs.thrift_types as immutable_types
import thrift.python.mutable_serializer as mutable_serializer
import thrift.python.serializer as immutable_serializer
from parameterized import parameterized_class
from python_test.refs.thrift_types import Circular, ComplexRef as ComplexRefType
from thrift.python.mutable_types import (
    _ThriftListWrapper,
    _ThriftMapWrapper,
    to_thrift_list,
    to_thrift_map,
    to_thrift_set,
)

ListT = TypeVar("ListT")
MapKey = TypeVar("MapKey")
MapValue = TypeVar("MapValue")


@parameterized_class(
    ("test_types", "serializer_module"),
    [
        (immutable_types, immutable_serializer),
        (mutable_types, mutable_serializer),
    ],
)
class RefTest(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `test_types`
        self.ComplexRef: Type[ComplexRefType] = self.test_types.ComplexRef
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )
        self.Circular: Type[Circular] = self.test_types.Circular
        # pyre-ignore[16]: has no attribute `serializer_module`
        self.serializer: types.ModuleType = self.serializer_module

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

    def to_map(
        self, map_data: dict[MapKey, MapValue]
    ) -> dict[MapKey, MapValue] | _ThriftMapWrapper:
        return to_thrift_map(map_data) if self.is_mutable_run else map_data

    def test_circular(self) -> None:
        # works whether `child` is specified or left defaulted
        c = self.Circular(val="foo", child=self.Circular())
        self.assertEqual(c.val, "foo")
        c = c(child=c)
        self.assertIsNotNone(c.child)
        self.assertEqual(c.child.val, "foo")

        # thrift mutable python fails on serialize with PyList check error
        # if we rely on __call__ API
        if self.is_mutable_run:
            c = mutable_types.Circular(val="foo")
            c.child = c
            # if we proceed, will get stack overflow on serialize
            return

        buf = self.serializer.serialize(c)
        self.assertEqual(self.serializer.deserialize(self.Circular, buf), c)

    def test_create_default(self) -> None:
        x = self.ComplexRef()
        self.assertEqual(x.name, "")
        self.assertIsNone(x.list_basetype_ref)
        self.assertIsNone(x.list_recursive_ref)
        self.assertIsNone(x.set_basetype_ref)
        self.assertIsNone(x.set_recursive_ref)
        self.assertIsNone(x.map_basetype_ref)
        self.assertIsNone(x.map_recursive_ref)
        self.assertIsNone(x.list_shared_ref)
        self.assertIsNone(x.set_const_shared_ref)

    def test_single(self) -> None:
        x = self.ComplexRef(name="foo", ref=self.ComplexRef(name="bar"))
        self.assertIsNotNone(x.ref)
        assert x.ref  # for type checking
        self.assertEqual(x.ref.name, "bar")

    def test_list(self) -> None:
        bar, baz = self.ComplexRef(name="bar"), self.ComplexRef(name="baz")
        x = self.ComplexRef(
            name="foo",
            # pyre-ignore[6]: TODO: Thrift-Container init
            list_basetype_ref=self.to_list([1, 2, 3]),
            # pyre-ignore[6]: TODO: Thrift-Container init
            list_recursive_ref=self.to_list([bar, baz]),
        )
        self.assertEqual(x.list_basetype_ref, (1, 2, 3))
        self.assertEqual(x.list_recursive_ref, (bar, baz))

    def test_set(self) -> None:
        bar, baz = self.ComplexRef(name="bar"), self.ComplexRef(name="baz")
        x = self.ComplexRef(
            name="foo",
            # pyre-ignore[6]: TODO: Thrift-Container init
            set_basetype_ref=to_thrift_set({1, 2, 3})
            if self.is_mutable_run
            else {1, 2, 3},
            # pyre-ignore[6]: TODO: Thrift-Container init
            set_recursive_ref=to_thrift_set(set())  # mutable-types not hashable
            if self.is_mutable_run
            else {bar, baz},
        )
        self.assertEqual(x.set_basetype_ref, {1, 2, 3})
        self.assertEqual(
            x.set_recursive_ref, set() if self.is_mutable_run else {bar, baz}
        )

    def test_map(self) -> None:
        bar, baz = self.ComplexRef(name="bar"), self.ComplexRef(name="baz")
        x = self.ComplexRef(
            name="foo",
            # pyre-ignore[6]: TODO: Thrift-Container init
            map_basetype_ref=self.to_map({1: 1, 2: 2, 3: 3}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            map_recursive_ref=self.to_map({1: bar, 2: baz}),
        )
        self.assertEqual(x.map_basetype_ref, {1: 1, 2: 2, 3: 3})
        self.assertEqual(x.map_recursive_ref, {1: bar, 2: baz})

    def test_shared_ref(self) -> None:
        bar, baz = self.ComplexRef(name="bar"), self.ComplexRef(name="baz")
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.ComplexRef(name="foo", list_shared_ref=self.to_list([bar, baz]))
        self.assertEqual(x.list_shared_ref, (bar, baz))

    def test_const_shared_ref(self) -> None:
        bar, baz = self.ComplexRef(name="bar"), self.ComplexRef(name="baz")
        x = self.ComplexRef(
            name="foo",
            # pyre-ignore[6]: TODO: Thrift-Container init
            set_const_shared_ref=to_thrift_set(set())
            if self.is_mutable_run
            else {bar, baz},
        )
        self.assertEqual(
            x.set_const_shared_ref, set() if self.is_mutable_run else {bar, baz}
        )

    def test_recursive(self) -> None:
        bar = self.ComplexRef(name="bar")
        baz = self.ComplexRef(recursive=bar)
        self.assertEqual(baz.recursive, bar)
