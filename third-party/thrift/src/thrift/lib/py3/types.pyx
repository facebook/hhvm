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

from collections import defaultdict
from collections.abc import Iterable, Mapping, Set as pySet
import enum
import itertools
import warnings
cimport cython

from cpython.object cimport Py_LT, Py_EQ, Py_NE, Py_GT, Py_GE, Py_LE
from cython.operator cimport dereference as deref, postincrement as inc
from folly.cast cimport down_cast_ptr
from folly.iobuf import IOBuf
from types import MappingProxyType

from thrift.py3.exceptions cimport GeneratedError
from thrift.py3.serializer import deserialize, serialize
from thrift.python.types cimport BadEnum as _fbthrift_python_BadEnum
from thrift.python.types import (
    Enum as _fbthrift_python_Enum,
    EnumMeta as _fbthrift_python_EnumMeta,
    Flag as _fbthrift_python_Flag,
    StructOrUnion as _fbthrift_python_StructOrUnion,
)

# ensures that common classes can be reliably imported from thrift.py3.types
BadEnum = _fbthrift_python_BadEnum
EnumMeta = _fbthrift_python_EnumMeta

__all__ = ['Struct', 'BadEnum', 'Union', 'Enum', 'Flag', 'EnumMeta']


# This isn't exposed to the module dict
Object = cython.fused_type(Struct, GeneratedError)



cdef list_compare(object first, object second, int op):
    """ Take either Py_EQ or Py_LT, everything else is derived """
    if not (isinstance(first, Iterable) and isinstance(second, Iterable)):
        if op == Py_EQ:
            return False
        else:
            return NotImplemented

    if op == Py_EQ:
        if len(first) != len(second):
            return False

    for x, y in zip(first, second):
        if x != y:
            if op == Py_LT:
                return x < y
            else:
                return False

    if op == Py_LT:
        return len(first) < len(second)
    return True


@cython.internal
@cython.auto_pickle(False)
cdef class StructMeta(type):
    """
    We set helper functions here since they can't possibly confict with field names
    """
    @staticmethod
    def isset(Object struct):
        return struct._fbthrift_isset()

    @staticmethod
    def update_nested_field(Struct obj, path_to_values):
        # There is some optimzation opportunity here for cases like this:
        # { "a.b.c": foo, "a.b.d": var }
        try:
            obj = _fbthrift_struct_update_nested_field(
                obj,
                [(p.split("."), v) for p, v in path_to_values.items()]
            )
            return obj
        except (AttributeError, TypeError) as e:
            # Unify different exception types to ValueError
            raise ValueError(e)

    def __iter__(cls):
        for i in range(cls._fbthrift_get_struct_size()):
            yield cls._fbthrift_get_field_name_by_index(i), None

    def __dir__(cls):
        return tuple(name for name, _ in cls) + ("__iter__", )


cdef Struct _fbthrift_struct_update_nested_field(Struct obj, list path_and_vals):
    field_to_nested_path_and_vals = defaultdict(list)
    cdef dict field_to_vals = {}
    for path, val in path_and_vals:
        field = path[0]
        if len(path) > 1:
            field_to_nested_path_and_vals[field].append((path[1:], val))
        else:
            field_to_vals[field] = val

    cdef dict updatedict = {}
    for field, val in field_to_vals.items():
        if field in field_to_nested_path_and_vals:
            conflicts = [
                ".".join([field] + p)
                for p, _ in field_to_nested_path_and_vals[field]
            ] + [field]
            raise ValueError("Conflicting overrides: {}".format(conflicts))
        updatedict[field] = val

    for field, nested_path_and_vals in field_to_nested_path_and_vals.items():
        updatedict[field] = _fbthrift_struct_update_nested_field(
            getattr(obj, field),
            nested_path_and_vals)

    return obj(**updatedict)


class _IsSet:
    __slots__ = ['__dict__', '__name__']

    def __init__(self, n, d):
        self.__name__ = n
        self.__dict__ = d

    def __repr__(self):
        args = ''.join(f', {name}={value}' for name, value in self.__dict__.items())
        return f'Struct.isset(<{self.__name__}>{args})'

