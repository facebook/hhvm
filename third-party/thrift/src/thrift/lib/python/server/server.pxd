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
from libcpp.string cimport string
from folly.iobuf cimport cIOBuf
from thrift.python.types cimport ServiceInterface as cServiceInterface
from thrift.py3.server cimport cAsyncProcessorFactory, AsyncProcessorFactory, ThriftServer as ThriftServer_py3
from thrift.py3.exceptions cimport cException

cdef extern from "thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h" namespace "::apache::thrift":
    cpdef enum class RpcKind:
        SINGLE_REQUEST_SINGLE_RESPONSE = 0
        SINGLE_REQUEST_NO_RESPONSE = 1
        SINGLE_REQUEST_STREAMING_RESPONSE = 4
        SINK = 6

cdef extern from "thrift/lib/python/server/server.h" namespace "::thrift::python":
    cdef cppclass cPythonAsyncProcessorFactory "::thrift::python::PythonAsyncProcessorFactory"(cAsyncProcessorFactory):
        cPythonAsyncProcessorFactory()
    cdef cppclass cPythonUserException "::thrift::python::PythonUserException"(cException):
        cPythonUserException(string, string, unique_ptr[cIOBuf] buf) except +

cdef extern from "thrift/lib/cpp2/async/RpcTypes.h" namespace "::apache::thrift":
    cdef cppclass SerializedRequest "::apache::thrift::SerializedRequest":
        unique_ptr[cIOBuf] buffer

cdef class PythonAsyncProcessorFactory(AsyncProcessorFactory):
    @staticmethod
    cdef PythonAsyncProcessorFactory create(dict funcMap, list lifecycleFuncs, bytes serviceName, object server)

cdef class PythonUserException(Exception):
    cdef unique_ptr[cPythonUserException] _cpp_obj

cdef class ThriftServer(ThriftServer_py3):
    cdef readonly dict funcMap
    cdef readonly list lifecycle
    cdef readonly cServiceInterface handler
