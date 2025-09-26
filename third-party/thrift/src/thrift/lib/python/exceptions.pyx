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

import copy
from enum import Enum, Flag

from cython.operator cimport dereference as deref
from thrift.python.serializer cimport cserialize, cdeserialize
from thrift.python.types cimport StructInfo, createImmutableStructTupleWithDefaultValues, set_struct_field
from libcpp.vector cimport vector
from libcpp.utility cimport move as cmove


cdef class Error(Exception):
    """base class for all thrift exceptions (TException)"""
    def __init__(self, *args):
        super().__init__(*args)

cdef create_Error(const cTException* ex):
    if not ex:
        return
    message = (<bytes>(deref(ex).what())).decode('utf-8')
    inst = <Error>Error.__new__(Error, message)
    return inst


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


cdef class ApplicationError(Error):
    """All Application Level Errors (TApplicationException)"""

    def __init__(ApplicationError self, type, str message):
        assert message, "message is empty"
        if not isinstance(type, ApplicationErrorType):
            try:
                type = ApplicationErrorType(int(type))
            except ValueError as e:
                raise TypeError(f"Invalid ApplicationErrorType {type}") from e
        super().__init__(type, message)

    @property
    def type(self):
        return self.args[0]

    @property
    def message(self):
        return self.args[1]


cdef ApplicationError create_ApplicationError(const cTApplicationException* ex):
    if not ex:
        return
    type = ApplicationErrorType(deref(ex).getType())
    message = (<bytes>deref(ex).what()).decode('utf-8')
    # Strip out the message prefix its verbose for python
    message = message[message.startswith('TApplicationException: ')*23:]
    return <ApplicationError>ApplicationError.__new__(
        ApplicationError,
        type,
        message,
    )

cdef class LibraryError(Error):
    """Equivalent of a C++ TLibraryException"""
    def __init__(self, *args):
        super().__init__(*args)


cdef object create_LibraryError(const cTLibraryException* ex):
    if not ex:
        return
    message = (<bytes>deref(ex).what()).decode('utf-8')
    inst = <LibraryError>LibraryError.__new__(LibraryError, message)
    return inst

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
    EARLY_DATA_REJECTED = cTTransportExceptionType__EARLY_DATA_REJECTED
    STREAMING_CONTRACT_VIOLATION = cTTransportExceptionType__STREAMING_CONTRACT_VIOLATION
    INVALID_SETUP = cTTransportExceptionType__INVALID_SETUP


class TransportOptions(Flag):
    CHANNEL_IS_VALID = cTTransportExceptionOptions__CHANNEL_IS_VALID


cdef class TransportError(LibraryError):
    """All Transport Level Errors (TTransportException)"""

    def __init__(TransportError self, type, str message, int errno, options, *args):
        if type is None:
            type = TransportErrorType.UNKNOWN
        elif not isinstance(type, TransportErrorType):
            try:
                type = TransportErrorType(type)
            except ValueError as e:
                raise TypeError(f"Invalid TransportErrorType {type}") from e
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


class ProtocolErrorType(Enum):
    UNKNOWN = cTProtocolExceptionType__UNKNOWN
    INVALID_DATA = cTProtocolExceptionType__INVALID_DATA
    NEGATIVE_SIZE = cTProtocolExceptionType__NEGATIVE_SIZE
    SIZE_LIMIT = cTProtocolExceptionType__SIZE_LIMIT
    BAD_VERSION = cTProtocolExceptionType__BAD_VERSION
    NOT_IMPLEMENTED = cTProtocolExceptionType__NOT_IMPLEMENTED
    MISSING_REQUIRED_FIELD = cTProtocolExceptionType__MISSING_REQUIRED_FIELD


cdef class ProtocolError(LibraryError):
    """Equivalent of a C++ TProtocolException"""
    def __init__(self, type, str message):
        super().__init__(type, message)

    @property
    def type(self):
        return self.args[0]

    @property
    def message(self):
        return self.args[1]


cdef ProtocolError create_ProtocolError(const cTProtocolException* ex):
    if not ex:
        return
    type = ProtocolErrorType(deref(ex).getType())
    message = (<bytes>deref(ex).what()).decode('utf-8')
    inst = <ProtocolError>ProtocolError.__new__(ProtocolError, type, message)
    return inst


cdef vector[Handler] handlers


cdef void addHandler(Handler handler):
    handlers.push_back(handler)


cdef void removeAllHandlers():
    handlers.clear()


cdef object runHandlers(const cFollyExceptionWrapper& ex, RpcOptions options):
    for handler in handlers:
        pyex = handler(ex, <PyObject *> options)
        if pyex:
            return pyex


cdef TransportError create_TransportError(const cTTransportException* ex):
    if not ex:
        return
    type = TransportErrorType(deref(ex).getType())
    message = (<bytes>deref(ex).what()).decode('utf-8')
    # Strip off the c++ message prefix
    message = message[message.startswith('TTransportException: ')*21:]
    errno = deref(ex).getErrno()
    options = TransportOptions(deref(ex).getOptions())
    return <TransportError>TransportError.__new__(
        TransportError,
        type,
        message,
        errno,
        options,
    )


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


