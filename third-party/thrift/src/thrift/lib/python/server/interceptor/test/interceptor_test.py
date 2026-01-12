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
import time
from pathlib import Path
from typing import AsyncIterator, Awaitable, Callable, cast, Type

from later.unittest import TestCase
from thrift.lib.python.server.interceptor.test.interceptors import (
    CountingInterceptor,
    OnConnectThrowsInterceptor,
    OnRequestThrowsInterceptor,
    OnResponseThrowsInterceptor,
)
from thrift.py3.server import SocketAddress
from thrift.python.client import AsyncClient, ClientType, get_client
from thrift.python.client.client_wrapper import TAsyncClient
from thrift.python.exceptions import ApplicationError, TransportError
from thrift.python.server import ServiceInterface, ThriftServer
from thrift.python.server_impl.interceptor.server_module import PythonServerModule
from thrift.python.server_impl.interceptor.service_interceptor import (
    AbstractServiceInterceptor,
    PyObservableServiceInterceptor,
)
from thrift.python.service_interceptor.basic_service.thrift_clients import (
    BasicService,
    OnewayService,
    StreamingService,
)
from thrift.python.service_interceptor.basic_service.thrift_services import (
    BasicServiceInterface,
    OnewayServiceInterface,
    StreamingServiceInterface,
)


def to_lower_snake(input: str) -> str:
    if not input:
        return ""

    chars = [input[0].lower()]
    for c in input[1:]:
        if c.isupper():
            chars.append("_")
        chars.append(c.lower())

    return "".join(chars)


class Handler(BasicServiceInterface):
    async def toLowerSnake(self, input: str) -> str:
        return to_lower_snake(input)


class OnewayHandler(OnewayServiceInterface):
    async def toLowerSnake(self, input: str) -> None:
        raise RuntimeError("Oops")
        return None


class StreamHandler(StreamingServiceInterface):
    async def toLowerSnake(self, input: str) -> AsyncIterator[str]:
        yield to_lower_snake(input)


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


class BaseInterceptorTestCase(TestCase):
    server: TestServer | None = None
    client_class: Type[AsyncClient] = AsyncClient
    observer: AbstractServiceInterceptor[object, object] | None = None

    @classmethod
    def setUpClass(cls) -> None:
        # Python 3.14+ requires explicit event loop management.
        # Reuse existing loop if available, otherwise create a new one.
        try:
            _ = asyncio.get_running_loop()
            cls.loop = None
        except RuntimeError:
            cls.loop = asyncio.new_event_loop()
            asyncio.set_event_loop(cls.loop)

        cls.server = TestServer()

    @classmethod
    def tearDownClass(cls) -> None:
        if cls.loop:
            cls.loop.close()

    def setUp(self) -> None:
        if self.observer is None:
            super().setUp()
            return

        self.module = PythonServerModule("TestModule")
        assert self.observer is not None
        self.module.add_service_interceptor(
            PyObservableServiceInterceptor(self.observer)
        )
        self.assertIsNotNone(self.server)
        self.server.server.add_server_module(self.module)
        super().setUp()

    async def run_with_local_server(
        self,
        func: Callable[[TAsyncClient], Awaitable[None]],
        client_type: ClientType = ClientType.THRIFT_HEADER_CLIENT_TYPE,
    ) -> None:
        assert self.server is not None
        async with self.server as server_addr:
            assert server_addr.port and server_addr.ip
            async with get_client(
                # pyre-ignore[6]: pyre is confused by Sync vs Async
                self.client_class,
                host=server_addr.ip,
                port=server_addr.port,
                client_type=client_type,
            ) as client:
                await func(cast(TAsyncClient, client))

    async def run_with_local_server_rocket(
        self, func: Callable[[TAsyncClient], Awaitable[None]]
    ) -> None:
        await self.run_with_local_server(func, ClientType.THRIFT_ROCKET_CLIENT_TYPE)


class BasicServiceTestCase(BaseInterceptorTestCase):
    def setUp(self) -> None:
        self.server = TestServer(handler=Handler(), ip="::1")
        self.client_class = BasicService
        super().setUp()

    async def assert_return(self, client: BasicService) -> None:
        self.assertEqual("", await client.toLowerSnake(""))
        self.assertEqual("hello_world", await client.toLowerSnake("HelloWorld"))

    async def test_no_interceptors_header(self) -> None:
        if self.__class__.__name__ != "BasicServiceTestCase":
            return
        await self.run_with_local_server(self.assert_return)

    async def test_no_interceptors_rocket(self) -> None:
        if self.__class__.__name__ != "BasicServiceTestCase":
            return
        await self.run_with_local_server_rocket(self.assert_return)


