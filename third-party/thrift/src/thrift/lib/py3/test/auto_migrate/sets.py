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

import copy
import unittest
from typing import AbstractSet, Sequence, Tuple

import thrift.python.types as python_types

from testing.types import (
    Color,
    ColorGroups,
    Set__Color,
    Set__i32,
    Set__string,
    SetI32,
    SetI32Lists,
    SetSetI32Lists,
    SetTypes,
)
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import (
    brokenInAutoMigrate,
    is_auto_migrated,
)

from thrift.lib.python.test.testing_utils import Untruthy
from thrift.py3.types import Container, Set as Py3Set
from thrift.python.types import Set as PythonSet


class SetTests(unittest.TestCase):
    def test_and(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = Set__i32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x & y, set(x) & set(y))
        self.assertEqual(y & x, set(y) & set(x))
        self.assertEqual(x & z, set(x) & set(y))
        self.assertEqual(z & x, set(y) & set(x))
        self.assertEqual(x.intersection(y), set(x) & set(y))

    def test_or(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = Set__i32({1, 2, 4, 6})
        z = {1, 3, 4, 5}
        self.assertEqual(x | y, set(x) | set(y))
        self.assertEqual(y | x, set(y) | set(x))
        self.assertEqual(z | y, set(x) | set(y))
        self.assertEqual(y | z, set(y) | set(x))
        self.assertEqual(x.union(y), set(x) | set(y))

    def test_xor(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = Set__i32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x ^ y, set(x) ^ set(y))
        self.assertEqual(y ^ x, set(y) ^ set(x))
        self.assertEqual(z ^ x, set(y) ^ set(x))
        self.assertEqual(x ^ z, set(x) ^ set(y))
        self.assertEqual(x.symmetric_difference(y), set(x) ^ set(y))

    def test_sub(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = Set__i32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x - y, set(x) - set(y))
        self.assertEqual(y - x, set(y) - set(x))
        self.assertEqual(z - x, set(y) - set(x))
        self.assertEqual(x - z, set(x) - set(y))
        self.assertEqual(x.difference(y), set(x) - set(y))

    def test_isinstance(self) -> None:
        self.assertIsInstance(Set__i32({1, 2, 4}), Py3Set)
        self.assertTrue(issubclass(Set__i32, Py3Set))
        self.assertIsInstance(Set__i32({1, 2, 4}), Set__i32)
        color_set = {Color.red, Color.blue}
        self.assertIsInstance(ColorGroups(color_set=color_set).color_set, Py3Set)
        self.assertTrue(
            issubclass(ColorGroups(color_set=color_set).color_set.__class__, Py3Set)
        )
        self.assertIsInstance(ColorGroups(color_set=color_set).color_set, Set__Color)
        self.assertIsInstance(SetI32({1, 2, 4}), Py3Set)
        self.assertIsInstance(SetI32({1, 2, 4}), Set__i32)

        if is_auto_migrated():
            self.assertIsInstance(Set__i32({1, 2, 4}), PythonSet)
            self.assertIsInstance(ColorGroups(color_set=color_set).color_set, PythonSet)
            self.assertIsInstance(SetI32({1, 2, 4}), PythonSet)

        self.assertNotIsInstance(Set__i32({1, 2, 4}), Set__Color)
        self.assertNotIsInstance(Set__string({f"str_{i}" for i in range(5)}), Set__i32)

    def test_comparisons(self) -> None:
        x = SetI32({1, 2, 3, 4})
        y = Set__i32({1, 2, 3})
        z = SetI32({1, 2, 4})
        x2 = copy.copy(x)
        y2 = {1, 2, 3}

        def eq(t: AbstractSet[int], s: AbstractSet[int]) -> Tuple[bool, ...]:
            return (t == s, set(t) == s, t == set(s), set(t) == set(s))

        def neq(t: AbstractSet[int], s: AbstractSet[int]) -> Tuple[bool, ...]:
            return (t != s, set(t) != s, t != set(s), set(t) != set(s))

        def lt(t: AbstractSet[int], s: AbstractSet[int]) -> Tuple[bool, ...]:
            return (t < s, set(t) < s, t < set(s), set(t) < set(s))

        def gt(t: AbstractSet[int], s: AbstractSet[int]) -> Tuple[bool, ...]:
            return (t > s, set(t) > s, t > set(s), set(t) > set(s))

        def le(t: AbstractSet[int], s: AbstractSet[int]) -> Tuple[bool, ...]:
            return (t <= s, set(t) <= s, t <= set(s), set(t) <= set(s))

        def ge(t: AbstractSet[int], s: AbstractSet[int]) -> Tuple[bool, ...]:
            return (t >= s, set(t) >= s, t >= set(s), set(t) >= set(s))

        # = and != testing
        self.assertTrue(all(eq(x, x2)))
        self.assertTrue(all(eq(y2, y)))
        self.assertTrue(all(neq(x, y)))
        self.assertFalse(any(eq(x, y)))
        self.assertFalse(any(neq(x, x2)))
        self.assertFalse(any(neq(y2, y)))

        # lt
        self.assertTrue(all(lt(y, x)))
        self.assertFalse(any(lt(x, y)))
        self.assertFalse(any(lt(x, x2)))
        self.assertFalse(any(lt(y2, y)))
        self.assertFalse(any(lt(y, z)))
        self.assertFalse(any(lt(z, y)))

        # gt
        self.assertTrue(all(gt(x, y)))
        self.assertFalse(any(gt(y, x)))
        self.assertFalse(any(gt(x, x2)))
        self.assertFalse(any(gt(y2, y)))
        self.assertFalse(any(gt(y, z)))
        self.assertFalse(any(gt(z, y)))

        # le
        self.assertTrue(all(le(y, x)))
        self.assertFalse(any(le(x, y)))
        self.assertTrue(all(le(x, x2)))
        self.assertTrue(all(le(y2, y)))
        self.assertFalse(any(le(y, z)))
        self.assertFalse(any(le(z, y)))

        # ge
        self.assertTrue(all(ge(x, y)))
        self.assertFalse(any(ge(y, x)))
        self.assertTrue(all(ge(x, x2)))
        self.assertTrue(all(ge(y2, y)))
        self.assertFalse(any(ge(y, z)))
        self.assertFalse(any(ge(z, y)))

    def test_subset_superset(self) -> None:
        x = SetI32({0, 1})
        y = SetI32({1, 2})
        w = SetI32({2, 1})
        z = SetI32({0})
        # subset
        self.assertFalse(x.issubset(y))
        self.assertTrue(z.issubset(x))
        self.assertFalse(x.issubset(z))
        self.assertTrue(w.issubset(y))
        # superset
        self.assertFalse(x.issuperset(y))
        self.assertTrue(x.issuperset(z))
        self.assertFalse(z.issuperset(x))
        self.assertTrue(w.issuperset(y))

    def test_default_ctor(self) -> None:
        self.assertEqual(SetI32(), set())
        self.assertEqual(Set__i32(), set())

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `Optional[AbstractSet[Sequence[int]]]` for 1st
            #  param but got `Set[None]`.
            SetI32Lists({None})
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected
            #  `Optional[AbstractSet[AbstractSet[Sequence[int]]]]` for 1st param but
            #  got `Set[typing.Set[None]]`.
            SetSetI32Lists({{None}})

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            Set__i32().__dict__

    def test_empty(self) -> None:
        SetI32Lists(set())
        SetI32Lists({()})
        SetSetI32Lists(set())
        SetSetI32Lists({SetI32Lists()})
        SetSetI32Lists({SetI32Lists({()})})

    def test_mixed_construction(self) -> None:
        x = SetI32Lists({(0, 1, 2), (3, 4, 5)})
        z = SetSetI32Lists({x})
        pz = set(z)
        pz.add(x)
        nx: AbstractSet[Sequence[int]] = {(9, 10, 11)}
        pz.add(SetI32Lists(nx))
        cz = SetSetI32Lists(pz)
        self.assertIn(nx, cz)
        # pyre-fixme[6]: Expected `AbstractSet[Sequence[int]]` for 1st param but got
        #  `int`.
        pz.add(5)
        with self.assertRaises(TypeError):
            SetSetI32Lists(pz)

    def test_create_untruthy_set(self) -> None:
        with self.assertRaisesRegex(ValueError, "Do not dare question my truth"):
            bool(Untruthy(5))

        self.assertEqual(SetI32(Untruthy(5)), set(range(5)))
        self.assertEqual(SetTypes(second=Untruthy(5)).second, set(range(5)))

    def test_create_set_str_field(self) -> None:
        s = SetTypes(first={"foo"})
        self.assertIn("foo", s.first)
        self.assertEqual(len(s.first), 1)

        self.assertTrue(s.first)
        self.assertEqual(s.first, {"foo"})
        self.assertEqual(s.first, SetTypes(first={"foo"}).first)

        s = SetTypes(first={"foo", "bar"})
        self.assertIn("foo", s.first)
        self.assertIn("bar", s.first)
        self.assertEqual(len(s.first), 2)
        self.assertTrue(s.first)
        self.assertEqual(s.first, {"foo", "bar"})
        self.assertEqual(s.first, SetTypes(first={"foo", "bar"}).first)

        self.assertEqual(SetTypes().first, set())
        self.assertEqual(SetTypes().first or "foo", "foo")
        self.assertEqual(SetTypes().first or set(), SetTypes().first)

    def test_hashability(self) -> None:
        hash(SetI32Lists())
        z = SetSetI32Lists({SetI32Lists({(1, 2, 3)})})
        hash(z)
        for sub_set in z:
            hash(sub_set)

    def test_set_op_return_type(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = SetI32({1, 2, 4, 6})
        expected_type = python_types.Set if is_auto_migrated() else SetI32

        self.assertIsInstance(x & y, expected_type)
        self.assertIsInstance(x | y, expected_type)
        self.assertIsInstance(x ^ y, expected_type)
        self.assertIsInstance(x - y, expected_type)

        self.assertIsInstance(x.__rand__(y), expected_type)
        self.assertIsInstance(x.__ror__(y), expected_type)
        self.assertIsInstance(x.__rxor__(y), expected_type)
        self.assertIsInstance(x.__rsub__(y), expected_type)

    def test_is_container(self) -> None:
        self.assertIsInstance(SetI32Lists(), Container)
        self.assertIsInstance(SetSetI32Lists(), Container)
        self.assertIsInstance(SetI32(), Container)

    @brokenInAutoMigrate()
    def test_set_op_type_error_thrift_set(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = SetI32Lists({(1, 2), (4, 6)})
        with self.assertRaises(TypeError):
            x & y
        with self.assertRaises(TypeError):
            x | y
        with self.assertRaises(TypeError):
            x ^ y
        with self.assertRaises(TypeError):
            x - y

    @brokenInAutoMigrate()
    def test_set_op_type_error_raw_set(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = {"lol", "woops"}
        with self.assertRaises(TypeError):
            x & y
        with self.assertRaises(TypeError):
            x | y
        with self.assertRaises(TypeError):
            x ^ y
        with self.assertRaises(TypeError):
            x - y
