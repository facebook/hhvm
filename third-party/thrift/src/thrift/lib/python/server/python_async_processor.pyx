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

from thrift.python.serializer import serialize_iobuf
from thrift.python.types import ServiceInterface

cimport cython
from cpython.ref cimport PyObject
from cython.operator cimport dereference
from libcpp.map cimport map as cmap
from libcpp.memory cimport make_unique, make_shared, static_pointer_cast
from libcpp.optional cimport optional
from libcpp.pair cimport pair
from libcpp.unordered_set cimport unordered_set
from libcpp.utility cimport move as cmove
from libcpp.vector cimport vector as cvector

from folly cimport cFollyPromise, cFollyUnit, c_unit
from folly.executor cimport get_executor
from folly.iobuf cimport IOBuf, from_unique_ptr
from thrift.python.exceptions cimport (
    ApplicationError,
    cTApplicationException,
    cTApplicationExceptionType__UNKNOWN,
)
from thrift.python.server_impl.request_context cimport (
    Cpp2RequestContext,
    handleAddressCallback,
    RequestContext,
    THRIFT_REQUEST_CONTEXT,
)
from thrift.python.server_impl.request_context import (
    RequestContext,
    SocketAddress,
)
from thrift.py3.stream cimport (
    cServerStream,
    cResponseAndServerStream,
    createResponseAndServerStream,
    createAsyncIteratorFromPyIterator,
    ServerStream
)
from thrift.python.types cimport ServiceInterface as cServiceInterface
from thrift.python.protocol cimport Protocol
from thrift.python.streaming.py_promise cimport (
    Promise_IOBuf,
    Promise_Optional_IOBuf,
    Promise_Py,
    genNextStreamValue,
)
from thrift.python.streaming.python_user_exception cimport (
    cPythonUserException,
    PythonUserException,
)
from thrift.python.streaming.sink cimport (
    createResponseAndSinkConsumer,
    cResponseAndSinkConsumer,
    cSinkConsumer,
    makeIOBufSinkConsumer,
)
from cpython.contextvars cimport PyContextVar_Set, PyContextVar_Reset
from folly.request_context import _RequestContext as _FOLLY_REQUEST_CONTEXT
from folly.request_context cimport create as folly_request_context_create



ctypedef unique_ptr[cIOBuf] UniqueIOBuf
ctypedef cResponseAndServerStream[UniqueIOBuf, UniqueIOBuf] StreamResponse
ctypedef cResponseAndSinkConsumer[UniqueIOBuf, UniqueIOBuf, UniqueIOBuf] SinkResponse

@cython.final
cdef class ServerSink_IOBuf:
    cdef unique_ptr[cSinkConsumer[UniqueIOBuf, UniqueIOBuf]] _cSink

    @staticmethod
    cdef _fbthrift_create(object sink_callback):
        cdef ServerSink_IOBuf inst = ServerSink_IOBuf.__new__(ServerSink_IOBuf)
        # currently uses a dummy no-op callback that accumulates IOBuf sink elements
        # then returns them as a single IOBuf
        inst._cSink = makeIOBufSinkConsumer(sink_callback, get_executor())
        return inst

cdef class ResponseAndSinkConsumer:
    cdef unique_ptr[SinkResponse] _cResponseSink

    @staticmethod
    cdef _fbthrift_create(object val, object sink):
        cdef ResponseAndSinkConsumer inst = ResponseAndSinkConsumer.__new__(ResponseAndSinkConsumer)
        inst._cResponseSink = make_unique[SinkResponse](
            createResponseAndSinkConsumer[UniqueIOBuf, UniqueIOBuf, UniqueIOBuf](
                cmove((<IOBuf>val)._ours),
                cmove(dereference((<ServerSink_IOBuf>sink)._cSink))
            )
        )
        return inst


cdef class Promise_Sink(Promise_Py):
    cdef cFollyPromise[SinkResponse]* _cPromise

    def __cinit__(self):
        self._cPromise = new cFollyPromise[SinkResponse](cFollyPromise[SinkResponse].makeEmpty())

    def __dealloc__(self):
        del self._cPromise

    cdef error_ta(Promise_Sink self, cTApplicationException err):
        self._cPromise.setException(err)

    cdef error_py(Promise_Sink self, cPythonUserException err):
        self._cPromise.setException(cmove(err))

    cdef complete(Promise_Sink self, object pyobj):
        self._cPromise.setValue(
            cmove(dereference(cmove((<ResponseAndSinkConsumer>pyobj)._cResponseSink)))
        )

    @staticmethod
    cdef create(cFollyPromise[SinkResponse] promise):
        cdef Promise_Sink inst = Promise_Sink.__new__(Promise_Sink)
        inst._cPromise[0] = cmove(promise)
        return inst

