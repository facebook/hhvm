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

from types import MappingProxyType

from cpython.ref cimport PyObject, Py_INCREF, Py_XDECREF
from libcpp.vector cimport vector as cvector

cpdef enum NumberType:
    NOT_A_NUMBER = 0
    BYTE = 1
    I08 = 1
    I16 = 2
    I32 = 3
    I64 = 4
    FLOAT = 5
    DOUBLE = 6


cpdef enum Qualifier:
    UNQUALIFIED = 1
    REQUIRED = 2
    OPTIONAL = 3


cpdef enum StructType:
  STRUCT = 1
  UNION = 2
  EXCEPTION = 3


cdef class StructSpec:
    cdef readonly str name
    cdef cvector[PyObject*] _fields
    cdef readonly object kind
    cdef readonly object annotations

    @staticmethod
    cdef _fbthrift_create(str name, StructType kind, dict annotations)
    cdef void add_field(self, FieldSpec field)


cdef class FieldSpec:
    cdef readonly int id
    cdef readonly str name
    cdef readonly str py_name
    cdef readonly object type
    cdef readonly object kind
    cdef readonly object qualifier
    cdef readonly object default "default_"
    cdef readonly object annotations

    @staticmethod
    cdef _fbthrift_create(
        int id,
        str name,
        str py_name,
        object type,
        NumberType kind,
        Qualifier qualifier,
        object default,
        dict annotations,
    )


cdef class ListSpec:
    cdef readonly object value
    cdef readonly object kind

    @staticmethod
    cdef _fbthrift_create(object value, NumberType kind)


cdef class SetSpec:
    cdef readonly object value
    cdef readonly object kind

    @staticmethod
    cdef _fbthrift_create(object value, NumberType kind)


cdef class MapSpec:
    cdef readonly object key
    cdef readonly object key_kind
    cdef readonly object value
    cdef readonly object value_kind

    @staticmethod
    cdef _fbthrift_create(
        object key,
        NumberType key_kind,
        object value,
        NumberType value_kind,
    )


cdef class InterfaceSpec:
    cdef readonly str name
    cdef cvector[PyObject*] _methods
    cdef readonly object annotations

    @staticmethod
    cdef _fbthrift_create(str name, dict annotations)
    cdef void add_method(self, MethodSpec method)


cdef class MethodSpec:
    cdef readonly str name
    cdef readonly tuple arguments
    cdef readonly object result_kind
    cdef readonly object result
    cdef readonly tuple exceptions
    cdef readonly object annotations

    @staticmethod
    cdef _fbthrift_create(
        str name,
        tuple arguments,
        NumberType result_kind,
        object result,
        tuple exceptions,
        dict annotations,
    )


cdef class ArgumentSpec:
    cdef readonly str name
    cdef readonly object kind
    cdef readonly object type
    cdef readonly object annotations

    @staticmethod
    cdef _fbthrift_create(str name, NumberType kind, object type, dict annotations)
