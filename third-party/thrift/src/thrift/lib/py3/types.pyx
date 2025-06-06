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
from collections.abc import Iterable, Mapping, Set as pySet, Sized
import enum
import itertools
import warnings
cimport cython

from cpython.object cimport Py_LT, Py_EQ, Py_NE, Py_GT, Py_GE, Py_LE
from cython.operator cimport dereference as deref, postincrement as inc
from folly.cast cimport down_cast_ptr
from folly.iobuf import IOBuf
from types import MappingProxyType

from folly cimport cFollyIsDebug
from thrift.python.exceptions cimport GeneratedError as _fbthrift_python_GeneratedError
from thrift.py3.serializer import deserialize, serialize
from thrift.python.types cimport (
    BadEnum as _fbthrift_python_BadEnum,
    _fbthrift_struct_update_nested_field as _fbthrift_python_struct_update_nested_field,
)
from apache.thrift.metadata.types_auto_migrated import _fbthrift__is_py3_auto_migrated
from thrift.python.types import (
    Container as _fbthrift_python_Container,
    Enum as _fbthrift_python_Enum,
    EnumMeta as _fbthrift_python_EnumMeta,
    Flag as _fbthrift_python_Flag,
    List as _fbthrift_python_List,
    Map as _fbthrift_python_Map,
    Set as _fbthrift_python_Set,
    Struct as _fbthrift_python_Struct,
    StructOrUnion as _fbthrift_python_StructOrUnion,
    isset as _fbthrift_python_isset,
    Union as _fbthrift_python_Union,
)

# ensures that common classes can be reliably imported from thrift.py3.types
BadEnum = _fbthrift_python_BadEnum
EnumMeta = _fbthrift_python_EnumMeta

__all__ = ['Struct', 'BadEnum', 'Union', 'Enum', 'Flag', 'EnumMeta']


_fbthrift__module_name__ = "thrift.py3.types"

cdef list_eq(List self, object other):
    if (
        not isinstance(other, Iterable) or
        not isinstance(other, Sized) or
        len(self) != len(other)
    ):
        return False

    for x, y in zip(self, other):
        if x != y:
            return False
    return True


cdef list_lt(object first, object second):
    if not (isinstance(first, Iterable) and isinstance(second, Iterable)):
        return NotImplemented

    for x, y in zip(first, second):
        if x != y:
            return x < y

    return len(first) < len(second)


class StructMeta(type):
    def __new__(cls, name, bases, classdict):
        # enforce no inheritance, for in-place migrated structs
        for base in bases:
            if (
                base is Struct or
                hasattr(base, '_fbthrift_allow_inheritance_DO_NOT_USE')
                or not hasattr(base, '_FBTHRIFT__PYTHON_CLASS')
            ):
                continue
            raise TypeError(
                f"Inheritance of thrift-generated {base.__name__} from {name}"
                " is deprecated. Please use composition."
            )

        return type.__new__(cls, name, bases, classdict)

    """
    We set helper functions here since they can't possibly confict with field names
    """
    @staticmethod
    def isset(struct):
        if isinstance(struct, (_fbthrift_python_Struct, _fbthrift_python_GeneratedError, _fbthrift_python_Union)):
            return _IsSet(type(struct).__name__, _fbthrift_python_isset(struct))
        if hasattr(struct.__class__, "_FBTHRIFT__PYTHON_CLASS"):
            return _IsSet(type(struct).__name__, struct._fbthrift__isset())
        elif isinstance(struct, Struct):
            return (<Struct>struct)._fbthrift_isset()
        elif isinstance(struct, GeneratedError):
            return (<GeneratedError>struct)._fbthrift_isset()

    @staticmethod
    def update_nested_field(obj, path_to_values):
        # There is an optimization opportunity here for cases like this:
        # { "a.b.c": foo, "a.b.d": var }
        try:
            if isinstance(obj, _fbthrift_python_Struct):
                return _fbthrift_python_struct_update_nested_field(
                    obj,
                    [(p.split("."), v) for p, v in path_to_values.items()]
                )
            elif isinstance(obj, Struct):
                return _fbthrift_struct_update_nested_field(
                    obj,
                    [(p.split("."), v) for p, v in path_to_values.items()]
                )
        except (AttributeError, TypeError) as e:
            # Unify different exception types to ValueError
            raise ValueError(e)

        # obj is not a py3 or python Struct
        raise TypeError("`update_nested_field` requires thrift.py3.Struct or thrift.python.Struct")

    # make the __module__ customizable
    # this just enables the slot; impl here is ignored
    def __module__(cls):
        pass

    def __iter__(cls):
        for i in range(cls._fbthrift_get_struct_size()):
            yield cls._fbthrift_get_field_name_by_index(i), None

    def __dir__(cls):
        return tuple(name for name, _ in cls) + ("__iter__", )

    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_StructOrUnion):
            return inst._fbthrift_auto_migrate_enabled()
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_StructOrUnion):
            return sub._fbthrift_auto_migrate_enabled()
        return super().__subclasscheck__(sub)


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
    __module__ = _fbthrift__module_name__

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
            try:
                yield name, getattr(self, name)
            except AttributeError:
                if name.startswith("__"):
                    mangled = f"_{self.__class__.__name__}{name}"
                    yield name, getattr(self, mangled)
                else:
                    raise

    def __dir__(self):
        return dir(type(self))

    cdef bint _fbthrift_noncomparable_eq(self, other):
        if self is other:
            return True
        name = None  # as sentinel to indicate if no fields
        for name, value in self.__iter__():
            if value != getattr(other, name):
                return False
        # in case of no fields, only True when identity equal (beginning).
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


