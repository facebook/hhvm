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
from folly.iobuf cimport IOBuf

from thrift.python.exceptions cimport Error
from thrift.python.protocol cimport Protocol

from thrift.python.mutable_types cimport MutableStruct

cdef extern from "<thrift/lib/cpp2/protocol/TableBasedSerializer.h>" namespace "::apache::thrift::detail":
    cdef struct cStructInfo "::apache::thrift::detail::StructInfo":
        pass

cdef extern from "<thrift/lib/python/types.h>" namespace "::apache::thrift::python":
    cdef object createMutableStructListWithDefaultValues(const cStructInfo& structInfo) except+
    cdef object createStructListWithNones(const cStructInfo& structInfo)
    cdef void populateMutableStructListUnsetFieldsWithDefaultValues(object, const cStructInfo& structInfo) except+
    cdef void resetFieldToStandardDefault(object, const cStructInfo& structInfo, int index) except+


# Base class for all generated (mutable) exceptions defined in Thrift IDL
cdef class MutableGeneratedError(Error):
    cdef list _fbthrift_data
    cdef IOBuf _fbthrift_serialize(MutableGeneratedError self, Protocol proto)
    cdef uint32_t _fbthrift_deserialize(MutableGeneratedError self, IOBuf buf, Protocol proto) except? 0
    cdef _fbthrift_get_field_value(MutableGeneratedError self, int16_t index)
    cdef _initStructListWithValues(MutableGeneratedError self, object kwargs) except *
    cdef _fbthrift_set_field_value(self, int16_t index, object value) except *
    cdef _fbthrift_reset_field_to_standard_default(self, int16_t index) except *
    cdef _fbthrift_get_cached_field_value(MutableGeneratedError self, int16_t index) except *
