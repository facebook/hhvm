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

cimport folly.iobuf
from libc.stdint cimport uint32_t
from libcpp.memory cimport unique_ptr

from thrift.python.types cimport cDynamicStructInfo


cdef extern from "<thrift/lib/cpp/protocol/TProtocolTypes.h>" namespace "apache::thrift::protocol":
    cpdef enum Protocol "apache::thrift::protocol::PROTOCOL_TYPES":
        BINARY "apache::thrift::protocol::T_BINARY_PROTOCOL"
        COMPACT "apache::thrift::protocol::T_COMPACT_PROTOCOL"
        JSON "apache::thrift::protocol::T_SIMPLE_JSON_PROTOCOL"


cdef extern from "<thrift/lib/python/Serializer.h>" namespace "::apache::thrift::python":
    cdef unique_ptr[folly.iobuf.cIOBuf] cserialize "::apache::thrift::python::serialize"(const cDynamicStructInfo& structInfo, obj, Protocol proto) except +
    cdef uint32_t cdeserialize "::apache::thrift::python::deserialize"(const cDynamicStructInfo& structInfo, const folly.iobuf.cIOBuf* buf, obj, Protocol proto) except +
