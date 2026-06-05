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

# pyre-strict

import asyncio
import unittest
from typing import AsyncIterator, Tuple

from thrift.lib.python.test.test_server import TestServer
from thrift.py3.server import get_context, RequestContext
from thrift.py3.test.included.included.thrift_types import Included
from thrift.py3.test.stream.thrift_clients import StreamTestService
from thrift.py3.test.stream.thrift_services import StreamTestServiceInterface
from thrift.py3.test.stream.thrift_types import FuncEx, StreamEx
from thrift.python.client import ClientType, get_client
from thrift.python.common import RpcOptions
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    ApplicationOverloadError,
)


class _TestStreamError(Exception):
    """Test-only exception used in generator tests to avoid matching
    framework-internal errors like RuntimeError."""


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


def local_server() -> TestServer:
    return TestServer(handler=Handler(), ip="::1")


class StreamClientTest(unittest.IsolatedAsyncioTestCase):
    async def test_return_stream(self) -> None:
        async with local_server() as sa:
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
        async with local_server() as sa:
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
        async with local_server() as sa:
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
        async with local_server() as sa:
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

    async def test_stream_application_overload_error(self) -> None:
        """
        ApplicationOverloadError raised from within a stream generator reaches
        the client as ApplicationError, but the type is NOT preserved -- it is
        coerced to UNKNOWN.

        This is a known limitation: unlike the unary handler path, stream/sink
        element errors do not round-trip the TApplicationException type over the
        stream protocol, so the trusted overload error code is lost. (Raising it
        is still handled cleanly server-side, i.e. not logged as an unexpected
        error.)
        """

        class OverloadStreamHandler(Handler):
            async def returnstream(
                self, i32_from: int, i32_to: int
            ) -> AsyncIterator[int]:
                yield i32_from
                raise ApplicationOverloadError("shedding load")

        async with TestServer(handler=OverloadStreamHandler(), ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(0, 10)
                self.assertEqual(await stream.__anext__(), 0)
                with self.assertRaises(ApplicationError) as ex:
                    async for _ in stream:  # noqa: F841
                        pass
                self.assertEqual(ex.exception.type, ApplicationErrorType.UNKNOWN)

    async def test_stream_cancel(self) -> None:
        async with local_server() as sa:
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
        async with local_server() as sa:
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
        async with local_server() as sa:
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

    async def test_stream_cancel_propagates_during_blocked_anext(self) -> None:
        """
        Stream cancellation must work when __anext__ is in-flight.
        When cancelAsyncGenerator calls aclose() on the server-side stream wrapper
        while its __anext__() is blocked waiting for the next item from the handler's
        generator, cancellation must propagate to the handler's generator.
        """
        generator_cleanup_event = asyncio.Event()
        generator_blocked_event = asyncio.Event()

        class BlockingStreamHandler(Handler):
            async def returnstream(
                self, i32_from: int, i32_to: int
            ) -> AsyncIterator[int]:
                try:
                    yield i32_from
                    generator_blocked_event.set()
                    # Block forever; cancellation must interrupt this
                    await asyncio.Future()
                    yield i32_to  # Should never reach here
                finally:
                    generator_cleanup_event.set()

        async with TestServer(handler=BlockingStreamHandler(), ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(0, 10)
                first = await stream.__anext__()
                self.assertEqual(first, 0)
                # Ensure server generator is blocked in __anext__.
                await asyncio.wait_for(generator_blocked_event.wait(), timeout=5.0)

            # Client disconnected; cancelAsyncGenerator fires on server side.
            await asyncio.wait_for(generator_cleanup_event.wait(), timeout=5.0)

    async def _assert_cancel_cleanup_despite_finally_error(
        self,
        async_sleep_in_finally: bool,
    ) -> None:
        """Generator finally block does cleanup then raises; cleanup must
        still happen and server must not crash."""
        generator_cleanup_event = asyncio.Event()
        generator_blocked_event = asyncio.Event()

        class FinallyErrorHandler(Handler):
            async def returnstream(
                self, i32_from: int, i32_to: int
            ) -> AsyncIterator[int]:
                try:
                    for i in range(i32_from, i32_to):
                        yield i
                        if i == i32_from + 2:
                            generator_blocked_event.set()
                            # Block forever; cancellation must interrupt this
                            await asyncio.Future()
                finally:
                    generator_cleanup_event.set()
                    if async_sleep_in_finally:
                        await asyncio.sleep(0)
                    raise _TestStreamError("bug in finally")

        async with TestServer(handler=FinallyErrorHandler(), ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(0, 100)
                results = []
                async for val in stream:
                    results.append(val)
                    if len(results) >= 3:
                        break
                self.assertEqual(results, [0, 1, 2])
                await asyncio.wait_for(generator_blocked_event.wait(), timeout=5.0)

            # Client disconnected; cancellation must trigger generator cleanup.
            await asyncio.wait_for(generator_cleanup_event.wait(), timeout=5.0)

    async def test_stream_cancel_cleanup_despite_finally_error(self) -> None:
        await self._assert_cancel_cleanup_despite_finally_error(
            async_sleep_in_finally=False
        )

    async def test_stream_cancel_cleanup_despite_async_finally_error(self) -> None:
        await self._assert_cancel_cleanup_despite_finally_error(
            async_sleep_in_finally=True
        )

    async def _assert_server_disconnect(self, yield_before_block: bool) -> None:
        """Server shutdown must unblock the client, not hang it.

        When yield_before_block is True, the handler yields one item then
        blocks — the server stops while the client waits for the *second*
        element. When False, the handler blocks immediately — the server
        stops before the client receives *any* element.
        """
        generator_blocked_event = asyncio.Event()

        class BlockingStreamHandler(Handler):
            async def returnstream(
                self, i32_from: int, i32_to: int
            ) -> AsyncIterator[int]:
                if yield_before_block:
                    yield i32_from
                generator_blocked_event.set()
                await asyncio.Future()
                yield i32_to  # never reached

        server = TestServer(handler=BlockingStreamHandler(), ip="::1")
        async with server as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(0, 10)
                if yield_before_block:
                    first = await stream.__anext__()
                    self.assertEqual(first, 0)
                # __anext__ will block waiting for the next (or first) element
                read_task = asyncio.ensure_future(stream.__anext__())
                await asyncio.wait_for(generator_blocked_event.wait(), timeout=5.0)
                # Shut down the server while the client is waiting
                server.server.stop()
                assert server.serve_task is not None
                await server.serve_task
                # Client should observe the disconnect, not hang
                try:
                    await asyncio.wait_for(read_task, timeout=5.0)
                    self.fail("Expected an error after server disconnect")
                except asyncio.TimeoutError:
                    self.fail("Client hung after server disconnect")
                except ApplicationError as ex:
                    self.assertEqual(ex.type, ApplicationErrorType.INTERRUPTION)
                    self.assertIn("cancelling stream", ex.message)

    async def test_server_disconnect_during_stream(self) -> None:
        await self._assert_server_disconnect(yield_before_block=True)

    async def test_server_disconnect_before_first_stream_element(self) -> None:
        await self._assert_server_disconnect(yield_before_block=False)

    async def test_stream_credit_timeout(self) -> None:
        """Client gets ApplicationErrorType.TIMEOUT when it stops consuming."""
        server = TestServer(handler=Handler(), ip="::1")
        server.server.set_stream_expire_time(0.1)
        async with server as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                options = RpcOptions()
                options.chunk_buffer_size = 1
                stream = await client.returnstream(0, 1000000, rpc_options=options)
                first = await stream.__anext__()
                self.assertEqual(first, 0)
                await asyncio.sleep(0.5)
                with self.assertRaises(ApplicationError) as ctx:
                    async for _ in stream:
                        pass
                self.assertEqual(ctx.exception.type, ApplicationErrorType.TIMEOUT)

    async def test_stream_request_context_invalid_after_request(self) -> None:
        """Accessing Cpp2RequestContext properties inside a stream generator
        after the initial request's context has been deallocated must raise
        RuntimeError, not dereference freed memory."""
        captured_ctx: RequestContext | None = None
        priority_error: BaseException | None = None
        generator_started: asyncio.Event = asyncio.Event()
        eviction_done: asyncio.Event = asyncio.Event()

        class ContextCheckHandler(Handler):
            async def returnstream(
                self, i32_from: int, i32_to: int
            ) -> AsyncIterator[int]:
                nonlocal captured_ctx
                captured_ctx = get_context()

                async def gen() -> AsyncIterator[int]:
                    nonlocal priority_error
                    generator_started.set()
                    await eviction_done.wait()
                    try:
                        _ = captured_ctx.priority
                    except RuntimeError as e:
                        priority_error = e
                    yield i32_from

                return gen()

        test_server = TestServer(handler=ContextCheckHandler(), ip="::1")
        test_server.server.set_io_worker_threads(1)
        async with test_server as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                StreamTestService,
                host=ip,
                port=port,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
            ) as client:
                stream = await client.returnstream(42, 100)
                await asyncio.wait_for(generator_started.wait(), timeout=5.0)
                for _ in range(11):
                    dummy = await client.stringstream()
                    async for _ in dummy:
                        pass
                eviction_done.set()
                results = [n async for n in stream]
                self.assertEqual(results, [42])

        assert captured_ctx is not None
        self.assertFalse(captured_ctx.is_valid())
        self.assertIsInstance(priority_error, RuntimeError)
