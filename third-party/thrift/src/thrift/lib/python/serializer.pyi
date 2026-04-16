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
from enum import Enum

from folly.iobuf import IOBuf
from thrift.python.exceptions import GeneratedError
from thrift.python.protocol import Protocol as _Protocol
from thrift.python.types import StructOrUnion

sT = typing.TypeVar("sT", bound=typing.Union[StructOrUnion, GeneratedError])

Protocol = _Protocol

class JsonWriterOptions:
    def __init__(
        self,
        *,
        list_trailing_comma: bool = False,
        object_trailing_comma: bool = False,
        unquote_object_name: bool = False,
        allow_nan_inf: bool = False,
        indent_width: int = 0,
    ) -> None: ...

JSON5_MODE: Json5ProtocolWriterOptions

class Json5ProtocolWriterOptions:
    writer: JsonWriterOptions
    def __init__(
        self,
        *,
        writer: JsonWriterOptions | None = None,
    ) -> None: ...

def serialize_iobuf(
    strct: sT,
    protocol: Protocol = Protocol.COMPACT,
    options: typing.Optional[Json5ProtocolWriterOptions] = None,
) -> IOBuf: ...
def serialize(
    struct: sT,
    protocol: Protocol = Protocol.COMPACT,
    options: typing.Optional[Json5ProtocolWriterOptions] = None,
) -> bytes: ...
def deserialize_with_length(
    klass: typing.Type[sT],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = Protocol.COMPACT,
    *,
    fully_populate_cache: bool = False,
) -> typing.Tuple[sT, int]: ...
def deserialize(
    klass: typing.Type[sT],
    # pyre-fixme[24]: Generic type `memoryview` expects 1 type parameter.
    buf: typing.Union[bytes, bytearray, IOBuf, memoryview],
    protocol: Protocol = Protocol.COMPACT,
    *,
    fully_populate_cache: bool = False,
) -> sT: ...
