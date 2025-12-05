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
from typing import (
    Iterator,
    Mapping,
    Optional,
    overload,
    Protocol,
    Sequence,
    Tuple,
    Type,
    Union,
)

from apache.thrift.metadata.thrift_types import (
    ThriftBidiType,
    ThriftConstStruct,
    ThriftConstValue,
    ThriftEnum,
    ThriftEnumType,
    ThriftException,
    ThriftField,
    ThriftFunction,
    ThriftListType,
    ThriftMapType,
    ThriftMetadata,
    ThriftPrimitiveType,
    ThriftService,
    ThriftSetType,
    ThriftSinkType,
    ThriftStreamType,
    ThriftStruct,
    ThriftStructType,
    ThriftType,
    ThriftTypedefType,
    ThriftUnionType,
)
from thrift.python.client import Client
from thrift.python.exceptions import GeneratedError

# pyre-fixme[21]: Could not find module `thrift.python.server`.
from thrift.python.server import ServiceInterface
from thrift.python.types import Enum as ThriftEnumClass, StructOrUnion

class ThriftKind(Enum):
    PRIMITIVE: ThriftKind = ...
    LIST: ThriftKind = ...
    SET: ThriftKind = ...
    MAP: ThriftKind = ...
    ENUM: ThriftKind = ...
    STRUCT: ThriftKind = ...
    UNION: ThriftKind = ...
    TYPEDEF: ThriftKind = ...
    STREAM: ThriftKind = ...
    SINK: ThriftKind = ...
    BIDI: ThriftKind = ...

class ThriftConstKind(Enum):
    CV_BOOL: ThriftConstKind = ...
    CV_INT: ThriftConstKind = ...
    CV_FLOAT: ThriftConstKind = ...
    CV_STRING: ThriftConstKind = ...
    CV_MAP: ThriftConstKind = ...
    CV_LIST: ThriftConstKind = ...
    CV_STRUCT: ThriftConstKind = ...

class Metadata(Protocol):
    def getThriftModuleMetadata(self) -> ThriftMetadata: ...

class ThriftTypeProxy:
    thriftType: Union[
        ThriftPrimitiveType,
        ThriftSetType,
        ThriftListType,
        ThriftMapType,
        ThriftTypedefType,
        ThriftEnum,
        ThriftStruct,
        ThriftSinkType,
        ThriftBidiType,
        ThriftStreamType,
    ]
    thriftMeta: ThriftMetadata
    kind: ThriftKind
    def as_primitive(self) -> ThriftPrimitiveType: ...
    def as_struct(self) -> ThriftStructProxy: ...
    def as_union(self) -> ThriftStructProxy: ...
    def as_enum(self) -> ThriftEnum: ...
    def as_list(self) -> ThriftListProxy: ...
    def as_set(self) -> ThriftSetProxy: ...
    def as_map(self) -> ThriftMapProxy: ...
    def as_typedef(self) -> ThriftTypedefProxy: ...
    def as_stream(self) -> ThriftStreamProxy: ...
    def as_sink(self) -> ThriftSinkProxy: ...
    def as_bidi(self) -> ThriftBidiProxy: ...

class ThriftSetProxy(ThriftTypeProxy):
    thriftType: ThriftSetType
    valueType: ThriftTypeProxy

class ThriftListProxy(ThriftTypeProxy):
    thriftType: ThriftListType
    valueType: ThriftTypeProxy

class ThriftMapProxy(ThriftTypeProxy):
    thriftType: ThriftMapType
    valueType: ThriftTypeProxy
    keyType: ThriftTypeProxy

class ThriftTypedefProxy(ThriftTypeProxy):
    thriftType: ThriftTypedefType
    name: str
    underlyingType: ThriftTypeProxy

class ThriftSinkProxy(ThriftTypeProxy):
    thriftType: ThriftSinkType
    elemType: ThriftTypeProxy
    initialResponseType: Optional[ThriftTypeProxy]
    finalResponseType: Optional[ThriftTypeProxy]

