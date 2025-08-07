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

from folly cimport cFollyPromise, cFollyTry
from folly.coro cimport cFollyCoroTask, bridgeCoroTaskWithCancellation, cFollyCancellationSource
from folly.executor cimport get_executor

from cython.operator import dereference
from cpython.ref cimport PyObject
from libcpp.memory cimport make_shared, make_unique, shared_ptr
from libcpp.utility cimport move as cmove
from thrift.python.serializer import deserialize
from thrift.python.mutable_serializer import (
    deserialize as deserialize_mutable,
)
from folly.iobuf cimport from_unique_ptr as iobuf_from_unique_ptr
from thrift.python.types import Struct
from thrift.python.mutable_types import MutableStruct

from thrift.python.streaming.py_promise cimport genNextSinkValue


cdef class ClientSink:
    @staticmethod
    cdef _fbthrift_create(
        unique_ptr[cIOBufClientSink]&& client_sink,
        type sink_elem_cls,
        type sink_final_resp_cls,
        Protocol protocol,
    ):
        inst = <ClientSink>ClientSink.__new__(ClientSink)
        inst._cpp_obj = cmove(client_sink)
        inst._sink_elem_cls = sink_elem_cls
        inst._sink_final_resp_cls = sink_final_resp_cls
        inst._protocol = protocol
        return inst

  # for testing only, will remove
    def __init__(self):
        self._cpp_obj = make_unique[cIOBufClientSink]()

    async def sink(self, agen):
        cancellation_source = cFollyCancellationSource()
        loop = asyncio.get_event_loop()
        fut = loop.create_future()
        user_data = (fut, self._sink_final_resp_cls, self._protocol)

        handled_agen = self._sink_elem_cls._fbthrift__sink_elem_handler(
            agen,
        )
        bridgeCoroTaskWithCancellation[cIOBuf](
            get_executor(),
            dereference(self._cpp_obj).sink(
                toAsyncGenerator[unique_ptr[cIOBuf]](
                    handled_agen,
                    get_executor(),
                    genNextSinkValue,
                )
            ),
            sink_final_resp_callback,
            <PyObject *>fut,
            cancellation_source.getToken(),
        )
        try:
            return await asyncio.shield(fut)
        except asyncio.CancelledError:
            cancellation_source.requestCancellation()
            return await fut
                

cdef void sink_final_resp_callback(
    cFollyTry[cIOBuf]&& res,
    PyObject* user_data,
):
    future, final_resp_cls, protocol = <object> user_data
    try:
        if res.hasException():
            res.exception().throw_exception()

        res_buf = iobuf_from_unique_ptr(
            make_unique[cIOBuf](cmove(res.value()))
        )
        
        if issubclass(final_resp_cls, Struct):
            final_resp = deserialize(final_resp_cls, res_buf, protocol)
        elif issubclass(final_resp_cls, MutableStruct):
            final_resp = deserialize_mutable(final_resp_cls, res_buf, protocol)
        else:
            raise RuntimeError(
                f"Invalid final response class: {final_resp_cls.__name__}"
            )
        
        if final_resp.success is not None:
            future.set_result(final_resp.success)
            return
        
        for _ex_field_name, ex_val in final_resp:
            if ex_val is not None:
                raise ex_val

        # This is the expected result for void return
        future.set_result(None)


    except Exception as ex:
        future.set_exception(ex)

cdef public api void cancelAsyncGenerator(object generator):
    asyncio.get_event_loop().create_task(
        generator.aclose()
    )
