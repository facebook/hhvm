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

"""Tests that reflection gracefully degrades when thrift.python.reflection
and/or thrift.python.container_typedefs are unavailable, simulating an older
thrift runtime.
"""

from __future__ import annotations

import contextlib
import importlib
import sys
import unittest
from collections.abc import Iterator
from types import ModuleType
from typing import Optional

from parameterized import parameterized
from test_thrift.thrift_types import SimpleStruct
from thrift.python.reflection import inspect, inspectable

BLOCKED_MODULES = [
    "thrift.python.container_typedefs",
    "thrift.python.reflection",
    "thrift.python.reflection.types_reflection",
    "thrift.python.reflection.constants_reflection",
]

MODULE_NAME = "test_thrift.thrift_types"
REFLECTION_MODULE_NAME = "test_thrift.thrift_reflection"


@contextlib.contextmanager
def _block_reflection_modules() -> Iterator[ModuleType]:
    """Reload test_thrift.thrift_types with reflection modules blocked.

    The blocked state persists for the lifetime of the context manager,
    so __get_reflection__() calls inside the block will return None.
    """
    all_blocked = [*BLOCKED_MODULES, REFLECTION_MODULE_NAME]

    saved: dict[str, Optional[ModuleType]] = {}
    for name in all_blocked:
        saved[name] = sys.modules.get(name)
        sys.modules[name] = None  # type: ignore[assignment]

    saved_mod: Optional[ModuleType] = sys.modules.pop(MODULE_NAME, None)

    try:
        reloaded = importlib.import_module(MODULE_NAME)
        yield reloaded
    finally:
        sys.modules.pop(MODULE_NAME, None)
        if saved_mod is not None:
            sys.modules[MODULE_NAME] = saved_mod

        for name in all_blocked:
            prev = saved[name]
            if prev is not None:
                sys.modules[name] = prev
            else:
                sys.modules.pop(name, None)


class ReflectionCompatibilityTest(unittest.TestCase):
    def test_module_loads_successfully(self) -> None:
        with _block_reflection_modules() as types:
            self.assertIsNotNone(types)
            self.assertTrue(hasattr(types, "SimpleStruct"))

    @parameterized.expand(
        [
            ("struct", "SimpleStruct"),
            ("union", "Integers"),
            ("exception", "HardError"),
            ("annotated_struct", "AnnotatedForReflection"),
        ]
    )
    def test_get_reflection_returns_none(self, _name: str, type_name: str) -> None:
        with _block_reflection_modules() as types:
            cls = getattr(types, type_name)
            self.assertIsNone(cls.__get_reflection__())
            self.assertIsNone(inspect(cls))

    def test_original_types_unaffected(self) -> None:
        self.assertTrue(inspectable(SimpleStruct))
        self.assertIsNotNone(inspect(SimpleStruct))
