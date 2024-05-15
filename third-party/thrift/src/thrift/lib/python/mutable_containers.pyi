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

class MutableList:
    def __init__(
        self,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        typeinfo: typing.Any,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        list_data: typing.List[typing.Any],
    ) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    # pyre-ignore[3]: Parameter annotation cannot be `Any`.
    def __getitem__(self, index: typing.Any) -> typing.Any: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def __setitem__(self, index: typing.Any, value: typing.Any) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def __delitem__(self, index: typing.Any) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def insert(self, index: typing.Any, value: typing.Any) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def append(self, value: typing.Any) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def extend(self, values: typing.Iterable[typing.Any]) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def pop(self, index: typing.Optional[typing.Any] = -1) -> None: ...
    def clear(self) -> None: ...

class MutableSet:
    def __init__(
        self,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        typeinfo: typing.Any,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        set_data: typing.Set[typing.Any],
    ) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def __contains__(self, item: typing.Any) -> bool: ...
    def __iter__(self) -> MutableSetIterator: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def isdisjoint(self, other: typing.Iterable[typing.Any]) -> bool: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def __and__(self, other: typing.Iterable[typing.Any]) -> MutableSet: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    # pyre-ignore[15]: Inconsisten override
    def __or__(self, other: typing.Iterable[typing.Any]) -> MutableSet: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def __sub__(self, other: typing.Iterable[typing.Any]) -> MutableSet: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def __xor__(self, other: typing.Iterable[typing.Any]) -> MutableSet: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def add(self, value: typing.Any) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def discard(self, value: typing.Any) -> None: ...
    # pyre-ignore[2]: Parameter annotation cannot be `Any`.
    def remove(self, value: typing.Any) -> None: ...
    # pyre-ignore[3]: Return annotation cannot be `Any`.
    def pop(self) -> typing.Any: ...
    def clear(self) -> None: ...
    @classmethod
    def _from_iterable(
        cls,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        typeinfo: typing.Any,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        set_data: typing.Set[typing.Any],
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        it: typing.Iterable[typing.Any],
    ) -> MutableSet: ...

class MutableSetIterator:
    def __init__(
        self,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        typeinfo: typing.Any,
        # pyre-ignore[2]: Parameter annotation cannot be `Any`.
        set_data: typing.Set[typing.Any],
    ) -> None: ...
    # pyre-ignore[3]: Return annotation cannot be `Any`.
    def __next__(self) -> typing.Any: ...
    def __iter__(self) -> MutableSetIterator: ...
