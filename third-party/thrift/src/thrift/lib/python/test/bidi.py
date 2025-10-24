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

from typing import AsyncGenerator, Callable, Tuple
from unittest import IsolatedAsyncioTestCase

from thrift.lib.python.test.test_server import TestServer
from thrift.python.bidi_service.thrift_clients import TestBidiService
from thrift.python.bidi_service.thrift_services import TestBidiServiceInterface

from thrift.python.client import ClientType, get_client

from thrift.python.streaming.bidistream import BidirectionalStream


def local_server() -> TestServer:
    return TestServer(handler=BidiHandler(), ip="::1")


async def yield_strs(start: int, stop: int) -> AsyncGenerator[str, None]:
    for i in range(start, stop):
        yield str(i)


class BidiTests(IsolatedAsyncioTestCase):
    def test_bidi_init(self) -> None:
        with self.assertRaisesRegex(
            RuntimeError, "Do not instantiate BidirectionalStream from Python"
        ):
            BidirectionalStream()

    async def test_bidi_service_basic_request(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                TestBidiService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                bidi = await client.echo()
                client_sink = bidi.sink
                client_stream = bidi.stream
                await client_sink.sink(yield_strs(1, 3))
                i: int = 1
                async for item in client_stream:
                    self.assertEqual(item, str(i))
                    i += 1

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
                await client_sink.sink(yield_strs(1, 3))
                i: int = 1
                async for item in client_stream:
                    self.assertEqual(item, str(i))
                    i += 1


class BidiHandler(TestBidiServiceInterface):
    async def echo(
        self,
    ) -> Callable[
        [AsyncGenerator[str, None]],
        AsyncGenerator[str, None],
    ]:
        async def callback(
            agen: AsyncGenerator[str, None],
        ) -> AsyncGenerator[str, None]:
            async for item in agen:
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
