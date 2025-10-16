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
from collections.abc import Iterable, Mapping, Sequence, Set as pySet, Sized
from types import MappingProxyType
import warnings

from cpython cimport bool as pbool, int as pint, float as pfloat
from cpython.long cimport PyLong_AsLong
from cpython.object cimport Py_LT, Py_EQ, PyCallable_Check
from cpython.tuple cimport PyTuple_New, PyTuple_SET_ITEM, PyTuple_GET_ITEM, PyTuple_Check
from cpython.ref cimport Py_INCREF, Py_DECREF
from cpython.set cimport PyFrozenSet_New, PySet_Add
from cpython.unicode cimport PyUnicode_AsUTF8String, PyUnicode_FromEncodedObject
from cython.operator cimport dereference as deref
from cython cimport final as _cython__final
from libcpp cimport bool as cbool
from libcpp.utility cimport move as cmove
from libcpp.memory cimport make_unique
from folly.iobuf cimport cIOBuf, IOBuf, from_unique_ptr

import copy
import cython
import enum
import itertools
import typing

from folly cimport cFollyIsDebug
from thrift.python.exceptions cimport GeneratedError
from thrift.python.serializer cimport cserialize, cdeserialize

# if True, then cinder is importable, and use_cinder = True,
# meaning cinder functions use native extensions, not inefficient fallbacks
cdef cbool _fbthrift_is_cinder_runtime = False
try:
    import cinder
    import cinderx
    _fbthrift_is_cinder_runtime = cinderx.is_initialized()
# ImportError for non-linux
# AttributeError for pins to old cinder, via third-party
except (ImportError, AttributeError):
    pass


cdef extern from *:
    """
    #undef _serialize
    """

_fbthrift__PY3_STRUCTURED = None

cdef class _fbthrift_UnmatchableType:
    pass

cdef _is_py3_structured(obj):
    global _fbthrift__PY3_STRUCTURED
    if _fbthrift__PY3_STRUCTURED is None:
        try:
            from thrift.py3.types import Struct
            from thrift.py3.exceptions import GeneratedError
            _fbthrift__PY3_STRUCTURED = (Struct, GeneratedError)
        except ImportError:
            _fbthrift__PY3_STRUCTURED = _fbthrift_UnmatchableType

    return isinstance(obj, _fbthrift__PY3_STRUCTURED)



cdef _make_non_primitive_property(struct_class, int field_index, str field_name):
    # there are two cases where cinder.cached_property is not used:
    # 1. On MacOs/Windows, cinder is not available.
    # 2. On Linux, in python_binary where use_cinder = False (the default)
    #    cinder is available but cinder.cached_property is very slow.
    if _fbthrift_is_cinder_runtime:
        getter_fn = lambda self: (<Struct>self)._fbthrift_py_value_from_internal_data(field_index)
        return cinder.cached_property(
            getter_fn,
            getattr(struct_class, field_name),
        )
    cdef pbool disable_cache = getattr(
        struct_class,
        '_fbthrift_disable_field_cache_DO_NOT_USE',
        False
    )
    if disable_cache:
        return _StructUncachedField(field_index, field_name)
    else:
        return _StructCachedField(field_index, field_name)


cdef public api object deepcopy(object obj):
    """
    Wraps the python `copy.deepcopy()` operation for use from C++. There is no
    direct C API implementation of the copy library.
    """
    return copy.deepcopy(obj)

cdef public api cIOBuf* get_cIOBuf(object buf):
    if buf is None:
        return NULL
    return (<IOBuf>buf)._ours.get()

cdef public api object create_IOBuf(unique_ptr[cIOBuf] ciobuf):
    return from_unique_ptr(cmove(ciobuf))

cdef class TypeInfoBase:
    cdef to_internal_data(self, object value):
        raise NotImplementedError("Not implemented on base TypeInfoBase class")

    cdef to_python_value(self, object value):
        raise NotImplementedError("Not implemented on base TypeInfoBase class")

    cdef const cTypeInfo* get_cTypeInfo(self):
        raise NotImplementedError("Not implemented on base TypeInfoBase class")

    def __eq__(self, other):
        raise NotImplementedError(
            "Use the 'same_as' method for comparing TypeInfoBase instances."
        )

    def same_as(TypeInfoBase self, other):
        """
        `TypeInfo` classes are a sort of bridge between Thrift IDL types and Python
        types. For example, few examples of mapping between Thrift-IDL-type to
        Python-type are below,

        | Thrift IDL Type  | Python Type |
        |------------------|-------------|
        | i32              | int         |
        | i64              | int         |
        | float            | float       |
        | double           | float       |

        `same_as()` returns `True` if the `TypeInfo` class maps the same IDL Thrift
        type to the same Python type.
        """
        raise NotImplementedError("Not implemented on base TypeInfoBase class")

    def is_container(self):
        """
        Return `True` if it is one of the immutable or mutable `TypeInfo` classes
        for list, map or set.
        """
        return False

@_cython__final
cdef class TypeInfo(TypeInfoBase):
    @staticmethod
    cdef create(
        const cTypeInfo& cpp_obj,
        type true_pytype,
        tuple allowed_pytypes,
        str singleton_name
    ):
        cdef TypeInfo inst = TypeInfo.__new__(TypeInfo)
        inst.cpp_obj = &cpp_obj
        inst.true_pytype = true_pytype
        inst.allowed_pytypes = (true_pytype,) + allowed_pytypes
        inst.singleton_name = singleton_name
        return inst

    # validate and convert to format serializer may understand
    cpdef to_internal_data(TypeInfo self, object value):
        cdef type value_type = type(value)
        if value_type is self.true_pytype:
            return value
        if not issubclass(value_type, self.allowed_pytypes):
            raise TypeError(
                f'value {value} is not a {self.allowed_pytypes !r}, is actually of type '
                f'{type(value)}'
            )
        return self.true_pytype(value)

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        return value

    def to_container_value(self, object value):
        return self.to_internal_data(value)

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj

    def same_as(TypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, TypeInfo):
            return False

        return (self.cpp_obj.type == (<TypeInfo>other).cpp_obj.type and
                self.allowed_pytypes == (<TypeInfo>other).allowed_pytypes)

    def __reduce__(self):
        # For primitives use the singleton pickling strategy (i.e. return the name of a global variable)
        # instead of repeatedly constructing TypeInfo instances
        return self.singleton_name

@_cython__final
cdef class IntegerTypeInfo(TypeInfoBase):
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, min_value, max_value, str singleton_name):
        cdef IntegerTypeInfo inst = IntegerTypeInfo.__new__(IntegerTypeInfo)
        inst.cpp_obj = &cpp_obj
        inst.min_value = min_value
        inst.max_value = max_value
        inst.singleton_name = singleton_name
        return inst

    # validate and convert to format serializer may understand
    cpdef to_internal_data(self, object value):
        cdef type value_type = type(value)
        if value_type is not pint:
            if not issubclass(value_type, pint):
                raise TypeError(
                    f"value {value} is not a <class 'int'>, is actually of type {value_type}"
                )
            # convert IntEnum and other int-extending types
            value = int(value)
        cdef int64_t cvalue = value
        if cvalue > self.max_value or cvalue < self.min_value:
            raise OverflowError()
        return value

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        return value

    def to_container_value(self, object value not None):
        return self.to_internal_data(value)

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj

    def same_as(IntegerTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, IntegerTypeInfo):
            return False

        cdef IntegerTypeInfo other_typeinfo = other
        return (self.min_value == other_typeinfo.min_value and
            self.max_value == other_typeinfo.max_value)

    def __reduce__(self):
        return self.singleton_name

