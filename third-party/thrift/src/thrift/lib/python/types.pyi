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

import enum
import typing
from typing import Never

from folly.iobuf import IOBuf

from thrift.python.adapter import Adapter
from thrift.python.exceptions import GeneratedError
from thrift.python.serializer import Protocol

usT = typing.TypeVar("usT", bound=StructOrUnion)
sT = typing.TypeVar("sT", bound=Struct)
eT = typing.TypeVar("eT", bound=Enum)
TChunk = typing.TypeVar("TChunk")

class TypeInfo:
    pass

class IntegerTypeInfo:
    pass

class StringTypeInfo:
    pass

class IOBufTypeInfo:
    pass

class FieldQualifier(enum.Enum):
    Unqualified: FieldQualifier = ...
    Optional: FieldQualifier = ...
    Terse: FieldQualifier = ...

class cServiceHealth(Enum):
    cServiceHealth_OK: cServiceHealth = ...
    cServiceHealth_ERROR: cServiceHealth = ...

# Export enum values at module level for direct import
cServiceHealth_OK: cServiceHealth = ...
cServiceHealth_ERROR: cServiceHealth = ...

typeinfo_bool: TypeInfo
typeinfo_byte: IntegerTypeInfo
typeinfo_i16: IntegerTypeInfo
typeinfo_i32: IntegerTypeInfo
typeinfo_i64: IntegerTypeInfo
typeinfo_double: TypeInfo
typeinfo_float: TypeInfo
typeinfo_string: StringTypeInfo
typeinfo_binary: TypeInfo

StructOrError = typing.Union[Struct, GeneratedError]

AnyTypeInfo = typing.Union[
    StructTypeInfo,
    ListTypeInfo,
    SetTypeInfo,
    MapTypeInfo,
    EnumTypeInfo,
    TypeInfo,
    IntegerTypeInfo,
    StringTypeInfo,
]

class FieldInfo:
    def __init__(
        self,
        id: int,
        qualifier: FieldQualifier,
        name: str,
        py_name: str,
        type_info: AnyTypeInfo | typing.Callable[[], AnyTypeInfo],
        default_value: object,
        adapter_info: typing.Optional[tuple[object, Struct]],
        is_primitive: bool,
        idl_type: int = -1,
    ) -> None: ...

class ListTypeInfo:
    def __init__(self, val_info: AnyTypeInfo) -> None: ...
    def get_val_info(self) -> AnyTypeInfo: ...

class SetTypeInfo:
    def __init__(self, val_info: AnyTypeInfo) -> None: ...
    def get_val_info(self) -> AnyTypeInfo: ...

class MapTypeInfo:
    def __init__(self, key_info: AnyTypeInfo, val_info: AnyTypeInfo) -> None: ...
    def get_key_info(self) -> AnyTypeInfo: ...
    def get_val_info(self) -> AnyTypeInfo: ...

class StructTypeInfo:
    def __init__(self, klass: typing.Type[sT]) -> None: ...

class EnumTypeInfo:
    def __init__(self, klass: typing.Type[eT]) -> None: ...

TAdapter = typing.TypeVar("TAdapter", bound=Adapter[object, object])

class AdaptedTypeInfo:
    def __init__(
        self,
        orig_type_info: AnyTypeInfo,
        adapter_class: typing.Type[TAdapter],
        transitive_annotation_factory: typing.Callable[[], typing.Optional[Struct]],
    ) -> None: ...

# Parent class for structs and unions
class StructOrUnion(typing.Hashable):
    def __eq__(self: usT, other: object) -> bool: ...
    def __hash__(self) -> int: ...
    def __lt__(self: usT, other: object) -> bool: ...
    def __le__(self: usT, other: object) -> bool: ...
    @staticmethod
    def __get_thrift_uri__() -> typing.Optional[str]: ...

class Struct(
    StructOrUnion,
    typing.Iterable[typing.Tuple[str, typing.Any]],
    metaclass=StructMeta,
):
    def __copy__(self: sT) -> sT: ...
    def __dir__(self) -> typing.Sequence[str]: ...

