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

"""Tests that container typedefs gracefully degrade to the old factory-based
behavior when thrift.python.container_typedefs is not available (simulating
an older thrift runtime without BUCK-built dependencies).
"""

from __future__ import annotations

import importlib
import sys
import unittest
from collections.abc import Mapping, Sequence, Set as AbstractSetABC
from types import ModuleType
from typing import Optional

import python_test.lists.thrift_types  # noqa: F401 - needed as reload target
import python_test.maps.thrift_types  # noqa: F401 - needed as reload target
import python_test.sets.thrift_types  # noqa: F401 - needed as reload target
import thrift.python.types as _fbthrift_python_types


def _reload_module_without_container_typedefs(module_name: str) -> ModuleType:
    """
    Reload a generated thrift_types module with thrift.python.container_typedefs
    blocked.  Setting sys.modules[name] = None causes ImportError on import,
    so the generated try/except falls through to the factory-based fallback.
    The original sys.modules state is restored after the reload.
    """
    saved_ct: Optional[ModuleType] = sys.modules.get("thrift.python.container_typedefs")
    saved_mod: Optional[ModuleType] = sys.modules.pop(module_name, None)

    # None in sys.modules causes ImportError on import
    sys.modules["thrift.python.container_typedefs"] = None  # type: ignore[assignment]

    try:
        reloaded = importlib.import_module(module_name)
    finally:
        # Remove the reloaded version so it doesn't shadow the original
        sys.modules.pop(module_name, None)
        if saved_mod is not None:
            sys.modules[module_name] = saved_mod

        # Restore container_typedefs
        if saved_ct is not None:
            sys.modules["thrift.python.container_typedefs"] = saved_ct
        else:
            sys.modules.pop("thrift.python.container_typedefs", None)

    return reloaded


class ListTypedefCompatibilityTest(unittest.TestCase):
    """
    Tests for list typedefs when container_typedefs is unavailable.
    Mirrors test_typedef_isinstance in lists.py but expects factory-based
    fallback behavior instead of class-based typedefs.
    """

    lists_types: ModuleType

    @classmethod
    def setUpClass(cls) -> None:
        cls.lists_types = _reload_module_without_container_typedefs(
            "python_test.lists.thrift_types"
        )

    def test_container_typedefs_not_available(self) -> None:
        self.assertIsNone(self.lists_types._fbthrift_python_container_typedefs)

    def test_list_typedef_is_factory_instance(self) -> None:
        """Without container_typedefs, list typedefs are ListTypeFactory instances."""
        self.assertIsInstance(
            self.lists_types.I32List,
            # pyre-ignore[16]: ListTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.ListTypeFactory,
        )
        self.assertIsInstance(
            self.lists_types.IdList,
            # pyre-ignore[16]: ListTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.ListTypeFactory,
        )
        self.assertIsInstance(
            self.lists_types.StringList,
            # pyre-ignore[16]: ListTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.ListTypeFactory,
        )
        self.assertIsInstance(
            self.lists_types.StrList2D,
            # pyre-ignore[16]: ListTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.ListTypeFactory,
        )
        self.assertIsInstance(
            self.lists_types.AnotherStrList2D,
            # pyre-ignore[16]: ListTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.ListTypeFactory,
        )

    def test_list_typedef_construction(self) -> None:
        """Constructing lists via typedef factories still works."""
        I32List = self.lists_types.I32List
        int_list = I32List([1, 2, 3])
        self.assertEqual(list(int_list), [1, 2, 3])
        self.assertIsInstance(int_list, _fbthrift_python_types.List)
        self.assertIsInstance(int_list, Sequence)

    def test_list_typedef_nested_construction(self) -> None:
        """Nested list typedefs also work with factories."""
        StrList2D = self.lists_types.StrList2D
        nested = StrList2D([["a", "b"], ["c", "d"]])
        self.assertEqual(len(nested), 2)
        self.assertEqual(list(nested[0]), ["a", "b"])
        self.assertEqual(list(nested[1]), ["c", "d"])
        self.assertIsInstance(nested, _fbthrift_python_types.List)
        self.assertIsInstance(nested[0], _fbthrift_python_types.List)


