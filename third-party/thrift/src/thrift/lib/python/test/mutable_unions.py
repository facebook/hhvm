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

from testing.thrift_mutable_types import Digits, Integers

from thrift.python.mutable_types import to_thrift_list


class ThriftPython_MutableUnion_Test(unittest.TestCase):
    def setUp(self) -> None:
        # Disable maximum printed diff length.
        self.maxDiff = None

    def test_union_instances(self) -> None:
        """
        struct Digits {
          1: optional list<Integers> data;
        }
        union Integers {
          1: byte tiny;
          2: i16 small;
          3: i32 medium;
          ...
        }
        """
        digit = Integers(tiny=1)
        self.assertEqual(
            digit.FbThriftUnionFieldEnum.tiny, digit.fbthrift_current_field
        )
        self.assertEqual(1, digit.fbthrift_current_value)

        # Initialize the list with the same instance of `digit`.
        # `to_thrift_list()` uses reference semantics if it contains union
        # instances.
        digits = Digits(data=to_thrift_list([digit, digit]))

        # `digit`, `digits.data[0]`, and `digits.data[1]` are the same union
        # instances
        self.assertEqual(1, digit.fbthrift_current_value)
        # pyre-ignore[16]: Optional type has no attribute `__getitem__`
        self.assertEqual(1, digits.data[0].fbthrift_current_value)
        self.assertEqual(1, digits.data[1].fbthrift_current_value)

        # Updating the `digit` instance will update all three instances.
        digit.small = 5
        self.assertEqual(5, digit.fbthrift_current_value)
        self.assertEqual(5, digits.data[0].fbthrift_current_value)
        self.assertEqual(5, digits.data[1].fbthrift_current_value)

        # Updating through the `digits.data[0]` instance reveals a caching issue.
        # All of them should have the current value of 10.
        digits.data[0].medium = 10
        self.assertEqual(5, digit.fbthrift_current_value)
        self.assertEqual(10, digits.data[0].fbthrift_current_value)
        self.assertEqual(10, digits.data[1].fbthrift_current_value)
