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

import unittest

from typing import Type, TypeVar

import python_test.refs.thrift_mutable_types as mutable_types
import python_test.refs.thrift_types as immutable_ypes

from parameterized import parameterized_class

from python_test.refs.thrift_types import ComplexRef as ComplexRefType
from thrift.python.mutable_types import _ThriftListWrapper, to_thrift_list

ListT = TypeVar("ListT")


@parameterized_class(
    ("test_types"),
    [(immutable_ypes,), (mutable_types,)],
)
class RefTest(unittest.TestCase):
    def setUp(self) -> None:
        """
        The `setUp` method performs these assignments with type hints to enable
        pyre when using 'parameterized'. Otherwise, Pyre cannot deduce the types
        behind `test_types`.
        """
        # pyre-ignore[16]: has no attribute `sets_types`
        self.ComplexRef: Type[ComplexRefType] = self.test_types.ComplexRef
        self.is_mutable_run: bool = self.test_types.__name__.endswith(
            "thrift_mutable_types"
        )

    def to_list(self, list_data: list[ListT]) -> list[ListT] | _ThriftListWrapper:
        return to_thrift_list(list_data) if self.is_mutable_run else list_data

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
            set_basetype_ref={1, 2, 3},
            set_recursive_ref=set() if self.is_mutable_run else {bar, baz},
        )
        self.assertEqual(x.set_basetype_ref, {1, 2, 3})
        self.assertEqual(
            x.set_recursive_ref, set() if self.is_mutable_run else {bar, baz}
        )

    def test_map(self) -> None:
        bar, baz = self.ComplexRef(name="bar"), self.ComplexRef(name="baz")
        x = self.ComplexRef(
            name="foo",
            map_basetype_ref={1: 1, 2: 2, 3: 3},
            map_recursive_ref={1: bar, 2: baz},
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
            set_const_shared_ref=set() if self.is_mutable_run else {bar, baz},
        )
        self.assertEqual(
            x.set_const_shared_ref, set() if self.is_mutable_run else {bar, baz}
        )

    def test_recursive(self) -> None:
        bar = self.ComplexRef(name="bar")
        baz = self.ComplexRef(recursive=bar)
        self.assertEqual(baz.recursive, bar)
