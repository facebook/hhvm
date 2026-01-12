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
import pickle
import unittest
from collections.abc import MutableSequence
from typing import cast

from python_test.containers.thrift_mutable_types import Foo, Lists
from python_test.lists.thrift_mutable_types import (
    easy,
    EasyList,
    I32List,
    ListOfStrToI32Map,
    StrList2D,
)
from thrift.python.mutable_containers import MutableList, MutableMap
from thrift.python.mutable_typeinfos import MutableListTypeInfo
from thrift.python.mutable_types import to_thrift_list
from thrift.python.types import typeinfo_i32


def _create_MutableList_i32(lst: list[int]) -> MutableList[int]:
    return MutableList(typeinfo_i32, lst)


class MutableListTest(unittest.TestCase):
    """
    Some of the tests use a Python `list` for verification. They create a
    `MutableList` and a Python `list`, apply the same operations to both of
    them, and check if they are equal.
    """

    def test_smoke(self) -> None:
        mutable_list = _create_MutableList_i32([])
        self.assertTrue(isinstance(mutable_list, MutableList))
        self.assertTrue(isinstance(mutable_list, MutableSequence))
        self.assertEqual(0, len(mutable_list))
        self.assertFalse(mutable_list)

    def test_extend(self) -> None:
        mutable_list = _create_MutableList_i32([])
        python_list = []

        mutable_list.extend(range(10))
        python_list.extend(range(10))

        self.assertEqual(python_list, mutable_list)

    def test_extend_wrong_type(self) -> None:
        mutable_list = _create_MutableList_i32([])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_list.extend([1, 2, "Not an integer", 3])

        # basic exception safety
        self.assertEqual([1, 2], mutable_list)

    def test_extend_i32_overflow(self) -> None:
        mutable_list = _create_MutableList_i32([])
        with self.assertRaises(OverflowError):
            mutable_list.extend([1, 2, 2**31, 3])

        # basic exception safety
        self.assertEqual([1, 2], mutable_list)

    def test_append(self) -> None:
        mutable_list = _create_MutableList_i32([])
        python_list = []

        for i in range(100):
            mutable_list.append(i)
            python_list.append(i)

        self.assertEqual(python_list, mutable_list)

    def test_append_wrong_type(self) -> None:
        mutable_list = _create_MutableList_i32([])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_list.append("Not an interger")

    def test_append_i32_overflow(self) -> None:
        mutable_list = _create_MutableList_i32([])
        with self.assertRaises(OverflowError):
            mutable_list.append(2**31)

    def test_remove(self) -> None:
        mutable_list = _create_MutableList_i32(list(range(100)))
        python_list = list(range(100))

        elements_to_remove = [2, 4, 24, 42, 72]

        for element in elements_to_remove:
            mutable_list.remove(element)
            python_list.remove(element)

        self.assertEqual(python_list, mutable_list)

    def test_getitem(self) -> None:
        mutable_list = _create_MutableList_i32([])
        mutable_list.extend(range(100))

        for i in range(100):
            self.assertEqual(i, mutable_list[i])
            self.assertEqual(99 - i, mutable_list[-i - 1])

        with self.assertRaisesRegex(IndexError, "list index out of range"):
            mutable_list[100]

    def test_getitem_slice(self) -> None:
        mutable_list = _create_MutableList_i32([])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        self.assertEqual(python_list[1:3], mutable_list[1:3])
        self.assertEqual(python_list[9:3], mutable_list[9:3])
        self.assertEqual(python_list[9:3:-1], mutable_list[9:3:-1])
        self.assertEqual(python_list[::-1], mutable_list[::-1])
        self.assertEqual(python_list[101:201], mutable_list[101:201])

    def test_insert(self) -> None:
        mutable_list = _create_MutableList_i32([])
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

    def test_insert_struct(self) -> None:
        lists = Lists(structList=to_thrift_list([Foo(value=1)]))
        foo_list = lists.structList
        python_foo_list = list(lists.structList)
        self.assertEqual(foo_list, python_foo_list)

        foo_list.insert(0, Foo(value=0))
        python_foo_list.insert(0, Foo(value=0))
        self.assertEqual(foo_list, python_foo_list)

    def test_insert_wrong_type(self) -> None:
        mutable_list = _create_MutableList_i32([])
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_list.insert(0, "Not an interger")

    def test_insert_i32_overflow(self) -> None:
        mutable_list = _create_MutableList_i32([])
        with self.assertRaises(OverflowError):
            mutable_list.insert(0, 2**31)

    def test_setitem(self) -> None:
        mutable_list = _create_MutableList_i32([])
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
        mutable_list = _create_MutableList_i32([])
        mutable_list.append(0)
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_list[0] = "Not an integer"

    def test_setitem_i32_overflow(self) -> None:
        mutable_list = _create_MutableList_i32([])
        mutable_list.append(0)
        with self.assertRaises(OverflowError):
            mutable_list[0] = 2**31

    def test_setitem_slice(self) -> None:
        mutable_list = _create_MutableList_i32([])
        python_list = []

        mutable_list.extend(range(100))
        python_list.extend(range(100))

        python_list[1:3] = [22, 22]
        # MutableList doesn't support slice assignment
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'list'"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_list[1:3] = [22, 22]

    def test_delitem(self) -> None:
        mutable_list = _create_MutableList_i32([])
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
        mutable_list = _create_MutableList_i32([])
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
        mutable_list = _create_MutableList_i32([])
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
        mutable_list = _create_MutableList_i32([])
        mutable_list.extend(range(100))
        self.assertEqual(100, len(mutable_list))
        mutable_list.clear()
        self.assertEqual(0, len(mutable_list))

    def test_contains(self) -> None:
        mutable_list = _create_MutableList_i32([])
        mutable_list.extend(range(10))

        for i in range(10):
            self.assertTrue(i in mutable_list)

        self.assertFalse(10 in mutable_list)

    def test_contains_wrong_type(self) -> None:
        mutable_list = _create_MutableList_i32([1, 2, 3])

        self.assertIn(3, mutable_list)
        self.assertNotIn(4, mutable_list)
        self.assertNotIn("Not an Integer", mutable_list)
        self.assertNotIn(2**31, mutable_list)

    def test_add(self) -> None:
        mutable_list = _create_MutableList_i32([])
        mutable_list.extend(range(100))

        result = mutable_list + list(range(100, 200))
        self.assertEqual(list(range(200)), result)
        self.assertIsInstance(result, MutableList)
        self.assertIsNot(result, mutable_list)

    def test_radd(self) -> None:
        mutable_list = _create_MutableList_i32([])

        result_1 = [] + mutable_list
        self.assertIsInstance(result_1, MutableList)
        self.assertEqual([], result_1)

        mutable_list.extend(range(100, 200))
        self.assertEqual([], result_1)

        result_2 = list(range(100)) + mutable_list
        self.assertIsInstance(result_2, MutableList)
        self.assertEqual(list(range(200)), result_2)

        # Left hand side with wrong type raises `TypeError`
        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[58]: Intentional for test
            _ = ["1", "2"] + mutable_list

    def test_count(self) -> None:
        mutable_list = _create_MutableList_i32([1, 2, 1, 1, 3, 2])

        self.assertEqual(3, mutable_list.count(1))
        self.assertEqual(2, mutable_list.count(2))
        self.assertEqual(1, mutable_list.count(3))

    def test_count_wrong_type(self) -> None:
        mutable_list = _create_MutableList_i32([1, 2, 1, 1, 3, 2])

        self.assertEqual(0, mutable_list.count("Not an Integer"))
        self.assertEqual(0, mutable_list.count(2**31))

    def test_index(self) -> None:
        mutable_list = _create_MutableList_i32([1, 2, 1, 1, 3, 2])

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
        mutable_list = _create_MutableList_i32([1, 2, 1, 1, 3, 2])

        with self.assertRaises(ValueError):
            _ = mutable_list.index("Not an Integer")

        with self.assertRaises(ValueError):
            # It suppresses `OverflowError` and raises `ValueError`
            _ = mutable_list.index(2**31)

    def test_pickle_round_trip(self) -> None:
        mutable_list = _create_MutableList_i32([1, 2, 1, 1, 3, 2])

        pickled = pickle.dumps(mutable_list, protocol=pickle.HIGHEST_PROTOCOL)
        mutable_list_unpickled = pickle.loads(pickled)
        self.assertIsInstance(mutable_list_unpickled, MutableList)
        self.assertEqual(mutable_list, mutable_list_unpickled)

    def test_match(self) -> None:
        mutable_list = _create_MutableList_i32([1, 2, 1, 1, 3, 2])
        match mutable_list:
            case [1, 2, 1, 2]:
                self.fail("shouldn't happen")
            case [1, 2, 1, 1]:
                self.fail("shouldn't happen")
            case [1, 2, 1, 1, *rest]:
                self.assertEqual([3, 2], rest)
            case _:
                self.fail("shouldn't happen")


