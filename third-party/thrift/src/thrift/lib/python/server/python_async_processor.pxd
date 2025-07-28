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

from cpython.ref cimport PyObject
from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from libcpp.map cimport map as cmap
from libcpp.pair cimport pair
from libcpp.vector cimport vector as cvector
from folly.iobuf cimport cIOBuf
from thrift.python.types cimport ServiceInterface as cServiceInterface
from thrift.python.server_impl.async_processor cimport (
    cAsyncProcessorFactory,
    AsyncProcessorFactory,
)
from thrift.python.exceptions cimport cException
from libcpp.memory cimport shared_ptr
from libcpp cimport bool as cbool
from folly.executor cimport cAsyncioExecutor

# cython doesn't support * in template parameters
# Make a typedef to workaround this.
ctypedef PyObject* PyObjPtr


cdef extern from "thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h" namespace "::apache::thrift":
    cpdef enum class RpcKind:
        SINGLE_REQUEST_SINGLE_RESPONSE = 0
        SINGLE_REQUEST_NO_RESPONSE = 1
        SINGLE_REQUEST_STREAMING_RESPONSE = 4
        SINK = 6

cdef extern from "thrift/lib/python/streaming/PythonUserException.h" namespace "::apache::thrift::python":
    cdef cppclass cPythonUserException "::apache::thrift::python::PythonUserException"(cException):
        cPythonUserException(string, string, unique_ptr[cIOBuf] buf) except +

cdef extern from "thrift/lib/python/server/PythonAsyncProcessorFactory.h" namespace "::apache::thrift::python":
    cdef cppclass cPythonAsyncProcessorFactory "::apache::thrift::python::PythonAsyncProcessorFactory"(cAsyncProcessorFactory):
        pass

    cdef shared_ptr[cPythonAsyncProcessorFactory] \
        cCreatePythonAsyncProcessorFactory "::apache::thrift::python::PythonAsyncProcessorFactory::create"(
            PyObject* server,
            cmap[string, pair[RpcKind, PyObjPtr]] funcs,
            cvector[PyObjPtr] lifecycle,
            cAsyncioExecutor* executor,
            string serviceName,
        ) except +

cdef extern from "thrift/lib/cpp2/async/RpcTypes.h" namespace "::apache::thrift":
    cdef cppclass SerializedRequest "::apache::thrift::SerializedRequest":
        unique_ptr[cIOBuf] buffer

cdef class PythonAsyncProcessorFactory(AsyncProcessorFactory):
    cdef dict funcMap
    cdef list lifecycleFuncs

    @staticmethod
    cdef PythonAsyncProcessorFactory create(cServiceInterface server)

cdef class PythonUserException(Exception):
    cdef unique_ptr[cPythonUserException] _cpp_obj
