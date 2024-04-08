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

from thrift.python.types import (
    AdaptedTypeInfo,
    EnumTypeInfo,
    ListTypeInfo,
    Map,
    MapTypeInfo,
    Set,
    SetTypeInfo,
    StructTypeInfo,
    TypeInfoBase,
    typeinfo_bool,
    typeinfo_i64,
    typeinfo_iobuf,
    typeinfo_string,
)
from thrift.python.test.containers.thrift_types import Foo, Bar
from thrift.python.test.adapters.atoi import AtoiAdapter

cdef class TypeInfoTests():
    def __cinit__(self, unit_test):
        self.ut = unit_test

    def test_ListTypeInfo(self) -> None:
        list_type_info = ListTypeInfo(typeinfo_i64)
        self.ut.assertTrue(isinstance(list_type_info, TypeInfoBase))

        with self.ut.assertRaises(TypeError):
            list_type_info.to_internal_data(None)

        data = list_type_info.to_internal_data([1, 2, 3])
        self.ut.assertEqual(data, (1, 2, 3))

    def test_ListTypeInfo_nested(self) -> None:
        element_type_info = ListTypeInfo(typeinfo_i64)
        list_type_info = ListTypeInfo(element_type_info)
        self.ut.assertTrue(isinstance(list_type_info, TypeInfoBase))

        init_val = [[1, 2], [3, 4], []]
        data = list_type_info.to_internal_data(init_val)
        self.ut.assertEqual(data, ((1, 2), (3, 4), ()))
        self.ut.assertEqual(list_type_info.to_python_value(data), init_val)

    def test_SetTypeInfo(self) -> None:
        set_type_info = SetTypeInfo(typeinfo_i64)
        self.ut.assertTrue(isinstance(set_type_info, TypeInfoBase))

        with self.ut.assertRaises(TypeError):
            set_type_info.to_internal_data(None)

        data = set_type_info.to_internal_data([1, 2, 3])
        self.ut.assertEqual(data, frozenset([1, 2, 3]))

        data = set_type_info.to_internal_data([1, 2, 3, 1, 2])
        self.ut.assertEqual(data, frozenset([1, 2, 3]))

    def test_SetTypeInfo_nested(self) -> None:
        element_type_info = SetTypeInfo(typeinfo_i64)
        set_type_info = SetTypeInfo(element_type_info)
        self.ut.assertTrue(isinstance(set_type_info, TypeInfoBase))

        init_val = [[1]]
        data = set_type_info.to_internal_data(init_val)
        self.ut.assertEqual(data, frozenset({frozenset([1])}))

        expected_python_val = Set(element_type_info, [[1]])
        self.ut.assertEqual(set_type_info.to_python_value(data), expected_python_val)

    def test_IntegerTypeInfo(self) -> None:
        self.ut.assertTrue(isinstance(typeinfo_i64, TypeInfoBase))
        with self.ut.assertRaises(TypeError):
            typeinfo_i64.to_internal_data(None)

        with self.ut.assertRaises(OverflowError):
            typeinfo_i64.to_internal_data(2**63)

        data = typeinfo_i64.to_internal_data(2**63 - 1)
        self.ut.assertEqual(data, 2**63 - 1)

    def test_StringTypeInfo(self) -> None:
        self.ut.assertTrue(isinstance(typeinfo_string, TypeInfoBase))
        with self.ut.assertRaises(TypeError):
            typeinfo_string.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            typeinfo_string.to_internal_data(b"AbC")

        data = typeinfo_string.to_internal_data("AbC")
        self.ut.assertEqual(data, b"AbC")

    def test_TypeInfo(self) -> None:
        self.ut.assertTrue(isinstance(typeinfo_bool, TypeInfoBase))
        with self.ut.assertRaises(TypeError):
            typeinfo_bool.to_internal_data(None)

        data = typeinfo_bool.to_internal_data(True)
        self.ut.assertEqual(data, True)

    def test_StructTypeInfo(self) -> None:
        struct_type_info = StructTypeInfo(Foo)
        self.ut.assertTrue(isinstance(struct_type_info, TypeInfoBase))

        with self.ut.assertRaises(TypeError):
            struct_type_info.to_internal_data(None)

        data = struct_type_info.to_internal_data(Foo(value=11))
        self.ut.assertEqual(
                struct_type_info.to_python_value(data),
                Foo(value=11))

    def test_EnumTypeInfo(self) -> None:
        enum_type_info = EnumTypeInfo(Bar)
        self.ut.assertTrue(isinstance(enum_type_info, TypeInfoBase))

        with self.ut.assertRaises(TypeError):
            enum_type_info.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            enum_type_info.to_python_value(None)

        data = enum_type_info.to_internal_data(Bar.ONE)
        self.ut.assertEqual(enum_type_info.to_python_value(data), Bar.ONE)

    def test_AdaptedTypeInfo(self) -> None:
        adapted_type_info = AdaptedTypeInfo(typeinfo_string, AtoiAdapter, lambda: None)
        self.ut.assertTrue(isinstance(adapted_type_info, TypeInfoBase))

        with self.ut.assertRaises(TypeError):
            adapted_type_info.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            adapted_type_info.to_python_value(None)

        data = adapted_type_info.to_internal_data(1)
        self.ut.assertEqual(typeinfo_string.to_internal_data("1"), data)

        internal_str_data = typeinfo_string.to_internal_data("21")
        self.ut.assertEqual(21, adapted_type_info.to_python_value(internal_str_data))

    def test_IOBufTypeInfo(self) -> None:
        self.ut.assertTrue(isinstance(typeinfo_iobuf, TypeInfoBase))
        
        with self.ut.assertRaises(TypeError):
            typeinfo_iobuf.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            self.ut.assertEqual("abc", typeinfo_iobuf.to_internal_data("abc"))

        self.ut.assertEqual(None, typeinfo_iobuf.to_python_value(None))
        self.ut.assertEqual("abc", typeinfo_iobuf.to_python_value("abc"))

    def test_MapTypeInfo(self) -> None:
        map_type_info = MapTypeInfo(typeinfo_string, typeinfo_i64)
        self.ut.assertTrue(isinstance(map_type_info, TypeInfoBase))

        with self.ut.assertRaises(TypeError):
            map_type_info.to_internal_data(None)

        data = map_type_info.to_internal_data({"a":1, "b":2})
        self.ut.assertEqual(data, ((b"a", 1), (b"b", 2)))
        expected_python_val = Map(typeinfo_string, typeinfo_i64, {"a": 1, "b":2})
        self.ut.assertEqual(map_type_info.to_python_value(data), expected_python_val)

