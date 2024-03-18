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

from libc.stdint cimport uint32_t, int16_t, int64_t
cimport folly.iobuf

from cpython.ref cimport PyObject
from libcpp.memory cimport unique_ptr

from thrift.python.serializer cimport Protocol


cdef extern from "<thrift/lib/cpp2/protocol/TableBasedSerializer.h>" namespace "::apache::thrift::detail":
    cdef struct cTypeInfo "::apache::thrift::detail::TypeInfo":
        pass
    cdef struct cStructInfo "::apache::thrift::detail::StructInfo":
        pass
    cpdef enum class FieldQualifier "::apache::thrift::detail::FieldQualifier":
        Unqualified
        Optional
        Terse

cdef extern from "<thrift/lib/python/types.h>" namespace "::apache::thrift::python":
    cdef cppclass cDynamicStructInfo "::apache::thrift::python::DynamicStructInfo":
        cDynamicStructInfo(const char* name, int16_t numFields, bint isUnion)
        const cStructInfo& getStructInfo()
        void addFieldInfo(int16_t id, FieldQualifier qualifier, const char* name, const cTypeInfo* typeInfo) except+
        void addFieldValue(int16_t index, object fieldValue) except+
        bint isUnion()

    cdef cppclass cListTypeInfo "::apache::thrift::python::ListTypeInfo":
        cListTypeInfo(cTypeInfo& valInfo)
        const cTypeInfo* get()

    cdef cppclass cSetTypeInfo "::apache::thrift::python::SetTypeInfo":
        cSetTypeInfo(cTypeInfo& valInfo)
        const cTypeInfo* get()

    cdef cppclass cMapTypeInfo "::apache::thrift::python::MapTypeInfo":
        cMapTypeInfo(cTypeInfo& keyInfo, cTypeInfo& valInfo)
        const cTypeInfo* get()

    cdef object createStructTupleWithDefaultValues(const cStructInfo& structInfo) except+
    cdef object createStructTupleWithNones(const cStructInfo& structInfo)
    cdef void populateStructTupleUnsetFieldsWithDefaultValues(object, const cStructInfo& structInfo) except+ 
    cdef object createUnionTuple() except+
    cdef cTypeInfo createStructTypeInfo(const cDynamicStructInfo& structInfo) except+
    cdef void setStructIsset(object, int index, bint set) except+

    cdef const cTypeInfo& boolTypeInfo
    cdef const cTypeInfo& byteTypeInfo
    cdef const cTypeInfo& i16TypeInfo
    cdef const cTypeInfo& i32TypeInfo
    cdef const cTypeInfo& i64TypeInfo
    cdef const cTypeInfo& floatTypeInfo
    cdef const cTypeInfo& doubleTypeInfo
    cdef const cTypeInfo  stringTypeInfo
    cdef const cTypeInfo  binaryTypeInfo
    cdef const cTypeInfo  iobufTypeInfo


cdef extern from "<Python.h>":
    cdef const char * PyUnicode_AsUTF8(object unicode)


cdef class TypeInfo:
    cdef const cTypeInfo* cpp_obj
    cdef tuple pytypes
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, pytypes)

cdef class IntegerTypeInfo:
    cdef const cTypeInfo* cpp_obj
    cdef int64_t min_value
    cdef int64_t max_value
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, min_value, max_value)

cdef class StringTypeInfo:
    cdef const cTypeInfo* cpp_obj
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj)

cdef class IOBufTypeInfo:
    cdef const cTypeInfo* cpp_obj
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj)

cdef class StructInfo:
    cdef unique_ptr[cDynamicStructInfo] cpp_obj
    cdef tuple type_infos
    cdef tuple fields
    cdef dict name_to_index
    cdef void fill(self) except *
    cdef void store_field_values(self) except *

cdef class UnionInfo:
    cdef unique_ptr[cDynamicStructInfo] cpp_obj
    cdef dict type_infos
    cdef dict id_to_adapter_info
    cdef tuple fields
    cdef dict name_to_index
    cdef void fill(self) except *

cdef class ListTypeInfo:
    cdef object val_info
    cdef unique_ptr[cListTypeInfo] cpp_obj
    cdef const cTypeInfo* get(self)

cdef class SetTypeInfo:
    cdef object val_info
    cdef unique_ptr[cSetTypeInfo] cpp_obj
    cdef const cTypeInfo* get(self)

cdef class MapTypeInfo:
    cdef object key_info
    cdef object val_info
    cdef unique_ptr[cMapTypeInfo] cpp_obj
    cdef const cTypeInfo* get(self)

cdef class StructTypeInfo:
    cdef cTypeInfo cpp_obj
    cdef object _class
    cdef const cTypeInfo* get(self)
    cdef bint is_union

cdef class EnumTypeInfo:
    cdef object _class

cdef class AdaptedTypeInfo:
    cdef object _orig_type_info
    cdef object _adapter_info
    cdef object _transitive_annotation

cdef class StructOrUnion:
    cdef tuple _fbthrift_data
    cdef folly.iobuf.IOBuf _serialize(StructOrUnion self, Protocol proto)
    cdef uint32_t _deserialize(StructOrUnion self, folly.iobuf.IOBuf buf, Protocol proto) except? 0
    cdef _fbthrift_get_field_value(self, int16_t index)

cdef api object _get_fbthrift_data(object struct_or_union)
cdef api object _get_exception_fbthrift_data(object generated_error)

cdef class Struct(StructOrUnion):
    cdef tuple _fbthrift_field_cache
    cdef folly.iobuf.IOBuf _serialize(Struct self, Protocol proto)
    cdef uint32_t _deserialize(Struct self, folly.iobuf.IOBuf buf, Protocol proto) except? 0
    cdef _fbthrift_populate_primitive_fields(Struct self)
    cdef _fbthrift_fully_populate_cache(Struct self)
    cdef _initStructTupleWithValues(Struct self, object kwargs) except *

cdef class Union(StructOrUnion):
    cdef readonly object type
    cdef readonly object value
    cdef void _fbthrift_load_cache(self) except *
    cdef object _fbthrift_to_internal_data(self, type_value, value)
    cdef void _fbthrift_update_type_value(self, type_value, value) except *
    cdef folly.iobuf.IOBuf _serialize(Union self, Protocol proto)
    cdef uint32_t _deserialize(Union self, folly.iobuf.IOBuf buf, Protocol proto) except? 0

cdef class BadEnum:
    cdef object _enum
    cdef readonly int value
    cdef readonly str name

cdef class Container:
    cdef object _fbthrift_val_info
    cdef object _fbthrift_elements

cdef class List(Container):
    pass

cdef class Set(Container):
    pass

cdef class Map(Container):
    cdef object _fbthrift_key_info

cdef void set_struct_field(tuple struct_tuple, int16_t index, value) except *

cdef class ServiceInterface:
    pass

cdef const cTypeInfo* getCTypeInfo(type_info)
