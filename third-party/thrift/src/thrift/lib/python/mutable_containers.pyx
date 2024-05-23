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


from collections.abc import (
    Iterable,
    MutableSequence,
    MutableSet as pyMutableSet,
    Set
)
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
    def __cinit__(self, TypeInfoBase value_typeinfo, list list_data):
        self._val_typeinfo = value_typeinfo
        self._list_data = list_data

    def __getitem__(self, object index_obj):
        if isinstance(index_obj, slice):
            return MutableList(self._val_typeinfo, self._list_data[index_obj])

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


MutableSequence.register(MutableList)


cdef class MutableSet:
    """
    A mutable container used to represent a Thrift mutable set. It implements
    the [`MutableSet` abstract base class](https://docs.python.org/3.10/library/collections.abc.html#collections-abstract-base-classes).
    base class
    """
    # DO_BEFORE(alperyoney,20240603): Implement missing methods from abstract

    def __cinit__(MutableSet self, TypeInfoBase value_typeinfo, set set_data):
        """
        Initialize a new `MutableSet` object.
        Args:
            value_typeinfo (TypeInfoBase): The type information for the values
                that will be stored in the set. This is used to ensure that all
                values in the set are type-checked and of the same type.
            set_data (set): The initial data for the set. This should be an
                empty set or a set of values that match the type defined by
                `value_typeinfo`.
        """
        self._val_typeinfo = value_typeinfo
        self._set_data = set_data

    def __contains__(MutableSet self, item):
        if item is None:
            return False

        internal_item = self._val_typeinfo.to_internal_data(item)
        return internal_item in self._set_data

    def __iter__(MutableSet self):
        return MutableSetIterator(self._val_typeinfo, self._set_data)

    def __len__(MutableSet self):
        return len(self._set_data)

    def isdisjoint(MutableSet self, other):
        if self._is_same_type_of_set(other):
            return self._set_data.isdisjoint((<MutableSet>other)._set_data)

        if not isinstance(other, Iterable):
            return NotImplemented

        for value in other:
            if self._val_typeinfo.to_internal_data(value) in self._set_data:
                return False

        return True

    def __and__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            result_set_data = self._set_data & (<MutableSet>other)._set_data
            return MutableSet(self._val_typeinfo, result_set_data)

        if not isinstance(other, Iterable):
            return NotImplemented

        cdef TypeInfoBase typeinfo = self._val_typeinfo
        cdef set type_checked_set = set()
        for value in other:
            internal_value = typeinfo.to_internal_data(value)
            if internal_value in self._set_data:
                type_checked_set.add(internal_value)

        return MutableSet(self._val_typeinfo, type_checked_set)

    def __or__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            result_set_data = self._set_data | (<MutableSet>other)._set_data
            return MutableSet(self._val_typeinfo, result_set_data)

        if not isinstance(other, Iterable):
            return NotImplemented

        result_set = MutableSet(self._val_typeinfo, self._set_data.copy())
        for value in other:
            result_set.add(value)

        return result_set

    def __sub__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            result_set_data = self._set_data - (<MutableSet>other)._set_data
            return MutableSet(self._val_typeinfo, result_set_data)

        if not isinstance(other, Iterable):
            return NotImplemented

        if not isinstance(other, Set):
            other = set(other)

        cdef TypeInfoBase typeinfo = self._val_typeinfo
        return MutableSet._from_iterable(self._val_typeinfo,
                                         set(),
                                         (value for value in self._set_data
                                          if typeinfo.to_python_value(value) not in other))

    def __xor__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            result_set_data = self._set_data ^ (<MutableSet>other)._set_data
            return MutableSet(self._val_typeinfo, result_set_data)

        if not isinstance(other, Iterable):
            return NotImplemented

        other = MutableSet._from_iterable(self._val_typeinfo, set(), other)
        return (self - other) | (other - self)

    def __eq__(MutableSet self, other):
        if self is other:
            return True

        if self._is_same_type_of_set(other):
            return self._set_data == (<MutableSet>other)._set_data

        if not isinstance(other, Set):
            return NotImplemented

        if len(self._set_data) != len(other):
            return False

        for value in other:
            internal_value = self._val_typeinfo.to_internal_data(value)
            if internal_value not in self._set_data:
                return False

        return True

    def union(MutableSet self, other):
        return self | other

    def add(self, value):
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._set_data.add(internal_value)

    def discard(MutableSet self, value):
        """Remove an element.  Do not raise an exception if absent."""
        try:
            internal_value = self._val_typeinfo.to_internal_data(value)
            self._set_data.discard(internal_value)
        except Exception:
            pass

    def remove(MutableSet self, value):
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._set_data.remove(internal_value)

    def pop(self):
        """Return the popped value.  Raise KeyError if empty."""
        internal_value = self._set_data.pop()
        return self._val_typeinfo.to_python_value(internal_value)

    def clear(MutableSet self):
        self._set_data.clear()

    def __repr__(MutableSet self):
        if not self:
            return 'iset()'
        return f'i{{{", ".join(map(repr, self))}}}'

    @classmethod
    def _from_iterable(cls, TypeInfoBase value_typeinfo, set set_data, object it):
        s = MutableSet(value_typeinfo, set_data)
        for value in it:
            s.add(value)

        return s

    def _is_same_type_of_set(MutableSet self, other):
        """
        Returns `True` if `other` is a `MutableSet` with the same
        `_val_typeinfo` as `self`, `False` otherwise.
        """
        if not isinstance(other, MutableSet):
            return False

        return self._val_typeinfo.same_as((<MutableSet>other)._val_typeinfo)


pyMutableSet.register(MutableSet)


cdef class MutableSetIterator:
    def __cinit__(MutableSetIterator self, TypeInfoBase value_typeinfo, set set_data):
        self._val_typeinfo = value_typeinfo
        self._iter = iter(set_data)

    def __next__(MutableSetIterator self):
        return self._val_typeinfo.to_python_value(next(self._iter))

    def __iter__(self):
        return self
