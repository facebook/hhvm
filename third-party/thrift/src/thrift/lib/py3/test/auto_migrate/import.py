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

# deliberately importing .containers before .types
# this validates we can handle some unfortunate pickling behavior
# please always import containers from .types
from testing.containers_FBTHRIFT_ONLY_DO_NOT_USE import List__i32
from testing.types import I32List


class Import(unittest.TestCase):
    def test_direct_import(self) -> None:
        self.assertEqual(I32List(list(range(3))), list(range(3)))
        self.assertEqual(List__i32(list(range(3))), list(range(3)))
