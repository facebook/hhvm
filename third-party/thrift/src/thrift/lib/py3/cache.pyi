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

from typing import AbstractSet, FrozenSet, Mapping, Sequence, Tuple, TypeVar

_T = TypeVar("_T")
_K = TypeVar("_K")

def to_tuple(py3_list: Sequence[_T]) -> Tuple[_T]: ...
def to_frozenset(py3_set: AbstractSet[_T]) -> FrozenSet[_T]: ...
def to_mappingproxy(py3_map: Mapping[_K, _T]) -> Mapping[_K, _T]: ...
