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


from libc.stdint cimport uint32_t, int16_t
from libcpp.memory cimport unique_ptr
from folly.iobuf cimport IOBuf

from thrift.python.protocol cimport Protocol
from thrift.python.types cimport FieldInfo

cdef extern from "<thrift/lib/cpp2/protocol/TableBasedSerializer.h>" namespace "::apache::thrift::detail":
    cdef struct cTypeInfo "::apache::thrift::detail::TypeInfo":
        pass
    cdef struct cStructInfo "::apache::thrift::detail::StructInfo":
        pass
    cpdef enum class FieldQualifier "::apache::thrift::detail::FieldQualifier":
        Unqualified
        Optional
        Terse

cdef extern from "<thrift/lib/cpp2/type/BaseType.h>" namespace "::apache::thrift::type":
    cdef enum class ThriftIdlType "::apache::thrift::type::BaseType":
        Void
        Bool
        Byte
        I16
        I32
        I64
        Float
        Double
        String
        Binary
        Enum
        Struct
        Union
        Exception
        List
        Set
        Map

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
        void addMutableFieldInfo(
            int16_t id,
            FieldQualifier qualifier,
            const char* name,
            const cTypeInfo* typeInfo
        ) except+
        void addFieldValue(int16_t index, object fieldValue) except+
        bint isUnion()

    cdef object createMutableStructListWithDefaultValues(
        const cStructInfo& structInfo
    ) except+
    cdef object createStructListWithNones(const cStructInfo& structInfo)
    cdef void populateMutableStructListUnsetFieldsWithDefaultValues(
        object, const cStructInfo& structInfo
    ) except+
    cdef void resetFieldToStandardDefault(
        object, const cStructInfo& structInfo, int index
    ) except+
    cdef void setMutableStructIsset(object, int index, bint set) except+

    cdef object createUnionTuple() except+

cdef class MutableStructOrUnion:
    cdef object _fbthrift_data
    cdef IOBuf _fbthrift_serialize(MutableStructOrUnion self, Protocol proto)
    cdef uint32_t _fbthrift_deserialize(
        MutableStructOrUnion self, IOBuf buf, Protocol proto
    ) except? 0
    cdef _fbthrift_get_field_value(self, int16_t index)

cdef class MutableStruct(MutableStructOrUnion):
    cdef IOBuf _fbthrift_serialize(MutableStruct self, Protocol proto)
    cdef uint32_t _fbthrift_deserialize(
        MutableStruct self, IOBuf buf, Protocol proto
    ) except? 0
    cdef _fbthrift_get_field_value(MutableStruct self, int16_t index)
    cdef _initStructListWithValues(MutableStruct self, object kwargs) except *
    cdef _fbthrift_set_field_value(self, int16_t index, object value) except *
    cdef _fbthrift_reset_field_to_standard_default(self, int16_t index) except *
    cdef _fbthrift_get_cached_field_value(MutableStruct self, int16_t index) except *

cdef class MutableStructInfo:
    cdef unique_ptr[cDynamicStructInfo] cpp_obj
    cdef tuple type_infos
    cdef tuple fields
    cdef dict name_to_index
    cdef void fill(self) except *
    cdef void _initialize_default_values(self) except *

cdef class MutableUnionInfo:
    cdef unique_ptr[cDynamicStructInfo] cpp_obj
    cdef dict type_infos
    cdef dict id_to_adapter_info
    cdef tuple fields
    cdef dict name_to_index
    cdef void _fill_mutable_union_info(self) except *

cdef class _MutableUnionFieldDescriptor:
    cdef FieldInfo _field_info

cdef class MutableUnion(MutableStructOrUnion):
    cdef readonly object fbthrift_current_field
    cdef readonly object fbthrift_current_value
    cdef void _fbthrift_update_current_field_attributes(self) except *

    cdef object _fbthrift_get_current_field_python_value(
        self, int current_field_enum_value
    )

    cdef void _fbthrift_set_mutable_union_value(
        self, int field_id, object field_python_value
    ) except *

    cdef object _fbthrift_convert_field_python_value_to_internal_data(
            self, int field_id, object field_python_value)

    cdef object _fbthrift_get_field_value(self, int16_t field_id)

cdef object _mutable_struct_meta_new(
    object cls, object cls_name, object bases, object dct
)
cdef void set_mutable_struct_field(list struct_list, int16_t index, value) except *
