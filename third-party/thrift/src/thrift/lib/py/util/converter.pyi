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

from thrift.py3.types import Struct as Py3Struct
from thrift.python.mutable_types import MutableStructOrUnion as PythonMutableStruct
from thrift.python.types import Enum as PythonEnum, StructOrUnion as PythonStruct

# thrift-py-deprecated struct doesn't have a base class,
# thus this hacky way to do type checking
class PyDeprecatedStruct(typing.Protocol):
    # pyre-ignore[4]: Attribute annotation cannot be `Any`.
    thrift_spec: typing.Any

T = typing.TypeVar("T", bound=PyDeprecatedStruct)
TObj = PythonStruct | Py3Struct | PyDeprecatedStruct | PythonMutableStruct

@typing.overload
def to_py_struct(cls: typing.Type[T], obj: TObj) -> T: ...
@typing.overload
def to_py_struct(
    cls: typing.Type[T], obj: typing.Optional[TObj]
) -> typing.Optional[T]: ...

TEnum = typing.TypeVar("TEnum", bound=PythonEnum)

def fbthrift_name_or_key_error(enum_class: typing.Type[TEnum], value: int) -> str: ...
