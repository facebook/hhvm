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
from folly cimport cFollyExceptionWrapper, iobuf
from libc.stdint cimport int16_t, uint32_t
from libcpp.memory cimport unique_ptr
from libcpp.string cimport string
from thrift.py3.common cimport RpcOptions

from thrift.python.serializer cimport Protocol


cdef extern from "thrift/lib/cpp/Thrift.h" namespace "apache::thrift":
    cdef cppclass cTException "apache::thrift::TException":
        const char* what() nogil

    cdef cppclass cTLibraryException "apache::thrift::TLibraryException"(cTException):
        pass


cdef extern from "thrift/lib/cpp/TApplicationException.h" namespace "apache::thrift":

    cpdef enum class ApplicationErrorType "apache::thrift::TApplicationException::TApplicationExceptionType":
        UNKNOWN "apache::thrift::TApplicationException::UNKNOWN"
        UNKNOWN_METHOD "apache::thrift::TApplicationException::UNKNOWN_METHOD"
        INVALID_MESSAGE_TYPE "apache::thrift::TApplicationException::INVALID_MESSAGE_TYPE"
        WRONG_METHOD_NAME "apache::thrift::TApplicationException::WRONG_METHOD_NAME"
        BAD_SEQUENCE_ID "apache::thrift::TApplicationException::BAD_SEQUENCE_ID"
        MISSING_RESULT "apache::thrift::TApplicationException::MISSING_RESULT"
        INTERNAL_ERROR "apache::thrift::TApplicationException::INTERNAL_ERROR"
        PROTOCOL_ERROR "apache::thrift::TApplicationException::PROTOCOL_ERROR"
        INVALID_TRANSFORM "apache::thrift::TApplicationException::INVALID_TRANSFORM"
        INVALID_PROTOCOL "apache::thrift::TApplicationException::INVALID_PROTOCOL"
        UNSUPPORTED_CLIENT_TYPE "apache::thrift::TApplicationException::UNSUPPORTED_CLIENT_TYPE"
        LOADSHEDDING "apache::thrift::TApplicationException::LOADSHEDDING"
        TIMEOUT "apache::thrift::TApplicationException::TIMEOUT"
        INJECTED_FAILURE "apache::thrift::TApplicationException::INJECTED_FAILURE"

    cdef cppclass cTApplicationException "apache::thrift::TApplicationException"(cTException):
        cTApplicationException(ApplicationErrorType type, const string& message) nogil except +
        ApplicationErrorType getType() nogil


cdef extern from "thrift/lib/cpp/transport/TTransportException.h" namespace "apache::thrift::transport":

    cpdef enum class TransportErrorType "apache::thrift::transport::TTransportException::TTransportExceptionType":
        UNKNOWN "apache::thrift::transport::TTransportException::UNKNOWN"
        NOT_OPEN "apache::thrift::transport::TTransportException::NOT_OPEN"
        ALREADY_OPEN "apache::thrift::transport::TTransportException::ALREADY_OPEN"
        TIMED_OUT "apache::thrift::transport::TTransportException::TIMED_OUT"
        END_OF_FILE "apache::thrift::transport::TTransportException::END_OF_FILE"
        INTERRUPTED "apache::thrift::transport::TTransportException::INTERRUPTED"
        BAD_ARGS "apache::thrift::transport::TTransportException::BAD_ARGS"
        CORRUPTED_DATA "apache::thrift::transport::TTransportException::CORRUPTED_DATA"
        INTERNAL_ERROR "apache::thrift::transport::TTransportException::INTERNAL_ERROR"
        NOT_SUPPORTED "apache::thrift::transport::TTransportException::NOT_SUPPORTED"
        INVALID_STATE "apache::thrift::transport::TTransportException::INVALID_STATE"
        INVALID_FRAME_SIZE "apache::thrift::transport::TTransportException::INVALID_FRAME_SIZE"
        SSL_ERROR "apache::thrift::transport::TTransportException::SSL_ERROR"
        COULD_NOT_BIND "apache::thrift::transport::TTransportException::COULD_NOT_BIND"
        NETWORK_ERROR "apache::thrift::transport::TTransportException::NETWORK_ERROR"

    cdef cppclass cTTransportException "apache::thrift::transport::TTransportException"(cTLibraryException):
        int getOptions()
        TransportErrorType getType()
        int getErrno()


cdef extern from "thrift/lib/python/exceptions.h" namespace "::thrift::python::exception":
    cdef unique_ptr[T] try_make_unique_exception[T](const cFollyExceptionWrapper& ex)


cdef class Error(Exception):
    """base class for all Thrift exceptions"""
    pass

cdef class LibraryError(Error):
    pass


cdef class ApplicationError(Error):
    pass

cdef ApplicationError create_ApplicationError(unique_ptr[cTApplicationException] ex)


cdef class TransportError(LibraryError):
    pass

cdef TransportError create_TransportError(unique_ptr[cTTransportException] ex)


cdef object create_py_exception(const cFollyExceptionWrapper& ex, RpcOptions options)


# Base class for all generated exceptions defined in Thrift IDL
cdef class GeneratedError(Error):
    cdef object _fbthrift_data
    cdef _fbthrift_get_field_value(self, int16_t index)
    cdef iobuf.IOBuf _serialize(GeneratedError self, Protocol proto)
    cdef uint32_t _deserialize(GeneratedError self, iobuf.IOBuf buf, Protocol proto) except? 0

# TODO: add a HandlerManager class to wrap functionality below
ctypedef object(*Handler)(const cFollyExceptionWrapper& ex, PyObject* user_data)
cdef void addHandler(Handler handler)
cdef void removeAllHandlers()
cdef object runHandlers(const cFollyExceptionWrapper& ex, RpcOptions options)
