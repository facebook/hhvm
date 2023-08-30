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


from __future__ import annotations

import asyncio
import unittest
from pathlib import Path
from typing import Optional, Sequence

from apache.thrift.metadata.thrift_types import ThriftServiceMetadataResponse
from testing.thrift_services import TestingServiceInterface
from testing.thrift_types import Color, easy, SimpleError
from thrift.py3.server import get_context, SocketAddress
from thrift.python.serializer import deserialize, Protocol
from thrift.python.server import ServiceInterface, ThriftServer

from .metadata_response import get_serialized_cpp_metadata


class Handler(TestingServiceInterface):
    async def invert(self, value: bool) -> bool:
        ctx = get_context()
        if "from client" in ctx.read_headers:
            ctx.set_header("from server", "with love")
        return not value

    async def getName(self) -> str:
        ctx = get_context()
        ctx.set_header("contextvar", "true")
        return "Testing"

    async def getMethodName(self) -> str:
        ctx = get_context()
        return ctx.method_name

    async def getRequestId(self) -> str:
        ctx = get_context()
        return ctx.request_id

    async def getRequestTimeout(self) -> float:
        ctx = get_context()
        return ctx.request_timeout

    async def shutdown(self) -> None:
        pass

    async def complex_action(
        self, first: str, second: str, third: int, fourth: str
    ) -> int:
        return third

    async def takes_a_list(self, ints: Sequence[int]) -> None:
        raise SimpleError(color=Color.red)

    async def take_it_easy(self, how: int, what: easy) -> None:
        pass

    async def pick_a_color(self, color: Color) -> None:
        pass

    async def int_sizes(self, one: int, two: int, three: int, four: int) -> None:
        pass

    async def hard_error(self, valid: bool) -> None:
        pass

    async def renamed_func(self, ret: bool) -> bool:
        return ret


# pyre-fixme[13]: Attribute `serve_task` is never initialized.
class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task

    def __init__(
        self,
        ip: Optional[str] = None,
        path: Optional["Path"] = None,
        handler: ServiceInterface = Handler(),  # noqa: B008
    ) -> None:
        self.server = ThriftServer(handler, ip=ip, path=path)

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    # pyre-fixme[2]: Parameter must be annotated.
    async def __aexit__(self, *exc_info) -> None:
        self.server.stop()
        await self.serve_task


class MetadataResponseTest(unittest.TestCase):
    """
    These are tests where a client and server talk to each other
    """

    async def test_server_localhost(self) -> None:
        server = TestServer(ip="::1")
        async with server as _:
            metadata_cpp = deserialize(
                ThriftServiceMetadataResponse,
                await get_serialized_cpp_metadata(server.server),
                protocol=Protocol.BINARY,
            )
            metadata_python = (
                TestingServiceInterface.__get_metadata_service_response__()
            )
            self.assertEqual(metadata_cpp, metadata_python)
