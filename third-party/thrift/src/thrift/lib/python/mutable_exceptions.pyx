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

from folly.iobuf cimport from_unique_ptr
from libcpp.utility cimport move as std_move

import copy

from thrift.python.mutable_serializer cimport c_mutable_serialize, c_mutable_deserialize
import thrift.python.mutable_serializer as mutable_serializer
from thrift.python.mutable_types cimport (
    MutableStructInfo,
    set_mutable_struct_field,
    _mutable_struct_meta_new,
)
from thrift.python.types cimport (
    FieldInfo,
    TypeInfoBase,
    _fbthrift_compare_struct_less,
)

from cython.operator cimport dereference as deref


class MutableGeneratedErrorMeta(type):
    """Metaclass for all generated (mutable) thrift-python Exception types."""

    def __new__(cls, cls_name, bases, dct):
        if bases:
            raise TypeError("Inheriting from thrift-python data types is forbidden: "
                           f"'{cls_name}' cannot inherit from '{bases[0].__name__}'")

        return _mutable_struct_meta_new(cls, cls_name, (MutableGeneratedError,), dct)

    def _fbthrift_fill_spec(cls):
        (<MutableStructInfo>cls._fbthrift_mutable_struct_info).fill()

    def _fbthrift_store_field_values(cls):
        (<MutableStructInfo>cls._fbthrift_mutable_struct_info)._initialize_default_values()

    def __iter__(cls):
        cdef MutableStructInfo mutable_struct_info = cls._fbthrift_mutable_struct_info
        for name in mutable_struct_info.name_to_index.keys():
            yield name, None


cdef class MutableGeneratedError(Error):
    """
    Base class for all generated (mutable) classes corresponding to Thrift
    exception in thrift-python.

    `MutableGeneratedError` is very similar to MutableStruct, with the following
    main differences:
    - It accepts positional arguments `*args`.
    - It includes a `__str__` method that provides output similar to `BaseException`.
    - It features an args property, which provides similar interface to `GeneratedError`
      and `BaseException`.

    Instance variables:
        _fbthrift_data: "mutable struct list" that holds the "isset" flag array and
            values for all fields. See `createMutableStructListWithDefaultValues()`.

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

    def __cinit__(self, *args, **kwargs):
        """
        Args:
            **kwargs: names and values of the Thrift fields to set for this
                 instance. All names must match declared fields of this Thrift
                 Exception (or a `TypeError` will be raised). Values are in
                 "Python value" representation, as opposed to "internal data"
                 representation (see `*TypeInfo` classes).
        """
        cdef MutableStructInfo struct_info = type(self)._fbthrift_mutable_struct_info

        names_iter = iter(struct_info.name_to_index)
        for idx, value in enumerate(args):
            try:
                name = next(names_iter)
            except StopIteration:
                raise TypeError(f"{type(self).__name__}() only takes {idx} arguments")
            if name in kwargs:
                raise TypeError(f"__init__() got multiple values for argument '{name}'")
            kwargs[name] = value

        self._initStructListWithValues(kwargs)
        self._fbthrift_field_cache = [None] * len(struct_info.fields)
        # Append `MutableGeneratedError` instance, see `_fbthrift_has_exception_instance()`
        self._fbthrift_data.append(self)

    def __init__(self, *args, **kwargs):
        pass

    cdef _initStructListWithValues(self, kwargs) except *:
        cdef MutableStructInfo mutable_struct_info = self._fbthrift_mutable_struct_info

        # If no keyword arguments are provided, initialize the Exception with
        # default values.
        if not kwargs:
            self._fbthrift_data = createMutableStructListWithDefaultValues(mutable_struct_info.cpp_obj.get().getStructInfo())
            return

        # Instantiate a list with 'None' values, then assign the provided
        # keyword arguments to the respective fields.
        self._fbthrift_data = createStructListWithNones(mutable_struct_info.cpp_obj.get().getStructInfo())
        for name, value in kwargs.items():
            field_index = mutable_struct_info.name_to_index.get(name)
            if field_index is None:
                raise TypeError(f"{self.__class__.__name__}.__init__() got an unexpected keyword argument '{name}'")

            self._fbthrift_set_field_value(field_index, value)

        # If any fields remain unset, initialize them with their respective
        # default values.
        populateMutableStructListUnsetFieldsWithDefaultValues(
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

        set_mutable_struct_field(
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

    cdef _fbthrift_get_cached_field_value(MutableGeneratedError self, int16_t index):
        cached = self._fbthrift_field_cache[index]
        if cached is not None:
            return cached

        value = self._fbthrift_get_field_value(index)
        self._fbthrift_field_cache[index] = value
        return value

    @property
    def args(self):
        cdef MutableStructInfo struct_info = self._fbthrift_mutable_struct_info
        args = []
        for index, type_info in enumerate(struct_info.type_infos):
            data = self._fbthrift_data[index + 1]
            args.append(None if data is None else type_info.to_python_value(data))

        return args

    def __str__(self) -> str:
        args = self.args
        if len(args) == 1:
            return str(args[0])

        return str(tuple(args)) if len(args) else ""

    def __repr__(self):
        fields = ", ".join(f"{name}={repr(value)}" for name, value in self)
        return f"{type(self).__name__}({fields})"


    def __copy__(MutableGeneratedError self):
        assert isinstance(self._fbthrift_data[-1], MutableGeneratedError)
        return self._fbthrift_from_internal_data(copy.copy(self._fbthrift_data[:-1]))

    def __deepcopy__(MutableGeneratedError self, memo):
        # Use serialization-deserialization to create a truly fresh copy.
        # See the comment for MutableStruct.__deepcopy__ for more explanation.
        cdef bytes buf = mutable_serializer.serialize(self, Protocol.BINARY)
        return mutable_serializer.deserialize(type(self), buf, Protocol.BINARY)

    def __eq__(MutableGeneratedError self, other):
        if other is self:
            return True

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
            std_move(
                c_mutable_serialize(deref(info.cpp_obj), self._fbthrift_data, proto)
            )
        )

    cdef uint32_t _fbthrift_deserialize(self, IOBuf buf, Protocol proto) except? 0:
        cdef MutableStructInfo info = self._fbthrift_mutable_struct_info
        cdef uint32_t length = c_mutable_deserialize(
            deref(info.cpp_obj), buf._this, self._fbthrift_data, proto
        )
        return length

    @classmethod
    def _fbthrift_from_internal_data(cls, data):
        if cls._fbthrift_has_exception_instance(data):
            # An instance of `MutableGeneratedError` has already created for
            # given `._fbthrift_data`, just return the previous instance.
            return data[-1]

        cdef MutableGeneratedError inst = cls.__new__(cls)
        inst._fbthrift_data = data
        # Append `MutableGeneratedError` instance,
        # see `_fbthrift_has_exception_instance()`
        inst._fbthrift_data.append(inst)
        return inst

    @classmethod
    def _fbthrift_has_exception_instance(cls, list fbthrift_data):
        """
        Implementation of exceptions and structs are very similar.
        See `MutableStructInfo._fbthrift_has_struct_instance()`
        """
        return len(fbthrift_data) and isinstance(fbthrift_data[-1], cls)