class Union(
    StructOrUnion,
    metaclass=UnionMeta,
):
    # these types are made more specific in gencode, so can't be `Final` here
    type: enum.Enum
    value: object
    # these are added to avoid naming collisions with `type` and `value` arms
    fbthrift_current_value: object
    fbthrift_current_field: enum.Enum
    def __bool__(self) -> bool: ...

class StructMeta(type, typing.Iterable[typing.Tuple[str, typing.Any]]): ...
class UnionMeta(type): ...

class EnumMeta(enum.EnumMeta):
    def __call__(
        cls: typing.Type[eT], value: typing.Union[int, typing.SupportsInt]
    ) -> eT: ...
    @property
    def __members__(self: typing.Type[eT]) -> typing.Mapping[str, eT]: ...

    # the following methods are inherited from enum.EnumMeta:
    # def __iter__(cls: typing.Type[eT]) -> typing.Iterator[eT]: ...
    # def __reversed__(cls: typing.Type[eT]) -> typing.Iterator[eT]: ...
    # def __contains__(cls: typing.Type[eT], item: object) -> bool: ...
    # def __getitem__(cls: typing.Type[eT], name: str) -> eT: ...
    # def __len__(cls) -> int: ...

class Enum(typing.SupportsInt, metaclass=EnumMeta):
    name: typing.Final[str]
    value: typing.Final[int]
    @staticmethod
    def __get_thrift_name__() -> str: ...
    @staticmethod
    def __get_thrift_uri__() -> typing.Optional[str]: ...
    def __init__(self, value: typing.Union[eT, int]) -> None: ...
    def __hash__(self) -> int: ...
    # note that __int__ and __index are defined:
    #   - in thrift-python, via `int` base of each generated enum
    #   - in thrift-py3, in each generated enum
    def __int__(self) -> int: ...
    def __index__(self) -> int: ...

class Flag(Enum):
    # pyre-fixme[14]: `__contains__` overrides method defined in `EnumMeta`
    #  inconsistently.
    def __contains__(self: eT, other: eT) -> bool: ...
    # pyre-ignore[15]: This is a pyre bug ignore
    def __or__(self: eT, other: eT) -> eT: ...
    def __and__(self: eT, other: eT) -> eT: ...
    def __xor__(self: eT, other: eT) -> eT: ...
    def __invert__(self: eT) -> eT: ...

class BadEnum(typing.SupportsInt):
    enum: typing.Final[typing.Type[Enum]]
    name: typing.Final[str]
    value: typing.Final[int]
    def __init__(self, the_enum: typing.Type[Enum], value: int) -> None: ...
    def __int__(self) -> int: ...
    def __index__(self) -> int: ...

class Container: ...
class List(Container): ...
class Set(Container): ...
class Map(Container): ...

def fill_specs(*struct_types: StructTypeInfo) -> None: ...
def isset(struct: StructOrError) -> typing.Mapping[str, bool]: ...
def update_nested_field(
    obj: sT, path_to_values: typing.Mapping[str, typing.Any]
) -> sT: ...

_DefaultFieldValue = typing.Union[
    bool,
    int,
    float,
    str,
    bytes,
    IOBuf,
    Enum,
    Struct,
    Union,
    GeneratedError,
    typing.Sequence[Never],
    typing.AbstractSet[Never],
    typing.Mapping[Never, Never],
]

def get_standard_immutable_default_value_for_type(
    type_info: AnyTypeInfo,
) -> _DefaultFieldValue: ...

class _fbthrift_ResponseStreamResult(Struct, typing.Generic[TChunk]):
    success: typing.Final[TChunk]

class ServiceInterface:
    @staticmethod
    def service_name() -> bytes: ...
    def getFunctionTable(
        self,
    ) -> typing.Mapping[bytes, typing.Callable[..., object]]: ...
    async def __aenter__(self) -> typing.Any: ...
    async def __aexit__(
        self,
        exc_type: typing.Optional[typing.Type[BaseException]],
        exc_value: typing.Optional[BaseException],
        traceback: typing.Optional[typing.TracebackType],
    ) -> typing.Optional[bool]: ...
    async def onStartServing(self) -> None: ...
    async def onStopRequested(self) -> None: ...
    async def getStatus(self) -> cServiceHealth: ...
