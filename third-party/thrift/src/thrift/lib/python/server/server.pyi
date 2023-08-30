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

import ipaddress
import os
from enum import Enum
from types import TracebackType
from typing import Any, Awaitable, Callable, Mapping, Optional, Type, TypeVar, Union

from folly.iobuf import IOBuf
from thrift.py3.server import ThriftServer as ThriftServer_py3

# This looks really dumb but otherwise this name doesn't get re-exported
from thrift.python.types import ServiceInterface as ServiceInterface

IPAddress = Union[ipaddress.IPv4Address, ipaddress.IPv6Address]
# pyre-fixme[24]: Generic type `os.PathLike` expects 1 type parameter.
Path = Union[str, bytes, os.PathLike]

_T = TypeVar("_T", bound=Callable[..., Awaitable[None]])

class RpcKind(Enum):
    SINGLE_REQUEST_SINGLE_RESPONSE: RpcKind = ...
    SINGLE_REQUEST_NO_RESPONSE: RpcKind = ...
    SINGLE_REQUEST_STREAMING_RESPONSE: RpcKind = ...
    SINK: RpcKind = ...

class PythonUserException(Exception):
    def __init__(self, type_: str, reason: str, buf: IOBuf) -> None: ...

class ThriftServer(ThriftServer_py3):
    def __init__(
        self,
        handler: ServiceInterface,
        port: int = 0,
        ip: Optional[IPAddress | str] = None,
        path: Optional[Path] = None,
    ) -> None: ...
