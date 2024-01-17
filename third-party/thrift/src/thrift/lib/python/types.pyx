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
from collections.abc import Iterable, Mapping, Sequence, Set as pySet
from types import MappingProxyType

from folly.iobuf cimport cIOBuf, IOBuf, from_unique_ptr
from libcpp.utility cimport move as cmove
from libcpp.memory cimport make_unique
from cpython cimport bool as pbool, int as pint, float as pfloat
from cpython.object cimport Py_LT, Py_EQ, PyCallable_Check
from cpython.tuple cimport PyTuple_New, PyTuple_SET_ITEM, PyTuple_GET_ITEM, PyTuple_Check
from cpython.ref cimport Py_INCREF, Py_DECREF
from cpython.unicode cimport PyUnicode_AsUTF8String, PyUnicode_FromEncodedObject
from cython.operator cimport dereference as deref

import copy
import cython
import enum
import functools
import itertools
import threading

from thrift.python.exceptions cimport GeneratedError
from thrift.python.serializer cimport cserialize, cdeserialize

try:
    import thrift.py3.types
    def _is_py3_struct(obj):
        return isinstance(obj, thrift.py3.types.Struct)
    def _is_py3_enum(obj):
        return isinstance(obj, thrift.py3.types.Enum)
except ImportError:
    def _is_py3_struct(obj):
        return False
    def _is_py3_enum(obj):
        return False


try:
    import cinder
    def _make_cached_property(f, owner, name):
        return cinder.cached_property(f, getattr(owner, name))
except ImportError:
    # On MacOS/Windows where Cinder is not available, degrade to not-cached property.
    # Field values are always cached in Cython level, we just lose the Python level cache.
    def _make_cached_property(f, owner, name):
        prop = property(f)
        prop.__set_name__(owner, name)
        return prop


cdef public api cIOBuf* get_cIOBuf(object buf):
    if buf is None:
        return NULL
    return (<IOBuf>buf)._ours.get()

cdef public api object create_IOBuf(unique_ptr[cIOBuf] ciobuf):
    return from_unique_ptr(cmove(ciobuf))

cdef class TypeInfo:
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, pytypes):
        cdef TypeInfo inst = TypeInfo.__new__(TypeInfo)
        inst.cpp_obj = &cpp_obj
        inst.pytypes = pytypes
        return inst

    # validate and convert to format serializer may understand
    def to_internal_data(self, object value):
        if not isinstance(value, self.pytypes):
            raise TypeError(f'value {value} is not a {self.pytypes !r}, is actually of type {type(value)}')
        return value

    # convert deserialized data to user format
    def to_python_value(self, object value):
        return value

    def to_container_value(self, object value):
        return self.to_internal_data(value)


cdef class IntegerTypeInfo:
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, min_value, max_value):
        cdef IntegerTypeInfo inst = IntegerTypeInfo.__new__(IntegerTypeInfo)
        inst.cpp_obj = &cpp_obj
        inst.min_value = min_value
        inst.max_value = max_value
        return inst

    # validate and convert to format serializer may understand
    def to_internal_data(self, object value not None):
        if not isinstance(value, pint):
            raise TypeError(f"value {value} is not a <class 'int'>, is actually of type {type(value)}")
        cdef int64_t cvalue = value
        if cvalue > self.max_value or cvalue < self.min_value:
            raise OverflowError()
        return value

    # convert deserialized data to user format
    def to_python_value(self, object value):
        return value

    def to_container_value(self, object value not None):
        return self.to_internal_data(value)


cdef class StringTypeInfo:
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj):
        cdef StringTypeInfo inst = StringTypeInfo.__new__(StringTypeInfo)
        inst.cpp_obj = &cpp_obj
        return inst

    # validate and convert to format serializer may understand
    def to_internal_data(self, object value not None):
        return PyUnicode_AsUTF8String(value)

    # convert deserialized data to user format
    def to_python_value(self, object value):
        return PyUnicode_FromEncodedObject(value, NULL, NULL)

    def to_container_value(self, object value not None):
        if not isinstance(value, str):
            raise TypeError(f"value {value} is not a <class 'str'>, is actually of type {type(value)}")
        return value


cdef class IOBufTypeInfo:
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj):
        cdef IOBufTypeInfo inst = IOBufTypeInfo.__new__(IOBufTypeInfo)
        inst.cpp_obj = &cpp_obj
        return inst

    def to_internal_data(self, IOBuf value):
        return value

    def to_python_value(self, object value):
        return value

    def to_container_value(self, IOBuf value):
        return value


typeinfo_bool = TypeInfo.create(boolTypeInfo, (pbool,))
typeinfo_byte = IntegerTypeInfo.create(byteTypeInfo, -128, 127)
typeinfo_i16 = IntegerTypeInfo.create(i16TypeInfo, -1<<15, (1<<15)-1)
typeinfo_i32 = IntegerTypeInfo.create(i32TypeInfo, -1<<31, (1<<31)-1)
typeinfo_i64 = IntegerTypeInfo.create(i64TypeInfo, -1<<63, (1<<63)-1)
typeinfo_double = TypeInfo.create(doubleTypeInfo, (pfloat, pint))
typeinfo_float = TypeInfo.create(floatTypeInfo, (pfloat, pint))
typeinfo_string = StringTypeInfo.create(stringTypeInfo)
typeinfo_binary = TypeInfo.create(binaryTypeInfo, (bytes,))
typeinfo_iobuf = IOBufTypeInfo.create(iobufTypeInfo)


StructOrError = cython.fused_type(Struct, GeneratedError)


