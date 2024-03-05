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

from testing.types import F14MapFollyString, SetI32Lists, StrList2D
from thrift.py3.cache import to_frozenset, to_mappingproxy, to_tuple


class CachedContainerTest(unittest.TestCase):
    def test_cached_list(self) -> None:
        uncached = StrList2D([["one", "two"], ["cyan", "magenta"]])
        # default no cache, so every access creates a new instance
        self.assertIsNot(uncached[0], uncached[0])
        # with cache they should be same
        self.assertIs(to_tuple(uncached)[0], to_tuple(uncached)[0])
        # nested list are still not cached by default
        self.assertIsNot(to_tuple(uncached)[0][0], to_tuple(uncached)[0][0])
        self.assertIs(
            to_tuple(to_tuple(uncached)[0])[0], to_tuple(to_tuple(uncached)[0])[0]
        )

    def test_cached_set(self) -> None:
        uncached = SetI32Lists({(0, 1, 2)})
        # default no cache, so every access creates a new instance
        self.assertIsNot(list(uncached)[0], list(uncached)[0])
        # with cache they should be same
        self.assertIs(list(to_frozenset(uncached))[0], list(to_frozenset(uncached))[0])

    def test_cached_map(self) -> None:
        uncached = F14MapFollyString({"foo": "foo_value"})
        # default no cache, so every access creates a new instance
        self.assertIsNot(uncached["foo"], uncached["foo"])
        # with cache they should be same
        self.assertIs(
            to_mappingproxy(uncached)["foo"], to_mappingproxy(uncached)["foo"]
        )