class OnewayServiceTestCase(BaseInterceptorTestCase):
    def setUp(self) -> None:
        self.server = TestServer(handler=OnewayHandler(), ip="::1")
        self.client_class = OnewayService
        super().setUp()

    async def assert_return(self, client: OnewayService) -> None:
        self.assertIsNone(await client.toLowerSnake(""))
        self.assertIsNone(await client.toLowerSnake("HelloWorld"))
        # oneway client returns immediately, so need some time for the server
        # to actually process the request
        time.sleep(0.5)

    async def test_no_interceptors_header(self) -> None:
        if self.__class__.__name__ != "OnewayServiceTestCase":
            return
        await self.run_with_local_server(self.assert_return)

    async def test_no_interceptors_rocket(self) -> None:
        if self.__class__.__name__ != "OnewayServiceTestCase":
            return
        await self.run_with_local_server_rocket(self.assert_return)


class StreamingServiceTestCase(BaseInterceptorTestCase):
    def setUp(self) -> None:
        self.server = TestServer(handler=StreamHandler(), ip="::1")
        self.client_class = StreamingService
        super().setUp()

    async def assert_return(self, client: StreamingService) -> None:
        empty_stream = await client.toLowerSnake("")
        empty_resp = [resp async for resp in empty_stream]
        self.assertEqual(empty_resp, [""])

        real_stream = await client.toLowerSnake("HelloWorld")
        real_resp = [resp async for resp in real_stream]
        self.assertEqual(real_resp, ["hello_world"])

    async def test_no_interceptors(self) -> None:
        if self.__class__.__name__ != "StreamingServiceTestCase":
            return
        # streaming only works with ROCKET, not HEADER_CLIENT_TYPE
        await self.run_with_local_server_rocket(self.assert_return)


def assert_counts_pre(
    self: BaseInterceptorTestCase, observer: CountingInterceptor
) -> None:
    self.assertEqual(observer.counts.onConnect, 0)
    self.assertEqual(observer.counts.onConnectClosed, 0)


def assert_counts_post(
    self: BaseInterceptorTestCase,
    observer: CountingInterceptor,
    rocket: bool = False,
    oneway: bool = False,
) -> None:
    # 2 connections for HEADER_CLIENT_TYPE, 1 for ROCKET_CLIENT_TYPE
    expected_connections = 1 if rocket else 2
    self.assertEqual(observer.counts.onConnect, expected_connections)
    self.assertEqual(observer.counts.onConnectClosed, expected_connections)
    self.assertEqual(len(observer.connection_states), expected_connections)
    self.assertEqual(observer.counts.onRequest, 2)
    self.assertEqual(observer.services, {self.client_class.__name__})
    self.assertEqual(observer.defining_services, {self.client_class.__name__})
    self.assertEqual(observer.methods, {"toLowerSnake"})
    expected_responses = 0 if oneway else 2
    self.assertEqual(observer.counts.onResponse, expected_responses)


class CountingInterceptorBasicTest(BasicServiceTestCase):
    def setUp(self) -> None:
        self.observer = CountingInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        assert_counts_pre(self, self.observer)
        await self.run_with_local_server(self.assert_return)
        assert_counts_post(self, self.observer)

    async def test_interceptors_rocket(self) -> None:
        assert_counts_pre(self, self.observer)
        await self.run_with_local_server_rocket(self.assert_return)
        assert_counts_post(self, self.observer, rocket=True)


class CountingInterceptorOnewayTest(OnewayServiceTestCase):
    def setUp(self) -> None:
        self.observer = CountingInterceptor()
        super().setUp()

    @property
    def counts(self) -> CountingInterceptor.Counts:
        return self.observer.counts

    async def test_interceptors_header(self) -> None:
        assert_counts_pre(self, self.observer)
        await self.run_with_local_server(self.assert_return)
        assert_counts_post(self, self.observer, oneway=True)

    async def test_interceptors_rocket(self) -> None:
        assert_counts_pre(self, self.observer)
        await self.run_with_local_server_rocket(self.assert_return)
        assert_counts_post(self, self.observer, rocket=True, oneway=True)


