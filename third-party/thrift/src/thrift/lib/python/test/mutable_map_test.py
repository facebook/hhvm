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
import string
import unittest

from typing import cast, ItemsView, Iterator, KeysView, Optional, ValuesView

from python_test.maps.thrift_mutable_types import StrIntMap

from thrift.python.mutable_containers import (
    MapItemsView,
    MapKeysView,
    MapValuesView,
    MutableList,
    MutableMap,
)

from thrift.python.mutable_typeinfos import MutableListTypeInfo
from thrift.python.mutable_types import to_thrift_list, to_thrift_map, to_thrift_set

from thrift.python.types import typeinfo_i32, typeinfo_string


def _create_MutableMap_str_i32(map: dict[str, int]) -> MutableMap[str, int]:
    """
    Helper function to create and return a `MutableMap`.
    The return type annotation enables Pyre type checking support.
    """
    # pyre-ignore[6]: Incompatible parameter type
    return MutableMap(typeinfo_string, typeinfo_i32, map)


class MutableMapTypeHints(unittest.TestCase):
    """
    Tests type hints for MutableMap class.

    This test suite checks that Pyre correctly identifies and raises errors
    when there are type mismatches in function calls or assignments.

    Casting an object to MutableMap[K, V] is sufficient to check the type hints.

    The `pyre-ignore` comments in the test are intended to suppress real errors,
    as we run Pyre with the `--report-unused-ignores` flag.
    """

    def test_type_hints(self) -> None:
        mutable_map = cast(MutableMap[str, int], object())
        try:
            ### get() ###
            v1: Optional[int] = mutable_map.get("key")  # noqa

            # pyre-ignore[9]: v2 is type `int` but is used as type `Optional[int]`
            v2: int = mutable_map.get("key")  # noqa

            v3: int = mutable_map.get("key", 42)  # noqa
            v4: int | str = mutable_map.get("key", "value")  # noqa

            # pyre-ignore[6]: 1st positional argument, expected `str` but got `int`
            v5: int = mutable_map.get(999, 42)  # noqa

            ###################################################################

            ### __setitem__() ###
            mutable_map["key"] = 1

            # pyre-ignore[6]: expected `int` but got `str`
            mutable_map["key"] = "value"
            # pyre-ignore[6]: expected `int` but got `float`
            mutable_map["key"] = 1.0

            # pyre-ignore[6]: expected `str` but got `int`
            mutable_map[1] = 1
            # pyre-ignore[6]: expected `str` but got `bytes`
            mutable_map[b"key"] = 1

            # pyre-ignore[6]: expected `str` but got `int` and expected `int` but got `str`
            mutable_map[1] = "value"

            ###################################################################

            ### __contains__() ###
            _ = 1 in mutable_map
            _ = "key" in mutable_map
            _ = b"key" in mutable_map

            ###################################################################

            ### __delitem() ###
            del mutable_map["key"]

            # pyre-ignore[6]: expected `str` but got `int`
            del mutable_map[1]
            # pyre-ignore[6]: expected `str` but got `bytes`
            del mutable_map[b"key"]

            ###################################################################

            ### pop() ####
            v6: int = mutable_map.pop("key")  # noqa

            # pyre-ignore[6]: expected `str` but got `bytes`
            v7: int = mutable_map.pop(b"key")  # noqa
            # pyre-ignore[9]: v8 type `str` but is used as type `int`
            v8: str = mutable_map.pop("key")  # noqa

            v9: int = mutable_map.pop("key", 42)  # noqa
            v10: int | float = mutable_map.pop("key", 42.0)  # noqa

            ###################################################################

            ### keys() ####

            v11: MapKeysView[str] = mutable_map.keys()
            v12: KeysView[str] = mutable_map.keys()  # noqa

            # pyre-ignore[9]: v13 is type `MapKeysView[int]` but is used as type
            #  `MapKeysView[str]`
            v13: MapKeysView[int] = mutable_map.keys()  # noqa

            keys_iter_1: Iterator[str] = iter(v11)  # noqa

            # pyre-ignore[9]: keys_iter_2 is type `Iterator[int]` but is used as
            #  type `Iterator[str]`
            keys_iter_2: Iterator[int] = iter(v11)  # noqa

            ###################################################################

            ### items() ####

            v14: MapItemsView[str, int] = mutable_map.items()
            v15: ItemsView[str, int] = mutable_map.items()  # noqa

            # pyre-ignore[9]: v16 is type `MapItemsView[str, str]` but is used
            #  as type `MapItemsView[str, int]`
            v16: MapItemsView[str, str] = mutable_map.items()  # noqa

            items_iter_1: Iterator[tuple[str, int]] = iter(v14)  # noqa

            # pyre-ignore[9]: items_iter_2 is type `Iterator[Tuple[str, str]]`
            #  but is used as type `Iterator[Tuple[str, int]]`
            items_iter_2: Iterator[tuple[str, str]] = iter(v14)  # noqa

            ###################################################################

            ### values() ####

            v17: MapValuesView[int] = mutable_map.values()
            v18: ValuesView[int] = mutable_map.values()  # noqa

            # pyre-ignore[9]: v19 is type `MapValuesView[str]` but is used as
            #  type `MapValuesView[int]`
            v19: MapValuesView[str] = mutable_map.values()  # noqa

            values_iter_1: Iterator[int] = iter(v17)  # noqa

            # pyre-ignore[9]: values_iter_2 is type `Iterator[str]` but is used
            #  as type `Iterator[int]`
            values_iter_2: Iterator[str] = iter(v17)  # noqa

        except Exception:
            pass

    def test_type_hints_with_container_value(self) -> None:
        mutable_map = cast(MutableMap[str, MutableList[int]], object())
        try:
            ### get() ###
            v1: Optional[MutableList[int]] = mutable_map.get("key")  # noqa

            # pyre-ignore[9]: v2 is type `MutableList[int]` but is used as type `Optional[MutableList[int]]`.
            v2: MutableList[int] = mutable_map.get("key")  # noqa

            default_value_1 = cast(MutableList[int], object())
            v3: MutableList[int] = mutable_map.get("key", default_value_1)  # noqa

            default_value_2 = cast(MutableList[str], object())
            v4: MutableList[int] | MutableList[str] = mutable_map.get(  # noqa
                "key", default_value_2
            )

            # pyre-ignore[6]: 1st positional argument, expected `str` but got `int`
            v5: MutableList[int] = mutable_map.get(999, default_value_1)

            ###################################################################

            ### __setitem__() ###
            mutable_list_int = cast(MutableList[int], object())
            mutable_list_str = cast(MutableList[str], object())

            mutable_map["key"] = to_thrift_list([])
            mutable_map["key"] = mutable_list_int

            # pyre-ignore[6]: expected `MutableList[int]` but got `MutableList[str]`
            mutable_map["key"] = mutable_list_str
            # pyre-ignore[6]: expected `MutableList[int]` but got `List[int]`
            mutable_map["key"] = [1, 2, 3]

            # pyre-ignore[6]: expected `str` but got `int`
            mutable_map[1] = mutable_list_int
            # pyre-ignore[6]: expected `str` but got `bytes`
            mutable_map[b"key"] = mutable_list_int

            # pyre-ignore[6]: expected `str` but got `int` and expected `MutableList[int]` but got `MutableList[str]`
            mutable_map[1] = mutable_list_str

            ###################################################################

            ### __contains__() ###
            _ = [1, 2, 3] in mutable_map
            _ = to_thrift_list([]) in mutable_map
            _ = to_thrift_set(set()) in mutable_map
            _ = to_thrift_map({}) in mutable_map

            ###################################################################

            ### __delitem() ###
            del mutable_map["key"]

            # pyre-ignore[6]: expected `str` but got `List[int]`
            del mutable_map[[1, 2, 3]]
            # pyre-ignore[6]: expected `str` but got `_ThriftListWrapper`
            del mutable_map[to_thrift_list([])]

            ###################################################################

            ### pop() ####
            v6: MutableList[int] = mutable_map.pop("key")  # noqa

            # pyre-ignore[6]: expected `str` but got `bytes`
            v7: MutableList[int] = mutable_map.pop(b"key")  # noqa
            # pyre-ignore[9]: type `MutableList[str]` but is used as type `MutableList[int]`
            v8: MutableList[str] = mutable_map.pop("key")  # noqa

            mutable_list_int = cast(MutableList[int], object())
            v9: MutableList[int] = mutable_map.pop("key", mutable_list_int)  # noqa

            mutable_list_str = cast(MutableList[str], object())
            v10: MutableList[int] | MutableList[str] = mutable_map.pop(  # noqa
                "key", mutable_list_str
            )

        except Exception:
            pass


