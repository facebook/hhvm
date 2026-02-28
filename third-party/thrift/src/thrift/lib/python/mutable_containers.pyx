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

from cython cimport final as _cython__final

from collections.abc import (
    ItemsView,
    Iterable,
    Mapping,
    MutableMapping,
    MutableSequence,
    MutableSet as pyMutableSet,
    KeysView,
    Set,
    ValuesView,
)
from cpython.object cimport Py_LT, Py_EQ, PyCallable_Check
import copy
import itertools

from thrift.python.mutable_typeinfos cimport (
    MutableListTypeInfo,
    MutableSetTypeInfo,
    MutableStructTypeInfo,
    MutableMapTypeInfo,
)
from thrift.python.mutable_types cimport _ThriftContainerWrapper
from thrift.python.types cimport (
    TypeInfoBase,
    AdaptedTypeInfo,
    list_eq,
    list_lt,
)


@_cython__final
cdef class MutableListTypeFactory:
    cdef TypeInfoBase value_typeinfo
    def __init__(self, value_typeinfo):
        self.value_typeinfo = value_typeinfo

    def __call__(self, values=None):
        if values is None:
            return MutableList(self.value_typeinfo, [])

        list_typeinfo = MutableListTypeInfo(self.value_typeinfo)
        internal_data = list_typeinfo.to_internal_data(values)
        return MutableList(self.value_typeinfo, internal_data)


@_cython__final
cdef class MutableList:
    """
    A mutable container used to represent a Thrift mutable list.
    It implements the [`MutableSequence` abstract base class](https://docs.python.org/3.10/library/collections.abc.html#collections-abstract-base-classes).

    Additionally, it supports other methods from the built-in `list` data type,
    including `append()`, `extend()`, `pop()` `clear()`, `remove()` and `sort()`.

    Attributes:
        _list_data (Python list): reference to the the `list` in the mutable struct
            tuple (`struct._fbthrift_data`). Any change to `_list_data` results in an
            actual update to the connected Thrift struct.

            Additionally, any update operation on the `list` elements follows the type
            checking rules. For instance, if it is `list<i32>`, assigning a `string`
            will raise a `TypeError`. Another consequence of the type checking is that
            `MutableList` cannot contain `None` elements.
        _val_typeinfo (TypeInfoBase): The type of the values in this list.
        _value_type_is_container (bool): Whether the values of this list are (nested)
            containers.
    """
    def __cinit__(self, TypeInfoBase value_typeinfo, list list_data):
        self._val_typeinfo = value_typeinfo
        self._list_data = list_data
        self._value_type_is_container = value_typeinfo.is_container()

    def __iter__(self):
        return ValueIterator(self._val_typeinfo, self._list_data)

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
        self._list_data.insert(index, internal_value)

    def append(self, value):
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._list_data.append(internal_value)

    def extend(self, values):
        for value in values:
            internal_value = self._val_typeinfo.to_internal_data(value)
            self._list_data.append(internal_value)

    def pop(self, index=-1):
        internal_value = self._list_data.pop(index)
        return self._val_typeinfo.to_python_value(internal_value)

    def remove(self, value):
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._list_data.remove(internal_value)

    def clear(self):
        self._list_data.clear()

    def sort(self, *, object key=None, object reverse=False):
        self._list_data.sort(key=key, reverse=reverse)

    def __eq__(self, other):
        return list_eq(self, other)

    def __ne__(self, other):
        return not list_eq(self, other)

    def __lt__(self, other):
        return list_lt(self, other)

    def __gt__(self, other):
        return list_lt(other, self)

    def __le__(self, other):
        result = list_lt(other, self)
        if result is NotImplemented:
            return NotImplemented

        return not result

    def __ge__(self, other):
        result = list_lt(self, other)
        if result is NotImplemented:
            return NotImplemented

        return not result

    def __repr__(self):
        if not self:
            return '[]'
        return f'[{", ".join(map(repr, self))}]'

    def __contains__(self, value):
        if value is None:
            return False

        try:
            internal_value = self._value_to_internal_data(value)
        except (TypeError, OverflowError):
            return False

        return internal_value in self._list_data

    def __add__(self, other):
        lst = MutableList(self._val_typeinfo, self._list_data[:])
        lst.extend(other)
        return lst

    def __radd__(self, other):
        lst = MutableList(self._val_typeinfo, [])
        lst.extend(other)
        lst._list_data.extend(self._list_data)
        return lst

    def __deepcopy__(self, memo):
        return MutableList(self._val_typeinfo, copy.deepcopy(self._list_data, memo))

    def __reduce__(self):
        return (MutableList, (self._val_typeinfo, self._list_data))

    def count(self, value):
        try:
            internal_value = self._value_to_internal_data(value)
        except (TypeError, OverflowError):
            return 0

        return self._list_data.count(internal_value)

    def index(self, value, start=0, stop=None):
        try:
            internal_value = self._value_to_internal_data(value)
        except (TypeError, OverflowError):
            raise ValueError

        if stop is None:
            return self._list_data.index(internal_value, start)
        else:
            return self._list_data.index(internal_value, start, stop)

    @classmethod
    def __class_getitem__(cls, _):
        """
        PEP 560 – Core support for typing module and generic types
        It enables generic types like `MutableList[T]`
        """
        return cls

    cdef _value_to_internal_data(self, value):
        """
        The `_value_to_internal_data()` method is internal and used to wrap the
        value when it is a container. This should be done implicitly in some cases.
        For example, for a given list field (list<list<int>>), the user must use
        `to_thrift_list()` for assignment:

        s.list_of_list_field = to_thrift_map([[1], [2]])

        However, for `in` check, it is implicit:

        [1] in s.list_of_list_field

        This method is called when an implicit wrapper is needed.
        """
        value = (_ThriftContainerWrapper(value)
                 if self._value_type_is_container
                 else value)
        return self._val_typeinfo.to_internal_data(value)


