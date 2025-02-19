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
from cpython cimport bool as pbool, int as pint, float as pfloat

import threading
from functools import wraps
from types import MappingProxyType
from thrift.python.types cimport (
    Container as python_Container,
    EnumTypeInfo,
    StructTypeInfo,
    TypeInfoBase,
    Map as python_Map,
)
from thrift.python.types import (
    List as python_List,
    ListTypeInfo,
    MapTypeInfo,
    Set as python_Set,
    SetTypeInfo,
    UnionInfo,
    typeinfo_bool,
    typeinfo_byte,
    typeinfo_i16,
    typeinfo_i32,
    typeinfo_i64,
    typeinfo_double,
    typeinfo_float,
    typeinfo_string,
    typeinfo_binary,
    typeinfo_iobuf,
)
from folly.iobuf import IOBuf
import importlib


cdef threadsafe_cached(fn):
    __cache = {}
    lock = threading.Lock()
    @wraps(fn)
    def wrapper(key):
        # lock free read when present in cache
        if key in __cache:
            return __cache[key]
        with lock:
            # check again if in cache
            if key in __cache:
                return __cache[key]
            # update cache
            result = __cache[key] = fn(key)
            return result
    return wrapper


@threadsafe_cached
def _inspect_impl(cls):
    return cls.__get_reflection__()


def inspect(cls):
    klass = cls if isinstance(cls, type) else type(cls)
    if hasattr(klass, '__get_reflection__'):
        return _inspect_impl(klass)
    if isinstance(cls, python_Container):
        return _fbthrift_python_container_inst(cls)
    if issubclass(klass, python_Container):
        return _fbthrift_python_container_cls(klass)
    raise TypeError(
        f"No reflection information found: '{klass.__module__}.{klass.__name__}'"
    )

def inspectable(cls):
    if not isinstance(cls, type):
        cls = type(cls)
    return (
        hasattr(cls, '__get_reflection__') or 
        issubclass(cls, python_Container)
    )

### Note in thrift-python the class doesn't have any info about the
### the element types, so this is the best we can do.
cdef _fbthrift_python_container_cls(type klass):
    cdef num_type = NumberType.NOT_A_NUMBER
    if issubclass(klass, python_List):
        return ListSpec._fbthrift_create(None, num_type)
    if issubclass(klass, python_Set):
        return SetSpec._fbthrift_create(None, num_type)
    if issubclass(klass, python_Map):
        return MapSpec._fbthrift_create(None, num_type, None, num_type)

cdef _fbthrift_python_container_inst(python_Container inst):
    if isinstance(inst, python_List):
        kls, num_type = spec_from_TypeInfo(inst._fbthrift_val_info)
        return ListSpec._fbthrift_create(kls, num_type)
    if isinstance(inst, python_Set):
        kls, num_type = spec_from_TypeInfo(inst._fbthrift_val_info)
        return SetSpec._fbthrift_create(kls, num_type)
    if isinstance(inst, python_Map):
        val_kls, val_num_type = spec_from_TypeInfo(inst._fbthrift_val_info)
        key_kls, key_num_type = spec_from_TypeInfo(
            (<python_Map>inst)._fbthrift_key_info
        )
        return MapSpec._fbthrift_create(
            key_kls,
            key_num_type,
            val_kls,
            val_num_type,
        )




@cython.auto_pickle(False)
cdef class StructSpec:
    def __cinit__(self, str name, fields, StructType kind, dict annotations = {}):
        self.name = name
        if fields:
            for field in fields:
                Py_INCREF(field)
                self._fields.push_back(<PyObject*>field)
        self.kind = StructType(kind)
        self.annotations = MappingProxyType(annotations)

    @staticmethod
    def _fbthrift_create(str name, StructType kind, dict annotations):
        return StructSpec.__new__(StructSpec, name, None, kind, annotations)

    def __iter__(self):
        yield self.name
        yield self.fields
        yield self.kind
        yield self.annotations

    def __eq__(self, other):
        if not isinstance(other, StructSpec):
            return False
        return tuple(self) == tuple(other)

    def add_field(self, FieldSpec field):
        Py_INCREF(field)
        self._fields.push_back(<PyObject*>field)

    def __dealloc__(self):
        for _field in self._fields:
            Py_XDECREF(_field)

    @property
    def fields(self):
        return tuple(<object>field for field in self._fields)


@cython.auto_pickle(False)
cdef class FieldSpec:
    def __cinit__(
        self,
        int id,
        str name,
        str py_name,
        type,
        NumberType kind,
        Qualifier qualifier,
        default,
        dict annotations = {},
    ):
        self.id = id
        self.name = name
        self.py_name = py_name
        self.type = type
        self.kind = NumberType(kind)
        self.qualifier = Qualifier(qualifier)
        self.default = default
        self.annotations = MappingProxyType(annotations)

    @staticmethod
    def _fbthrift_create(
        int id,
        str name,
        str py_name,
        object type,
        NumberType kind,
        Qualifier qualifier,
        object default,
        dict annotations,
    ):
        return FieldSpec.__new__(FieldSpec, id, name, py_name, type, kind, qualifier, default, annotations)

    def __iter__(self):
        yield self.id
        yield self.name
        yield self.type
        yield self.kind
        yield self.qualifier
        yield self.default
        yield self.annotations

    def __eq__(self, other):
        if not isinstance(other, FieldSpec):
            return False
        return tuple(self) == tuple(other)


