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
    StructTypeInfo, ListTypeInfo, SetTypeInfo, MapTypeInfo, EnumTypeInfo
]

class ListTypeInfo:
    def __init__(self, val_info: AnyTypeInfo) -> None: ...

class SetTypeInfo:
    def __init__(self, val_info: AnyTypeInfo) -> None: ...

class MapTypeInfo:
    def __init__(self, key_info: AnyTypeInfo, val_info: AnyTypeInfo) -> None: ...

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
        transitive_annotation: typing.Callable[[], typing.Optional[Struct]],
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
    # pyre-ignore[4]: it can be anything
    type: typing.Any
    # pyre-ignore[4]: it can be anything
    value: typing.Any
    def __bool__(self) -> bool: ...

class StructMeta(type, typing.Iterable[typing.Tuple[str, typing.Any]]): ...
class UnionMeta(type): ...

class EnumMeta(type):
    def __iter__(cls: typing.Type[eT]) -> typing.Iterator[eT]: ...
    def __reversed__(cls: typing.Type[eT]) -> typing.Iterator[eT]: ...
    def __getitem__(cls: typing.Type[eT], name: str) -> eT: ...
    def __len__(cls) -> int: ...
    @property
    def __members__(self: typing.Type[eT]) -> typing.Mapping[str, eT]: ...

class Enum(metaclass=EnumMeta):
    name: typing.Final[str]
    value: typing.Final[int]
    @staticmethod
    def __get_thrift_name__() -> str: ...
    @staticmethod
    def __get_thrift_uri__() -> typing.Optional[str]: ...
    def __init__(self, val: int) -> None: ...

class Flag(Enum):
    def __contains__(self: eT, other: eT) -> bool: ...
    # pyre-ignore[15]: This is a pyre bug ignore
    def __or__(self: eT, other: eT) -> eT: ...
    def __and__(self: eT, other: eT) -> eT: ...
    def __xor__(self: eT, other: eT) -> eT: ...
    def __invert__(self: eT) -> eT: ...

class BadEnum(typing.SupportsInt):
    enum: typing.Type[Enum]
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

class _fbthrift_ResponseStreamResult(Struct, typing.Generic[TChunk]):
    success: typing.Final[TChunk]

class ServiceInterface:
    @staticmethod
    def service_name() -> bytes: ...
    def getFunctionTable(self) -> typing.Mapping[bytes, typing.Callable[..., ...]]: ...
    # pyre-ignore[3]: it can return anything
    async def __aenter__(self) -> typing.Any: ...
    async def __aexit__(
        self,
        exc_type: typing.Optional[typing.Type[BaseException]],
        exc_value: typing.Optional[BaseException],
        traceback: typing.Optional[typing.TracebackType],
    ) -> typing.Optional[bool]: ...
    async def onStartServing(self) -> None: ...
    async def onStopRequested(self) -> None: ...
