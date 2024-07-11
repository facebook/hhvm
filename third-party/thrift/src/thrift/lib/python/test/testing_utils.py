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

from collections.abc import Sequence, Set


# This is a proxy for np.array. It's an allowable input type
# for a container, but it raises on `if Untruthy():`
class Untruthy(Set, Sequence):
    def __init__(self, limit: int) -> None:
        self.i: int = 0
        self.limit: int = limit

    def __len__(self) -> int:
        return self.limit

    # pyre-ignore[14]
    def __iter__(self) -> "Untruthy":
        self.i: int = 0
        return self

    def __contains__(self, x: object) -> bool:
        if not isinstance(x, int):
            return False
        return x >= 0 and x < self.limit

    def __getitem__(self, item: int) -> int:
        if item in self:
            return item
        raise IndexError

    def __next__(self) -> int:
        if self.i >= self.limit:
            raise StopIteration
        ret = self.i
        self.i += 1
        return ret

    def __bool__(self) -> bool:
        raise ValueError("Do not dare question my truth")
