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


from collections.abc import Iterable, MutableSequence
from cpython.object cimport Py_LT, Py_EQ, PyCallable_Check
import itertools

from thrift.python.types cimport (
    TypeInfoBase,
    list_compare,
)


cdef class MutableList:
    """
    A mutable container used to represent a Thrift mutable list.
    It implements the [`MutableSequence` abstract base class](https://docs.python.org/3.10/library/collections.abc.html#collections-abstract-base-classes).
    Additionally, it supports other methods from the built-in `list` data type,
    including `append()`, `extend()`, `pop()` and `clear()`.

    The `_list_data` member of `MutableList` is a reference to the `list` in
    the mutable struct tuple (`struct._fbthrift_data`). Any change to
    `_list_data` results in an actual update to the connected Thrift struct.
    Additionally, any update operation on the `list` elements follows the type
    checking rules. For instance, if it is `list<i32>`, assigning a `string`
    will raise a `TypeError`. Another consequence of the type checking is that
    `MutableList` cannot contain `None` elements.
    """
    def __init__(self):
        pass

    def __getitem__(self, object index_obj):
        if isinstance(index_obj, slice):
            return self._create_slice(index_obj)

        return self._val_typeinfo.to_python_value(self._list_data[index_obj])

    def __setitem__(self, index, value):
        self._list_data[index] = self._val_typeinfo.to_internal_data(value)

    def __delitem__(self, index):
        del self._list_data[index]

    def __len__(MutableList self):
        return len(self._list_data)

    def insert(self, index, value):
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._list_data.insert(index, value)

    def append(self, value):
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._list_data.append(value)

    def extend(self, values):
        for value in values:
            internal_value = self._val_typeinfo.to_internal_data(value)
            self._list_data.append(internal_value)

    def pop(self, index=-1):
        internal_value = self._list_data.pop(index)
        return self._val_typeinfo.to_python_value(internal_value)

    def clear(self):
        self._list_data.clear()

    def __eq__(self, other):
        return list_compare(self, other, Py_EQ)

    def __ne__(self, other):
        return not list_compare(self, other, Py_EQ)

    def __lt__(self, other):
        return list_compare(self, other, Py_LT)

    def __gt__(self, other):
        return list_compare(other, self, Py_LT)

    def __le__(self, other):
        result = list_compare(other, self, Py_LT)
        if result is NotImplemented:
            return NotImplemented

        return not result

    def __ge__(self, other):
        result = list_compare(self, other, Py_LT)
        if result is NotImplemented:
            return NotImplemented

        return not result

    def __repr__(self):
        if not self:
            return 'i[]'
        return f'i[{", ".join(map(repr, self))}]'

    def __contains__(self, item):
        if item is None:
            return False

        internal_item = self._val_typeinfo.to_internal_data(item)
        return internal_item in self._list_data

    cdef _create_slice(self, slice index_obj):
        cdef MutableList inst = MutableList.__new__(MutableList)
        inst._val_typeinfo = self._val_typeinfo
        inst._list_data = self._list_data[index_obj]
        return inst


MutableSequence.register(MutableList)
