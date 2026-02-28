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
from libcpp.optional cimport optional
from cpython.ref cimport PyObject

from folly cimport cFollyPromise
from folly.iobuf cimport cIOBuf, IOBuf
from thrift.python.exceptions cimport cTApplicationException
from thrift.python.streaming.python_user_exception cimport cPythonUserException

# cython doesn't support * in template parameters
# Make a typedef to workaround this.
ctypedef PyObject* PyObjPtr

cdef class Promise_Py:
    cdef error_ta(Promise_Py self, cTApplicationException err)
    cdef error_py(Promise_Py self, cPythonUserException err)
    cdef complete(Promise_Py self, object pyobj)

cdef class Promise_Optional_IOBuf(Promise_Py):
    cdef cFollyPromise[optional[unique_ptr[cIOBuf]]]* cPromise

    cdef error_ta(Promise_Optional_IOBuf self, cTApplicationException err)
    cdef error_py(Promise_Optional_IOBuf self, cPythonUserException err)
    cdef complete(Promise_Optional_IOBuf self, object pyobj)

    @staticmethod
    cdef create(cFollyPromise[optional[unique_ptr[cIOBuf]]] promise)

cdef class Promise_IOBuf(Promise_Py):
    cdef cFollyPromise[unique_ptr[cIOBuf]]* cPromise

    cdef error_ta(Promise_IOBuf self, cTApplicationException err)
    cdef error_py(Promise_IOBuf self, cPythonUserException err)
    cdef complete(Promise_IOBuf self, object pyobj)

    @staticmethod
    cdef create(cFollyPromise[unique_ptr[cIOBuf]] cPromise)

cdef class Promise_PyObject(Promise_Py):
    cdef cFollyPromise[PyObjPtr]* cPromise

    cdef error_ta(Promise_PyObject self, cTApplicationException err)
    cdef error_py(Promise_PyObject self, cPythonUserException err)
    cdef error_py_object(Promise_PyObject self, object ex)
    cdef complete(Promise_PyObject self, object pyobj)

    @staticmethod
    cdef create(cFollyPromise[PyObjPtr] promise)

# Make this API usable from Cython for bidi
cdef api void genNextStreamValue(object generator, cFollyPromise[optional[unique_ptr[cIOBuf]]] promise) noexcept

cdef void genNextSinkValue(object generator, cFollyPromise[optional[unique_ptr[cIOBuf]]] promise) noexcept