@_cython__final
cdef class StringTypeInfo(TypeInfoBase):
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, str singleton_name):
        cdef StringTypeInfo inst = StringTypeInfo.__new__(StringTypeInfo)
        inst.cpp_obj = &cpp_obj
        inst.singleton_name = singleton_name
        return inst

    # validate and convert to format serializer may understand
    cpdef to_internal_data(self, object value):
        """
        Validates that `value` is a `str` and returns it. From python,
        only possible to set `string` fields to valid unicode `str` objects.
        """
        if type(value) is str:
            return value
        # it is legal to set string field to `str` subclass
        if isinstance(value, str):
            # some versions of StringEnum only override __format__, not __str__
            return f"{value}"

        raise TypeError(
            "Cannot create internal string data representation. "
            f"Expected type <class 'str'>, got: {type(value)}."
        )

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        """
        Returns the internal data `str`. If the string was deserialized from
        invalid unicode, then the internal data is `bytes` and this method
        will raise a `UnicodeDecodeError`.

        Args:
            value: `str | bytes`: Typically a `str` created by decoding
            UTF-8 during deserialization. If that failed, then a `bytes` object
            containing the original serialized data.
        """
        cdef value_type = type(value)
        if value_type is str:
            return value
        if value_type is bytes:
            # this should only happen when UnicodeDecodeError occurred during
            # deserialize, so this should raise UnicodeDecodeError
            return value.decode("utf-8")
        raise TypeError(f"Expected a str, encountered {value_type}")


    def to_container_value(self, object value not None):
        if not isinstance(value, str):
            raise TypeError(f"value {value} is not a <class 'str'>, is actually of type {type(value)}")
        return value

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj

    def same_as(StringTypeInfo self, other):
        if other is self:
            return True

        return isinstance(other, StringTypeInfo)

    def __reduce__(self):
        return self.singleton_name

@_cython__final
cdef class IOBufTypeInfo(TypeInfoBase):
    @staticmethod
    cdef create(const cTypeInfo& cpp_obj, str singleton_name):
        cdef IOBufTypeInfo inst = IOBufTypeInfo.__new__(IOBufTypeInfo)
        inst.cpp_obj = &cpp_obj
        inst.singleton_name = singleton_name
        return inst

    cpdef to_internal_data(self, object value):
        return <IOBuf?>value

    cpdef to_python_value(self, object value):
        return value

    def to_container_value(self, IOBuf value):
        return value

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj

    def same_as(IOBufTypeInfo self, other):
        if other is self:
            return True

        return isinstance(other, IOBufTypeInfo)

    def __reduce__(self):
        return self.singleton_name


typeinfo_bool = TypeInfo.create(boolTypeInfo, pbool, (), "typeinfo_bool")
typeinfo_byte = IntegerTypeInfo.create(byteTypeInfo, -128, 127, "typeinfo_byte")
typeinfo_i16 = IntegerTypeInfo.create(i16TypeInfo, -1<<15, (1<<15)-1, "typeinfo_i16")
typeinfo_i32 = IntegerTypeInfo.create(i32TypeInfo, -1<<31, (1<<31)-1, "typeinfo_i32")
typeinfo_i64 = IntegerTypeInfo.create(i64TypeInfo, -1<<63, (1<<63)-1, "typeinfo_i64")
typeinfo_double = TypeInfo.create(doubleTypeInfo, pfloat, (pint,), "typeinfo_double")
typeinfo_float = TypeInfo.create(floatTypeInfo, pfloat, (pint,), "typeinfo_float")
typeinfo_string = StringTypeInfo.create(stringTypeInfo, "typeinfo_string")
typeinfo_binary = TypeInfo.create(binaryTypeInfo, bytes, (), "typeinfo_binary")
typeinfo_iobuf = IOBufTypeInfo.create(iobufTypeInfo, "typeinfo_iobuf")


StructOrError = cython.fused_type(Struct, GeneratedError)

AnyTypeInfo = typing.Union[
    StructTypeInfo,
    ListTypeInfo,
    SetTypeInfo,
    MapTypeInfo,
    EnumTypeInfo,
    TypeInfo,
    IntegerTypeInfo,
    StringTypeInfo,
]

@_cython__final
cdef class FieldInfo:
    def __cinit__(self, id, qualifier, name, py_name, type_info, default_value, adapter_info, is_primitive, idl_type = -1):
        """
        Args:
            id (int): The field ID specified in the IDL.

            qualifier (FieldQualifier): Unqualified, Optional, ...

            name (str): The name of the Thrift field, as specified in the IDL.

            py_name (str):
                The actual output name of the Thrift field, as specified in an
                `@python.Name{}` annotation if present, or just the IDL name if there's
                no annotation

            type_info (TypeInfoBase | Callable[[], TypeInfoBase]):
                Type information object corresponding to this field (eg. typeinfo_string,
                ListTypeInfo, SetTypeInfo, etc.), OR a callable (eg. lambda) that returns
                such an object (to handle types with dependencies in arbitrary order).

            default_value (typing.Optional[object | Callable[[], object]]):
                Custom default value specified in the IDL, or None. If present, this can
                also be a callable which will be called (exactly once) to obtain the
                default value.

            adapter_info (typing.Optional[AdapterInfo])

            is_primitive (bool):
                Whether the field has a "primitive" type, such as: bool, byte, i16, i32,
                i62, double, float.
                Note that this definition of "primitive" DOES NOT match that of Thrift IDL
                primitive types, as the latter include string and binary (see:
                https://github.com/facebook/fbthrift/blob/main/thrift/doc/idl/index.md#primitive-types).

            idl_type (int):
                Thrift IDL type of the field. The enum values are defined in
                `apache::thrift::type::BaseType` located at `thrift/lib/cpp2/type/BaseType.h`
                Default value is -1, which is not a valid value for `BaseType` enum.
                This field is currently only used by mutable types.
        """
        self.id = id
        self.qualifier = qualifier
        self.name = name
        self.py_name = py_name
        self.type_info = type_info
        self.default_value = default_value
        self.adapter_info = adapter_info
        self.is_primitive = is_primitive
        self.idl_type = idl_type

    @property
    def id(self):
        return self.id

    @property
    def qualifier(self):
        return self.qualifier

    @property
    def name(self):
        return self.name

    @property
    def py_name(self):
        return self.py_name

    @property
    def type_info(self):
        return self.type_info

    @type_info.setter
    def type_info(self, type_info):
        self.type_info = type_info

    @property
    def default_value(self):
        return self.default_value

    @property
    def adapter_info(self):
        return self.adapter_info

    @property
    def is_primitive(self):
        return self.is_primitive

    @property
    def idl_type(self):
        return self.idl_type

@_cython__final
cdef class StructInfo:
    """
    Stores information for a specific Thrift Struct class.

    Instance Variables:
        fields (tuple[FieldInfo, ...]): Specifications of each field in this Thrift
            struct.

        cpp_obj: cDynamicStructInfo for this struct.

        type_infos: Tuple whose size matches the number of fields in the Thrift
            struct. Initialized by calling `_fill_struct_info()`.

        name_to_index: Dict[str (field name), int (index in `fields`).].
            Initialized by calling `_fill_struct_info()`.
    """

    def __cinit__(self, name: str, fields):
        """
        Stores information for a Thrift Struct class with the given name.

        Args:
            name: Name of the Thrift Struct (as specified in IDL)
            fields (tuple[FieldInfo, ...]): Field spec tuples.
        """
        self.fields = fields
        cdef int16_t num_fields = len(fields)
        self.cpp_obj = make_unique[cDynamicStructInfo](
            PyUnicode_AsUTF8(name),
            num_fields,
            False, # isUnion
            False, # isMutable
        )
        self.type_infos = PyTuple_New(num_fields)
        self.name_to_index = {}

    cdef void _fill_struct_info(self) except *:
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
        for idx, field_info in enumerate(self.fields):
            # type_info can be a lambda function so types with dependencies
            # won't need to be defined in order, see class docstring above.
            field_type_info = field_info.type_info
            if PyCallable_Check(field_info.type_info):
                field_type_info = field_info.type_info()

            # The rest of the code assumes that all the `TypeInfo` classes extend
            # from `TypeInfoBase`. Instances are typecast to `TypeInfoBase` before
            # the `to_internal_data()` and `to_python_value()` methods are called.
            if not isinstance(field_type_info, TypeInfoBase):
                raise TypeError(f"{type(field_type_info).__name__} is not subclass of TypeInfoBase.")

            Py_INCREF(field_type_info)
            PyTuple_SET_ITEM(type_infos, idx, field_type_info)
            field_info.type_info = field_type_info
            self.name_to_index[field_info.py_name] = idx
            dynamic_struct_info.addFieldInfo(
                field_info.id,
                field_info.qualifier,
                PyUnicode_AsUTF8(field_info.name),
                getCTypeInfo(field_type_info)
            )

    cdef void _initialize_default_values(self) except *:
        """
        Initializes the default values of fields in this Struct.

        Upon successful return, the field value(s) in `self.cpp_obj` are
        iniitalized (see `DynamicStructInfo::addFieldValue()`).
        """
        cdef cDynamicStructInfo* dynamic_struct_info = self.cpp_obj.get()
        for idx, field_info in enumerate(self.fields):
            default_value = field_info.default_value
            if default_value is None:
                continue
            if callable(default_value):
                default_value = default_value()
            type_info = self.type_infos[idx]
            if isinstance(type_info, AdaptedTypeInfo):
                type_info = (<AdaptedTypeInfo>type_info)._orig_type_info
            default_value = (<TypeInfoBase>type_info).to_internal_data(default_value)
            dynamic_struct_info.addFieldValue(idx, default_value)

