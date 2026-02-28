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

from thrift.python.mutable_types cimport (
    MutableStruct,
    MutableUnion,
    MutableStructInfo,
    MutableUnionInfo,
    _ThriftListWrapper,
    _ThriftSetWrapper,
    _ThriftMapWrapper,
)
from thrift.python.mutable_typeinfos cimport (
    MutableStructTypeInfo,
    MutableListTypeInfo,
    MutableSetTypeInfo,
    MutableMapTypeInfo,
)
from thrift.python.types cimport (
    Struct,
    Union,
    FieldInfo,
    EnumTypeInfo,
    BadEnum,
)
from thrift.python.mutable_exceptions import MutableGeneratedError
from thrift.python.exceptions import GeneratedError
import thrift.py3.exceptions as py3_exceptions
cimport thrift.py3.types as py3_types


def to_mutable_python_struct_or_union(mutable_thrift_python_cls, src_struct_or_union):
    """
    Converts Thrift `src_struct_or_union` to the given
    `mutable_thrift_python_cls`.

    Args:
        mutable_thrift_python_cls (class): Conversion target type, i.e. the type of
           the mutable thrift-python struct that should be returned by this function.

        src_struct_or_union (object): Input to convert to a mutable thrift-python
            struct, or None.  If None, this method returns None. If no conversion is
            necessary (i.e., `src_struct_or_union` is already an instance of the target
            `mutable_thrift_python_cls`), this method returns `src_struct_or_union`.
            Otherwise, `src_struct_or_union` must be an instance of a type that can be
            converted to a mutable thrift-python type, i.e. an instance of the
            corresponding thrift-py3, thrift-py-deprecated or immutable thrift-python
            types).

    Returns: Optional[mutable_thrift_python_cls]
    """
    if src_struct_or_union is None:
        return None

    if isinstance(src_struct_or_union, mutable_thrift_python_cls):
        return src_struct_or_union

    return _to_mutable_python_struct_or_union(
        mutable_thrift_python_cls, src_struct_or_union
    )


cdef object _to_mutable_python_struct_or_union(
    object mutable_thrift_python_cls, object src_struct_or_union
):
    """
    Converts the given non-None `src_struct_or_union` to the given
    `mutable_thrift_python_cls`.

    See `to_mutable_python_struct_or_union()`.
    """
    if issubclass(mutable_thrift_python_cls, (MutableStruct, MutableGeneratedError)):
        mutable_struct_info = (
            <MutableStructInfo>mutable_thrift_python_cls._fbthrift_mutable_struct_info
        )
        return mutable_thrift_python_cls(
            **{
                field.py_name: _to_mutable_python_field_value(
                    _get_src_struct_field_value(src_struct_or_union, field),
                    field.type_info,
                    is_nested_container=False,
                )
                for field in mutable_struct_info.fields
            }
        )
    elif issubclass(mutable_thrift_python_cls, MutableUnion):
        mutable_union_info = (
            <MutableUnionInfo>mutable_thrift_python_cls._fbthrift_mutable_struct_info
        )
        for field in mutable_union_info.fields:
            try:
                src_union_value = _get_src_union_field_value(src_struct_or_union, field)
                mutable_python_value = _to_mutable_python_field_value(
                    src_union_value, field.type_info, is_nested_container=False,
                )
                return mutable_thrift_python_cls(
                    **{field.py_name: mutable_python_value}
                )
            except (AssertionError, AttributeError):
                pass
        return mutable_thrift_python_cls()
    else:
        raise TypeError(
            f"{mutable_thrift_python_cls} is not a (Mutable) Struct, Union, or Exception"
        )


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

    # immutable thrift-python
    if isinstance(src_struct, (Struct, GeneratedError)):
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

    # immutable thrift-python
    if isinstance(src_union, Union):
        return getattr(src_union, field.py_name)

    # thrift-py-deprecated
    for spec in src_union.thrift_spec:
        if spec and spec[0] == field.id:
            return getattr(src_union, f"get_{spec[2]}")()

    raise AttributeError(
        f"{src_union} doesn't have field with id {field.id} or name {field.py_name}"
    )

cdef object _to_mutable_python_field_value(
    object src_value, object mutable_type_info, bint is_nested_container
):
    """
    Converts `src_value` (from the src object) to a value suitable for a thrift-python
    field with the given `mutable_type_info`.

    Effectively, the returned value should correspond to the value that a client would
    provide at initialization time, if directly creating a thrift-python instance.

    Args:
        src_value: The source value to be converted.
        mutable_type_info: An object providing type information for the field.
        is_nested_container: A boolean indicating whether the current container is nested
            within another container. Only applicable if `mutable_type_info` is a container
            type info (MutableListTypeInfo, MutableSetTypeInfo, or MutableMapTypeInfo).

    Note:
        - Mutable thrift-python fields should be assigned with to_thrift_list(),
        to_thrift_set(), or to_thrift_map(). These functions use the corresponding
        _Thrift{List,Set,Map}Wrapper structs.
        - For nested containers, only the top container should be wrapped, not
        the nested containers. For example, for list<set<i32>>, assignment should
        be to_thrift_list([{1, 2}, {3, 4}]), wrapping the internal container will
        result in an error (to_thrift_list([to_thrift_set({1, 2})])).
    """
    if src_value is None:
        return None

    if callable(mutable_type_info):
        mutable_type_info = mutable_type_info()

    if isinstance(mutable_type_info, MutableStructTypeInfo):
        return _to_mutable_python_struct_or_union(
            (<MutableStructTypeInfo>mutable_type_info)._mutable_struct_class, src_value
        )
    elif isinstance(mutable_type_info, MutableListTypeInfo):
        list_value = [
            _to_mutable_python_field_value(
                elem,
                (<MutableListTypeInfo>mutable_type_info).val_info,
                is_nested_container=True,
            )
            for elem in src_value
        ]
        return list_value if is_nested_container else _ThriftListWrapper(list_value)
    elif isinstance(mutable_type_info, MutableSetTypeInfo):
        set_value = {
            _to_mutable_python_field_value(
                elem,
                (<MutableSetTypeInfo>mutable_type_info).val_info,
                is_nested_container=True
            )
            for elem in src_value
        }
        return set_value if is_nested_container else _ThriftSetWrapper(set_value)
    elif isinstance(mutable_type_info, MutableMapTypeInfo):
        mutable_map_type_info = <MutableMapTypeInfo>mutable_type_info
        map_value = {
            _to_mutable_python_field_value(
                k, mutable_map_type_info.key_info, is_nested_container=True):
            _to_mutable_python_field_value(
                v, mutable_map_type_info.val_info, is_nested_container=True)
            for k, v in src_value.items()
        }
        return map_value if is_nested_container else _ThriftMapWrapper(map_value)
    elif isinstance(mutable_type_info, EnumTypeInfo):
        try:
            return (<EnumTypeInfo>mutable_type_info)._class(int(src_value))
        except ValueError:
            return BadEnum((<EnumTypeInfo>mutable_type_info)._class, int(src_value))
    else:
        return src_value
