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


from __future__ import annotations

import unittest

from thrift.python.test.adapters.atoi import (
    AtoiAdapter,
    ItoaListAdapter,
    ItoaNestedListAdapter,
)


class AtoiAdapterTest(unittest.TestCase):
    def test_round_trip(self) -> None:
        a = "42"
        i = AtoiAdapter.from_thrift(a)
        self.assertEqual(a, AtoiAdapter.to_thrift(i))


class ItoaListAdapterTest(unittest.TestCase):
    def test_round_trip(self) -> None:
        ints = [1, 10, 100, 1000]
        strs = ItoaListAdapter.from_thrift(ints)
        self.assertEqual(ints, ItoaListAdapter.to_thrift(strs))


class ItoaNestedListAdapterTest(unittest.TestCase):
    def test_round_trip(self) -> None:
        ints = [[{1: 2}, {10: 11}], [{100: 123}], [{1000: 2200}]]
        strs = ItoaNestedListAdapter.from_thrift(ints)
        self.assertEqual(ints, ItoaNestedListAdapter.to_thrift(strs))
