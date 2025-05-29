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

import asyncio
from pathlib import Path

from later.unittest import TestCase
from thrift.lib.python.server.interceptor.test.interceptors import (
    CountingInterceptor,
    OnConnectThrowsInterceptor,
)
from thrift.py3.server import SocketAddress
from thrift.python.client import get_client
from thrift.python.exceptions import TransportError
from thrift.python.server import ServiceInterface, ThriftServer
from thrift.python.server_impl.interceptor.server_module import PythonServerModule

from thrift.python.server_impl.interceptor.service_interceptor import (
    PyObservableServiceInterceptor,
)
from thrift.python.service_interceptor.basic_service.thrift_clients import BasicService
from thrift.python.service_interceptor.basic_service.thrift_services import (
    BasicServiceInterface,
)


class Handler(BasicServiceInterface):
    async def toLowerSnake(self, input: str) -> str:
        if not input:
            return ""

        chars = [input[0].lower()]
        for c in input[1:]:
            if c.isupper():
                chars.append("_")
            chars.append(c.lower())

        return "".join(chars)


class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task | None

    def __init__(
        self,
        ip: str | None = None,
        path: Path | None = None,
        handler: ServiceInterface = Handler(),  # noqa: B008
    ) -> None:
        self.server = ThriftServer(handler, ip=ip, path=path)
        self.serve_task: asyncio.Task | None = None

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    async def __aexit__(self, *_exc_info: object) -> None:
        self.server.stop()
        assert self.serve_task
        await self.serve_task


class BasicServerTest(TestCase):
    def setUp(self) -> None:
        self.server = TestServer(handler=Handler(), ip="::1")
        super().setUp()

    async def test_basic(self) -> None:
        async with self.server as server_addr:
            assert server_addr.port and server_addr.ip
            async with get_client(
                BasicService, host=server_addr.ip, port=server_addr.port
            ) as client:
                self.assertEqual("", await client.toLowerSnake(""))
                self.assertEqual("hello_world", await client.toLowerSnake("HelloWorld"))


class CountingInterceptorTest(TestCase):
    def setUp(self) -> None:
        self.observer = CountingInterceptor()
        self.module = PythonServerModule("TestModule")
        self.module.add_service_interceptor(
            PyObservableServiceInterceptor(self.observer)
        )
        self.server = TestServer(handler=Handler(), ip="::1")
        self.server.server.add_server_module(self.module)
        super().setUp()

    @property
    def counts(self) -> CountingInterceptor.Counts:
        return self.observer.counts

    async def test_basic(self) -> None:
        self.assertEqual(self.counts.onConnect, 0)
        self.assertEqual(self.counts.onConnectClosed, 0)

        async with self.server as server_addr:
            assert server_addr.port and server_addr.ip
            async with get_client(
                BasicService, host=server_addr.ip, port=server_addr.port
            ) as client:
                self.assertEqual("", await client.toLowerSnake(""))
                self.assertEqual("hello_world", await client.toLowerSnake("HelloWorld"))

        self.assertEqual(self.counts.onConnectClosed, 2)
        self.assertEqual(self.counts.onConnect, 2)
        self.assertEqual(len(self.observer.connection_states), 2)
        # BAD: onRequest and onResponse are not yet invoked by PythonAsyncProcessor
        self.assertEqual(self.counts.onRequest, 0)
        self.assertEqual(self.counts.onResponse, 0)


class ThrowingInterceptorTest(TestCase):
    def setUp(self) -> None:
        self.observer = OnConnectThrowsInterceptor()
        self.module = PythonServerModule("TestModule")
        self.module.add_service_interceptor(
            PyObservableServiceInterceptor(self.observer)
        )
        self.server = TestServer(handler=Handler(), ip="::1")
        self.server.server.add_server_module(self.module)
        super().setUp()

    async def test_basic(self) -> None:
        async with self.server as server_addr:
            assert server_addr.port and server_addr.ip
            async with get_client(
                BasicService, host=server_addr.ip, port=server_addr.port
            ) as client:
                with self.assertRaises(TransportError):
                    await client.toLowerSnake("HelloWorld")

        self.assertEqual(self.observer.on_connection_throws, 1)
