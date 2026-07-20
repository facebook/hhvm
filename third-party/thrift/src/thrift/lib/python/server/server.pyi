#!/usr/bin/env python3
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
import os
from enum import Enum
from types import TracebackType
from typing import Any, Awaitable, Callable, Mapping, Optional, Type, TypeVar, Union

from folly.iobuf import IOBuf
from thrift.py3.server import SSLPolicy as _SSLPolicy, ThriftServer as _ThriftServer

# they are now merged into one
ThriftServer = _ThriftServer
SSLPolicy = _SSLPolicy

# This looks really dumb but otherwise this name doesn't get re-exported
from thrift.python.server_impl.interactions import Interaction as Interaction

# Re-exported to mirror server.pyx, which imports these from
# thrift.python.server_impl.request_context. This lets callers use the
# thrift-python-native `from thrift.python.server import get_context`.
from thrift.python.server_impl.request_context import (
    get_context as get_context,
    RequestContext as RequestContext,
    SocketAddress as SocketAddress,
)
from thrift.python.types import (
    FunctionEntry as FunctionEntry,
    ServiceInterface as ServiceInterface,
)

# Re-exported from thrift.python.server_impl.python_async_processor for generated
# service code: installs a handler-returned interaction Tile for a
# factory-with-initial-response method.
def install_interaction_tile(tile: object) -> None: ...

IPAddress = Union[ipaddress.IPv4Address, ipaddress.IPv6Address]
# pyre-fixme[24]: Generic type `os.PathLike` expects 1 type parameter.
Path = Union[str, bytes, os.PathLike]

_T = TypeVar("_T", bound=Callable[..., Awaitable[None]])

class RpcKind(Enum):
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_SINGLE_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_NO_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_STREAMING_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINK: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    BIDIRECTIONAL_STREAM: RpcKind = ...

class PythonUserException(Exception):
    def __init__(self, type_: str, reason: str, buf: IOBuf) -> None: ...
