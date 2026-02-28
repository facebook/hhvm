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
from libcpp.memory cimport unique_ptr

from thrift.python.types cimport cTypeInfo, Protocol


cdef extern from "<thrift/lib/python/any/serializer.h>" namespace "::apache::thrift::python":
    cdef unique_ptr[folly.iobuf.cIOBuf] cserialize_type "::apache::thrift::python::serialize_type"(const cTypeInfo& typeInfo, obj, Protocol proto) except +
    cdef object cdeserialize_type "::apache::thrift::python::deserialize_type"(const cTypeInfo& typeInfo, const folly.iobuf.cIOBuf* buf, Protocol proto) except +