@_cython__final
cdef class UnionInfo:
    """
    Stores information for a specific (immutable) Thrift union class.

    Attributes:
        fields (tuple[FieldInfo, ...])

        cpp_obj (cDynamicStructInfo):
            Fully initialized only after `_fill_union_info()` completes.

        type_infos (dict[int, TypeInfoBase | Callable[[], TypeInfoBase]):
            Mapping from union field id to the corresponding TypeInfo (or callable that
            returns a TypeInfo). Initialized by `_fill_union_info()`.

        id_to_adapter_info (dict[int, Optional[AdapterInfo]]):
            Initialized by `_fill_union_info()`.

        name_to_index (dict[str, int]):
            Mapping from union field name to field id. Initialized by `_fill_union_info()`.
    """

    def __cinit__(self, name: str, field_infos: tuple[FieldInfo, ...]):
        self.fields = field_infos
        self.cpp_obj = make_unique[cDynamicStructInfo](
            PyUnicode_AsUTF8(name),
            len(field_infos),
            True, # isUnion
            False, # isMutable
        )
        self.type_infos = {}
        self.id_to_adapter_info = {}
        self.name_to_index = {}

    cdef void _fill_union_info(self) except *:
        """
        Completes the initialization of this instance. Must be called exactly once.
        """

        cdef cDynamicStructInfo* dynamic_struct_info = self.cpp_obj.get()
        for idx, field_info in enumerate(self.fields):
            # type_info can be a lambda function so types with dependencies
            # won't need to be defined in order
            if callable(field_info.type_info):
                field_info.type_info = field_info.type_info()
            self.type_infos[field_info.id] = field_info.type_info
            self.id_to_adapter_info[field_info.id] = field_info.adapter_info
            self.name_to_index[field_info.py_name] = idx
            dynamic_struct_info.addFieldInfo(
                field_info.id,
                field_info.qualifier,
                PyUnicode_AsUTF8(field_info.name),
                getCTypeInfo(field_info.type_info)
            )


cdef inline const cTypeInfo* getCTypeInfo(type_info):
        return (<TypeInfoBase>type_info).get_cTypeInfo()


cdef to_container_elements_no_convert(type_info):
    return isinstance(type_info, (TypeInfo, IntegerTypeInfo)) or type_info is typeinfo_iobuf


@_cython__final
cdef class ListTypeInfo(TypeInfoBase):
    def __cinit__(self, val_info):
        self.val_info = val_info
        self.cpp_obj = make_unique[cListTypeInfo](getCTypeInfo(val_info))

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj.get().get()

    # validate and convert to format serializer may understand
    cpdef to_internal_data(self, object value):
        return self.to_internal_from_values(value, <TypeInfoBase>self.val_info)

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        cdef List inst = List.__new__(List)
        inst._fbthrift_val_info = self.val_info
        inst._fbthrift_elements = value if to_container_elements_no_convert(self.val_info) else self.to_python_from_values(value, <TypeInfoBase>self.val_info)
        return inst

    def to_container_value(self, object value not None):
        if isinstance(value, List):
            return value
        return List(self.val_info, value)

    cdef to_internal_from_values(self, object values, TypeInfoBase val_type_info):
        cdef size_t idx = 0
        cdef int value_len
        try:
            value_len = len(values)
        except TypeError:
            try:
                warnings.warn(
                    f"list type should be a Sequence with length, got {type(values)}",
                    RuntimeWarning,
                )
                # if there's no length, convert iterable to list
                # if it's not an iterable, then raise TypeError
                values = list(values)
                value_len = len(values)
            except Exception:
                raise TypeError(
                    f"values must be a Sequence with length, got {type(values)}"
                )

        cdef tuple tpl = PyTuple_New(value_len)
        for idx, value in enumerate(values):
            internal_data = val_type_info.to_internal_data(value)
            Py_INCREF(internal_data)
            PyTuple_SET_ITEM(tpl, idx, internal_data)

        return tpl

    cdef to_python_from_values(self, object values, TypeInfoBase val_type_info):
        cdef size_t idx = 0
        cdef tuple tpl = PyTuple_New(len(values))
        for idx, value in enumerate(values):
            python_value = val_type_info.to_python_value(value)
            Py_INCREF(python_value)
            PyTuple_SET_ITEM(tpl, idx, python_value)

        return tpl

    def same_as(ListTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, ListTypeInfo):
            return False

        return self.val_info.same_as((<ListTypeInfo>other).val_info)

    def is_container(self):
        return True

    def get_val_info(self):
        return self.val_info

    def __reduce__(self):
        return (ListTypeInfo, (self.val_info,))

@_cython__final
cdef class SetTypeInfo(TypeInfoBase):
    def __cinit__(self, val_info):
        self.val_info = val_info
        self.cpp_obj = make_unique_base[cSetTypeInfoBase, cSetTypeInfo](
            getCTypeInfo(val_info)
        )

    def enableKeySorted(self):
        self.cpp_obj = self.cpp_obj.get().asKeySorted()
        return self

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj.get().get()

    # validate and convert to format serializer may understand
    cpdef to_internal_data(self, object value):
        return self.to_internal_from_values(value, <TypeInfoBase>self.val_info)

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        cdef Set inst = Set.__new__(Set)
        inst._fbthrift_val_info = self.val_info
        inst._fbthrift_elements = value if to_container_elements_no_convert(self.val_info) else self.to_python_from_values(value, <TypeInfoBase>self.val_info)
        return inst

    def to_container_value(self, object value not None):
        if isinstance(value, Set):
            return value
        return Set(self.val_info, value)

    cdef to_internal_from_values(self, object values, TypeInfoBase val_type_info):
        cdef frozenset frozen_set = PyFrozenSet_New(<object>NULL)
        for value in values:
            PySet_Add(frozen_set, val_type_info.to_internal_data(value))

        return frozen_set

    cdef to_python_from_values(self, object values, TypeInfoBase val_type_info):
        cdef frozenset frozen_set = PyFrozenSet_New(<object>NULL)
        for value in values:
            PySet_Add(frozen_set, val_type_info.to_python_value(value))

        return frozen_set

    def same_as(SetTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, SetTypeInfo):
            return False

        return self.val_info.same_as((<SetTypeInfo>other).val_info)

    def is_container(self):
        return True

    def get_val_info(self):
        return self.val_info

    def __reduce__(self):
        return (SetTypeInfo, (self.val_info,))

@_cython__final
cdef class MapTypeInfo(TypeInfoBase):
    def __cinit__(self, key_info, val_info):
        self.key_info = key_info
        self.val_info = val_info
        self.cpp_obj = make_unique_base[cMapTypeInfoBase, cMapTypeInfo](
            getCTypeInfo(key_info),
            getCTypeInfo(val_info),
        )

    def enableKeySorted(self):
        self.cpp_obj = self.cpp_obj.get().asKeySorted()
        return self

    cdef const cTypeInfo* get_cTypeInfo(self):
        return self.cpp_obj.get().get()

    # validate and convert to format serializer may understand
    cpdef to_internal_data(self, object value):
        if value is None:
            raise TypeError("Argument 'value' must not be None")

        return self.to_internal_from_values(value)

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        cdef Map inst = Map.__new__(Map)
        inst._fbthrift_key_info = self.key_info
        inst._fbthrift_val_info = self.val_info
        inst._fbthrift_elements = self.to_python_from_values(value)
        return inst

    def to_container_value(self, object value not None):
        if isinstance(value, Map):
            return value
        return Map(self.key_info, self.val_info, value)

    cdef to_internal_from_values(self, object values):
        cdef TypeInfoBase key_type_info = self.key_info
        cdef TypeInfoBase val_type_info = self.val_info
        cdef int idx = 0
        cdef tuple tpl = PyTuple_New(len(values))
        for idx, (key, value) in enumerate(values.items()):
            internal_key_data = key_type_info.to_internal_data(key)
            internal_value_data = val_type_info.to_internal_data(value)
            internal_data = (internal_key_data, internal_value_data)
            Py_INCREF(internal_data)
            PyTuple_SET_ITEM(tpl, idx, internal_data)

        return tpl

    cdef to_python_from_values(self, object values):
        cdef TypeInfoBase key_type_info = self.key_info
        cdef TypeInfoBase val_type_info = self.val_info
        return {
            key_type_info.to_python_value(k): val_type_info.to_python_value(v) for k, v in values
        }

    def same_as(MapTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, MapTypeInfo):
            return False

        return (self.key_info.same_as((<MapTypeInfo>other).key_info) and
            self.val_info.same_as((<MapTypeInfo>other).val_info))

    def is_container(self):
        return True

    def get_key_info(self):
        return self.key_info

    def get_val_info(self):
        return self.val_info

    def __reduce__(self):
        return (MapTypeInfo, (self.key_info, self.val_info))

