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

from libcpp.memory cimport make_unique
from libcpp.utility cimport move as std_move
from folly.iobuf cimport from_unique_ptr

from cpython.object cimport PyCallable_Check
from cpython.ref cimport Py_INCREF, Py_DECREF
from cpython.tuple cimport PyTuple_New, PyTuple_SET_ITEM
from cpython.unicode cimport PyUnicode_AsUTF8String

from thrift.python.mutable_serializer cimport cserialize, cdeserialize
from thrift.python.mutable_typeinfos cimport (
    MutableListTypeInfo,
    MutableSetTypeInfo,
)
from thrift.python.types cimport (
    AdaptedTypeInfo,
    FieldInfo,
    StringTypeInfo,
    TypeInfoBase,
    getCTypeInfo,
    set_struct_field,
)

from cython.operator cimport dereference as deref

cdef extern from "<Python.h>":
    cdef const char * PyUnicode_AsUTF8(object unicode)


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
        to a `Struct` (i.e., created by/instance of `MutableStructMeta`)
    """

    for cls in struct_types:
        cls._fbthrift_fill_spec()

    for cls in struct_types:
        cls._fbthrift_store_field_values()


class _MutableStructField:
    """
    The `_MutableStructField` class is a descriptor class that is used to
    manage the access to a specific field in a mutable Thrift struct object.
    It uses the descriptor protocol.
    """
    # `_field_index` is the insertion order of the field in the
    # `MutableStructInfo` (this is not the Thrift field id)
    __slots__ = ('_field_index',)

    def __init__(self, field_id):
        self._field_index = field_id

    def __get__(self, obj, objtype):
        return (<MutableStruct>obj)._fbthrift_get_field_value(self._field_index)

    def __set__(self, obj, value):
        (<MutableStruct>obj)._fbthrift_set_field_value(self._field_index, value)


cdef is_cacheable_non_primitive(ThriftIdlType idl_type):
    return idl_type == ThriftIdlType.String


class _MutableStructCachedField:
    __slots__ = ('_field_index')

    def __init__(self, field_id):
        self._field_index = field_id

    def __get__(self, obj, objtype):
        return (<MutableStruct>obj)._fbthrift_get_cached_field_value(self._field_index)

    def __set__(self, obj, value):
        (<MutableStruct>obj)._fbthrift_set_field_value(self._field_index, value)
        obj._fbthrift_field_cache[self._field_index] = None


cdef is_container(ThriftIdlType idl_type):
    return idl_type in (ThriftIdlType.List, ThriftIdlType.Set, ThriftIdlType.Map)

class _MutableStructContainerField:
    __slots__ = ('_field_index')

    def __init__(self, field_id):
        self._field_index = field_id

    def __get__(self, obj, objtype):
        return (<MutableStruct>obj)._fbthrift_get_cached_field_value(self._field_index)

    def __set__(self, obj, value):
        raise TypeError(
                "Thrift container types do not support direct assignment."
                " However, you can add, remove or update items within the"
                " container")


cdef class MutableStructOrUnion:
    cdef IOBuf _fbthrift_serialize(self, Protocol proto):
        raise NotImplementedError("Not implemented on base MutableStructOrUnion class")
    cdef uint32_t _fbthrift_deserialize(self, IOBuf buf, Protocol proto) except? 0:
        raise NotImplementedError("Not implemented on base MutableStructOrUnion class")
    cdef _fbthrift_get_field_value(self, int16_t index):
        raise NotImplementedError("Not implemented on base MutableStructOrUnion class")


cdef class MutableStruct(MutableStructOrUnion):
    """
    Base class for all generated (mutable) classes corresponding to Thrift
    struct in thrift-python.

    Instance variables:
        _fbthrift_data: "mutable struct tuple" that holds the "isset" flag array and
            values for all fields. See `createMutableStructTupleWithDefaultValues()`.

        _fbthrift_field_cache: This is a list that stores instances of a field's
            Python value. It is especially useful when creating a Python value is
            relatively expensive, such as when calling the `TypeInfo.to_python_value()`
            method. For example, in the case of adapted types, we store the Python
            value in this list to avoid repeated calls to the adapter class.
            This list also stores instances when we want to return the same instance
            multiple times. For instance, if a struct field is a Thrift `list`, we store
            the `MutableList` instance in this list. This allows us to return the same
            `MutableList` instance for all attribute accesses.
    """

    def __cinit__(self, **kwargs):
        """
        Args:
            **kwargs: names and values of the Thrift fields to set for this
                 instance. All names must match declared fields of this Thrift
                 Struct (or a `TypeError` will be raised). Values are in
                 "Python value" representation, as opposed to "internal data"
                 representation (see `*TypeInfo` classes).
        """
        self._initStructTupleWithValues(kwargs)
        cdef MutableStructInfo mutable_struct_info = type(self)._fbthrift_mutable_struct_info
        self._fbthrift_field_cache = [None] * len(mutable_struct_info.fields)

    def __init__(self, **kwargs):
        pass

    cdef _initStructTupleWithValues(self, kwargs) except *:
        cdef MutableStructInfo mutable_struct_info = self._fbthrift_mutable_struct_info

        # If no keyword arguments are provided, initialize the Struct with
        # default values.
        if not kwargs:
            self._fbthrift_data = createMutableStructTupleWithDefaultValues(mutable_struct_info.cpp_obj.get().getStructInfo())
            return

        # Instantiate a tuple with 'None' values, then assign the provided
        # keyword arguments to the respective fields.
        self._fbthrift_data = createStructTupleWithNones(mutable_struct_info.cpp_obj.get().getStructInfo())
        for name, value in kwargs.items():
            field_index = mutable_struct_info.name_to_index.get(name)
            if field_index is None:
                raise TypeError(f"{self.__class__.__name__}.__init__() got an unexpected keyword argument '{name}'")

            self._fbthrift_set_field_value(field_index, value)

        # If any fields remain unset, initialize them with their respective
        # default values.
        populateMutableStructTupleUnsetFieldsWithDefaultValues(
                self._fbthrift_data,
                mutable_struct_info.cpp_obj.get().getStructInfo()
        )

    cdef _fbthrift_set_field_value(self, int16_t index, object value):
        cdef MutableStructInfo mutable_struct_info = self._fbthrift_mutable_struct_info
        cdef FieldInfo field_info = mutable_struct_info.fields[index]

        if field_info.adapter_info is not None:
            adapter_class, transitive_annotation = field_info.adapter_info
            value = adapter_class.to_thrift_field(
                value,
                field_info.id,
                self,
                transitive_annotation=transitive_annotation(),
            )

        set_struct_field(
            self._fbthrift_data,
            index,
            (<TypeInfoBase>mutable_struct_info.type_infos[index]).to_internal_data(value),
        )

    cdef _fbthrift_get_field_value(self, int16_t index):
        cdef MutableStructInfo mutable_struct_info = self._fbthrift_mutable_struct_info
        cdef TypeInfoBase field_type_info = mutable_struct_info.type_infos[index]
        cdef FieldInfo field_info = mutable_struct_info.fields[index]

        data = self._fbthrift_data[index + 1]
        if field_info.adapter_info is not None:
            py_value = field_type_info.to_python_value(data)
            adapter_class, transitive_annotation = field_info.adapter_info
            return adapter_class.from_thrift_field(
                py_value,
                field_info.id,
                self,
                transitive_annotation=transitive_annotation(),
            )

        return field_type_info.to_python_value(data) if data is not None else None

    cdef _fbthrift_get_cached_field_value(MutableStruct self, int16_t index):
        cached = self._fbthrift_field_cache[index]
        if cached is not None:
            return cached

        value = self._fbthrift_get_field_value(index)
        self._fbthrift_field_cache[index] = value
        return value

    def __eq__(MutableStruct self, other):
        if other is self:
            return True

        if type(other) != type(self):
            return False

        for name, value in self:
            if value != getattr(other, name):
                return False

        return True

    def __iter__(self):
        cdef MutableStructInfo info = self._fbthrift_mutable_struct_info
        for name in info.name_to_index:
            yield name, getattr(self, name)

    cdef _fbthrift_reset_field_to_standard_default(self, int16_t index):
        cdef MutableStructInfo mutable_struct_info = self._fbthrift_mutable_struct_info
        resetFieldToStandardDefault(
            self._fbthrift_data,
            mutable_struct_info.cpp_obj.get().getStructInfo(),
            index,
        )

    cdef IOBuf _fbthrift_serialize(self, Protocol proto):
        cdef MutableStructInfo info = self._fbthrift_mutable_struct_info
        return from_unique_ptr(
            std_move(cserialize(deref(info.cpp_obj), self._fbthrift_data, proto))
        )

    cdef uint32_t _fbthrift_deserialize(self, IOBuf buf, Protocol proto) except? 0:
        cdef MutableStructInfo info = self._fbthrift_mutable_struct_info
        cdef uint32_t lenght = cdeserialize(deref(info.cpp_obj), buf._this, self._fbthrift_data, proto)
        return lenght

    def _do_not_use_resetFieldToStandardDefault(self, field_name: str):
        """
        This method is currently intended for internal testing purposes only.
        Please avoid using it as it will be replaced with a more appropriate
        mechanism in the future
        """
        cdef MutableStructInfo mutable_struct_info = self._fbthrift_mutable_struct_info
        field_index = mutable_struct_info.name_to_index.get(field_name)
        if field_index is None:
            raise TypeError(f"got an unexpected field_name: '{field_name}'")
        self._fbthrift_reset_field_to_standard_default(field_index)
        self._fbthrift_field_cache[field_index] = None

    @classmethod
    def _fbthrift_create(cls, data):
        cdef MutableStruct inst = cls.__new__(cls)
        inst._fbthrift_data = data
        return inst


cdef class MutableStructInfo:
    """
    Stores information for a specific Thrift Struct class.

    Instance Variables:
        fields: tuple[FieldInfo, ...] (from `_fbthrift_SPEC`).

        cpp_obj: cDynamicStructInfo for this struct.

        type_infos: Tuple whose size matches the number of fields in the Thrift
            struct. Initialized by calling `fill()`.

        name_to_index: Dict[str (field name), int (index in `fields`).].
            Initialized by calling `fill()`.
    """

    def __cinit__(self, name: str, fields: tuple[FieldInfo, ...]):
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
        for idx, field_info in enumerate(self.fields):
            # field_type_info can be a lambda function so types with dependencies
            # won't need to be defined in order, see class docstring above.
            field_type_info = field_info.type_info
            if PyCallable_Check(field_type_info):
                field_type_info = field_type_info()

            # The rest of the code assumes that all the `TypeInfo` classes extend
            # from `TypeInfoBase`. Instances are typecast to `TypeInfoBase` before
            # the `to_internal_data()` and `to_python_value()` methods are called.
            if not isinstance(field_type_info, TypeInfoBase):
                raise TypeError(f"{type(field_type_info).__name__} is not subclass of TypeInfoBase.")

            Py_INCREF(field_type_info)
            PyTuple_SET_ITEM(type_infos, idx, field_type_info)
            self.name_to_index[field_info.py_name] = idx
            dynamic_struct_info.addFieldInfo(
                field_info.id,
                field_info.qualifier,
                PyUnicode_AsUTF8(field_info.name),
                getCTypeInfo(field_type_info),
            )

    cdef void _initialize_default_values(self) except *:
        """
        Initializes the default values of fields in this Struct.

        Upon successful return, the field value(s) in `self.cpp_obj` are
        iniitalized (see `DynamicStructInfo::addFieldValue()`).
        """
        cdef cDynamicStructInfo* dynamic_struct_info = self.cpp_obj.get()
        for idx, field in enumerate(self.fields):
            default_value = field.default_value
            if default_value is None:
                continue
            if callable(default_value):
                default_value = default_value()

            type_info = self.type_infos[idx]
            if isinstance(type_info, AdaptedTypeInfo):
                type_info = (<AdaptedTypeInfo>type_info)._orig_type_info

            default_value = (<TypeInfoBase>type_info).to_internal_data(default_value)
            dynamic_struct_info.addFieldValue(idx, default_value)


class MutableStructMeta(type):
    """Metaclass for all generated (mutable) thrift-python Struct types."""

    def __new__(cls, cls_name, bases, dct):
        """
        Returns a new Thrift Struct class with the given name and members.

        Args:
            cls_name (str): Name of the Thrift Struct, as specified in the
                Thrift IDL.
            bases: Unused, expected to always be empty. If not, this method
                will raise a `TypeError`.
            dct (Dict): Class members, including the SPEC for this class under
                the key '_fbthrift_SPEC'.

        Returns:
            A new class, with the given `cls_name`, corresponding to a Thrift
            Struct. The returned class inherits from `MutableStruct` and
            provides properties for all primitive and non-primitive fields
            (including any adapted fields) specified in the SPEC.

            The returned class will also have the following additional class
            attribute, meant for internal (Thrift) processing:

                _fbthrift_mutable_struct_info: MutableStructInfo
        """
        if bases:
            raise TypeError("Inheriting from thrift-python data types is forbidden: "
                           f"'{cls_name}' cannot inherit from '{bases[0].__name__}'")

        # Set[Tuple (field spec)]. See `MutableStructInfo` class docstring for the
        # contents of the field spec tuples.
        fields = dct.pop('_fbthrift_SPEC')
        dct["_fbthrift_mutable_struct_info"] = MutableStructInfo(cls_name, fields)

        # List[Tuple[int (index in fields), str (field name), int (IDL Type)]
        primitive_types = []
        # List[Tuple[int (index in fields), str (field name), int (IDL Type), Optional[AdapterInfo]]
        non_primitive_types = []

        slots = ['_fbthrift_field_cache']
        for field_index, field_info in enumerate(fields):
            slots.append(field_info.py_name)

            # if field has an adapter or is not primitive type, consider
            # as "non-primitive"
            if field_info.adapter_info is not None or not field_info.is_primitive:
                non_primitive_types.append(
                    (field_index, field_info.py_name, field_info.idl_type, field_info.adapter_info))
            else:
                primitive_types.append((field_index, field_info.py_name, field_info.idl_type))

        dct["__slots__"] = slots
        klass = super().__new__(cls, cls_name, (MutableStruct,), dct)

        for field_index, field_name, *_ in primitive_types:
            type.__setattr__(
                klass,
                field_name,
                _MutableStructField(field_index),
            )

        # DO_BEFORE(alperyoney,20240515): Implement descriptor for non-primitive
        # mutable types.
        # For now, handle non-primitive-types similarly to primitive types and
        # use the `_MutableStructField` descriptor for all types, except for
        # `list`. For `list`, use `_MutableStructContainerField` descriptor.
        for field_index, field_name, idl_type, adapter_info in non_primitive_types:
            field_descriptor = _MutableStructField(field_index)
            if is_container(idl_type):
                field_descriptor = _MutableStructContainerField(field_index)
            elif adapter_info is not None or is_cacheable_non_primitive(idl_type):
                field_descriptor = _MutableStructCachedField(field_index)

            type.__setattr__(
                klass,
                field_name,
                field_descriptor,
            )

        return klass

    def _fbthrift_fill_spec(cls):
        """
        Completes the initialization of all specs for this Struct class.

        This should be called once, after all generated classes (including
        unions and structs) for a given module have been created.
        """
        (<MutableStructInfo>cls._fbthrift_mutable_struct_info).fill()

    def _fbthrift_store_field_values(cls):
        """
        Initializes the default values of fields (if any) for this Struct.

        This should be called once, after `_fbthrift_fill_spec()` has been
        called for all generated classes (including unions and structs) in
        a module.
        """
        (<MutableStructInfo>cls._fbthrift_mutable_struct_info)._initialize_default_values()

    def __iter__(cls):
        """
        Iterating over Thrift-generated Struct classes yields the names of the
        fields in the struct.

        Should not be called prior to `_fbthrift_fill_spec()`.
        """
        cdef MutableStructInfo mutable_struct_info = cls._fbthrift_mutable_struct_info
        for name in mutable_struct_info.name_to_index.keys():
            yield name, None


class MutableUnionMeta(type):
    """Metaclass for all generated (mutable) thrift-python Union types."""

    def __new__(cls, cls_name, bases, dct):
        """
        Returns a new Thrift Union class with the given name and members.
        """
        raise NotImplementedError(
            "Mutable thrift-python Unions are not implemented yet."
        )
