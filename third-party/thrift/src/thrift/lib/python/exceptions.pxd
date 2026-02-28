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
from thrift.python.common cimport RpcOptions

from thrift.python.protocol cimport Protocol


cdef extern from * namespace "std":
    cdef cppclass cException "std::exception":
        const char* what() nogil


cdef extern from "thrift/lib/cpp/Thrift.h" namespace "apache::thrift":
    cdef cppclass cTException "apache::thrift::TException"(cException):
        pass

    cdef cppclass cTLibraryException "apache::thrift::TLibraryException"(cTException):
        pass


cdef extern from "thrift/lib/cpp/TApplicationException.h" namespace "apache::thrift":

    enum cTApplicationExceptionType "apache::thrift::TApplicationException::TApplicationExceptionType":
        cTApplicationExceptionType__UNKNOWN "apache::thrift::TApplicationException::UNKNOWN"
        cTApplicationExceptionType__UNKNOWN_METHOD "apache::thrift::TApplicationException::UNKNOWN_METHOD"
        cTApplicationExceptionType__INVALID_MESSAGE_TYPE "apache::thrift::TApplicationException::INVALID_MESSAGE_TYPE"
        cTApplicationExceptionType__WRONG_METHOD_NAME "apache::thrift::TApplicationException::WRONG_METHOD_NAME"
        cTApplicationExceptionType__BAD_SEQUENCE_ID "apache::thrift::TApplicationException::BAD_SEQUENCE_ID"
        cTApplicationExceptionType__MISSING_RESULT "apache::thrift::TApplicationException::MISSING_RESULT"
        cTApplicationExceptionType__INTERNAL_ERROR "apache::thrift::TApplicationException::INTERNAL_ERROR"
        cTApplicationExceptionType__PROTOCOL_ERROR "apache::thrift::TApplicationException::PROTOCOL_ERROR"
        cTApplicationExceptionType__INVALID_TRANSFORM "apache::thrift::TApplicationException::INVALID_TRANSFORM"
        cTApplicationExceptionType__INVALID_PROTOCOL "apache::thrift::TApplicationException::INVALID_PROTOCOL"
        cTApplicationExceptionType__UNSUPPORTED_CLIENT_TYPE "apache::thrift::TApplicationException::UNSUPPORTED_CLIENT_TYPE"
        cTApplicationExceptionType__LOADSHEDDING "apache::thrift::TApplicationException::LOADSHEDDING"
        cTApplicationExceptionType__TIMEOUT "apache::thrift::TApplicationException::TIMEOUT"
        cTApplicationExceptionType__INJECTED_FAILURE "apache::thrift::TApplicationException::INJECTED_FAILURE"

    cdef cppclass cTApplicationException "apache::thrift::TApplicationException"(cTException):
        cTApplicationException(cTApplicationExceptionType type, const string& message) except + nogil
        cTApplicationExceptionType getType() noexcept nogil


