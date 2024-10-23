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

from apache.thrift.type.standard.thrift_types import TypeName
from apache.thrift.type.type.thrift_types import Type
from cython.view cimport memoryview
from folly.iobuf cimport IOBuf, from_unique_ptr
from libcpp cimport bool as cbool
from libcpp.utility cimport move as cmove
from thrift.python.exceptions cimport GeneratedError
from thrift.python.types cimport (
    EnumTypeInfo,
    getCTypeInfo,
    List,
    ListTypeInfo,
    Map,
    MapTypeInfo,
    Set,
    SetTypeInfo,
    Struct,
    StructOrUnion,
    StructTypeInfo,
    Union,
)
from thrift.python.types import (
    Enum,
    typeinfo_bool,
    typeinfo_byte,
    typeinfo_i16,
    typeinfo_i32,
    typeinfo_i64,
    typeinfo_double,
    typeinfo_float,
    typeinfo_string,
    typeinfo_binary,
    typeinfo_iobuf
)
from cython.operator cimport dereference as deref

import cython
import typing

Buf = cython.fused_type(IOBuf, bytes, bytearray, memoryview)


cdef _thrift_type_to_type_info(thrift_type, cls):
    if thrift_type.name.type is TypeName.Type.boolType:
        return typeinfo_bool
    if thrift_type.name.type is TypeName.Type.byteType:
        return typeinfo_byte
    if thrift_type.name.type is TypeName.Type.i16Type:
        return typeinfo_i16
    if thrift_type.name.type is TypeName.Type.i32Type:
        return typeinfo_i32
    if thrift_type.name.type is TypeName.Type.i64Type:
        return typeinfo_i64
    if thrift_type.name.type is TypeName.Type.floatType:
        return typeinfo_float
    if thrift_type.name.type is TypeName.Type.doubleType:
        return typeinfo_double
    if thrift_type.name.type is TypeName.Type.stringType:
        return typeinfo_string
    if thrift_type.name.type is TypeName.Type.binaryType:
        if issubclass(cls, IOBuf):
            return typeinfo_iobuf
        return typeinfo_binary
    if thrift_type.name.type is TypeName.Type.enumType:
        return EnumTypeInfo(cls)
    if thrift_type.name.type in (
        TypeName.Type.structType,
        TypeName.Type.unionType,
        TypeName.Type.exceptionType,
    ):
        return StructTypeInfo(cls)
    raise NotImplementedError(f"Unsupported type: {thrift_type}")


cdef _infer_type_info_from_cls(cls):
    if issubclass(cls, Enum):
        return EnumTypeInfo(cls)
    if issubclass(cls, (StructOrUnion, GeneratedError)):
        return StructTypeInfo(cls)
    if issubclass(cls, bool):
        return typeinfo_bool
    if issubclass(cls, int):
        return typeinfo_i64
    if issubclass(cls, float):
        return typeinfo_double
    if issubclass(cls, str):
        return typeinfo_string
    if issubclass(cls, bytes):
        return typeinfo_binary
    if issubclass(cls, IOBuf):
        return typeinfo_iobuf
    raise NotImplementedError(f"Can not infer thrift type from: {cls}")


cdef _serialize_with_type_info(obj, protocol, type_info):
    return folly.iobuf.from_unique_ptr(
        cmove(
            cserialize_type(
                deref(getCTypeInfo(type_info)),
                type_info.to_internal_data(obj),
                protocol,
            )
        )
    )


cdef _deserialize_with_type_info(buf, protocol, type_info):
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    return type_info.to_python_value(
        cdeserialize_type(
            deref(getCTypeInfo(type_info)),
            iobuf._this,
            protocol,
        )
    )


def serialize_primitive(obj, Protocol protocol=Protocol.COMPACT, thrift_type=None):
    if thrift_type is None:
        type_info = _infer_type_info_from_cls(type(obj))
    else:
        type_info = _thrift_type_to_type_info(thrift_type, type(obj))
    return _serialize_with_type_info(obj, protocol, type_info)


def deserialize_primitive(cls, Buf buf, Protocol protocol=Protocol.COMPACT, thrift_type=None):
    if thrift_type is None:
        type_info = _infer_type_info_from_cls(cls)
    else:
        type_info = _thrift_type_to_type_info(thrift_type, cls)
    return _deserialize_with_type_info(buf, protocol, type_info)


def serialize_list(obj, Protocol protocol=Protocol.COMPACT):
    if not isinstance(obj, (list, List)):
        raise ValueError("Not a list")
    if obj:
        type_info = ListTypeInfo(_infer_type_info_from_cls(type(obj[0])))
    else:
        # Empty list, element type info doesn't matter
        type_info = ListTypeInfo(typeinfo_bool)
    return _serialize_with_type_info(obj, protocol, type_info)


def deserialize_list(elem_cls, Buf buf, Protocol protocol=Protocol.COMPACT):
    type_info = ListTypeInfo(_infer_type_info_from_cls(elem_cls))
    return _deserialize_with_type_info(buf, protocol, type_info)


def serialize_set(obj, Protocol protocol=Protocol.COMPACT):
    if not isinstance(obj, (set, Set)):
        raise ValueError("Not a set")
    if obj:
        type_info = SetTypeInfo(_infer_type_info_from_cls(type(next(iter(obj)))))
    else:
        # Empty set, element type info doesn't matter
        type_info = SetTypeInfo(typeinfo_bool)
    return _serialize_with_type_info(obj, protocol, type_info)


def deserialize_set(elem_cls, Buf buf, Protocol protocol=Protocol.COMPACT):
    type_info = SetTypeInfo(_infer_type_info_from_cls(elem_cls))
    return _deserialize_with_type_info(buf, protocol, type_info)


def serialize_map(obj, Protocol protocol=Protocol.COMPACT):
    if not isinstance(obj, (dict, Map)):
        raise ValueError("Not a map")
    if obj:
        k, v = next(iter(obj.items()))
        type_info = MapTypeInfo(
            _infer_type_info_from_cls(type(k)),
            _infer_type_info_from_cls(type(v)),
        )
    else:
        # Empty dict, key/value type info doesn't matter
        type_info = MapTypeInfo(typeinfo_bool, typeinfo_bool)
    return _serialize_with_type_info(obj, protocol, type_info)


def deserialize_map(key_cls, value_cls, Buf buf, Protocol protocol=Protocol.COMPACT):
    type_info = MapTypeInfo(
        _infer_type_info_from_cls(key_cls),
        _infer_type_info_from_cls(value_cls),
    )
    return _deserialize_with_type_info(buf, protocol, type_info)


def serialize_with_type_info(obj, protocol, type_info):
    return _serialize_with_type_info(obj, protocol, type_info)