cdef class Struct:
    """
    Base class for all thrift structs
    """
    cdef IOBuf _fbthrift_serialize(self, Protocol proto):
        return IOBuf(b'')

    cdef uint32_t _fbthrift_deserialize(self, const cIOBuf* buf, Protocol proto) except? 0:
        return 0

    cdef object _fbthrift_isset(self):
        raise TypeError(f"{type(self)} does not have concept of isset")

    @classmethod
    def _fbthrift_get_field_name_by_index(cls, idx):
        raise NotImplementedError()

    def __init__(self, **kwargs):
        for name, value in kwargs.items():
            if value is not None:
                self._fbthrift_set_field(name, value)

    def __hash__(self):
        if not self._fbthrift_hash:
            value_tuple = tuple(v for _, v in self)
            self._fbthrift_hash = hash(
                value_tuple if value_tuple
                else type(self)  # Hash the class there are no fields
            )
        return self._fbthrift_hash

    @classmethod
    def _fbthrift_get_struct_size(cls):
        return 0

    def __iter__(self):
        for i in range(self._fbthrift_get_struct_size()):
            name = self._fbthrift_get_field_name_by_index(i)
            yield name, getattr(self, name)

    def __dir__(self):
        return dir(type(self))

    cdef bint _fbthrift_noncomparable_eq(self, other):
        if self is other:
            return True
        name = None  # as sentinel to indicate if no fields
        for name, value in self.__iter__():
            if value != getattr(other, name):
                return False
        # in case of no fields, only True when identity equal (begining).
        return name is not None

    cdef void _fbthrift_set_field(self, str name, object value) except *:
        pass

    def __reduce__(self):
        return (deserialize, (type(self), serialize(self)))

    def __repr__(self):
        fields = ", ".join(f"{name}={repr(value)}" for name, value in self)
        return f"{type(self).__name__}({fields})"

    def __deepcopy__(self, _):
        """
        copying a thrift-py3 struct is always deep copy
        """
        return self.__copy__()

    cdef object _fbthrift_cmp_sametype(self, other, int op):
        if (not isinstance(self, Struct) or not isinstance(other, Struct) or
                (not isinstance(other, type(self)) and not isinstance(self, type(other)))):
            if op == Py_EQ:  # different types are never equal
                return False
            if op == Py_NE:  # different types are always notequal
                return True
            return NotImplemented
        # otherwise returns None

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        raise NotImplementedError()


SetMetaClass(<PyTypeObject*> Struct, <PyTypeObject*> StructMeta)


cdef class Union(Struct):
    """
    Base class for all thrift Unions
    """
    cdef bint _fbthrift_noncomparable_eq(self, other):
        return self.type == other.type and self.value == other.value

    def __hash__(self):
        if not self._fbthrift_hash:
            self._fbthrift_hash = hash((
                self.type,
                self.value,
            ))
        return self._fbthrift_hash

    def __repr__(self):
        return f"{type(self).__name__}(type={self.type.name}, value={self.value!r})"

    def __bool__(self):
        return self.type.value != 0

    def __iter__(self):
        yield from ()

    def __dir__(self):
        return dir(type(self))

    def get_type(self):
        return self.type


@cython.internal
@cython.auto_pickle(False)
cdef class UnionMeta(type):
    def __dir__(cls):
        return [
            cls._fbthrift_get_field_name_by_index(i)
            for i in range(cls._fbthrift_get_struct_size())
        ] + ["type", "value"]


SetMetaClass(<PyTypeObject*> Union, <PyTypeObject*> UnionMeta)


@cython.auto_pickle(False)
cdef class Container:
    """
    Base class for all thrift containers
    """
    def __hash__(self):
        if not self._fbthrift_hash:
            self._fbthrift_hash = hash(tuple(self))
        return self._fbthrift_hash

    def __len__(self):
        raise NotImplementedError()

cdef class _ListPrivateCtorToken:
    pass

_fbthrift_list_private_ctor = _ListPrivateCtorToken()

