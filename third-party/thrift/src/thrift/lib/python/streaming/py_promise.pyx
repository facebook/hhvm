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

from cpython cimport bool as pbool
from cython.operator cimport dereference
from libcpp.utility cimport move as cmove

import asyncio
import traceback
import sys

from folly.iobuf cimport IOBuf
from thrift.python.exceptions cimport (
    ApplicationError,
    cTApplicationExceptionType__UNKNOWN,
)
from thrift.python.streaming.python_user_exception cimport PythonUserException


cdef class Promise_Py:
    cdef error_ta(Promise_Py self, cTApplicationException err):
        pass

    cdef error_py(Promise_Py self, cPythonUserException err):
        pass

    cdef complete(Promise_Py self, object pyobj):
        pass


cdef class Promise_Optional_IOBuf(Promise_Py):
    def __cinit__(self):
        self.cPromise = new cFollyPromise[optional[unique_ptr[cIOBuf]]](
            cFollyPromise[optional[unique_ptr[cIOBuf]]].makeEmpty()
        )

    def __dealloc__(self):
        del self.cPromise

    cdef error_ta(Promise_Optional_IOBuf self, cTApplicationException err):
        self.cPromise.setException(err)

    cdef error_py(Promise_Optional_IOBuf self, cPythonUserException err):
        self.cPromise.setException(cmove(err))

    cdef complete(Promise_Optional_IOBuf self, object pyobj):
        self.cPromise.setValue(cmove((<IOBuf>pyobj)._ours))

    @staticmethod
    cdef create(cFollyPromise[optional[unique_ptr[cIOBuf]]] promise):
        cdef Promise_Optional_IOBuf inst = Promise_Optional_IOBuf.__new__(Promise_Optional_IOBuf)
        inst.cPromise[0] = cmove(promise)
        return inst

cdef str SERVER_ERR_MSG = "server stream handler"
cdef str SINK_ERR_MSG = "client sink generator"

# run a user-supplied async generator to get next item
async def runGenerator(
    object generator,
    Promise_Optional_IOBuf promise not None,
    pbool is_client_sink not None
):
    try:
        item = await generator.asend(None)
    except StopAsyncIteration:
        promise.cPromise.setValue(optional[unique_ptr[cIOBuf]]())
    except PythonUserException as pyex:
        promise.error_py(cmove(dereference((<PythonUserException>pyex)._cpp_obj.release())))
    except ApplicationError as ex:
        # If the handler raised an ApplicationError convert it to a C++ one
        promise.cPromise.setException(cTApplicationException(
            ex.type.value, ex.message.encode('UTF-8')
        ))
    except asyncio.CancelledError as ex:
        err_msg = SINK_ERR_MSG if is_client_sink else SERVER_ERR_MSG
        print(f"Coroutine was cancelled in {err_msg}:", file=sys.stderr)
        traceback.print_exc()
        blame = "client" if is_client_sink else "server"
        msg = f"Application was cancelled on the {blame} with message: {str(ex)}"
        promise.error_ta(
            cTApplicationException(
                cTApplicationExceptionType__UNKNOWN, 
                msg.encode('UTF-8'),
            )
        )
    except Exception as ex:
        err_msg = SINK_ERR_MSG if is_client_sink else SERVER_ERR_MSG
        print(
            f"Unexpected error in {err_msg}:",
            file=sys.stderr
        )
        traceback.print_exc()
        promise.error_ta(cTApplicationException(
            cTApplicationExceptionType__UNKNOWN, repr(ex).encode('UTF-8')
        ))
    else:
        promise.complete(item)

cdef void genNextStreamValue(object generator, cFollyPromise[optional[unique_ptr[cIOBuf]]] promise) noexcept:
    cdef Promise_Optional_IOBuf __promise = Promise_Optional_IOBuf.create(cmove(promise))
    asyncio.get_event_loop().create_task(
        runGenerator(
            generator,
            __promise,
            is_client_sink=False,
        )
    )

cdef void genNextSinkValue(object generator, cFollyPromise[optional[unique_ptr[cIOBuf]]] promise) noexcept:
    cdef Promise_Optional_IOBuf __promise = Promise_Optional_IOBuf.create(cmove(promise))
    asyncio.get_event_loop().create_task(
        runGenerator(
            generator,
            __promise,
            is_client_sink=True,
        )
    )
