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

# pyre-strict

import typing

from apache.thrift.type.type.thrift_types import Type
from folly.iobuf import IOBuf
from thrift.lib.python.any.typestub import (
    PrimitiveType,
    SerializableType,
    SerializableTypeOrContainers,
    StructOrUnionOrException,
    TKey,
    TPrimitive,
    TSerializable,
    TValue,
)
from thrift.python.exceptions import GeneratedError
from thrift.python.serializer import Protocol
from thrift.python.types import AnyTypeInfo, Enum, StructOrUnion

def serialize_primitive(
    obj: TPrimitive,
    protocol: Protocol = ...,
    thrift_type: typing.Optional[Type] = ...,
) -> IOBuf: ...
def deserialize_primitive(
    cls: typing.Type[TPrimitive],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
    thrift_type: typing.Optional[Type] = ...,
) -> TPrimitive: ...
def serialize_list(
    obj: typing.Sequence[SerializableType],
    protocol: Protocol = ...,
) -> IOBuf: ...
def deserialize_list(
    elem_cls: typing.Type[TSerializable],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
) -> typing.Sequence[TSerializable]: ...
def serialize_set(
    obj: typing.AbstractSet[SerializableType],
    protocol: Protocol = ...,
) -> IOBuf: ...
def deserialize_set(
    elem_cls: typing.Type[TSerializable],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
) -> typing.AbstractSet[TSerializable]: ...
def serialize_map(
    obj: typing.Mapping[TKey, TValue],
    protocol: Protocol = ...,
) -> IOBuf: ...
def deserialize_map(
    key_cls: typing.Type[TKey],
    value_cls: typing.Type[TValue],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
) -> typing.Mapping[TKey, TValue]: ...
def serialize_with_type_info(
    obj: SerializableTypeOrContainers,
    protocol: Protocol,
    type_info: AnyTypeInfo,
) -> IOBuf: ...
