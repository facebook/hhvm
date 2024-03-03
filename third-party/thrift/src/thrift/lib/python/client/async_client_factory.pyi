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

import ipaddress
import os
import typing

from thrift.python.client.async_client import AsyncClient
from thrift.python.client.client_wrapper import Client, TAsyncClient, TSyncClient
from thrift.python.client.request_channel import ClientType
from thrift.python.client.ssl import SSLContext
from thrift.python.serializer import Protocol

Path = typing.Union[str, bytes, os.PathLike[str], os.PathLike[bytes]]

def get_client(
    clientKlass: typing.Type[Client[TAsyncClient, TSyncClient]],
    *,
    host: typing.Optional[
        typing.Union[str, ipaddress.IPv4Address, ipaddress.IPv6Address]
    ] = ...,
    port: typing.Optional[int] = ...,
    path: typing.Optional[Path] = ...,
    timeout: float = ...,
    client_type: ClientType = ...,
    protocol: Protocol = ...,
    ssl_context: typing.Optional[SSLContext] = ...,
    ssl_timeout: float = ...,
) -> TAsyncClient: ...
def install_proxy_factory(
    factory: typing.Optional[typing.Callable[[typing.Type[AsyncClient]], ...]],
) -> None: ...
def get_proxy_factory() -> (
    typing.Optional[typing.Callable[[typing.Type[AsyncClient]], ...]]
): ...
