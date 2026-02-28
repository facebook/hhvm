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

from libc.stdint cimport int16_t
from libcpp.memory cimport unique_ptr
from thrift.python.types cimport cMapTypeInfoBase, cSetTypeInfoBase 

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

    cdef cppclass cMutableListTypeInfo "::apache::thrift::python::MutableListTypeInfo":
        cMutableListTypeInfo(cTypeInfo& valInfo)
        const cTypeInfo* get()

    cdef cppclass cMutableSetTypeInfo "::apache::thrift::python::MutableSetTypeInfo"(cSetTypeInfoBase):
        cMutableSetTypeInfo(cTypeInfo& valInfo)

    cdef cppclass cMutableMapTypeInfo "::apache::thrift::python::MutableMapTypeInfo"(cMapTypeInfoBase):
        cMutableMapTypeInfo(cTypeInfo& keyInfo, cTypeInfo& valInfo)

cdef extern from "<thrift/lib/python/types.h>" namespace "::apache::thrift::python":
    cdef cTypeInfo createMutableStructTypeInfo(
        const cDynamicStructInfo& structInfo
    ) except+

from thrift.python.types cimport TypeInfoBase

cdef class MutableStructTypeInfo(TypeInfoBase):
    cdef cTypeInfo cpp_obj
    cdef object _mutable_struct_class
    cdef const cTypeInfo* get_cTypeInfo(self)
    cdef to_internal_data(self, object)
    cdef to_python_value(self, object)

cdef class MutableListTypeInfo(TypeInfoBase):
    cdef object val_info
    cdef bint value_type_is_container
    cdef unique_ptr[cMutableListTypeInfo] cpp_obj
    cdef const cTypeInfo* get_cTypeInfo(self)
    cdef to_internal_data(self, object)
    cdef to_python_value(self, object)
    cdef from_python_values(self, object)

cdef class MutableSetTypeInfo(TypeInfoBase):
    cdef object val_info
    cdef bint value_type_is_container
    cdef unique_ptr[cSetTypeInfoBase] cpp_obj
    cdef const cTypeInfo* get_cTypeInfo(self)
    cdef to_internal_data(self, object)
    cdef to_python_value(self, object)
    cdef from_python_values(self, object)

cdef class MutableMapTypeInfo(TypeInfoBase):
    cdef object key_info
    cdef object val_info
    cdef bint key_type_is_container
    cdef bint value_type_is_container
    cdef unique_ptr[cMapTypeInfoBase] cpp_obj
    cdef const cTypeInfo* get_cTypeInfo(self)
    cdef to_internal_data(self, object)
    cdef to_python_value(self, object)
    cdef from_python_values(self, object)
