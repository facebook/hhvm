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

from typing import AsyncIterator, Generic, Tuple, TypeVar

_T = TypeVar("_T")
rT = TypeVar("rT")

class ClientBufferedStream(AsyncIterator[_T]):
    """
    Base class for all ClientBufferedStream object
    """

    def __aiter__(self) -> AsyncIterator[_T]: ...
    async def __anext__(self) -> _T: ...

class ResponseAndClientBufferedStream(Generic[rT, _T]):
    def __iter__(self) -> Tuple[rT, ClientBufferedStream[_T]]: ...

class ServerStream(Generic[_T]):
    pass

class ServerPublisher(Generic[_T]):
    pass

class ResponseAndServerStream(Generic[rT, _T]):
    pass
