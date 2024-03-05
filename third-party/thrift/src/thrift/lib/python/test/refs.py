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

from testing.thrift_types import ComplexRef


class RefTest(unittest.TestCase):
    def test_create_default(self) -> None:
        x = ComplexRef()
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
        x = ComplexRef(name="foo", ref=ComplexRef(name="bar"))
        self.assertIsNotNone(x.ref)
        assert x.ref  # for type checking
        self.assertEqual(x.ref.name, "bar")

    def test_list(self) -> None:
        bar, baz = ComplexRef(name="bar"), ComplexRef(name="baz")
        x = ComplexRef(
            name="foo", list_basetype_ref=[1, 2, 3], list_recursive_ref=[bar, baz]
        )
        self.assertEqual(x.list_basetype_ref, (1, 2, 3))
        self.assertEqual(x.list_recursive_ref, (bar, baz))

    def test_set(self) -> None:
        bar, baz = ComplexRef(name="bar"), ComplexRef(name="baz")
        x = ComplexRef(
            name="foo", set_basetype_ref={1, 2, 3}, set_recursive_ref={bar, baz}
        )
        self.assertEqual(x.set_basetype_ref, {1, 2, 3})
        self.assertEqual(x.set_recursive_ref, {bar, baz})

    def test_map(self) -> None:
        bar, baz = ComplexRef(name="bar"), ComplexRef(name="baz")
        x = ComplexRef(
            name="foo",
            map_basetype_ref={1: 1, 2: 2, 3: 3},
            map_recursive_ref={1: bar, 2: baz},
        )
        self.assertEqual(x.map_basetype_ref, {1: 1, 2: 2, 3: 3})
        self.assertEqual(x.map_recursive_ref, {1: bar, 2: baz})

    def test_shared_ref(self) -> None:
        bar, baz = ComplexRef(name="bar"), ComplexRef(name="baz")
        x = ComplexRef(name="foo", list_shared_ref=[bar, baz])
        self.assertEqual(x.list_shared_ref, (bar, baz))

    def test_const_shared_ref(self) -> None:
        bar, baz = ComplexRef(name="bar"), ComplexRef(name="baz")
        x = ComplexRef(name="foo", set_const_shared_ref={bar, baz})
        self.assertEqual(x.set_const_shared_ref, {bar, baz})

    def test_recursive(self) -> None:
        bar = ComplexRef(name="bar")
        baz = ComplexRef(recursive=bar)
        self.assertEqual(baz.recursive, bar)
