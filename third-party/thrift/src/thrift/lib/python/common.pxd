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

# distutils: language=c++

from libcpp.memory cimport shared_ptr
from libcpp.utility cimport pair
from libc.stdint cimport int32_t, int64_t
from libcpp.string cimport string
from thrift.python.std_libcpp cimport milliseconds
from libcpp cimport bool as cpp_bool


cdef extern from "thrift/lib/cpp/concurrency/Thread.h":

    enum cPriority "apache::thrift::concurrency::PRIORITY":
        cHIGH_IMPORTANT "apache::thrift::concurrency::HIGH_IMPORTANT"
        cHIGH "apache::thrift::concurrency::HIGH"
        cIMPORTANT "apache::thrift::concurrency::IMPORTANT"
        cNORMAL "apache::thrift::concurrency::NORMAL"
        cBEST_EFFORT "apache::thrift::concurrency::BEST_EFFORT"
        cN_PRIORITIES "apache::thrift::concurrency::N_PRIORITIES"


cdef inline cPriority Priority_to_cpp(object value):
    cdef int cvalue = value.value
    return <cPriority>cvalue

cdef extern from "thrift/lib/cpp2/FieldRef.h" namespace "apache::thrift":
    cdef cppclass cFieldRef "apache::thrift::field_ref"[T]:
        T& operator*()

cdef extern from "folly/container/F14Map.h" namespace "folly":
  cdef cppclass F14NodeMap[K, T]:
    cppclass const_iterator:
        pair[K, T]& operator*()
        const_iterator operator++()
        const_iterator operator--()
        cpp_bool operator==(const_iterator)
        cpp_bool operator!=(const_iterator)
    int64_t size()
    const_iterator begin()
    const_iterator end()
    const_iterator find(const K&)
    int count(const K&)

cdef class Headers:
    cdef object __weakref__
    cdef const F14NodeMap[string, string]* _getMap(self)


cdef extern from "thrift/lib/cpp2/async/RpcOptions.h" namespace "apache::thrift":
    cdef cppclass cRpcOptions "apache::thrift::RpcOptions":
        cRpcOptions()
        cRpcOptions& setTimeout(milliseconds timeout)
        milliseconds getTimeout()
        cRpcOptions& setPriority(cPriority priority)
        cPriority getPriority()
        cRpcOptions& setChunkTimeout(milliseconds timeout)
        milliseconds getChunkTimeout()
        cRpcOptions& setQueueTimeout(milliseconds timeout)
        milliseconds getQueueTimeout()
        cRpcOptions& setChunkBufferSize(int32_t chunkBufferSize)
        int32_t getChunkBufferSize()
        cRpcOptions& setMemoryBufferSize(size_t targetSize, int32_t initialChunks, int32_t maxChunks)
        void setWriteHeader(const string& key, const string value)
        void setReadHeaders(F14NodeMap[string, string]&& map)
        const F14NodeMap[string, string]& getReadHeaders()
        const F14NodeMap[string, string]& getWriteHeaders()


cdef extern from "thrift/lib/cpp2/gen/module_metadata_h.h" namespace "::apache::thrift::metadata":
    cdef cppclass cThriftServiceContextRef "::apache::thrift::metadata::ThriftServiceContextRef":
        cThriftServiceContextRef()
    cdef cppclass cThriftMetadata "::apache::thrift::metadata::ThriftMetadata":
        cThriftMetadata()
    cdef cppclass cThriftServiceMetadataResponse "::apache::thrift::metadata::ThriftServiceMetadataResponse":
        cThriftServiceMetadataResponse()
        cFieldRef[cThriftServiceContextRef] services()
        cFieldRef[cThriftMetadata] metadata()
    cdef cppclass ServiceMetadata "::apache::thrift::detail::md::ServiceMetadata"[T]:
        @staticmethod
        const void gen(cThriftServiceMetadataResponse& context)


cdef class RpcOptions:
    cdef object __weakref__
    cdef object _readheaders
    cdef object _writeheaders
    cdef cRpcOptions _cpp_obj


cdef class ReadHeaders(Headers):
    cdef RpcOptions _parent
    @staticmethod
    cdef _fbthrift_create(RpcOptions rpc_options)


cdef class WriteHeaders(Headers):
    cdef RpcOptions _parent
    @staticmethod
    cdef _fbthrift_create(RpcOptions rpc_options)


cdef class MetadataBox:
    cdef shared_ptr[cThriftMetadata] _cpp_obj
    @staticmethod
    cdef box(cThriftMetadata&& meta)
