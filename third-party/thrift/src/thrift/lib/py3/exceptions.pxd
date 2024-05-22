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

# distutils: language = c++

from libcpp.string cimport string
from cpython.ref cimport PyObject
from folly cimport cFollyExceptionWrapper
from folly.iobuf cimport IOBuf, cIOBuf
from libc.stdint cimport uint32_t
from libcpp.memory cimport shared_ptr
from thrift.python.common cimport RpcOptions, cThriftMetadata
from thrift.py3.std_libcpp cimport string_view, sv_to_str
from thrift.python.exceptions cimport Error as BaseError, LibraryError as cLibraryError
from thrift.python.protocol cimport Protocol

cdef extern from * namespace "std":
    cdef cppclass cException "std::exception":
        const char* what() nogil

cdef extern from "thrift/lib/cpp/Thrift.h" namespace "apache::thrift":
    cdef cppclass cTException "apache::thrift::TException"(cException):
        pass

    cdef cppclass cTLibraryException "apache::thrift::TLibraryException"(cTException):
        pass

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


cdef extern from "thrift/lib/cpp/transport/TTransportException.h" \
        namespace "apache::thrift::transport":

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

    enum cTTransportExceptionOptions "apache::thrift::transport::TTransportException::Options":
        cTTransportExceptionOptions__CHANNEL_IS_VALID "apache::thrift::transport::TTransportException::CHANNEL_IS_VALID"

    cdef cppclass cTTransportException "apache::thrift::transport::TTransportException"(cTLibraryException):
        ## No need to instance from Python
        int getOptions()
        cTTransportExceptionType getType()
        int getErrno()

cdef extern from "Python.h":
    ctypedef extern class builtins.Exception[object PyBaseExceptionObject]:
        pass


cdef extern from "thrift/lib/py3/exceptions.h" namespace "::thrift::py3::exception":
    cdef shared_ptr[T] try_make_shared_exception[T](
        const cFollyExceptionWrapper& excepton)


ctypedef object(*Handler)(const cFollyExceptionWrapper& ex, PyObject* user_data)
cdef void addHandler(Handler handler)
cdef object runHandlers(const cFollyExceptionWrapper& ex, RpcOptions options)
cdef object create_py_exception(const cFollyExceptionWrapper& ex, RpcOptions options)

# cdef Inheritence sucks in cython
cdef object create_Error(const cTException* ex)
cdef object create_LibraryError(const cTLibraryException* ex)
cdef object create_TransportError(const cTTransportException* ex)
cdef object create_ProtocolError(const cTProtocolException* ex)


cdef class ProtocolError(cLibraryError):
    pass

cdef class TransportError(cLibraryError):
    pass

cdef class GeneratedError(BaseError):
    cdef object __weakref__
    cdef object _fbthrift_hash
    cdef IOBuf _fbthrift_serialize(self, Protocol proto)
    cdef uint32_t _fbthrift_deserialize(self, const cIOBuf* buf, Protocol proto) except? 0
    cdef object _fbthrift_isset(self)
    cdef object _fbthrift_cmp_sametype(self, other, int op)
    cdef void _fbthrift_set_field(self, str name, object value) except *
