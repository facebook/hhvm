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

__all__ = ['Struct', 'BadEnum', 'NOTSET', 'Union', 'Enum', 'Flag']


cdef __NotSet NOTSET = __NotSet.__new__(__NotSet)

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

@cython.auto_pickle(False)
cdef class List(Container):
    """
    Base class for all thrift lists
    """

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
            return self._get_slice(index_obj)
        return self._get_single_item(self._normalize_index(<int?>index_obj))

    def __contains__(self, item):
        try:
            self.index(item)
        except ValueError:
            return False
        return True

    def __iter__(self):
        for i in range(len(self)):
            yield self._get_single_item(i)

    def __reversed__(self):
        for i in reversed(range(len(self))):
            yield self._get_single_item(i)

    cdef int _normalize_index(self, int index) except *:
        cdef int size = len(self)
        # Convert a negative index
        if index < 0:
            index = size + index
        if index >= size or index < 0:
            raise IndexError('list index out of range')
        return index

    cdef _get_slice(self, slice index_obj):
        raise NotImplementedError()

    cdef _get_single_item(self, size_t index):
        raise NotImplementedError()

    cdef _check_item_type(self, item):
        raise NotImplementedError()

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

    cdef _check_key_type(self, key):
        raise NotImplementedError()



@cython.auto_pickle(False)
cdef class EnumData:
    @staticmethod
    cdef EnumData _fbthrift_create(cEnumData* ptr, py_type):
        cdef EnumData inst = EnumData.__new__(EnumData)
        inst._py_type = py_type
        inst._cpp_obj = unique_ptr[cEnumData](ptr)
        return inst

    cdef get_by_name(self, str name):
        cdef bytes name_bytes = name.encode("utf-8") # to keep the buffer alive
        cdef string_view name_sv = string_view(name_bytes)
        cdef pair[PyObjectPtr, cOptionalInt] r = self._cpp_obj.get().tryGetByName(name_sv)
        cdef PyObject* inst = r.first
        cdef optional[int] value = r.second
        if inst != NULL:
            return <object>inst
        if not value.has_value():
            raise AttributeError(f"'{self._py_type.__name__}' has no attribute '{name}'")
        return <object>self._add_to_cache(name, value.value())

    cdef get_by_value(self, int value):
        if value < -(1<<31) or value >= (1 << 31):
            self._value_error(value)
        cdef pair[PyObjectPtr, string_view] r = self._cpp_obj.get().tryGetByValue(value)
        cdef PyObject* inst = r.first
        cdef string_view name = r.second
        if inst != NULL:
            return <object>inst
        if name.data() == NULL:
            self._value_error(value)
        return <object>self._add_to_cache(sv_to_str(name), value)

    cdef PyObject* _add_to_cache(self, str name, int value) except *:
        new_inst = self._py_type.__new__(self._py_type, name, value, NOTSET)
        return self._cpp_obj.get().tryAddToCache(
            value,
            <PyObject*>new_inst
        )

    def get_all_names(self):
        cdef cEnumData* cpp_obj_ptr = self._cpp_obj.get()
        cdef cRange[const string_view*] names = cpp_obj_ptr.getNames()
        cdef string_view name
        for name in names:
            yield sv_to_str(cpp_obj_ptr.getPyName(string_view(name.data())))

    cdef int size(self):
        return self._cpp_obj.get().size()

    cdef void _value_error(self, int value) except *:
        raise ValueError(f"{value} is not a valid {self._py_type.__name__}")


@cython.auto_pickle(False)
cdef class EnumFlagsData(EnumData):

    @staticmethod
    cdef EnumFlagsData _fbthrift_create(cEnumFlagsData* ptr, py_type):
        cdef EnumFlagsData inst = EnumFlagsData.__new__(EnumFlagsData)
        inst._py_type = py_type
        inst._cpp_obj = unique_ptr[cEnumData](ptr)
        return inst

    cdef get_by_value(self, int value):
        cdef cEnumFlagsData* cpp_obj_ptr = down_cast_ptr[
            cEnumFlagsData, cEnumData](self._cpp_obj.get())
        if value < 0:
            value = cpp_obj_ptr.convertNegativeValue(value)
        cdef pair[PyObjectPtr, string_view] r = cpp_obj_ptr.tryGetByValue(value)
        cdef PyObject* inst = r.first
        cdef string_view name = r.second
        if inst != NULL:
            return <object>inst
        if name.data() == NULL:
            self._value_error(value)
        if not name.empty():
            # it's not a derived value
            return <object>self._add_to_cache(sv_to_str(name), value)
        # it's a derived value
        new_inst = self._py_type.__new__(
            self._py_type,
            cpp_obj_ptr.getNameForDerivedValue(value).decode("utf-8"),
            value,
            NOTSET,
        )
        return <object>cpp_obj_ptr.tryAddToFlagValuesCache(value, <PyObject*>new_inst)

    cdef get_invert(self, uint32_t value):
        cdef cEnumFlagsData* cpp_obj_ptr = down_cast_ptr[
            cEnumFlagsData, cEnumData](self._cpp_obj.get())
        return self.get_by_value(cpp_obj_ptr.getInvertValue(value))

