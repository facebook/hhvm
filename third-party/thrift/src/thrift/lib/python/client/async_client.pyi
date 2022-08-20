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

import enum
import types
import typing

from thrift.python.client.omni_client import RpcKind
from thrift.python.types import _fbthrift_ResponseStreamResult, StructOrUnion, TChunk

TAsyncClient = typing.TypeVar("TAsyncClient", bound="AsyncClient")
TResponse = typing.TypeVar("TResponse", bound=StructOrUnion)

class AsyncClient:
    def __init__(self) -> None: ...
    async def __aenter__(self: TAsyncClient) -> TAsyncClient: ...
    async def __aexit__(
        self,
        exc_type: typing.Optional[typing.Type[BaseException]],
        exc_value: typing.Optional[BaseException],
        traceback: typing.Optional[types.TracebackType],
    ) -> None: ...
    async def _create_interaction(
        self: AsyncClient,
        methodName: str,
        interactionClass: typing.Type[TAsyncClient],
    ) -> TAsyncClient: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion,
        response_cls: None,
        rpc_kind: RpcKind = ...,
    ) -> None: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion,
        response_cls: typing.Type[TResponse],
        rpc_kind: RpcKind = ...,
    ) -> TResponse: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion,
        response_cls: typing.Tuple[
            typing.Type[TResponse],
            typing.Type[_fbthrift_ResponseStreamResult[TChunk]],
        ],
        rpc_kind: RpcKind = ...,
    ) -> typing.Tuple[TResponse, typing.AsyncGenerator[TChunk, None]]: ...
    def set_persistent_header(self, key: str, value: str) -> None: ...
    # pyre-ignore[2]: callback returns are ignored, can be any type
    def _at_aexit(self, callback: typing.Callable[[], typing.Any]) -> None: ...
