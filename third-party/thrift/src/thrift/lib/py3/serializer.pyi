#!/usr/bin/env python3
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

from enum import Enum
from typing import Tuple, Type, TypeVar, Union

from folly.iobuf import IOBuf
from thrift.py3.common import Protocol as Protocol
from thrift.py3.exceptions import GeneratedError
from thrift.py3.types import Struct
from thrift.python.exceptions import GeneratedError as PythonGeneratedError
from thrift.python.types import StructOrUnion as PythonStruct

sT = TypeVar(
    "sT", bound=Union[Struct, GeneratedError, PythonGeneratedError, PythonStruct]
)

class Transform(Enum):
    NONE: Transform = ...
    ZLIB_TRANSFORM: Transform = ...
    ZSTD_TRANSFORM: Transform = ...

def serialize(tstruct: sT, protocol: Protocol = ...) -> bytes: ...
def serialize_iobuf(tstruct: sT, protocol: Protocol = ...) -> IOBuf: ...
def deserialize(
    structKlass: Type[sT],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
) -> sT: ...
def deserialize_with_length(
    structKlass: Type[sT],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = ...,
) -> Tuple[sT, int]: ...
def serialize_with_header(
    tstruct: sT, protocol: Protocol = ..., transform: Transform = ...
) -> bytes: ...
def serialize_with_header_iobuf(
    tstruct: sT, protocol: Protocol = ..., transform: Transform = ...
) -> IOBuf: ...
def deserialize_from_header(
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    structKlass: Type[sT],
    buf: Union[bytes, bytearray, IOBuf, memoryview],
) -> sT: ...
