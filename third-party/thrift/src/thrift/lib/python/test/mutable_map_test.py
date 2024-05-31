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
import unittest

from thrift.python.mutable_containers import MutableMap

from thrift.python.types import typeinfo_i32, typeinfo_string


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
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        self.assertEqual(mutable_map, mutable_map)
        self.assertEqual(mutable_map, MutableMap(typeinfo_string, typeinfo_i32, {}))
        self.assertEqual(mutable_map, {})
        self.assertEqual({}, mutable_map)
        self.assertNotEqual({"a": 1}, mutable_map)
        self.assertNotEqual(mutable_map, {"a": 1})

        with self.assertRaisesRegex(TypeError, "types do not match"):
            _ = mutable_map == MutableMap(typeinfo_i32, typeinfo_string, {})

    def test_setitem(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        mutable_map["A"] = 65
        mutable_map["a"] = 97
        self.assertEqual({"A": 65, "a": 97}, mutable_map)

    def test_setitem_wrong_type(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})

        # Wrong key type
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            mutable_map[1] = 65

        # Wrong value type
        with self.assertRaisesRegex(TypeError, "is not a <class 'int'>"):
            mutable_map["A"] = "str"

        self.assertEqual({}, mutable_map)

    def test_setitem_i32_overflow(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        with self.assertRaises(OverflowError):
            mutable_map["max"] = 2**31

    def test_getitem(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        with self.assertRaises(KeyError):
            mutable_map["not_exists"]

        self.assertEqual(65, mutable_map["A"])
        self.assertEqual(97, mutable_map["a"])

    def test_getitem_wrong_type(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            _ = mutable_map[1]

    def test_iter(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        python_set = {"a", "A"}
        for key in mutable_map:
            # `remove()` throws `KeyError` if key is absent
            python_set.remove(key)

        self.assertEqual(0, len(python_set))

    def test_iter_next(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
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
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
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
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            mutable_map.get(123)

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            mutable_map.get(123, "default_value")

    def test_contains(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})
        mutable_map["A"] = 65
        mutable_map["a"] = 97

        self.assertIn("A", mutable_map)
        self.assertIn("a", mutable_map)
        self.assertNotIn("x", mutable_map)
        self.assertNotIn("y", mutable_map)

    def test_contains_wrong_type(self) -> None:
        mutable_map = MutableMap(typeinfo_string, typeinfo_i32, {})

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            _ = 1 in mutable_map

        mutable_map["A"] = 65

        with self.assertRaisesRegex(
            TypeError, "Expected type <class 'str'>, got: <class 'int'>"
        ):
            _ = 999 in mutable_map