class ThriftStreamProxy(ThriftTypeProxy):
    thriftType: ThriftStreamType
    elemType: ThriftTypeProxy
    initialResponseType: Optional[ThriftTypeProxy]

class ThriftBidiProxy(ThriftTypeProxy):
    streamElemType: ThriftTypeProxy
    sinkElemType: ThriftTypeProxy
    initialResponseType: ThriftTypeProxy

class ThriftFieldProxy(Protocol):
    id: int
    name: str
    pyname: str
    is_optional: bool
    type: ThriftTypeProxy
    thriftType: ThriftField
    thriftMeta: ThriftMetadata
    structuredAnnotations: Sequence[ThriftConstStructProxy]

class ThriftStructProxy(ThriftTypeProxy):
    thriftType: ThriftStruct
    thriftMeta: ThriftMetadata
    name: str
    fields: Iterator[ThriftFieldProxy]
    is_union: bool
    structuredAnnotations: Sequence[ThriftConstStructProxy]

ConstType = Union[  # type: ignore
    bool,
    int,
    float,
    str,
    Sequence[ThriftConstValueProxy],
    Mapping[ConstType, ThriftConstValueProxy],
    ThriftConstStructProxy,
]

class ThriftConstValueProxy:
    thriftType: ThriftConstValue
    type: ConstType
    kind: ThriftConstKind
    def as_bool(self) -> bool: ...
    def as_int(self) -> int: ...
    def as_float(self) -> float: ...
    def as_string(self) -> str: ...
    def as_list(self) -> Sequence[ThriftConstValueProxy]: ...
    def as_map(self) -> Mapping[ConstType, ThriftConstValueProxy]: ...
    def as_struct(self) -> ThriftConstStructProxy: ...

class ThriftConstStructProxy:
    thriftType: ThriftConstStruct
    fields: Mapping[str, ThriftConstValueProxy]
    name: str
    kind: ThriftKind
    def __init__(self, struct: ThriftConstStruct) -> None: ...

class ThriftExceptionProxy(Protocol):
    name: str
    fields: Iterator[ThriftFieldProxy]
    thriftType: ThriftException
    thriftMeta: ThriftMetadata
    structuredAnnotations: Sequence[ThriftConstStructProxy]

class ThriftFunctionProxy(Protocol):
    name: str
    return_type: ThriftTypeProxy
    arguments: Iterator[ThriftFieldProxy]
    exceptions: Iterator[ThriftFieldProxy]
    is_oneway: bool
    thriftType: ThriftFunction
    thriftMeta: ThriftMetadata
    structuredAnnotations: Sequence[ThriftConstStructProxy]

class ThriftServiceProxy(Protocol):
    name: str
    functions: Iterator[ThriftFunctionProxy]
    parent: Optional[ThriftServiceProxy]
    thriftType: ThriftService
    thriftMeta: ThriftMetadata
    structuredAnnotations: Sequence[ThriftConstStructProxy]

@overload
def gen_metadata(cls: Metadata) -> ThriftMetadata: ...
@overload
def gen_metadata(
    cls: Union[StructOrUnion, Type[StructOrUnion]],
) -> ThriftStructProxy: ...
@overload
def gen_metadata(
    cls: Union[GeneratedError, Type[GeneratedError]],
) -> ThriftExceptionProxy: ...
@overload
def gen_metadata(
    # pyre-fixme[11]: Annotation `ServiceInterface` is not defined as a type.
    cls: Union[ServiceInterface, Type[ServiceInterface], Client, Type[Client]],
) -> ThriftServiceProxy: ...
@overload
# pyre-fixme[43]: Signature of overloaded function `gen_metadata` will never be matched.
def gen_metadata(cls: Union[ThriftEnumClass, Type[ThriftEnumClass]]) -> ThriftEnum: ...
