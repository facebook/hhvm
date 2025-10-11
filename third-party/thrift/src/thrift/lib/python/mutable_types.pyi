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

from thrift.python.exceptions import Error
from thrift.python.types import Struct

# Base class for mutable structs and mutable unions
class MutableStructOrUnion:
    def fbthrift_reset(self) -> None: ...
    def __eq__(self, other: object) -> bool: ...
    def __lt__(self, other: object) -> bool: ...
    def __le__(self, other: object) -> bool: ...

class MutableStructMeta(type): ...
class MutableUnionMeta(type): ...
class MutableGeneratedError(Error): ...

class MutableStruct(
    MutableStructOrUnion,
    typing.Iterable[typing.Tuple[str, typing.Any]],
    metaclass=MutableStructMeta,
):
    def fbthrift_copy_from(self, other: object) -> None: ...
    def _to_python(self) -> Struct: ...
    def __replace__(
        self, *args: typing.Any, **changes: typing.Any
    ) -> MutableStruct: ...

class MutableUnion(MutableStructOrUnion, metaclass=MutableUnionMeta): ...

MutableStructOrError = typing.Union[MutableStruct, MutableGeneratedError]

class _ThriftListWrapper: ...
class _ThriftSetWrapper: ...
class _ThriftMapWrapper: ...

def _isset(struct: MutableStructOrError) -> typing.Mapping[str, bool]: ...

# TODO(alperyoney): Make param types more specific
def to_thrift_list(list_data: object) -> _ThriftListWrapper: ...
def to_thrift_set(set_data: object) -> _ThriftSetWrapper: ...
def to_thrift_map(map_data: object) -> _ThriftMapWrapper: ...

TChunk = typing.TypeVar("TChunk")

class _fbthrift_MutableResponseStreamResult(MutableStruct, typing.Generic[TChunk]):
    success: typing.Final[TChunk]