@_cython__final
cdef class StructTypeInfo(TypeInfoBase):
    def __cinit__(self, klass):
        self._class = klass
        py_struct_info = klass._fbthrift_struct_info
        cdef cDynamicStructInfo* c_struct_info
        if isinstance(py_struct_info, UnionInfo):
            self.is_union = True
            c_struct_info = (<UnionInfo>py_struct_info).cpp_obj.get()
        else:
            self.is_union = False
            c_struct_info = (<StructInfo>py_struct_info).cpp_obj.get()
        self.cpp_obj = createImmutableStructTypeInfo(
            deref(c_struct_info)
        )

    cdef const cTypeInfo* get_cTypeInfo(self):
        return &self.cpp_obj

    cpdef to_internal_data(self, object value):
        """
        Validates and converts the given (struct) `value` to a format that the
        serializer can udnerstand.

        Args:
            value: should either be an instance of `self._class`, or a py3
              struct which, when converted to thrift-python, returns an instance
              of `self._class`. Otherwise, raises `TypeError`.

        Raises:
            TypeError if `value` is not an instance of `self._class` (even after
              py3 to thrift-python conversion, if applicable).
        """
        if not isinstance(value, self._class):
            if _is_py3_structured(value):
                value = value._to_python()
            else:
                raise TypeError(
                    f"value {value} is not a {self._class !r}, is actually of type {type(value)}."
                )

            if not isinstance(value, self._class):
                raise TypeError(f"value {value} of type {type(value)}, cannot be converted to {self._class !r}.")

        if isinstance(value, Struct):
            return (<Struct>value)._fbthrift_data
        if isinstance(value, GeneratedError):
            return (<GeneratedError>value)._fbthrift_data
        if isinstance(value, Union):
            return (<Union>value)._fbthrift_data
        raise TypeError(f"{self._class} not supported")

    # convert deserialized data to user format
    cpdef to_python_value(self, value):
        if value is None:
            raise TypeError("StructTypeInfo.to_python_value requires valid internal data tuple, not None")
        return self._class._fbthrift_from_internal_data(value)

    def to_container_value(self, object value not None):
        if not isinstance(value, self._class):
            raise TypeError(f"value {value} is not a {self._class !r}, is actually of type {type(value)}.")
        return value

    def same_as(StructTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, StructTypeInfo):
            return False

        return self._class == (<StructTypeInfo>other)._class

    def __reduce__(self):
        return (StructTypeInfo, (self._class,))

@_cython__final
cdef class EnumTypeInfo(TypeInfoBase):
    def __cinit__(self, klass):
        self._class = klass

    cpdef to_internal_data(self, object value):
        """
        Validates and converts the given (enum) `value` to a format that the
        serializaer can understand.

        Args:
            value: should either be an instance of `self._class` or `BadEnum`.
              If it is an instance of `BadEnum`, `value` is converted to an
              `int` and returned.
              If `value` is not an instance of `self._class`, but is a py3 enum,
              it is auto-converted to a thrift-python (which must then be an
              instance of `self._class`).

        Raises:
            TypeError: the given `value` is neither a `BadEnum` nor an instance
                of `self._class` (even after py3 to thrift-python conversion, if
                applicable).
        """
        if isinstance(value, BadEnum):
            return int(value)

        if not isinstance(value, self._class):
            raise TypeError(f"value {value} is not '{self._class}', is actually of type {type(value)}.")

        return value._fbthrift_value_

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        try:
            return self._class(value)
        except ValueError:
            return BadEnum(self._class, value)

    def to_container_value(self, object value not None):
        if not isinstance(value, self._class):
            raise TypeError(f"value {value} is not '{self._class}', is actually of type {type(value)}.")
        return value

    cdef const cTypeInfo* get_cTypeInfo(self):
        return &i32TypeInfo

    def same_as(EnumTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, EnumTypeInfo):
            return False

        return self._class == (<EnumTypeInfo>other)._class

    def __reduce__(self):
        return (EnumTypeInfo, (self._class,))

@_cython__final
cdef class AdaptedTypeInfo(TypeInfoBase):
    def __cinit__(self, orig_type_info, adapter_class, transitive_annotation_factory):
        self._orig_type_info = orig_type_info
        self._adapter_class = adapter_class
        self._transitive_annotation_factory = transitive_annotation_factory

    # validate and convert to format serializer may understand
    cpdef to_internal_data(self, object value):
        if value is None:
            raise TypeError("Argument 'value' must not be None")

        return (<TypeInfoBase>self._orig_type_info).to_internal_data(
            self._adapter_class.to_thrift(
                value,
                transitive_annotation=self._transitive_annotation_factory(),
            )
        )

    # convert deserialized data to user format
    cpdef to_python_value(self, object value):
        return self._adapter_class.from_thrift(
            (<TypeInfoBase>self._orig_type_info).to_python_value(value),
            transitive_annotation=self._transitive_annotation_factory(),
        )

    def to_container_value(self, object value not None):
        return value

    cdef const cTypeInfo* get_cTypeInfo(self):
        return (<TypeInfoBase>self._orig_type_info).get_cTypeInfo()

    def same_as(AdaptedTypeInfo self, other):
        if other is self:
            return True

        if not isinstance(other, AdaptedTypeInfo):
            return False

        cdef AdaptedTypeInfo other_typeinfo = other

        # TypeInfoBase::same_as specifies the semantics of same_as as follows:
        #   `same_as()` returns `True` if the `TypeInfo` class maps the same IDL Thrift
        #   type to the same Python type.
        #
        # Adapter is defined as follows:
        #   class Adapter(typing.Generic[TAdaptFrom, TAdaptTo]):
        #       @classmethod
        #       def from_thrift(...) -> TAdaptTo: ...
        #       @classmethod
        #       def to_thrift(...) -> TAdaptFrom: ...
        #
        # As you can see the types from_thrift and to_thrift are purely
        # dependent of the TAdaptFrom (the thrift Type) and the TAdaptTo (the python type)
        # The transitive annotation has no part in the type calculus and should not
        # appear in the comparison.
        #
        return (self._orig_type_info.same_as(other_typeinfo._orig_type_info) and
            self._adapter_class == other_typeinfo._adapter_class)

    def __reduce__(self):
        return (AdaptedTypeInfo, (self._orig_type_info, self._adapter_class, self._transitive_annotation_factory))


