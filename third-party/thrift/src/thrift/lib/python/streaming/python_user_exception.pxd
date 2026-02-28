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

from folly.iobuf cimport cIOBuf, IOBuf
from folly cimport cFollyExceptionWrapper
from thrift.python.exceptions cimport cException


cdef extern from "thrift/lib/python/streaming/PythonUserException.h" namespace "::apache::thrift::python":
    cdef cppclass cPythonUserException "::apache::thrift::python::PythonUserException"(cException):
        cPythonUserException(string, string, unique_ptr[cIOBuf] buf) except +

    unique_ptr[cIOBuf] extractBufFromPythonUserException(cFollyExceptionWrapper& ew)


cdef class PythonUserException(Exception):
    cdef unique_ptr[cPythonUserException] _cpp_obj


# steals IOBuf from folly::exception_wrapper that contains PythonUserException,
# meaning the PythonUserException no longer has a valid IOBuf after this call
# may return None if the exception is not a PythonUserException
cdef IOBuf extractPyUserExceptionIOBuf(cFollyExceptionWrapper& ew)
