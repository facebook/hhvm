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

from cpython.object cimport PyTypeObject
from cpython.ref cimport PyObject
from libcpp.memory cimport unique_ptr

from thrift.python.protocol cimport Protocol

# gcc's `serializeintrin.h` header defines a macro named `_serialize`, which
# clobbers the `_serialize` method on IOBufs.  This should be fixed in more
# recent versions, but add this to avoid compile errors on older ones
# (see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100438).
cdef extern from *:
    """
    #undef _serialize
    """

cdef extern from "<thrift/lib/cpp/protocol/TType.h>" namespace "::apache::thrift::protocol":
    cdef enum cTType "::apache::thrift::protocol::TType":
        pass

cdef extern from "<thrift/lib/cpp2/protocol/TableBasedSerializer.h>" namespace "::apache::thrift::detail":
    cdef struct cTypeInfo "::apache::thrift::detail::TypeInfo":
        cTType type
    cdef struct cStructInfo "::apache::thrift::detail::StructInfo":
        pass
    cpdef enum class FieldQualifier "::apache::thrift::detail::FieldQualifier":
        Unqualified
        Optional
        Terse

cdef extern from "<thrift/lib/python/types.h>" namespace "::apache::thrift::python":
    cdef cppclass cDynamicStructInfo "::apache::thrift::python::DynamicStructInfo":
        cDynamicStructInfo(
            const char* name, int16_t numFields, bint isUnion, bint isMutable
        )
        const cStructInfo& getStructInfo()
        void addFieldInfo(
            int16_t id,
            FieldQualifier qualifier,
            const char* name,
            const cTypeInfo* typeInfo,
        ) except+
        void addFieldValue(int16_t index, object fieldValue) except+
        bint isUnion()

    cdef cppclass cListTypeInfo "::apache::thrift::python::ListTypeInfo":
        cListTypeInfo(cTypeInfo& valInfo)
        const cTypeInfo* get()

    cdef cppclass cSetTypeInfoBase "::apache::thrift::python::SetTypeInfoBase":
        const cTypeInfo* get()
        unique_ptr[cSetTypeInfoBase] asKeySorted()

    cdef cppclass cSetTypeInfo "::apache::thrift::python::SetTypeInfo"(cSetTypeInfoBase):
        cSetTypeInfo(cTypeInfo& valInfo)

    cdef cppclass cMapTypeInfo "::apache::thrift::python::MapTypeInfo":
        cMapTypeInfo(cTypeInfo& keyInfo, cTypeInfo& valInfo)
        const cTypeInfo* get()

    cdef object createImmutableStructTupleWithDefaultValues(
        const cStructInfo& structInfo
    ) except+
    cdef object createStructTupleWithNones(const cStructInfo& structInfo)
    cdef void populateImmutableStructTupleUnsetFieldsWithDefaultValues(
        object, const cStructInfo& structInfo
    ) except+
    cdef object createUnionTuple() except+
    cdef cTypeInfo createImmutableStructTypeInfo(
        const cDynamicStructInfo& structInfo
    ) except+
    cdef void setStructIsset(object, int index, bint set) except+
    cdef object getStandardImmutableDefaultValuePtrForType(
        const cTypeInfo& typeInfo
    ) except+
    cdef void tag_object_as_sequence(PyTypeObject*)
    cdef void tag_object_as_mapping(PyTypeObject*)

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

cdef class TypeInfoBase:
    cdef to_internal_data(self, object)
    cdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

cdef class TypeInfo(TypeInfoBase):
    cdef const cTypeInfo* cpp_obj
    cdef type true_pytype
    cdef tuple allowed_pytypes
    cdef str singleton_name
    @staticmethod
    cdef create(
        const cTypeInfo& cpp_obj,
        type true_pytype,
        tuple allowed_pytypes,
        str singleton_name
    )
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

cdef class IntegerTypeInfo(TypeInfoBase):
    cdef const cTypeInfo* cpp_obj
    cdef int64_t min_value
    cdef int64_t max_value
    cdef str singleton_name
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, min_value, max_value, str singleton_name)
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

cdef class StringTypeInfo(TypeInfoBase):
    cdef const cTypeInfo* cpp_obj
    cdef str singleton_name
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, str singleton_name)
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

cdef class IOBufTypeInfo(TypeInfoBase):
    cdef const cTypeInfo* cpp_obj
    cdef str singleton_name
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, str singleton_name)
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

