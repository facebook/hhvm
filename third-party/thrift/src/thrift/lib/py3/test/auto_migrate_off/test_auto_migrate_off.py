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

import unittest

import testing.thrift_types

import testing.types

from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated
from thrift.py3.test.auto_migrate_off.module.thrift_types import TestStruct


class AutoMigrateOffTests(unittest.TestCase):
    def test_auto_migrate_off(self) -> None:
        self.assertIsNot(testing.types.Nested1, testing.thrift_types.Nested1)

    def test_auto_migrate_util(self) -> None:
        self.assertFalse(is_auto_migrated())
        # pyre-ignore[16]: Undefined attribute testing a non-public API
        self.assertFalse(testing.thrift_types.Nested1._fbthrift_auto_migrate_enabled())

    # this tests the case where auto-migrate is set in PACKAGE. The above
    # tests a thrift library where auto-migrate is not set at PACKAGE
    def test_auto_migrate_cls_attribute_generation(self) -> None:
        # pyre-ignore[16]: Undefined attribute testing a non-public API
        self.assertFalse(TestStruct._fbthrift_auto_migrate_enabled())
