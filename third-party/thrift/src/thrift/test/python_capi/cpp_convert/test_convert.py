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

import thrift.test.python_capi.cpp_convert.lib as lib

from thrift.test.python_capi.thrift_dep.thrift_types import DepEnum, DepStruct, DepUnion


class TestConvert(unittest.TestCase):
    def test_enum_from_cpp(self) -> None:
        self.assertEqual(DepEnum.Arm2, lib.enum_from_cpp())

    def test_struct_from_cpp(self) -> None:
        s = lib.struct_from_cpp()
        self.assertEqual(s.s, "Hello")
        self.assertEqual(s.i, 42)

    def test_union_from_cpp(self) -> None:
        self.assertEqual("World", lib.union_from_cpp().s)

    def test_enum_to_cpp(self) -> None:
        self.assertEqual(DepEnum.Arm2, DepEnum(lib.unpack_python_enum(DepEnum.Arm2)))

    def test_struct_to_cpp(self) -> None:
        self.assertEqual("Hey: 3", lib.unpack_python_struct(DepStruct(s="Hey", i=3)))

    def test_union_to_cpp(self) -> None:
        self.assertEqual("Yo", lib.unpack_python_union(DepUnion(s="Yo")))
        self.assertEqual(0, lib.unpack_python_union(DepUnion(i=0)))
        # max int for DepUnion.i is 2^63-1 because thrift type is i64
        self.assertEqual(2**63 - 1, lib.unpack_python_union(DepUnion(i=2**63 - 1)))

        self.assertEqual("Yo", lib.unpack_python_union_throws(DepUnion(s="Yo")))
        self.assertEqual(
            2**63 - 1, lib.unpack_python_union_throws(DepUnion(i=2**63 - 1))
        )

    def test_union_to_cpp_overflow(self) -> None:
        # should raise OverflowError because cpp.Type is uint64_t
        with self.assertRaises(OverflowError):
            lib.unpack_python_union(DepUnion(i=-1))
        # handlePythonError always throws std::runtime_error, which is translated to
        # RuntimeError by cython bindings
        with self.assertRaises(RuntimeError):
            lib.unpack_python_union_throws(DepUnion(i=-1))
