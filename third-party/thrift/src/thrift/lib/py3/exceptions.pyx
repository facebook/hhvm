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

from cython.operator cimport dereference as deref
from cpython.exc cimport PyErr_Occurred
from cpython.object cimport Py_LT, Py_EQ, Py_NE
from libcpp.vector cimport vector
from thrift.python.common import RpcOptions
from thrift.python.exceptions import Error, LibraryError

from enum import Enum, Flag
import itertools

class TransportErrorType(Enum):
    UNKNOWN = cTTransportExceptionType__UNKNOWN
    NOT_OPEN = cTTransportExceptionType__NOT_OPEN
    ALREADY_OPEN = cTTransportExceptionType__ALREADY_OPEN
    TIMED_OUT = cTTransportExceptionType__TIMED_OUT
    END_OF_FILE = cTTransportExceptionType__END_OF_FILE
    INTERRUPTED = cTTransportExceptionType__INTERRUPTED
    BAD_ARGS = cTTransportExceptionType__BAD_ARGS
    CORRUPTED_DATA = cTTransportExceptionType__CORRUPTED_DATA
    INTERNAL_ERROR = cTTransportExceptionType__INTERNAL_ERROR
    NOT_SUPPORTED = cTTransportExceptionType__NOT_SUPPORTED
    INVALID_STATE = cTTransportExceptionType__INVALID_STATE
    INVALID_FRAME_SIZE = cTTransportExceptionType__INVALID_FRAME_SIZE
    SSL_ERROR = cTTransportExceptionType__SSL_ERROR
    COULD_NOT_BIND = cTTransportExceptionType__COULD_NOT_BIND
    NETWORK_ERROR = cTTransportExceptionType__NETWORK_ERROR


class TransportOptions(Flag):
    CHANNEL_IS_VALID = cTTransportExceptionOptions__CHANNEL_IS_VALID


class ApplicationErrorType(Enum):
    UNKNOWN = cTApplicationExceptionType__UNKNOWN
    UNKNOWN_METHOD = cTApplicationExceptionType__UNKNOWN_METHOD
    INVALID_MESSAGE_TYPE = cTApplicationExceptionType__INVALID_MESSAGE_TYPE
    WRONG_METHOD_NAME = cTApplicationExceptionType__WRONG_METHOD_NAME
    BAD_SEQUENCE_ID = cTApplicationExceptionType__BAD_SEQUENCE_ID
    MISSING_RESULT = cTApplicationExceptionType__MISSING_RESULT
    INTERNAL_ERROR = cTApplicationExceptionType__INTERNAL_ERROR
    PROTOCOL_ERROR = cTApplicationExceptionType__PROTOCOL_ERROR
    INVALID_TRANSFORM = cTApplicationExceptionType__INVALID_TRANSFORM
    INVALID_PROTOCOL = cTApplicationExceptionType__INVALID_PROTOCOL
    UNSUPPORTED_CLIENT_TYPE = cTApplicationExceptionType__UNSUPPORTED_CLIENT_TYPE
    LOADSHEDDING = cTApplicationExceptionType__LOADSHEDDING
    TIMEOUT = cTApplicationExceptionType__TIMEOUT
    INJECTED_FAILURE = cTApplicationExceptionType__INJECTED_FAILURE


class ProtocolErrorType(Enum):
    UNKNOWN = cTProtocolExceptionType__UNKNOWN
    INVALID_DATA = cTProtocolExceptionType__INVALID_DATA
    NEGATIVE_SIZE = cTProtocolExceptionType__NEGATIVE_SIZE
    SIZE_LIMIT = cTProtocolExceptionType__SIZE_LIMIT
    BAD_VERSION = cTProtocolExceptionType__BAD_VERSION
    NOT_IMPLEMENTED = cTProtocolExceptionType__NOT_IMPLEMENTED
    MISSING_REQUIRED_FIELD = cTProtocolExceptionType__MISSING_REQUIRED_FIELD




cdef create_Error(const cTException* ex):
    if not ex:
        return
    message = (<bytes>deref(ex).what()).decode('utf-8')
    inst = <BaseError>BaseError.__new__(BaseError, message)
    return inst


