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

import types
import typing

from thrift.python.client.request_channel import RequestChannel
from thrift.python.common import RpcOptions
from thrift.python.types import Struct, Union

TSyncClient = typing.TypeVar("TSyncClient", bound="SyncClient")
StructOrUnion = typing.TypeVar("StructOrUnion", bound=typing.Union[Struct, Union])

class SyncClient:
    def __init__(self, channel: RequestChannel) -> None: ...
    def __enter__(self: TSyncClient) -> TSyncClient: ...
    def __exit__(
        self,
        type: typing.Type[Exception],
        value: Exception,
        traceback: types.TracebackType,
    ) -> None: ...
    def clear_event_handlers(self: TSyncClient) -> None: ...
    def _send_request(
        self,
        service_name: str = ...,
        function_name: str = ...,
        args: Struct = ...,
        response_cls: typing.Optional[typing.Type[StructOrUnion]] = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
    ) -> StructOrUnion: ...
    def set_persistent_header(self, key: str, value: str) -> None: ...
    # pyre-ignore[2]: callback returns are ignored, can be any type
    def _at_exit(self, callback: typing.Callable[[], typing.Any]) -> None: ...