@cython.auto_pickle(False)
cdef class UnionTypeEnumData(EnumData):

    @staticmethod
    cdef UnionTypeEnumData _fbthrift_create(cEnumData* ptr, py_type):
        cdef UnionTypeEnumData inst = UnionTypeEnumData.__new__(UnionTypeEnumData)
        inst._py_type = py_type
        inst._cpp_obj = unique_ptr[cEnumData](ptr)
        inst.__empty = py_type.__new__(py_type, "EMPTY", 0, NOTSET)
        return inst

    def get_all_names(self):
        yield "EMPTY"
        yield from EnumData.get_all_names(self)

    cdef get_by_name(self, str name):
        if name == "EMPTY":
            return self.__empty
        return EnumData.get_by_name(self, name)

    cdef get_by_value(self, int value):
        if value == 0:
            return self.__empty
        return EnumData.get_by_value(self, value)

    cdef int size(self):
        return EnumData.size(self) + 1  # for EMPTY


@cython.auto_pickle(False)
cdef class EnumMeta(type):
    def _fbthrift_get_by_value(cls, int value):
        return NotImplemented

    def _fbthrift_get_all_names(cls):
        return NotImplemented

    def __call__(cls, value):
        if isinstance(value, cls):
            return value
        if not isinstance(value, int):
            raise ValueError(f"{repr(value)} is not a valid {cls.__name__}")
        return cls._fbthrift_get_by_value(value)

    def __getitem__(cls, name):
        if type(name) is not str:
            if not isinstance(name, str):
                raise KeyError(name)
            name = str(name) # cast to str for Cython
        try:
            return getattr(cls, name)
        except AttributeError:
            raise KeyError(name)

    def __iter__(cls):
        for name in cls._fbthrift_get_all_names():
            yield getattr(cls, name)

    def __reversed__(cls):
        return reversed(iter(cls))

    def __contains__(cls, item):
        if not isinstance(item, cls):
            return False
        return item in cls.__iter__()

    def __len__(cls):
        return NotImplemented

    @property
    def __members__(cls):
        return MappingProxyType({inst.name: inst for inst in cls.__iter__()})

    def __dir__(cls):
        return ['__class__', '__doc__', '__members__', '__module__'] + [name for name in cls._fbthrift_get_all_names()]


@cython.auto_pickle(False)
cdef class CompiledEnum:
    """
    Base class for all thrift Enum
    """
    def __cinit__(self, name, value, __NotSet guard = None):
        if guard is not NOTSET:
            raise TypeError('__new__ is disabled in the interest of type-safety')
        self.name = name
        self.value = value
        self._fbthrift_hash = hash(name)
        self.__str = f"{type(self).__name__}.{name}"
        self.__repr = f"<{self.__str}: {value}>"

    cdef get_by_name(self, str name):
        return NotImplemented

    def __getattribute__(self, str name not None):
        if name.startswith("__") or name in ("name", "value", "_to_python", "_to_py3", "_to_py_deprecated"):
            return super().__getattribute__(name)
        return self.get_by_name(name)

    def __repr__(self):
        return self.__repr

    def __str__(self):
        return self.__str

    def __int__(self):
        return self.value

    def __index__(self):
        return self.value

    def __hash__(self):
        return self._fbthrift_hash

    def __reduce__(self):
        return type(self), (self.value,)

    def __eq__(self, other):
        if type(other) is not type(self):
            warnings.warn(f"comparison not supported between instances of { type(self) } and {type(other)}", RuntimeWarning, stacklevel=2)
            return False
        return self is other

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        raise NotImplementedError()



Enum = CompiledEnum
# I wanted to call the base class Enum, but there is a cython bug
# See https://github.com/cython/cython/issues/2474
# Will move when the bug is fixed


@cython.auto_pickle(False)
cdef class Flag(CompiledEnum):
    """
    Base class for all thrift Flag
    """
    def __contains__(self, other):
        if type(other) is not type(self):
            return NotImplemented
        return other.value & self.value == other.value

    def __bool__(self):
        return bool(self.value)

    def __or__(self, other):
        cls = type(self)
        if type(other) is not cls:
            return NotImplemented
        return cls(self.value | other.value)

    def __and__(self, other):
        cls = type(self)
        if type(other) is not cls:
            return NotImplemented
        return cls(self.value & other.value)

    def __xor__(self, other):
        cls = type(self)
        if type(other) is not cls:
            return NotImplemented
        return cls(self.value ^ other.value)

    def __invert__(self):
        return NotImplemented


cdef class BadEnum:
    """
    This represents a BadEnum value from thrift.
    So an out of date thrift definition or a default value that is not
    in the enum
    """

    def __init__(self, the_enum, value):
        self._enum = the_enum
        self.value = value
        self.name = '#INVALID#'

    def __repr__(self):
        return f'<{self.enum.__name__}.{self.name}: {self.value}>'

    def __int__(self):
        return self.value

    def __index__(self):
        return self.value

    @property
    def enum(self):
        return self._enum

    def __reduce__(self):
        return BadEnum, (self._enum, self.value)

    def __hash__(self):
        return hash((self._enum, self.value))

    def __eq__(self, other):
        if not isinstance(other, BadEnum):
            return False
        cdef BadEnum cother = <BadEnum>other
        return (self._enum, self.value) == (cother._enum, cother.value)

    def __ne__(self, other):
        return not(self == other)

cdef class StructFieldsSetter:
    cdef void set_field(self, const char* name, object val) except *:
        pass


cdef translate_cpp_enum_to_python(object EnumClass, int value):
    try:
        return EnumClass(value)
    except ValueError:
        return BadEnum(EnumClass, value)


try:
    import thrift.python.types
    def _is_python_struct(obj):
        return isinstance(obj, thrift.python.types.StructOrUnion)
    def _is_python_enum(obj):
        return isinstance(obj, thrift.python.types.Enum)
except ImportError:
    def _is_python_struct(obj):
        return False
    def _is_python_enum(obj):
        return False
