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
import sys
import traceback

from folly cimport cFollyPromise, cFollyTry
from folly.coro cimport (
    bridgeCoroTaskWith,
    bridgeCoroTaskWithCancellation,
    cFollyCancellationSource,
    cFollyCoroTask,
)
from folly.executor cimport get_executor

from cython.operator import dereference
from cpython.ref cimport PyObject
from libcpp.memory cimport make_shared, make_unique, shared_ptr
from libcpp.utility cimport move as cmove
from thrift.python.serializer import deserialize
from thrift.python.mutable_serializer import (
    deserialize as deserialize_mutable,
)
from folly.iobuf cimport IOBuf, from_unique_ptr as iobuf_from_unique_ptr
from thrift.python.exceptions cimport (
    ApplicationError,
    cTApplicationException,
    cTApplicationExceptionType__UNKNOWN,
    create_py_exception,
)
from thrift.python.types import Struct
from thrift.python.mutable_types import MutableStruct

from thrift.python.streaming.py_promise cimport (
    genNextSinkValue,
    Promise_IOBuf,
)
from thrift.python.streaming.python_user_exception cimport (
    cPythonUserException,
    extractPyUserExceptionIOBuf,
    PythonUserException,
)


cdef class ClientSink:
    @staticmethod
    cdef _fbthrift_create(
        unique_ptr[cIOBufClientSink]&& client_sink,
        sink_elem_cls,
        sink_final_resp_cls,
        Protocol protocol,
    ):
        inst = <ClientSink>ClientSink.__new__(ClientSink)
        inst._cpp_obj = cmove(client_sink)
        inst._sink_elem_cls = sink_elem_cls
        inst._sink_final_resp_cls = sink_final_resp_cls
        inst._protocol = protocol
        return inst

    def __init__(self):
        raise RuntimeError("Do not instantiate ClientSink from Python")

    async def sink(self, agen):
        cancellation_source = cFollyCancellationSource()
        loop = asyncio.get_event_loop()
        future = loop.create_future()
        user_data = (
            future, self._sink_final_resp_cls, self._sink_elem_cls, self._protocol,
        )

        handled_agen = self._sink_elem_cls._fbthrift__sink_elem_handler(
            agen,
            self._protocol,
        )
        bridgeCoroTaskWithCancellation[cIOBuf](
            get_executor(),
            dereference(self._cpp_obj).sink[cPythonUserException](
                toAsyncGenerator[unique_ptr[cIOBuf]](
                    handled_agen,
                    get_executor(),
                    genNextSinkValue,
                )
            ),
            sink_final_resp_callback,
            <PyObject *>user_data,
            cancellation_source.getToken(),
        )
        try:
            return await future
        except asyncio.CancelledError:
            cancellation_source.requestCancellation()
            return await future


# a helper to deserialize a response struct when 
# we don't know whether it's immutable or mutable
cdef deserialize_buf(resp_class, buf, protocol):
    if issubclass(resp_class, Struct):
        return deserialize(resp_class, buf, protocol)
    elif issubclass(resp_class, MutableStruct):
        return deserialize_mutable(resp_class, buf, protocol)
    else:
        raise RuntimeError(
            f"Invalid final response class: {resp_class.__name__}"
        )

# raises the first non-None exception in the response struct
# assumes that the .success field is None
cdef raise_first_exception_field(response_struct):
    for ex_field_name, ex_val in response_struct:
        if ex_val is not None:
            assert ex_field_name != "success"
            raise ex_val

    # this is legitimate return for void function
    return None

                

cdef void sink_final_resp_callback(
    cFollyTry[cIOBuf]&& res,
    PyObject* user_data,
):
    future, final_resp_cls, sink_elem_cls, protocol = <object> user_data
    try:
        if res.hasException():
            # PythonUserException denotes an expected (IDL-declared) exception
            user_ex_buf = extractPyUserExceptionIOBuf(res.exception())
            if user_ex_buf is not None:
                user_ex_struct = deserialize_buf(sink_elem_cls, user_ex_buf, protocol)
                raise_first_exception_field(user_ex_struct)
            # Catch-all unexpected exception from user code
            if res.exception().get_exception[cTApplicationException]():
                raise create_py_exception(res.exception(), None)
            # Translate C++ folly::OperationCancelled to Python
            if res.exception().get_exception[cFollyOperationCancelled]():
                raise asyncio.CancelledError()
            # Totally unexpected exception
            res.exception().throw_exception()


        res_buf = iobuf_from_unique_ptr(
            make_unique[cIOBuf](cmove(res.value()))
        )

        final_resp = deserialize_buf(final_resp_cls, res_buf, protocol)
        
        if final_resp.success is not None:
            future.set_result(final_resp.success)
            return

        raise_first_exception_field(final_resp)

        # This is the expected result for void return
        future.set_result(None)

    except Exception as ex:
        future.set_exception(ex)

