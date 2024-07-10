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

from collections.abc import MutableSequence

from thrift.python.mutable_containers import MutableList

from thrift.python.types import typeinfo_i32


class MutableListTest(unittest.TestCase):
    """
    Some of the tests use a Python `list` for verification. They create a
    `MutableList` and a Python `list`, apply the same operations to both of
    them, and check if they are equal.
    """

    def test_smoke(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        self.assertTrue(isinstance(mutable_list, MutableList))
        self.assertTrue(isinstance(mutable_list, MutableSequence))
        self.assertEqual(0, len(mutable_list))
        self.assertFalse(mutable_list)

    def test_extend(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(10))
        python_list.extend(range(10))

        self.assertEqual(python_list, mutable_list)

    def test_extend_wrong_type(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            mutable_list.extend([1, 2, "Not an integer", 3])

        # basic exception safety
        self.assertEqual([1, 2], mutable_list)

    def test_extend_i32_overflow(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        with self.assertRaises(OverflowError):
            mutable_list.extend([1, 2, 2**31, 3])

        # basic exception safety
        self.assertEqual([1, 2], mutable_list)

    def test_append(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        for i in range(100):
            mutable_list.append(i)
            python_list.append(i)

        self.assertEqual(python_list, mutable_list)

    def test_append_wrong_type(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            mutable_list.append("Not an interger")

    def test_append_i32_overflow(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        with self.assertRaises(OverflowError):
            mutable_list.append(2**31)

    def test_getitem(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.extend(range(100))

        for i in range(100):
            self.assertEqual(i, mutable_list[i])
            self.assertEqual(99 - i, mutable_list[-i - 1])

        with self.assertRaisesRegex(IndexError, "list index out of range"):
            mutable_list[100]

    def test_getitem_slice(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        self.assertEqual(python_list[1:3], mutable_list[1:3])
        self.assertEqual(python_list[9:3], mutable_list[9:3])
        self.assertEqual(python_list[9:3:-1], mutable_list[9:3:-1])
        self.assertEqual(python_list[::-1], mutable_list[::-1])
        self.assertEqual(python_list[101:201], mutable_list[101:201])

    def test_insert(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        mutable_list.insert(0, 101)
        python_list.insert(0, 101)
        self.assertEqual(python_list, mutable_list)
        mutable_list.insert(len(mutable_list), 102)
        python_list.insert(len(mutable_list), 102)
        self.assertEqual(python_list, mutable_list)
        mutable_list.insert(50, 103)
        python_list.insert(50, 103)
        self.assertEqual(python_list, mutable_list)

    def test_insert_wrong_type(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            mutable_list.insert(0, "Not an interger")

    def test_insert_i32_overflow(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        with self.assertRaises(OverflowError):
            mutable_list.insert(0, 2**31)

    def test_setitem(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        for i in range(10):
            mutable_list[i] = i * 111
            python_list[i] = i * 111

        self.assertEqual(python_list, mutable_list)

        with self.assertRaisesRegex(IndexError, "list assignment index out of range"):
            mutable_list[111] = 123

    def test_setitem_wrong_type(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.append(0)
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            mutable_list[0] = "Not an integer"

    def test_setitem_i32_overflow(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.append(0)
        with self.assertRaises(OverflowError):
            mutable_list[0] = 2**31

    def test_setitem_slice(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        python_list[1:3] = [22, 22]
        # MutableList doesn't support slice assignment
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'list'"
        ):
            mutable_list[1:3] = [22, 22]

    def test_delitem(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        del mutable_list[7]
        del python_list[7]
        self.assertEqual(python_list, mutable_list)
        del mutable_list[-1]
        del python_list[-1]
        self.assertEqual(python_list, mutable_list)

        with self.assertRaisesRegex(IndexError, "list assignment index out of range"):
            del mutable_list[100]

    def test_delitem_slice(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        del mutable_list[2:7]
        del python_list[2:7]
        self.assertEqual(python_list, mutable_list)
        del mutable_list[10:-3]
        del python_list[10:-3]
        self.assertEqual(python_list, mutable_list)
        del mutable_list[5:200]
        del python_list[5:200]
        self.assertEqual(python_list, mutable_list)

    def test_pop(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.extend(range(100))

        self.assertEqual(50, mutable_list.pop(50))
        self.assertEqual(0, mutable_list.pop(0))
        self.assertEqual(99, mutable_list.pop())

        # pop rest of the elements
        for _ in range(100 - 3):
            mutable_list.pop()

        with self.assertRaisesRegex(IndexError, "pop from empty list"):
            mutable_list.pop()

    def test_clear(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.extend(range(100))
        self.assertEqual(100, len(mutable_list))
        mutable_list.clear()
        self.assertEqual(0, len(mutable_list))

    def test_contains(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.extend(range(10))

        for i in range(10):
            self.assertTrue(i in mutable_list)

        self.assertFalse(10 in mutable_list)

    def test_add(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [])
        mutable_list.extend(range(100))

        result = mutable_list + list(range(100, 200))
        self.assertEqual(list(range(200)), result)
        self.assertIsInstance(result, MutableList)
        self.assertIsNot(result, mutable_list)

    def test_count(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [1, 2, 1, 1, 3, 2])

        self.assertEqual(3, mutable_list.count(1))
        self.assertEqual(2, mutable_list.count(2))
        self.assertEqual(1, mutable_list.count(3))

    def test_count_wrong_type(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [1, 2, 1, 1, 3, 2])

        self.assertEqual(0, mutable_list.count("Not an Integer"))
        self.assertEqual(0, mutable_list.count(2**31))

    def test_index(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [1, 2, 1, 1, 3, 2])

        self.assertEqual(0, mutable_list.index(1))
        self.assertEqual(2, mutable_list.index(1, 1))
        self.assertEqual(3, mutable_list.index(1, 3))
        self.assertEqual(3, mutable_list.index(1, 3, 999))

        with self.assertRaisesRegex(ValueError, "not in list"):
            _ = mutable_list.index(1, 4)

        self.assertEqual(1, mutable_list.index(2))
        self.assertEqual(4, mutable_list.index(3))

        with self.assertRaisesRegex(ValueError, "not in list"):
            _ = mutable_list.index(5)

    def test_index_wrong_type(self) -> None:
        mutable_list = MutableList(typeinfo_i32, [1, 2, 1, 1, 3, 2])

        with self.assertRaises(ValueError):
            _ = mutable_list.index("Not an Integer")

        with self.assertRaises(ValueError):
            # It suppresses `OverflowError` and raises `ValueError`
            _ = mutable_list.index(2**31)