@cython.auto_pickle(False)
cdef class ListSpec:
    def __cinit__(self, value, NumberType kind):
        self.value = value
        self.kind = NumberType(kind)

    @staticmethod
    def _fbthrift_create(object value, NumberType kind):
        return ListSpec.__new__(ListSpec, value, kind)

    def __iter__(self):
        yield self.value
        yield self.kind

    def __eq__(self, other):
        if not isinstance(other, ListSpec):
            return False
        return tuple(self) == tuple(other)


@cython.auto_pickle(False)
cdef class SetSpec:
    def __cinit__(self, value, NumberType kind):
        self.value = value
        self.kind = NumberType(kind)

    @staticmethod
    def _fbthrift_create(object value, NumberType kind):
        return SetSpec.__new__(SetSpec, value, kind)

    def __iter__(self):
        yield self.value
        yield self.kind

    def __eq__(self, other):
        if not isinstance(other, SetSpec):
            return False
        return tuple(self) == tuple(other)


@cython.auto_pickle(False)
cdef class MapSpec:
    def __cinit__(
        self,
        key,
        NumberType key_kind,
        value,
        NumberType value_kind,
    ):
        self.key = key
        self.key_kind = NumberType(key_kind)
        self.value = value
        self.value_kind = NumberType(value_kind)

    @staticmethod
    def _fbthrift_create(
        object key,
        NumberType key_kind,
        object value,
        NumberType value_kind,
    ):
        return MapSpec.__new__(MapSpec, key, key_kind, value, value_kind)

    def __iter__(self):
        yield self.key
        yield self.key_kind
        yield self.value
        yield self.value_kind

    def __eq__(self, other):
        if not isinstance(other, MapSpec):
            return False
        return tuple(self) == tuple(other)


@cython.auto_pickle(False)
cdef class InterfaceSpec:
    def __cinit__(self, str name, methods, dict annotations = {}):
        self.name = name
        self._methods = list(methods) if methods else list()
        self.annotations = MappingProxyType(annotations)

    def add_method(self, MethodSpec method):
        self._methods.append(method)

    @property
    def methods(self):
        return tuple(method for method in self._methods)

    def __iter__(self):
        yield self.name
        yield self.methods
        yield self.annotations


    def __eq__(self, other):
        if not isinstance(other, InterfaceSpec):
            return False
        return tuple(self) == tuple(other)


@cython.auto_pickle(False)
cdef class MethodSpec:
    def __cinit__(
        self,
        str name,
        arguments,
        NumberType result_kind,
        result,
        exceptions = (),
        annotations = {}
    ):
        self.name = name
        self.arguments = tuple(arguments)
        self.result_kind = NumberType(result_kind)
        self.result = result
        self.exceptions = tuple(exceptions)
        self.annotations = MappingProxyType(annotations)

    def __iter__(self):
        yield self.name
        yield self.arguments
        yield self.result_kind
        yield self.result
        yield self.exceptions
        yield self.annotations

    def __eq__(self, other):
        if not isinstance(other, MethodSpec):
            return False
        return tuple(self) == tuple(other)


@cython.auto_pickle(False)
cdef class ArgumentSpec:
    def __cinit__(self, str name, NumberType kind, type, dict annotations = {}):
        self.name = name
        self.kind = NumberType(kind)
        self.type = type
        self.annotations = MappingProxyType(annotations)

    def __iter__(self):
        yield self.name
        yield self.kind
        yield self.type
        yield self.annotations

    def __eq__(self, other):
        if not isinstance(other, ArgumentSpec):
            return False
        return tuple(self) == tuple(other)

### This is all for thrift-py3-auto-migrate.
### If we can remove reflection from thrift-py3 usage, we can delete this.


cdef spec_from_TypeInfo(info):
    if isinstance(info, (EnumTypeInfo, StructTypeInfo)):
        kls = info.__reduce__()[1][0]
        return (kls, NumberType.NOT_A_NUMBER)
    if info is typeinfo_bool:
        return (pbool, NumberType.NOT_A_NUMBER)
    if info is typeinfo_byte:
        return (int, NumberType.BYTE)
    if info is typeinfo_i16:
        return (int, NumberType.I16)
    if info is typeinfo_i32:
        return (int, NumberType.I32)
    if info is typeinfo_i64:
        return (int, NumberType.I64)
    if info is typeinfo_double:
        return (int, NumberType.DOUBLE)
    if info is typeinfo_float:
        return (int, NumberType.FLOAT)
    if info is typeinfo_string:
        return (str, NumberType.NOT_A_NUMBER)
    if info is typeinfo_binary:
        return (bytes, NumberType.NOT_A_NUMBER)
    if info is typeinfo_iobuf:
        return (IOBuf, NumberType.NOT_A_NUMBER)
    # note these aren't going to work for recursive calls
    # because the python Container classes themselves don't
    # have the element type info
    if isinstance(info, ListTypeInfo):
        return (python_List, NumberType.NOT_A_NUMBER)
    if isinstance(info, SetTypeInfo):
        return (python_Set, NumberType.NOT_A_NUMBER)
    if isinstance(info, MapTypeInfo):
        return (python_Map, NumberType.NOT_A_NUMBER)
