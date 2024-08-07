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

from thrift.python.mutable_types cimport cDynamicStructInfo
from thrift.python.protocol cimport Protocol as cProtocol


cdef extern from "<thrift/lib/python/Serializer.h>" namespace "::apache::thrift::python":
    cdef unique_ptr[folly.iobuf.cIOBuf] cserialize "::apache::thrift::python::mutable_serialize"(const cDynamicStructInfo& structInfo, obj, cProtocol proto) except +
    cdef uint32_t cdeserialize "::apache::thrift::python::mutable_deserialize"(const cDynamicStructInfo& structInfo, const folly.iobuf.cIOBuf* buf, obj, cProtocol proto) except +
