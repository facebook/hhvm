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

from thrift.py3.types import Struct as Py3Struct
from thrift.python.mutable_types import (
    MutableStructOrUnion as PythonMutableStructOrUnion,
)
from thrift.python.types import StructOrUnion as PythonImmutableStructOrUnion

# thrift-py-deprecated struct doesn't have a base class,
# thus this hacky way to do type checking
class PyDeprecatedStruct(typing.Protocol):
    thrift_spec: typing.Any

T = typing.TypeVar("T", bound=PythonMutableStructOrUnion)
TObj = (
    PythonImmutableStructOrUnion
    | PythonMutableStructOrUnion
    | Py3Struct
    | PyDeprecatedStruct
)

@typing.overload
def to_mutable_python_struct_or_union(
    mutable_thrift_python_cls: typing.Type[T], src_struct_or_union: TObj
) -> T: ...
@typing.overload
def to_mutable_python_struct_or_union(
    mutable_thrift_python_cls: typing.Type[T],
    src_struct_or_union: typing.Optional[TObj],
) -> typing.Optional[T]: ...
