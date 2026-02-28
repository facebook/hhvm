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

import collections.abc
import pickle
import unittest
from typing import Iterable

from python_test.sets.thrift_mutable_types import SetI32
from thrift.python.mutable_containers import MutableSet
from thrift.python.mutable_types import to_thrift_set
from thrift.python.types import typeinfo_i32


def _create_MutableSet_i32(iterable: Iterable[int]) -> MutableSet[int]:
    """
    Helper function to create and return a `MutableSet`.
    The return type annotation enables Pyre type checking support.
    """
    return MutableSet._from_iterable(typeinfo_i32, set(), iterable)


class MutableSetTest(unittest.TestCase):
    """
    Some of the tests use a Python `set` for verification. They create a
    `MutableSet` and a Python `set`, apply the same operations to both of
    them, and check if they are equal.
    """

    def test_smoke(self) -> None:
        mutable_set = MutableSet(typeinfo_i32, set())
        self.assertIsInstance(mutable_set, MutableSet)
        self.assertIsInstance(mutable_set, collections.abc.MutableSet)
        self.assertEqual(0, len(mutable_set))
        self.assertFalse(mutable_set)

    def test_init(self) -> None:
        with self.assertRaisesRegex(
            TypeError, r"incorrect type \(expected set, got list\)"
        ):
            # pyre-ignore[6]: Incompatible parameter type
            MutableSet(typeinfo_i32, [])

        with self.assertRaisesRegex(
            TypeError, r"incorrect type \(expected set, got dict\)"
        ):
            # pyre-ignore[6]: Incompatible parameter type
            MutableSet(typeinfo_i32, {})

        with self.assertRaisesRegex(
            TypeError, r"incorrect type \(expected set, got frozenset\)"
        ):
            # pyre-ignore[6]: Incompatible parameter type
            MutableSet(typeinfo_i32, frozenset())

    def test_contains(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))

        for i in range(10):
            self.assertIn(i, mutable_set)

        self.assertNotIn(10, mutable_set)

    def test_contains_wrong_type(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        self.assertIn(1, mutable_set)
        self.assertNotIn("Not an interger", mutable_set)

    def test_contains_i32_overflow(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        self.assertIn(1, mutable_set)
        self.assertNotIn(2**31, mutable_set)

    def test_iter(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        python_set = set(range(10))

        for i in mutable_set:
            # `remove()` throws `KeyError` if key is absent
            python_set.remove(i)

        self.assertEqual(0, len(python_set))

    def test_iter_next(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        iter1 = iter(mutable_set)
        iter2 = iter(mutable_set)

        for _ in range(10):
            next(iter1)

        with self.assertRaises(StopIteration):
            next(iter1)

        for _ in range(10):
            next(iter2)

        with self.assertRaises(StopIteration):
            next(iter2)

    def test_isdisjoint(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(10))
        mutable_set_2 = _create_MutableSet_i32(range(1))
        self.assertFalse(mutable_set_1.isdisjoint(mutable_set_2))

        mutable_set_3 = _create_MutableSet_i32(range(10, 20))
        self.assertTrue(mutable_set_1.isdisjoint(mutable_set_3))

        self.assertFalse(mutable_set_1.isdisjoint(range(3)))
        self.assertTrue(mutable_set_1.isdisjoint(range(10, 11)))
        self.assertTrue(mutable_set_1.isdisjoint([10]))
        self.assertTrue(mutable_set_1.isdisjoint(frozenset([10])))

    def test_eq(self) -> None:
        mutable_set_1 = _create_MutableSet_i32([1, 2])
        mutable_set_2 = _create_MutableSet_i32([1, 2, 3])

        self.assertEqual({1, 2}, mutable_set_1)
        self.assertEqual(mutable_set_1, {1, 2})
        self.assertNotEqual({1, 2, 3}, mutable_set_1)
        self.assertNotEqual(mutable_set_1, {1, 2, 3})

        self.assertEqual({1, 2, 3}, mutable_set_2)
        self.assertEqual(mutable_set_2, {1, 2, 3})
        self.assertNotEqual({1, 2}, mutable_set_2)
        self.assertNotEqual(mutable_set_2, {1, 2})

        self.assertNotEqual(mutable_set_1, mutable_set_2)
        self.assertNotEqual(mutable_set_2, mutable_set_1)

    def test_and(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 6))

        result_set = mutable_set_1 & mutable_set_2

        self.assertIsInstance(result_set, MutableSet)
        self.assertIsNot(result_set, mutable_set_1)
        self.assertIsNot(result_set, mutable_set_2)
        self.assertEqual(2, len(result_set))
        self.assertEqual({2, 3}, result_set)

        result_set = mutable_set_1 & [1, 2]
        self.assertEqual(2, len(result_set))
        self.assertEqual({1, 2}, result_set)

    def test_or(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 6))

        result_set = mutable_set_1 | mutable_set_2

        self.assertIsInstance(result_set, MutableSet)
        self.assertIsNot(result_set, mutable_set_1)
        self.assertTrue(result_set is not mutable_set_2)
        self.assertEqual(6, len(result_set))
        self.assertEqual({0, 1, 2, 3, 4, 5}, result_set)

        result_set = mutable_set_1 | [10, 11, 12, 13]
        self.assertEqual(8, len(result_set))
        self.assertEqual({0, 1, 2, 3, 10, 11, 12, 13}, result_set)

    def test_sub(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 7))

        result_set = mutable_set_1 - mutable_set_2
        self.assertIsInstance(result_set, MutableSet)
        self.assertEqual(2, len(result_set))
        self.assertEqual({0, 1}, result_set)

        result_set = mutable_set_2 - mutable_set_1
        self.assertIsInstance(result_set, MutableSet)
        self.assertEqual(3, len(result_set))
        self.assertEqual({4, 5, 6}, result_set)

        result_set = mutable_set_1 - [1, 5, 6]
        self.assertIsInstance(result_set, MutableSet)
        self.assertEqual(3, len(result_set))
        self.assertEqual({0, 2, 3}, result_set)

    def test_xor(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 7))

        result_set = mutable_set_1 ^ mutable_set_2
        self.assertIsInstance(result_set, MutableSet)
        self.assertEqual(5, len(result_set))
        self.assertEqual({0, 1, 4, 5, 6}, result_set)

        result_set = mutable_set_1 ^ [1, 2, 3, 4]
        self.assertIsInstance(result_set, MutableSet)
        self.assertEqual(2, len(result_set))
        self.assertEqual({0, 4}, result_set)

    def test_add(self) -> None:
        mutable_set = _create_MutableSet_i32([])
        python_set = set()

        for i in range(10):
            mutable_set.add(i)
            python_set.add(i)

        self.assertEqual(10, len(mutable_set))
        self.assertEqual(python_set, mutable_set)

    def test_add_wrong_type(self) -> None:
        mutable_set = _create_MutableSet_i32([])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_set.add("Not an interger")

    def test_add_i32_overflow(self) -> None:
        mutable_set = _create_MutableSet_i32([])
        with self.assertRaises(OverflowError):
            mutable_set.add(2**31)

    def test_discard(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        python_set = set(range(10))

        for i in range(1, 10, 2):
            mutable_set.discard(i)
            python_set.discard(i)

        self.assertEqual(5, len(mutable_set))
        self.assertEqual(python_set, mutable_set)

    def test_discard_wrong_type(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        # Discard doesn't raise an error, should it?
        # pyre-ignore[6]: Intentional for test
        mutable_set.discard("Not an integer")

    def test_discard_i32_overflow(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))
        # Discard doesn't raise an error, should it?
        mutable_set.discard(2**31)

    def test_remove(self) -> None:
        mutable_set = _create_MutableSet_i32(range(3))

        # `remove()` raises an `KeyError` if key is absent
        with self.assertRaisesRegex(KeyError, "999"):
            mutable_set.remove(999)

        mutable_set.remove(2)
        mutable_set.remove(1)
        mutable_set.remove(0)

        self.assertEqual(0, len(mutable_set))

    def test_remove_wrong_type(self) -> None:
        mutable_set = _create_MutableSet_i32(range(3))
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_set.remove("Not an interger")

    def test_remove_i32_overflow(self) -> None:
        mutable_set = _create_MutableSet_i32(range(3))
        with self.assertRaises(OverflowError):
            mutable_set.remove(2**31)

    def test_set_op_return_type(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 6))

        self.assertIsInstance(mutable_set_1 & mutable_set_2, MutableSet)
        self.assertIsInstance(mutable_set_1 | mutable_set_2, MutableSet)
        self.assertIsInstance(mutable_set_1 ^ mutable_set_2, MutableSet)
        self.assertIsInstance(mutable_set_1 - mutable_set_2, MutableSet)

    def test_pop(self) -> None:
        mutable_set = _create_MutableSet_i32(range(3))

        mutable_set.pop()
        mutable_set.pop()
        mutable_set.pop()

        # Raises `KeyError` if empty
        with self.assertRaises(KeyError):
            mutable_set.pop()

    def test_clear(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))

        for i in range(10):
            mutable_set.add(i)

        mutable_set.clear()
        self.assertEqual(0, len(mutable_set))

    def test_pickle_round_trip(self) -> None:
        mutable_set = _create_MutableSet_i32(range(10))

        pickled = pickle.dumps(mutable_set, protocol=pickle.HIGHEST_PROTOCOL)
        mutable_set_unpickled = pickle.loads(pickled)
        self.assertIsInstance(mutable_set_unpickled, MutableSet)
        self.assertEqual(mutable_set, mutable_set_unpickled)

    def test_ior(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 6))

        mutable_set_1 |= mutable_set_2

        self.assertEqual(6, len(mutable_set_1))
        self.assertEqual({0, 1, 2, 3, 4, 5}, mutable_set_1)

        mutable_set_2 |= {10, 11, 12, 13}
        self.assertEqual(8, len(mutable_set_2))
        self.assertEqual({2, 3, 4, 5, 10, 11, 12, 13}, mutable_set_2)

    def test_ior_exception(self) -> None:
        mutable_set = _create_MutableSet_i32(range(3))

        # basic exception safety
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # the list is necessary to guarantee the order of the elements
            # pyre-ignore[6]: Intentional for test
            mutable_set |= [10, 11, "Not an Integer", 13, 14]

        self.assertEqual(5, len(mutable_set))
        self.assertEqual({0, 1, 2, 10, 11}, mutable_set)

    def test_iand(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 6))

        mutable_set_1 &= mutable_set_2

        self.assertEqual(2, len(mutable_set_1))
        self.assertEqual({2, 3}, mutable_set_1)

        mutable_set_1 &= {1, 2}
        self.assertEqual(1, len(mutable_set_1))
        self.assertEqual({2}, mutable_set_1)

    def test_iand_exception(self) -> None:
        mutable_set = _create_MutableSet_i32(range(4))

        # No exception is raised, the non-integers are ignored, and only valid
        # integers are processed
        mutable_set &= {0, "Not an Integer", 1, 2, 5, 6}
        self.assertEqual(3, len(mutable_set))
        self.assertEqual({0, 1, 2}, mutable_set)

    def test_ixor(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 7))

        mutable_set_1 ^= mutable_set_2
        self.assertEqual(5, len(mutable_set_1))
        self.assertEqual({0, 1, 4, 5, 6}, mutable_set_1)

        mutable_set_1 ^= {4, 5, 6, 7}
        self.assertEqual(3, len(mutable_set_1))
        self.assertEqual({0, 1, 7}, mutable_set_1)

    def test_ixor_exception(self) -> None:
        mutable_set = _create_MutableSet_i32(range(4))

        # strong exception safety
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_set ^= {10, 11, "Not an Integer", 13, 14}

        self.assertEqual(4, len(mutable_set))
        self.assertEqual({0, 1, 2, 3}, mutable_set)

    def test_isub(self) -> None:
        mutable_set_1 = _create_MutableSet_i32(range(4))
        mutable_set_2 = _create_MutableSet_i32(range(2, 7))

        mutable_set_1 -= mutable_set_2
        self.assertEqual(2, len(mutable_set_1))
        self.assertEqual({0, 1}, mutable_set_1)

        mutable_set_3 = _create_MutableSet_i32(range(4))
        mutable_set_4 = _create_MutableSet_i32(range(2, 7))

        mutable_set_4 -= mutable_set_3
        self.assertEqual(3, len(mutable_set_4))
        self.assertEqual({4, 5, 6}, mutable_set_4)

        mutable_set_5 = _create_MutableSet_i32(range(4))
        mutable_set_5 -= {1, 5, 6}
        self.assertEqual(3, len(mutable_set_5))
        self.assertEqual({0, 2, 3}, mutable_set_5)

    def test_isub_exception(self) -> None:
        mutable_set = _create_MutableSet_i32(range(4))

        # No exception is raised, the non-integers are ignored, and only valid
        # integers are processed
        mutable_set -= {0, "Not an Integer", 1, 5, 6}
        self.assertEqual(2, len(mutable_set))
        self.assertEqual({2, 3}, mutable_set)

    def test_le(self) -> None:
        mutable_set_1 = _create_MutableSet_i32([1, 2])
        mutable_set_2 = _create_MutableSet_i32([1, 2])
        mutable_set_3 = _create_MutableSet_i32([1, 2, 3])
        mutable_set_4 = _create_MutableSet_i32([2, 3])

        self.assertTrue(mutable_set_1 <= mutable_set_2)
        self.assertTrue(mutable_set_2 <= mutable_set_1)

        self.assertTrue(mutable_set_1 <= mutable_set_3)
        self.assertFalse(mutable_set_3 <= mutable_set_1)

        self.assertFalse(mutable_set_1 <= mutable_set_4)
        self.assertFalse(mutable_set_4 <= mutable_set_1)

        self.assertTrue(mutable_set_1 <= {1, 2})
        self.assertTrue(mutable_set_1 <= {1, 2, 3})
        self.assertFalse(mutable_set_1 <= {2, 3})

        with self.assertRaisesRegex(
            TypeError,
            "'<=' not supported between instances of '.*.MutableSet' and 'list'",
        ):
            _ = mutable_set_1 <= [1, 2]

    def test_lt(self) -> None:
        mutable_set_1 = _create_MutableSet_i32([1, 2])
        mutable_set_2 = _create_MutableSet_i32([1, 2])
        mutable_set_3 = _create_MutableSet_i32([1, 2, 3])
        mutable_set_4 = _create_MutableSet_i32([2, 3])

        self.assertFalse(mutable_set_1 < mutable_set_2)
        self.assertFalse(mutable_set_2 < mutable_set_1)

        self.assertTrue(mutable_set_1 < mutable_set_3)
        self.assertFalse(mutable_set_3 < mutable_set_1)

        self.assertFalse(mutable_set_1 < mutable_set_4)
        self.assertFalse(mutable_set_4 < mutable_set_1)

        self.assertFalse(mutable_set_1 < {1, 2})
        self.assertTrue(mutable_set_1 < {1, 2, 3})
        self.assertFalse(mutable_set_1 < {2, 3})

        with self.assertRaisesRegex(
            TypeError,
            "'<' not supported between instances of '.*.MutableSet' and 'list'",
        ):
            _ = mutable_set_1 < [1, 2]

    def test_ge(self) -> None:
        mutable_set_1 = _create_MutableSet_i32([1, 2, 3])
        mutable_set_2 = _create_MutableSet_i32([1, 2, 3])
        mutable_set_3 = _create_MutableSet_i32([1, 2])
        mutable_set_4 = _create_MutableSet_i32([2, 3, 4])

        self.assertTrue(mutable_set_1 >= mutable_set_2)
        self.assertTrue(mutable_set_2 >= mutable_set_1)

        self.assertTrue(mutable_set_1 >= mutable_set_3)
        self.assertFalse(mutable_set_3 >= mutable_set_1)

        self.assertFalse(mutable_set_1 >= mutable_set_4)
        self.assertFalse(mutable_set_4 >= mutable_set_1)

        self.assertTrue(mutable_set_1 >= {1, 2, 3})
        self.assertTrue(mutable_set_1 >= {1, 2})
        self.assertFalse(mutable_set_1 >= {2, 3, 4})

        with self.assertRaisesRegex(
            TypeError,
            "'>=' not supported between instances of '.*.MutableSet' and 'list'",
        ):
            _ = mutable_set_1 >= [1, 2]

    def test_gt(self) -> None:
        mutable_set_1 = _create_MutableSet_i32([1, 2, 3])
        mutable_set_2 = _create_MutableSet_i32([1, 2, 3])
        mutable_set_3 = _create_MutableSet_i32([1, 2])
        mutable_set_4 = _create_MutableSet_i32([2, 3, 4])

        self.assertFalse(mutable_set_1 > mutable_set_2)
        self.assertFalse(mutable_set_2 > mutable_set_1)

        self.assertTrue(mutable_set_1 > mutable_set_3)
        self.assertFalse(mutable_set_3 > mutable_set_1)

        self.assertFalse(mutable_set_1 > mutable_set_4)
        self.assertFalse(mutable_set_4 > mutable_set_1)

        self.assertFalse(mutable_set_1 > {1, 2, 3})
        self.assertTrue(mutable_set_1 > {1, 2})
        self.assertFalse(mutable_set_1 > {2, 3, 4})

        with self.assertRaisesRegex(
            TypeError,
            "'>' not supported between instances of '.*.MutableSet' and 'list'",
        ):
            _ = mutable_set_1 > [1, 2]


class MutableSetTypedefTest(unittest.TestCase):
    TYPE_ERROR_MESSAGE = (
        "Expected values to be an instance of Thrift mutable set with matching "
        r"element type, or the result of `to_thrift_set\(\)`, but got type "
        r"<class 'set'>."
    )

    def test_set_i32(self) -> None:
        """
        typedef set<i32> SetI32
        """
        # Mutable container typedef should be initialized with the same mutable
        # container type, or it should be wrapped with `to_thrift_set()`.
        # Otherwise, it will result in both runtime and Pyre errors.
        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-fixme[20]: Argument `set_data` expected.
            _ = SetI32({1, 2, 3})

        # Initialize with `to_thrift_set()` and verify that the initial
        # Python set `s` and the MutableSet `seti32` are separate sets.
        s = {1, 2, 3}
        # pyre-fixme[20]: Argument `set_data` expected.
        seti32: MutableSet[int] = SetI32(to_thrift_set(s))
        self.assertEqual({1, 2, 3}, seti32)

        s.remove(1)
        self.assertEqual({2, 3}, s)
        self.assertEqual({1, 2, 3}, seti32)