cdef void set_struct_field(tuple struct_tuple, int16_t index, value) except *:
    """
    Updates the given `struct_tuple` to have the given `value` for the field at
    the given `index`.

    The "isset" byte for the corresponding field (i.e., the `index`-th byte of
     the first element of `struct_tuple` is set to 1.

     Args:
        struct_tuple: see `createImmutableStructTupleWithDefaultValues()`
        index: field index, as defined by its insertion order in the parent
            `StructInfo` (this is not the field id).
        value: new value for this field, in "internal data" represntation (as
            opposed to "Python value" representation - see `*TypeInfo` classes).
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
    cdef _fbthrift_get_cached_field_value(self, int16_t index):
        raise NotImplementedError("Not implemented on base StructOrUnion class")

    @staticmethod
    def from_python(obj: StructOrUnion) -> StructOrUnion:
        if not isinstance(obj, StructOrUnion):
            raise TypeError(f'value {obj} expected to be a thrift-python Struct or Union, was actually of type ' f'{type(obj)}')
        return obj

    @classmethod
    def _fbthrift_auto_migrate_enabled(cls):
        return False

def _unpickle_struct(klass, bytes data):
    cdef IOBuf iobuf = IOBuf(data)
    cdef Struct inst = klass._fbthrift_new()
    inst._deserialize(iobuf, Protocol.COMPACT)
    return inst

cdef api object _get_fbthrift_data(object struct_or_union):
    return (<StructOrUnion> struct_or_union)._fbthrift_data

cdef api object _get_exception_fbthrift_data(object generated_error):
    return (<GeneratedError> generated_error)._fbthrift_data

cdef _fbthrift_compare_struct_less(lhs, rhs, return_if_same_value):
    if type(lhs) != type(rhs):
        return NotImplemented
    for name, lhs_value in lhs:
        rhs_value = getattr(rhs, name)
        if lhs_value == rhs_value:
            continue
        if lhs_value is None:
            return True
        if rhs_value is None:
            return False
        return lhs_value < rhs_value
    return return_if_same_value

cdef class Struct(StructOrUnion):
    """
    Base class for all generated classes corresponding to a Thrift struct in
    thrift-python.

    Instance variables:
        _fbthrift_data: "struct tuple" that holds the "isset" flag array and
            values for all fields. See `createImmutableStructTupleWithDefaultValues()`.

        _fbthrift_field_cache: Tuple of length numFields. Each item may contain
            a previously retrieved value for a non-primitive (or adapted) field.
            If present, values are in the "Python" representation (as opposed to
            the "internal data" representations - see `*TypeInfo` classes).
    """

    def __cinit__(self, *_, **kwargs):
        """

        Args:
            **kwargs: names and values of the Thrift fields to set for this
                 instance. All names must match declared fields of this Thrift
                 Struct (or a `TypeError` will be raised). Values are in
                 "Python value" representation, as opposed to "internal data"
                 representation (see `*TypeInfo` classes in this file).
            *_: accept and ignore positional arguments. This is necessary in
                cases where users derive from `Struct` and pass positional
                arguments during initialization.
        """
        cdef StructInfo struct_info = self._fbthrift_struct_info
        if _fbthrift_is_cinder_runtime or getattr(self, '_fbthrift_disable_field_cache_DO_NOT_USE', False):
            # in cinder, caching happens in property layer
            self._fbthrift_field_cache = None
        else:
            self._fbthrift_field_cache = PyTuple_New(len(struct_info.fields))

    def __init__(self, **kwargs):
        self._initStructTupleWithValues(kwargs)

    def __call__(self, **kwargs):
        if not kwargs:
            return self
        cdef StructInfo struct_info = self._fbthrift_struct_info
        klass = type(self)
        # note this puts the set kwargs into new_inst._fbthrift_data
        cdef Struct new_inst = klass._fbthrift_new(**kwargs)
        not_found = object()
        isset_flags = self._fbthrift_data[0]

        for field_name, field_index in struct_info.name_to_index.items():
            value = kwargs.pop(field_name, not_found)
            if value is None:  # reset to default value, no change needed
                continue
            if value is not_found:  # borrow ref to old value
                if isset_flags[field_index] == 0:
                    # old field not set, so keep default
                    continue
                borrowed_value = self._fbthrift_data[field_index + 1]
                set_struct_field(new_inst._fbthrift_data, field_index, borrowed_value)

        if kwargs:
            raise TypeError(
                f"'{type(self).__name__}' object does not have attribute(s): "
                f"'{', '.join(kwargs.keys())}'"
            )
        return new_inst

    def __copy__(Struct self):
        return self

    def __deepcopy__(Struct self, _memo):
        return self

    def __replace__(Struct self, **kwargs):
        return self(**kwargs)

    def __eq__(Struct self, other):
        if type(other) != type(self):
            return False
        for name, value in self:
            if value != getattr(other, name):
                return False
        return True

    def __lt__(self, other):
        return _fbthrift_compare_struct_less(self, other, False)

    def __le__(self, other):
        return _fbthrift_compare_struct_less(self, other, True)

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
        return (
            _unpickle_struct, (type(self), b''.join(self._serialize(Protocol.COMPACT)))
        )

    cdef folly.iobuf.IOBuf _serialize(self, Protocol proto):
        cdef StructInfo info = self._fbthrift_struct_info
        return folly.iobuf.from_unique_ptr(
            cmove(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _deserialize(self, folly.iobuf.IOBuf buf, Protocol proto) except? 0:
        cdef StructInfo info = self._fbthrift_struct_info
        cdef uint32_t len = cdeserialize(
            deref(info.cpp_obj), buf._this, self._fbthrift_data, proto
        )
        return len

    cdef _fbthrift_py_value_from_internal_data(self, int16_t index):
        cdef StructInfo struct_info = self._fbthrift_struct_info
        cdef FieldInfo field_info = struct_info.fields[index]
        cdef int field_id = field_info.id
        adapter_info = field_info.adapter_info
        data = self._fbthrift_data[index + 1]
        if data is not None:
            py_value = (
                (<TypeInfoBase>struct_info.type_infos[index]).to_python_value(data)
            )
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

        return py_value

    cdef _fbthrift_get_cached_field_value(self, int16_t index):
        cdef PyObject* cached_value = PyTuple_GET_ITEM(
            self._fbthrift_field_cache, index
        )
        if cached_value != NULL:
            return <object>cached_value

        py_value = self._fbthrift_py_value_from_internal_data(index)

        PyTuple_SET_ITEM(self._fbthrift_field_cache, index, py_value)
        Py_INCREF(py_value)
        return py_value

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

    cdef _initStructTupleWithValues(self, kwargs):
        cdef StructInfo struct_info = self._fbthrift_struct_info

        # If no keyword arguments are provided, initialize the Struct with default values.
        if not kwargs:
            self._fbthrift_data = createImmutableStructTupleWithDefaultValues(
                struct_info.cpp_obj.get().getStructInfo()
            )
            return

        # Instantiate a tuple with 'None' values, then assign the provided keyword arguments
        # to the respective fields.
        self._fbthrift_data = createStructTupleWithNones(
            struct_info.cpp_obj.get().getStructInfo()
        )
        for name, value in kwargs.items():
            field_index = struct_info.name_to_index.get(name)
            if field_index is None:
                # try mangled name if leading dunder prefix
                field_index = _get_index_if_mangled(struct_info, self, name)
                if field_index is None:
                    raise TypeError(
                        f"'{type(self).__name__}' initialization error: unknown keyword argument "
                        f"'{name}'."
                    )

            if value is None:
                continue

            field_spec = struct_info.fields[field_index]

            try:
                # Handle field w/ adapter
                adapter_info = field_spec.adapter_info
                if adapter_info is not None:
                    adapter_class, transitive_annotation = adapter_info
                    field_id = field_spec.id
                    value = adapter_class.to_thrift_field(
                                value,
                                field_id,
                                self,
                                transitive_annotation=transitive_annotation(),
                            )

                set_struct_field(
                    self._fbthrift_data,
                    field_index,
                    (
                        <TypeInfoBase>struct_info.type_infos[field_index]
                    ).to_internal_data(value),
                )
            except Exception as exc:
                raise type(exc)(
                    f"{type(self)}: error initializing Thrift struct field "
                    f"'{field_spec.py_name}': {exc}"
                ) from exc

        # If any fields remain unset, initialize them with their respective default
        # values.
        populateImmutableStructTupleUnsetFieldsWithDefaultValues(
                self._fbthrift_data,
                struct_info.cpp_obj.get().getStructInfo()
        )

    # Initializes Struct _fbthrift_data from already valid internal `fbthrift_data`
    # Used for lazy initialization of struct-type field on first access
    # On completion:
    #   - iternal data `_fbthrift_data` is valid
    #   - `_fbthrift_field_cache` contains no cached values
    #   - primitive attributes are set
    @classmethod
    def _fbthrift_from_internal_data(cls, tuple fbthrift_data not None):
        cdef Struct inst = cls.__new__(cls)
        inst._fbthrift_data = fbthrift_data
        return inst

    # Initializes Struct to partially valid state.
    # On completion:
    #   - Struct internal data `_fbthrift_data` is valid
    #   - Struct `_fbthrift_field_cache` contains no cached values
    @classmethod
    def _fbthrift_new(cls, **kwargs):
        cdef Struct inst = cls.__new__(cls)
        inst._initStructTupleWithValues(kwargs)
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

cdef tuple _validate_union_init_kwargs(
    object union_class, object fields_enum_type, dict kwargs
):
    """
    Validates the given Thrift union initialization keyword arguments and returns the
    data needed to set the corresponding field (if any).

    Returns: tuple[field_enum, field_value], where:
        `field_enum` corresponds to the field being initialized, and must be one of the
        values in the `field_enum_type` enumeration type. If no field is
        being initialized (i.e., the Thrift union is empty), the member of that
        enumeration type with value 0, corresponding to an "empty union", is returned.

        `field_value` holds the value specified for the corresponding field, in "python
        value" format (as opposed to "internal data", see `TypeInfoBase`). If no field
        is specified (see `field_enum` above), then this value is `None`.

    Raises: TypeError if the given keyword arguments are invalid.
    """

    current_field_enum = None
    current_field_value = None

    for field_name, field_value in kwargs.items():
        if current_field_enum is not None and field_value is not None:
            raise TypeError(
                f"Cannot initialize Thrift union ({union_class.__name__}) with more "
                f"than one keyword argument (got non-None value for {field_name}, but "
                f"already had one for {current_field_enum.name})."
            )

        # Check that the field name is valid, regardless of whether it has a value.
        try:
            field_enum = fields_enum_type[field_name]
        except KeyError as e:
            raise TypeError(
                f"Cannot initialize Thrift union ({union_class.__name__}): unknown "
                f"field ({field_name})."
            ) from e
        else:
            if field_value is None:
                continue

            current_field_enum = field_enum
            current_field_value = field_value

    if current_field_enum is None:
        assert current_field_value is None
        return (fields_enum_type(0), None)
    else:
        assert current_field_value is not None
        return (current_field_enum, current_field_value)

# attributes that start with __ are mangled to _{{struct:name}}__{{field:py_name}}
cdef inline _get_index_if_mangled(StructInfo struct_info, instance, str name) noexcept:
    if not name.startswith("__"):
        return None
    cdef str mangle = f"_{instance.__class__.__name__}{name}"
    return struct_info.name_to_index.get(mangle)

cdef class Union(StructOrUnion):
    """
    Base class for all generated (immutable) thrift-python unions.

    Concrete derived classes of this base class are created by the `UnionMeta`
    metaclass.

    Attributes:
        type: (instance of the "field enum type" `type(self).Type`) Which field is
            currently holding a value, or `EMPTY` if none.

        value: (Optional[object]) Current value set for this union, if any. Its actual
            type should match that of the field corresponding to the `type` attribute.

        One attribute per union field name (see `UnionMeta`).

        _fbthrift_data: see `createUnionTuple()`
    """
    def __cinit__(self):
        self._fbthrift_data = createUnionTuple()
        self.py_type = None

    def __init__(self, **kwargs):
        self_type = type(self)
        field_enum, field_python_value = _validate_union_init_kwargs(
            self_type, self_type.Type, kwargs
        )
        cdef int field_id = field_enum.value

        # If no field is specified, exit early.
        if field_id == 0:
            self._fbthrift_update_current_field_attributes()
            return

        try:
            self._fbthrift_set_union_value(
                field_id,
                self._fbthrift_to_internal_data(field_id, field_python_value),
            )
        except Exception as exc:
            raise type(exc)(
                f"{type(self)}: error initializing Thrift union with field "
                f"'{field_enum.name}': {exc}"
            ) from exc

    # Initializes Union _fbthrift_data from already valid internal `fbthrift_data`
    @classmethod
    def _fbthrift_from_internal_data(cls, tuple data not None):
        cdef Union inst = cls.__new__(cls)
        inst._fbthrift_data = data
        inst._fbthrift_update_current_field_attributes()
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

    cdef void _fbthrift_set_union_value(self, field_id, value) except *:
        """
        Args:
            field_id (int)
            value (object)
        """
        Py_INCREF(field_id)
        old_field_id = self._fbthrift_data[0]
        PyTuple_SET_ITEM(self._fbthrift_data, 0, field_id)
        Py_DECREF(old_field_id)

        old_value = self._fbthrift_data[1]
        Py_INCREF(value)
        PyTuple_SET_ITEM(self._fbthrift_data, 1, value)
        Py_DECREF(old_value)

        self._fbthrift_update_current_field_attributes()

    cdef void _fbthrift_update_current_field_attributes(self) except *:
        """
        Updates the `value` attribute from the internal data tuple
        of this union (`self._fbthrift_data`).
        Resets `py_type` to None
        """
        self.py_type = None
        val = self._fbthrift_data[1]
        if val is None:
            self.value = None
            return
        cdef UnionInfo info = self._fbthrift_struct_info
        self.value = (
            info.type_infos[self._fbthrift_data[0]].to_python_value(val)
        )

    cdef folly.iobuf.IOBuf _serialize(self, Protocol proto):
        cdef UnionInfo info = self._fbthrift_struct_info
        return folly.iobuf.from_unique_ptr(
            cmove(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _deserialize(self, folly.iobuf.IOBuf buf, Protocol proto) except? 0:
        cdef UnionInfo info = self._fbthrift_struct_info
        cdef uint32_t size = cdeserialize(deref(info.cpp_obj), buf._this, self._fbthrift_data, proto)
        self._fbthrift_update_current_field_attributes()
        return size

    cdef _fbthrift_get_cached_field_value(self, int16_t field_id):
        """
        Returns the value of the field with the given `field_id` if it is indeed the
        field that is (currently) set for this union. Otherwise, raises AttributeError.
        """
        if _fbthrift_get_Union_type_int(self) != field_id:
            # TODO in python 3.10 update this to use name and obj fields
            raise AttributeError(
                f'Union contains a value of type {self.get_type().name}, not '
                f'{type(self).Type(field_id).name}')
        return self.value


    cdef object _fbthrift_py_type_enum(self):
        '''
        Initializes self.py_type enum if None.
        '''
        if self.py_type is None:
            self.py_type = type(self).Type(
                _fbthrift_get_Union_type_int(self)
            )
        return self.py_type

    @property
    def type(Union self not None):
        return self._fbthrift_py_type_enum()

    def get_type(Union self not None):
        return self._fbthrift_py_type_enum()

    @property
    def fbthrift_current_field(Union self not None):
        return self._fbthrift_py_type_enum()

    @property
    def fbthrift_current_value(Union self not None):
        return self.value

    @classmethod
    def fromValue(cls, value):
        """
        Creates a new instance of this Thrift union, populating the first field whose
        type is compatible with the given `value`. If `value` is None, the returned
        instance is empty.

        WARNING: The heuristic above can lead to confusing behavior (see union_test.py),
        and therefore usage of this method is strongly discouraged.

        Args:
            value (typing.Optional[object])
        """
        cdef Union union_instance = cls.__new__(cls)
        if value is None:
            return union_instance
        cdef UnionInfo union_info = cls._fbthrift_struct_info
        for type_value, typeinfo in union_info.type_infos.items():
            # stricter type checking for consistency with py3
            if _strict_type_info_mismatch(value, typeinfo):
                continue
            try:
                value = union_instance._fbthrift_to_internal_data(type_value, value)
            except (TypeError, OverflowError):
                continue
            else:
                union_instance._fbthrift_set_union_value(type_value, value)
                break
        return union_instance

    def __copy__(Union self):
        return self

    def __deepcopy__(Union self, _memo):
        return self

    def __eq__(Union self not None, other):
        if type(other) != type(self):
            return False
        cdef Union other_u = other
        cdef int self_type_int = _fbthrift_get_Union_type_int(self)
        cdef int other_type_int = _fbthrift_get_Union_type_int(other_u)
        return  self_type_int == other_type_int and self.value == other_u.value

    def __lt__(Union self not None, other):
        if type(self) != type(other):
            return NotImplemented
        cdef Union other_u = other
        cdef int self_type_int = _fbthrift_get_Union_type_int(self)
        cdef int other_type_int = _fbthrift_get_Union_type_int(other_u)
        return (self_type_int, self.value) < (other_type_int, other_u.value)

    def __le__(Union self not None, other):
        if type(self) != type(other):
            return NotImplemented
        cdef Union other_u = other
        cdef int self_type_int = _fbthrift_get_Union_type_int(self)
        cdef int other_type_int = _fbthrift_get_Union_type_int(other_u)
        return (self_type_int, self.value) <= (other_type_int, other_u.value)

    def __hash__(Union self not None):
        cdef int self_type_int = _fbthrift_get_Union_type_int(self)
        return hash((self_type_int, self.value))

    def __repr__(self):
        return f"{type(self).__name__}({self.type.name}={self.value!r})"

    def __bool__(self not None):
        return _fbthrift_get_Union_type_int(self) != 0

    def __dir__(self):
        return dir(type(self))

    def __reduce__(self):
        return (_unpickle_union, (type(self), b''.join(self._serialize(Protocol.COMPACT))))


cdef inline int _fbthrift_get_Union_type_int(Union u):
    return u._fbthrift_data[0]

# stricter type checking for Union.fromValue for easily confused types
#   - typeinfo_int accidentally accepts bool because bool subclasses int
#   - typeinfo_float accepts int since int in range can be converted to double
# returns True if value is convertible to type_info but is not strict match
# returns False if no mismatch; there may be mismatch detectable by TypeError
cdef inline pbool _strict_type_info_mismatch(value, type_info):
    if isinstance(type_info, IntegerTypeInfo):
        return isinstance(value, (float, bool))
    if type_info is typeinfo_float or type_info is typeinfo_double:
        return not isinstance(value, float)
    return False


cdef _make_fget_union(field_id, adapter_info):
    """
    Returns a function that takes a `Union` instance and returns the value of the field
    with the given `field_id`.

    If `adapter_info` is not None, the corresponding adapter will be called with the
    field value prior to returning.

    Args:
        field_id (int)
        adapter_info (typing.Optional[object])
    """
    if adapter_info:
        adapter_class, transitive_annotation = adapter_info
        return property(lambda self:
            adapter_class.from_thrift_field(
                (<Union>self)._fbthrift_get_cached_field_value(field_id),
                field_id,
                self,
                transitive_annotation=transitive_annotation(),
            )
        )
    return property(lambda self: (<Union>self)._fbthrift_get_cached_field_value(field_id))


def _make_readonly_mutate_attr():
    """
    Returns a setter and deleter for read-only attributes, always throws AttributeError.
    """
    def _readonly_setattr(self, name, _value):
        raise AttributeError(f"Cannot set attribute '{name}' in Thrift struct '{type(self)}'.")

    def _readonly_delattr(self, name):
        raise AttributeError(f"Cannot delete attribute '{name}' in Thrift struct '{type(self)}'.")

    return _readonly_setattr, _readonly_delattr

cdef class _FieldDescriptorBase:
    """
    A descriptor parent class that enforces immutability.
    """
    cdef str _field_name

    def __init__(self, str field_name):
        self._field_name = field_name

    def __set__(self, obj, value):
        raise AttributeError(f"Cannot set attribute {self._field_name}: thrift-python structs are immutable")

    def __delete__(self, obj):
        raise AttributeError(f"Cannot delete attribute {self._field_name}: thrift-python structs are immutable")


@_cython__final
cdef class _StructCachedField(_FieldDescriptorBase):
    """
    A descriptor that enforces immutability and implements cached field access.
    """
    cdef int16_t _field_index

    def __init__(self, int16_t field_index, str field_name):
        self._field_index = field_index
        super().__init__(field_name)

    def __get__(self, Struct obj, objtype):
        if obj is None:
            return None

        return obj._fbthrift_get_cached_field_value(self._field_index)

@cython.final
cdef class _StructUncachedField(_FieldDescriptorBase):
    """ A descriptor that for UncachedField.
        Return _fbthrift_py_value_from_internal_data directly.
    """
    cdef int16_t _field_index

    def __init__(self, int16_t field_index, str field_name):
        self._field_index = field_index
        super().__init__(field_name)

    def __get__(self, Struct obj, objtype):
        if obj is None:
            return None
        return obj._fbthrift_py_value_from_internal_data(self._field_index)


@_cython__final
cdef class _StructPrimitiveField(_FieldDescriptorBase):
    """
    A descriptor that enforces immutability and implements field access for
    types where the internal data type is exactly the python value type.
    """
    cdef int16_t _tuple_index

    def __init__(self, int16_t field_index, str field_name):
        self._tuple_index = field_index + 1
        super().__init__(field_name)

    def __get__(self, Struct obj, objtype):
        if obj is None:
            return None

        return obj._fbthrift_data[self._tuple_index]

cdef inline _is_primitive_field(FieldInfo field_info) noexcept:
    return field_info.is_primitive and field_info.adapter_info is None \
        and not isinstance(field_info.type_info, AdaptedTypeInfo)


class StructMeta(type):
    """Metaclass for all generated (immutable) thrift-python Struct types."""

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
        """
        for base in bases:
            if getattr(base, '_fbthrift_allow_inheritance_DO_NOT_USE', False):
                return super().__new__(cls, cls_name, bases, dct)
            raise TypeError(
                f"Inheritance from generated thrift struct {cls_name} is deprecated. Please use composition."
            )
        # Set[Tuple (field spec)]. See `StructInfo` class docstring for the
        # contents of the field spec tuples.
        fields = dct.pop('_fbthrift_SPEC', ())

        dct["_fbthrift_struct_info"] = StructInfo(cls_name, fields)

        cdef list slots = []
        for i, field_info in enumerate(fields):
            slots.append(field_info.py_name)

        dct["__slots__"] = slots
        all_bases = bases if bases else (Struct,)
        klass = super().__new__(cls, cls_name, all_bases, dct)

        for field_index, field_info in enumerate(fields):
            field_name = field_info.py_name
            if _is_primitive_field(field_info):
                descriptor = _StructPrimitiveField(field_index, field_name)
            else:
                descriptor = _make_non_primitive_property(
                    klass,
                    field_index,
                    field_name,
                )
            type.__setattr__(klass, field_name, descriptor)

        klass.__setattr__, klass.__delattr__ = _make_readonly_mutate_attr()
        return klass

    def _fbthrift_fill_spec(cls):
        """
        Completes initialization of all specs for this Struct class.

        This should be called once, after all generated classes (unions and
        structs) for a given module have been created.

        Typically called by `fill_specs()`, at the end of the generated thrift_types
        module (after all type classes have been created).
        """
        (<StructInfo>cls._fbthrift_struct_info)._fill_struct_info()

    def _fbthrift_store_field_values(cls):
        """
        Initializes the default values of fields (if any) for this Struct.

        This should be called once, after `_fbthrift_fill_spec()` has been
        called for all generated classes (unions and structs) in a module.
        """
        (<StructInfo>cls._fbthrift_struct_info)._initialize_default_values()

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


