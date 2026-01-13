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

import types
import typing

from thrift.python.client.omni_client import (
    FunctionQualifier,
    InteractionMethodPosition,
    RpcKind,
)
from thrift.python.common import RpcOptions
from thrift.python.mutable_types import (
    _fbthrift_MutableResponseStreamResult,
    _fbthrift_MutableSinkFinalResult,
    _fbthrift_MutableSinkResult,
    MutableStructOrUnion,
)
from thrift.python.streaming.bidistream import BidirectionalStream
from thrift.python.streaming.sink import ClientSink
from thrift.python.types import (
    _fbthrift_ResponseStreamResult,
    _fbthrift_SinkFinalResult,
    _fbthrift_SinkResult,
    StructOrUnion,
    TChunk,
)

TAsyncClient = typing.TypeVar("TAsyncClient", bound="AsyncClient")
TResponse = typing.TypeVar(
    "TResponse", bound=typing.Union[StructOrUnion, MutableStructOrUnion]
)
TSinkChunk = typing.TypeVar("TSinkChunk")
TFinalResponse = typing.TypeVar("TFinalResponse")
TStreamChunk = typing.TypeVar("TStreamChunk")

class AsyncClient:
    def __init__(self) -> None: ...
    async def __aenter__(self: TAsyncClient) -> TAsyncClient: ...
    async def __aexit__(
        self,
        exc_type: typing.Optional[typing.Type[BaseException]],
        exc_value: typing.Optional[BaseException],
        traceback: typing.Optional[types.TracebackType],
    ) -> None: ...
    def _create_interaction(
        self: AsyncClient,
        methodName: str,
        interactionClass: typing.Type[TAsyncClient],
    ) -> TAsyncClient: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion | MutableStructOrUnion,
        response_cls: None,
        *,
        rpc_kind: RpcKind = ...,
        qualifier: FunctionQualifier = ...,
        interaction_position: InteractionMethodPosition = ...,
        interaction_name: str = ...,
        created_interaction: AsyncClient = ...,
        uri_or_name: str = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
        is_mutable_types: typing.Optional[bool] = False,
    ) -> None: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion | MutableStructOrUnion,
        response_cls: typing.Type[TResponse],
        rpc_kind: RpcKind = ...,
        qualifier: FunctionQualifier = ...,
        interaction_position: InteractionMethodPosition = ...,
        interaction_name: str = ...,
        created_interaction: AsyncClient = ...,
        uri_or_name: str = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
        is_mutable_types: typing.Optional[bool] = False,
    ) -> TResponse: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion | MutableStructOrUnion,
        response_cls: typing.Tuple[
            typing.Type[TResponse],
            typing.Type[_fbthrift_ResponseStreamResult[TChunk]]
            | typing.Type[_fbthrift_MutableResponseStreamResult[TChunk]],
        ],
        rpc_kind: RpcKind = ...,
        qualifier: FunctionQualifier = ...,
        interaction_position: InteractionMethodPosition = ...,
        interaction_name: str = ...,
        created_interaction: AsyncClient = ...,
        uri_or_name: str = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
        is_mutable_types: typing.Optional[bool] = False,
    ) -> typing.Tuple[TResponse, typing.AsyncGenerator[TChunk, None]]: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion | MutableStructOrUnion,
        response_cls: typing.Tuple[
            typing.Type[TResponse],
            typing.Type[_fbthrift_SinkResult[TSinkChunk]]
            | typing.Type[_fbthrift_MutableSinkResult[TSinkChunk]],
            typing.Type[_fbthrift_ResponseStreamResult[TStreamChunk]]
            | typing.Type[_fbthrift_MutableResponseStreamResult[TStreamChunk]],
        ],
        rpc_kind: RpcKind = ...,
        qualifier: FunctionQualifier = ...,
        interaction_position: InteractionMethodPosition = ...,
        interaction_name: str = ...,
        created_interaction: AsyncClient = ...,
        uri_or_name: str = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
        is_mutable_types: typing.Optional[bool] = False,
    ) -> typing.Tuple[TResponse, BidirectionalStream[TSinkChunk, TStreamChunk]]: ...
    @typing.overload
    async def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: StructOrUnion | MutableStructOrUnion,
        response_cls: typing.Tuple[
            typing.Type[TResponse],
            typing.Type[_fbthrift_SinkResult[TSinkChunk]]
            | typing.Type[_fbthrift_MutableSinkResult[TSinkChunk]],
            typing.Type[_fbthrift_SinkFinalResult[TFinalResponse]]
            | typing.Type[_fbthrift_MutableSinkFinalResult[TFinalResponse]],
        ],
        rpc_kind: RpcKind = ...,
        qualifier: FunctionQualifier = ...,
        interaction_position: InteractionMethodPosition = ...,
        interaction_name: str = ...,
        created_interaction: AsyncClient = ...,
        uri_or_name: str = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
        is_mutable_types: typing.Optional[bool] = False,
    ) -> typing.Tuple[
        TResponse,
        ClientSink[TSinkChunk, TFinalResponse]
        | BidirectionalStream[TSinkChunk, TFinalResponse],
    ]: ...
    def set_persistent_header(self, key: str, value: str) -> None: ...
    def _at_aexit(self, callback: typing.Callable[[], typing.Any]) -> None: ...
