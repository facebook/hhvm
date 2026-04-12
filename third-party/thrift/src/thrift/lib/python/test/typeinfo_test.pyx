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
    Map,
    Set,
    typeinfo_bool,
    typeinfo_double,
    typeinfo_float,
    typeinfo_i32,
    typeinfo_i64,
    typeinfo_iobuf,
    typeinfo_string,
)
from thrift.python.types cimport (
    AdaptedTypeInfo,
    EnumTypeInfo,
    IntegerTypeInfo,
    IOBufTypeInfo,
    ListTypeInfo,
    MapTypeInfo,
    SetTypeInfo,
    StringTypeInfo,
    StructTypeInfo,
    TypeInfo,
    TypeInfoBase,
    getCTypeInfo,
    i32TypeInfo,
)
from thrift.python.mutable_typeinfos cimport (
    MutableListTypeInfo,
    MutableSetTypeInfo,
    MutableMapTypeInfo,
    MutableStructTypeInfo,
)
from thrift.python.mutable_containers cimport (
    MutableMap,
    MutableSet,
)
from python_test.containers.thrift_types import (
    Foo,
    Bar,
    OtherFoo,
    OtherBar,
)
from python_test.containers.thrift_mutable_types import (
    Foo as FooMutable,
    OtherFoo as OtherFooMutable,
)
from thrift.python.test.adapters.atoi import AtoiAdapter
from thrift.python.test.adapters.datetime import DatetimeAdapter
from thrift.python.mutable_types import (
    _ThriftListWrapper,
    _ThriftSetWrapper,
    _ThriftMapWrapper,
)

