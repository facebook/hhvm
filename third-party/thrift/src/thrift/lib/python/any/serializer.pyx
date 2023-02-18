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
from libcpp.utility cimport move as cmove
from thrift.python.types cimport Struct, StructOrUnion, Union, doubleTypeInfo, floatTypeInfo, i16TypeInfo, i32TypeInfo, i64TypeInfo

import cython
import typing

Buf = cython.fused_type(IOBuf, bytes, bytearray, memoryview)
Primitive = cython.fused_type(int, float)

cdef cTypeInfo _thrift_type_to_type_info(thrift_type):
    if thrift_type.name.type is TypeName.Type.i16Type:
        return i16TypeInfo
    if thrift_type.name.type is TypeName.Type.i32Type:
        return i32TypeInfo
    if thrift_type.name.type is TypeName.Type.i64Type:
        return i64TypeInfo
    if thrift_type.name.type is TypeName.Type.floatType:
        return floatTypeInfo
    if thrift_type.name.type is TypeName.Type.doubleType:
        return doubleTypeInfo
    raise NotImplementedError(f"Unsupported type: {thrift_type}")


cdef cTypeInfo _infer_type_info_from_cls(cls):
    if issubclass(cls, int):
        return i64TypeInfo
    if issubclass(cls, float):
        return doubleTypeInfo
    raise NotImplementedError(f"Can not infer thrift type from: {cls}")


def serialize_primitive(Primitive obj, Protocol protocol=Protocol.COMPACT, thrift_type=None):
    cdef cTypeInfo type_info
    if thrift_type is None:
        type_info = _infer_type_info_from_cls(type(obj))
    else:
        type_info = _thrift_type_to_type_info(thrift_type)
    return folly.iobuf.from_unique_ptr(
        cmove(cserialize_type(type_info, obj, protocol))
    )


def deserialize_primitive(cls, Buf buf, Protocol protocol=Protocol.COMPACT, thrift_type=None):
    cdef cTypeInfo type_info
    if thrift_type is None:
        type_info = _infer_type_info_from_cls(cls)
    else:
        type_info = _thrift_type_to_type_info(thrift_type)
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    return cdeserialize_type(type_info, iobuf._this, protocol)