@cython.auto_pickle(False)
cdef class List(Container):
    """
    Base class for all thrift lists
    """

    def __init__(self, list py_obj not None, child_cls not None):
        self._py_obj = py_obj
        self._child_cls = child_cls

    def __len__(self):
        return len(self._py_obj)

    def __hash__(self):
        return super().__hash__()

    def __add__(self, other):
        return type(self)(itertools.chain(self, other))

    def __radd__(List self, other):
        return type(other)(itertools.chain(other, self))

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
        return not result if result is not NotImplemented else NotImplemented

    def __ge__(self, other):
        result = list_compare(self, other, Py_LT)
        return not result if result is not NotImplemented else NotImplemented

    def __repr__(self):
        if not self:
            return 'i[]'
        return f'i[{", ".join(map(repr, self))}]'

    def __reduce__(self):
        return (type(self), (list(self), ))

    def __getitem__(self, object index_obj):
        if isinstance(index_obj, slice):
            return self._child_cls(self._py_obj[index_obj])
        cdef int norm_index = self._normalize_index(<int?>index_obj)
        return self._py_obj[norm_index]

    def __contains__(self, item):
        try:
            self.index(item)
        except ValueError:
            return False
        return True

    def __iter__(self):
        yield from self._py_obj

    def __reversed__(self):
        yield from reversed(self._py_obj)

    def index(self, item, start=0, stop=None):
        item = self._child_cls._check_item_type_or_none(item)
        if not self or item is None:
            raise ValueError(f'{item} is not in list')
        if stop is None:
            return self._py_obj.index(item, start)
        else:
            return self._py_obj.index(item, start, stop)

    def count(self, item):
        item = self._child_cls._check_item_type_or_none(item)
        if item is None:
            return 0
        return self._py_obj.count(item)

    cdef int _normalize_index(self, int index) except *:
        cdef int size = len(self)
        # Convert a negative index
        if index < 0:
            index = size + index
        if index >= size or index < 0:
            raise IndexError('list index out of range')
        return index


@cython.auto_pickle(False)
cdef class Set(Container):
    """
    Base class for all thrift sets
    """

    def __reduce__(self):
        return (type(self), (set(self), ))

    def __repr__(self):
        if not self:
            return 'iset()'
        return f'i{{{", ".join(map(repr, self))}}}'

    def isdisjoint(self, other):
        return len(self & other) == 0

    def union(self, other):
        return self | other

    def intersection(self, other):
        return self & other

    def difference(self, other):
        return self - other

    def symmetric_difference(self, other):
        return self ^ other

    def issubset(self, other):
        return self <= other

    def issuperset(self, other):
        return self >= other

    cdef _fbthrift_py_richcmp(self, other, int op):
        if op == Py_LT:
            return pySet.__lt__(self, other)
        elif op == Py_LE:
            return pySet.__le__(self, other)
        elif op == Py_EQ:
            return pySet.__eq__(self, other)
        elif op == Py_NE:
            return pySet.__ne__(self, other)
        elif op == Py_GT:
            return pySet.__gt__(self, other)
        elif op == Py_GE:
            return pySet.__ge__(self, other)

    cdef _fbthrift_do_set_op(self, other, cSetOp op):
        raise NotImplementedError()

    def __and__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.AND)

    def __sub__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.SUB)

    def __or__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.OR)

    def __xor__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.XOR)

    def __rand__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.AND)

    def __ror__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.OR)

    def __rxor__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.XOR)

    def __rsub__(Set self, other):
        return self._fbthrift_do_set_op(other, cSetOp.REVSUB)


@cython.auto_pickle(False)
cdef class Map(Container):
    """
    Base class for all thrift maps
    """

    def __eq__(self, other):
        if not (isinstance(self, Mapping) and isinstance(other, Mapping)):
            return False
        if len(self) != len(other):
            return False

        for key in self:
            if key not in other:
                return False
            if other[key] != self[key]:
                return False

        return True

    def __ne__(self, other):
        return not self.__eq__(other)

    def __hash__(self):
        if not self._fbthrift_hash:
            self._fbthrift_hash = hash(tuple(self.items()))
        return self._fbthrift_hash

    def __repr__(self):
        if not self:
            return 'i{}'
        return f'i{{{", ".join(map(lambda i: f"{repr(i[0])}: {repr(i[1])}", self.items()))}}}'

    def __reduce__(self):
        return (type(self), (dict(self), ))

    def keys(self):
        return self.__iter__()

    def get(self, key, default=None):
        try:
            return self.__getitem__(key)
        except KeyError:
            return default

CompiledEnum = _fbthrift_python_Enum
Enum = _fbthrift_python_Enum
# I wanted to call the base class Enum, but there is a cython bug
# See https://github.com/cython/cython/issues/2474
# Will move when the bug is fixed

Flag = _fbthrift_python_Flag

cdef class StructFieldsSetter:
    cdef void set_field(self, const char* name, object val) except *:
        pass


cdef translate_cpp_enum_to_python(object EnumClass, int value):
    try:
        return EnumClass(value)
    except ValueError:
        return BadEnum(EnumClass, value)


def _is_python_struct(obj):
    return isinstance(obj, _fbthrift_python_StructOrUnion)

def _is_python_enum(obj):
    return (
        isinstance(obj, _fbthrift_python_Enum) and
        obj.__class__.__module__.endswith(".thrift_types")
    )
