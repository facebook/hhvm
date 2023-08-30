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

from thrift.python.metadata import gen_metadata, ThriftMetadata
from thrift.python.serializer import serialize_iobuf
from thrift.python.types import ServiceInterface

from cpython.ref cimport PyObject
from cython.operator cimport dereference
from libcpp.map cimport map as cmap
from libcpp.memory cimport make_unique, make_shared, static_pointer_cast
from libcpp.pair cimport pair
from libcpp.unordered_set cimport unordered_set
from libcpp.utility cimport move as cmove
from libcpp.vector cimport vector as cvector
from folly.executor cimport get_executor
from folly.iobuf cimport IOBuf, from_unique_ptr
from thrift.py3.exceptions cimport cTApplicationException, cTApplicationExceptionType__UNKNOWN, ApplicationError
from thrift.py3.server cimport Cpp2RequestContext, RequestContext, THRIFT_REQUEST_CONTEXT
from thrift.python.types cimport ServiceInterface as cServiceInterface
from thrift.python.serializer cimport Protocol
from folly cimport (
  cFollyPromise,
  cFollyUnit,
  c_unit,
)

cdef class PythonUserException(Exception):
    def __init__(self, type_: str, reason: str, buf: IOBuf) -> None:
        self._cpp_obj = make_unique[cPythonUserException](<string>type_.encode('UTF-8'), <string>reason.encode('UTF-8'), cmove(buf._ours))

cdef class Promise_Py:
    cdef error_ta(Promise_Py self, cTApplicationException err):
        pass

    cdef error_py(Promise_Py self, cPythonUserException err):
        pass

    cdef complete(Promise_Py self, object pyobj):
        pass

cdef class Promise_IOBuf(Promise_Py):
    cdef cFollyPromise[unique_ptr[cIOBuf]]* cPromise

    def __cinit__(self):
        self.cPromise = new cFollyPromise[unique_ptr[cIOBuf]](cFollyPromise[unique_ptr[cIOBuf]].makeEmpty())

    def __dealloc__(self):
        del self.cPromise

    cdef error_ta(Promise_IOBuf self, cTApplicationException err):
        self.cPromise.setException(err)

    cdef error_py(Promise_IOBuf self, cPythonUserException err):
        self.cPromise.setException(cmove(err))

    cdef complete(Promise_IOBuf self, object pyobj):
        self.cPromise.setValue(cmove((<IOBuf>pyobj)._ours))

    @staticmethod
    cdef create(cFollyPromise[unique_ptr[cIOBuf]] cPromise):
        cdef Promise_IOBuf inst = Promise_IOBuf.__new__(Promise_IOBuf)
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

async def serverCallback_coro(object callFunc, str funcName, Promise_Py promise, IOBuf buf, Protocol prot):
    try:
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
            cTApplicationExceptionType__UNKNOWN, repr(ex).encode('UTF-8')
        ))
    except asyncio.CancelledError as ex:
        print(f"Coroutine was cancelled in service handler {funcName}:", file=sys.stderr)
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN, (f'Application was cancelled on the server with message: {str(ex)}').encode('UTF-8')
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

cdef void combinedHandler(object func, string funcName, Cpp2RequestContext* ctx, Promise_Py promise, SerializedRequest serializedRequest, Protocol prot):
    __context = RequestContext._fbthrift_create(ctx)
    __context_token = THRIFT_REQUEST_CONTEXT.set(__context)

    asyncio.get_event_loop().create_task(
        serverCallback_coro(
            func,
            funcName.decode('UTF-8'),
            promise,
            from_unique_ptr(cmove(serializedRequest.buffer)),
            prot,
        )
    )

    THRIFT_REQUEST_CONTEXT.reset(__context_token)

cdef public api void handleLifecycleCallback(object func, string funcName, cFollyPromise[cFollyUnit] cPromise):
    cdef Promise_cFollyUnit __promise = Promise_cFollyUnit.create(cmove(cPromise))
    asyncio.get_event_loop().create_task(lifecycle_coro(func, funcName.decode('UTF-8'), __promise))

cdef public api void handleServerCallback(object func, string funcName, Cpp2RequestContext* ctx, cFollyPromise[unique_ptr[cIOBuf]] cPromise, SerializedRequest serializedRequest, Protocol prot):
    cdef Promise_IOBuf __promise = Promise_IOBuf.create(cmove(cPromise))
    combinedHandler(func, funcName, ctx, __promise, cmove(serializedRequest), prot)


cdef public api void handleServerCallbackOneway(object func, string funcName, Cpp2RequestContext* ctx, cFollyPromise[cFollyUnit] cPromise, SerializedRequest serializedRequest, Protocol prot):
    cdef Promise_cFollyUnit __promise = Promise_cFollyUnit.create(cmove(cPromise))
    combinedHandler(func, funcName, ctx, __promise, cmove(serializedRequest), prot)

cdef public api unique_ptr[cIOBuf] getSerializedPythonMetadata(object server):
    metadata = server.__get_metadata_service_response__()
    iobuf = serialize_iobuf(metadata, protocol=Protocol.BINARY)
    return cmove((<IOBuf>iobuf)._ours)

# Cython is dumb
ctypedef PyObject* PyObjPtr

cdef class PythonAsyncProcessorFactory(AsyncProcessorFactory):
    @staticmethod
    cdef PythonAsyncProcessorFactory create(dict funcMap, list lifecycleFuncs, bytes serviceName, object server):
        cdef cmap[string, pair[RpcKind, PyObjPtr]] funcs
        cdef cvector[PyObject*] lifecycle

        for name, (rpc_kind, func) in funcMap.items():
            funcs[<string>name] = pair[RpcKind, PyObjPtr](<RpcKind>rpc_kind, <PyObject*>func)

        for func in lifecycleFuncs:
            lifecycle.push_back(<PyObject*>func)

        cdef PythonAsyncProcessorFactory inst = PythonAsyncProcessorFactory.__new__(PythonAsyncProcessorFactory)
        inst._cpp_obj = static_pointer_cast[cAsyncProcessorFactory, cPythonAsyncProcessorFactory](
            make_shared[cPythonAsyncProcessorFactory](<PyObject*>server, cmove(funcs), cmove(lifecycle), get_executor(), serviceName))
        return inst

cdef class ThriftServer(ThriftServer_py3):
    def __init__(self, cServiceInterface server, int port=0, ip=None, path=None):
        self.funcMap = server.getFunctionTable()
        self.handler = server
        self.lifecycle = [self.handler.onStartServing, self.handler.onStopRequested]
        super().__init__(PythonAsyncProcessorFactory.create(self.funcMap, self.lifecycle, self.handler.service_name(), self.handler), port, ip, path)
