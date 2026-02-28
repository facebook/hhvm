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

from folly cimport cFollyFuture
from libc.stdint cimport uint16_t, uint32_t
from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from thrift.python.client.ssl cimport cSSLContext
from thrift.python.protocol cimport Protocol as cProtocol

cdef extern from "thrift/lib/cpp/transport/THeader.h":
    cpdef enum ClientType "CLIENT_TYPE":
        THRIFT_HEADER_CLIENT_TYPE,
        THRIFT_ROCKET_CLIENT_TYPE,
        THRIFT_FRAMED_DEPRECATED,
        THRIFT_UNFRAMED_DEPRECATED,
        THRIFT_HTTP_SERVER_TYPE,
        THRIFT_HTTP_CLIENT_TYPE,
        THRIFT_HTTP2_CLIENT_TYPE,
        THRIFT_FRAMED_COMPACT,
        THRIFT_HTTP_GET_CLIENT_TYPE,
        THRIFT_UNKNOWN_CLIENT_TYPE,
        THRIFT_UNFRAMED_COMPACT_DEPRECATED

cdef extern from "thrift/lib/cpp2/async/RequestChannel.h" namespace "::apache::thrift":
    cdef cppclass cRequestChannel "::apache::thrift::RequestChannel":
        pass

    cdef cppclass cRequestChannel_ptr "::apache::thrift::RequestChannel::Ptr":
        pass

cdef extern from "thrift/lib/cpp/TProcessorEventHandler.h" namespace "::apache::thrift":
    cdef cppclass cTProcessorEventHandler "apache::thrift::TProcessorEventHandler":
        pass

cdef extern from "thrift/lib/cpp/EventHandlerBase.h" namespace "::apache::thrift":
    cdef cppclass cTProcessorBase "apache::thrift::TProcessorBase":
        @staticmethod
        void addProcessorEventHandler_deprecated(shared_ptr[cTProcessorEventHandler] handler)
        @staticmethod
        void removeProcessorEventHandler(shared_ptr[cTProcessorEventHandler] handler)

cdef extern from "thrift/lib/python/client/RequestChannel.h" namespace "::apache::thrift::python::client":
    cdef cppclass ChannelFactory "::apache::thrift::python::client::ChannelFactory":
        cFollyFuture[cRequestChannel_ptr] createThriftChannelTCP(
            const string& host,
            const uint16_t port,
            const uint32_t connect_timeout,
            ClientType,
            cProtocol,
            const string& endpoint,
        )

        cRequestChannel_ptr sync_createThriftChannelTCP(
            const string& host,
            const uint16_t port,
            const uint32_t connect_timeout,
            ClientType,
            cProtocol,
            const string& endpoint,
        )

        cFollyFuture[cRequestChannel_ptr] createThriftChannelUnix(
            const string& path,
            const uint32_t connect_timeout,
            ClientType,
            cProtocol,
        )

        cRequestChannel_ptr sync_createThriftChannelUnix(
            const string& path,
            const uint32_t connect_timeout,
            ClientType,
            cProtocol,
        )

        cFollyFuture[cRequestChannel_ptr] createThriftChannelSSL(
            shared_ptr[cSSLContext]& ctx,
            const string& host,
            const uint16_t port,
            const uint32_t connect_timeout,
            const uint32_t ssl_timeout,
            ClientType,
            cProtocol,
            const string& endpoint,
        )

        cRequestChannel_ptr sync_createThriftChannelSSL(
            shared_ptr[cSSLContext]& ctx,
            const string& host,
            const uint16_t port,
            const uint32_t connect_timeout,
            const uint32_t ssl_timeout,
            ClientType,
            cProtocol,
            const string& endpoint,
        ) except +

    cdef cppclass DefaultChannelFactory "::apache::thrift::python::client::DefaultChannelFactory" (ChannelFactory):
        DefaultChannelFactory()

cdef class RequestChannel:
    cdef cRequestChannel_ptr _cpp_obj
    @staticmethod
    cdef RequestChannel create(cRequestChannel_ptr channel)
