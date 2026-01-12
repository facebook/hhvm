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
from typing import (
    Any,
    Iterable,
    Iterator,
    Mapping,
    Protocol,
    Sequence,
    Tuple,
    Type,
    TypeVar,
)

import thrift.python.types
from thrift.py3.exceptions import GeneratedError

Enum = thrift.python.types.Enum
EnumMeta = thrift.python.types.EnumMeta
Flag = thrift.python.types.Flag
BadEnum = thrift.python.types.BadEnum

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
    def isset_DEPRECATED(struct: HasIsSet[_T]) -> _T: ...
    @staticmethod
    def update_nested_field(obj: _T, path_to_values: Mapping[str, Any]) -> _T: ...

class Struct(Iterable[Tuple[str, Any]], metaclass=StructMeta):
    def __copy__(self: _T) -> _T: ...
    def __repr__(self) -> str: ...
    def __iter__(self) -> Iterator[Tuple[str, Any]]: ...
    def __dir__(self) -> Sequence[str]: ...

class Union(Struct):
    # these are overridden in gencode, so can't be `Final` here
    type: enum.Enum
    value: object
    def __bool__(self) -> bool: ...
    def get_type(self) -> enum.Enum: ...

class Container:
    def __repr__(self) -> str: ...
    def __hash__(self) -> int: ...

class List(Container): ...
class Set(Container): ...
class Map(Container): ...