tag_object_as_sequence(<PyTypeObject*>MutableList)
MutableSequence.register(MutableList)


@_cython__final
cdef class MutableSetTypeFactory:
    cdef TypeInfoBase value_typeinfo
    def __init__(self, value_typeinfo):
        self.value_typeinfo = value_typeinfo

    def __call__(self, values=None):
        if values is None:
            return MutableSet(self.value_typeinfo, set())

        set_typeinfo = MutableSetTypeInfo(self.value_typeinfo)
        internal_data = set_typeinfo.to_internal_data(values)
        return MutableSet(self.value_typeinfo, internal_data)


@_cython__final
cdef class MutableSet:
    """
    A mutable container used to represent a Thrift mutable set. It implements
    the [`MutableSet` abstract base class](https://docs.python.org/3.10/library/collections.abc.html#collections-abstract-base-classes).
    base class
    """

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

        try:
            internal_item = self._val_typeinfo.to_internal_data(item)
        except (TypeError, OverflowError):
            return False

        return internal_item in self._set_data

    def __iter__(MutableSet self):
        return ValueIterator(self._val_typeinfo, self._set_data)

    def __len__(MutableSet self):
        return len(self._set_data)

    def __le__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            return self._set_data <= (<MutableSet>other)._set_data

        if not isinstance(other, Set):
            return NotImplemented

        if len(self) > len(other):
            return False

        for elem in self:
            if elem not in other:
                return False

        return True

    def __lt__(MutableSet self, other):
        if not isinstance(other, Set):
            return NotImplemented

        return len(self) < len(other) and self <= other

    def __ge__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            return self._set_data >= (<MutableSet>other)._set_data

        if not isinstance(other, Set):
            return NotImplemented

        if len(self) < len(other):
            return False

        for elem in other:
            if elem not in self:
                return False

        return True

    def __gt__(MutableSet self, other):
        if not isinstance(other, Set):
            return NotImplemented

        return len(self) > len(other) and self >= other

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

    def __rand__(MutableSet self, other):
        return self & other

    def __iand__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            self._set_data &= (<MutableSet>other)._set_data
            return self

        for value in (self - other):
            self.discard(value)

        return self

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

    def __ror__(MutableSet self, other):
        return self | other

    def __ior__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            self._set_data |= (<MutableSet>other)._set_data
            return self

        for value in other:
            self.add(value)

        return self

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

    def __isub__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            self._set_data -= (<MutableSet>other)._set_data
            return self

        for value in other:
            self.discard(value)

        return self

    def __xor__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            result_set_data = self._set_data ^ (<MutableSet>other)._set_data
            return MutableSet(self._val_typeinfo, result_set_data)

        if not isinstance(other, Iterable):
            return NotImplemented

        other = MutableSet._from_iterable(self._val_typeinfo, set(), other)
        return (self - other) | (other - self)

    def __rxor__(MutableSet self, other):
        return self ^ other

    def __ixor__(MutableSet self, other):
        if self._is_same_type_of_set(other):
            self._set_data ^= (<MutableSet>other)._set_data
            return self

        other = MutableSet._from_iterable(self._val_typeinfo, set(), other)
        self._set_data ^= (<MutableSet>other)._set_data
        return self

    def __eq__(MutableSet self, other):
        if self is other:
            return True

        # Note: comparing internal data is buggy if any elements are
        # mutable structs. However, this is not possible because MutableStruct
        # etc. and mutable containers are not hashable.
        if self._is_same_type_of_set(other):
            return self._set_data == (<MutableSet>other)._set_data

        if not isinstance(other, Set):
            return NotImplemented

        if len(self._set_data) != len(other):
            return False

        for value in other:
            if value not in self:
                return False

        return True

    def __deepcopy__(self, memo):
        return MutableSet(self._val_typeinfo, copy.deepcopy(self._set_data, memo))

    def __reduce__(self):
        return (MutableSet, (self._val_typeinfo, self._set_data))

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
            return 'set()'
        return f'{{{", ".join(map(repr, self))}}}'

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
        if self is other:
            return True

        if not isinstance(other, MutableSet):
            return False

        return self._val_typeinfo.same_as((<MutableSet>other)._val_typeinfo)

    @classmethod
    def __class_getitem__(cls, _):
        """
        PEP 560 – Core support for typing module and generic types
        It enables generic types like `MutableSet[T]`
        """
        return cls

