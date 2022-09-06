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


from __future__ import annotations

import typing

from thrift.python.adapter import Adapter
from thrift.python.types import Struct


T = typing.TypeVar("T")


class Wrapped(typing.Generic[T]):
    __slots__ = ["obj"]

    def __init__(self, obj: T) -> None:
        self.obj: T = obj


class Wrapper(Adapter[T, Wrapped[T]]):
    @classmethod
    def from_thrift(
        cls,
        original: T,
        *,
        transitive_annotation: typing.Optional[Struct] = None,
    ) -> Wrapped[T]:
        return Wrapped(obj=original)

    @classmethod
    def to_thrift(
        cls,
        adapted: Wrapped[T],
        *,
        transitive_annotation: typing.Optional[Struct] = None,
    ) -> T:
        return adapted.obj
