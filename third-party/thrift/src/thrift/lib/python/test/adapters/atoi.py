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


from __future__ import annotations

from typing import Mapping, Optional, Sequence

from thrift.python.adapter import Adapter
from thrift.python.types import Struct


class AtoiAdapter(Adapter[str, int]):
    @classmethod
    def from_thrift(
        cls,
        original: str,
        *,
        transitive_annotation: Optional[Struct] = None,
        constant_uri: Optional[str] = None,
    ) -> int:
        return int(original)

    @classmethod
    def to_thrift(
        cls,
        adapted: int,
        *,
        transitive_annotation: Optional[Struct] = None,
        constant_uri: Optional[str] = None,
    ) -> str:
        return str(adapted)


class ItoaListAdapter(Adapter[Sequence[int], Sequence[str]]):
    @classmethod
    def from_thrift(
        cls,
        original: Sequence[int],
        *,
        transitive_annotation: Optional[Struct] = None,
        constant_uri: Optional[str] = None,
    ) -> Sequence[str]:
        return [str(i) for i in original]

    @classmethod
    def to_thrift(
        cls,
        adapted: Sequence[str],
        *,
        transitive_annotation: Optional[Struct] = None,
        constant_uri: Optional[str] = None,
    ) -> Sequence[int]:
        return [int(a) for a in adapted]


class ItoaNestedListAdapter(
    Adapter[
        Sequence[Sequence[Mapping[int, int]]], Sequence[Sequence[Mapping[str, str]]]
    ]
):
    @classmethod
    def from_thrift(
        cls,
        original: Sequence[Sequence[Mapping[int, int]]],
        *,
        transitive_annotation: Optional[Struct] = None,
        constant_uri: Optional[str] = None,
    ) -> Sequence[Sequence[Mapping[str, str]]]:
        return [
            [{str(key): str(value) for key, value in j.items()} for j in i]
            for i in original
        ]

    @classmethod
    def to_thrift(
        cls,
        adapted: Sequence[Sequence[Mapping[str, str]]],
        *,
        transitive_annotation: Optional[Struct] = None,
        constant_uri: Optional[str] = None,
    ) -> Sequence[Sequence[Mapping[int, int]]]:
        return [
            [{int(key): int(value) for key, value in j.items()} for j in i]
            for i in adapted
        ]
