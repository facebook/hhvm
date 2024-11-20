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

from thrift.python.mutable_containers import (
    MapItemsView,
    MapKeysView,
    MapValuesView,
    MutableList,
    MutableMap,
)

from thrift.python.mutable_typeinfos import MutableListTypeInfo
from thrift.python.mutable_types import to_thrift_list

from thrift.python.types import typeinfo_i32, typeinfo_string


def _create_MutableMap_str_i32(map: dict[str, int]) -> MutableMap[str, int]:
    """
    Helper function to create and return a `MutableMap`.
    The return type annotation enables Pyre type checking support.
    """
    # pyre-ignore[6]: Incompatible parameter type
    return MutableMap(typeinfo_string, typeinfo_i32, map)


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
            TypeError, "decoding to str: need a bytes-like object, int found"
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

        with self.assertRaisesRegex(TypeError, "types do not match"):
            _ = mutable_map == MutableMap(typeinfo_i32, typeinfo_string, {})

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
