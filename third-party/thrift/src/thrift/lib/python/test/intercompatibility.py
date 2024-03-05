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

import testing.thrift_types as python_types
import testing.types as py3_types


class Py3CompatibilityTest(unittest.TestCase):
    def test_init_python_struct_with_py3_union(self) -> None:
        py3_integers = py3_types.Integers(small=2023)
        python_easy = python_types.easy(an_int=py3_integers)
        self.assertEqual(2023, python_easy.an_int.small)

    def test_update_python_struct_with_py3_union(self) -> None:
        python_easy = python_types.easy()
        py3_integers = py3_types.Integers(small=2023)
        python_easy = python_easy(an_int=py3_integers)
        self.assertEqual(2023, python_easy.an_int.small)

    def test_init_python_struct_with_py3_enum(self) -> None:
        python_file = python_types.File(
            permissions=(py3_types.Perm.read | py3_types.Perm.read),
            type=py3_types.Kind.FIFO,
        )
        self.assertEqual(
            python_types.Perm.read | python_types.Perm.read,
            python_file.permissions,
        )
        self.assertEqual(python_types.Kind.FIFO, python_file.type)

    def test_update_python_struct_with_py3_enum(self) -> None:
        python_file = python_types.File()
        python_file = python_file(
            permissions=(py3_types.Perm.read | py3_types.Perm.read),
            type=py3_types.Kind.FIFO,
        )
        self.assertEqual(
            python_types.Perm.read | python_types.Perm.read,
            python_file.permissions,
        )
        self.assertEqual(python_types.Kind.FIFO, python_file.type)

    def test_init_python_union_with_py3_struct(self) -> None:
        py3_easy = py3_types.easy(name="foo")
        python_complex_union = python_types.ComplexUnion(easy_struct=py3_easy)
        self.assertEqual("foo", python_complex_union.easy_struct.name)