# used by Union.fromValue to avoid matching a python `float` (64-bit) to
# a thrift `float` (32-bit), when doing so would cause precision loss
def _fbthrift__is_float32(double f64):
    cdef float f32 = f64
    return f32 == f64

# use to replicate thrift-py3 rounding of `float` (32-bit) fields
def _fbthrift__round_float32(double f64):
    return <float> f64


cdef class Union(Struct):
    """
    Base class for all thrift Unions
    """
    __module__ = _fbthrift__module_name__

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


class UnionMeta(type):
    def __new__(cls, name, bases, classdict):
        # enforce no inheritance, for in-place migrated exceptions
        for base in bases:
            if base is Union or not hasattr(base, '_FBTHRIFT__PYTHON_CLASS'):
                continue
            raise TypeError(
                f"Inheritance of thrift-generated {base.__name__} from {name}"
                " is deprecated. Please use composition."
            )

        return type.__new__(cls, name, bases, classdict)


    def __dir__(cls):
        return [
            cls._fbthrift_get_field_name_by_index(i)
            for i in range(cls._fbthrift_get_struct_size())
        ] + ["type", "value"]

    # make the __module__ customizeable
    # this just enables the slot; impl here is ignored
    def __module__(cls):
        pass

    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_Union):
            return inst._fbthrift_auto_migrate_enabled()
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_Union):
            return sub._fbthrift_auto_migrate_enabled()
        return super().__subclasscheck__(sub)



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

class ContainerMeta(type):
    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_Container):
            return _fbthrift__is_py3_auto_migrated
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_Container):
            return _fbthrift__is_py3_auto_migrated
        return super().__subclasscheck__(sub)

SetMetaClass(<PyTypeObject*> Container, <PyTypeObject*> ContainerMeta)

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


class ListMeta(type):
    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_List):
            return _fbthrift__is_py3_auto_migrated
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_List):
            return _fbthrift__is_py3_auto_migrated
        return super().__subclasscheck__(sub)


SetMetaClass(<PyTypeObject*> List, <PyTypeObject*> ListMeta)


cdef class _SetPrivateCtorToken:
    pass


_fbthrift_set_private_ctor = _SetPrivateCtorToken()