pyMutableSet.register(MutableSet)

@_cython__final
cdef class ValueIterator:
    def __cinit__(self, TypeInfoBase value_typeinfo, data: Iterable):
        self._val_typeinfo = value_typeinfo
        self._iter = iter(data)

    def __next__(self):
        return self._val_typeinfo.to_python_value(next(self._iter))

    def __iter__(self):
        return self


@_cython__final
cdef class MutableMapTypeFactory:
    cdef TypeInfoBase key_typeinfo
    cdef TypeInfoBase value_typeinfo
    def __init__(self, key_typeinfo, value_typeinfo):
        self.key_typeinfo = key_typeinfo
        self.value_typeinfo = value_typeinfo

    def __call__(self, values=None):
        if values is None:
            return MutableMap(self.key_typeinfo, self.value_typeinfo, {})

        map_typeinfo = MutableMapTypeInfo(self.key_typeinfo, self.value_typeinfo)
        internal_data = map_typeinfo.to_internal_data(values)
        return MutableMap(self.key_typeinfo, self.value_typeinfo, internal_data)


@_cython__final
cdef class MutableMap:
    """
    A mutable container used to represent a Thrift mutable map. It implements
    the [`MutableMap` abstract base class](https://docs.python.org/3.10/library/collections.abc.html#collections-abstract-base-classes).
    """

    def __cinit__(MutableMap self, TypeInfoBase key_typeinfo, TypeInfoBase value_typeinfo, dict map_data not None):
        """
        map_data: It should contain valid elements. Any invalid elements within
            `map_data` may lead to undefined behavior.
        """
        self._key_typeinfo = key_typeinfo
        self._val_typeinfo = value_typeinfo
        self._value_type_is_container = value_typeinfo.is_container()
        # Compare internal data representations only if `_value_type` is not a structured type
        # or container type, as these may include additional information like `isset-flags`
        # or `internal-cache`. For structured or container types, convert them to Python types,
        # as the type system will ignore these 'hidden' fields during comparison.
        self._use_internal_data_to_compare = not (value_typeinfo.is_container()
            or isinstance(value_typeinfo, (AdaptedTypeInfo, MutableStructTypeInfo)))
        self._map_data = map_data

    def __len__(self):
        return len(self._map_data)

    def __eq__(self, other):
        if self is other:
            return True

        if isinstance(other, MutableMap):
            if not self._is_same_type_of_map(other):
                return False
            if self._use_internal_data_to_compare:
                return self._map_data == (<MutableMap>other)._map_data

        if not isinstance(other, Mapping):
            return NotImplemented

        if len(self._map_data) != len(other):
            return False

        for other_key, other_value in other.items():
            self_value = self.get(other_key, None)
            # self cannot contain None values, so None means not present
            if self_value is None or self_value != other_value:
                return False

        return True

    def __getitem__(self, key):
        internal_key = self._key_typeinfo.to_internal_data(key)
        return self._val_typeinfo.to_python_value(self._map_data[internal_key])

    def __iter__(MutableMap self):
        return ValueIterator(self._key_typeinfo, self._map_data)

    def get(self, key, default=None):
        try:
            return self[key]
        except KeyError:
            return default

    def __contains__(self, key):
        if key is None:
            return False

        try:
            internal_key = self._key_typeinfo.to_internal_data(key)
        except (TypeError, OverflowError):
            return False

        return internal_key in self._map_data

    def __reduce__(self):
        return (MutableMap, (self._key_typeinfo, self._val_typeinfo, self._map_data))

    __sentinel = object()

    def pop(self, key, default=__sentinel):
        try:
            internal_key = self._key_typeinfo.to_internal_data(key)
            return self._val_typeinfo.to_python_value(self._map_data.pop(internal_key))
        except (TypeError, KeyError):
            if default is self.__sentinel:
                raise KeyError(f"{key}")
            return default

    def popitem(self):
        """
        Remove and return a (key, value) pair from the dictionary. Pairs are returned in LIFO order.
        Changed in version Python 3.7: LIFO order is now guaranteed.
        In prior versions, popitem() would return an arbitrary key/value pair.
        """
        k, v = self._map_data.popitem()
        return self._key_typeinfo.to_python_value(k), self._val_typeinfo.to_python_value(v)

    def clear(self):
        self._map_data.clear()

    def keys(self):
        return MapKeysView(self._key_typeinfo, self._map_data.keys())

    def items(self):
        return MapItemsView(self._key_typeinfo, self._val_typeinfo, self._map_data.items())

    def values(self):
        return MapValuesView(self._val_typeinfo, self._map_data.values())

    def setdefault(self, key, default=None):
        internal_key = self._key_typeinfo.to_internal_data(key)
        internal_default = self._val_typeinfo.to_internal_data(default)
        return self._val_typeinfo.to_python_value(
                    self._map_data.setdefault(internal_key, internal_default))

    def update(self, other=(), /, **keywords):
        """
        Update MutableMap from mapping/iterable other and keywords
        """
        if self._use_internal_data_to_compare and self._is_same_type_of_map(other):
            self._map_data.update(<MutableMap>other._map_data)
        elif isinstance(other, Mapping):
            for key in other:
                self[key] = other[key]
        elif hasattr(other, "keys"):
            for key in other.keys():
                self[key] = other[key]
        else:
            for key, value in other:
                self[key] = value

        for key, value in keywords.items():
            self[key] = value

    def __setitem__(self, key, value):
        internal_key = self._key_typeinfo.to_internal_data(key)
        internal_value = self._val_typeinfo.to_internal_data(value)
        self._map_data[internal_key] = internal_value

    def __delitem__(self, key):
        try:
            internal_key = self._key_typeinfo.to_internal_data(key)
        except TypeError:
            raise KeyError(f"{key}")

        del self._map_data[internal_key]

    def __deepcopy__(self, memo):
        return MutableMap(self._key_typeinfo, self._val_typeinfo, copy.deepcopy(self._map_data, memo))

    def _is_same_type_of_map(MutableMap self, other):
        """
        Returns `True` if `other` is a `MutableMap` with the same
        `_key_typeinfo` and `_val_typeinfo` as `self`, `False` otherwise.
        """
        if not isinstance(other, MutableMap):
            return False

        return (self._key_typeinfo.same_as((<MutableMap>other)._key_typeinfo)
            and self._val_typeinfo.same_as((<MutableMap>other)._val_typeinfo))

    # The `_value_to_internal_data()` methods are internal and used to wrap
    # the value or key when they are containers. This should be done implicitly
    # in some cases. For example, for a given map field (map<int, list<int>>),
    # the user must use `to_thrift_map()` for assignment:
    #
    # s.map_field = to_thrift_map({1: [1]})
    #
    # However, for comparison, it is implicit:
    #
    # s.map_field == {1: [1]}
    #
    # These is no `_key_to_internal_data` method because mutable containers
    # cannot be used as keys (not hashable)
    #
    # This method is called when an implicit wrapper is needed.
    cdef _value_to_internal_data(self, value):
        return self._val_typeinfo.to_internal_data(
                _ThriftContainerWrapper(value)
                if self._value_type_is_container
                else value)

    def __repr__(self):
        if not self:
            return '{}'
        return f'{{{", ".join(map(lambda i: f"{repr(i[0])}: {repr(i[1])}", self.items()))}}}'

    @classmethod
    def __class_getitem__(cls, _):
        """
        PEP 560 – Core support for typing module and generic types
        It enables generic types like `MutableSet[T]`
        """
        return cls