class SetTypedefCompatibilityTest(unittest.TestCase):
    """
    Tests for set typedefs when container_typedefs is unavailable.
    Mirrors test_typedef_isinstance in sets.py but expects factory-based
    fallback behavior.
    """

    sets_types: ModuleType

    @classmethod
    def setUpClass(cls) -> None:
        cls.sets_types = _reload_module_without_container_typedefs(
            "python_test.sets.thrift_types"
        )

    def test_container_typedefs_not_available(self) -> None:
        self.assertIsNone(self.sets_types._fbthrift_python_container_typedefs)

    def test_set_typedef_is_factory_instance(self) -> None:
        """Without container_typedefs, set typedefs are SetTypeFactory instances."""
        self.assertIsInstance(
            self.sets_types.SetI32,
            # pyre-ignore[16]: SetTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.SetTypeFactory,
        )
        self.assertIsInstance(
            self.sets_types.AnotherSetI32,
            # pyre-ignore[16]: SetTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.SetTypeFactory,
        )
        self.assertIsInstance(
            self.sets_types.SetI32Lists,
            # pyre-ignore[16]: SetTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.SetTypeFactory,
        )

    def test_set_typedef_construction(self) -> None:
        """Constructing sets via typedef factories still works."""
        SetI32 = self.sets_types.SetI32
        int_set = SetI32({1, 2, 3})
        self.assertEqual(set(int_set), {1, 2, 3})
        self.assertIsInstance(int_set, _fbthrift_python_types.Set)
        self.assertIsInstance(int_set, AbstractSetABC)

    def test_set_typedef_nested_construction(self) -> None:
        """Nested set typedefs also work with factories."""
        SetI32Lists = self.sets_types.SetI32Lists
        nested = SetI32Lists({(1, 2), (3, 4)})
        self.assertEqual(len(nested), 2)
        self.assertIsInstance(nested, _fbthrift_python_types.Set)

    def test_set_typedef_empty(self) -> None:
        SetI32 = self.sets_types.SetI32
        empty = SetI32(set())
        self.assertEqual(len(empty), 0)


class MapTypedefCompatibilityTest(unittest.TestCase):
    """
    Tests for map typedefs when container_typedefs is unavailable.
    Mirrors test_typedef_isinstance in maps.py but expects factory-based
    fallback behavior.
    """

    maps_types: ModuleType

    @classmethod
    def setUpClass(cls) -> None:
        cls.maps_types = _reload_module_without_container_typedefs(
            "python_test.maps.thrift_types"
        )

    def test_container_typedefs_not_available(self) -> None:
        self.assertIsNone(self.maps_types._fbthrift_python_container_typedefs)

    def test_map_typedef_is_factory_instance(self) -> None:
        """Without container_typedefs, map typedefs are MapTypeFactory instances."""
        self.assertIsInstance(
            self.maps_types.StrIntMap,
            # pyre-ignore[16]: MapTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.MapTypeFactory,
        )
        self.assertIsInstance(
            self.maps_types.AnotherStrIntMap,
            # pyre-ignore[16]: MapTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.MapTypeFactory,
        )
        self.assertIsInstance(
            self.maps_types.StrI32ListMap,
            # pyre-ignore[16]: MapTypeFactory is intentionally omitted from types.pyi
            _fbthrift_python_types.MapTypeFactory,
        )

    def test_map_typedef_construction(self) -> None:
        """Constructing maps via typedef factories still works."""
        StrIntMap = self.maps_types.StrIntMap
        int_map = StrIntMap({"foo": 1, "bar": 2})
        self.assertEqual(dict(int_map), {"foo": 1, "bar": 2})
        self.assertIsInstance(int_map, _fbthrift_python_types.Map)
        self.assertIsInstance(int_map, Mapping)

    def test_map_typedef_nested_construction(self) -> None:
        """Nested map typedefs also work with factories."""
        StrI32ListMap = self.maps_types.StrI32ListMap
        nested = StrI32ListMap({"a": [1, 2], "b": [3, 4]})
        self.assertEqual(len(nested), 2)
        self.assertEqual(list(nested["a"]), [1, 2])
        self.assertEqual(list(nested["b"]), [3, 4])
        self.assertIsInstance(nested, _fbthrift_python_types.Map)

    def test_map_typedef_empty(self) -> None:
        StrIntMap = self.maps_types.StrIntMap
        empty = StrIntMap({})
        self.assertEqual(len(empty), 0)
        self.assertEqual(dict(empty), {})