def create_MutableList_List_i32(lst: list[list[int]]) -> MutableList[MutableList[int]]:
    """Converts the given (built-in) nested int list to a nested MutableList."""
    return cast(
        MutableList[MutableList[int]],
        MutableList(MutableListTypeInfo(typeinfo_i32), lst),
    )


class MutableListNestedTest(unittest.TestCase):
    """
    Tests nested containers as element types
    """

    TYPE_ERROR_MESSAGE = (
        "Expected values to be an instance of Thrift mutable list with matching "
        r"element type, or the result of `to_thrift_list\(\)`, but got type "
        r"<class '(list|int)'>."
    )

    def test_extend(self) -> None:
        mutable_list = create_MutableList_List_i32([[1]])
        self.assertEqual([[1]], mutable_list)

        mutable_list.extend([to_thrift_list([2]), to_thrift_list([3, 4])])
        self.assertEqual([[1], [2], [3, 4]], mutable_list)

        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-ignore[6]: Intentional for test
            mutable_list.extend([4, 5])

    def test_append(self) -> None:
        mutable_list = create_MutableList_List_i32([])
        self.assertEqual([], mutable_list)

        mutable_list.append(to_thrift_list([1]))
        self.assertEqual([[1]], mutable_list)

        mutable_list.append(to_thrift_list([2, 3]))
        self.assertEqual([[1], [2, 3]], mutable_list)

        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-ignore[6]: Intentional for test
            mutable_list.append([4, 5])

    def test_setitem(self) -> None:
        mutable_list = create_MutableList_List_i32([[1], [2], [3]])
        self.assertEqual([[1], [2], [3]], mutable_list)

        mutable_list[0] = to_thrift_list([11])
        self.assertEqual([[11], [2], [3]], mutable_list)

        mutable_list[1] = to_thrift_list([12])
        self.assertEqual([[11], [12], [3]], mutable_list)

        mutable_list[2] = to_thrift_list([13])
        self.assertEqual([[11], [12], [13]], mutable_list)

        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-ignore[6]: Intentional for test
            mutable_list[0] = [21]

        # Exception guarantee: failure above does not affect existing data
        self.assertEqual([[11], [12], [13]], mutable_list)

    def test_contains(self) -> None:
        mutable_list = create_MutableList_List_i32([[1], [2], [3]])
        self.assertEqual([[1], [2], [3]], mutable_list)

        self.assertIn([1], mutable_list)
        self.assertIn([2], mutable_list)
        self.assertNotIn([1, 2], mutable_list)

        self.assertIn(mutable_list[0], mutable_list)
        self.assertIn(mutable_list[-1], mutable_list)

    def test_add(self) -> None:
        mutable_list = create_MutableList_List_i32([[1]])
        self.assertEqual([[1]], mutable_list)

        result_1 = mutable_list + [to_thrift_list([2]), to_thrift_list([3])]
        self.assertEqual([[1], [2], [3]], result_1)

        result_2 = mutable_list + result_1
        self.assertEqual([[1], [1], [2], [3]], result_2)

    def test_count(self) -> None:
        mutable_list = create_MutableList_List_i32([[1], [2], [2]])
        self.assertEqual([[1], [2], [2]], mutable_list)

        self.assertEqual(1, mutable_list.count([1]))
        self.assertEqual(2, mutable_list.count([2]))

        self.assertEqual(1, mutable_list.count(mutable_list[0]))
        self.assertEqual(2, mutable_list.count(mutable_list[1]))

    def test_index(self) -> None:
        mutable_list = create_MutableList_List_i32([[1], [2], [1, 2]])

        self.assertEqual(0, mutable_list.index([1]))
        self.assertEqual(1, mutable_list.index([2]))
        self.assertEqual(2, mutable_list.index([1, 2]))

        with self.assertRaisesRegex(ValueError, "not in list"):
            _ = mutable_list.index([3])


