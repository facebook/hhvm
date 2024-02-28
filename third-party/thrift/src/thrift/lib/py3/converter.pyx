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

from enum import Enum
from typing import Any, Type

from thrift.py3.reflection import inspect

from libcpp cimport bool
from thrift.py3.reflection cimport FieldSpec, MapSpec, Qualifier, StructType
from thrift.py3.types cimport BadEnum, CompiledEnum, Container, Struct
from thrift.python.types cimport Struct as PythonStruct, Union as PythonUnion

def to_py3_struct(cls, obj):
    if obj is None:
        return None
    if isinstance(obj, cls):
        return obj
    return _to_py3_struct(cls, obj)


cdef object _to_py3_struct(object cls, object obj):
    struct_spec = inspect(cls)
    if struct_spec.kind == StructType.STRUCT:
        return cls(
            **{
                field_spec.py_name: _to_py3_field(
                    field_spec.type, _get_src_field(obj, field_spec)
                )
                for field_spec in struct_spec.fields
                if not _should_ignore_field(obj, field_spec)
            }
        )
    elif struct_spec.kind == StructType.UNION:
        for field_spec in struct_spec.fields:
            try:
                value = _get_src_union_field(obj, field_spec)
                field = _to_py3_field(field_spec.type, value)
                return cls(**{field_spec.py_name: field})
            except (AssertionError, AttributeError):
                pass
        return cls()
    else:
        raise NotImplementedError("Can not convert {}".format(struct_spec.kind))


cdef object _get_src_field(object obj, FieldSpec field_spec):
    if isinstance(obj, PythonStruct):
        return getattr(obj, field_spec.py_name, None)
    return getattr(obj, field_spec.name, None)


cdef object _get_src_union_field(object obj, FieldSpec field_spec):
    if isinstance(obj, PythonUnion):
        return getattr(obj, field_spec.py_name)
    return getattr(obj, "get_" + field_spec.name)()


cdef bool _should_ignore_field(object obj, FieldSpec field_spec) noexcept:
    dft = field_spec.default
    if not (field_spec.qualifier == Qualifier.OPTIONAL and dft is not None):
        return False

    typ = field_spec.type
    val = getattr(obj, field_spec.name)
    casted = val if not issubclass(typ, CompiledEnum) else typ(val)
    return casted == dft


cdef object _to_py3_field(object cls, object obj):
    if obj is None:
        return None
    if issubclass(cls, Struct):
        return _to_py3_struct(cls, obj)
    elif issubclass(cls, Container):
        container_spec = inspect(cls)
        if isinstance(container_spec, MapSpec):
            return {
                _to_py3_field(container_spec.key, k): _to_py3_field(
                    container_spec.value, v
                )
                for k, v in obj.items()
            }
        else:
            return [_to_py3_field(container_spec.value, elem) for elem in obj]
    elif issubclass(cls, CompiledEnum):
        int_val = int(obj)
        try:
            return cls(int_val)
        except ValueError:
            return BadEnum(cls, int_val)
    else:
        return obj
