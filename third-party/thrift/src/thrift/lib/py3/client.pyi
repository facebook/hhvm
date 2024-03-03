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
from enum import Enum
from types import TracebackType
from typing import (
    Any,
    Callable,
    ClassVar,
    Dict,
    Mapping,
    Optional,
    Type,
    TypeVar,
    Union,
)

from thrift.py3.common import Headers, Priority
from thrift.py3.serializer import Protocol
from thrift.py3.ssl import SSLContext

IPAddress = Union[ipaddress.IPv4Address, ipaddress.IPv6Address]
Path = Union[str, bytes, os.PathLike[str], os.PathLike[bytes]]
TClient = TypeVar("TClient", bound="Client")

class ClientType(Enum):
    THRIFT_HEADER_CLIENT_TYPE: ClientType = ...
    THRIFT_FRAMED_DEPRECATED: ClientType = ...
    THRIFT_UNFRAMED_DEPRECATED: ClientType = ...
    THRIFT_HTTP_SERVER_TYPE: ClientType = ...
    THRIFT_HTTP_CLIENT_TYPE: ClientType = ...
    THRIFT_ROCKET_CLIENT_TYPE: ClientType = ...
    THRIFT_FRAMED_COMPACT: ClientType = ...
    THRIFT_HTTP_GET_CLIENT_TYPE: ClientType = ...
    THRIFT_UNKNOWN_CLIENT_TYPE: ClientType = ...
    THRIFT_UNFRAMED_COMPACT_DEPRECATED: ClientType = ...

class Client:
    def set_persistent_header(self, key: str, value: str) -> None: ...
    async def __aenter__(self: TClient) -> TClient: ...
    async def __aexit__(
        self,
        exc_type: Optional[Type[BaseException]],
        exc_value: Optional[BaseException],
        traceback: Optional[TracebackType],
    ) -> Optional[bool]: ...
    annotations: ClassVar[Mapping[str, str]] = ...

def get_client(
    clientKlass: Type[TClient],
    *,
    host: Union[IPAddress, str] = ...,
    port: int = ...,
    path: Optional[Path] = ...,
    timeout: float = ...,
    headers: Dict[str, str] = ...,
    client_type: ClientType = ...,
    protocol: Protocol = ...,
    ssl_context: Optional[SSLContext] = ...,
    ssl_timeout: float = ...,
) -> TClient: ...
def install_proxy_factory(
    # pyre-ignore[2] : it may return anything
    factory: Optional[Callable[[Type[TClient]], Callable[[TClient], Any]]],
) -> None: ...

# pyre-ignore[3] : it may return anything
def get_proxy_factory() -> (
    Optional[Callable[[Type[Client]], Callable[[Client], Any]]]
): ...