cdef extern from "thrift/lib/cpp/transport/TTransportException.h" namespace "apache::thrift::transport":

    enum cTTransportExceptionType "apache::thrift::transport::TTransportException::TTransportExceptionType":
        cTTransportExceptionType__UNKNOWN "apache::thrift::transport::TTransportException::UNKNOWN"
        cTTransportExceptionType__NOT_OPEN "apache::thrift::transport::TTransportException::NOT_OPEN"
        cTTransportExceptionType__ALREADY_OPEN "apache::thrift::transport::TTransportException::ALREADY_OPEN"
        cTTransportExceptionType__TIMED_OUT "apache::thrift::transport::TTransportException::TIMED_OUT"
        cTTransportExceptionType__END_OF_FILE "apache::thrift::transport::TTransportException::END_OF_FILE"
        cTTransportExceptionType__INTERRUPTED "apache::thrift::transport::TTransportException::INTERRUPTED"
        cTTransportExceptionType__BAD_ARGS "apache::thrift::transport::TTransportException::BAD_ARGS"
        cTTransportExceptionType__CORRUPTED_DATA "apache::thrift::transport::TTransportException::CORRUPTED_DATA"
        cTTransportExceptionType__INTERNAL_ERROR "apache::thrift::transport::TTransportException::INTERNAL_ERROR"
        cTTransportExceptionType__NOT_SUPPORTED "apache::thrift::transport::TTransportException::NOT_SUPPORTED"
        cTTransportExceptionType__INVALID_STATE "apache::thrift::transport::TTransportException::INVALID_STATE"
        cTTransportExceptionType__INVALID_FRAME_SIZE "apache::thrift::transport::TTransportException::INVALID_FRAME_SIZE"
        cTTransportExceptionType__SSL_ERROR "apache::thrift::transport::TTransportException::SSL_ERROR"
        cTTransportExceptionType__COULD_NOT_BIND "apache::thrift::transport::TTransportException::COULD_NOT_BIND"
        cTTransportExceptionType__NETWORK_ERROR "apache::thrift::transport::TTransportException::NETWORK_ERROR"
        cTTransportExceptionType__EARLY_DATA_REJECTED "apache::thrift::transport::TTransportException::EARLY_DATA_REJECTED"
        cTTransportExceptionType__STREAMING_CONTRACT_VIOLATION "apache::thrift::transport::TTransportException::STREAMING_CONTRACT_VIOLATION"
        cTTransportExceptionType__INVALID_SETUP "apache::thrift::transport::TTransportException::INVALID_SETUP"

    enum cTTransportExceptionOptions "apache::thrift::transport::TTransportException::Options":
        cTTransportExceptionOptions__CHANNEL_IS_VALID "apache::thrift::transport::TTransportException::CHANNEL_IS_VALID"

    cdef cppclass cTTransportException "apache::thrift::transport::TTransportException"(cTLibraryException):
        int getOptions() noexcept
        cTTransportExceptionType getType() noexcept
        int getErrno() noexcept

cdef extern from "thrift/lib/cpp/protocol/TProtocolException.h":
    enum cTProtocolExceptionType "apache::thrift::protocol::TProtocolException::TProtocolExceptionType":
        cTProtocolExceptionType__UNKNOWN "apache::thrift::protocol::TProtocolException::UNKNOWN"
        cTProtocolExceptionType__INVALID_DATA "apache::thrift::protocol::TProtocolException::INVALID_DATA"
        cTProtocolExceptionType__NEGATIVE_SIZE "apache::thrift::protocol::TProtocolException::NEGATIVE_SIZE"
        cTProtocolExceptionType__SIZE_LIMIT "apache::thrift::protocol::TProtocolException::SIZE_LIMIT"
        cTProtocolExceptionType__BAD_VERSION "apache::thrift::protocol::TProtocolException::BAD_VERSION"
        cTProtocolExceptionType__NOT_IMPLEMENTED "apache::thrift::protocol::TProtocolException::NOT_IMPLEMENTED"
        cTProtocolExceptionType__MISSING_REQUIRED_FIELD "apache::thrift::protocol::TProtocolException::MISSING_REQUIRED_FIELD"

    cdef cppclass cTProtocolException "apache::thrift::protocol::TProtocolException"(cTLibraryException):
        cTProtocolExceptionType getType()


cdef class Error(Exception):
    """base class for all Thrift exceptions"""
    pass


cdef class LibraryError(Error):
    pass

cdef class ApplicationError(Error):
    pass

cdef ApplicationError create_ApplicationError(const cTApplicationException* ex)


cdef class TransportError(LibraryError):
    pass

cdef class ProtocolError(LibraryError):
    pass


cdef object create_py_exception(const cFollyExceptionWrapper& ex, RpcOptions options)


# Base class for all generated exceptions defined in Thrift IDL
cdef class GeneratedError(Error):
    cdef object _fbthrift_data
    cdef int _fbthrift_populate_field_values(self) except -1
    cdef iobuf.IOBuf _serialize(GeneratedError self, Protocol proto)
    cdef uint32_t _deserialize(GeneratedError self, iobuf.IOBuf buf, Protocol proto) except? 0

# TODO: T134656128
ctypedef object(*Handler)(const cFollyExceptionWrapper& ex, PyObject* user_data)
cdef void addHandler(Handler handler)
cdef void removeAllHandlers()
cdef object runHandlers(const cFollyExceptionWrapper& ex, RpcOptions options)
