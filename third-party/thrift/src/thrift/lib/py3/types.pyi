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

import enum
from typing import (
    Any,
    Iterable,
    Iterator,
    Mapping,
    Optional,
    Protocol,
    Sequence,
    SupportsInt,
    Tuple,
    Type,
    TypeVar,
    Union as tUnion,
)

from thrift.py3.exceptions import GeneratedError

_T = TypeVar("_T")
eT = TypeVar("eT", bound=Enum)

class __NotSet:
    pass

NOTSET = __NotSet()

# pyre-fixme[52]: Protocol `HasIsSet` has private property `__fbthrift_IsSet`.
class HasIsSet(Protocol[_T]):
    __fbthrift_IsSet: Type[_T]

class StructMeta(type, Iterable[Tuple[str, Any]]):
    @staticmethod
    def isset(struct: HasIsSet[_T]) -> _T: ...
    @staticmethod
    def update_nested_field(obj: _T, path_to_values: Mapping[str, Any]) -> _T: ...

class Struct(Iterable[Tuple[str, Any]], metaclass=StructMeta):
    def __copy__(self: _T) -> _T: ...
    def __repr__(self) -> str: ...
    # pyre-ignore[3]: it can be anything
    def __iter__(self) -> Iterator[Tuple[str, Any]]: ...
    def __dir__(self) -> Sequence[str]: ...

class Union(Struct):
    # pyre-ignore[4]: it can be anything
    type: Any
    # pyre-ignore[4]: it can be anything
    value: Any
    def __bool__(self) -> bool: ...
    # pyre-ignore[3]: it can be anything
    def get_type(self) -> Any: ...

class Container:
    def __repr__(self) -> str: ...
    def __hash__(self) -> int: ...

class List(Container): ...
class Set(Container): ...
class Map(Container): ...

class EnumMeta(type):
    def __iter__(self: Type[_T]) -> Iterator[_T]: ...
    def __reversed__(self: Type[_T]) -> Iterator[_T]: ...
    # pyre-ignore[2]: it can be anything
    def __contains__(self: Type[_T], item: Any) -> bool: ...
    def __getitem__(self: Type[_T], name: str) -> _T: ...
    def __len__(self) -> int: ...
    @property
    def __members__(self: Type[_T]) -> Mapping[str, _T]: ...

class Enum(metaclass=EnumMeta):
    name: str
    value: int
    def __getattr__(self: eT, name: str) -> eT: ...
    def __init__(self: eT, value: tUnion[eT, int]) -> None: ...  # __call__ for meta
    def __repr__(self) -> str: ...
    def __str__(self) -> str: ...
    def __hash__(self) -> int: ...
    def __int__(self) -> int: ...
    # pyre-ignore[2]: it can be anything
    def __eq__(self, other: Any) -> bool: ...

class Flag(Enum):
    # pyre-fixme[14]: `__contains__` overrides method defined in `EnumMeta`
    #  inconsistently.
    # pyre-fixme[14]: `__contains__` overrides method defined in `EnumMeta`
    #  inconsistently.
    def __contains__(self: eT, other: eT) -> bool: ...
    def __bool__(self) -> bool: ...
    # pyre-ignore[15]: This is a pyre bug ignore
    def __or__(self: eT, other: eT) -> eT: ...
    def __and__(self: eT, other: eT) -> eT: ...
    def __xor__(self: eT, other: eT) -> eT: ...
    def __invert__(self: eT, other: eT) -> eT: ...

class BadEnum(SupportsInt):
    name: str
    value: int
    enum: Enum
    def __init__(self, the_enum: Type[eT], value: int) -> None: ...
    def __repr__(self) -> str: ...
    def __int__(self) -> int: ...
