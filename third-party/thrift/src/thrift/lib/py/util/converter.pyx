#! /usr/bin/python3
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


from thrift.py3.types cimport Struct as py3_Struct, Union as py3_Union
import thrift.py3.reflection as py3_reflection
cimport thrift.python.types as python_types
cimport thrift.python.mutable_types as python_mutable_types
from thrift.Thrift import TType
from thrift.python.types import Enum, BadEnum
from thrift.util import parse_struct_spec


def to_py_struct(cls, obj):
    if obj is None:
        return None
    if isinstance(obj, cls):
        return obj
    return _to_py_struct(cls, obj)


cdef object _to_py_struct(object cls, object obj):
    field_id_to_name = {}
    if isinstance(obj, py3_Struct):
        try:
            field_id_to_name = {
                field_spec.id: field_spec.py_name
                for field_spec in py3_reflection.inspect(obj).fields
            }
        except TypeError:
            pass
    elif isinstance(obj, python_types.Struct):
        field_id_to_name = {
            spec.id: spec.py_name
            for spec in (<python_types.StructInfo>obj._fbthrift_struct_info).fields
        }
    elif isinstance(obj, python_mutable_types.MutableStruct):
        field_id_to_name = {
            spec.id: spec.py_name
            for spec in (<python_mutable_types.MutableStructInfo>obj._fbthrift_mutable_struct_info).fields
        }
    elif isinstance(obj, python_types.Union):
        field_id_to_name = {
            spec.id: spec.py_name
            for spec in (<python_types.UnionInfo>obj._fbthrift_struct_info).fields
        }
    elif isinstance(obj, python_mutable_types.MutableUnion):
        field_id_to_name = {
            spec.id: spec.py_name
            for spec in (<python_mutable_types.MutableUnionInfo>obj._fbthrift_mutable_struct_info).fields
        }

    if cls.isUnion():
        if not isinstance(obj, (py3_Union, python_types.Union, python_mutable_types.MutableUnion)):
            raise TypeError("Source object is not an Union")

        return cls(
            **{
                field.name: _to_py_field(
                    field.type,
                    field.type_args,
                    getattr(obj, field_id_to_name.get(field.id, field.name), None),
                )
                for field in parse_struct_spec(cls)
                if field_id_to_name.get(field.id, field.name) == obj.get_type().name
            }
        )
    else:
        return cls(
            **{
                field.name: _to_py_field(
                    field.type,
                    field.type_args,
                    getattr(obj, field_id_to_name.get(field.id, field.name), None),
                )
                for field in parse_struct_spec(cls)
            }
        )


cdef object _to_py_field(object field_type, object type_args, object obj):
    if obj is None:
        return None
    if field_type == TType.STRUCT:
        return _to_py_struct(type_args[0], obj)
    if field_type == TType.LIST:
        return [_to_py_field(type_args[0], type_args[1], elem) for elem in obj]
    if field_type == TType.SET:
        return {_to_py_field(type_args[0], type_args[1], elem) for elem in obj}
    if field_type == TType.MAP:
        return {
            _to_py_field(type_args[0], type_args[1], k): _to_py_field(
                type_args[2], type_args[3], v
            )
            for k, v in obj.items()
        }
    if isinstance(obj, (Enum, BadEnum)):
        return obj.value
    return obj
