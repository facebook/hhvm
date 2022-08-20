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
import functools

from cython.operator cimport dereference as deref
from thrift.python.serializer cimport cserialize, cdeserialize
from thrift.python.types cimport StructInfo, createStructTuple, set_struct_field
from libcpp.vector cimport vector
from libcpp.utility cimport move as cmove


cdef class Error(Exception):
    """base class for all thrift exceptions (TException)"""
    pass


cdef class ApplicationError(Error):
    """All Application Level Errors (TApplicationException)"""

    def __init__(ApplicationError self, ApplicationErrorType type, str message):
        assert message, "message is empty"
        super().__init__(type, message)

    @property
    def type(self):
        return self.args[0]

    @property
    def message(self):
        return self.args[1]


cdef ApplicationError create_ApplicationError(unique_ptr[cTApplicationException] ex):
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


cdef TransportError create_TransportError(unique_ptr[cTTransportException] ex):
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

    pyex = create_ApplicationError(try_make_unique_exception[cTApplicationException](ex))
    if pyex:
        return pyex

    pyex = create_TransportError(try_make_unique_exception[cTTransportException](ex))
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
    return functools.cached_property(lambda self: (<GeneratedError>self)._fbthrift_get_field_value(i))


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
        self._fbthrift_data = createStructTuple(
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
        # for builtin.BaseException
        super().__init__(*(value for _, value in self))

    def __iter__(self):
        cdef StructInfo info = self._fbthrift_struct_info
        for name in info.name_to_index:
            yield name, getattr(self, name)

    cdef iobuf.IOBuf _serialize(self, Protocol proto):
        cdef StructInfo info = self._fbthrift_struct_info
        return iobuf.from_unique_ptr(
            cmove(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _deserialize(self, iobuf.IOBuf buf, Protocol proto) except? 0:
        cdef StructInfo info = self._fbthrift_struct_info
        return cdeserialize(deref(info.cpp_obj), buf._this, self._fbthrift_data, proto)

    cdef _fbthrift_get_field_value(self, int16_t index):
        data = self._fbthrift_data[index + 1]
        if data is None:
            return
        cdef StructInfo info = self._fbthrift_struct_info
        return info.type_infos[index].to_python_value(data)

    def __copy__(GeneratedError self):
        # deep copy the instance
        return self._fbthrift_create(copy.deepcopy(self._fbthrift_data))

    def __lt__(self, other):
        if type(self) != type(other):
            return NotImplemented
        ret = (<GeneratedError>self)._fbthrift_data[1:] < (<GeneratedError>other)._fbthrift_data[1:]
        return ret

    def __le__(self, other):
        return self < other or self == other

    def __eq__(GeneratedError self, other):
        if type(other) != type(self):
            return False
        cdef StructInfo info = self._fbthrift_struct_info
        for name in info.name_to_index:
            if getattr(self, name) != getattr(other, name):
                return False
        return True

    def __hash__(GeneratedError self):
        return hash(self._fbthrift_data)

    @classmethod
    def _fbthrift_create(cls, data):
        cdef GeneratedError inst = cls.__new__(cls)
        inst._fbthrift_data = data
        return inst

    @staticmethod
    def __get_thrift_uri__():
        return NotImplementedError()