tag_object_as_mapping(<PyTypeObject*>MutableMap)
MutableMapping.register(MutableMap)

@_cython__final
cdef class MapKeysView:
    def __cinit__(self, TypeInfoBase key_typeinfo, dict_keys):
        self._key_typeinfo = key_typeinfo
        self._dict_keys = dict_keys

    def __len__(self):
        return len(self._dict_keys)

    def __contains__(self, key):
        if key is None:
            return False

        try:
            internal_key = self._key_typeinfo.to_internal_data(key)
        except (TypeError, OverflowError):
            return False

        return internal_key in self._dict_keys

    def __iter__(self):
        return ValueIterator(self._key_typeinfo, self._dict_keys)


KeysView.register(MapKeysView)

@_cython__final
cdef class MapItemsView:
    def __cinit__(self, TypeInfoBase key_typeinfo, TypeInfoBase value_typeinfo, dict_items):
        self._key_typeinfo = key_typeinfo
        self._val_typeinfo = value_typeinfo
        self._dict_items = dict_items

    def __len__(self):
        return len(self._dict_items)

    def __contains__(self, item):
        if item is None:
            return False

        is_container = self._val_typeinfo.is_container();
        # Implicit container wrapping for cases like
        # ("a", [1, 2]) in my_map_items
        value = _ThriftContainerWrapper(item[1]) if is_container else item[1]

        try:
            internal_item = (self._key_typeinfo.to_internal_data(item[0]),
                             self._val_typeinfo.to_internal_data(value))
        except (TypeError, OverflowError):
            return False

        return internal_item in self._dict_items

    def __iter__(self):
        return MapItemIterator(self._key_typeinfo, self._val_typeinfo, self._dict_items)


ItemsView.register(MapItemsView)

@_cython__final
cdef class MapItemIterator:
    def __cinit__(self, TypeInfoBase key_typeinfo, TypeInfoBase value_typeinfo, dict_items):
        self._key_typeinfo = key_typeinfo
        self._val_typeinfo = value_typeinfo
        self._iter = iter(dict_items)

    def __next__(self):
        it = next(self._iter)
        return (self._key_typeinfo.to_python_value(it[0]),
                self._val_typeinfo.to_python_value(it[1]))

    def __iter__(self):
        return self

@_cython__final
cdef class MapValuesView:
    def __cinit__(self, TypeInfoBase value_typeinfo, dict_values):
        self._val_typeinfo = value_typeinfo
        self._dict_values = dict_values

    def __len__(self):
        return len(self._dict_values)

    def __iter__(self):
        return ValueIterator(self._val_typeinfo, self._dict_values)


ValuesView.register(MapValuesView)
