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

from libcpp.memory cimport unique_ptr

from thrift.python.protocol cimport Protocol
from thrift.python.streaming.sink cimport ClientSink, cIOBufClientSink
from thrift.python.streaming.stream cimport ClientBufferedStream, cIOBufClientBufferedStream

cdef extern from "thrift/lib/cpp2/async/BiDiStream.h" namespace "::apache::thrift":

    cdef cppclass cBidirectionalStream "::apache::thrift::BidirectionalStream"[TSinkElement, TStreamElement]:
        cIOBufClientSink sink
        # ClientBufferedStream will be imported from stream module

    cdef cppclass cResponseAndBidirectionalStream "::apache::thrift::ResponseAndBidirectionalStream"[TResponse, TSinkElement, TStreamElement]:
        TResponse response
        cIOBufClientSink sink
        # ClientBufferedStream will be imported from stream module


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
