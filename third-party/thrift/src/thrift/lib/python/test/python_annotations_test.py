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

import types
import unittest
from typing import Type, Union

import thrift.python.test.annotations.thrift_mutable_types as mutable_test_types
import thrift.python.test.annotations.thrift_types as immutable_test_types
from parameterized import parameterized
from thrift.python.mutable_types import MutableStructOrUnion
from thrift.python.types import StructOrUnion


class TestPythonAnnotations(unittest.TestCase):
    @parameterized.expand(
        [
            (immutable_test_types, StructOrUnion),
            (mutable_test_types, MutableStructOrUnion),
        ]
    )
    def test_python_Name_on_struct(
        self,
        test_types: types.ModuleType,
        base_type: Type[Union[StructOrUnion, MutableStructOrUnion]],
    ) -> None:
        """
        @python.Name{name = "RenamedEmpty"}
        struct Empty {}
        """
        with self.assertRaisesRegex(AttributeError, "has no attribute 'Empty'"):
            test_types.Empty()

        self.assertIsInstance(test_types.RenamedEmpty(), base_type)

    @parameterized.expand([immutable_test_types, mutable_test_types])
    def test_python_Name_on_field(self, test_types: types.ModuleType) -> None:
        """
        struct Struct {
          1: i32 first;
          @python.Name{name = "renamed_second"}
          2: i32 second;
        }
        """
        struct = test_types.Struct()

        with self.assertRaisesRegex(AttributeError, "has no attribute 'second'"):
            struct.second

        self.assertEqual(0, struct.renamed_second)

    def test_python_Name_on_exception_message(self) -> None:
        """
        exception ExceptionWithMessage {
            @thrift.ExceptionMessage
            @python.Name{name = "message"}
            1: string msg;
        }
        """
        with self.assertRaisesRegex(
            immutable_test_types.ExceptionWithMessage,
            "Oh Noes!!!",
        ):
            raise immutable_test_types.ExceptionWithMessage(message="Oh Noes!!!")