class MutableListTypedefTest(unittest.TestCase):
    TYPE_ERROR_MESSAGE = (
        "Expected values to be an instance of Thrift mutable list with matching "
        r"element type, or the result of `to_thrift_list\(\)`, but got type "
        r"<class '(list|int)'>."
    )

    def test_list_i32(self) -> None:
        """
        typedef list<i32> I32List
        """
        # Mutable container typedef should be initialized with the same mutable
        # container type, or it should be wrapped with `to_thrift_list()`.
        # Otherwise, it will result in both runtime and Pyre errors.
        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-fixme[20]: Argument `list_data` expected.
            _ = I32List([1, 2, 3])

        # Initialize with `to_thrift_python()` and verify that the initial
        # Python list `lst` and the MutableList `i32list` are separate lists.
        lst = [1, 2, 3]
        # pyre-fixme[20]: Argument `list_data` expected.
        i32list: MutableList[int] = I32List(to_thrift_list(lst))
        self.assertEqual([1, 2, 3], i32list)

        lst[0] = 11
        self.assertEqual([11, 2, 3], lst)
        self.assertEqual([1, 2, 3], i32list)

    def test_str_list_2d(self) -> None:
        """
        typedef list<list<string>> StrList2D
        """
        # Mutable container typedef should be initialized with the same mutable
        # container type, or it should be wrapped with `to_thrift_list()`.
        # Otherwise, it will result in both runtime and Pyre errors.
        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-fixme[20]: Argument `list_data` expected.
            _ = StrList2D([["a", "b"], ["c", "d"]])

        # Initialize with `to_thrift_python()` and verify that the initial
        # Python list `lst` and the MutableList `strlist2d` are separate lists.
        lst = [["a", "b"], ["c", "d"]]
        # pyre-fixme[20]: Argument `list_data` expected.
        strlist2d: MutableList[MutableList[str]] = StrList2D(to_thrift_list(lst))
        self.assertEqual([["a", "b"], ["c", "d"]], strlist2d)

        lst[0][0] = "A"
        self.assertEqual([["A", "b"], ["c", "d"]], lst)
        self.assertEqual([["a", "b"], ["c", "d"]], strlist2d)

    def test_init_typedef_with_same_type(self) -> None:
        """
        typedef list<list<string>> StrList2D
        """
        # Initializing the typedef container with the same mutable container
        # carries reference semantics.
        # pyre-fixme[20]: Argument `list_data` expected.
        strlist2d_1 = StrList2D(to_thrift_list([["a"], ["c"]]))
        # pyre-fixme[20]: Argument `list_data` expected.
        strlist2d_2 = StrList2D(strlist2d_1)

        self.assertEqual(strlist2d_1, strlist2d_2)
        strlist2d_1[0].append("b")
        self.assertEqual([["a", "b"], ["c"]], strlist2d_1)
        self.assertEqual(strlist2d_1, strlist2d_2)

        strlist2d_2[1].append("d")
        self.assertEqual([["a", "b"], ["c", "d"]], strlist2d_2)
        self.assertEqual(strlist2d_1, strlist2d_2)

    def test_str_list_of_str_to_i32_map(self) -> None:
        """
        typedef list<map<string, i32>> ListOfStrToI32Map
        """
        # Mutable container typedef should be initialized with the same mutable
        # container type, or it should be wrapped with `to_thrift_list()`.
        # Otherwise, it will result in both runtime and Pyre errors.
        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-fixme[20]: Argument `list_data` expected.
            _ = ListOfStrToI32Map([{"a": 1, "b": 2}])

        lst = [{"a": 1, "b": 2}]
        # pyre-fixme[20]: Argument `list_data` expected.
        strlist2d: MutableList[MutableMap[str, int]] = ListOfStrToI32Map(
            to_thrift_list(lst)
        )
        self.assertEqual([{"a": 1, "b": 2}], strlist2d)

    def test_easy_list(self) -> None:
        """
        struct easy {
            3: optional string name;
            1: i32 val;
            ...
        }
        typedef list<easy> EasyList
        """
        # If the elements are structured types, `to_thrift_list()` does not
        # deep-copy them; they carry reference semantics.
        lst = [easy(val=1), easy(val=2), easy(val=3)]
        # pyre-fixme[20]: Argument `list_data` expected.
        easylist: MutableList[easy] = EasyList(to_thrift_list(lst))
        self.assertIs(lst[0], easylist[0])
        self.assertIs(lst[1], easylist[1])
        self.assertIs(lst[2], easylist[2])
        self.assertEqual(lst, easylist)

        # The elements are the "same" but the containers are different.
        # Popping from one container does not affect the other.
        lst.pop()
        self.assertNotEqual(lst, easylist)

    def test_default_init_and_populate(self) -> None:
        """
        typedef list<i32> I32List
        """
        # pyre-fixme[20]: Argument `list_data` expected.
        i32list: MutableList[int] = I32List(to_thrift_list([]))
        self.assertEqual([], i32list)
        i32list.append(1)
        self.assertEqual([1], i32list)
        i32list.append(2)
        self.assertEqual([1, 2], i32list)

        with self.assertRaisesRegex(
            TypeError, "is not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # pyre-ignore[6]: Intentional for test
            i32list.append("Not an integer")
