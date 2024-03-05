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

import asyncio
import typing

from unittest import IsolatedAsyncioTestCase

from thrift.lib.python.client.test.event_handler_helper import (
    client_handler_that_throws,
)

from thrift.python.client import (
    ClientType,
    get_client,
    get_proxy_factory,
    install_proxy_factory,
)
from thrift.python.client.async_client import AsyncClient
from thrift.python.common import RpcOptions
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    TransportError,
    TransportErrorType,
)
from thrift.python.leaf.thrift_clients import LeafService
from thrift.python.serializer import Protocol
from thrift.python.test.test_server import server_in_event_loop
from thrift.python.test.thrift_clients import EchoService, TestService
from thrift.python.test.thrift_types import (
    ArithmeticException,
    EmptyException,
    SimpleResponse,
)

from .exceptions_helper import HijackTestException, HijackTestHelper


TEST_HEADER_KEY = "headerKey"
TEST_HEADER_VALUE = "headerValue"


class ThriftClientTestProxy:
    inner: AsyncClient

    def __init__(self, inner: AsyncClient) -> None:
        self.inner = inner


def test_proxy_factory(
    client_class: typing.Type[AsyncClient],
) -> typing.Callable[[AsyncClient], ...]:
    return ThriftClientTestProxy


class AsyncClientTests(IsolatedAsyncioTestCase):
    async def test_basic(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                sum = await client.add(1, 2)
                self.assertEqual(3, sum)

    async def test_client_type_and_protocol(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host=addr.ip,
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
                protocol=Protocol.BINARY,
            ) as client:
                sum = await client.add(1, 2)
                self.assertEqual(3, sum)

    async def test_void_return(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                res = await client.noop()
                self.assertIsNone(res)

    async def test_exception(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                res = await client.divide(6, 3)
                self.assertAlmostEqual(2, res)
                with self.assertRaises(ArithmeticException):
                    await client.divide(1, 0)

    async def test_void_return_with_exception(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                with self.assertRaises(EmptyException):
                    await client.oops()

    async def test_oneway(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                res = await client.oneway()
                self.assertIsNone(res)
                await asyncio.sleep(1)  # wait for server to clear the queue

    async def test_oneway_with_rocket(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host=addr.ip,
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                res = await client.oneway()
                self.assertIsNone(res)
                await asyncio.sleep(1)  # wait for server to clear the queue

    async def test_unexpected_exception(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                with self.assertRaises(ApplicationError) as ex:
                    await client.surprise()
                self.assertEqual(ex.exception.message, "ValueError('Surprise!')")
                self.assertEqual(ex.exception.type, ApplicationErrorType.UNKNOWN)

    async def test_derived_service(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(EchoService, host=addr.ip, port=addr.port) as client:
                out = await client.echo("hello")
                self.assertEqual("hello", out)
                sum = await client.add(1, 2)
                self.assertEqual(3, sum)

    async def test_deriving_from_external_service(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(LeafService, host=addr.ip, port=addr.port) as client:
                rev = await client.reverse([1, 2, 3])
                self.assertEqual([3, 2, 1], list(rev))
                out = await client.echo("hello")
                self.assertEqual("hello", out)
                sum = await client.add(1, 2)
                self.assertEqual(3, sum)

    async def test_transport_error(self) -> None:
        async with get_client(TestService, path="/no/where") as client:
            with self.assertRaises(TransportError) as ex:
                await client.add(1, 2)
            self.assertEqual(TransportErrorType.UNKNOWN, ex.exception.type)

    async def test_hostname(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService, host="localhost", port=addr.port
            ) as client:
                sum = await client.add(1, 2)
                self.assertEqual(3, sum)

    async def test_persistent_header(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService, host="localhost", port=addr.port
            ) as client:
                client.set_persistent_header(TEST_HEADER_KEY, TEST_HEADER_VALUE)
                value = await client.readHeader(TEST_HEADER_KEY)
                self.assertEqual(TEST_HEADER_VALUE, value)

    async def test_stream_nums(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host="localhost",
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.nums(2, 4)
                result = []
                async for num in stream:
                    result.append(num)
                self.assertEqual(
                    result, [SimpleResponse(value=f"{i}") for i in (2, 3, 4)]
                )

    async def test_stream_nums_throws_inside(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host="localhost",
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.nums(8, 11)
                result = []
                with self.assertRaises(ArithmeticException) as e:
                    async for num in stream:
                        result.append(num)
                self.assertEqual(
                    result, [SimpleResponse(value=f"{i}") for i in (8, 9, 10)]
                )
                self.assertEqual(e.exception.msg, "from inside of stream")

    async def test_stream_nums_throws_undeclared(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host="localhost",
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.nums(-1, 2)
                result = []
                with self.assertRaises(ApplicationError) as e:
                    async for num in stream:
                        result.append(num)
                self.assertEqual(
                    result, [SimpleResponse(value=f"{i}") for i in (-1, 0, 1, 2)]
                )
                self.assertEqual(
                    e.exception.message,
                    "apache::thrift::TApplicationException: ValueError('from is negative')",
                )

    async def test_stream_nums_throws_outside(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host="localhost",
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                with self.assertRaises(ArithmeticException) as e:
                    await client.nums(4, 2)
                self.assertEqual(e.exception.msg, "from outside of stream")

    async def test_stream_sumAndNums(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host="localhost",
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                resp, stream = await client.sumAndNums(2, 4)
                self.assertEqual(resp, 9)
                result = []
                async for num in stream:
                    result.append(num)
                self.assertEqual(
                    result, [SimpleResponse(value=f"{i}") for i in (2, 3, 4)]
                )

    async def test_stream_sumAndNums_throws(self) -> None:
        async with server_in_event_loop() as addr:
            async with get_client(
                TestService,
                host="localhost",
                port=addr.port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                with self.assertRaises(ArithmeticException) as e:
                    await client.nums(4, 2)
                self.assertEqual(e.exception.msg, "from outside of stream")

    async def test_proxy_factory(self) -> None:
        # Should be empty before we assign it
        self.assertEqual(get_proxy_factory(), None)

        # Should be able to assign/get a test factory
        install_proxy_factory(test_proxy_factory)
        self.assertEqual(get_proxy_factory(), test_proxy_factory)
        async with server_in_event_loop() as addr:
            self.assertIsInstance(
                get_client(TestService, host=addr.ip, port=addr.port),
                ThriftClientTestProxy,
            )
        # Should be able to unhook a factory
        install_proxy_factory(None)
        self.assertEqual(get_proxy_factory(), None)

    async def test_add_test_handler_with_rpc_options_should_hijack_transport_error_and_use_rpc_options(
        self,
    ) -> None:
        with HijackTestHelper():
            async with get_client(TestService, path="/no/where") as client:
                with self.assertRaises(HijackTestException) as context:
                    options = RpcOptions()
                    options.timeout = 12.5
                    await client.add(1, 2, rpc_options=options)
                self.assertEqual(context.exception.timeout, 12.5)

    async def test_add_test_handler_without_rpc_options_should_hijack_transport_error(
        self,
    ) -> None:
        with HijackTestHelper():
            async with get_client(TestService, path="/no/where") as client:
                with self.assertRaises(HijackTestException) as context:
                    await client.add(1, 2)
                self.assertEqual(context.exception.timeout, 0.0)

    async def test_exit_callback(self) -> None:
        class Callback:
            def __init__(self):
                self.triggered = False

            def trigger(self):
                self.triggered = True

            async def async_trigger(self):
                self.triggered = True

        cb1 = Callback()
        cb2 = Callback()

        async with server_in_event_loop() as addr:
            async with get_client(TestService, host=addr.ip, port=addr.port) as client:
                client._at_aexit(cb1.trigger)
                client._at_aexit(cb2.async_trigger)

        self.assertTrue(cb1.triggered)
        self.assertTrue(cb2.triggered)

    async def test_exception_in_client_event_handler(self) -> None:
        async with server_in_event_loop() as addr:
            with self.assertRaises(RuntimeError):
                with client_handler_that_throws():
                    async with get_client(
                        TestService, host=addr.ip, port=addr.port
                    ) as client:
                        await client.add(1, 2)