def _gen_union_field_enum_members(field_infos):
    """
    Generates a sequence of (enum name, enum value) pairs for the given fields of a
    Thrift union.

    Args:
        field_infos (Iterable[FieldInfo])

    Yields: (name, value) pairs, where:
        name (str): py_name of the corresponding Thrift union field, or 'EMPTY'.
        value (int): value of the corresponding Thrift union field, or 0 for 'EMPTY'.

        The first pair yielded is always `("EMPTY", 0)`, regardless of the given field
        definitions. Subsequently, a pair is yielded for every field in the given
        `field_infos`.
    """
    yield ("EMPTY", 0)
    for f in field_infos:
        yield (f.py_name, f.id)


class UnionMeta(type):
    def __new__(cls, union_name, bases, union_class_namespace):
        """
        Returns a new class corresponding to a thrift-python Union type.

        Args:
            union_name (str)
            bases (tuple): Parent classes. Must be empty.
            union_class_namespace (dict)

        Returns:
            A new class, with the given `union_name`, corresponding to a Thrift union.
            The returned class inherits from `Union`. It has the following attributes:

            At the class level:
                _fbthrift_struct_info (UnionInfo)

                Type (class enum.Enum): Enumeration of all fields declared for this
                    Thrift union (see `_gen_union_field_enum_members()`).

                FbThriftUnionFieldEnum: Alias for Type

            At the instance level (see also `Union`):
                A property for every field in the Thrift union, with its corresponding
                `py_name`.
        """
        if bases:
            raise TypeError(
                f"Inheritance from generated thrift union {bases[0].__name__} is deprecated."
                " Please use composition."
            )

        field_infos = union_class_namespace.pop('_fbthrift_SPEC')
        union_class_namespace["_fbthrift_struct_info"] = UnionInfo(
            union_name, field_infos
        )
        cdef list slots = []
        for field_info in field_infos:
            slots.append(field_info.py_name)
        union_class_namespace["__slots__"] = slots
        klass = super().__new__(cls, union_name, (Union,), union_class_namespace)
        for field_info in field_infos:
            type.__setattr__(
                klass,
                field_info.py_name,
                _make_fget_union(field_info.id, field_info.adapter_info),
            )

        FbThriftUnionFieldEnum = (
            union_class_namespace.pop('_fbthrift_abstract_base_class').FbThriftUnionFieldEnum
                if "_fbthrift_abstract_base_class" in union_class_namespace
                else enum.Enum(union_name, _gen_union_field_enum_members(field_infos))
        )

        type.__setattr__(
            klass,
            "FbThriftUnionFieldEnum",
            FbThriftUnionFieldEnum,
        )
        type.__setattr__(
            klass,
            "Type",
            FbThriftUnionFieldEnum,
        )

        return klass

    def __dir__(cls):
        return tuple((<UnionInfo>cls._fbthrift_struct_info).name_to_index.keys()) + (
            "type", "value", 'fbthrift_current_field', 'fbthrift_current_value')

    def _fbthrift_fill_spec(cls):
        (<UnionInfo>cls._fbthrift_struct_info)._fill_union_info()

