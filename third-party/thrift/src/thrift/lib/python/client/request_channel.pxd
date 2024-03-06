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
from libcpp.string cimport string
from thrift.python.serializer cimport Protocol as cProtocol


cdef extern from "thrift/lib/cpp/transport/THeader.h":
    cpdef enum ClientType "CLIENT_TYPE":
        THRIFT_HEADER_CLIENT_TYPE,
        THRIFT_ROCKET_CLIENT_TYPE,
        THRIFT_HTTP_CLIENT_TYPE,
        THRIFT_HTTP2_CLIENT_TYPE,

cdef extern from "thrift/lib/cpp2/async/RequestChannel.h" namespace "::apache::thrift":
    cdef cppclass cRequestChannel "::apache::thrift::RequestChannel":
        pass

cdef extern from "thrift/lib/python/client/RequestChannel.h" namespace "::thrift::python::client":
    cdef cppclass cRequestChannel_ptr "::thrift::python::client::RequestChannel_ptr":
        pass

    cdef cFollyFuture[cRequestChannel_ptr] createThriftChannelTCP(
        const string& host,
        const uint16_t port,
        const uint32_t connect_timeout,
        ClientType,
        cProtocol,
        const string& endpoint,
    )

    cdef cRequestChannel_ptr sync_createThriftChannelTCP(
        const string& host,
        const uint16_t port,
        const uint32_t connect_timeout,
        ClientType,
        cProtocol,
        const string& endpoint,
    )

    cdef cFollyFuture[cRequestChannel_ptr] createThriftChannelUnix(
        const string& path,
        const uint32_t connect_timeout,
        ClientType,
        cProtocol,
    )

    cdef cRequestChannel_ptr sync_createThriftChannelUnix(
        const string& path,
        const uint32_t connect_timeout,
        ClientType,
        cProtocol,
    )

cdef class RequestChannel:
    cdef cRequestChannel_ptr _cpp_obj
    @staticmethod
    cdef RequestChannel create(cRequestChannel_ptr channel)
