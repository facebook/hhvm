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
from thrift.python.exceptions cimport (
    create_ApplicationError,
    create_Error,
    create_LibraryError,
    create_ProtocolError,
    create_TransportError,
    cTApplicationException,
    cTException,
    cTLibraryException,
    cTProtocolException,
    cTTransportException,
)
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    Error,
    LibraryError,
    ProtocolError,
    ProtocolErrorType,
    TransportError,
    TransportErrorType,
    TransportOptions,
)


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
