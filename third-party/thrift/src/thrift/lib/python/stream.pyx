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

from cpython.ref cimport PyObject
from cython.operator cimport dereference as deref
from libcpp.memory cimport make_unique, unique_ptr
from libcpp.utility cimport move as cmove

from folly cimport cFollyTry
from folly.coro cimport bridgeCoroTaskWith
from folly.executor cimport get_executor
from folly.iobuf cimport from_unique_ptr
from folly.optional cimport cOptional

from thrift.python.exceptions cimport create_py_exception
from thrift.python.serializer import deserialize


cdef void client_stream_callback(
    cFollyTry[cOptional[cIOBuf]]&& result,
    PyObject* userdata,
) noexcept:
    cdef cOptional[cIOBuf] opt_val
    cdef ClientBufferedStream stream
    stream, pyfuture = <object> userdata
    if result.hasException():
        pyfuture.set_exception(
            #TODO: T133841402
            create_py_exception(result.exception(), None)
        )
    else:
        opt_val = result.value()
        if opt_val.has_value():
            try:
                response_iobuf = from_unique_ptr(make_unique[cIOBuf](cmove(opt_val.value())))
                response = deserialize(stream._class, response_iobuf, protocol=stream._protocol)
                if response.success is not None:
                    pyfuture.set_result(response.success)
                else:
                    for f, v in response:
                        if v is not None:
                            pyfuture.set_exception(v)
                            break
            except Exception as ex:
                pyfuture.set_exception(ex.with_traceback(None))
        else:
            pyfuture.set_exception(StopAsyncIteration())


cdef class ClientBufferedStream:
    @staticmethod
    cdef _fbthrift_create(unique_ptr[cIOBufClientBufferedStream] stream, klass, Protocol prot):
        cdef ClientBufferedStream inst = ClientBufferedStream.__new__(ClientBufferedStream)
        inst._executor = get_executor()
        inst._class = klass
        inst._protocol = prot
        inst._gen = make_unique[cIOBufClientBufferedStreamWrapper](deref(stream))
        return inst

    def __aiter__(self):
        return self

    def __anext__(self):
        loop = asyncio.get_running_loop()
        future = loop.create_future()
        # to avoid "Future exception was never retrieved" error at SIGINT
        future.add_done_callback(lambda x: x.exception())
        userdata = (self, future)
        bridgeCoroTaskWith[cOptional[cIOBuf]](
            self._executor,
            deref(self._gen).getNext(),
            client_stream_callback,
            <PyObject *>userdata,
        )
        return asyncio.shield(future)
