#!/usr/bin/env python
# Copyright (c) Facebook, Inc. and its affiliates.
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

from enums_py3.types import EmptyEnum, MyEnum1


class EnumTest(unittest.TestCase):
    def test_foo(self):
        # Test EmptyEnum works, doesn't cause errors.
        print(EmptyEnum.__dict__)

    def test_values(self):
        # Check that all the enum values match what we expect
        my_enum1_values = [x.value for x in MyEnum1]
        self.assertEqual(min(my_enum1_values), 0)
        self.assertEqual(MyEnum1.ME1_0.value, 0)
        self.assertEqual(MyEnum1.ME1_1.value, 1)
        self.assertEqual(MyEnum1.ME1_2.value, 2)
        self.assertEqual(MyEnum1.ME1_3.value, 3)
        self.assertEqual(MyEnum1.ME1_5.value, 5)
        self.assertEqual(MyEnum1.ME1_6.value, 6)
        self.assertEqual(max(my_enum1_values), 6)

    def test_enum_names(self):
        self.assertEqual(MyEnum1.ME1_0.name, "ME1_0")

    def test_names_to_values(self):
        self.assertEqual(MyEnum1["ME1_0"], MyEnum1.ME1_0)

    def test_compare(self):
        self.assertEqual(MyEnum1.ME1_1, MyEnum1(1))
        self.assertEqual(MyEnum1.ME1_1.value, 1)
        self.assertEqual(MyEnum1.ME1_1.__hash__(), MyEnum1(1).__hash__())

        with self.assertWarns(RuntimeWarning):
            self.assertFalse(MyEnum1.ME1_1 == 1)
        with self.assertWarns(RuntimeWarning):
            self.assertTrue(MyEnum1.ME1_1 != 1)
        with self.assertRaises(TypeError):
            MyEnum1.ME1_1 > 0


if __name__ == "__main__":
    unittest.main()
