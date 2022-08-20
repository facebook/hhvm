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


class AtoiAdapter(Adapter[str, int]):
    @classmethod
    def from_thrift(cls, original: str) -> int:
        return int(original)

    @classmethod
    def to_thrift(cls, adapted: int) -> str:
        return str(adapted)


class ItoaListAdapter(Adapter[typing.Sequence[int], typing.Sequence[str]]):
    @classmethod
    def from_thrift(cls, original: typing.Sequence[int]) -> typing.Sequence[str]:
        return [str(i) for i in original]

    @classmethod
    def to_thrift(cls, adapted: typing.Sequence[str]) -> typing.Sequence[int]:
        return [int(a) for a in adapted]
