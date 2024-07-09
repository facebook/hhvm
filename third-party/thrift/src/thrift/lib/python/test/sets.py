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

import copy
import unittest
from typing import AbstractSet, Sequence, Tuple, Type

import python_test.containers.thrift_mutable_types as mutable_containers_types
import python_test.containers.thrift_types as immutable_containers_types

import python_test.sets.thrift_mutable_types as immutable_sets_types
import python_test.sets.thrift_types as mutable_sets_types

from folly.iobuf import IOBuf

from parameterized import parameterized_class

from python_test.containers.thrift_types import Color, Foo, Sets
from python_test.sets.thrift_types import (
    easy,
    EasySet,
    SetI32,
    SetI32Lists,
    SetSetI32Lists,
)


@parameterized_class(
    ("containers_types", "sets_types"),
    [
        (immutable_containers_types, immutable_sets_types),
        (mutable_containers_types, mutable_sets_types),
    ],
)
class SetTests(unittest.TestCase):
    def setUp(self) -> None:
        # pyre-ignore[16]: has no attribute `sets_types`
        self.easy: Type[easy] = self.sets_types.easy
        self.EasySet: Type[EasySet] = self.sets_types.EasySet
        self.SetI32: Type[SetI32] = self.sets_types.SetI32
        self.SetI32Lists: Type[SetI32Lists] = self.sets_types.SetI32Lists
        self.SetSetI32Lists: Type[SetSetI32Lists] = self.sets_types.SetSetI32Lists
        # pyre-ignore[16]: has no attribute `containers_types`
        self.Foo: Type[Foo] = self.containers_types.Foo
        self.Sets: Type[Sets] = self.containers_types.Sets
        self.Color: Type[Color] = self.containers_types.Color

    def test_and(self) -> None:
        x = self.SetI32({1, 3, 4, 5})
        y = self.SetI32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x & y, set(x) & set(y))
        self.assertEqual(y & x, set(y) & set(x))
        self.assertEqual(x & z, set(x) & set(y))
        self.assertEqual(z & x, set(y) & set(x))
        self.assertEqual(x.intersection(y), set(x) & set(y))

    def test_or(self) -> None:
        x = self.SetI32({1, 3, 4, 5})
        y = self.SetI32({1, 2, 4, 6})
        z = {1, 3, 4, 5}
        self.assertEqual(x | y, set(x) | set(y))
        self.assertEqual(y | x, set(y) | set(x))
        self.assertEqual(z | y, set(x) | set(y))
        self.assertEqual(y | z, set(y) | set(x))
        self.assertEqual(x.union(y), set(x) | set(y))

    def test_xor(self) -> None:
        x = self.SetI32({1, 3, 4, 5})
        y = self.SetI32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x ^ y, set(x) ^ set(y))
        self.assertEqual(y ^ x, set(y) ^ set(x))
        self.assertEqual(z ^ x, set(y) ^ set(x))
        self.assertEqual(x ^ z, set(x) ^ set(y))
        self.assertEqual(x.symmetric_difference(y), set(x) ^ set(y))

    def test_sub(self) -> None:
        x = self.SetI32({1, 3, 4, 5})
        y = self.SetI32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x - y, set(x) - set(y))
        self.assertEqual(y - x, set(y) - set(x))
        self.assertEqual(z - x, set(y) - set(x))
        self.assertEqual(x - z, set(x) - set(y))
        self.assertEqual(x.difference(y), set(x) - set(y))

    def test_contains_enum(self) -> None:
        cset = self.Sets(colorSet={self.Color.red, self.Color.blue})
        self.assertIn(self.Color.red, cset.colorSet)
        self.assertIn(self.Color.blue, cset.colorSet)
        self.assertNotIn(self.Color.green, cset.colorSet)

    def test_comparisons(self) -> None:
        x = self.SetI32({1, 2, 3, 4})
        y = self.SetI32({1, 2, 3})
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

        # gt
        self.assertTrue(all(gt(x, y)))
        self.assertFalse(any(gt(y, x)))
        self.assertFalse(any(gt(x, x2)))
        self.assertFalse(any(gt(y2, y)))

        # le
        self.assertTrue(all(le(y, x)))
        self.assertFalse(any(le(x, y)))
        self.assertTrue(all(le(x, x2)))
        self.assertTrue(all(le(y2, y)))

        # ge
        self.assertTrue(all(ge(x, y)))
        self.assertFalse(any(ge(y, x)))
        self.assertTrue(all(ge(x, x2)))
        self.assertTrue(all(ge(y2, y)))

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.SetI32Lists({None})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.SetSetI32Lists({{None}})

    def test_struct_with_set_fields(self) -> None:
        # pyre-ignore[16]: has no attribute `lists_types`
        is_immutable = self.sets_types.__name__.endswith("immutable_types")

        s = self.Sets(
            boolSet={True, False},
            byteSet={1, 2, 3},
            i16Set={4, 5, 6},
            i64Set={7, 8, 9},
            doubleSet={1.23, 4.56},
            floatSet={7.89, 10.11},
            stringSet={"foo", "bar"},
            binarySet={b"foo", b"bar"},
            iobufSet={IOBuf(b"foo"), IOBuf(b"bar")},
            structSet=(
                {self.Foo(value=1), self.Foo(value=2)} if is_immutable else set()
            ),
        )
        self.assertEqual(s.boolSet, {True, False})
        self.assertEqual(s.byteSet, {1, 2, 3})
        self.assertEqual(s.i16Set, {4, 5, 6})
        self.assertEqual(s.i64Set, {7, 8, 9})
        self.assertEqual(s.doubleSet, {1.23, 4.56})
        self.assertEqual(s.floatSet, {7.89, 10.11})
        self.assertEqual(s.stringSet, {"foo", "bar"})
        self.assertEqual(s.binarySet, {b"foo", b"bar"})
        self.assertEqual(s.iobufSet, {IOBuf(b"foo"), IOBuf(b"bar")})
        if is_immutable:
            self.assertEqual(s.structSet, {Foo(value=1), Foo(value=2)})
            # test reaccess the set element won't have to recreating the struct
            structs1 = list(s.structSet)
            structs2 = list(s.structSet)
            self.assertIs(structs1[0], structs2[0])
            self.assertIs(structs1[1], structs2[1])


