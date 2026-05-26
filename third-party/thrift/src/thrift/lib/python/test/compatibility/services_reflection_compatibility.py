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

"""Tests that services reflection gracefully degrades when
thrift.python.reflection.services_reflection is unavailable, simulating an
older thrift runtime.
"""

from __future__ import annotations

import contextlib
import importlib
import sys
import unittest
from collections.abc import Iterator
from types import ModuleType
from typing import Optional

from test_thrift.thrift_services import TestingServiceInterface
from thrift.python.reflection import inspect
from thrift.python.reflection.services_reflection import ServiceSpec

BLOCKED_MODULES = [
    "thrift.python.reflection.services_reflection",
]

SERVICES_MODULE_NAME = "test_thrift.thrift_services"
SERVICES_REFLECTION_MODULE_NAME = "test_thrift.thrift_services_reflection"


@contextlib.contextmanager
def _block_services_reflection() -> Iterator[ModuleType]:
    all_blocked = [*BLOCKED_MODULES, SERVICES_REFLECTION_MODULE_NAME]

    saved: dict[str, Optional[ModuleType]] = {}
    for name in all_blocked:
        saved[name] = sys.modules.get(name)
        sys.modules[name] = None  # type: ignore[assignment]

    saved_mod: Optional[ModuleType] = sys.modules.pop(SERVICES_MODULE_NAME, None)

    try:
        reloaded = importlib.import_module(SERVICES_MODULE_NAME)
        yield reloaded
    finally:
        sys.modules.pop(SERVICES_MODULE_NAME, None)
        if saved_mod is not None:
            sys.modules[SERVICES_MODULE_NAME] = saved_mod

        for name in all_blocked:
            prev = saved[name]
            if prev is not None:
                sys.modules[name] = prev
            else:
                sys.modules.pop(name, None)


class ServicesReflectionCompatibilityTest(unittest.TestCase):
    def test_module_loads_successfully(self) -> None:
        with _block_services_reflection() as services:
            self.assertIsNotNone(services)
            self.assertTrue(hasattr(services, "TestingServiceInterface"))

    def test_get_reflection_returns_none(self) -> None:
        with _block_services_reflection() as services:
            cls = services.TestingServiceInterface
            self.assertIsNone(cls.__get_reflection__())

    def test_inspect_returns_none(self) -> None:
        with _block_services_reflection() as services:
            cls = services.TestingServiceInterface
            self.assertIsNone(inspect(cls))

    def test_original_services_unaffected(self) -> None:
        spec = TestingServiceInterface.__get_reflection__()
        self.assertIsInstance(spec, ServiceSpec)
