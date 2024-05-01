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
