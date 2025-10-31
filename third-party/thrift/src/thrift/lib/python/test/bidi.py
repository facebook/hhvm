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

from typing import AsyncGenerator, Callable, Generator, Tuple, TypeVar
from unittest import IsolatedAsyncioTestCase

from thrift.lib.python.test.test_server import TestServer
from thrift.python.bidi_service.thrift_clients import TestBidiService
from thrift.python.bidi_service.thrift_services import TestBidiServiceInterface
from thrift.python.bidi_service.thrift_types import (
    FirstRequest,
    FirstResponse,
    SinkChunk,
    StreamChunk,
)

from thrift.python.client import ClientType, get_client

from thrift.python.streaming.bidistream import BidirectionalStream
from thrift.python.streaming.stream import ClientBufferedStream


def local_server() -> TestServer:
    return TestServer(handler=BidiHandler(), ip="::1")


async def yield_strs(
    start: int, stop: int, delay: float = 0.0
) -> AsyncGenerator[str, None]:
    for i in range(start, stop):
        await asyncio.sleep(delay)
        print(f"client yielding {i}")
        yield str(i)


async def yield_structs(
    start: int, stop: int, delay: float = 0.0
) -> AsyncGenerator[SinkChunk, None]:
    for i in range(start, stop):
        await asyncio.sleep(delay)
        yield SinkChunk(value=str(i))


StreamT = TypeVar("StreamT")


class BidiTests(IsolatedAsyncioTestCase):
    async def assert_stream(
        self,
        stream: ClientBufferedStream[StreamT],
        expected_gen: Generator[StreamT, None, None],
    ) -> int:
        count = 0
        async for item in stream:
            print(f"Client stream received {item} from server")
            self.assertEqual(item, next(expected_gen))
            count += 1

        return count

    def test_bidi_init(self) -> None:
        with self.assertRaisesRegex(
            RuntimeError, "Do not instantiate BidirectionalStream from Python"
        ):
            BidirectionalStream()

    def gen_int_strings(self, start: int) -> Generator[str, None, None]:
        i = 0
        for i in range(start, start + 3):
            yield str(i)

    async def test_bidi_service_str_request(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                bidi = await client.echo(0.0)
                client_sink, client_stream = bidi.sink, bidi.stream
                start: int = 1
                stop: int = 4

                def stringify_nums() -> Generator[str, None, None]:
                    for i in range(start, stop):
                        yield str(i)

                (_, total_items) = await asyncio.gather(
                    client_sink.sink(yield_strs(start, stop)),
                    self.assert_stream(client_stream, stringify_nums()),
                )
                self.assertEqual(total_items, stop - start)

    async def test_bidi_service_str_request_delay(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                # server waits 0.1 seconds before echoing back item
                bidi = await client.echo(0.1)
                client_sink, client_stream = bidi.sink, bidi.stream
                start: int = 1
                stop: int = 5

                def stringify_nums() -> Generator[str, None, None]:
                    for i in range(start, stop):
                        yield str(i)

                # use asyncio gather so client-side stream
                # receives the echos as they arrive, before sink is completed
                (_, total_items) = await asyncio.gather(
                    client_sink.sink(yield_strs(start, stop, 0.1)),
                    self.assert_stream(client_stream, stringify_nums()),
                )

                self.assertEqual(total_items, stop - start)

    async def test_bidi_service_unused_stream(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                bidi = await client.echo(0.0)
                # if we call method but don't start the sink we crash
                await bidi.sink.sink(yield_strs(1, 5, 0.1))

    async def test_bidi_service_partial_consume_stream(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                bidi = await client.echo(0.0)
                await bidi.sink.sink(yield_strs(1, 5, 0.1))

                async for item in bidi.stream:
                    self.assertEqual(item, "1")
                    break

    async def test_bidi_first_response(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                first_response, bidi = await client.echoWithResponse("start")
                self.assertEqual("start", first_response)
                client_sink = bidi.sink
                client_stream = bidi.stream

                # in this version, the sink exhausts before we pull anything off the stream
                await client_sink.sink(yield_strs(1, 3))
                i: int = 1
                async for item in client_stream:
                    self.assertEqual(item, str(i))
                    i += 1

                self.assertEqual(i, 3)

    async def test_bidi_cancellation(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                first_response, bidi = await client.echoWithResponse("start")
                self.assertEqual("start", first_response)
                client_sink = bidi.sink
                with self.assertRaises(asyncio.TimeoutError):
                    await asyncio.wait_for(client_sink.sink(yield_strs(1, 3, 0.1)), 0.2)
                # ensure no weird segfault
                await asyncio.sleep(1)

    async def test_bidi_different_datatype(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                # in this case, the server echoes at slower rate than client
                bidi = await client.intStream(0.15)
                client_sink, client_stream = bidi.sink, bidi.stream
                start: int = 1
                stop: int = 4

                def gen_nums() -> Generator[int, None, None]:
                    for i in range(start, stop):
                        yield i

                (_, total_items) = await asyncio.gather(
                    client_sink.sink(yield_strs(start, stop, 0.1)),
                    self.assert_stream(client_stream, gen_nums()),
                )
                self.assertEqual(total_items, stop - start)

    async def test_struct_bidi(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                first_response, bidi = await client.structBidi(
                    FirstRequest(value="start")
                )
                self.assertEqual("start", first_response.value)

                client_sink, client_stream = bidi.sink, bidi.stream
                start: int = 1
                stop: int = 4

                def stringify_value_field() -> Generator[StreamChunk, None, None]:
                    for i in range(start, stop):
                        yield StreamChunk(value=str(i))

                (_, total_items) = await asyncio.gather(
                    client_sink.sink(yield_structs(start, stop)),
                    self.assert_stream(client_stream, stringify_value_field()),
                )

                self.assertEqual(total_items, stop - start)


class BidiHandler(TestBidiServiceInterface):
    async def echo(
        self,
        serverDelay: float,
    ) -> Callable[
        [AsyncGenerator[str, None]],
        AsyncGenerator[str, None],
    ]:
        async def callback(
            agen: AsyncGenerator[str, None],
        ) -> AsyncGenerator[str, None]:
            async for item in agen:
                await asyncio.sleep(serverDelay)
                yield item

        return callback

    async def echoWithResponse(
        self, initial: str
    ) -> Tuple[
        str,
        Callable[
            [AsyncGenerator[str, None]],
            AsyncGenerator[str, None],
        ],
    ]:
        async def callback(
            agen: AsyncGenerator[str, None],
        ) -> AsyncGenerator[str, None]:
            async for item in agen:
                yield item

        return initial, callback

    async def intStream(
        self,
        serverDelay: float,
    ) -> Callable[
        [AsyncGenerator[str, None]],
        AsyncGenerator[int, None],
    ]:
        async def callback(
            agen: AsyncGenerator[str, None],
        ) -> AsyncGenerator[int, None]:
            async for item in agen:
                await asyncio.sleep(serverDelay)
                yield int(item)

        return callback

    async def structBidi(
        self, request: FirstRequest
    ) -> Tuple[
        FirstResponse,
        Callable[
            [AsyncGenerator[SinkChunk, None]],
            AsyncGenerator[StreamChunk, None],
        ],
    ]:
        async def callback(
            agen: AsyncGenerator[SinkChunk, None],
        ) -> AsyncGenerator[StreamChunk, None]:
            async for item in agen:
                yield StreamChunk(value=item.value)

        return FirstResponse(value=request.value), callback
