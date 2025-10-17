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

import asyncio
from libcpp.memory cimport unique_ptr
from libcpp.utility cimport move as cmove
from folly cimport cFollyPromise

from thrift.python.protocol cimport Protocol
from thrift.python.streaming.sink cimport ServerSinkGenerator, ClientSink, cIOBufClientSink
from thrift.python.streaming.stream cimport ClientBufferedStream, cIOBufClientBufferedStream
from thrift.python.streaming.py_promise cimport Promise_PyObject


cdef class BidirectionalStream:
    @staticmethod
    cdef _fbthrift_create(
        unique_ptr[cIOBufClientSink]&& client_sink,
        unique_ptr[cIOBufClientBufferedStream] stream,
        sink_elem_cls,
        stream_elem_cls,
        Protocol protocol,
    ):
        inst = <BidirectionalStream>BidirectionalStream.__new__(BidirectionalStream)
        inst._sink = ClientSink._fbthrift_create(
            cmove(client_sink),
            sink_elem_cls,
            None,  # BidirectionalStream sink doesn't return a final response
            protocol,
        )
        inst._stream = ClientBufferedStream._fbthrift_create(
            cmove(stream),
            stream_elem_cls,
            protocol,
        )
        inst._sink_elem_cls = sink_elem_cls
        inst._stream_elem_cls = stream_elem_cls
        return inst

    def __init__(self):
        raise RuntimeError("Do not instantiate BidirectionalStream from Python")

    @property
    def sink(self):
        return self._sink

    @property
    def stream(self):
        return self._stream


cdef class ResponseAndBidirectionalStream:
    @staticmethod
    cdef _fbthrift_create(
        object response,
        unique_ptr[cIOBufClientSink]&& client_sink,
        unique_ptr[cIOBufClientBufferedStream] stream,
        response_cls,
        sink_elem_cls,
        stream_elem_cls,
        Protocol protocol,
    ):
        inst = <ResponseAndBidirectionalStream>ResponseAndBidirectionalStream.__new__(ResponseAndBidirectionalStream)
        inst._response = response
        inst._sink = ClientSink._fbthrift_create(
            cmove(client_sink),
            sink_elem_cls,
            None,  # BidirectionalStream sink doesn't return a final response
            protocol,
        )
        inst._stream = ClientBufferedStream._fbthrift_create(
            cmove(stream),
            stream_elem_cls,
            protocol,
        )
        inst._response_cls = response_cls
        inst._sink_elem_cls = sink_elem_cls
        inst._stream_elem_cls = stream_elem_cls
        return inst

    def __init__(self):
        raise RuntimeError("Do not instantiate ResponseAndBidirectionalStream from Python")

    @property
    def response(self):
        return self._response

    @property
    def sink(self):
        return self._sink

    @property
    def stream(self):
        return self._stream

async def invokeBidiTransformCallback(
    bidi_callback,
    ServerSinkGenerator input_gen,
    Promise_PyObject promise,
):
    """
    Invoke bidi callback with input generator and return output generator.
    bidi_callback should be Callable[AsyncGenerator, AsyncGenerator]
    """
    async def invoke_cpp_iobuf_gen():
        async for elem in input_gen:
            yield elem

    try:
        input_gen_wrapper = invoke_cpp_iobuf_gen()
        output_gen = bidi_callback(input_gen_wrapper)
        # Store the output generator in the promise for later retrieval
        promise.complete(output_gen)
    except Exception as ex:
        promise.error_py_object(ex)


cdef api int invoke_server_bidi_callback(
    object bidi_callback,
    cFollyExecutor* executor,
    cIOBufSinkGenerator cpp_input_gen,
    cFollyPromise[PyObject*] cpp_promise,
) except -1:
    input_gen = ServerSinkGenerator._fbthrift_create(
        cmove(cpp_input_gen),
        executor,
    )
    cdef Promise_PyObject promise = Promise_PyObject.create(cmove(cpp_promise))
    asyncio.get_event_loop().create_task(
        invokeBidiTransformCallback(
            bidi_callback,
            input_gen,
            promise,
        ),
    )
    return 0
