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


from __future__ import annotations

import unittest
from typing import Sequence

from apache.thrift.metadata.thrift_types import ThriftServiceMetadataResponse
from testing.thrift_services import TestingServiceInterface
from testing.thrift_types import Color, easy, SimpleError
from thrift.lib.python.test.test_server import TestServer
from thrift.py3.server import get_context
from thrift.python.serializer import deserialize, Protocol

from .metadata_response import get_serialized_cpp_metadata


def local_server() -> TestServer:
    return TestServer(handler=Handler(), ip="::1")


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


class MetadataResponseTest(unittest.IsolatedAsyncioTestCase):
    """
    These are tests where a client and server talk to each other
    """

    async def test_server_localhost(self) -> None:
        server = local_server()
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
