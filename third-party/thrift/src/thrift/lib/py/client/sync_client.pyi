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

from thrift.py.client.common import RequestChannel
from thrift.py3.common import RpcOptions

TSyncClient = typing.TypeVar("TSyncClient", bound="SyncClient")
# thrift-py types doesn't have a base class
TStruct = typing.TypeVar("TStruct")
TStructOrUnion = typing.TypeVar("TStructOrUnion")

class SyncClient:
    def __init__(self, channel: RequestChannel, service_name: str) -> None: ...
    def __enter__(self: TSyncClient) -> TSyncClient: ...
    def __exit__(
        self,
        type: typing.Type[Exception],
        value: Exception,
        traceback: types.TracebackType,
    ) -> None: ...
    def _send_request(
        self,
        service_name: str,
        function_name: str,
        args: TStruct,
        response_cls: typing.Optional[typing.Type[TStructOrUnion]],
        *,
        uri_or_name: str = ...,
        rpc_options: typing.Optional[RpcOptions] = ...,
    ) -> TStructOrUnion: ...
    def set_persistent_header(self, key: str, value: str) -> None: ...
    def get_persistent_headers(self) -> typing.Mapping[str, str]: ...
    def clear_persistent_headers(self) -> None: ...
    def set_onetime_header(self, key: str, value: str) -> None: ...
    def get_last_response_headers(self) -> typing.Mapping[str, str]: ...