cdef class Promise_Stream(Promise_Py):
    cdef cFollyPromise[StreamResponse]* cPromise

    def __cinit__(self):
        self.cPromise = new cFollyPromise[StreamResponse](cFollyPromise[StreamResponse].makeEmpty())

    def __dealloc__(self):
        del self.cPromise

    cdef error_ta(Promise_Stream self, cTApplicationException err):
        self.cPromise.setException(err)

    cdef error_py(Promise_Stream self, cPythonUserException err):
        self.cPromise.setException(cmove(err))

    cdef complete(Promise_Stream self, object pyobj):
        self.cPromise.setValue(cmove(dereference(cmove((<ResponseAndServerStream>pyobj).cResponseStream))))

    @staticmethod
    cdef create(cFollyPromise[StreamResponse] cPromise):
        cdef Promise_Stream inst = Promise_Stream.__new__(Promise_Stream)
        inst.cPromise[0] = cmove(cPromise)
        return inst


cdef class Promise_cFollyUnit(Promise_Py):
    cdef cFollyPromise[cFollyUnit]* cPromise

    def __cinit__(self):
        self.cPromise = new cFollyPromise[cFollyUnit](cFollyPromise[cFollyUnit].makeEmpty())

    def __dealloc__(self):
        del self.cPromise

    cdef error_ta(Promise_cFollyUnit self, cTApplicationException err):
        self.cPromise.setException(err)

    cdef error_py(Promise_cFollyUnit self, cPythonUserException err):
        self.cPromise.setException(cmove(err))

    cdef complete(Promise_cFollyUnit self, object _):
        self.cPromise.setValue(c_unit)

    @staticmethod
    cdef create(cFollyPromise[cFollyUnit] cPromise):
        cdef Promise_cFollyUnit inst = Promise_cFollyUnit.__new__(Promise_cFollyUnit)
        inst.cPromise[0] = cmove(cPromise)
        return inst

cdef class ServerStream_IOBuf(ServerStream):
    cdef unique_ptr[cServerStream[UniqueIOBuf]] cStream

    @staticmethod
    cdef _fbthrift_create(object stream):
        cdef ServerStream_IOBuf inst = ServerStream_IOBuf.__new__(ServerStream_IOBuf)
        inst.cStream = make_unique[cServerStream[UniqueIOBuf]](
            createAsyncIteratorFromPyIterator[unique_ptr[cIOBuf]](
                stream,
                get_executor(),
                genNextStreamValue
            )
        )
        return inst

cdef class ResponseAndServerStream:
    cdef unique_ptr[StreamResponse] cResponseStream

    @staticmethod
    cdef _fbthrift_create(object val, object stream):
        cdef ResponseAndServerStream inst = ResponseAndServerStream.__new__(ResponseAndServerStream)
        inst.cResponseStream = make_unique[StreamResponse](
            createResponseAndServerStream[UniqueIOBuf, UniqueIOBuf](
                cmove((<IOBuf>val)._ours),
                cmove(dereference((<ServerStream_IOBuf>stream).cStream))
            )
        )
        return inst

async def serverCallback_coro(object callFunc, str funcName, Promise_Py promise, IOBuf buf, Protocol prot, RpcKind kind):
    try:
        if kind is RpcKind.SINGLE_REQUEST_STREAMING_RESPONSE:
            val, stream = await callFunc(buf, prot)
            stream = ServerStream_IOBuf._fbthrift_create(stream)
            val = ResponseAndServerStream._fbthrift_create(val, stream)
        elif kind is RpcKind.SINK:
            val, sink = await callFunc(buf, prot)
            sink = ServerSink_IOBuf._fbthrift_create(sink)
            val = ResponseAndSinkConsumer._fbthrift_create(val, sink)
        else:
            val = await callFunc(buf, prot)
    except PythonUserException as pyex:
        promise.error_py(cmove(dereference((<PythonUserException>pyex)._cpp_obj.release())))
    except ApplicationError as ex:
        # If the handler raised an ApplicationError convert it to a C++ one
        promise.error_ta(cTApplicationException(
            ex.type.value, ex.message.encode('UTF-8')
        ))
    except Exception as ex:
        print(
            f"Unexpected error in {funcName}:",
            file=sys.stderr)
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN,
            repr(ex).encode('UTF-8'),
        ))
    except asyncio.CancelledError as ex:
        print(f"Coroutine was cancelled in service handler {funcName}:", file=sys.stderr)
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN,
            (f'Application was cancelled on the server with message: {str(ex)}').encode('UTF-8'),
        ))
    else:
        promise.complete(val)

async def lifecycle_coro(object func, str funcName, Promise_Py promise):
    try:
        if func is None:
            promise.complete(c_unit)
            return
        await func()
    except PythonUserException as pyex:
        promise.error_py(cmove(dereference((<PythonUserException>pyex)._cpp_obj.release())))
    except ApplicationError as ex:
        # If the handler raised an ApplicationError convert it to a C++ one
        promise.error_ta(cTApplicationException(
            ex.type.value, ex.message.encode('UTF-8')
        ))
    except Exception as ex:
        print(
            f"Unexpected error in {funcName}:",
            file=sys.stderr)
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN, repr(ex).encode('UTF-8')
        ))
    except asyncio.CancelledError as ex:
        print(f"Coroutine was cancelled in service handler {funcName}:", file=sys.stderr)
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN, (f'Application was cancelled on the server with message: {str(ex)}').encode('UTF-8')
        ))
    else:
        promise.complete(c_unit)

