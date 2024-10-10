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

import unittest

from facebook.thrift.compiler.test.fixtures.default_values.module.thrift_types import (
    StructWithCustomDefaultValues,
    StructWithNoCustomDefaultValues,
    TrivialStruct,
)


class ThriftPythonDefaultValuesTest(unittest.TestCase):
    def test_intrinsic_default_values(self) -> None:
        """
        Tests intrinsic (i.e., non-custom) default values for struct fields.
        """
        self.assertEqual(
            StructWithNoCustomDefaultValues(),
            StructWithNoCustomDefaultValues(
                unqualified_integer=0,
                optional_integer=None,
                required_integer=0,
                unqualified_struct=TrivialStruct(int_value=0),
                optional_struct=None,
                required_struct=TrivialStruct(int_value=0),
            ),
        )

    def test_custom_default_values(self) -> None:
        """
        Tests custom default values for struct fields.
        """

        # Note how custom default values for `optional` fields are ignored.
        self.assertEqual(
            StructWithCustomDefaultValues(),
            StructWithCustomDefaultValues(
                unqualified_integer=42,
                optional_integer=None,
                required_integer=44,
                unqualified_struct=TrivialStruct(int_value=123),
                optional_struct=None,
                required_struct=TrivialStruct(int_value=789),
            ),
        )
