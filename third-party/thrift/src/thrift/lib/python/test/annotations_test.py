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

import thrift.python.test.annotations.thrift_types as test_types

from thrift.python.types import StructOrUnion


class TypeInfoTests(unittest.TestCase):
    def test_python_Name_on_struct(self) -> None:
        """
        @python.Name{name = "RenamedEmpty"}
        struct Empty {}
        """
        with self.assertRaisesRegex(AttributeError, "has no attribute 'Empty'"):
            # pyre-ignore[16]: no attribute
            test_types.Empty()

        self.assertTrue(isinstance(test_types.RenamedEmpty(), StructOrUnion))

    def test_python_Name_on_field(self) -> None:
        """
        struct Struct {
          1: i32 first;
          @python.Name{name = "renamed_second"}
          2: i32 second;
        }
        """
        struct = test_types.Struct()

        with self.assertRaisesRegex(AttributeError, "has no attribute 'second'"):
            # pyre-ignore[16]: no attribute
            struct.second

        self.assertEqual(0, struct.renamed_second)
