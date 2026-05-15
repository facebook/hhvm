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

import ipaddress
import pathlib
from typing import NamedTuple, Optional, overload, TypeVar, Union

from thrift.python.common import Headers, Priority

IPAddress = Union[ipaddress.IPv4Address, ipaddress.IPv6Address]
T = TypeVar("T")

class SocketAddress(NamedTuple):
    ip: Optional[IPAddress]
    port: Optional[int]
    path: Optional[pathlib.Path]

@overload
def get_context() -> RequestContext: ...
@overload
def get_context(default: T) -> Union[T, RequestContext]: ...

class ClientMetadata:
    @property
    def agent(self) -> str: ...
    @property
    def hostname(self) -> str: ...
    def getMetadataField(self, key: str) -> str: ...

class ConnectionContext:
    @property
    def peer_address(self) -> SocketAddress: ...
    @property
    def peer_common_name(self) -> str: ...
    @property
    def security_protocol(self) -> str: ...
    @property
    def peer_certificate(self) -> bytes: ...
    @property
    def peer_certificate_identity(self) -> str: ...
    @property
    def local_address(self) -> SocketAddress: ...
    @property
    def client_metadata(self) -> ClientMetadata: ...

class RequestContext:
    def is_valid(self) -> bool: ...
    @property
    def connection_context(self) -> ConnectionContext: ...
    @property
    def read_headers(self) -> ReadHeaders:
        """Raises RuntimeError if the request context is no longer valid when accessed."""
        ...
    @property
    def write_headers(self) -> WriteHeaders:
        """Raises RuntimeError if the request context is no longer valid when accessed."""
        ...
    @property
    def priority(self) -> Priority:
        """Raises RuntimeError if the request context is no longer valid."""
        ...
    def set_header(self, key: str, value: str) -> None:
        """Raises RuntimeError if the request context is no longer valid."""
        ...
    @property
    def method_name(self) -> str: ...
    @property
    def request_id(self) -> str: ...
    @property
    def request_timeout(self) -> float:
        """Raises RuntimeError if the request context is no longer valid."""
        ...
    @property
    def read_headers_or_none(self) -> Optional[ReadHeaders]:
        """Returns read headers, or None if the request context is no longer valid.
        Validity is checked at call time but does not guard against subsequent
        RuntimeError on header map access if the ReadHeaders outlives the
        Cpp2RequestContext*."""
    @property
    def write_headers_or_none(self) -> Optional[WriteHeaders]:
        """Returns write headers, or None if the request context is no longer valid.
        Validity is checked at call time but does not guard against subsequent
        RuntimeError on header map access if the WriteHeaders outlives the
        Cpp2RequestContext*."""
    @property
    def priority_or_none(self) -> Optional[Priority]:
        """Returns priority, or None if the request context is no longer valid."""
    @property
    def request_timeout_or_none(self) -> Optional[float]:
        """Returns request timeout, or None if the request context is no longer valid."""

class ReadHeaders(Headers): ...
class WriteHeaders(Headers): ...
