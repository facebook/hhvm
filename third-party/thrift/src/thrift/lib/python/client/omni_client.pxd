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

from libcpp cimport bool
from folly cimport cFollyExceptionWrapper, cFollyExecutor, cFollySemiFuture, cFollyFuture
from folly.expected cimport cExpected
from folly.iobuf cimport cIOBuf
from libc.stdint cimport uint16_t
from libcpp.memory cimport unique_ptr, shared_ptr
from libcpp.string cimport string
from libcpp.unordered_map cimport unordered_map
from libcpp.pair cimport pair
from thrift.python.client.request_channel cimport cRequestChannel_ptr, cRequestChannel, cTProcessorEventHandler
from thrift.python.common cimport cRpcOptions
from thrift.python.streaming.sink cimport cIOBufClientSink


cdef extern from "thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h" namespace "::apache::thrift":
    cpdef enum class RpcKind:
        SINGLE_REQUEST_SINGLE_RESPONSE = 0
        SINGLE_REQUEST_NO_RESPONSE = 1
        SINGLE_REQUEST_STREAMING_RESPONSE = 4
        SINK = 6

cdef extern from "thrift/lib/cpp2/util/MethodMetadata.h" namespace "::apache::thrift":
    cpdef enum class FunctionQualifier:
        Unspecified = 0
        OneWay = 1
        Idempotent = 2
        ReadOnly = 3

    cpdef enum class InteractionMethodPosition:
        None = 0
        Factory = 1
        Member = 2

    cdef cppclass cData "::apache::thrift::MethodMetadata::Data" nogil:
        cData(string name, FunctionQualifier qualifier, string uriOrName, InteractionMethodPosition position, string interaction)

cdef extern from "folly/container/F14Map.h" namespace "folly":
  cdef cppclass F14NodeMap[K, T]:
    cppclass iterator:
        bool operator!=(iterator)
        bool operator==(iterator)
        iterator operator++()
        iterator operator--()
        pair[K, T]& operator*()
    F14NodeMap()
    iterator begin()
    iterator end()
    T& operator[](const K&)
    bool empty()

cdef extern from "thrift/lib/python/client/OmniClient.h" namespace "::apache::thrift::python::client":
    cdef cppclass cIOBufClientBufferedStream "::apache::thrift::python::client::IOBufClientBufferedStream":
        pass

    cdef cppclass cOmniClientResponseWithHeaders "::apache::thrift::python::client::OmniClientResponseWithHeaders":
        cExpected[unique_ptr[cIOBuf], cFollyExceptionWrapper] buf
        F14NodeMap[string, string] headers
        unique_ptr[cIOBufClientBufferedStream] stream
        unique_ptr[cIOBufClientSink] sink

    cdef cppclass cOmniClient "::apache::thrift::python::client::OmniClient" nogil:
        cOmniClient(cRequestChannel_ptr channel, const string& serviceName)
        cOmniClient(shared_ptr[cRequestChannel] channel, const string& serviceName)
        cOmniClientResponseWithHeaders sync_send(
            const string& serviceName,
            const string& methodName,
            unique_ptr[cIOBuf] args,
            cData&& metadata,
            const unordered_map[string, string] headers,
            cRpcOptions&& options,
        ) except+
        void oneway_send(
            const string& serviceName,
            const string& methodName,
            unique_ptr[cIOBuf] args,
            cData&& metadata,
            const unordered_map[string, string] headers,
            cRpcOptions&& options,
        ) except+
        cFollySemiFuture[cOmniClientResponseWithHeaders] semifuture_send(
            const string& serviceName,
            const string& methodName,
            unique_ptr[cIOBuf] args,
            cData&& metadata,
            const unordered_map[string, string] headers,
            cRpcOptions&& options,
            cFollyExecutor* executor,
            const RpcKind rpcKind,
        )
        shared_ptr[cRequestChannel] getChannelShared()
        uint16_t getChannelProtocolId()
        void clearEventHandlers()
        void addEventHandler(const shared_ptr[cTProcessorEventHandler]& handler)

        void set_interaction_factory(cOmniClient *client)

    cdef cppclass cOmniInteractionClient "::apache::thrift::python::client::OmniInteractionClient"(cOmniClient) nogil:
        cOmniInteractionClient(shared_ptr[cRequestChannel] channel, const string& methodName)

    cdef cFollyFuture[unique_ptr[cOmniInteractionClient]]& createOmniInteractionClient(shared_ptr[cRequestChannel] channel, const string& methodName)
