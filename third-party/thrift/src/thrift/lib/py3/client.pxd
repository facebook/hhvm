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

# distutils: language = c++

from folly cimport (
    cFollyExceptionWrapper,
    cFollyFuture,
    cFollyExecutor,
    cFollyTry,
    cFollyUnit,
)
from libc.stdint cimport uint16_t, uint32_t, int64_t
from libcpp.memory cimport unique_ptr, shared_ptr
from libcpp.string cimport string
from libcpp.utility cimport move
from libcpp.typeinfo cimport type_info
from libcpp.map cimport map
from libcpp.vector cimport vector
from cpython.ref cimport PyObject
from libcpp cimport bool

# This is just here to make the cython compile happy.
from asyncio import InvalidStateError as asyncio_InvalidStateError
from thrift.py3.common cimport Protocol as cProtocol, cThriftMetadata
from folly.executor cimport AsyncioExecutor

cdef extern from "thrift/lib/cpp/transport/THeader.h":
    cpdef enum ClientType "CLIENT_TYPE":
        THRIFT_HEADER_CLIENT_TYPE,
        THRIFT_FRAMED_DEPRECATED,
        THRIFT_UNFRAMED_DEPRECATED,
        THRIFT_HTTP_SERVER_TYPE,
        THRIFT_HTTP_CLIENT_TYPE,
        THRIFT_ROCKET_CLIENT_TYPE,
        THRIFT_FRAMED_COMPACT,
        THRIFT_HTTP_GET_CLIENT_TYPE,
        THRIFT_UNKNOWN_CLIENT_TYPE,
        THRIFT_UNFRAMED_COMPACT_DEPRECATED

cdef extern from "thrift/lib/py3/client.h" namespace "::thrift::py3":
    # The custome deleter is hard, so instead make cython treat it as class
    cdef cppclass cRequestChannel_ptr "::thrift::py3::RequestChannel_ptr":
        pass

    cdef cFollyFuture[cRequestChannel_ptr] createThriftChannelTCP(
        string&& host,
        const uint16_t port,
        const uint32_t connect_timeout,
        ClientType,
        cProtocol,
        string&& endpoint,
    )

    cdef cFollyFuture[cRequestChannel_ptr] createThriftChannelUnix(
        string&& path,
        const uint32_t connect_timeout,
        ClientType,
        cProtocol,
    )
    cdef void destroyInEventBaseThread(cRequestChannel_ptr)
    cdef unique_ptr[cClientWrapper] makeClientWrapper[T, U](cRequestChannel_ptr channel)

cdef extern from "thrift/lib/py3/client_wrapper.h" namespace "::thrift::py3":
    cdef cppclass cClientWrapper "::thrift::py3::ClientWrapper":
        void setPersistentHeader(const string& key, const string& value)
        void addEventHandler(const shared_ptr[cTProcessorEventHandler]& handler)

cdef extern from "thrift/lib/cpp/TProcessorEventHandler.h" namespace "::apache::thrift":
    cdef cppclass cTProcessorEventHandler "apache::thrift::TProcessorEventHandler":
        pass

cdef class Client:
    cdef object __weakref__
    cdef object _context_entered
    cdef object _connect_future
    cdef object _deferred_headers
    cdef object _aexit_callback
    cdef vector[shared_ptr[cTProcessorEventHandler]] _deferred_event_handlers
    cdef cFollyExecutor* _executor
    cdef AsyncioExecutor _executor_wrapper
    cdef unique_ptr[cClientWrapper] _client
    cdef inline _check_connect_future(self):
        if not self._connect_future.done():
            # This is actually using the import in the generated client
            raise asyncio_InvalidStateError(f'thrift-py3 client: {self!r} is not in Context')
        ex = self._connect_future.exception()
        if ex:
            raise ex

    cdef const type_info* _typeid(self)
    cdef bind_client(self, cRequestChannel_ptr&& channel)
    cdef add_event_handler(self, const shared_ptr[cTProcessorEventHandler]& handler)

cdef void requestchannel_callback(
        cFollyTry[cRequestChannel_ptr]&& result,
        PyObject* userData) noexcept

cdef void interactions_callback(
        cFollyTry[unique_ptr[cClientWrapper]]&& result,
        PyObject* userData) noexcept

cpdef object get_proxy_factory()
