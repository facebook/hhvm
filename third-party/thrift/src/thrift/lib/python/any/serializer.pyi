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

import typing

from apache.thrift.type.type.thrift_types import Type

from folly.iobuf import IOBuf
from thrift.python.serializer import Protocol
from thrift.python.types import Enum

PrimitiveType = typing.Union[bool, int, float, str, bytes, IOBuf, Enum]
Primitive = typing.TypeVar("Primitive", bound=PrimitiveType)

def serialize_primitive(
    obj: Primitive,
    protocol: Protocol = ...,
    thrift_type: typing.Optional[Type] = ...,
) -> IOBuf: ...
def deserialize_primitive(
    cls: typing.Type[Primitive],
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
    thrift_type: typing.Optional[Type] = ...,
) -> Primitive: ...