class CountingInterceptorStreamingTest(StreamingServiceTestCase):
    def setUp(self) -> None:
        self.observer = CountingInterceptor()
        super().setUp()

    @property
    def counts(self) -> CountingInterceptor.Counts:
        return self.observer.counts

    async def test_interceptors(self) -> None:
        assert_counts_pre(self, self.observer)
        # streaming only works with ROCKET, not HEADER_CLIENT_TYPE
        await self.run_with_local_server_rocket(self.assert_return)
        assert_counts_post(self, self.observer, rocket=True)


class OnConnectThrowsInterceptorBasicTest(BasicServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnConnectThrowsInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            with self.assertRaises(TransportError):
                await client.toLowerSnake("HelloWorld")

        await self.run_with_local_server(assert_basic)

        self.assertEqual(self.observer.on_connection_throws, 1)


class OnConnectThrowsInterceptorOnewayTest(OnewayServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnConnectThrowsInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            # client return is hard-coded to None
            self.assertIsNone(await client.toLowerSnake("HelloWorld"))
            time.sleep(0.5)

        await self.run_with_local_server(assert_basic)

        self.assertEqual(self.observer.on_connection_throws, 1)


class OnConnectThrowsInterceptorStreamingTest(StreamingServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnConnectThrowsInterceptor()
        super().setUp()

    async def test_interceptors_rocket(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            with self.assertRaises(TransportError):
                await client.toLowerSnake("HelloWorld")

        await self.run_with_local_server_rocket(assert_basic)

        self.assertEqual(self.observer.on_connection_throws, 1)


def assert_on_request_throws_post(
    self: BaseInterceptorTestCase, observer: OnRequestThrowsInterceptor
) -> None:
    self.assertEqual(observer.on_request_throws, 1)
    self.assertEqual(observer.on_response, 1)
    self.assertEqual(len(observer.response_errors), 1)
    for err_msg in observer.response_errors:
        self.assertIn("Expect the unexpected", err_msg)


class OnRequestThrowsInterceptorBasicTest(BasicServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnRequestThrowsInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            with self.assertRaisesRegex(
                ApplicationError,
                "OnRequestThrowsInterceptor.*Expect the unexpected",
            ):
                await client.toLowerSnake("HelloWorld")

        await self.run_with_local_server(assert_basic)
        assert_on_request_throws_post(self, self.observer)


class OnRequestThrowsInterceptorOnewayTest(OnewayServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnRequestThrowsInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            # client return is hard-coded to None
            self.assertIsNone(await client.toLowerSnake("HelloWorld"))
            time.sleep(0.5)

        await self.run_with_local_server(assert_basic)
        assert_on_request_throws_post(self, self.observer)


class OnRequestThrowsInterceptorStreamingTest(StreamingServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnRequestThrowsInterceptor()
        super().setUp()

    async def test_interceptors_rocket(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            with self.assertRaisesRegex(
                ApplicationError,
                "OnRequestThrowsInterceptor.*Expect the unexpected",
            ):
                await client.toLowerSnake("HelloWorld")

        await self.run_with_local_server_rocket(assert_basic)
        assert_on_request_throws_post(self, self.observer)


class OnResponseThrowsInterceptorBasicTest(BasicServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnResponseThrowsInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            with self.assertRaisesRegex(
                ApplicationError,
                "OnResponseThrowsInterceptor.*Expect the unexpected",
            ):
                await client.toLowerSnake("HelloWorld")

        await self.run_with_local_server(assert_basic)

        self.assertEqual(self.observer.on_response_throws, 1)


class OnResponseThrowsInterceptorOnewayTest(OnewayServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnResponseThrowsInterceptor()
        super().setUp()

    async def test_interceptors_header(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            # client return is hard-coded to None
            self.assertIsNone(await client.toLowerSnake("HelloWorld"))
            time.sleep(0.5)

        await self.run_with_local_server(assert_basic)

        # oneway onReponse only runs if error in onRequest interceptor
        self.assertEqual(self.observer.on_response_throws, 0)


class OnResponseThrowsInterceptorStreamingTest(StreamingServiceTestCase):
    def setUp(self) -> None:
        self.observer = OnResponseThrowsInterceptor()
        super().setUp()

    async def test_interceptors_rocket(self) -> None:
        async def assert_basic(client: BasicService) -> None:
            with self.assertRaisesRegex(
                ApplicationError,
                "OnResponseThrowsInterceptor.*Expect the unexpected",
            ):
                await client.toLowerSnake("HelloWorld")

        await self.run_with_local_server_rocket(assert_basic)

        self.assertEqual(self.observer.on_response_throws, 1)
