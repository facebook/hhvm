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

from contextlib import AbstractAsyncContextManager
from typing import TypeVar

from thrift.python.client.async_client import AsyncClient
from thrift.python.client.client_wrapper import Client
from thrift.python.client.sync_client import SyncClient

_AsyncClient = TypeVar("_AsyncClient", bound=AsyncClient)
_SyncClient = TypeVar("_SyncClient", bound=SyncClient)

def get_compression_test_client(
    client_class: type[Client[_AsyncClient, _SyncClient]], host: str, port: int
) -> AbstractAsyncContextManager[_AsyncClient, None]: ...
def set_compression_offload_enabled(enabled: bool) -> None: ...
def reset_compression_offload_flag() -> None: ...
