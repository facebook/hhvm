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

# cython: c_string_type=unicode, c_string_encoding=utf8

from thrift.python.types import ListTypeInfo, Set, SetTypeInfo, typeinfo_i64, typeinfo_string

cdef class TypeInfoTests():
    def __cinit__(self, unit_test):
        self.ut = unit_test

    def test_ListTypeInfo(self) -> None:
        list_type_info = ListTypeInfo(typeinfo_i64)

        with self.ut.assertRaises(TypeError):
            list_type_info.to_internal_data(None)

        data = list_type_info.to_internal_data([1, 2, 3])
        self.ut.assertEqual(data, (1, 2, 3))

    def test_ListTypeInfo_nested(self) -> None:
        element_type_info = ListTypeInfo(typeinfo_i64)
        list_type_info = ListTypeInfo(element_type_info)

        init_val = [[1, 2], [3, 4], []]
        data = list_type_info.to_internal_data(init_val)
        self.ut.assertEqual(data, ((1, 2), (3, 4), ()))
        self.ut.assertEqual(list_type_info.to_python_value(data), init_val)

    def test_SetTypeInfo(self) -> None:
        set_type_info = SetTypeInfo(typeinfo_i64)

        with self.ut.assertRaises(TypeError):
            set_type_info.to_internal_data(None)

        data = set_type_info.to_internal_data([1, 2, 3])
        self.ut.assertEqual(data, frozenset([1, 2, 3]))

        data = set_type_info.to_internal_data([1, 2, 3, 1, 2])
        self.ut.assertEqual(data, frozenset([1, 2, 3]))

    def test_SetTypeInfo_nested(self) -> None:
        element_type_info = SetTypeInfo(typeinfo_i64)
        set_type_info = SetTypeInfo(element_type_info)

        init_val = [[1]]
        data = set_type_info.to_internal_data(init_val)
        self.ut.assertEqual(data, frozenset({frozenset([1])}))

        expected_python_val = Set(element_type_info, [[1]])
        self.ut.assertEqual(set_type_info.to_python_value(data), expected_python_val)

    def test_IntegerTypeInfo(self) -> None:
        with self.ut.assertRaises(TypeError):
            typeinfo_i64.to_internal_data(None)

        with self.ut.assertRaises(OverflowError):
            typeinfo_i64.to_internal_data(2**63)

        data = typeinfo_i64.to_internal_data(2**63 - 1)
        self.ut.assertEqual(data, 2**63 - 1)

    def test_StringTypeInfo(self) -> None:
        with self.ut.assertRaises(TypeError):
            typeinfo_string.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            typeinfo_string.to_internal_data(b"AbC")

        data = typeinfo_string.to_internal_data("AbC")
        self.ut.assertEqual(data, b"AbC")
