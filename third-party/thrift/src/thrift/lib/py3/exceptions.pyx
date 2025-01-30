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

cimport cython
from cpython.object cimport Py_LT, Py_EQ, Py_NE
from thrift.python.common import RpcOptions
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    GeneratedError as _fbthrift_python_GeneratedError,
    Error,
    LibraryError,
    ProtocolError,
    ProtocolErrorType,
    TransportError,
    TransportErrorType,
    TransportOptions,
)

@cython.internal
@cython.auto_pickle(False)
cdef class ErrorMeta(type):
    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_GeneratedError):
            return inst._fbthrift_auto_migrate_enabled()
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_GeneratedError):
            return sub._fbthrift_auto_migrate_enabled()
        return super().__subclasscheck__(sub)

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

SetMetaClass(<PyTypeObject*> GeneratedError, <PyTypeObject*> ErrorMeta)
