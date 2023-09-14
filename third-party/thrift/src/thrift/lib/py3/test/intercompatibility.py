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

import testing.thrift_types as python_types
import testing.types as py3_types


class PythonCompatibilityTest(unittest.TestCase):
    def test_init_py3_struct_with_python_union(self) -> None:
        python_integers = python_types.Integers(small=2023)
        py3_easy = py3_types.easy(an_int=python_integers)
        self.assertEqual(2023, py3_easy.an_int.small)

    def test_update_py3_struct_with_python_union(self) -> None:
        py3_easy = py3_types.easy()
        python_integers = python_types.Integers(small=2023)
        py3_easy = py3_easy(an_int=python_integers)
        self.assertEqual(2023, py3_easy.an_int.small)

    def test_init_py3_struct_with_python_enum(self) -> None:
        py3_file = py3_types.File(
            permissions=(python_types.Perm.read | python_types.Perm.read),
            type=python_types.Kind.FIFO,
        )
        self.assertEqual(
            py3_types.Perm.read | py3_types.Perm.read,
            py3_file.permissions,
        )
        self.assertEqual(py3_types.Kind.FIFO, py3_file.type)

    def test_update_py3_struct_with_python_enum(self) -> None:
        py3_file = py3_types.File()
        py3_file = py3_file(
            permissions=(python_types.Perm.read | python_types.Perm.read),
            type=python_types.Kind.FIFO,
        )
        self.assertEqual(
            py3_types.Perm.read | py3_types.Perm.read,
            py3_file.permissions,
        )
        self.assertEqual(py3_types.Kind.FIFO, py3_file.type)

    def test_init_py3_union_with_python_struct(self) -> None:
        python_easy = python_types.easy(name="foo")
        py3_complex_union = py3_types.ComplexUnion(easy_struct=python_easy)
        self.assertEqual("foo", py3_complex_union.easy_struct.name)
