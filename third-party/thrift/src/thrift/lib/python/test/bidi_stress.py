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
from typing import AsyncGenerator, Callable, Generator
from unittest import IsolatedAsyncioTestCase

from thrift.lib.python.test.test_server import TestServer
from thrift.python.bidi_service.thrift_clients import TestBidiService
from thrift.python.bidi_service.thrift_services import TestBidiServiceInterface
from thrift.python.client import ClientType, get_client
from thrift.python.streaming.stream import ClientBufferedStream


class BidiStressHandler(TestBidiServiceInterface):
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


class BidiStressTest(IsolatedAsyncioTestCase):
    async def test_bidi_service_str_request_concurrently(self) -> None:
        async with TestServer(
            handler=BidiStressHandler(), ip="::1", queue_timeout=1.0
        ) as sa:
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

                    async def yield_strs() -> AsyncGenerator[str, None]:
                        for i in range(start, stop):
                            yield str(i)

                    (_, total_items) = await asyncio.gather(
                        client_sink.sink(yield_strs()),
                        self._assert_stream(client_stream, stringify_nums()),
                    )
                    self.assertEqual(total_items, stop - start)

                async def run_n_times() -> None:
                    for _ in range(100):
                        await run_single_bidi()

                await asyncio.gather(*[run_n_times() for _ in range(100)])

    async def _assert_stream(
        self,
        stream: ClientBufferedStream[str],
        expected_gen: Generator[str, None, None],
    ) -> int:
        count = 0
        async for item in stream:
            try:
                expected = next(expected_gen)
            except StopIteration:
                break
            self.assertEqual(item, expected)
            count += 1
        return count
