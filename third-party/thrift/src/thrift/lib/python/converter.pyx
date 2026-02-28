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

from thrift.python.mutable_types import (
    MutableStruct,
    MutableUnion,
)
from thrift.python.mutable_exceptions import MutableGeneratedError
from thrift.python.types cimport (
    FieldInfo,
    StructInfo,
    UnionInfo,
    StructTypeInfo,
    ListTypeInfo,
    SetTypeInfo,
    MapTypeInfo,
    EnumTypeInfo,
)
from thrift.python.exceptions import GeneratedError
from thrift.python.types import (
    Struct,
    Union,
    BadEnum,
)
import thrift.py3.exceptions as py3_exceptions
import thrift.py3.types as py3_types


def to_python_struct(immutable_thrift_python_cls, src_struct_or_union):
    """
    Converts Thrift instance `src_struct_or_union` to the given
    `immutable_thrift_python_cls`.

    Args:
        immutable_thrift_python_cls (class): Conversion target type, i.e. the type of
           the immutable thrift-python struct that should be returned by this function.

        src_struct_or_union (object): Input to convert to an immutable thrift-python
            instance, or None.  If None, this method returns None. If no conversion is
            necessary (i.e., `src_struct_or_union` is already an instance of the target
            `immutable_thrift_python_cls`), this method returns `src_struct_or_union`.
            Otherwise, `src_struct_or_union` must be an instance of a type that can be
            converted to an immutable thrift-python type, i.e. an instance of the
            corresponding thrift-py3, thrift-py-deprecated or mutable thrift-python
            types.

    Returns: Optional[immutable_thrift_python_cls]
    """
    if src_struct_or_union is None:
        return None
    if isinstance(src_struct_or_union, immutable_thrift_python_cls):
        return src_struct_or_union
    return _to_immutable_python_struct_or_union(
        immutable_thrift_python_cls, src_struct_or_union
    )


cdef object _to_immutable_python_struct_or_union(
    object immutable_thrift_python_cls, object src_struct_or_union
):
    """
    Converts the given non-None `src_struct_or_union` to the given
    `immutable_thrift_python_cls`.

    See `to_python_struct()`.
    """
    if issubclass(immutable_thrift_python_cls, (Struct, GeneratedError)):
        return immutable_thrift_python_cls(
            **{
                field.py_name: _to_immutable_python_field_value(
                    _get_src_struct_field_value(src_struct_or_union, field),
                    field.type_info
                )
                for field in (
                    <StructInfo>immutable_thrift_python_cls._fbthrift_struct_info
                ).fields
            }
        )
    elif issubclass(immutable_thrift_python_cls, Union):
        for field in (
            <UnionInfo>immutable_thrift_python_cls._fbthrift_struct_info
        ).fields:
            try:
                src_union_value = _get_src_union_field_value(src_struct_or_union, field)
                immutable_python_value = _to_immutable_python_field_value(
                    src_union_value, field.type_info
                )
                return immutable_thrift_python_cls(
                    **{field.py_name: immutable_python_value}
                )
            except (AssertionError, AttributeError):
                pass
        return immutable_thrift_python_cls()
    else:
        raise TypeError(f"{immutable_thrift_python_cls} not a Struct nor an Union")


cdef object _get_src_struct_field_value(object src_struct, FieldInfo field):
    """
    Returns the value of the given field in `src_struct`, or None if n/a.

    Args:
        src_struct (object): Input Thrift struct to convert, in one of the supported
            variants (thrift-py3, thrift-py-deprecated, etc.)

    """
    # thrift-py3
    if isinstance(src_struct, (py3_types.Struct, py3_exceptions.GeneratedError)):
        return getattr(src_struct, field.py_name, None)

    # mutable thrift-python
    if isinstance(src_struct, (MutableStruct, MutableGeneratedError)):
        return getattr(src_struct, field.py_name, None)

    # py-deprecated types do not inherit from a specific class, so we consider them as
    # a fallback, and assume that the expected attributes (i.e., thrift_spec and the
    # corresponding field names) are present.
    for spec in src_struct.thrift_spec:
        if spec and spec[0] == field.id:
            field_name = spec[2]
            return getattr(src_struct, field_name, None)

    return None


cdef object _get_src_union_field_value(object src_union, FieldInfo field):
    """
    Returns the value of `field` in the given `src_union`, if it is the current field.

    Args:
        src_union (object): A Thrift union in one of the supported input variants (i.e.,
            thrift-py3, thrift-py-deprecated, etc.).

    Raises: Exception if `field` does not exist or is not currently set in `src_union`.
    """
    # thrift-py3
    if isinstance(src_union, py3_types.Union):
        return getattr(src_union, field.py_name)

    # mutable thrift-python
    if isinstance(src_union, MutableUnion):
        return getattr(src_union, field.py_name)

    # thrift-py-deprecated
    for spec in src_union.thrift_spec:
        if spec and spec[0] == field.id:
            return getattr(src_union, f"get_{spec[2]}")()

    raise AttributeError(
        f"{src_union} doesn't have field with id {field.id} or name {field.py_name}"
    )

cdef object _to_immutable_python_field_value(
    object src_value, object immutable_type_info
):
    """
    Converts `src_value` (from the src object) to a value suitable for a thrift-python
    field with the given `immutable_type_info`.

    Effectively, the returned value should correspond to the value that a client would
    provide at initialization time, if directly creating a thrift-python instance.
    """
    if src_value is None:
        return None

    if callable(immutable_type_info):
        immutable_type_info = immutable_type_info()

    if isinstance(immutable_type_info, StructTypeInfo):
        return _to_immutable_python_struct_or_union(
            (<StructTypeInfo>immutable_type_info)._class, src_value
        )
    elif isinstance(immutable_type_info, ListTypeInfo):
        return [
            _to_immutable_python_field_value(
                elem, (<ListTypeInfo>immutable_type_info).val_info
            )
            for elem in src_value
        ]
    elif isinstance(immutable_type_info, SetTypeInfo):
        return {
            _to_immutable_python_field_value(
                elem, (<SetTypeInfo>immutable_type_info).val_info
            )
            for elem in src_value
        }
    elif isinstance(immutable_type_info, MapTypeInfo):
        return {
            _to_immutable_python_field_value(
                k, (<MapTypeInfo>immutable_type_info).key_info
            ): _to_immutable_python_field_value(
                v, (<MapTypeInfo>immutable_type_info).val_info
            )
            for k, v in src_value.items()
        }
    elif isinstance(immutable_type_info, EnumTypeInfo):
        try:
            return (<EnumTypeInfo>immutable_type_info)._class(int(src_value))
        except ValueError:
            return BadEnum((<EnumTypeInfo>immutable_type_info)._class, int(src_value))
    else:
        return src_value
