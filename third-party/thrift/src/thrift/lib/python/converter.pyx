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

from thrift.python.types cimport (
    StructOrUnion,
    Struct,
    Union,
    StructInfo,
    UnionInfo,
    StructTypeInfo,
    ListTypeInfo,
    SetTypeInfo,
    MapTypeInfo,
    EnumTypeInfo,
    BadEnum,
)
cimport thrift.py3.types as py3_types


def to_python_struct(cls, obj):
    if obj is None:
        return None
    if isinstance(obj, cls):
        return obj
    return _to_python_struct(cls, obj)


cdef object _to_python_struct(object cls, object obj):
    if issubclass(cls, Struct):
        return cls(
            **{
                field[2]: _to_python_field(
                    _get_src_field(obj, field), field[3]
                )
                for field in (<StructInfo>cls._fbthrift_struct_info).fields
            }
        )
    elif issubclass(cls, Union):
        for field in (<UnionInfo>cls._fbthrift_struct_info).fields:
            try:
                src = _get_src_union_field(obj, field)
                val = _to_python_field(src, field[3])
                return cls(**{field[2]: val})
            except (AssertionError, AttributeError):
                pass
        return cls()
    else:
        raise TypeError(f"{cls} not a Struct nor an Union")


cdef object _get_src_field(object obj, tuple field):
    if isinstance(obj, py3_types.Struct):
        return getattr(obj, field[2], None)
    for spec in obj.thrift_spec:
        if spec and spec[0] == field[0]:
            return getattr(obj, spec[2], None)
    return None


cdef object _get_src_union_field(object obj, tuple field):
    if isinstance(obj, py3_types.Union):
        return getattr(obj, field[2])
    for spec in obj.thrift_spec:
        if spec and spec[0] == field[0]:
            return getattr(obj, f"get_{spec[2]}")()
    raise AttributeError(f"{obj} doesn't have field with id {field[0]} or name {field[2]}")


cdef object _to_python_field(object obj, object type_info):
    if obj is None:
        return None
    if callable(type_info):
        type_info = type_info()
    if isinstance(type_info, StructTypeInfo):
        return _to_python_struct((<StructTypeInfo>type_info)._class, obj)
    elif isinstance(type_info, ListTypeInfo):
        return [
            _to_python_field(elem, (<ListTypeInfo>type_info).val_info)
            for elem in obj
        ]
    elif isinstance(type_info, SetTypeInfo):
        return {
            _to_python_field(elem, (<SetTypeInfo>type_info).val_info)
            for elem in obj
        }
    elif isinstance(type_info, MapTypeInfo):
        return {
            _to_python_field(k, (<MapTypeInfo>type_info).key_info):
                _to_python_field(v, (<MapTypeInfo>type_info).val_info)
            for k, v in obj.items()
        }
    elif isinstance(type_info, EnumTypeInfo):
        try:
            return (<EnumTypeInfo>type_info)._class(int(obj))
        except ValueError:
            return BadEnum((<EnumTypeInfo>type_info)._class, int(obj))
    else:
        return obj