@cython.auto_pickle(False)
cdef class Set(Container):
    """
    Base class for pure python thrift sets
    """
    def __init__(self, frozenset py_obj not None, child_cls not None):
        self._py_obj = py_obj
        self._child_cls = child_cls

    def __len__(self):
        return len(self._py_obj)

    def __hash__(self):
        return super().__hash__()

    def __reduce__(self):
        return (type(self), (set(self), ))

    def __repr__(self):
        if not self:
            return 'iset()'
        # historically these were stored in std::set
        # the `sorted` preserves old order
        return f'i{{{", ".join(sorted(map(repr, self)))}}}'

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

    def __contains__(self, item):
        item = self._child_cls._check_item_type_or_none(item)
        if not self or item is None:
            return False
        return item in self._py_obj

    def __iter__(self):
        if not self:
            return
        yield from self._py_obj

    def __richcmp__(self, other, int op):
        if isinstance(other, Set):
            return self._fbthrift_py_richcmp((<Set>other)._py_obj, op)
        return self._fbthrift_py_richcmp(other, op)

    cdef _fbthrift_py_richcmp(self, other, int op):
        if op == Py_LT:
            return pySet.__lt__(self._py_obj, other)
        elif op == Py_LE:
            return pySet.__le__(self._py_obj, other)
        elif op == Py_EQ:
            return pySet.__eq__(self._py_obj, other)
        elif op == Py_NE:
            return pySet.__ne__(self._py_obj, other)
        elif op == Py_GT:
            return pySet.__gt__(self._py_obj, other)
        elif op == Py_GE:
            return pySet.__ge__(self._py_obj, other)

    def __and__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj & _fbthrift_set_to_py_set(self, other)
        )

    def __or__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj | _fbthrift_set_to_py_set(self, other)
        )

    def __xor__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj ^ _fbthrift_set_to_py_set(self, other)
        )

    def __sub__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj - _fbthrift_set_to_py_set(self, other)
        )

    def __rand__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj & _fbthrift_set_to_py_set(self, other)
        )

    def __ror__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj | _fbthrift_set_to_py_set(self, other)
        )

    def __rxor__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            self._py_obj ^ _fbthrift_set_to_py_set(self, other)
        )

    def __rsub__(Set self, other):
        return _py_set_to_fbthrift_set_unchecked(
            self,
            _fbthrift_set_to_py_set(self, other) - self._py_obj
        )

cdef inline _fbthrift_set_to_py_set(Set self, other):
    if isinstance(other, self._child_cls):
        return (<Set>other)._py_obj
    # try converting to self type
    return (<Set>self._child_cls(other))._py_obj

cdef inline _py_set_to_fbthrift_set_unchecked(Set self, py_set):
    return self._child_cls(py_set, _fbthrift_set_private_ctor)

class SetMeta(type):
    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_Set):
            return _fbthrift__is_py3_auto_migrated
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_Set):
            return _fbthrift__is_py3_auto_migrated
        return super().__subclasscheck__(sub)


SetMetaClass(<PyTypeObject*> Set, <PyTypeObject*> SetMeta)

cdef class _MapPrivateCtorToken:
    pass

_fbthrift_map_private_ctor = _MapPrivateCtorToken()

