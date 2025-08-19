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

from typing import AsyncGenerator, Awaitable, Callable, Tuple
from unittest import IsolatedAsyncioTestCase

from thrift.lib.python.test.test_server import TestServer

from thrift.python.client import ClientType, get_client
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.sink_service.thrift_clients import TestSinkService
from thrift.python.sink_service.thrift_services import TestSinkServiceInterface
from thrift.python.sink_service.thrift_types import (
    FinalException,
    MyException,
    SinkException,
)

from thrift.python.streaming.sink import ClientSink

AsyncIntGenerator = AsyncGenerator[int, None]
BoolSinkCallback = Callable[[AsyncIntGenerator], Awaitable[bool]]
IntSinkCallback = Callable[[AsyncIntGenerator], Awaitable[int]]


def local_server() -> TestServer:
    return TestServer(handler=SinkHandler(), ip="::1")


async def range_gen(begin: int, stop: int) -> AsyncGenerator[int, None]:
    for i in range(begin, stop):
        yield i


class SinkTests(IsolatedAsyncioTestCase):
    def test_sink_init(self) -> None:
        with self.assertRaisesRegex(
            RuntimeError, "Do not instantiate ClientSink from Python"
        ):
            ClientSink()

    async def test_sink_service_basic_request(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                self.assertEqual(await client.test(), 31)

    async def test_sink_service_range_simple(self) -> None:
        begin: int = 2
        stop: int = 5

        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                sink = await client.range_(begin, stop)
                self.assertIsInstance(sink, ClientSink)
                final_resp = await sink.sink(range_gen(begin, stop))
                self.assertEqual(sum(range(begin, stop)), final_resp)

    async def test_sink_service_range_throw(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                with self.assertRaises(ApplicationError) as e:
                    await client.rangeThrow(5, 1)
                self.assertEqual(
                    e.exception.message, "RuntimeError('fr must be less than to')"
                )
                self.assertEqual(e.exception.type, ApplicationErrorType.UNKNOWN)

    async def test_sink_service_range_final_throw(self) -> None:
        begin: int = 3
        stop: int = 7

        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                sink = await client.rangeFinalResponseThrow(begin, stop)
                self.assertIsInstance(sink, ClientSink)
                with self.assertRaises(ApplicationError) as e:
                    await sink.sink(range_gen(3, 7))
                self.assertEqual(
                    "apache::thrift::TApplicationException: RuntimeError('final response throw')",
                    e.exception.message,
                )
                self.assertEqual(e.exception.type, ApplicationErrorType.UNKNOWN)

    async def test_sink_service_range_early_response(self) -> None:
        begin: int = 5
        stop: int = 11
        early: int = 9

        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                sink = await client.rangeEarlyResponse(begin, stop, early)
                self.assertIsInstance(sink, ClientSink)
                final_resp = await sink.sink(range_gen(begin, stop))
                self.assertEqual(early, final_resp)

    async def test_unimplemented(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                with self.assertRaises(ApplicationError) as e:
                    await client.unimplemented()
                self.assertEqual(e.exception.type, ApplicationErrorType.UNKNOWN)
                self.assertEqual(
                    e.exception.message,
                    "NotImplementedError('async def unimplemented is not implemented')",
                )

    async def test_sink_service_initial_throw(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                with self.assertRaises(MyException) as e:
                    await client.initialThrow()
                self.assertEqual(e.exception.reason, "why not?")

    async def test_sink_service_sink_throw(self) -> None:
        async def raise_gen() -> AsyncIntGenerator:
            yield 3
            yield 7
            raise SinkException(reason="why not?")

        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                initial, sink = await client.sinkThrow()
                self.assertIs(initial, True)
                with self.assertRaises(SinkException) as e:
                    await sink.sink(raise_gen())
                self.assertEqual(
                    e.exception.reason,
                    "why not?",
                )

    async def test_sink_service_sink_final_throw(self) -> None:
        async def raise_gen() -> AsyncIntGenerator:
            yield 3
            yield 7

        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestSinkService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                sink = await client.sinkFinalThrow()
                with self.assertRaises(FinalException) as e:
                    await sink.sink(raise_gen())
                self.assertEqual(e.exception.reason, "I'm finished!")


class SinkHandler(TestSinkServiceInterface):
    async def test(self) -> int:
        return 31

    async def range_(self, fr: int, to: int) -> IntSinkCallback:
        async def callback(agen: AsyncIntGenerator) -> int:
            expected = fr
            total = 0
            async for item in agen:
                assert item == expected
                total += item
                expected += 1

            assert expected == to, "final count doesn't match `to`"

            return total

        return callback

    async def rangeThrow(self, fr: int, to: int) -> IntSinkCallback:
        print(f"rangeThrow({fr}, {to})")
        if to <= fr:
            raise RuntimeError("fr must be less than to")

        return await self.range_(fr, to)

    async def rangeEarlyResponse(self, fr: int, to: int, early: int) -> IntSinkCallback:
        async def callback(agen: AsyncIntGenerator) -> int:
            expected = fr
            async for item in agen:
                assert item == expected
                if item == early:
                    return early
                expected += 1

            return -1

        return callback

    async def rangeFinalResponseThrow(self, fr: int, to: int) -> BoolSinkCallback:
        range_cb: IntSinkCallback = await self.range_(fr, to)

        async def final_resp_throw(agen: AsyncIntGenerator) -> bool:
            await range_cb(agen)
            raise RuntimeError("final response throw")

        return final_resp_throw

    async def initialThrow(self) -> Tuple[bool, BoolSinkCallback]:
        raise MyException(reason="why not?")

    async def sinkThrow(self) -> Tuple[bool, BoolSinkCallback]:
        async def callback(agen: AsyncIntGenerator) -> bool:
            first = await agen.__anext__()
            assert first == 3
            second = await agen.__anext__()
            assert second == 7
            try:
                await agen.__anext__()
            except SinkException as e:
                assert e.reason == "why not?"

            return True

        return True, callback

    async def sinkFinalThrow(self) -> BoolSinkCallback:
        async def callback(agen: AsyncIntGenerator) -> bool:
            first = await agen.__anext__()
            assert first == 3
            second = await agen.__anext__()
            assert second == 7
            try:
                result = await agen.__anext__()
                assert result is object(), "generator should have stopped"
            except StopAsyncIteration:
                pass

            raise FinalException(reason="I'm finished!")

        return callback
