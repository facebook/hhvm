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

from folly cimport cFollyExceptionWrapper
from libcpp.memory cimport unique_ptr
from libcpp.string cimport string


cdef extern from "thrift/lib/cpp/Thrift.h" namespace "apache::thrift":
    cdef cppclass cTException "apache::thrift::TException":
        const char* what() nogil

    cdef cppclass cTLibraryException "apache::thrift::TLibraryException"(cTException):
        pass


cdef extern from "thrift/lib/cpp/TApplicationException.h" namespace "apache::thrift":
    cdef enum class cTApplicationExceptionType "apache::thrift::TApplicationException::TApplicationExceptionType":
        pass

    cdef cppclass cTApplicationException "apache::thrift::TApplicationException"(cTException):
        cTApplicationExceptionType getType() nogil


cdef extern from "thrift/lib/cpp/transport/TTransportException.h" namespace "apache::thrift::transport":
    cdef enum class cTTransportExceptionType "apache::thrift::transport::TTransportException::TTransportExceptionType":
        pass

    cdef cppclass cTTransportException "apache::thrift::transport::TTransportException"(cTLibraryException):
        int getOptions()
        cTTransportExceptionType getType()
        int getErrno()


cdef extern from "thrift/lib/py/client/exceptions.h" namespace "::thrift::py::client::exception":
    cdef unique_ptr[T] try_make_unique_exception[T](const cFollyExceptionWrapper& ex)


cdef object create_ApplicationError(unique_ptr[cTApplicationException] ex)


cdef object create_TransportError(unique_ptr[cTTransportException] ex)


cdef object create_py_exception(const cFollyExceptionWrapper& ex)