@_cython__final
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

# TODO: unify List and MutableList so `self` can be typed
cdef list_eq(object self, object other):
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


@_cython__final
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

    def _fbthrift_same_type(self, other_elem_type):
        return self._fbthrift_val_info.same_as(other_elem_type)

tag_object_as_sequence(<PyTypeObject*>List)
Sequence.register(List)


@_cython__final
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
        return Set(self._fbthrift_val_info, self._fbthrift_elements & other)

    def __rand__(Set self, other):
        return other & self._fbthrift_elements

    def __sub__(Set self, other):
        return Set(self._fbthrift_val_info, self._fbthrift_elements - other)

    def __rsub__(Set self, other):
        return other - self._fbthrift_elements

    def __or__(Set self, other):
        return Set(self._fbthrift_val_info, self._fbthrift_elements | other)

    def __ror__(Set self, other):
        return other | self._fbthrift_elements

    def __xor__(Set self, other):
        return Set(self._fbthrift_val_info, self._fbthrift_elements ^ other)

    def __rxor__(Set self, other):
        return other ^ self._fbthrift_elements

    def __eq__(Set self, other):
        return self._fbthrift_elements == other

    def __lt__(Set self, other):
        return self._fbthrift_elements < other

    def __gt__(Set self, other):
        # For sets, `x > y` or `x >= y` can not be implemented using `not (x <= y)` or `not (x < y)`. 
        # Because `<` for sets form partial order unlike integers which form total order.
        # For example if `x > y` was implemented as `not (x <= y)` => `not (x == y or x < y)`, then we can 
        # find a counter example. Let `x={0}` and `y={1}`: `x > y` is False, but `not (x == y or x < y)` is True.
        return self._fbthrift_elements > other

    def __le__(Set self, other):
        return self == other or self < other

    def __ge__(Set self, other):
        return self == other or self > other

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

    def _fbthrift_same_type(self, other_elem_type):
        return self._fbthrift_val_info.same_as(other_elem_type)