async def fallibleClose(generator):
    try:
        await generator.aclose()
    # The above will raise:
    #   - if cancellation results from throw in generator, or
    #   - if the generator has already completed but the callback hasn't
    #     yet returned at time of cancellation, the above will raise.
    # There's now need to propagate; just suppress it.
    except Exception as ex:
        pass


cdef public api void cancelAsyncGenerator(object generator):
    asyncio.get_event_loop().create_task(
        fallibleClose(generator)
    )


async def invokeCallbackWithGenerator(
    sink_callback,
    ServerSinkGenerator sink_elem_gen,
    Promise_IOBuf promise,
):
    async def invoke_cpp_iobuf_gen():
        async for elem in sink_elem_gen:
            yield elem

    try:
        gen = invoke_cpp_iobuf_gen()
        final_resp_iobuf = await sink_callback(gen) 
        assert isinstance(final_resp_iobuf, IOBuf), f"Expected IOBuf, got {type(final_resp_iobuf)}"
        promise.complete(final_resp_iobuf)
    except PythonUserException as pyex:
        promise.error_py(cmove(dereference((<PythonUserException>pyex)._cpp_obj.release())))
    except ApplicationError as ex:
        promise.error_ta(
            cTApplicationException(ex.type.value, ex.message.encode('UTF-8'))
        )
    except asyncio.CancelledError as ex:
        print(f"Coroutine was cancelled in server sink handler:", file=sys.stderr)
        traceback.print_exc()
        msg = f"Application was cancelled on the server with message: {str(ex)}"
        promise.error_ta(
            cTApplicationException(
                cTApplicationExceptionType__UNKNOWN, 
                msg.encode('UTF-8'),
            )
        )
    except Exception as ex:
        print(
            f"Unexpected error in server sink handler:",
            file=sys.stderr
        )
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN, repr(ex).encode('UTF-8')
        ))


cdef class ServerSinkGenerator:
    cdef cIOBufSinkGenerator _cpp_gen 
    cdef cFollyExecutor* _executor

    @staticmethod
    cdef _fbthrift_create(
        cIOBufSinkGenerator cpp_gen,
        cFollyExecutor* executor,
    ):
        cdef ServerSinkGenerator inst = ServerSinkGenerator.__new__(ServerSinkGenerator)
        inst._cpp_gen = cmove(cpp_gen)
        inst._executor = executor
        return inst


    def __aiter__(self):
        return self

    def __anext__(self):
        cancellation_source = cFollyCancellationSource()
        loop = asyncio.get_event_loop()
        future = loop.create_future()
        # in case of SIGINT, retrieve the exception
        future.add_done_callback(lambda x: x.exception())
        userdata = (self, future)
        bridgeCoroTaskWithCancellation[unique_ptr[cIOBuf]](
            self._executor,
            self._cpp_gen.getNext(),
            server_sink_elem_callback,
            <PyObject*> userdata,
            cancellation_source.getToken(),
        )
        try:
            return future
        except asyncio.CancelledError:
            cancellation_source.requestCancellation()
            return future


cdef void server_sink_elem_callback(
    cFollyTry[unique_ptr[cIOBuf]]&& result,
    PyObject* userdata,
) noexcept:
    wrapped_sink, future = <object> userdata
    if result.hasException():
        future.set_exception(
            # creates ApplicationException
            create_py_exception(result.exception(), None)
        )
    elif result.value():
        future.set_result(iobuf_from_unique_ptr(cmove(result.value())))
    else: # nullptr indicates end of stream
        future.set_exception(StopAsyncIteration())

cdef api int invoke_server_sink_callback(
    object sink_callback,
    cFollyExecutor* executor,
    cIOBufSinkGenerator cpp_gen,
    cFollyPromise[unique_ptr[cIOBuf]] cpp_promise,
) except -1:
    sink_gen = ServerSinkGenerator._fbthrift_create(
        cmove(cpp_gen),
        executor,
    )
    cdef Promise_IOBuf promise = Promise_IOBuf.create(cmove(cpp_promise))
    asyncio.get_event_loop().create_task(
        invokeCallbackWithGenerator(
            sink_callback,
            sink_gen,
            promise,
        ),
    )
    return 0