@cython.auto_pickle(False)
cdef class Map(Container):
    """
    Base class for pure python thrift maps
    """
    def __init__(self, dict py_obj not None, child_cls not None):
        self._py_obj = py_obj
        self._child_cls = child_cls

    def __len__(Map self):
        return len(self._py_obj)

    def __eq__(Map self, other):
        if not isinstance(other, Mapping):
            return False
        if len(self._py_obj) != len(other):
            return False

        for key in self._py_obj:
            if key not in other:
                return False
            if other[key] != self._py_obj[key]:
                return False

        return True

    def __ne__(Map self, other):
        return not self.__eq__(other)

    def __hash__(Map self):
        if not self._fbthrift_hash:
            self._fbthrift_hash = hash(tuple(self.items()))
        return self._fbthrift_hash

    def __repr__(Map self):
        if not self:
            return 'i{}'
        # print in sorted order for backward compatibility
        if self._child_cls._FBTHRIFT_USE_SORTED_REPR:
            try:
                key_val = sorted(self.items(), key=lambda x: x[0])
            except TypeError as e:  # e.g., BadEnum
                if cFollyIsDebug:
                    warnings.warn(
                        f"thrift.py3.types.Map: Failed to sort map keys: {e}",
                        RuntimeWarning,
                    )
                key_val = self.items()
        else:
            key_val = self.items()
        return f'i{{{", ".join(map(lambda i: f"{repr(i[0])}: {repr(i[1])}", key_val))}}}'

    def __reduce__(Map self):
        return (type(self), (dict(self._py_obj), ))

    def __copy__(Map self):
        return self._child_cls(
            dict(self._py_obj),
            private_ctor_token=_fbthrift_map_private_ctor
        )

    def __contains__(Map self, key):
        key = self._child_cls._check_key_type_or_none(key)
        if not self or key is None:
            return False
        return key in self._py_obj

    def __getitem__(Map self, key):
        err = KeyError(f'{key}')
        key = self._child_cls._check_key_type_or_none(key)
        if key is None:
            raise err
        item = self._py_obj.get(key)
        if item is None:
            raise err
        return item

    def get(Map self, key, default=None):
        try:
            return self._py_obj[key]
        except KeyError:
            return default

    def __iter__(Map self):
        if not self:
            return
        yield from self._py_obj

    def keys(Map self):
        return self._py_obj.keys()

    def values(Map self):
        return self._py_obj.values()
    def items(Map self):
        return self._py_obj.items()

class MapMeta(type):
    def __instancecheck__(cls, inst):
        if isinstance(inst, _fbthrift_python_Map):
            return _fbthrift__is_py3_auto_migrated
        return super().__instancecheck__(inst)

    def __subclasscheck__(cls, sub):
        if issubclass(sub, _fbthrift_python_Map):
            return _fbthrift__is_py3_auto_migrated
        return super().__subclasscheck__(sub)


SetMetaClass(<PyTypeObject*> Map, <PyTypeObject*> MapMeta)

CompiledEnum = _fbthrift_python_Enum
Enum = _fbthrift_python_Enum
# I wanted to call the base class Enum, but there is a cython bug
# See https://github.com/cython/cython/issues/2474
# Will move when the bug is fixed

Flag = _fbthrift_python_Flag

cdef class StructFieldsSetter:
    cdef void set_field(self, const char* name, object val) except *:
        pass

# thrift-py3 drops kwargs with None value but thrift-python does not.
# This function emulates the thrift-py3 behavior for in-place migrate
def _fbthrift__filter_kwargs(kwargs, tuple field_names):
    cdef list bad_kwargs = []
    for key, val in kwargs.items():
        if val is None and key not in field_names:
            bad_kwargs.append(key)

    for bad_key in bad_kwargs:
        warnings.warn(
            f"Discarding unexpected kwarg set to None: {bad_key}",
            RuntimeWarning,
        )
        kwargs.pop(bad_key)

    return kwargs


cdef translate_cpp_enum_to_python(object EnumClass, int value):
    try:
        return EnumClass(value)
    except ValueError:
        return BadEnum(EnumClass, value)


cdef _is_python_structured(obj):
    return isinstance(obj, (_fbthrift_python_StructOrUnion, _fbthrift_python_GeneratedError))


cpdef _from_python_or_raise(thrift_value, str field_name, py3_type):
    if _is_python_structured(thrift_value):
        thrift_value = thrift_value._to_py3()
        if not isinstance(thrift_value, py3_type):
            raise TypeError(
                f"{field_name} is a thrift-python value of type {type(thrift_value) !r} "
                f"that can not be converted to {py3_type !r}."
            )
        return thrift_value
    else:
        raise TypeError(f'{field_name} is not a {py3_type !r}.')

cpdef _ensure_py3_or_raise(thrift_value, str field_name, py3_type):
    if thrift_value is None or isinstance(thrift_value, py3_type):
        return thrift_value
    return _from_python_or_raise(thrift_value, field_name, py3_type)

cpdef _ensure_py3_container_or_raise(thrift_value, py3_container_type):
    if thrift_value is None or isinstance(thrift_value, py3_container_type):
        return thrift_value
    return py3_container_type.from_python(thrift_value)
