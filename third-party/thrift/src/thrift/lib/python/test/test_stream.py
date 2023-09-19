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

import asyncio
import unittest
from typing import AsyncIterator, Optional, Tuple

from thrift.py3.server import get_context, SocketAddress
from thrift.py3.test.included.included.thrift_types import Included
from thrift.py3.test.stream.thrift_clients import StreamTestService
from thrift.py3.test.stream.thrift_services import StreamTestServiceInterface
from thrift.py3.test.stream.thrift_types import FuncEx, StreamEx

from thrift.python.client import ClientType, get_client
from thrift.python.common import RpcOptions
from thrift.python.server import ServiceInterface, ThriftServer


class Handler(StreamTestServiceInterface):
    async def returnstream(self, i32_from: int, i32_to: int) -> AsyncIterator[int]:
        for i in range(i32_from, i32_to):
            yield i

    # Unfortunately, the fact that RequestContext only exists during the lifetime of
    # the original request means this pattern is necessary
    async def methodNameStream(self) -> AsyncIterator[str]:
        method_name: str = get_context().method_name

        async def gen() -> AsyncIterator[str]:
            for char in method_name:
                yield char

        return gen()

    async def methodStream(self, name: str) -> AsyncIterator[str]:
        for char in name:
            yield char

    async def alwaysThrows(self) -> AsyncIterator[int]:
        raise StreamEx()
        yield

    async def stringstream(self) -> AsyncIterator[str]:
        yield "hi"
        yield "hello"

    # Has to be set up this way because if there's a yield in
    # this function then it transforms the whole function into a generator
    async def streamthrows(self, t: bool) -> AsyncIterator[int]:
        if t:
            raise FuncEx()
        else:
            return self.alwaysThrows()

    async def returnresponseandstream(
        self,
        foo: Included,
    ) -> Tuple[Included, AsyncIterator[Included]]:
        resp = Included(from_=100, to=200)

        async def inner() -> AsyncIterator[Included]:
            for x in range(foo.from_, foo.to):
                yield Included(from_=foo.from_, to=x)

        return (resp, inner())


# pyre-fixme[13]: Attribute `serve_task` is never initialized.
class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task

    def __init__(
        self,
        ip: Optional[str] = None,
        handler: ServiceInterface = Handler(),  # noqa: B008
    ) -> None:
        self.server = ThriftServer(handler, ip=ip)

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    # pyre-fixme[2]: Parameter must be annotated.
    async def __aexit__(self, *exc_info) -> None:
        self.server.stop()
        await self.serve_task


class StreamClientTest(unittest.IsolatedAsyncioTestCase):
    async def test_return_stream(self) -> None:
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(10, 1024)
                res = [n async for n in stream]
                self.assertEqual(res, list(range(10, 1024)))

    async def test_method_name_stream(self) -> None:
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.methodNameStream()
                res = [n async for n in stream]
                self.assertEqual(res, list("methodNameStream"))

    async def test_stringstream(self) -> None:
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.stringstream()
                res = [n async for n in stream]
                self.assertEqual(res, ["hi", "hello"])

    async def test_stream_throws(self) -> None:
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                with self.assertRaises(FuncEx):
                    await client.streamthrows(True)
                stream = await client.streamthrows(False)
                with self.assertRaises(StreamEx):
                    async for _ in stream:  # noqa: F841 current flake8 version too old to support "async for _ in"
                        pass

    async def test_stream_cancel(self) -> None:
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                _ = await client.streamthrows(False)
                otherStream = await client.stringstream()
                async for val in otherStream:
                    self.assertEqual(val, "hi")
                    break
                thirdStream = await client.stringstream()
                res = [n async for n in thirdStream]
                self.assertEqual(res, ["hi", "hello"])

    async def test_return_response_and_stream(self) -> None:
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                resp, stream = await client.returnresponseandstream(
                    Included(from_=39, to=42)
                )
                self.assertEqual(resp, Included(from_=100, to=200))
                expected_to = 39
                async for n in stream:
                    self.assertEqual(n, Included(from_=39, to=expected_to))
                    expected_to += 1
                self.assertEqual(expected_to, 42)

    async def test_return_stream_set_buffer_size(self) -> None:
        options = RpcOptions()
        options.chunk_buffer_size = 64
        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(10, 1024, rpc_options=options)
                res = [n async for n in stream]
                self.assertEqual(res, list(range(10, 1024)))