cdef make_fget_error(i):
    return property(lambda self: self.args[i])


class GeneratedErrorMeta(type):
    def __new__(cls, name, bases, dct):
        for base in bases:
            if getattr(base, '_fbthrift_allow_inheritance_DO_NOT_USE', False):
                return super().__new__(cls, name, bases, dct)
            raise TypeError(
                f"Inheritance from generated thrift exception {name} is deprecated."
            )
        fields = dct.pop('_fbthrift_SPEC', ())
        num_fields = len(fields)
        dct["_fbthrift_struct_info"] = StructInfo(name, fields)
        for i, f in enumerate(fields):
            dct[f.py_name] = make_fget_error(i)
        all_bases = bases if bases else (GeneratedError,) 
        if "_fbthrift_abstract_base_class" in dct:
            all_bases += (dct.pop("_fbthrift_abstract_base_class"),)
        return super().__new__(cls, name, all_bases, dct)

    def _fbthrift_fill_spec(cls):
        (<StructInfo>cls._fbthrift_struct_info)._fill_struct_info()

    def _fbthrift_store_field_values(cls):
        (<StructInfo>cls._fbthrift_struct_info)._initialize_default_values()


cdef class GeneratedError(Error):
    def __cinit__(self):
        cdef StructInfo info = self._fbthrift_struct_info
        self._fbthrift_data = createImmutableStructTupleWithDefaultValues(
            info.cpp_obj.get().getStructInfo()
        )

    def __init__(self, *args, **kwargs):
        cdef StructInfo info = self._fbthrift_struct_info
        names_iter = iter(info.name_to_index)
        for idx, value in enumerate(args):
            try:
                name = next(names_iter)
            except StopIteration:
                raise TypeError(f"{type(self).__name__}() only takes {idx} arguments")
            if name in kwargs:
                raise TypeError(f"__init__() got multiple values for argument '{name}'")
            kwargs[name] = value

        for name, value in kwargs.items():
            index = info.name_to_index.get(name)
            if index is None:
                raise TypeError(f"__init__() got an unexpected keyword argument '{name}'")
            if value is None:
                continue
            set_struct_field(
                self._fbthrift_data,
                index,
                info.type_infos[index].to_internal_data(value),
            )
        self._fbthrift_populate_field_values()

    @staticmethod
    def from_python(obj: GeneratedError) -> GeneratedError:
        if not isinstance(obj, GeneratedError):
            raise TypeError(f'value {obj} expected to be a thrift-python Exception, was actually of type ' f'{type(obj)}')
        return obj


    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

    def __iter__(self):
        cdef StructInfo info = self._fbthrift_struct_info
        for name, index in info.name_to_index.items():
            yield name, self.args[index]

    cdef iobuf.IOBuf _serialize(self, Protocol proto):
        cdef StructInfo info = self._fbthrift_struct_info
        return iobuf.from_unique_ptr(
            cmove(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _deserialize(self, iobuf.IOBuf buf, Protocol proto) except? 0:
        cdef StructInfo info = self._fbthrift_struct_info
        cdef uint32_t size = cdeserialize(deref(info.cpp_obj), buf._this, self._fbthrift_data, proto)
        self._fbthrift_populate_field_values()
        return size

    cdef int _fbthrift_populate_field_values(self) except -1:
        cdef StructInfo info = self._fbthrift_struct_info
        args = []
        for index, type_info in enumerate(info.type_infos):
            data = self._fbthrift_data[index + 1]
            args.append(None if data is None else type_info.to_python_value(data))
        self.args = args
        return 0

    def __repr__(self):
        fields = ", ".join(f"{name}={repr(value)}" for name, value in self)
        return f"{type(self).__name__}({fields})"

    def __copy__(GeneratedError self):
        return self

    def __deepcopy__(GeneratedError self, _memo):
        return self

    def __lt__(self, other):
        if type(self) != type(other):
            return NotImplemented
        for name, value in self:
            other_value = getattr(other, name)
            if value == other_value:
                continue
            return value < other_value
        return False

    def __le__(self, other):
        if type(self) != type(other):
            return NotImplemented
        for name, value in self:
            other_value = getattr(other, name)
            if value == other_value:
                continue
            return value < other_value
        return True

    def __eq__(GeneratedError self, other):
        if type(other) != type(self):
            return False
        for name, value in self:
            if value != getattr(other, name):
                return False
        return True

    def __hash__(GeneratedError self):
        value_tuple = tuple(v for _, v in self)
        return hash(value_tuple if value_tuple else type(self))

    @classmethod
    def _fbthrift_from_internal_data(cls, data):
        cdef GeneratedError inst = cls.__new__(cls)
        inst._fbthrift_data = data
        inst._fbthrift_populate_field_values()
        return inst

    @staticmethod
    def __get_thrift_uri__():
        return NotImplementedError()
