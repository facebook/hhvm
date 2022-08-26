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

from typing import Optional, Sequence

from thrift.python.adapter import Adapter
from thrift.python.types import Struct


class AtoiAdapter(Adapter[str, int]):
    @classmethod
    def from_thrift(
        cls,
        original: str,
        *,
        transitive_annotation: Optional[Struct] = None,
    ) -> int:
        return int(original)

    @classmethod
    def to_thrift(
        cls,
        adapted: int,
        *,
        transitive_annotation: Optional[Struct] = None,
    ) -> str:
        return str(adapted)


class ItoaListAdapter(Adapter[Sequence[int], Sequence[str]]):
    @classmethod
    def from_thrift(
        cls,
        original: Sequence[int],
        *,
        transitive_annotation: Optional[Struct] = None,
    ) -> Sequence[str]:
        return [str(i) for i in original]

    @classmethod
    def to_thrift(
        cls,
        adapted: Sequence[str],
        *,
        transitive_annotation: Optional[Struct] = None,
    ) -> Sequence[int]:
        return [int(a) for a in adapted]
