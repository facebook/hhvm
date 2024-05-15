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

from libcpp.memory cimport unique_ptr, shared_ptr
from libcpp.optional cimport optional
from libcpp.pair cimport pair
from folly cimport cFollyExecutor
from folly.coro cimport cFollyCoroTask
from folly.optional cimport cOptional
from folly.async_generator cimport cAsyncGeneratorWrapper, cAsyncGenerator
from thrift.py3.common cimport RpcOptions
from folly cimport cFollyPromise
from cpython.ref cimport PyObject
from libc.stdint cimport int32_t

cdef extern from "thrift/lib/cpp2/async/ClientBufferedStream.h" namespace "::apache::thrift":
    cdef cppclass cClientBufferedStream "::apache::thrift::ClientBufferedStream"[T]:
         cClientBufferedStream()

    cdef cppclass cResponseAndClientBufferedStream "::apache::thrift::ResponseAndClientBufferedStream"[R, S]:
        R response
        cClientBufferedStream[S] stream
        cResponseAndClientBufferedStream()

cdef extern from "thrift/lib/cpp2/async/ServerPublisherStream.h" namespace "::apache::thrift":
    cdef cppclass cServerStreamPublisher "::apache::thrift::ServerStreamPublisher"[T]:
        void next(T)
        void complete()

cdef extern from "thrift/lib/cpp2/async/ServerStream.h" namespace "::apache::thrift":
    cdef cppclass cServerStream "::apache::thrift::ServerStream"[T]:
        cServerStream()
        cServerStream(cAsyncGenerator[T])
        @staticmethod
        cServerStream[T] createEmpty()
        @staticmethod
        pair[cServerStream[T], cServerStreamPublisher[T]] createPublisher(void(*)())

    cdef cppclass cResponseAndServerStream "::apache::thrift::ResponseAndServerStream"[R, S]:
        R response
        cServerStream[S] stream

ctypedef void (*emptyFunc)() noexcept # This typedef is necessary because otherwise cython gets a syntax error
cdef extern from "thrift/lib/py3/stream.h" namespace "::thrift::py3":
    cdef cppclass cClientBufferedStreamWrapper "::thrift::py3::ClientBufferedStreamWrapper"[T]:
        cClientBufferedStreamWrapper() except +
        cClientBufferedStreamWrapper(cClientBufferedStream[T]&, int buffer_size) except +
        cFollyCoroTask[cOptional[T]] getNext()
    cResponseAndServerStream[R, S] createResponseAndServerStream[R, S](R, cServerStream[S])
    cServerStream[T] createAsyncIteratorFromPyIterator[T](object iter,
        cFollyExecutor*,
        void(*)(object, cFollyPromise[optional[T]]),
    )
    emptyFunc pythonFuncToCppFunc(object)

cdef public api void cancelAsyncGenerator(object generator)

cdef class ClientBufferedStream:
    cdef cFollyExecutor* _executor
    cdef RpcOptions _rpc_options

cdef class ResponseAndClientBufferedStream:
    pass

cdef class ServerStream:
    pass

cdef class ServerPublisher:
    pass

cdef class ResponseAndServerStream:
    pass