class ImmutableSetTests(unittest.TestCase):
    """
    These tests run only for immutable types because mutable types are not
    hashable.
    """

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
        # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
        pz.add(5)
        with self.assertRaises(TypeError):
            SetSetI32Lists(pz)

    def test_hashability(self) -> None:
        hash(SetI32Lists())
        z = SetSetI32Lists({SetI32Lists({(1, 2, 3)})})
        hash(z)
        for sub_set in z:
            hash(sub_set)

    def test_struct_set(self) -> None:
        a = EasySet({easy()})
        b = EasySet({easy(val=0)})
        c = EasySet({easy(val=1)})
        d = EasySet({easy(val_list=[])})
        f = EasySet({easy(), easy(val_list=[1])})
        self.assertEqual(a, b)
        self.assertEqual(a, d)
        self.assertNotEqual(a, c)
        self.assertEqual(a & b, {easy()})
        self.assertEqual(a | b, {easy()})
        self.assertEqual(a - b, set())
        self.assertEqual(a ^ b, set())
        self.assertLess(a, f)
        self.assertGreater(f, a)
        self.assertLessEqual(a, b)
        self.assertLessEqual(a, f)
        self.assertGreaterEqual(a, d)
        self.assertGreaterEqual(f, a)


# TODO: Collapse these two test cases into parameterized test above
class SetImmutablePythonTests(unittest.TestCase):
    Color = immutable_containers_types.Color
    Sets = immutable_containers_types.Sets

    def test_contains_enum(self) -> None:
        cset = self.Sets(colorSet={self.Color.red, self.Color.blue})
        self.assertNotIn("str", cset.colorSet)

        # This behavior is more permissive than thrift-py3
        # which implicitly converts int to enum
        self.assertIn(0, cset.colorSet)
        self.assertIn(1, cset.colorSet)
        self.assertNotIn(2, cset.colorSet)


# TODO: Collapse these two test cases into parameterized test above
class SetMutablePythonTests(unittest.TestCase):
    # pyre-ignore
    Color = mutable_containers_types.Color
    # pyre-ignore
    Sets = mutable_containers_types.Sets

    # this test case documents behavior divergences from thrift-python
    @unittest.expectedFailure
    def test_contains_enum(self) -> None:
        cset = self.Sets(colorSet={self.Color.red, self.Color.blue})
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertNotIn("str", cset.colorSet)
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertIn(0, cset.colorSet)
