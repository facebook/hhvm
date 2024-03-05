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

from testing.types import SetI32, SetI32Lists, SetSetI32Lists
from thrift.py3.types import Container


class SetTests(unittest.TestCase):
    def test_and(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = SetI32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x & y, set(x) & set(y))
        self.assertEqual(y & x, set(y) & set(x))
        self.assertEqual(x & z, set(x) & set(y))
        self.assertEqual(z & x, set(y) & set(x))
        self.assertEqual(x.intersection(y), set(x) & set(y))

    def test_or(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = SetI32({1, 2, 4, 6})
        z = {1, 3, 4, 5}
        self.assertEqual(x | y, set(x) | set(y))
        self.assertEqual(y | x, set(y) | set(x))
        self.assertEqual(z | y, set(x) | set(y))
        self.assertEqual(y | z, set(y) | set(x))
        self.assertEqual(x.union(y), set(x) | set(y))

    def test_xor(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = SetI32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x ^ y, set(x) ^ set(y))
        self.assertEqual(y ^ x, set(y) ^ set(x))
        self.assertEqual(z ^ x, set(y) ^ set(x))
        self.assertEqual(x ^ z, set(x) ^ set(y))
        self.assertEqual(x.symmetric_difference(y), set(x) ^ set(y))

    def test_sub(self) -> None:
        x = SetI32({1, 3, 4, 5})
        y = SetI32({1, 2, 4, 6})
        z = {1, 2, 4, 6}
        self.assertEqual(x - y, set(x) - set(y))
        self.assertEqual(y - x, set(y) - set(x))
        self.assertEqual(z - x, set(y) - set(x))
        self.assertEqual(x - z, set(x) - set(y))
        self.assertEqual(x.difference(y), set(x) - set(y))

    def test_comparisons(self) -> None:
        x = SetI32({1, 2, 3, 4})
        y = SetI32({1, 2, 3})
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
            # pyre-fixme[6]: Expected `Optional[AbstractSet[Sequence[int]]]` for 1st
            #  param but got `Set[None]`.
            SetI32Lists({None})
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected
            #  `Optional[AbstractSet[AbstractSet[Sequence[int]]]]` for 1st param but
            #  got `Set[typing.Set[None]]`.
            SetSetI32Lists({{None}})

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

    def test_hashability(self) -> None:
        hash(SetI32Lists())
        z = SetSetI32Lists({SetI32Lists({(1, 2, 3)})})
        hash(z)
        for sub_set in z:
            hash(sub_set)

    def test_is_container(self) -> None:
        self.assertIsInstance(SetI32Lists(), Container)
        self.assertIsInstance(SetSetI32Lists(), Container)
        self.assertIsInstance(SetI32(), Container)