cdef class StructInfo:
    """
    Stores information for a specific Thrift Struct class.

    Instance Variables:
        fields: Set containing the specifications of each field in this Thrift
            struct. Each field is represented as a tuple with the following
            structure:

            (
                id (int): The field ID specified in the IDL.

                qualifier (FieldQualifier enum): Unqualified, Optional, ...

                name (str); The name of the Thrift struct, as specified in the
                    IDL.

                type_info: Type information object corresponding to this field
                    (eg. typeinfo_string, ListTypeInfo, SetTypeInfo, etc.), OR
                    a callable (eg. lambda) that returns such an object (useful
                        to handle types with dependencies in arbitrary order).

                default_value: custom default value specified in the IDL, or
                    None. If present, this can also be a callable which will be
                    called (exactly once) to obtain the default value.

                adapter_info: if the field has an adapter, or None.

                is_primitive (bool): Whether the field has a "orimitive" type,
                    such as: bool, byte, i16, i32, i62, double, float.
            )

        cpp_obj: cDynamicStructInfo for this struct.

        type_infos: Tuple whose size matches the number of fields in the Thrift
            struct. Initialized by calling `fill()`.

        name_to_index: Dict[str (field name), int (index in `fields`).].
            Initialized by calling `fill()`.
    """

    def __cinit__(self, name: str, fields):
        """
        Stores information for a Thrift Struct class with the given name.

        Args:
            name (str): Name of the Thrift Struct (as specified in IDL)
            fields (Set[Tuple]): Field spec tuples. See class docstring above.
        """
        self.fields = fields
        cdef int16_t num_fields = len(fields)
        self.cpp_obj = make_unique[cDynamicStructInfo](
            PyUnicode_AsUTF8(name),
            num_fields,
            False, # isUnion
        )
        self.type_infos = PyTuple_New(num_fields)
        self.name_to_index = {}

    cdef void fill(self) except *:
        """
        Completes the initialization of this instance by populating all
        information relative to this Struct's fields.

        Must be called exactly once, after `__cinit__()` but before any other
        method.

        Upon successful return, the following attributes are fully initialized:
          - `self.type_infos`
          - `self.name_to_index`
          - field infos in the `self.cpp_obj` (see
            `DynamicStructInfo::addFieldInfo()`)
        """

        cdef cDynamicStructInfo* dynamic_struct_info = self.cpp_obj.get()
        type_infos = self.type_infos
        for idx, (id, qualifier, name, type_info, *_) in enumerate(self.fields):
            # type_info can be a lambda function so types with dependencies
            # won't need to be defined in order, see class docstring above.
            if PyCallable_Check(type_info):
                type_info = type_info()

            Py_INCREF(type_info)
            PyTuple_SET_ITEM(type_infos, idx, type_info)
            self.name_to_index[name] = idx
            dynamic_struct_info.addFieldInfo(
                id, qualifier, PyUnicode_AsUTF8(name), getCTypeInfo(type_info)
            )

    cdef void store_field_values(self) except *:
        """
        Initializes the default values of fields in this Struct.

        Upon successful return, the field value(s) in `self.cpp_obj` are
        iniitalized (see `DynamicStructInfo::addFieldValue()`).
        """
        cdef cDynamicStructInfo* dynamic_struct_info = self.cpp_obj.get()
        for idx, field in enumerate(self.fields):
            default_value = field[4]
            if default_value is None:
                continue
            if callable(default_value):
                default_value = default_value()
            type_info = self.type_infos[idx]
            if isinstance(type_info, AdaptedTypeInfo):
                type_info = (<AdaptedTypeInfo>type_info)._orig_type_info
            default_value = type_info.to_internal_data(default_value)
            dynamic_struct_info.addFieldValue(idx, default_value)


cdef class UnionInfo:
    def __cinit__(self, name, fields):
        self.fields = fields
        self.cpp_obj = make_unique[cDynamicStructInfo](
            PyUnicode_AsUTF8(name),
            len(fields),
            True,
        )
        self.type_infos = {}
        self.id_to_adapter_info = {}
        self.name_to_index = {}

    cdef void fill(self) except *:
        cdef cDynamicStructInfo* dynamic_struct_info = self.cpp_obj.get()
        for idx, (id, qualifier, name, type_info, _, adapter_info, _) in enumerate(self.fields):
            # type_info can be a lambda function so types with dependencies
            # won't need to be defined in order
            if callable(type_info):
                type_info = type_info()
            self.type_infos[id] = type_info
            self.id_to_adapter_info[id] = adapter_info
            self.name_to_index[name] = idx
            dynamic_struct_info.addFieldInfo(
                id, qualifier, PyUnicode_AsUTF8(name), getCTypeInfo(type_info)
            )


cdef const cTypeInfo* getCTypeInfo(type_info):
        if isinstance(type_info, TypeInfo):
            return (<TypeInfo>type_info).cpp_obj
        if isinstance(type_info, StringTypeInfo):
            return (<StringTypeInfo>type_info).cpp_obj
        if isinstance(type_info, IOBufTypeInfo):
            return (<IOBufTypeInfo>type_info).cpp_obj
        if isinstance(type_info, IntegerTypeInfo):
            return (<IntegerTypeInfo>type_info).cpp_obj
        if isinstance(type_info, StructTypeInfo):
            return (<StructTypeInfo>type_info).get()
        if isinstance(type_info, ListTypeInfo):
            return (<ListTypeInfo>type_info).get()
        if isinstance(type_info, SetTypeInfo):
            return (<SetTypeInfo>type_info).get()
        if isinstance(type_info, MapTypeInfo):
            return (<MapTypeInfo>type_info).get()
        if isinstance(type_info, EnumTypeInfo):
            return &i32TypeInfo
        if isinstance(type_info, AdaptedTypeInfo):
            return getCTypeInfo((<AdaptedTypeInfo>type_info)._orig_type_info)


cdef to_container_elements_no_convert(type_info):
    return isinstance(type_info, (TypeInfo, IntegerTypeInfo)) or type_info is typeinfo_iobuf


cdef class ListTypeInfo:
    def __cinit__(self, val_info):
        self.val_info = val_info
        self.cpp_obj = make_unique[cListTypeInfo](getCTypeInfo(val_info))

    cdef const cTypeInfo* get(self):
        return self.cpp_obj.get().get()

    # validate and convert to format serializer may understand
    def to_internal_data(self, value not None):
        return tuple(self.val_info.to_internal_data(v) for v in value)

    # convert deserialized data to user format
    def to_python_value(self, object value):
        cdef List inst = List.__new__(List)
        inst._fbthrift_val_info = self.val_info
        inst._fbthrift_elements = value if to_container_elements_no_convert(self.val_info) else tuple(self.val_info.to_python_value(v) for v in value)
        return inst

    def to_container_value(self, object value not None):
        if isinstance(value, List):
            return value
        return List(self.val_info, value)