# TODO(ffrancet): Refactor this to a c class and update all of its accesses to c
# functions
cdef class FieldInfo:
    cdef int id
    cdef FieldQualifier qualifier
    cdef str name
    cdef str py_name
    cdef object type_info
    cdef object default_value
    cdef object adapter_info
    cdef bint is_primitive
    cdef int idl_type

cdef class StructInfo:
    cdef unique_ptr[cDynamicStructInfo] cpp_obj
    cdef tuple type_infos
    cdef tuple[FieldInfo] fields
    cdef dict name_to_index
    cdef void _fill_struct_info(self) except *
    cdef void _initialize_default_values(self) except *

cdef class UnionInfo:
    cdef unique_ptr[cDynamicStructInfo] cpp_obj
    cdef dict type_infos
    cdef dict id_to_adapter_info
    cdef tuple fields
    cdef dict name_to_index
    cdef void _fill_union_info(self) except *

cdef class ListTypeInfo(TypeInfoBase):
    cdef object val_info
    cdef unique_ptr[cListTypeInfo] cpp_obj
    cdef const cTypeInfo* get_cTypeInfo(self)
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef to_internal_from_values(self, object values, TypeInfoBase val_type_info)
    cdef to_python_from_values(self, object values, TypeInfoBase val_type_info)

cdef class SetTypeInfo(TypeInfoBase):
    cdef object val_info
    cdef unique_ptr[cSetTypeInfoBase] cpp_obj
    cdef const cTypeInfo* get_cTypeInfo(self)
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef to_internal_from_values(self, object values, TypeInfoBase val_type_info)
    cdef to_python_from_values(self, object values, TypeInfoBase val_type_info)

cdef class MapTypeInfo(TypeInfoBase):
    cdef object key_info
    cdef object val_info
    cdef unique_ptr[cMapTypeInfo] cpp_obj
    cdef const cTypeInfo* get_cTypeInfo(self)
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef to_internal_from_values(self, object)
    cdef to_python_from_values(self, object)

cdef class StructTypeInfo(TypeInfoBase):
    cdef cTypeInfo cpp_obj
    cdef object _class
    cdef const cTypeInfo* get_cTypeInfo(self)
    cdef bint is_union
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)

cdef class EnumTypeInfo(TypeInfoBase):
    cdef object _class
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

cdef class AdaptedTypeInfo(TypeInfoBase):
    cdef object _orig_type_info
    cdef object _adapter_class
    cdef object _transitive_annotation_factory
    cpdef to_internal_data(self, object)
    cpdef to_python_value(self, object)
    cdef const cTypeInfo* get_cTypeInfo(self)

cdef class StructOrUnion:
    cdef tuple _fbthrift_data
    cdef folly.iobuf.IOBuf _serialize(StructOrUnion self, Protocol proto)
    cdef uint32_t _deserialize(
        StructOrUnion self, folly.iobuf.IOBuf buf, Protocol proto
    ) except? 0
    cdef _fbthrift_get_cached_field_value(self, int16_t index)

cdef api object _get_fbthrift_data(object struct_or_union)
cdef api object _get_exception_fbthrift_data(object generated_error)

cdef class Struct(StructOrUnion):
    cdef tuple _fbthrift_field_cache
    cdef folly.iobuf.IOBuf _serialize(Struct self, Protocol proto)
    cdef uint32_t _deserialize(
        Struct self, folly.iobuf.IOBuf buf, Protocol proto
    ) except? 0
    cdef _fbthrift_py_value_from_internal_data(self, int16_t index)
    cdef _fbthrift_get_cached_field_value(Struct self, int16_t index)
    cdef _fbthrift_fully_populate_cache(Struct self)
    cdef _initStructTupleWithValues(Struct self, object kwargs) except *

cdef Struct _fbthrift_struct_update_nested_field(Struct obj, list path_and_vals)

cdef tuple _validate_union_init_kwargs(
    object union_class, object fields_enum_type, dict kwargs
)

cdef class Union(StructOrUnion):
    cdef object py_type
    cdef readonly object value
    cdef void _fbthrift_update_current_field_attributes(self) except *
    cdef object _fbthrift_to_internal_data(self, type_value, value)
    cdef void _fbthrift_set_union_value(self, field_id, value) except *
    cdef folly.iobuf.IOBuf _serialize(Union self, Protocol proto)
    cdef uint32_t _deserialize(
        Union self, folly.iobuf.IOBuf buf, Protocol proto
    ) except? 0
    cdef object _fbthrift_py_type_enum(self)

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
cdef list_eq(object self, object other)
cdef list_lt(object first, object second)
cdef _fbthrift_compare_struct_less(object lhs, object rhs, object return_if_same_value)
