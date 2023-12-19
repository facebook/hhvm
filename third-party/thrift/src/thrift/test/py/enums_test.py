#!/usr/bin/env python
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

from __future__ import absolute_import, division, print_function, unicode_literals

import unittest

from .enums.ttypes import MyEnum1, MyEnum2, MyEnum3, MyEnum4, MyStruct


class EnumTest(unittest.TestCase):
    def test_values(self):
        # Check that all the enum values match what we expect
        print(MyEnum1.ME1_0, 0)
        self.assertEqual(min(MyEnum1._VALUES_TO_NAMES), 0)
        self.assertEqual(MyEnum1.ME1_0, 0)
        self.assertEqual(MyEnum1.ME1_1, 1)
        self.assertEqual(MyEnum1.ME1_2, 2)
        self.assertEqual(MyEnum1.ME1_3, 3)
        self.assertEqual(MyEnum1.ME1_5, 5)
        self.assertEqual(MyEnum1.ME1_6, 6)
        self.assertEqual(max(MyEnum1._VALUES_TO_NAMES), 6)

        self.assertEqual(min(MyEnum2._VALUES_TO_NAMES), 0)
        self.assertEqual(MyEnum2.ME2_0, 0)
        self.assertEqual(MyEnum2.ME2_1, 1)
        self.assertEqual(MyEnum2.ME2_2, 2)
        self.assertEqual(max(MyEnum2._VALUES_TO_NAMES), 2)

        self.assertEqual(min(MyEnum3._VALUES_TO_NAMES), -2)
        self.assertEqual(MyEnum3.ME3_0, 0)
        self.assertEqual(MyEnum3.ME3_1, 1)
        self.assertEqual(MyEnum3.ME3_N2, -2)
        self.assertEqual(MyEnum3.ME3_N1, -1)
        self.assertEqual(MyEnum3.ME3_9, 9)
        self.assertEqual(MyEnum3.ME3_10, 10)
        self.assertEqual(max(MyEnum3._VALUES_TO_NAMES), 10)

        self.assertEqual(min(MyEnum4._VALUES_TO_NAMES), 0x7FFFFFFD)
        self.assertEqual(MyEnum4.ME4_A, 0x7FFFFFFD)
        self.assertEqual(MyEnum4.ME4_B, 0x7FFFFFFE)
        self.assertEqual(MyEnum4.ME4_C, 0x7FFFFFFF)
        self.assertEqual(max(MyEnum4._VALUES_TO_NAMES), 0x7FFFFFFF)

    def test_struct(self):
        ms = MyStruct()
        self.assertEqual(ms.me2_2, 2)
        self.assertEqual(ms.me3_n2, -2)

    def test_enum_names(self):
        self.assertEqual(MyEnum2._VALUES_TO_NAMES[MyEnum2.ME2_2], "ME2_2")

    def test_names_to_values(self):
        self.assertEqual(MyEnum2._NAMES_TO_VALUES["ME2_2"], MyEnum2.ME2_2)
        self.assertEqual(MyEnum3._NAMES_TO_VALUES["ME3_N2"], MyEnum3.ME3_N2)

    def test_compare(self):
        self.assertTrue(MyEnum1.ME1_0 < MyEnum1.ME1_1)


if __name__ == "__main__":
    unittest.main()