cdef class GeneratedError(BaseError):
    """This is the base class for all Generated Thrift Exceptions"""

    def __init__(self, *args, **kwargs):
        names_iter = self.__iter_names()
        for idx, value in enumerate(args):
            try:
                name = next(names_iter)
            except StopIteration:
                raise TypeError(f"{type(self).__name__}() only takes {idx} arguments")
            else:
                self._fbthrift_set_field(name, value)
        for name in names_iter:
            value = kwargs.pop(name, None)
            if value is not None:
                self._fbthrift_set_field(name, value)
        if kwargs:  # still something left
            raise TypeError(f"{type(self).__name__}() found duplicate/undefined arguments {repr(kwargs)}")
        super().__init__(*(value for _, value in self))

    cdef IOBuf _fbthrift_serialize(self, Protocol proto):
        return IOBuf(b'')

    cdef uint32_t _fbthrift_deserialize(self, const cIOBuf* buf, Protocol proto) except? 0:
        return 0

    cdef object _fbthrift_isset(self):
        raise TypeError(f"{type(self)} does not have concept of isset")

    cdef object _fbthrift_cmp_sametype(self, other, int op):
        if not isinstance(other, type(self)):
            if op == Py_EQ:  # different types are never equal
                return False
            if op == Py_NE:  # different types are always notequal
                return True
            return NotImplemented
        # otherwise returns None

    cdef void _fbthrift_set_field(self, str name, object value) except *:
        pass

    @classmethod
    def _fbthrift_get_field_name_by_index(cls, idx):
        raise NotImplementedError()

    def __repr__(self):
        fields = ", ".join(f"{name}={repr(value)}" for name, value in self)
        return f"{type(self).__name__}({fields})"

    @classmethod
    def _fbthrift_get_struct_size(cls):
        return 0

    def __iter_names(self):
        for i in range(self._fbthrift_get_struct_size()):
            yield self._fbthrift_get_field_name_by_index(i)

    def __iter__(self):
        for name in self.__iter_names():
            yield name, getattr(self, name)

    def __hash__(self):
        if not self._fbthrift_hash:
            value_tuple = tuple(v for _, v in self)
            self._fbthrift_hash = hash(
                value_tuple if value_tuple
                else type(self)  # Hash the class there are no fields
            )
        return self._fbthrift_hash

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        raise NotImplementedError()


cdef class ApplicationError(BaseError):
    """All Application Level Errors (TApplicationException)"""

    def __init__(ApplicationError self, type, str message):
        assert type in ApplicationErrorType, f"{type} not in ApplicationErrorType"
        assert message, "message is empty"
        super().__init__(type, message)

    @property
    def type(self):
        return self.args[0]

    @property
    def message(self):
        return self.args[1]


cdef create_ApplicationError(const cTApplicationException* ex):
    if not ex:
        return
    type = ApplicationErrorType(deref(ex).getType())
    message = (<bytes>deref(ex).what()).decode('utf-8')
    # Strip out the message prefix its verbose for python
    message = message[message.startswith('TApplicationException: ')*23:]
    inst = <ApplicationError>ApplicationError.__new__(
        ApplicationError,
        type,
        message,
    )
    return inst


cdef create_LibraryError(const cTLibraryException* ex):
    if not ex:
        return
    message = (<bytes>deref(ex).what()).decode('utf-8')
    inst = <cLibraryError>cLibraryError.__new__(cLibraryError, message)
    return inst


cdef class ProtocolError(cLibraryError):
    """Equivalent of a C++ TProtocolException"""
    def __init__(self, type, str message):
        super().__init__(type, message)

    @property
    def type(self):
        return self.args[0]

    @property
    def message(self):
        return self.args[1]


cdef create_ProtocolError(const cTProtocolException* ex):
    if not ex:
        return
    type = ProtocolErrorType(deref(ex).getType())
    message = (<bytes>deref(ex).what()).decode('utf-8')
    inst = <ProtocolError>ProtocolError.__new__(ProtocolError, type, message)
    return inst


cdef class TransportError(cLibraryError):
    """All Transport Level Errors (TTransportException)"""

    def __init__(self, type, str message, int errno, options, *args):
        super().__init__(type, message, errno, options, *args)

    @property
    def type(self):
        return self.args[0]

    @property
    def message(self):
        return self.args[1]

    @property
    def errno(self):
        return self.args[2]

    @property
    def options(self):
        return self.args[3]


cdef create_TransportError(const cTTransportException* ex):
    if not ex:
        return
    type = TransportErrorType(deref(ex).getType())
    options = TransportOptions(deref(ex).getOptions())
    Errno = deref(ex).getErrno()
    message = (<bytes>deref(ex).what()).decode('utf-8')
    # Strip off the c++ message prefix
    message = message[message.startswith('TTransportException: ')*21:]
    inst = <TransportError>TransportError.__new__(
        TransportError,
        type,
        message,
        Errno,
        options,
    )
    return inst


# Our Registry
cdef vector[Handler] handlers


cdef void addHandler(Handler handler):
    handlers.push_back(handler)


cdef object runHandlers(const cFollyExceptionWrapper& ex, RpcOptions options):
    for handler in handlers:
        pyex = handler(ex, <PyObject *> options)
        if pyex:
            return pyex


cdef object create_py_exception(const cFollyExceptionWrapper& ex, RpcOptions options):
    # This will raise an exception if a handler raised one
    pyex = runHandlers(ex, options)
    if pyex:
        return pyex

    pyex = create_ApplicationError(ex.get_exception[cTApplicationException]())
    if pyex:
        return pyex

    pyex = create_TransportError(ex.get_exception[cTTransportException]())
    if pyex:
        return pyex

    pyex = create_ProtocolError(ex.get_exception[cTProtocolException]())
    if pyex:
        return pyex

    pyex = create_LibraryError(ex.get_exception[cTLibraryException]())
    if pyex:
        return pyex

    pyex = create_Error(ex.get_exception[cTException]())
    if pyex:
        return pyex

    try:
        # No clue what this is just throw it and let the default cython logic takeover
        ex.throw_exception()
    except Exception as pyex:
        # We don't try to shorten the traceback because this is Unknown
        # it will be helpful to know the most information possible.
        return pyex