class MutableMapTest(unittest.TestCase):
    def test_smoke(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        self.assertIsInstance(mutable_map, MutableMap)
        self.assertIsInstance(mutable_map, collections.abc.MutableMapping)
        self.assertEqual(0, len(mutable_map))
        self.assertFalse(mutable_map)

    def test_init(self) -> None:
        with self.assertRaisesRegex(
            TypeError, r"incorrect type \(expected dict, got NoneType\)"
        ):
            # pyre-ignore[6]: Incompatible parameter type
            MutableMap(typeinfo_string, typeinfo_i32, None)

        with self.assertRaisesRegex(
            TypeError, r"incorrect type \(expected dict, got list\)"
        ):
            # pyre-ignore[6]: Incompatible parameter type
            MutableMap(typeinfo_string, typeinfo_i32, [])

        with self.assertRaisesRegex(
            TypeError, r"incorrect type \(expected dict, got set\)"
        ):
            # pyre-ignore[6]: Incompatible parameter type
            MutableMap(typeinfo_string, typeinfo_i32, set())

    def test_init_with_invalid_map(self) -> None:
        # Initializing `MutableMap` with an invalid map is possible, but it
        # can result in undefined behavior or produce confusing errors as below:

        # Mapping should be string -> i32, but initialized with i32 -> string
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {1: "a", 2: "b"})
        with self.assertRaisesRegex(
            TypeError, "Expected a str, encountered <class 'int'>"
        ):
            for _, _ in mutable_map:
                pass

    def test_empty_eq(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        self.assertEqual(mutable_map, mutable_map)
        self.assertEqual(mutable_map, MutableMap(typeinfo_string, typeinfo_i32, {}))
        self.assertEqual(mutable_map, {})
        self.assertEqual({}, mutable_map)
        self.assertNotEqual({"a": 1}, mutable_map)
        self.assertNotEqual(mutable_map, {"a": 1})

        self.assertNotEqual(mutable_map, MutableMap(typeinfo_i32, typeinfo_string, {}))

    def test_setitem(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97
        self.assertEqual({"A": 65, "a": 97}, mutable_map)

    def test_setitem_wrong_type(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        # Wrong key type
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_map[1] = 65

        # Wrong value type
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            # pyre-ignore[6]: Intentional for test
            mutable_map["A"] = "str"

        self.assertEqual({}, mutable_map)

    def test_setitem_i32_overflow(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        with self.assertRaises(OverflowError):
            mutable_map["max"] = 2**31

    def test_delitem(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97
        self.assertEqual({"A": 65, "a": 97}, mutable_map)

        with self.assertRaisesRegex(KeyError, "c"):
            del mutable_map["c"]

        # Key is not expected type but MutableMap raises key error
        with self.assertRaisesRegex(KeyError, "1"):
            # pyre-ignore[6]: Intentional for test
            del mutable_map[1]

        self.assertEqual({"A": 65, "a": 97}, mutable_map)
        del mutable_map["a"]
        self.assertEqual({"A": 65}, mutable_map)
        del mutable_map["A"]
        self.assertEqual({}, mutable_map)

    def test_getitem(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        with self.assertRaises(KeyError):
            mutable_map["not_exists"]

        self.assertEqual(65, mutable_map["A"])
        self.assertEqual(97, mutable_map["a"])

    def test_getitem_wrong_type(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            _ = mutable_map[1]

    def test_iter(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        python_set = {"a", "A"}
        for key in mutable_map:
            # `remove()` throws `KeyError` if key is absent
            python_set.remove(key)

        self.assertEqual(0, len(python_set))

    def test_iter_next(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97
        mutable_map["last"] = 999

        iter1 = iter(mutable_map)
        iter2 = iter(mutable_map)

        for _ in range(3):
            next(iter1)

        with self.assertRaises(StopIteration):
            next(iter1)

        for _ in range(3):
            next(iter2)

        with self.assertRaises(StopIteration):
            next(iter2)

    def test_get(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        self.assertEqual(65, mutable_map.get("A"))
        self.assertEqual(65, mutable_map.get("A", 999))
        self.assertEqual(97, mutable_map.get("a"))
        self.assertEqual(97, mutable_map.get("a", 999))

        self.assertIsNone(mutable_map.get("not_exists"))
        self.assertIsNone(mutable_map.get("not_exists", None))
        self.assertEqual(999, mutable_map.get("not_exists", 999))

    def test_get_wrong_type(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_map.get(123)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_map.get(123, "default_value")

    def test_contains(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        self.assertIn("A", mutable_map)
        self.assertIn("a", mutable_map)
        self.assertNotIn("x", mutable_map)
        self.assertNotIn("y", mutable_map)

    def test_contains_wrong_type(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})
        self.assertNotIn(1, mutable_map)

        mutable_map["A"] = 65
        self.assertNotIn(999, mutable_map)
        self.assertIn("A", mutable_map)

    def test_keys(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        self.assertIsInstance(mutable_map.keys(), MapKeysView)
        self.assertIsInstance(mutable_map.keys(), collections.abc.KeysView)
        self.assertEqual(0, len(mutable_map.keys()))

        mutable_map["A"] = 65
        mutable_map["a"] = 97

        self.assertEqual(2, len(mutable_map.keys()))

        python_set = {"A", "a"}
        # The iteration below should remove all elements from the python_set.
        for key in mutable_map.keys():
            python_set.remove(key)

        self.assertEqual(0, len(python_set))

        self.assertIn("a", mutable_map.keys())
        self.assertIn("A", mutable_map.keys())
        self.assertNotIn("x", mutable_map.keys())
        self.assertNotIn("y", mutable_map.keys())

        # `MapKeysView.__contains__()` returns `False` on key type mismatch.
        self.assertNotIn(1, mutable_map.keys())

    def test_keys_view(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        # The `keys()` method returns a view of the map's keys. Any modifications
        # made to the map will be reflected in the `keys_view`.
        keys_view = mutable_map.keys()
        self.assertIsInstance(keys_view, MapKeysView)
        self.assertIsInstance(keys_view, collections.abc.KeysView)
        self.assertEqual(0, len(keys_view))

        mutable_map["A"] = 65

        self.assertEqual(1, len(keys_view))
        self.assertIn("A", keys_view)
        self.assertNotIn("a", keys_view)

        mutable_map["a"] = 97

        self.assertEqual(2, len(keys_view))
        self.assertIn("A", keys_view)
        self.assertIn("a", keys_view)

        mutable_map.clear()

        self.assertEqual(0, len(keys_view))
        self.assertNotIn("A", keys_view)
        self.assertNotIn("a", keys_view)

    def test_items(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        self.assertIsInstance(mutable_map.items(), MapItemsView)
        self.assertIsInstance(mutable_map.items(), collections.abc.ItemsView)
        self.assertEqual(0, len(mutable_map.items()))

        mutable_map["A"] = 65
        mutable_map["a"] = 97

        self.assertEqual(2, len(mutable_map.items()))

        python_map = {"A": 65, "a": 97}
        # The iteration below should remove all elements from the python_map.
        for key, value in mutable_map.items():
            self.assertEqual(python_map[key], value)
            python_map.pop(key)

        self.assertEqual(0, len(python_map))

        self.assertIn(("a", 97), mutable_map.items())
        self.assertIn(("A", 65), mutable_map.items())
        self.assertNotIn(("A", 66), mutable_map.items())
        self.assertNotIn(("B", 65), mutable_map.items())

        # `MapItemsView.__contains__()` returns `False` on key or value type
        # mismatch.
        self.assertNotIn((1, 97), mutable_map.items())
        self.assertNotIn(("a", "Not an integer"), mutable_map.items())

    def test_items_view(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        # The `items()` method returns a view of the map's items. Any modifications
        # made to the map will be reflected in the `items_view`.
        items_view = mutable_map.items()
        self.assertIsInstance(items_view, MapItemsView)
        self.assertIsInstance(items_view, collections.abc.ItemsView)
        self.assertEqual(0, len(items_view))

        mutable_map["A"] = 65

        self.assertEqual(1, len(items_view))
        self.assertIn(("A", 65), items_view)
        self.assertNotIn(("a", 97), items_view)

        mutable_map["a"] = 97

        self.assertEqual(2, len(items_view))
        self.assertIn(("A", 65), items_view)
        self.assertIn(("a", 97), items_view)

        mutable_map.clear()

        self.assertEqual(0, len(items_view))
        self.assertNotIn(("A", 65), items_view)
        self.assertNotIn(("a", 97), items_view)

    def test_items_view_container_value(self) -> None:
        mutable_map: MutableMap[str, MutableList[int]] = MutableMap(
            typeinfo_string,
            MutableListTypeInfo(typeinfo_i32),
            {},
        )

        # The `items()` method returns a view of the map's items. Any modifications
        # made to the map will be reflected in the `items_view`.
        items_view = mutable_map.items()
        self.assertIsInstance(items_view, MapItemsView)
        self.assertIsInstance(items_view, collections.abc.ItemsView)
        self.assertEqual(0, len(items_view))

        mutable_map["A"] = to_thrift_list([1, 2, 3])

        self.assertEqual(1, len(items_view))
        self.assertIn(("A", [1, 2, 3]), items_view)
        self.assertNotIn(("A", [3, 2, 1]), items_view)
        self.assertNotIn(("a", [3, 2, 1]), items_view)

        mutable_map["a"] = to_thrift_list([3, 2, 1])

        self.assertEqual(2, len(items_view))
        self.assertIn(("A", [1, 2, 3]), items_view)
        self.assertNotIn(("A", [3, 2, 1]), items_view)
        self.assertIn(("a", [3, 2, 1]), items_view)

        mutable_map.clear()

        self.assertEqual(0, len(items_view))
        self.assertNotIn(("A", [1, 2, 3]), items_view)
        self.assertNotIn(("a", [3, 2, 1]), items_view)

    def test_values(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        self.assertIsInstance(mutable_map.values(), MapValuesView)
        self.assertIsInstance(mutable_map.values(), collections.abc.ValuesView)
        self.assertEqual(0, len(mutable_map.values()))

        mutable_map["A"] = 65
        mutable_map["a"] = 97
        mutable_map["b"] = 97

        self.assertEqual(3, len(mutable_map.values()))

        python_list = [65, 97, 97]
        # The iteration below should remove all elements from the python_list.
        for value in mutable_map.values():
            python_list.remove(value)

        self.assertEqual(0, len(python_list))

        self.assertIn(97, mutable_map.values())
        self.assertIn(65, mutable_map.values())
        self.assertNotIn(1, mutable_map.values())
        self.assertNotIn(999, mutable_map.values())

        # `MapValuesView` does not define `__contains__()`, therefore, the
        # operator `in` compares while iterating over `MapValuesView`.
        # Consequently, no `TypeError` is raised.
        _ = "Not an integer" in mutable_map.values()

    def test_values_view(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        # The `values()` method returns a view of the map's values. Any modifications
        # made to the map will be reflected in the `values_view`.
        values_view = mutable_map.values()
        self.assertIsInstance(values_view, MapValuesView)
        self.assertIsInstance(values_view, collections.abc.ValuesView)
        self.assertEqual(0, len(values_view))

        mutable_map["A"] = 65

        self.assertEqual(1, len(values_view))
        self.assertIn(65, values_view)
        self.assertNotIn(97, values_view)

        mutable_map["a"] = 97

        self.assertEqual(2, len(values_view))
        self.assertIn(65, values_view)
        self.assertIn(97, values_view)

        mutable_map["b"] = 97

        self.assertEqual([65, 97, 97], sorted(values_view))

        mutable_map.clear()

        self.assertEqual(0, len(values_view))
        self.assertNotIn(65, values_view)
        self.assertNotIn(97, values_view)

    def test_values_view_with_container_value(self) -> None:
        mutable_map: MutableMap[str, MutableList[int]] = MutableMap(
            typeinfo_string,
            MutableListTypeInfo(typeinfo_i32),
            {},
        )

        mutable_list_1: MutableList[int] = MutableList(typeinfo_i32, [1, 2, 3])
        mutable_list_2: MutableList[int] = MutableList(typeinfo_i32, [4, 5, 6])

        # The `values()` method returns a view of the map's values. Any modifications
        # made to the map will be reflected in the `values_view`.
        values_view = mutable_map.values()
        self.assertIsInstance(values_view, MapValuesView)
        self.assertIsInstance(values_view, collections.abc.ValuesView)
        self.assertEqual(0, len(values_view))

        mutable_map["A"] = to_thrift_list([1, 2, 3])

        self.assertEqual(1, len(values_view))
        self.assertIn([1, 2, 3], values_view)
        self.assertIn(mutable_list_1, values_view)
        self.assertNotIn([4, 5, 6], values_view)
        self.assertNotIn(mutable_list_2, values_view)

        mutable_map["a"] = to_thrift_list([4, 5, 6])

        self.assertEqual(2, len(values_view))
        self.assertIn([1, 2, 3], values_view)
        self.assertIn(mutable_list_1, values_view)
        self.assertIn([4, 5, 6], values_view)
        self.assertIn(mutable_list_2, values_view)

        mutable_map.clear()
        self.assertEqual(0, len(values_view))

    def test_mutation_via_values_view(self) -> None:
        internal_map = {1: [1, 2, 3], 2: [4, 5], 3: [6]}
        mutable_map: MutableMap[int, MutableList[int]] = MutableMap(
            typeinfo_i32,
            MutableListTypeInfo(typeinfo_i32),
            # pyre-ignore[6]
            internal_map,
        )

        self.assertEqual(3, len(mutable_map))
        self.assertEqual([1, 2, 3], mutable_map[1])
        self.assertEqual([4, 5], mutable_map[2])
        self.assertEqual([6], mutable_map[3])

        values_view = mutable_map.values()
        self.assertEqual(3, len(values_view))

        # Modify each `MutableList` in the `values_view` by appending to them
        for value in values_view:
            value.append(999)

        # Assert the values have been updated in the `mutable_map`
        self.assertEqual(3, len(mutable_map))
        self.assertEqual([1, 2, 3, 999], mutable_map[1])
        self.assertEqual([4, 5, 999], mutable_map[2])
        self.assertEqual([6, 999], mutable_map[3])

    def test_pickle_round_trip(self) -> None:
        internal_map = {1: [1, 2, 3], 2: [4, 5], 3: [6]}
        mutable_map: MutableMap[int, MutableList[int]] = MutableMap(
            typeinfo_i32,
            MutableListTypeInfo(typeinfo_i32),
            # pyre-ignore[6]
            internal_map,
        )

        pickled = pickle.dumps(mutable_map, protocol=pickle.HIGHEST_PROTOCOL)
        mutable_map_unpickled = pickle.loads(pickled)
        self.assertIsInstance(mutable_map_unpickled, MutableMap)
        self.assertEqual(mutable_map, mutable_map_unpickled)

    def test_pop(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        mutable_map["A"] = 65
        mutable_map["a"] = 97

        with self.assertRaisesRegex(KeyError, "not exists"):
            mutable_map.pop("not exists")

        self.assertEqual(
            "default-value", mutable_map.pop("not exists", "default-value")
        )

        self.assertEqual(65, mutable_map.pop("A"))
        self.assertEqual({"a": 97}, mutable_map)
        self.assertEqual(97, mutable_map.pop("a"))
        self.assertEqual({}, mutable_map)

        with self.assertRaisesRegex(KeyError, "A"):
            mutable_map.pop("A")

        # Wrong key type raises a KeyError not a TypeError
        with self.assertRaisesRegex(KeyError, "123"):
            # pyre-ignore[6]: Intentional for test
            mutable_map.pop(123)

    def test_popitem(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        for char in string.ascii_letters:
            mutable_map[char] = ord(char)

        # Python 3.7+ LIFO order is now guaranteed.
        for char in string.ascii_letters[::-1]:
            self.assertEqual(mutable_map.popitem(), (char, ord(char)))

        self.assertEqual(0, len(mutable_map))

    def test_setdefault(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        self.assertEqual(65, mutable_map.setdefault("A", 65))
        self.assertEqual(65, mutable_map["A"])
        self.assertEqual(65, mutable_map.setdefault("A", 999))
        self.assertEqual(65, mutable_map["A"])

        # Wrong key type raises a TypeError
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # pyre-ignore[6]: Intentional for test
            mutable_map.setdefault(123, 999)

        # Default value for `setdefault()` is `None`, however, `None` is not a
        # valid thrift value, therefore, it is a TypeError
        with self.assertRaisesRegex(
            TypeError, "not a <class 'int'>, is actually of type <class 'NoneType'>"
        ):
            mutable_map.setdefault("new-key")

    def test_setdefault_container_value(self) -> None:
        mutable_map: MutableMap[str, MutableList[int]] = MutableMap(
            typeinfo_string,
            MutableListTypeInfo(typeinfo_i32),
            {},
        )

        value_A = mutable_map.setdefault("A", to_thrift_list([1, 2, 3]))
        self.assertIsInstance(value_A, MutableList)
        self.assertEqual([1, 2, 3], value_A)

        # key "A" already exists, we should get [1, 2, 3]
        value_A = mutable_map.setdefault("A", to_thrift_list([4, 5, 6]))
        self.assertIsInstance(value_A, MutableList)
        self.assertEqual([1, 2, 3], value_A)

        # Wrong value type raises a TypeError
        with self.assertRaisesRegex(
            TypeError, "not a <class 'int'>, is actually of type <class 'str'>"
        ):
            mutable_map.setdefault("B", to_thrift_list(["1", "2", "3"]))

        # set the key "A" and try to read it back with `setdefault()`
        mutable_map["A"] = to_thrift_list([11, 12, 13])
        value_A = mutable_map.setdefault("A", to_thrift_list([4, 5, 6]))
        self.assertIsInstance(value_A, MutableList)
        self.assertEqual([11, 12, 13], value_A)

    def test_update(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        mutable_map["A"] = 65
        mutable_map["a"] = 97

        mutable_map.update({"B": 66})
        self.assertEqual({"A": 65, "a": 97, "B": 66}, mutable_map)

        mutable_map.update({"B": 166, "A": 165})
        self.assertEqual({"A": 165, "a": 97, "B": 166}, mutable_map)

        mutable_map.update({"B": 66}, A=65, b=98)
        self.assertEqual({"A": 65, "a": 97, "B": 66, "b": 98}, mutable_map)

        mutable_map.update({}, B=166, b=198)
        self.assertEqual({"A": 65, "a": 97, "B": 166, "b": 198}, mutable_map)

        mutable_map.update(B=66, b=98)
        self.assertEqual({"A": 65, "a": 97, "B": 66, "b": 98}, mutable_map)

    def test_update_exception(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        mutable_map["A"] = 65
        mutable_map["a"] = 97

        # basic exception safety
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            # `{"B": 66}` raises before keyword argument
            # pyre-ignore[6]: Intentional for test
            mutable_map.update({1: 999}, A=65)

        self.assertEqual({"A": 65, "a": 97}, mutable_map)

        with self.assertRaisesRegex(
            TypeError, "not a <class 'int'>, is actually of type <class 'str'>"
        ):
            # `{"B": 66}` updates the mutable map before keyword argument raises
            # pyre-ignore[6]: Intentional for test
            mutable_map.update({"B": 66}, x="Not an Integer")

        self.assertEqual({"A": 65, "a": 97, "B": 66}, mutable_map)

    def test_match(self) -> None:
        mutable_map = _create_MutableMap_str_i32({})

        mutable_map["A"] = 65
        mutable_map["a"] = 97

        match mutable_map:
            case {"A": 65, "B": 97}:
                self.fail("shouldn't happen")
            case {"B": 65}:
                self.fail("shouldn't happen")
            case {"A": 66, **rest}:
                self.fail("shouldn't happen")
            case {"A": 65, **rest}:
                self.assertEqual({"a": 97}, rest)
            case _:
                self.fail("shouldn't happen")

        match mutable_map:
            case {"A": 65, "B": 97}:
                self.fail("shouldn't happen")
            case {"B": 65}:
                self.fail("shouldn't happen")
            case {"A": 66, **rest}:
                self.fail("shouldn't happen")
            case {"A": 65, "a": x}:
                self.assertEqual(x, 97)
            case _:
                self.fail("shouldn't happen")


class MutableMapTypedefTest(unittest.TestCase):
    TYPE_ERROR_MESSAGE = (
        "Expected values to be an instance of Thrift mutable map with matching "
        r"key type and value type, or the result of `to_thrift_map\(\)`, but "
        "got type <class 'dict'>."
    )

    def test_str_int_map(self) -> None:
        """
        typedef map<i32> StrIntMap
        """
        # Mutable container typedef should be initialized with the same mutable
        # container type, or it should be wrapped with `to_thrift_map()`.
        # Otherwise, it will result in both runtime and Pyre errors.
        with self.assertRaisesRegex(TypeError, self.TYPE_ERROR_MESSAGE):
            # pyre-ignore[6]: Intentional for test
            _ = StrIntMap({"a": 1, "b": 2})

        # Initialize with `to_thrift_map()` and verify that the initial
        # Python map `m` and the MutableMap `strintmap` are separate maps.
        m = {"a": 1, "b": 2}
        str_int_map: MutableMap[str, int] = StrIntMap(to_thrift_map(m))
        self.assertEqual({"a": 1, "b": 2}, str_int_map)

        m["a"] = 11
        self.assertEqual({"a": 11, "b": 2}, m)
        self.assertEqual({"a": 1, "b": 2}, str_int_map)
