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
import contextlib
import tempfile
import time
import typing
from multiprocessing import Event, Process, synchronize

from thrift.py3.server import get_context, SocketAddress, ThriftServer
from thrift.python.leaf.services import LeafServiceInterface
from thrift.python.test.services import EchoServiceInterface, TestServiceInterface
from thrift.python.test.types import ArithmeticException, EmptyException, SimpleResponse


class TestServiceHandler(TestServiceInterface):
    async def add(self, num1: int, num2: int) -> int:
        return num1 + num2

    async def divide(self, dividend: float, divisor: float) -> float:
        try:
            return dividend / divisor
        except ZeroDivisionError as e:
            raise ArithmeticException(msg=str(e))

    async def noop(self) -> None:
        pass

    async def oops(self) -> None:
        raise EmptyException()

    async def oneway(self) -> None:
        pass

    async def surprise(self) -> None:
        raise ValueError("Surprise!")

    async def readHeader(self, key: str) -> str:
        return get_context().read_headers.get(key, "")

    async def nums(self, f: int, t: int) -> typing.AsyncGenerator[SimpleResponse, None]:
        if f > t:
            raise ArithmeticException(msg="from outside of stream")

        async def gen() -> typing.AsyncGenerator[SimpleResponse, None]:
            for i in range(f, min(t + 1, 11)):
                yield SimpleResponse(value=f"{i}")
            if f < 0:
                raise ValueError("from is negative")
            elif t > 10:
                raise ArithmeticException(msg="from inside of stream")

        return gen()

    async def sumAndNums(
        self, f: int, t: int
    ) -> typing.Tuple[int, typing.AsyncGenerator[SimpleResponse, None]]:
        if f > t:
            raise ArithmeticException(msg="from outside of stream")

        async def gen() -> typing.AsyncGenerator[SimpleResponse, None]:
            for i in range(f, t + 1):
                yield SimpleResponse(value=f"{i}")

        return (f + t) * (t - f + 1) // 2, gen()


class EchoServiceHandler(TestServiceHandler, EchoServiceInterface):
    async def echo(self, input: str) -> str:
        return input


class LeafServiceHandler(EchoServiceHandler, LeafServiceInterface):
    async def reverse(self, input: typing.Sequence[int]) -> typing.Sequence[int]:
        return list(reversed(input))


@contextlib.contextmanager
def server_in_another_process() -> typing.Generator[str, None, None]:
    async def start_server_async(
        path: str,
        server_ready_event: synchronize.Event,
        test_done_event: synchronize.Event,
    ) -> None:
        server = ThriftServer(LeafServiceHandler(), path=path)
        task = asyncio.create_task(server.serve())
        await server.get_address()
        server_ready_event.set()
        while not test_done_event.is_set():
            await asyncio.sleep(0)
        server.stop()
        await task

    def start_server(
        path: str,
        server_ready_event: synchronize.Event,
        test_done_event: synchronize.Event,
    ) -> None:
        asyncio.run(start_server_async(path, server_ready_event, test_done_event))

    with tempfile.NamedTemporaryFile() as socket:
        socket.close()
        server_ready_event = Event()
        test_done_event = Event()
        process = Process(
            target=start_server, args=(socket.name, server_ready_event, test_done_event)
        )
        process.start()
        server_ready_event.wait()
        try:
            yield socket.name
        finally:
            test_done_event.set()
            process.join()


@contextlib.asynccontextmanager
async def server_in_event_loop() -> typing.AsyncGenerator[SocketAddress, None]:
    server = ThriftServer(LeafServiceHandler(), ip="::1")
    serve_task = asyncio.get_event_loop().create_task(server.serve())
    addr = await server.get_address()
    try:
        yield addr
    finally:
        server.stop()
        await serve_task
