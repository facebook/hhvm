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
import sys
from typing import AsyncGenerator, Callable, Generator, Tuple, TypeVar
from unittest import IsolatedAsyncioTestCase

from parameterized import parameterized
from thrift.lib.python.test.test_server import TestServer
from thrift.python.bidi_service.thrift_clients import TestBidiService
from thrift.python.bidi_service.thrift_services import TestBidiServiceInterface
from thrift.python.bidi_service.thrift_types import (
    FirstRequest,
    FirstResponse,
    MethodException,
    SinkChunk,
    SinkException,
    StreamChunk,
    StreamException,
    ThrowWhere,
)
from thrift.python.client import ClientType, get_client
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.streaming.bidistream import BidirectionalStream
from thrift.python.streaming.stream import ClientBufferedStream


def local_server() -> TestServer:
    return TestServer(handler=BidiHandler(), ip="::1")


async def yield_ints(
    start: int, stop: int, delay: float = 0.0
) -> AsyncGenerator[int, None]:
    for i in range(start, stop):
        await asyncio.sleep(delay)
        yield i


async def yield_ints_throw(
    start: int,
    throw: int,
    throw_expected: bool,
    delay: float = 0.0,
) -> AsyncGenerator[int, None]:
    if throw < start:
        msg = "throw before start"
        raise (
            SinkException(message=msg)
            if throw_expected
            else RuntimeError("unexpected " + msg)
        )
    async for i in yield_ints(start, throw + 1, delay):
        if i == throw:
            print("About to throw from sink")
            msg = f"throw at integer {i}"
            raise (
                SinkException(message=msg)
                if throw_expected
                else RuntimeError("unexpected " + msg)
            )
        yield i


async def yield_strs(
    start: int, stop: int, delay: float = 0.0
) -> AsyncGenerator[str, None]:
    async for i in yield_ints(start, stop, delay):
        yield str(i)


