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

from collections.abc import Mapping, Sequence, Set
from enum import Enum
from typing import Any, NamedTuple, Optional, overload, Tuple, Type, TypeVar, Union

from thrift.py3.exceptions import Error
from thrift.py3.types import Struct

@overload
def inspect(cls: Union[Struct, Type[Struct], Error, Type[Error]]) -> StructSpec: ...
@overload
def inspect(
    cls: Union[Sequence[Any], Type[Sequence[Any]]],
) -> ListSpec: ...
@overload
def inspect(cls: Union[Set[Any], Type[Set[Any]]]) -> SetSpec: ...
@overload
def inspect(cls: Union[Mapping[Any, Any], Type[Mapping[Any, Any]]]) -> MapSpec: ...
def inspectable(cls: Any) -> bool: ...

class NumberType(Enum):
    NOT_A_NUMBER: NumberType = ...
    BYTE: NumberType = ...
    I08: NumberType = ...
    I16: NumberType = ...
    I32: NumberType = ...
    I64: NumberType = ...
    FLOAT: NumberType = ...
    DOUBLE: NumberType = ...

class StructType(Enum):
    STRUCT: StructType = ...
    UNION: StructType = ...
    EXCEPTION: StructType = ...

class Qualifier(Enum):
    UNQUALIFIED: Qualifier = ...
    REQUIRED: Qualifier = ...
    OPTIONAL: Qualifier = ...

class StructSpec:
    name: str
    fields: Sequence[FieldSpec]
    kind: StructType
    annotations: Mapping[str, str] = {}
    def __init__(
        self,
        name: str,
        fields: Sequence[FieldSpec],
        kind: StructType,
        annotations: Mapping[str, str] = {},
    ) -> None: ...

class FieldSpec:
    id: int
    name: str
    py_name: str
    type: Type[Any]
    kind: NumberType
    qualifier: Qualifier
    default: Any
    annotations: Mapping[str, str] = {}
    def __init__(
        self,
        name: str,
        type: Type[Any],
        kind: NumberType,
        qualifier: Qualifier,
        default: Any,
        annotations: Mapping[str, str] = {},
    ) -> None: ...

class ListSpec:
    value: Type[Any]
    kind: NumberType
    def __init__(self, value: Type[Any], kind: NumberType) -> None: ...

class SetSpec:
    value: Type[Any]
    kind: NumberType
    def __init__(self, value: Type[Any], kind: NumberType) -> None: ...

class MapSpec:
    key: Type[Any]
    key_kind: NumberType
    value: Type[Any]
    value_kind: NumberType
    def __init__(
        self,
        key: Type[Any],
        key_kind: NumberType,
        value: Type[Any],
        value_kind: NumberType,
    ) -> None: ...