cdef class SetTypeInfo:
    def __cinit__(self, val_info):
        self.val_info = val_info
        self.cpp_obj = make_unique[cSetTypeInfo](getCTypeInfo(val_info))

    cdef const cTypeInfo* get(self):
        return self.cpp_obj.get().get()

    # validate and convert to format serializer may understand
    def to_internal_data(self, value not None):
        return frozenset(self.val_info.to_internal_data(v) for v in value)

    # convert deserialized data to user format
    def to_python_value(self, object value):
        cdef Set inst = Set.__new__(Set)
        inst._fbthrift_val_info = self.val_info
        inst._fbthrift_elements = value if to_container_elements_no_convert(self.val_info) else frozenset(self.val_info.to_python_value(v) for v in value)
        return inst

    def to_container_value(self, object value not None):
        if isinstance(value, Set):
            return value
        return Set(self.val_info, value)


cdef class MapTypeInfo:
    def __cinit__(self, key_info, val_info):
        self.key_info = key_info
        self.val_info = val_info
        self.cpp_obj = make_unique[cMapTypeInfo](
            getCTypeInfo(key_info),
            getCTypeInfo(val_info),
        )

    cdef const cTypeInfo* get(self):
        return self.cpp_obj.get().get()

    # validate and convert to format serializer may understand
    def to_internal_data(self, value not None):
        return tuple(
            (self.key_info.to_internal_data(k), self.val_info.to_internal_data(v)) for k, v in value.items()
        )

    # convert deserialized data to user format
    def to_python_value(self, object value):
        cdef Map inst = Map.__new__(Map)
        inst._fbthrift_key_info = self.key_info
        inst._fbthrift_val_info = self.val_info
        inst._fbthrift_elements = {
            self.key_info.to_python_value(k): self.val_info.to_python_value(v) for k, v in value
        }
        return inst

    def to_container_value(self, object value not None):
        if isinstance(value, Map):
            return value
        return Map(self.key_info, self.val_info, value)


cdef class StructTypeInfo:
    def __cinit__(self, klass):
        self._class = klass
        info = klass._fbthrift_struct_info
        cdef cDynamicStructInfo* c_struct_info
        if isinstance(info, UnionInfo):
            self.is_union = True
            c_struct_info = (<UnionInfo>info).cpp_obj.get()
        else:
            self.is_union = False
            c_struct_info = (<StructInfo>info).cpp_obj.get()
        self.cpp_obj = createStructTypeInfo(
            deref(c_struct_info)
        )

    cdef const cTypeInfo* get(self):
        return &self.cpp_obj

    # validate and convert to format serializer may understand
    def to_internal_data(self, value not None):
        if not isinstance(value, self._class):
            if _is_py3_struct(value):
                value = value._to_python()
                if not isinstance(value, self._class):
                    raise TypeError(f"value {value} is a py3 struct of type {type(value)}, can not be converted to {self._class !r}.")
            else:
                raise TypeError(f"value {value} is not a {self._class !r}, is actually of type {type(value)}.")
        if isinstance(value, Struct):
            return (<Struct>value)._fbthrift_data
        if isinstance(value, GeneratedError):
            return (<GeneratedError>value)._fbthrift_data
        if isinstance(value, Union):
            return (<Union>value)._fbthrift_data
        raise TypeError(f"{self._class} not supported")

    # convert deserialized data to user format
    def to_python_value(self, object value):
        return self._class._fbthrift_create(value)

    def to_container_value(self, object value not None):
        if not isinstance(value, self._class):
            raise TypeError(f"value {value} is not a {self._class !r}, is actually of type {type(value)}.")
        return value


cdef class EnumTypeInfo:
    def __cinit__(self, klass):
        self._class = klass

    # validate and convert to format serializer may understand
    def to_internal_data(self, value not None):
        if isinstance(value, BadEnum):
            return int(value)
        if not isinstance(value, self._class):
            if _is_py3_enum(value):
                value = value._to_python()
                if not isinstance(value, self._class):
                    raise TypeError(f"value {value} is a py3 enum of type {type(value)}, can not be converted to {self._class !r}.")
            else:
                raise TypeError(f"value {value} is not '{self._class}', is actually of type {type(value)}.")
        return value._fbthrift_value_

    # convert deserialized data to user format
    def to_python_value(self, object value):
        try:
            return self._class(value)
        except ValueError:
            return BadEnum(self._class, value)

    def to_container_value(self, object value not None):
        if not isinstance(value, self._class):
            raise TypeError(f"value {value} is not '{self._class}', is actually of type {type(value)}.")
        return value


cdef class AdaptedTypeInfo:
    def __cinit__(self, orig_type_info, adapter_info, transitive_annotation):
        self._orig_type_info = orig_type_info
        self._adapter_info = adapter_info
        self._transitive_annotation = transitive_annotation

    # validate and convert to format serializer may understand
    def to_internal_data(self, value not None):
        return self._orig_type_info.to_internal_data(
            self._adapter_info.to_thrift(
                value,
                transitive_annotation=self._transitive_annotation(),
            )
        )

    # convert deserialized data to user format
    def to_python_value(self, object value):
        return self._adapter_info.from_thrift(
            self._orig_type_info.to_python_value(value),
            transitive_annotation=self._transitive_annotation(),
        )

    def to_container_value(self, object value not None):
        return value


cdef void set_struct_field(tuple struct_tuple, int16_t index, value) except *:
    """
    Updates the given `struct_tuple` to have the given `value` for the field at
    the given `index`.

    The "isset" byte for the corresponding field (i.e., the `index`-th byte of
     the first element of `struct_tuple` is set to 1.

     Args:
        struct_tuple: see `createStructTupleWithDefaultValues()`
        index: field index, as defined by its insertion order in the parent
            `StructInfo` (this is not the field id).
        value: new value for this field.

    """
    setStructIsset(struct_tuple, index, 1)
    old_value = struct_tuple[index + 1]
    Py_INCREF(value)
    PyTuple_SET_ITEM(struct_tuple, index + 1, value)
    Py_DECREF(old_value)

