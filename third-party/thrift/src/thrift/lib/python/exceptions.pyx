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

from cython.operator cimport dereference as deref
from thrift.python.serializer cimport cserialize, cdeserialize
from thrift.python.types cimport StructInfo, createStructTupleWithDefaultValues, set_struct_field
from libcpp.vector cimport vector
from libcpp.utility cimport move as cmove


cdef class Error(Exception):
    """base class for all thrift exceptions (TException)"""
    pass


cdef class ApplicationError(Error):
    """All Application Level Errors (TApplicationException)"""

    def __init__(ApplicationError self, ApplicationErrorType type, str message):
        assert message, "message is empty"

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
    pass


cdef class TransportError(LibraryError):
    """All Transport Level Errors (TTransportException)"""

    def __init__(TransportError self, TransportErrorType type, str message, int errno, int options):
        super().__init__(type, message, errno, options)

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
    options = deref(ex).getOptions()
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
        fields = dct.pop('_fbthrift_SPEC')
        num_fields = len(fields)
        dct["_fbthrift_struct_info"] = StructInfo(name, fields)
        for i, f in enumerate(fields):
            dct[f[2]] = make_fget_error(i)
        return super().__new__(cls, name, (GeneratedError,), dct)

    def _fbthrift_fill_spec(cls):
        (<StructInfo>cls._fbthrift_struct_info).fill()

    def _fbthrift_store_field_values(cls):
        (<StructInfo>cls._fbthrift_struct_info).store_field_values()


cdef class GeneratedError(Error):
    def __cinit__(self):
        cdef StructInfo info = self._fbthrift_struct_info
        self._fbthrift_data = createStructTupleWithDefaultValues(
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

    cdef void _fbthrift_populate_field_values(self):
        cdef StructInfo info = self._fbthrift_struct_info
        args = []
        for index, type_info in enumerate(info.type_infos):
            data = self._fbthrift_data[index + 1]
            args.append(None if data is None else type_info.to_python_value(data))
        self.args = args

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
    def _fbthrift_create(cls, data):
        cdef GeneratedError inst = cls.__new__(cls)
        inst._fbthrift_data = data
        inst._fbthrift_populate_field_values()
        return inst

    @staticmethod
    def __get_thrift_uri__():
        return NotImplementedError()
