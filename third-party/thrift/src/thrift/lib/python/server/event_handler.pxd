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

from cpython.ref cimport PyObject
from libc.stdint cimport uint16_t
from libcpp.string cimport string
from folly cimport cFollyExecutor
from folly.range cimport StringPiece

cdef extern from "thrift/lib/python/server/event_handler.h" namespace "::thrift::py3":
    cdef cppclass cfollySocketAddress "folly::SocketAddress":
        uint16_t getPort()
        bint isFamilyInet()
        bint empty()
        string getAddressStr()
        string getPath()

    cfollySocketAddress makeFromPath "folly::SocketAddress::makeFromPath"(StringPiece path)

    cdef cppclass AddressHandler:  # This isn't true but its easier for cython
        pass

    AddressHandler object_partial(void(*)(PyObject*, cfollySocketAddress), PyObject*)

    cdef cppclass Py3ServerEventHandler:
        Py3ServerEventHandler(cFollyExecutor*, AddressHandler) nogil

    cdef cppclass cIsOverloadedFunc "apache::thrift::IsOverloadedFunc":
        pass

    string getRequestId() except +


cdef extern from "thrift/lib/cpp2/server/ThriftServer.h" \
        namespace "apache::thrift":

    cdef cppclass cSSLPolicy "apache::thrift::SSLPolicy":
        bint operator==(cSSLPolicy&)

    cSSLPolicy SSLPolicy__DISABLED "apache::thrift::SSLPolicy::DISABLED"
    cSSLPolicy SSLPolicy__PERMITTED "apache::thrift::SSLPolicy::PERMITTED"
    cSSLPolicy SSLPolicy__REQUIRED "apache::thrift::SSLPolicy::REQUIRED"

    cdef cppclass cBaseThriftServerMetadata "apache::thrift::BaseThriftServer::Metadata":
        string wrapper
        string languageFramework


cdef extern from "thrift/lib/cpp2/server/TransportRoutingHandler.h" \
        namespace "apache::thrift":
    cdef cppclass cTransportRoutingHandler "apache::thrift::TransportRoutingHandler":
        pass