cdef class StructOrUnion:
    cdef folly.iobuf.IOBuf _serialize(Struct self, Protocol proto):
        raise NotImplementedError("Not implemented on base StructOrUnion class")
    cdef uint32_t _deserialize(Struct self, folly.iobuf.IOBuf buf, Protocol proto) except? 0:
        raise NotImplementedError("Not implemented on base StructOrUnion class")
    cdef _fbthrift_get_field_value(self, int16_t index):
        raise NotImplementedError("Not implemented on base StructOrUnion class")

def _unpickle_struct(klass, bytes data):
    cdef IOBuf iobuf = IOBuf(data)
    inst = klass.__new__(klass)
    (<Struct>inst)._deserialize(iobuf, Protocol.COMPACT)
    return inst

cdef api object _get_fbthrift_data(object struct_or_union):
    return (<StructOrUnion> struct_or_union)._fbthrift_data

cdef api object _get_exception_fbthrift_data(object generated_error):
    return (<GeneratedError> generated_error)._fbthrift_data

cdef class Struct(StructOrUnion):
    """
    Base class for all generated classes corresponding to a Thrift struct in
    thrift-python.

    Instance variables:
        _fbthrift_data: "struct tuple" that holds the "isset" flag array and
            values for all fields. See `createStructTupleWithDefaultValues()`.

        _fbthrift_field_cache: Tuple
    """

    def __cinit__(self):
        cdef StructInfo struct_info = self._fbthrift_struct_info
        self._fbthrift_data = createStructTupleWithDefaultValues(
            struct_info.cpp_obj.get().getStructInfo()
        )
        self._fbthrift_field_cache = PyTuple_New(len(struct_info.fields))

    def __init__(self, **kwargs):
        """

        Args:
            **kwargs: names and values of the Thrift fields to set for this
                 instance. All names must match declared fields of this Thrift
                 Struct (or a `TypeError` will be raised).
        """
        cdef StructInfo struct_info = self._fbthrift_struct_info
        for name, value in kwargs.items():
            field_index = struct_info.name_to_index.get(name)
            if field_index is None:
                raise TypeError(f"__init__() got an unexpected keyword argument '{name}'")
            if value is None:
                continue

            field_spec = struct_info.fields[field_index]

            # Handle field w/ adapter
            adapter_info = field_spec[5]
            if adapter_info is not None:
                adapter_class, transitive_annotation = adapter_info
                field_id = field_spec[0]
                value = adapter_class.to_thrift_field(
                    value,
                    field_id,
                    self,
                    transitive_annotation=transitive_annotation(),
                )

            set_struct_field(
                self._fbthrift_data,
                field_index,
                struct_info.type_infos[field_index].to_internal_data(value),
            )
        self._fbthrift_populate_primitive_fields()

    def __call__(self, **kwargs):
        if not kwargs:
            return self
        cdef StructInfo struct_info = self._fbthrift_struct_info
        tp = type(self)
        cdef Struct new_inst = tp.__new__(tp)
        not_found = object()
        isset_flags = self._fbthrift_data[0]
        for field_name, field_index in struct_info.name_to_index.items():
            value = kwargs.pop(field_name, not_found)
            if value is None:  # reset to default value, no change needed
                continue
            if value is not_found:  # copy the old value if needed
                if isset_flags[field_index] == 0:
                    # old field not set, so keep default
                    continue
                value_to_copy = self._fbthrift_data[field_index + 1]
            else:  # new assigned value
                field_spec = struct_info.fields[field_index]
                adapter_info = field_spec[5]
                if adapter_info:
                    adapter_class, transitive_annotation = adapter_info
                    field_id = field_spec[0]
                    value = adapter_class.to_thrift_field(
                        value,
                        field_id,
                        self,
                        transitive_annotation=transitive_annotation(),
                    )
                value_to_copy = struct_info.type_infos[field_index].to_internal_data(value)
            set_struct_field(new_inst._fbthrift_data, field_index, value_to_copy)
        if kwargs:
            raise TypeError(f"__call__() got an expected keyword argument '{kwargs.keys()[0]}'")
        new_inst._fbthrift_populate_primitive_fields()
        return new_inst

    def __copy__(Struct self):
        return self

    def __deepcopy__(Struct self, _memo):
        return self

    def __eq__(Struct self, other):
        if type(other) != type(self):
            return False
        for name, value in self:
            if value != getattr(other, name):
                return False
        return True

    def __lt__(self, other):
        if type(self) != type(other):
            return NotImplemented
        for name, value in self:
            other_value = getattr(other, name)
            if value == other_value:
                continue
            return value < other_value
        return False

    def __le__(self, other):
        if type(self) != type(other):
            return NotImplemented
        for name, value in self:
            other_value = getattr(other, name)
            if value == other_value:
                continue
            return value < other_value
        return True

    def __hash__(Struct self):
        value_tuple = tuple(v for _, v in self)
        return hash(value_tuple if value_tuple else type(self))

    def __iter__(self):
        cdef StructInfo info = self._fbthrift_struct_info
        for name in info.name_to_index:
            yield name, getattr(self, name)

    def __dir__(self):
        return dir(type(self))

    def __repr__(self):
        fields = ", ".join(f"{name}={repr(value)}" for name, value in self)
        return f"{type(self).__name__}({fields})"

    def __reduce__(self):
        return (_unpickle_struct, (type(self), b''.join(self._serialize(Protocol.COMPACT))))

    cdef folly.iobuf.IOBuf _serialize(self, Protocol proto):
        cdef StructInfo info = self._fbthrift_struct_info
        return folly.iobuf.from_unique_ptr(
            cmove(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _deserialize(self, folly.iobuf.IOBuf buf, Protocol proto) except? 0:
        cdef StructInfo info = self._fbthrift_struct_info
        cdef uint32_t len = cdeserialize(deref(info.cpp_obj), buf._this, self._fbthrift_data, proto)
        self._fbthrift_populate_primitive_fields()
        return len

    cdef _fbthrift_get_field_value(self, int16_t index):
        cdef PyObject* value = PyTuple_GET_ITEM(self._fbthrift_field_cache, index)
        if value != NULL:
            return <object>value
        cdef StructInfo info = self._fbthrift_struct_info
        field_info = info.fields[index]
        field_id = field_info[0]
        adapter_info = field_info[5]
        data = self._fbthrift_data[index + 1]
        if data is not None:
            py_value = info.type_infos[index].to_python_value(data)
            if adapter_info is not None:
                adapter_class, transitive_annotation = adapter_info
                py_value = adapter_class.from_thrift_field(
                    py_value,
                    field_id,
                    self,
                    transitive_annotation=transitive_annotation(),
                )
        else:
            py_value = None
        PyTuple_SET_ITEM(self._fbthrift_field_cache, index, py_value)
        Py_INCREF(py_value)
        return py_value

    cdef _fbthrift_populate_primitive_fields(self):
        """
        Copies the values of all primitive fields from the underlying struct
        tuple (`_fbthrift_primitive_types`), or None if n/a, to instance
        attributes with the same names.
        """
        for index, name, type_info in self._fbthrift_primitive_types:
            data = self._fbthrift_data[index + 1]
            val = type_info.to_python_value(data) if data is not None else None
            object.__setattr__(self, name, val)

    cdef _fbthrift_fully_populate_cache(self):
        for _, field in self:
            if isinstance(field, (Sequence, Set)):
                for elem in field:
                    if not isinstance(elem, Struct):
                        break
                    (<Struct>elem)._fbthrift_fully_populate_cache()
            elif isinstance(field, Mapping):
                # keys are already materialized upon hashing
                for elem in field.values():
                    if not isinstance(elem, Struct):
                        break
                    (<Struct>elem)._fbthrift_fully_populate_cache()

    @classmethod
    def _fbthrift_create(cls, data):
        cdef Struct inst = cls.__new__(cls)
        inst._fbthrift_data = data
        inst._fbthrift_populate_primitive_fields()
        return inst

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__() -> str:
        return NotImplementedError()

    @staticmethod
    def __get_thrift_uri__():
        return NotImplementedError()

def _unpickle_union(klass, bytes data):
    cdef IOBuf iobuf = IOBuf(data)
    inst = klass.__new__(klass)
    (<Union>inst)._deserialize(iobuf, Protocol.COMPACT)
    return inst

cdef class Union(StructOrUnion):
    def __cinit__(self):
        self._fbthrift_data = createUnionTuple()

    def __init__(self, **kwargs):
        if not kwargs:
            self._fbthrift_load_cache()
            return
        if len(kwargs) != 1:
            raise TypeError("__init__() of a union may only take one keyword argument")
        for name, value in kwargs.items():
            break
        try:
            tpe = self.Type[name]
        except KeyError:
            raise TypeError(f"__init__() got an unexpected keyword argument '{name}'")
        self._fbthrift_update_type_value(
            tpe.value,
            self._fbthrift_to_internal_data(tpe.value, value),
        )

    @classmethod
    def _fbthrift_create(cls, data):
        cdef Union inst = cls.__new__(cls)
        inst._fbthrift_data = data
        inst._fbthrift_load_cache()
        return inst

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__() -> str:
        return NotImplementedError()

    @staticmethod
    def __get_thrift_uri__() -> typing.Optional[str]:
        return NotImplementedError()

    cdef object _fbthrift_to_internal_data(self, type_value, value):
        cdef UnionInfo union_info = self._fbthrift_struct_info
        adapter_info = union_info.id_to_adapter_info[type_value]
        if adapter_info:
            adapter_class, transitive_annotation = adapter_info
            value = adapter_class.to_thrift_field(
                value,
                type_value,
                self,
                transitive_annotation=transitive_annotation(),
            )
        return union_info.type_infos[type_value].to_internal_data(value)

    cdef void _fbthrift_update_type_value(self, type_value, value) except *:
        Py_INCREF(type_value)
        old_type_value = self._fbthrift_data[0]
        PyTuple_SET_ITEM(self._fbthrift_data, 0, type_value)
        Py_DECREF(old_type_value)
        old_value = self._fbthrift_data[1]
        Py_INCREF(value)
        PyTuple_SET_ITEM(self._fbthrift_data, 1, value)
        Py_DECREF(old_value)
        self._fbthrift_load_cache()

    cdef void _fbthrift_load_cache(self) except *:
        self.type = self.Type(self._fbthrift_data[0])
        val = self._fbthrift_data[1]
        if val is None:
            self.value = None
            return
        cdef UnionInfo info = self._fbthrift_struct_info
        self.value = info.type_infos[self._fbthrift_data[0]].to_python_value(val)

    cdef folly.iobuf.IOBuf _serialize(self, Protocol proto):
        cdef UnionInfo info = self._fbthrift_struct_info
        return folly.iobuf.from_unique_ptr(
            cmove(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _deserialize(self, folly.iobuf.IOBuf buf, Protocol proto) except? 0:
        cdef UnionInfo info = self._fbthrift_struct_info
        cdef uint32_t size = cdeserialize(deref(info.cpp_obj), buf._this, self._fbthrift_data, proto)
        self._fbthrift_load_cache()
        return size

    cdef _fbthrift_get_field_value(self, int16_t index):
        if self.type.value != index:
            # TODO in python 3.10 update this to use name and obj fields
            raise AttributeError(f'Union contains a value of type {self.type.name}, not {self.Type(index).name}')
        return self.value

    def get_type(self):
        return self.type

    @classmethod
    def fromValue(cls, value):
        cdef Union inst = cls.__new__(cls)
        if value is None:
            return inst
        cdef UnionInfo info = cls._fbthrift_struct_info
        for type_value, typeinfo in info.type_infos.items():
            try:
                value = inst._fbthrift_to_internal_data(type_value, value)
            except (TypeError, OverflowError):
                continue
            else:
                inst._fbthrift_update_type_value(type_value, value)
                break
        return inst

    def __copy__(Union self):
        return self

    def __deepcopy__(Union self, _memo):
        return self

    def __eq__(Union self, other):
        if type(other) != type(self):
            return False
        return self.type == other.type and self.value == other.value

    def __lt__(self, other):
        if type(self) != type(other):
            return NotImplemented
        return (self.type.value, self.value) < (other.type.value, other.value)

    def __le__(self, other):
        if type(self) != type(other):
            return NotImplemented
        return (self.type.value, self.value) <= (other.type.value, other.value)

    def __hash__(self):
        return hash((self.type, self.value))

    def __repr__(self):
        return f"{type(self).__name__}({self.type.name}={self.value!r})"

    def __bool__(self):
        return self.type.value != 0

    def __dir__(self):
        return dir(type(self))

    def __reduce__(self):
        return (_unpickle_union, (type(self), b''.join(self._serialize(Protocol.COMPACT))))

cdef make_fget_struct(i):
    return lambda self: (<Struct>self)._fbthrift_get_field_value(i)

cdef make_fget_union(type_value, adapter_info):
    if adapter_info:
        adapter_class, transitive_annotation = adapter_info
        return property(lambda self:
            adapter_class.from_thrift_field(
                (<Union>self)._fbthrift_get_field_value(type_value),
                type_value,
                self,
                transitive_annotation=transitive_annotation(),
            )
        )
    return property(lambda self: (<Union>self)._fbthrift_get_field_value(type_value))


def _fbthrift_readonly_setattr(name, _):
    """Setter for read-only attributes, always throws AttributeError."""
    raise AttributeError(f"can't set attribute '{name}'")


class StructMeta(type):
    """Metaclass for all generated Thrift Struct types."""

    def __new__(cls, cls_name, bases, dct):
        """
        Returns a new Thrift Struct class with the given name and members.

        Args:
            cls_name (str): Name of the Thrift Struct, as specified in the
                Thrift IDL.
            bases: unused (expected to always be empty).
            dct (Dict): class members, including the SPEC for this class under
                the key '_fbthrift_SPEC'.

        Returns:
            A new class, with the given `cls_name`, corresponding to a Thrift
            Struct. The returned class inherits from `Struct` and provides
            (read-only) properties for all non-primitive fields (including any
            adapted fields) specified in the SPEC.

            The returned class will also have the following additional class
            attributes, meant for internal (Thrift) processing:

            _fbthrift_struct_info: StructInfo

            _fbthrift_primitive_types:
                List[Tuple[int (index in fields), str (field name), type_info]]
                for all primitive (and non-adapted) fields.
        """
        # Set[Tuple (field spec)]. See `StructInfo` class docstring for the
        # contents of the field spec tuples.
        fields = dct.pop('_fbthrift_SPEC')

        num_fields = len(fields)
        dct["_fbthrift_struct_info"] = StructInfo(cls_name, fields)

        # List[Tuple[int (index in fields), str (field name), type_info]]
        primitive_types = []

        # List[Tuple[int (index in fields), str (field name)]]
        non_primitive_types = []

        slots = []
        for i, (id, qualifier, name, type_info, default_value, adapter_info, is_primitive) in enumerate(fields):
            slots.append(name)

            # if field has an adapter or is not primitive type, consider as "non-primitive"
            if adapter_info is not None or not is_primitive:
                non_primitive_types.append((i, name))
            else:
                primitive_types.append((i, name, type_info))

        dct["_fbthrift_primitive_types"] = primitive_types
        dct["__slots__"] = slots
        klass = super().__new__(cls, cls_name, (Struct,), dct)

        for field_index, field_name in non_primitive_types:
            type.__setattr__(
                klass,
                field_name,
                _make_cached_property(
                    make_fget_struct(field_index),
                    klass,
                    field_name,
                )
            )
        klass.__setattr__ = _fbthrift_readonly_setattr
        return klass

    def _fbthrift_fill_spec(cls):
        """
        Completes initialization of all specs for this Struct class.

        This should be called once, after all generated classes (unions and
        structs) for a given module have been created.
        """
        (<StructInfo>cls._fbthrift_struct_info).fill()

    def _fbthrift_store_field_values(cls):
        """
        Initializes the default values of fields (if any) for this Struct.

        This should be called once, after `_fbthrift_fill_spec()` has been
        called for all generated classes (unions and structs) in a module.
        """
        (<StructInfo>cls._fbthrift_struct_info).store_field_values()

    def __dir__(cls):
        return tuple(name for name, _ in cls) + (
            "__iter__",
        )

    def __iter__(cls):
        """
        Iterating over Thrift generated Struct classes yields the names of the
        fields in the struct.

        Should not be called prior to `_fbthrift_fill_spec()`.
        """
        for name in (<StructInfo>cls._fbthrift_struct_info).name_to_index.keys():
            yield name, None


def gen_enum(fields):
    yield ("EMPTY", 0)
    for f in fields:
        yield (f[2], f[0])


class UnionMeta(type):
    def __new__(cls, name, bases, dct):
        fields = dct.pop('_fbthrift_SPEC')
        num_fields = len(fields)
        dct["_fbthrift_struct_info"] = UnionInfo(name, fields)
        dct["Type"] = enum.Enum(name, gen_enum(fields))
        for f in fields:
            dct[f[2]] = make_fget_union(f[0], f[5])
        return super().__new__(cls, name, (Union,), dct)

    def __dir__(cls):
        return tuple((<UnionInfo>cls._fbthrift_struct_info).name_to_index.keys()) + (
            "type", "value")

    def _fbthrift_fill_spec(cls):
        (<UnionInfo>cls._fbthrift_struct_info).fill()


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


cdef class Container:
    """
    Base class for immutable container types
    """
    def __copy__(Container self):
        return self

    def __deepcopy__(Container self, _memo):
        return self

    def __len__(Container self):
        return len(self._fbthrift_elements)


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


cdef class ListTypeFactory:
    cdef object val_info
    def __init__(self, val_info):
        self.val_info = val_info

    def __call__(self, values=None):
        if values is None:
            values = ()
        return List(self.val_info, values)


cdef class List(Container):
    """
    A immutable container used to prepresent a Thrift list. It has compatible
    API with a Python list but has additional API to interact with other Python
    iterators
    """
    def __init__(self, val_info, values):
        self._fbthrift_val_info = val_info
        if isinstance(values, (str, bytes)):
            raise TypeError(
                "If you really want to pass a string or bytes into a "
                "_typing.Sequence[str] field, explicitly convert it first."
            )
        self._fbthrift_elements = tuple(val_info.to_container_value(v) for v in values)

    def __hash__(self):
        return hash(self._fbthrift_elements)

    def __add__(List self, other):
        return list(itertools.chain(self, other))

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
        return (List, (self._fbthrift_val_info, list(self),))

    def __getitem__(self, object index_obj):
        if not isinstance(index_obj, slice):
            return self._fbthrift_elements[index_obj]
        return List(self._fbthrift_val_info, self._fbthrift_elements[index_obj])

    def __contains__(self, item):
        if item is None:
            return False
        return item in self._fbthrift_elements

    def __iter__(self):
        return iter(self._fbthrift_elements)

    def __reversed__(self):
        return reversed(self._fbthrift_elements)

    def index(self, item, start=0, stop=None):
        if stop is None:
            stop = len(self)
        return self._fbthrift_elements.index(item, start, stop)

    def count(self, item):
        return self._fbthrift_elements.count(item)

Sequence.register(List)


cdef class SetTypeFactory:
    cdef object val_info
    def __init__(self, val_info):
        self.val_info = val_info

    def __call__(self, values=None):
        if values is None:
            values = ()
        return Set(self.val_info, values)


cdef class Set(Container):
    """
    A immutable set used to prepresent a Thrift set. It has compatible
    API with a Python set but has additional API to interact with other Python
    iterators
    """
    def __init__(self, val_info, values):
        self._fbthrift_val_info = val_info
        if isinstance(values, (str, bytes)):
            raise TypeError(
                "If you really want to pass a string or bytes into a "
                "_typing.Sequence[str] field, explicitly convert it first."
            )
        self._fbthrift_elements = frozenset(val_info.to_container_value(v) for v in values)

    def __hash__(self):
        return hash(self._fbthrift_elements)

    def __and__(Set self, other):
        return self._fbthrift_elements & other

    def __rand__(Set self, other):
        return other & self._fbthrift_elements

    def __sub__(Set self, other):
        return self._fbthrift_elements - other

    def __rsub__(Set self, other):
        return other - self._fbthrift_elements

    def __or__(Set self, other):
        return self._fbthrift_elements | other

    def __ror__(Set self, other):
        return other | self._fbthrift_elements

    def __xor__(Set self, other):
        return self._fbthrift_elements ^ other

    def __rxor__(Set self, other):
        return other ^ self._fbthrift_elements

    def __eq__(Set self, other):
        return self._fbthrift_elements == other

    def __lt__(Set self, other):
        return self._fbthrift_elements < other

    def __gt__(Set self, other):
        return not (self == other or self < other)

    def __le__(Set self, other):
        return self == other or self < other

    def __ge__(Set self, other):
        return not self < other

    def __repr__(self):
        if not self:
            return 'iset()'
        return f'i{{{", ".join(map(repr, self))}}}'

    def __reduce__(self):
        return (Set, (self._fbthrift_val_info, set(self),))

    def __contains__(self, item):
        if item is None:
            return False
        return item in self._fbthrift_elements

    def __iter__(self):
        return iter(self._fbthrift_elements)

    def __reversed__(self):
        return reversed(self._fbthrift_elements)

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

pySet.register(Set)


cdef class MapTypeFactory:
    cdef object key_info
    cdef object val_info
    def __init__(self, key_info, val_info):
        self.key_info = key_info
        self.val_info = val_info

    def __call__(self, values=None):
        if values is None:
            values = {}
        return Map(self.key_info, self.val_info, values)


cdef class Map(Container):
    """
    A immutable container used to prepresent a Thrift map. It has compatible
    API with a Python map but has additional API to interact with other Python
    iterators
    """
    def __init__(self, key_info, val_info, values):
        self._fbthrift_key_info = key_info
        self._fbthrift_val_info = val_info
        self._fbthrift_elements = {
            key_info.to_container_value(k): val_info.to_container_value(v)
            for k, v in values.items()
        }

    def __hash__(self):
        return hash(tuple(self.items()))

    def __eq__(Map self, other):
        if not isinstance(other, Mapping):
            return False
        if len(self) != len(other):
            return False
        for key in self:
            if key not in other:
                return False
            if other[key] != self[key]:
                return False
        return True

    def __repr__(self):
        if not self:
            return 'i{}'
        return f'i{{{", ".join(map(lambda i: f"{repr(i[0])}: {repr(i[1])}", self.items()))}}}'

    def __reduce__(Map self):
        return (Map, (self._fbthrift_key_info, self._fbthrift_val_info, dict(self),))

    def __getitem__(Map self, object key):
        return self._fbthrift_elements[key]

    def __contains__(self, key):
        if key is None:
            return False
        return key in self._fbthrift_elements

    def __iter__(Map self):
        return iter(self._fbthrift_elements)

    def keys(Map self):
        return self._fbthrift_elements.keys()

    def values(Map self):
        return self._fbthrift_elements.values()

    def items(self):
        return self._fbthrift_elements.items()

    def get(Map self, key, default=None):
        try:
            return self[key]
        except KeyError:
            return default


Mapping.register(Map)


# We will create all the classes first then call fill_specs after that so
# dependancies can be properly solved.
def fill_specs(*struct_types):
    """
    Completes the initialization of the given Thrift-generated Struct (and
    Union) classes.


    This is called at the end of the modules that define the corresponding
    generated types (i.e., the `thrift_types.py` files), after the given classes
    have been created but not yet fully initialized. It provides support for
    dependent classes being defined in arbitrary order.

    If struct A has a field of type struct B, but the generated class A is
    defined before B, we are not able to populate the specs for A as part of the
    class creation, hence this call.

    Args:
        *struct_types: Sequence of class objects, each one of which corresponds
        to either a `Struct` (i.e., created by/instance of `StructMeta`) or a
        `Union` (i.e., created by/instance of `UnionMeta`).
    """

    for cls in struct_types:
        cls._fbthrift_fill_spec()

    for cls in struct_types:
        if not isinstance(cls, UnionMeta):
            cls._fbthrift_store_field_values()


def isset(StructOrError struct):
    cdef StructInfo info = struct._fbthrift_struct_info
    isset_bytes = struct._fbthrift_data[0]
    return {
        name: bool(isset_bytes[index])
        for name, index in info.name_to_index.items()
    }


def update_nested_field(Struct obj, path_to_values):
    # There is some optimization opportunity here for cases like this:
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


class EnumMeta(type):
    def __new__(metacls, classname, bases, dct):
        # if no bases, it's creating Enum or Flag base class, no need to parse members.
        attrs = {
            "__members__": {},
            "__reversed_map__": {},
        }
        if not bases:
            return super().__new__(metacls, classname, (), dict(dct, **attrs))
        for name, value in dct.items():
            if not isinstance(value, int):
                attrs[name] = value
        klass = super().__new__(
            metacls,
            classname,
            bases,
            attrs,
        )
        for name, value in dct.items():
            if not isinstance(value, int):
                continue
            option = klass.__new__(klass, value)
            option._fbthrift_name_ = name
            option._fbthrift_value_ = value
            klass.__members__[name] = option
            klass.__reversed_map__[value] = option
            type.__setattr__(klass, name, option)
        return klass

    def __len__(cls):
        return len(cls.__members__)

    def __getitem__(cls, attribute):
        return cls.__members__[attribute]

    def __iter__(cls):
        return iter(cls.__members__.values())

    def __setattr__(cls, name, _):
        raise AttributeError(f"'{cls.__qualname__}' has no attribute '{name}'")

    def __delattr__(cls, name):
        raise AttributeError(f"{cls.__name__}: cannot delete Enum member.")

    def __call__(cls, value, /):
        if isinstance(value, cls):
            return value
        try:
            return cls.__reversed_map__[value]
        except KeyError:
            inst =  cls._fbthrift_missing_(value)
            if inst is not None:
                return inst
            raise ValueError(
                f"Enum type {cls.__name__} has no attribute with value {value!r}"
            ) from None


class Enum(metaclass=EnumMeta):
    def __init__(self, _):
        # pass on purpose to keep the __init__ interface consistent with the other base class (i.e. int)
        pass

    @classmethod
    def _fbthrift_missing_(cls, value):
        return None

    @property
    def name(self):
        return self._fbthrift_name_

    @property
    def value(self):
        return self._fbthrift_value_

    def __dir__(self):
        return ["name", "value"]

    def __str__(self):
        return f"{type(self).__name__}.{self.name}"

    def __repr__(self):
        return f"<{type(self).__name__}.{self.name}: {self.value}>"

    def __reduce_ex__(self, proto):
        return type(self), (self.value,)

    def __eq__(self, other):
        if isinstance(other, Enum):
            return self is other
        return self._fbthrift_value_ == other

    def __ne__(self, other):
        if isinstance(other, Enum):
            return self is not other
        return self._fbthrift_value_ != other

    def __hash__(self):
        return hash(self._fbthrift_value_)

    @staticmethod
    def __get_metadata__():
        raise NotImplementedError()

    @staticmethod
    def __get_thrift_name__():
        return NotImplementedError()

    @staticmethod
    def __get_thrift_uri__():
        return NotImplementedError()

    def __bool__(self):
        return True


class Flag(Enum):
    @classmethod
    def _fbthrift_missing_(cls, value):
        """
        Returns member (possibly creating it) if one can be found for value.
        """
        masked_value = 0
        for m in cls:
            masked_value |= value & m._fbthrift_value_
        if value >= 0 and value != masked_value:
            raise ValueError(f"{value!r} is not a valid {cls.__qualname__}")
        return cls._fbthrift_create_pseudo_member_(masked_value)

    @classmethod
    def _fbthrift_create_pseudo_member_(cls, value):
        """
        Create a composite member iff value contains only members.
        """
        pseudo_member = cls.__reversed_map__.get(value, None)
        if pseudo_member is None:
            pseudo_member = object.__new__(cls)
            pseudo_member._fbthrift_name_ = None
            pseudo_member._fbthrift_value_ = value
            # use setdefault in case another thread already created a composite
            # with this value
            pseudo_member = cls.__reversed_map__.setdefault(value, pseudo_member)
        return pseudo_member

    @classmethod
    def _fbthrift_get_name(cls, value):
        # _decompose is only called if the value is not named
        names = []
        while value != 0:
            highest = 1 << (value.bit_length() - 1)
            names.append(cls(highest)._fbthrift_name_)
            value &= ~highest
        return names

    def __repr__(self):
        cls = type(self)
        if self._fbthrift_name_ is not None:
            return f"<{cls.__name__}.{self._fbthrift_name_}: {self._fbthrift_value_!r}>"
        names = '|'.join(cls._fbthrift_get_name(self._fbthrift_value_))
        return f"<{cls.__name__}.{names}: {self._fbthrift_value_!r}>"

    def __str__(self):
        cls = type(self)
        if self._fbthrift_name_ is not None:
            return f"{cls.__name__}.{self._fbthrift_name_}"
        names = cls._fbthrift_get_name(self._fbthrift_value_)
        return f"{cls.__name__}.{names}"

    def __contains__(self, other):
        if type(other) is not type(self):
            return NotImplemented
        return other._fbthrift_value_ & self._fbthrift_value_ == other._fbthrift_value_

    def __bool__(self):
        return bool(self._fbthrift_value_)

    def __or__(self, other):
        cls = type(self)
        if type(other) is not cls:
            return NotImplemented
        return cls(self._fbthrift_value_ | other._fbthrift_value_)

    def __and__(self, other):
        cls = type(self)
        if type(other) is not cls:
            return NotImplemented
        return cls(self._fbthrift_value_ & other._fbthrift_value_)

    def __xor__(self, other):
        cls = type(self)
        if type(other) is not cls:
            return NotImplemented
        return cls(self._fbthrift_value_ ^ other._fbthrift_value_)

    def __invert__(self):
        cls = type(self)
        res = self._fbthrift_value_
        for m in cls:
            res ^= m._fbthrift_value_
        return cls(res)

cdef class ServiceInterface:
    @staticmethod
    def service_name():
        raise NotImplementedError("Service name not implemented")

    def getFunctionTable(self):
        return {}

    async def __aenter__(self):
        # Establish async context managers as a way for end users to async initalize
        # internal structures used by Service Handlers.
        return self

    async def __aexit__(self, *exc_info):
        # Same as above, but allow end users to define things to be cleaned up
        pass

    async def onStartServing(self):
        pass

    async def onStopRequested(self):
        pass
