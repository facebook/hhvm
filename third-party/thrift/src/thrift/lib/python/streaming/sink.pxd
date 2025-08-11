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

from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.optional cimport optional

from folly cimport cFollyExecutor, cFollyPromise
from folly.async_generator cimport cAsyncGenerator
from folly.coro cimport cFollyCoroTask
from folly.iobuf cimport cIOBuf
from thrift.python.protocol cimport Protocol

cdef extern from "thrift/lib/cpp2/async/Sink.h" namespace "::apache::thrift":

    cdef cppclass cResponseAndClientSink "::apache::thrift::ResponseAndClientSink"[TInitResponse, TChunk, TFinalResponse]:
        TInitResponse response
        cIOBufClientSink sink

cdef extern from "thrift/lib/python/streaming/Sink.h" namespace "::apache::thrift::python":
    cdef cppclass cIOBufClientSink "::apache::thrift::python::IOBufClientSink":
        cClientSink()
        cFollyCoroTask[cIOBuf] sink(cAsyncGenerator[unique_ptr[cIOBuf]])

    cAsyncGenerator[TChunk] toAsyncGenerator[TChunk](
      object,
      cFollyExecutor*,
      void(*)(object, cFollyPromise[optional[TChunk]])
    )

cdef class ClientSink:
    cdef unique_ptr[cIOBufClientSink] _cpp_obj
    cdef _sink_elem_cls
    cdef _sink_final_resp_cls
    cdef Protocol _protocol

    @staticmethod
    cdef _fbthrift_create(
        unique_ptr[cIOBufClientSink]&& client_sink,
        sink_cls,
        sink_final_resp_cls,
        Protocol protocol,
    )

cdef class ResponseAndClientSink:
    pass

cdef api void cancelAsyncGenerator(object generator)