cdef int combinedHandler(
    object func,
    string funcName,
    Cpp2RequestContext* ctx,
    Promise_Py promise,
    SerializedRequest serializedRequest,
    Protocol prot,
    RpcKind kind,
) except -1:
    __context = RequestContext._fbthrift_create(ctx)
    __context_token = PyContextVar_Set(THRIFT_REQUEST_CONTEXT, __context)

    __folly_context_token = PyContextVar_Set(_FOLLY_REQUEST_CONTEXT, folly_request_context_create())

    asyncio.get_event_loop().create_task(
        serverCallback_coro(
            func,
            funcName.decode('UTF-8'),
            promise,
            from_unique_ptr(cmove(serializedRequest.buffer)),
            prot,
            kind,
        )
    )

    PyContextVar_Reset(_FOLLY_REQUEST_CONTEXT, __folly_context_token)
    PyContextVar_Reset(THRIFT_REQUEST_CONTEXT, __context_token)
    return 0

cdef api void handleLifecycleCallback(object func, string funcName, cFollyPromise[cFollyUnit] cPromise):
    cdef Promise_cFollyUnit __promise = Promise_cFollyUnit.create(cmove(cPromise))
    asyncio.get_event_loop().create_task(lifecycle_coro(func, funcName.decode('UTF-8'), __promise))

cdef api int handleServerCallback(
    object func,
    string funcName,
    Cpp2RequestContext* ctx,
    cFollyPromise[unique_ptr[cIOBuf]] cPromise,
    SerializedRequest serializedRequest,
    Protocol prot,
    RpcKind kind,
) except -1:
    cdef Promise_IOBuf __promise = Promise_IOBuf.create(cmove(cPromise))
    return combinedHandler(func, funcName, ctx, __promise, cmove(serializedRequest), prot, kind)

cdef api int handleServerStreamCallback(
    object func,
    string funcName,
    Cpp2RequestContext* ctx,
    cFollyPromise[cResponseAndServerStream[unique_ptr[cIOBuf], unique_ptr[cIOBuf]]] cPromise,
    SerializedRequest serializedRequest,
    Protocol prot,
    RpcKind kind,
) except -1:
    cdef Promise_Stream __promise = Promise_Stream.create(cmove(cPromise))
    return combinedHandler(func, funcName, ctx, __promise, cmove(serializedRequest), prot, kind)

cdef api int handleServerSinkCallback(
    object func,
    string funcName,
    Cpp2RequestContext* ctx,
    cFollyPromise[cResponseAndSinkConsumer[unique_ptr[cIOBuf], unique_ptr[cIOBuf], unique_ptr[cIOBuf]]] cPromise,
    SerializedRequest serializedRequest,
    Protocol prot,
    RpcKind kind,
) except -1:
    cdef Promise_Sink __promise = Promise_Sink.create(cmove(cPromise))
    return combinedHandler(func, funcName, ctx, __promise, cmove(serializedRequest), prot, kind)

cdef api int handleServerCallbackOneway(
    object func,
    string funcName,
    Cpp2RequestContext* ctx,
    cFollyPromise[cFollyUnit] cPromise,
    SerializedRequest serializedRequest,
    Protocol prot,
    RpcKind kind,
) except -1:
    cdef Promise_cFollyUnit __promise = Promise_cFollyUnit.create(cmove(cPromise))
    return combinedHandler(func, funcName, ctx, __promise, cmove(serializedRequest), prot, kind)

cdef api unique_ptr[cIOBuf] getSerializedPythonMetadata(object server):
    metadata = server.__get_metadata_service_response__()
    iobuf = serialize_iobuf(metadata, protocol=Protocol.BINARY)
    return cmove((<IOBuf>iobuf)._ours)

cdef class PythonAsyncProcessorFactory(AsyncProcessorFactory):
    @staticmethod
    cdef PythonAsyncProcessorFactory create(cServiceInterface server):
        cdef cmap[string, pair[RpcKind, PyObjPtr]] funcs
        cdef cvector[PyObject*] lifecycle

        cdef dict funcMap = server.getFunctionTable()
        cdef list lifecycleFuncs = [server.onStartServing, server.onStopRequested]

        for name, (rpc_kind, func) in funcMap.items():
            funcs[<string>name] = pair[RpcKind, PyObjPtr](<RpcKind>rpc_kind, <PyObject*>func)

        for lifecycle_func in lifecycleFuncs:
            lifecycle.push_back(<PyObject*>lifecycle_func)

        cdef PythonAsyncProcessorFactory inst = PythonAsyncProcessorFactory.__new__(PythonAsyncProcessorFactory)
        inst.funcMap = funcMap
        inst.lifecycleFuncs = lifecycleFuncs
        inst._cpp_obj = static_pointer_cast[cAsyncProcessorFactory, cPythonAsyncProcessorFactory](
            cCreatePythonAsyncProcessorFactory(
                <PyObject*>server,
                cmove(funcs),
                cmove(lifecycle),
                get_executor(),
                <bytes>server.service_name()))
        return inst