cdef class TypeInfoTests():
    def __cinit__(self, unit_test):
        self.ut = unit_test

    cdef assertEqual(self, const cTypeInfo *lhs, const cTypeInfo *rhs):
        return self.ut.assertTrue(lhs == rhs)

    def test_ListTypeInfo(self) -> None:
        list_type_info = ListTypeInfo(typeinfo_i64)
        self.ut.assertIsInstance(list_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            list_type_info.to_internal_data(None)

        data = list_type_info.to_internal_data([1, 2, 3])
        self.ut.assertEqual(data, (1, 2, 3))

        self.assertEqual(
            (<ListTypeInfo>list_type_info).cpp_obj.get().get(),
            getCTypeInfo(list_type_info),
        )

        self.ut.assertTrue(list_type_info.same_as(list_type_info))
        self.ut.assertTrue(ListTypeInfo(typeinfo_i64).same_as(list_type_info))
        self.ut.assertFalse(ListTypeInfo(typeinfo_i32).same_as(list_type_info))

        with self.ut.assertRaisesRegex(
            NotImplementedError,
            "Use the 'same_as' method for comparing TypeInfoBase instances."):
            list_type_info == list_type_info

        self.ut.assertTrue(list_type_info.is_container())

    def test_ListTypeInfo_nested(self) -> None:
        element_type_info = ListTypeInfo(typeinfo_i64)
        list_type_info = ListTypeInfo(element_type_info)
        self.ut.assertIsInstance(list_type_info, TypeInfoBase)

        init_val = [[1, 2], [3, 4], []]
        data = list_type_info.to_internal_data(init_val)
        self.ut.assertEqual(data, ((1, 2), (3, 4), ()))
        self.ut.assertEqual(list_type_info.to_python_value(data), init_val)

    def test_SetTypeInfo(self) -> None:
        set_type_info = SetTypeInfo(typeinfo_i64)
        self.ut.assertIsInstance(set_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            set_type_info.to_internal_data(None)

        data = set_type_info.to_internal_data([1, 2, 3])
        self.ut.assertEqual(data, frozenset([1, 2, 3]))

        data = set_type_info.to_internal_data([1, 2, 3, 1, 2])
        self.ut.assertEqual(data, frozenset([1, 2, 3]))

        self.assertEqual(
            (<SetTypeInfo>set_type_info).cpp_obj.get().get(),
            getCTypeInfo(set_type_info),
        )

        self.ut.assertTrue(set_type_info.same_as(set_type_info))
        self.ut.assertTrue(SetTypeInfo(typeinfo_i64).same_as(set_type_info))
        self.ut.assertFalse(SetTypeInfo(typeinfo_i32).same_as(set_type_info))
        self.ut.assertTrue(set_type_info.is_container())

    def test_SetTypeInfo_nested(self) -> None:
        element_type_info = SetTypeInfo(typeinfo_i64)
        set_type_info = SetTypeInfo(element_type_info)
        self.ut.assertIsInstance(set_type_info, TypeInfoBase)

        init_val = [[1]]
        data = set_type_info.to_internal_data(init_val)
        self.ut.assertEqual(data, frozenset({frozenset([1])}))

        expected_python_val = Set(element_type_info, [[1]])
        self.ut.assertEqual(set_type_info.to_python_value(data), expected_python_val)

    def test_IntegerTypeInfo(self) -> None:
        self.ut.assertIsInstance(typeinfo_i64, TypeInfoBase)
        with self.ut.assertRaises(TypeError):
            typeinfo_i64.to_internal_data(None)

        with self.ut.assertRaises(OverflowError):
            typeinfo_i64.to_internal_data(2**63)

        data = typeinfo_i64.to_internal_data(2**63 - 1)
        self.ut.assertEqual(data, 2**63 - 1)

        self.assertEqual(
            (<IntegerTypeInfo>typeinfo_i64).cpp_obj,
            getCTypeInfo(typeinfo_i64),
        )

        self.ut.assertTrue(typeinfo_i64.same_as(typeinfo_i64))
        self.ut.assertFalse(typeinfo_i32.same_as(typeinfo_i64))
        self.ut.assertFalse(typeinfo_i32.is_container())

    def test_StringTypeInfo(self) -> None:
        self.ut.assertIsInstance(typeinfo_string, TypeInfoBase)
        with self.ut.assertRaises(TypeError):
            typeinfo_string.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            typeinfo_string.to_internal_data(b"AbC")

        data = typeinfo_string.to_internal_data("AbC")
        self.ut.assertEqual(data, "AbC")

        self.assertEqual(
            (<StringTypeInfo>typeinfo_string).cpp_obj,
            getCTypeInfo(typeinfo_string),
        )

        self.ut.assertTrue(typeinfo_string.same_as(typeinfo_string))
        self.ut.assertFalse(typeinfo_i32.same_as(typeinfo_string))
        self.ut.assertFalse(typeinfo_string.is_container())

    def test_TypeInfo(self) -> None:
        # `typeinfo_{bool,float,double}` are instances of `TypeInfo`
        self.ut.assertIsInstance(typeinfo_bool, TypeInfoBase)
        with self.ut.assertRaises(TypeError):
            typeinfo_bool.to_internal_data(None)

        data = typeinfo_bool.to_internal_data(True)
        self.ut.assertEqual(data, True)

        self.assertEqual(
            (<TypeInfo>typeinfo_bool).cpp_obj,
            getCTypeInfo(typeinfo_bool),
        )

        self.ut.assertTrue(typeinfo_bool.same_as(typeinfo_bool))
        self.ut.assertTrue(typeinfo_float.same_as(typeinfo_float))
        self.ut.assertTrue(typeinfo_double.same_as(typeinfo_double))

        self.ut.assertFalse(typeinfo_float.same_as(typeinfo_double))
        self.ut.assertFalse(typeinfo_double.same_as(typeinfo_float))
        self.ut.assertFalse(typeinfo_bool.same_as(typeinfo_float))
        self.ut.assertFalse(typeinfo_bool.is_container())

    def test_StructTypeInfo(self) -> None:
        struct_type_info = StructTypeInfo(Foo)
        self.ut.assertIsInstance(struct_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            struct_type_info.to_internal_data(None)

        data = struct_type_info.to_internal_data(Foo(value=11))
        self.ut.assertEqual(
                struct_type_info.to_python_value(data),
                Foo(value=11))

        self.assertEqual(
            &(<StructTypeInfo>struct_type_info).cpp_obj,
            getCTypeInfo(struct_type_info),
        )

        self.ut.assertTrue(struct_type_info.same_as(struct_type_info))
        self.ut.assertTrue(StructTypeInfo(Foo).same_as(struct_type_info))
        self.ut.assertFalse(StructTypeInfo(OtherFoo).same_as(struct_type_info))
        self.ut.assertFalse(struct_type_info.is_container())

    def test_EnumTypeInfo(self) -> None:
        enum_type_info = EnumTypeInfo(Bar)
        self.ut.assertIsInstance(enum_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            enum_type_info.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            enum_type_info.to_python_value(None)

        data = enum_type_info.to_internal_data(Bar.ONE)
        self.ut.assertEqual(enum_type_info.to_python_value(data), Bar.ONE)

        self.assertEqual(
            &i32TypeInfo,
            getCTypeInfo(enum_type_info),
        )

        self.ut.assertTrue(enum_type_info.same_as(enum_type_info))
        self.ut.assertTrue(EnumTypeInfo(Bar).same_as(enum_type_info))
        self.ut.assertFalse(EnumTypeInfo(OtherBar).same_as(enum_type_info))
        self.ut.assertFalse(enum_type_info.is_container())

    def test_AdaptedTypeInfo(self) -> None:
        adapted_type_info = AdaptedTypeInfo(typeinfo_string, AtoiAdapter, lambda: None)
        self.ut.assertIsInstance(adapted_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            adapted_type_info.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            adapted_type_info.to_python_value(None)

        data = adapted_type_info.to_internal_data(1)
        self.ut.assertEqual(typeinfo_string.to_internal_data("1"), data)

        internal_str_data = typeinfo_string.to_internal_data("21")
        self.ut.assertEqual(21, adapted_type_info.to_python_value(internal_str_data))

        self.assertEqual(
            (<StringTypeInfo>typeinfo_string).cpp_obj,
            getCTypeInfo(adapted_type_info),
        )

        self.ut.assertTrue(adapted_type_info.same_as(adapted_type_info))
        self.ut.assertTrue(AdaptedTypeInfo(typeinfo_string, AtoiAdapter, lambda: None).same_as(adapted_type_info))
        self.ut.assertFalse(AdaptedTypeInfo(typeinfo_string, DatetimeAdapter, lambda: None).same_as(adapted_type_info))
        self.ut.assertFalse(adapted_type_info.is_container())

    def test_IOBufTypeInfo(self) -> None:
        self.ut.assertIsInstance(typeinfo_iobuf, TypeInfoBase)
        with self.ut.assertRaises(TypeError):
            typeinfo_iobuf.to_internal_data(None)

        with self.ut.assertRaises(TypeError):
            self.ut.assertEqual("abc", typeinfo_iobuf.to_internal_data("abc"))

        self.ut.assertEqual(None, typeinfo_iobuf.to_python_value(None))
        self.ut.assertEqual("abc", typeinfo_iobuf.to_python_value("abc"))

        self.assertEqual(
            (<IOBufTypeInfo>typeinfo_iobuf).cpp_obj,
            getCTypeInfo(typeinfo_iobuf),
        )

        self.ut.assertTrue(typeinfo_iobuf.same_as(typeinfo_iobuf))
        self.ut.assertFalse(typeinfo_string.same_as(typeinfo_iobuf))
        self.ut.assertFalse(typeinfo_iobuf.is_container())

    def test_MapTypeInfo(self) -> None:
        map_type_info = MapTypeInfo(typeinfo_string, typeinfo_i64)
        self.ut.assertIsInstance(map_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            map_type_info.to_internal_data(None)

        data = map_type_info.to_internal_data({"a":1, "b":2})
        self.ut.assertEqual(data, {'a': 1, 'b': 2})
        expected_python_val = Map(typeinfo_string, typeinfo_i64, {"a": 1, "b":2})
        self.ut.assertEqual(map_type_info.to_python_value(data), expected_python_val)

        self.assertEqual(
            (<MapTypeInfo>map_type_info).cpp_obj.get().get(),
            getCTypeInfo(map_type_info),
        )

        self.ut.assertTrue(map_type_info.same_as(map_type_info))
        self.ut.assertTrue(MapTypeInfo(typeinfo_string, typeinfo_i64).same_as(map_type_info))
        self.ut.assertFalse(MapTypeInfo(typeinfo_string, typeinfo_i32).same_as(map_type_info))
        self.ut.assertFalse(MapTypeInfo(typeinfo_i32, typeinfo_i64).same_as(map_type_info))
        self.ut.assertTrue(map_type_info.is_container())

    def test_MapTypeInfo_nested(self) -> None:
        # Map[str, Map[int, List[int]]]
        inner_value_type = ListTypeInfo(typeinfo_i64)
        value_type_info = MapTypeInfo(typeinfo_i64, inner_value_type)
        map_type_info = MapTypeInfo(typeinfo_string, value_type_info)
        self.ut.assertIsInstance(map_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            map_type_info.to_internal_data(None)

        data = map_type_info.to_internal_data({"a":{1:[1,2]}, "b":{}})
        self.ut.assertEqual(data, {'a': {1: (1, 2)}, 'b': {}})
        expected_python_val = Map(typeinfo_string, value_type_info, {"a": {1: [1, 2]}, "b":{}})
        self.ut.assertEqual(map_type_info.to_python_value(data), expected_python_val)

    def test_MutableListTypeInfo(self) -> None:
        list_type_info = MutableListTypeInfo(typeinfo_i64)
        self.ut.assertIsInstance(list_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            list_type_info.to_internal_data(None)

        data = list_type_info.to_internal_data(_ThriftListWrapper([1, 2, 3]))
        self.ut.assertEqual(data, [1, 2, 3])

        self.assertEqual(
            (<MutableListTypeInfo>list_type_info).cpp_obj.get().get(),
            getCTypeInfo(list_type_info),
        )

        self.ut.assertTrue(list_type_info.same_as(list_type_info))
        self.ut.assertTrue(MutableListTypeInfo(typeinfo_i64).same_as(list_type_info))
        self.ut.assertFalse(MutableListTypeInfo(typeinfo_i32).same_as(list_type_info))

        with self.ut.assertRaisesRegex(
            NotImplementedError,
            "Use the 'same_as' method for comparing TypeInfoBase instances."):
            list_type_info == list_type_info

        self.ut.assertTrue(list_type_info.is_container())

    def test_MutableListTypeInfo_nested(self) -> None:
        element_type_info = MutableListTypeInfo(typeinfo_i64)
        list_type_info = MutableListTypeInfo(element_type_info)
        self.ut.assertIsInstance(list_type_info, TypeInfoBase)

        init_val = [[1, 2], [3, 4], []]
        data = list_type_info.to_internal_data(_ThriftListWrapper(init_val))
        self.ut.assertEqual(data, [[1, 2], [3, 4], []])
        self.ut.assertEqual(list_type_info.to_python_value(data), init_val)

    def test_MutableSetTypeInfo(self) -> None:
        set_type_info = MutableSetTypeInfo(typeinfo_i64)
        self.ut.assertIsInstance(set_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            set_type_info.to_internal_data(None)

        data = set_type_info.to_internal_data(_ThriftSetWrapper([1, 2, 3]))
        self.ut.assertEqual(data, {1, 2, 3})

        data = set_type_info.to_internal_data(_ThriftSetWrapper([1, 2, 3, 1, 2]))
        self.ut.assertEqual(data, {1, 2, 3})

        self.assertEqual(
            (<MutableSetTypeInfo>set_type_info).cpp_obj.get().get(),
            getCTypeInfo(set_type_info),
        )

        self.ut.assertTrue(set_type_info.same_as(set_type_info))
        self.ut.assertTrue(MutableSetTypeInfo(typeinfo_i64).same_as(set_type_info))
        self.ut.assertFalse(MutableSetTypeInfo(typeinfo_i32).same_as(set_type_info))
        self.ut.assertTrue(set_type_info.is_container())

    def test_MutableMapTypeInfo(self) -> None:
        map_type_info = MutableMapTypeInfo(typeinfo_string, typeinfo_i64)
        self.ut.assertIsInstance(map_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            map_type_info.to_internal_data(None)

        data = map_type_info.to_internal_data(_ThriftMapWrapper({"a":1, "b":2}))
        self.ut.assertEqual(data, {"a":1, "b":2})
        expected_python_val = MutableMap(typeinfo_string, typeinfo_i64, {"a": 1, "b":2})
        self.ut.assertEqual(map_type_info.to_python_value(data), expected_python_val)

        self.assertEqual(
            (<MutableMapTypeInfo>map_type_info).cpp_obj.get().get(),
            getCTypeInfo(map_type_info),
        )

        self.ut.assertTrue(map_type_info.same_as(map_type_info))
        self.ut.assertTrue(MutableMapTypeInfo(typeinfo_string, typeinfo_i64).same_as(map_type_info))
        self.ut.assertFalse(MutableMapTypeInfo(typeinfo_string, typeinfo_i32).same_as(map_type_info))
        self.ut.assertFalse(MutableMapTypeInfo(typeinfo_i32, typeinfo_i64).same_as(map_type_info))
        self.ut.assertTrue(map_type_info.is_container())

    def test_MutableStructTypeInfo(self) -> None:
        struct_type_info = MutableStructTypeInfo(FooMutable)
        self.ut.assertIsInstance(struct_type_info, TypeInfoBase)

        with self.ut.assertRaises(TypeError):
            struct_type_info.to_internal_data(None)

        data = struct_type_info.to_internal_data(FooMutable(value=11))
        self.ut.assertEqual(
                struct_type_info.to_python_value(data),
                FooMutable(value=11))

        self.assertEqual(
            &(<MutableStructTypeInfo>struct_type_info).cpp_obj,
            getCTypeInfo(struct_type_info),
        )

        self.ut.assertTrue(struct_type_info.same_as(struct_type_info))
        self.ut.assertTrue(MutableStructTypeInfo(FooMutable).same_as(struct_type_info))
        self.ut.assertFalse(MutableStructTypeInfo(OtherFooMutable).same_as(struct_type_info))
        self.ut.assertFalse(struct_type_info.is_container())
