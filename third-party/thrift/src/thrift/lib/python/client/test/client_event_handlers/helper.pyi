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
from typing import overload, Type, TypeVar

from thrift.py3.client import Client as Py3Client
from thrift.python.client.client_wrapper import Client, TAsyncClient, TSyncClient

TClient = TypeVar("TClient", bound=Py3Client)

class TestHelper:
    @overload
    def get_async_client(
        self,
        clientKlass: Type[Client[TAsyncClient, TSyncClient]],
        host: str = ...,
        port: int = ...,
    ) -> TAsyncClient: ...
    @overload
    def get_async_client(
        self, clientKlass: Type[TClient], host: str = ..., port: int = ...
    ) -> TClient: ...
    def get_sync_client(
        self,
        clientKlass: Type[Client[TAsyncClient, TSyncClient]],
        host: str = ...,
        port: int = ...,
    ) -> TSyncClient: ...
    def is_handler_called(self) -> bool: ...
