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
from thrift.python.streaming.stream cimport ClientBufferedStream, cIOBufClientBufferedStream

cdef extern from "folly/OperationCancelled.h":
    cdef cppclass cFollyOperationCancelled "folly::OperationCancelled"

cdef extern from "thrift/lib/cpp2/async/Sink.h" namespace "::apache::thrift":

    cdef cppclass cResponseAndClientSink "::apache::thrift::ResponseAndClientSink"[TInitResponse, TChunk, TFinalResponse]:
        TInitResponse response
        cIOBufClientSink sink

    cdef cppclass cResponseAndSinkConsumer "::apache::thrift::ResponseAndSinkConsumer"[TInitResponse, TChunk, TFinalResponse]:
        TInitResponse response
        cSinkConsumer[TChunk, TFinalResponse] sinkConsumer

    cdef cppclass cSinkConsumer "::apache::thrift::SinkConsumer"[TChunk, TFinalResponse]:
        pass

    cdef cppclass cBidirectionalStream "::apache::thrift::BidirectionalStream"[TSinkElement, TStreamElement]:
        cIOBufClientSink sink
        # ClientBufferedStream will be imported from stream module

    cdef cppclass cResponseAndBidirectionalStream "::apache::thrift::ResponseAndBidirectionalStream"[TResponse, TSinkElement, TStreamElement]:
        TResponse response
        cIOBufClientSink sink
        # ClientBufferedStream will be imported from stream module


cdef extern from "thrift/lib/python/streaming/Sink.h" namespace "::apache::thrift::python":
    cdef cppclass cIOBufClientSink "::apache::thrift::python::IOBufClientSink":
        cClientSink()
        cFollyCoroTask[unique_ptr[cIOBuf]] sink[ExpectedException](cAsyncGenerator[unique_ptr[cIOBuf]])

    cAsyncGenerator[unique_ptr[cIOBuf]] toAsyncGenerator(
      object,
      cFollyExecutor*,
      void(*)(object, cFollyPromise[optional[unique_ptr[cIOBuf]]])
    )

    cResponseAndSinkConsumer[InitResponse, TChunk, FinalResponse] \
    createResponseAndSinkConsumer[InitResponse, TChunk, FinalResponse](
        InitResponse response,
        cSinkConsumer[TChunk, FinalResponse] sink
    ) noexcept

    unique_ptr[cSinkConsumer[unique_ptr[cIOBuf], unique_ptr[cIOBuf]]] \
    makeIOBufSinkConsumer(
        object sink_callback,
        cFollyExecutor* executor,
    ) noexcept

    cdef cppclass cIOBufSinkGenerator "::apache::thrift::python::IOBufSinkGenerator":
        cIOBufSinkGenerator()
        cFollyCoroTask[unique_ptr[cIOBuf]] getNext()


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

cdef class BidirectionalStream:
    cdef ClientSink _sink
    cdef ClientBufferedStream _stream
    cdef _sink_elem_cls
    cdef _stream_elem_cls

    @staticmethod
    cdef _fbthrift_create(
        unique_ptr[cIOBufClientSink]&& client_sink,
        unique_ptr[cIOBufClientBufferedStream] stream,
        sink_elem_cls,
        stream_elem_cls,
        Protocol protocol,
    )

cdef class ResponseAndBidirectionalStream:
    cdef object _response
    cdef ClientSink _sink
    cdef ClientBufferedStream _stream
    cdef _response_cls
    cdef _sink_elem_cls
    cdef _stream_elem_cls

    @staticmethod
    cdef _fbthrift_create(
        object response,
        unique_ptr[cIOBufClientSink]&& client_sink,
        unique_ptr[cIOBufClientBufferedStream] stream,
        response_cls,
        sink_elem_cls,
        stream_elem_cls,
        Protocol protocol,
    )

cdef class ResponseAndClientSink:
    pass

cdef api void cancelAsyncGenerator(object generator)

cdef api int invoke_server_sink_callback(
    object sink_callback,
    cFollyExecutor* executor,
    cIOBufSinkGenerator cpp_gen,
    cFollyPromise[unique_ptr[cIOBuf]] cpp_promise,
) except -1
