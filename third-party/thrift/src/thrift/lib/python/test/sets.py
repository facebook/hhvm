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
from typing import AbstractSet, Sequence, Tuple, Type, TypeVar

import python_test.containers.thrift_mutable_types as mutable_containers_types
import python_test.containers.thrift_types as immutable_containers_types
import python_test.sets.thrift_mutable_types as mutable_sets_types
import python_test.sets.thrift_types as immutable_sets_types
from folly.iobuf import IOBuf
from parameterized import parameterized_class
from python_test.containers.thrift_types import Color, Foo, Sets
from python_test.sets.thrift_types import (
    constant_set,
    easy,
    EasySet,
    SetAtoIValue,
    SetI32,
    SetI32Lists,
    SetSetI32Lists,
)
from thrift.lib.python.test.testing_utils import Untruthy
from thrift.python.mutable_types import _ThriftSetWrapper, to_thrift_set
from thrift.python.test_helpers import round_thrift_to_float32

SetT = TypeVar("SetT")


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
        self.SetAtoIValue: Type[SetAtoIValue] = self.sets_types.SetAtoIValue
        self.SetI32Lists: Type[SetI32Lists] = self.sets_types.SetI32Lists
        self.SetSetI32Lists: Type[SetSetI32Lists] = self.sets_types.SetSetI32Lists
        # pyre-ignore[16]: has no attribute `containers_types`
        self.Foo: Type[Foo] = self.containers_types.Foo
        self.Sets: Type[Sets] = self.containers_types.Sets
        self.Color: Type[Color] = self.containers_types.Color
        self.is_mutable_run: bool = self.containers_types.__name__.endswith(
            "thrift_mutable_types"
        )

    def to_set(self, set_data: set[SetT]) -> set[SetT] | _ThriftSetWrapper:
        return to_thrift_set(set_data) if self.is_mutable_run else set_data

    def test_and(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 3, 4, 5}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.SetI32(self.to_set({1, 2, 4, 6}))
        z = {1, 2, 4, 6}
        self.assertEqual(x & y, set(x) & set(y))
        self.assertEqual(y & x, set(y) & set(x))
        self.assertEqual(x & z, set(x) & set(y))
        self.assertEqual(z & x, set(y) & set(x))
        if not self.is_mutable_run:
            self.assertEqual(x.intersection(y), set(x) & set(y))

    def test_or(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 3, 4, 5}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.SetI32(self.to_set({1, 2, 4, 6}))
        z = {1, 3, 4, 5}
        self.assertEqual(x | y, set(x) | set(y))
        self.assertEqual(y | x, set(y) | set(x))
        self.assertEqual(z | y, set(x) | set(y))
        self.assertEqual(y | z, set(y) | set(x))
        if not self.is_mutable_run:
            self.assertEqual(x.union(y), set(x) | set(y))

    def test_xor(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 3, 4, 5}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.SetI32(self.to_set({1, 2, 4, 6}))
        z = {1, 2, 4, 6}
        self.assertEqual(x ^ y, set(x) ^ set(y))
        self.assertEqual(y ^ x, set(y) ^ set(x))
        self.assertEqual(z ^ x, set(y) ^ set(x))
        self.assertEqual(x ^ z, set(x) ^ set(y))
        if not self.is_mutable_run:
            self.assertEqual(x.symmetric_difference(y), set(x) ^ set(y))

    def test_sub(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 3, 4, 5}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.SetI32(self.to_set({1, 2, 4, 6}))
        z = {1, 2, 4, 6}
        self.assertEqual(x - y, set(x) - set(y))
        self.assertEqual(y - x, set(y) - set(x))
        self.assertEqual(x - z, set(x) - set(y))
        if not self.is_mutable_run:
            self.assertEqual(z - x, set(y) - set(x))
            self.assertEqual(x.difference(y), set(x) - set(y))

    def test_contains_enum(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        cset = self.Sets(colorSet=self.to_set({self.Color.red, self.Color.blue}))
        self.assertIn(self.Color.red, cset.colorSet)
        self.assertIn(self.Color.blue, cset.colorSet)
        self.assertNotIn(self.Color.green, cset.colorSet)

    def test_comparisons(self) -> None:
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 3, 4, 5}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 3, 4, 5}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        x = self.SetI32(self.to_set({1, 2, 3, 4}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        y = self.SetI32(self.to_set({1, 2, 3}))
        # pyre-ignore[6]: TODO: Thrift-Container init
        z = self.SetI32(self.to_set({1, 2, 4}))
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

    def test_None(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.SetI32Lists({None})
        with self.assertRaises(TypeError):
            # pyre-ignore[6]: purposely use a wrong type to raise a TypeError
            self.SetSetI32Lists({{None}})

    def test_no_dict(self) -> None:
        with self.assertRaises(AttributeError):
            SetI32().__dict__

    def test_create_untruthy_set(self) -> None:
        with self.assertRaises(ValueError):
            bool(Untruthy(5))

        self.assertEqual(SetI32(Untruthy(5)), set(range(5)))
        self.assertEqual(Sets(i32Set=Untruthy(5)).i32Set, set(range(5)))

    def test_struct_with_set_fields(self) -> None:
        s = self.Sets(
            # pyre-ignore[6]: TODO: Thrift-Container init
            boolSet=self.to_set({True, False}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            byteSet=self.to_set({1, 2, 3}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i16Set=self.to_set({4, 5, 6}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            i64Set=self.to_set({7, 8, 9}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            doubleSet=self.to_set({1.23, 4.56}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            floatSet=self.to_set({7.89, 10.11}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            stringSet=self.to_set({"foo", "bar"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            binarySet=self.to_set({b"foo", b"bar"}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            iobufSet=self.to_set({IOBuf(b"foo"), IOBuf(b"bar")}),
            # pyre-ignore[6]: TODO: Thrift-Container init
            structSet=(
                to_thrift_set(set())
                if self.is_mutable_run
                else {self.Foo(value=1), self.Foo(value=2)}
            ),
        )
        self.assertEqual(s.boolSet, {True, False})
        self.assertEqual(s.byteSet, {1, 2, 3})
        self.assertEqual(s.i16Set, {4, 5, 6})
        self.assertEqual(s.i64Set, {7, 8, 9})
        self.assertEqual(s.doubleSet, {1.23, 4.56})
        self.assertEqual(s.floatSet, round_thrift_to_float32({7.89, 10.11}))
        self.assertEqual(s.stringSet, {"foo", "bar"})
        self.assertEqual(s.binarySet, {b"foo", b"bar"})
        self.assertEqual(s.iobufSet, {IOBuf(b"foo"), IOBuf(b"bar")})
        if not self.is_mutable_run:
            self.assertEqual(s.structSet, {Foo(value=1), Foo(value=2)})
            # test reaccess the set element won't have to recreating the struct
            structs1 = list(s.structSet)
            structs2 = list(s.structSet)
            self.assertIs(structs1[0], structs2[0])
            self.assertIs(structs1[1], structs2[1])

    def test_adapted_sets(self) -> None:
        int_set = {1, 2, 3}

        # pyre-ignore[6]: TODO: Thrift-Container init
        self.assertEqual(int_set, self.SetAtoIValue(self.to_set(int_set)))


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

    def test_set_module_name(self) -> None:
        easy_set = EasySet({easy()})
        self.assertEqual(easy_set.__class__.__module__, "thrift.python.types")

    def test_constant_set(self) -> None:
        self.assertEqual({"1", "2", "3"}, constant_set)
        with self.assertRaisesRegex(
            AttributeError,
            "'thrift.python.types.Set' object has no attribute 'add'",
        ):
            # pyre-ignore[16]: `AbstractSet` has no attribute `add`
            constant_set.add("4")

        with self.assertRaisesRegex(
            AttributeError,
            "'thrift.python.types.Set' object has no attribute 'remove'",
        ):
            # pyre-ignore[16]: `AbstractSet` has no attribute `remove`
            constant_set.remove("3")

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
    Color = mutable_containers_types.Color
    Sets = mutable_containers_types.Sets

    # this test case documents behavior divergences from thrift-python
    @unittest.expectedFailure
    def test_contains_enum(self) -> None:
        # pyre-ignore[6]: Fixme: type error to be addressed later
        cset = self.Sets(colorSet={self.Color.red, self.Color.blue})
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertNotIn("str", cset.colorSet)
        # TODO(T194526180): mutable thrift-python should not raise
        self.assertIn(0, cset.colorSet)
