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
from libcpp cimport bool as cbool
from libcpp.memory cimport unique_ptr

from thrift.python.types cimport cDynamicStructInfo
from thrift.python.protocol cimport Protocol as cProtocol


cdef extern from "<thrift/lib/python/Serializer.h>" namespace "apache::thrift::json5::detail":
    cdef cppclass cJsonWriterOptions "apache::thrift::json5::detail::JsonWriterOptions":
        cbool listTrailingComma
        cbool objectTrailingComma
        cbool unquoteObjectName
        cbool allowNanInf
        size_t indentWidth


cdef extern from "<thrift/lib/python/Serializer.h>" namespace "apache::thrift::json5::detail::Json5ProtocolWriter":
    cdef cppclass cJson5ProtocolWriterOptions "apache::thrift::json5::detail::Json5ProtocolWriter::Options":
        cJsonWriterOptions writer
        cbool enumAsInteger
        cbool binaryAsBase64String
        cbool mapPrimitiveKeysAsMemberNames


cdef extern from "<thrift/lib/python/Serializer.h>" namespace "::apache::thrift::python":
    cdef unique_ptr[folly.iobuf.cIOBuf] cserialize "::apache::thrift::python::serialize"(const cDynamicStructInfo& structInfo, obj, cProtocol proto) except +
    cdef uint32_t cdeserialize "::apache::thrift::python::deserialize"(const cDynamicStructInfo& structInfo, const folly.iobuf.cIOBuf* buf, obj, cProtocol proto) except +
    cdef unique_ptr[folly.iobuf.cIOBuf] cserializeJson5 "::apache::thrift::python::serializeJson5"(const cDynamicStructInfo& structInfo, obj, const cJson5ProtocolWriterOptions& options) except +
