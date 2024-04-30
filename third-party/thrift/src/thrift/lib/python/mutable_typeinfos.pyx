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

from folly.iobuf cimport IOBuf

from cython.operator cimport dereference as deref

from thrift.python.mutable_types cimport MutableStruct, MutableStructInfo
from thrift.python.exceptions cimport GeneratedError

cdef class MutableStructTypeInfo(TypeInfoBase):
    """
    `MutableStructTypeInfo` is similar to `StructTypeInfo`. However, we had to
    make a copy because mutable and immutable types inherit from different base
    classes, `MutableStruct` and `Struct`, respectively.
    """
    def __cinit__(self, mutable_struct_class):
        self._mutable_struct_class = mutable_struct_class
        cdef MutableStructInfo py_mutable_struct_info = mutable_struct_class._fbthrift_mutable_struct_info
        cdef cDynamicStructInfo* c_struct_info = py_mutable_struct_info.cpp_obj.get()
        self.cpp_obj = createStructTypeInfo(deref(c_struct_info))

    cdef const cTypeInfo* get_cTypeInfo(self):
        return &self.cpp_obj

    cdef to_internal_data(self, object value):
        """
        Validates and converts the given (mutable struct) `value` to a format
        that the serializer can understand.

        Args:
            value: should be an instance of `self._mutable_struct_class`, Otherwise, raises `TypeError`.

        Returns: the "struct tuple" of the given value (see `createStructTupleWithDefaultValues()`)

        Raises:
            TypeError if `value` is not an instance of `self._mutable_struct_class`
        """
        if not isinstance(value, self._mutable_struct_class):
            raise TypeError(f"value {value} is not a {self._mutable_struct_class !r}, is actually of type {type(value)}.")

        if isinstance(value, MutableStruct):
            return (<MutableStruct>value)._fbthrift_data
        if isinstance(value, GeneratedError):
            return (<GeneratedError>value)._fbthrift_data

        raise TypeError(f"MutableStructInfo cannot convert {self._mutable_struct_class} to internal data")

    # convert deserialized data to user format
    cdef to_python_value(self, object struct_tuple):
        return self._mutable_struct_class._fbthrift_create(struct_tuple)