async def yield_structs(
    start: int, stop: int, delay: float = 0.0
) -> AsyncGenerator[SinkChunk, None]:
    async for i in yield_ints(start, stop, delay):
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
            try:
                expected = next(expected_gen)
            except StopIteration:
                break
            self.assertEqual(item, expected)
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

    async def test_bidi_service_str_request_concurrently(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                client_: TestBidiService.Async = client

                async def run_single_bidi() -> None:
                    bidi = await client_.echo(0.0)
                    client_sink, client_stream = bidi.sink, bidi.stream
                    start: int = 1
                    stop: int = 5

                    def stringify_nums() -> Generator[str, None, None]:
                        for i in range(start, stop):
                            yield str(i)

                    (_, total_items) = await asyncio.gather(
                        client_sink.sink(yield_strs(start, stop)),
                        self.assert_stream(client_stream, stringify_nums()),
                    )
                    self.assertEqual(total_items, stop - start)

                async def run_n_times() -> None:
                    for _ in range(100):
                        await run_single_bidi()

                await asyncio.gather(*[run_n_times() for _ in range(100)])

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

    async def test_bidi_service_ignore_return(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                await client.echo(0.0)

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
                await bidi.sink.sink(yield_strs(1, 5, 0.1))

    async def test_bidi_service_stream_without_sink(self) -> None:
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

                def empty_generator() -> Generator[str, None, None]:
                    yield ""

                expected_err = (
                    TimeoutError
                    if sys.version_info.minor >= 12
                    else asyncio.exceptions.TimeoutError  # remove this when we drop python 3.10
                )
                with self.assertRaises(expected_err):
                    await asyncio.wait_for(
                        self.assert_stream(bidi.stream, empty_generator()), 0.2
                    )

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

    @parameterized.expand([(True,), (False,)])
    async def test_bidi_server_first_response_throws(
        self, expected_throw: bool
    ) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                base_msg = "first response throws"
                expected_ex_cls, msg = (
                    (MethodException, base_msg)
                    if expected_throw
                    else (ApplicationError, "unexpected " + base_msg)
                )
                with self.assertRaisesRegex(expected_ex_cls, msg):
                    await client.canThrow(ThrowWhere.FIRST_RESPONSE, expected_throw)

    @parameterized.expand(
        [
            (
                ThrowWhere.STREAM_BEFORE_FIRST_CHUNK,
                True,
                "stream throws before consume sink",
            ),
            (
                ThrowWhere.STREAM_BEFORE_FIRST_CHUNK,
                False,
                "unexpected stream throws before consume sink",
            ),
            (
                ThrowWhere.STREAM_AFTER_FIRST_CHUNK,
                True,
                "stream throws after yielding 1 from sink",
            ),
            (
                ThrowWhere.STREAM_AFTER_FIRST_CHUNK,
                False,
                "unexpected stream throws after yielding 1 from sink",
            ),
        ]
    )
    async def test_bidi_server_stream_throws(
        self, where: ThrowWhere, throw_expected: bool, expected_msg: str
    ) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                bidi = await client.canThrow(where, throw_expected)
                await bidi.sink.sink(yield_ints(1, 3))

                expected_ex_cls = (
                    StreamException if throw_expected else ApplicationError
                )
                with self.assertRaisesRegex(expected_ex_cls, expected_msg):
                    await self.assert_stream(bidi.stream, (i for i in range(1, 3)))

    @parameterized.expand(
        [
            (0, True, "throw before start"),
            (0, False, "unexpected throw before start"),
            (3, True, "throw at integer 3"),
            (3, False, "unexpected throw at integer 3"),
        ]
    )
    async def test_bidi_server_sink_throws(
        self, throw_at: int, throw_expected: bool, expected_msg: str
    ) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                bidi = await client.canThrow(ThrowWhere.FROM_SINK, throw_expected)

                start = 1
                (sink_ex, stream_ex) = await asyncio.gather(
                    bidi.sink.sink(
                        yield_ints_throw(start, throw_at, throw_expected=throw_expected)
                    ),
                    self.assert_stream(
                        bidi.stream, (i for i in range(start, throw_at))
                    ),
                    return_exceptions=True,
                )

                expected_sink_ex_cls = (
                    SinkException if throw_expected else ApplicationError
                )
                assert isinstance(sink_ex, Exception)  # ffs pyre
                with self.assertRaisesRegex(expected_sink_ex_cls, expected_msg):
                    raise sink_ex

                # if we don't have any delays, there's a race where the sink closes, meaning the
                # sink async generator in the server callback stops. In this case the handler will return 0.
                # Most of the time, the server gets the sink exception and reflects it back as ApplicationError
                # before the stream python handler code returns without exception.

                # NOTE: this only happens on Python 3.10, in local stress runs
                if stream_ex == 0 and sys.version_info.minor < 12:
                    return

                self.assertIsInstance(stream_ex, ApplicationError)
                assert isinstance(stream_ex, ApplicationError)  # ffs pyre
                # both the `sink.sink` method and stream will throw.
                # the "correct" type for the stream throw is ambiguous
                # ApplicationError is fine for now
                self.assertEqual(stream_ex.type, ApplicationErrorType.UNKNOWN)
                self.assertRegex(
                    stream_ex.message,
                    f"apache::thrift::TApplicationException: .*'{expected_msg}'",
                )


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

    async def canThrow(
        self,
        where: ThrowWhere,
        expected_throw: bool,
    ) -> Callable[
        [AsyncGenerator[int, None]],
        AsyncGenerator[int, None],
    ]:
        if where == ThrowWhere.FIRST_RESPONSE:
            msg = "first response throws"
            raise (
                MethodException(message=msg)
                if expected_throw
                else RuntimeError("unexpected " + msg)
            )

        async def callback(
            agen: AsyncGenerator[int, None],
        ) -> AsyncGenerator[int, None]:
            if where == ThrowWhere.STREAM_BEFORE_FIRST_CHUNK:
                msg = "stream throws before consume sink"
                raise (
                    StreamException(message=msg)
                    if expected_throw
                    else RuntimeError("unexpected " + msg)
                )
            async for i in agen:
                yield i
                if where == ThrowWhere.STREAM_AFTER_FIRST_CHUNK:
                    msg = f"stream throws after yielding {i} from sink"
                    raise (
                        StreamException(message=msg)
                        if expected_throw
                        else RuntimeError("unexpected " + msg)
                    )

        return callback