pySet.register(Set)

@_cython__final
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
    A immutable container used to represent a Thrift map. It has compatible
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

    def _fbthrift_same_type(self, other_key_type, other_val_type):
        return (
            self._fbthrift_key_info.same_as(other_key_type) and
            self._fbthrift_val_info.same_as(other_val_type)
        )

tag_object_as_mapping(<PyTypeObject*>Map)
Mapping.register(Map)


# We will create all the classes first then call fill_specs after that so
# dependancies can be properly solved.
def fill_specs(*structured_thrift_classes):
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
        *structured_thrift_classes: Sequence of class objects, each one of which
        corresponds to either a `Struct` (i.e., created by/instance of `StructMeta`) or
        a `Union` (i.e., created by/instance of `UnionMeta`).
    """

    for cls in structured_thrift_classes:
        cls._fbthrift_fill_spec()

    for cls in structured_thrift_classes:
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
            # name -> arm
            # NOTE: leading double underscore is used to preserve any previous
            # application logic that directly inspected internal attributes of Thrift
            # enum types and filtered out "private" members using them as a criterion.
            "__member_map__": {},
            # value -> arm
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
        if int in bases:
            type.__setattr__(klass, "__eq__", Enum.__int__eq__)

        for name, value in dct.items():
            if not isinstance(value, int) or name == "__firstlineno__":
                continue
            # pass value to int parent class constructor
            arm = klass.__new__(klass, value)
            arm._fbthrift_name_ = name
            arm._fbthrift_value_ = value
            klass.__member_map__[name] = arm
            klass.__reversed_map__[value] = arm
            type.__setattr__(klass, name, arm)
        return klass

    def __len__(cls):
        return len(cls.__member_map__)

    def __getitem__(cls, attribute):
        return cls.__member_map__[attribute]

    @property
    def __members__(cls):
        """
        Returns a read-only mapping of all names to their instance for this Thrift enum
        type.
        """
        return MappingProxyType(cls.__member_map__)

    def __contains__(cls, item):
        if isinstance(item, cls):
            return True

        # This is a foreign flag, and since they can compare as ints
        # They could show as 'in' an enum, by stdlib doesn't
        if isinstance(item, Flag):
            return False

        # Special Behavior for lazy flag combinations
        if issubclass(cls, Flag):
            try:
                # Try O(1) but fall back to O(n)
                return item in cls.__reversed_map__ or cls._fbthrift_missing_(item) is not None
            except ValueError:
                return False
        return item in cls.__reversed_map__

    def __iter__(cls):
        return iter(cls.__member_map__.values())

    def __reversed__(cls):
        return reversed(cls.__member_map__.values())

    def __setattr__(cls, name, _):
        raise AttributeError(f"Thrift enum type '{cls.__qualname__}': cannot assign member ('{name}').")

    def __delattr__(cls, name):
        raise AttributeError(f"Thrift enum type '{cls.__qualname__}': cannot delete member ('{name}').")

    def __call__(cls, value):
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

    def __dir__(cls):
        return list(cls.__member_map__.keys()) + [
            '__class__',
            '__doc__',
            '__members__',
            '__module__',
        ]

cdef inline bint _enum_eq_(self, other):
    if isinstance(other, Enum):
        # enums are singletons, so use `is` to compare
        return self is other
    if cFollyIsDebug and isinstance(other, (bool, float)):
        warnings.warn(
            f"Did you really mean to compare {type(self)} and {type(other)}?",
            RuntimeWarning,
            stacklevel=1
        )
    return self._fbthrift_value_ == other

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
        return _enum_eq_(self, other)

    # thrift-python enums have int base, so have to define
    # __ne__ to avoid __ne__ based on int value alone
    def __ne__(self, other):
        return not (self == other)

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

    def __int__eq__(self, other):
        if type(self) is type(other):
            return PyLong_AsLong(self) == PyLong_AsLong(other)
        return _enum_eq_(self, other)


class Flag(Enum):
    @classmethod
    def _fbthrift_missing_(cls, value):
        """
        Returns member (possibly creating it) if one can be found for value.
        """
        if value == 0:
            return cls._fbthrift_create_pseudo_member_(value)

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
            # must pass value to int parent class constructor
            pseudo_member = cls.__new__(cls, value)
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

    def __int__(self):
        return self.value

    def __invert__(self):
        cls = type(self)
        res = self._fbthrift_value_
        for m in cls:
            res ^= m._fbthrift_value_
        return cls(res)


@cython.auto_pickle(False)
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


def get_standard_immutable_default_value_for_type(TypeInfoBase typeinfo):
    return typeinfo.to_python_value(getStandardImmutableDefaultValuePtrForType(typeinfo.get_cTypeInfo()[0]))

# for fbthrift test introspection only, DO NOT USE elsewhere
def _fbthrift__runtime_is_cinder():
    return pbool(_fbthrift_is_cinder_runtime)
