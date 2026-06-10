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

"""End-to-end test for thrift-python server-side interactions (request/response
and server streaming).

An in-process thrift-python server using the hand-written interaction handlers
is exercised by a real thrift-python client over Rocket. Sinks, bidi, and
lifecycle hooks are covered in later commits.
"""

from __future__ import annotations

import asyncio
import unittest

from calculator.thrift_clients import Calculator
from calculator.thrift_types import NegativeError, Point
from thrift.py3.server import SocketAddress
from thrift.python.client import ClientType, get_client
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.server import ServiceInterface, ThriftServer

from .server_handlers import CalculatorHandler


class _InProcessServer:
    def __init__(self, handler: ServiceInterface) -> None:
        self.server: ThriftServer = ThriftServer(handler, ip="::1")
        self.serve_task: asyncio.Task[None] | None = None

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_running_loop().create_task(self.server.serve())
        return await self.server.get_address()

    async def __aexit__(self, *_exc_info: object) -> None:
        self.server.stop()
        assert self.serve_task is not None
        await self.serve_task


class CalculatorInteractionTest(unittest.IsolatedAsyncioTestCase):
    async def _client(self, addr: SocketAddress) -> Calculator.Async:
        return get_client(
            Calculator,
            host="::1",
            port=addr.port,
            client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        )

    # -- baseline (non-interaction) --

    async def test_baseline_echo(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                self.assertEqual(await calc.echo(42), 42)

    # -- handshake / factory forms --

    async def test_handshake_terminate_unused(self) -> None:
        """Open an interaction via the implicit `performs` factory and tear it
        down without issuing any call."""
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as _:
                    pass

    async def test_handshake_explicit_factory(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                counter = await calc.newCounter()
                async with counter as c:
                    self.assertEqual(await c.get(), 0)

    async def test_handshake_heartbeat_no_request_no_response(self) -> None:
        """Request/response-less explicit factory on a dedicated interaction."""
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                heartbeat = await calc.newHeartbeat()
                async with heartbeat as h:
                    self.assertIsNone(await h.ping())

    # -- request/response inside an interaction --

    async def test_reqresp_performs(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.add(3)
                    await c.add(4)
                    self.assertEqual(await c.get(), 7)

    async def test_reqresp_struct(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.accumulatePoint(Point(x=2, y=3))
                    p = await c.getPoint()
                    self.assertEqual((p.x, p.y), (2, 3))

    async def test_reqresp_reset(self) -> None:
        # `reset()` mutates per-session state back to its initial value; verify
        # the change is observable on the same interaction afterwards.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.add(8)
                    await c.accumulatePoint(Point(x=1, y=1))
                    self.assertEqual(await c.get(), 8)
                    await c.reset()
                    self.assertEqual(await c.get(), 0)
                    p = await c.getPoint()
                    self.assertEqual((p.x, p.y), (0, 0))

    async def test_reqresp_oneway(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.noop()
                    self.assertEqual(await c.get(), 0)

    async def test_reqresp_throws(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    self.assertEqual(await c.addChecked(5), 5)
                    with self.assertRaises(NegativeError):
                        await c.addChecked(-1)

    async def test_factory_exception_propagates(self) -> None:
        # A throwing server-side interaction factory (`createBoom`) must surface
        # as a client error, not be swallowed into a generic failure or a hang.
        # Implicit-`performs` path (createInteractionImpl).
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createBoom() as boom:
                    with self.assertRaisesRegex(
                        ApplicationError, "boom: factory intentionally failed"
                    ):
                        await boom.noop()

    # -- server stream --

    async def test_baseline_service_stream(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                count, stream = await calc.serviceTicks(3)
                self.assertEqual(count, 3)
                got = [v async for v in stream]
                self.assertEqual(got, [0, 1, 2])

    async def test_stream_inside_interaction(self) -> None:
        # The initial response and stream both derive from per-session state, so
        # this fails unless the stream dispatches against this Counter instance.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.add(10)
                    initial, stream = await c.ticks(4)
                    self.assertEqual(initial, 10)
                    got = [v async for v in stream]
                    self.assertEqual(got, [10, 11, 12, 13])

    async def test_stream_no_first_response_inside_interaction(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.add(3)
                    stream = await c.drain()
                    got = [v async for v in stream]
                    self.assertEqual(got, [0, 1, 2])

    async def test_stream_raises_declared_exception_inside_interaction(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    count, stream = await c.ticksThenFail(2)
                    self.assertEqual(count, 2)
                    got = []
                    with self.assertRaises(NegativeError):
                        async for v in stream:
                            got.append(v)
                    self.assertEqual(got, [0, 1])

    async def test_stream_isolation_across_interactions(self) -> None:
        # Two concurrent Counters with diverged state; each interaction's stream
        # must reflect its own per-session handler instance.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c1, calc.createCounter() as c2:
                    await c1.add(100)
                    await c2.add(5)
                    i1, s1 = await c1.ticks(3)
                    i2, s2 = await c2.ticks(3)
                    self.assertEqual(i1, 100)
                    self.assertEqual(i2, 5)
                    self.assertEqual([v async for v in s1], [100, 101, 102])
                    self.assertEqual([v async for v in s2], [5, 6, 7])

    async def test_explicit_factory_exception_propagates(self) -> None:
        # Same as above but via the explicit factory method `newBoom`
        # (maybeFulfillTilePromise path): the real factory message must reach the
        # client (not a generic "HandlerCallback not completed"), matching the
        # implicit `performs` path.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                with self.assertRaisesRegex(
                    ApplicationError, "boom: factory intentionally failed"
                ):
                    async with await calc.newBoom() as boom:
                        await boom.noop()

    async def test_unexpected_handler_exception(self) -> None:
        # An *undeclared* exception from an inside-interaction handler surfaces
        # as ApplicationError(UNKNOWN).
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    with self.assertRaises(ApplicationError) as cm:
                        await c.raiseUnexpected()
                    self.assertEqual(cm.exception.type, ApplicationErrorType.UNKNOWN)
                    self.assertIn("unexpected handler failure", cm.exception.message)

    async def test_handler_application_error(self) -> None:
        # A handler raising ApplicationError directly from inside an interaction:
        # the handler-supplied type is *not* trusted -- it is coerced to UNKNOWN
        # so a handler cannot forge error codes (cf. ApplicationOverloadError).
        # The message is still delivered.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    with self.assertRaises(ApplicationError) as cm:
                        await c.raiseAppError()
                    self.assertEqual(cm.exception.type, ApplicationErrorType.UNKNOWN)
                    self.assertIn("deliberate app error", cm.exception.message)

    # -- isolation across interactions / sessions --

    async def test_isolation_within_session(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as a:
                    await a.add(1)
                    self.assertEqual(await a.get(), 1)
                async with calc.createCounter() as b:
                    self.assertEqual(await b.get(), 0)
                    await b.add(99)
                    self.assertEqual(await b.get(), 99)

    async def test_isolation_across_sessions(self) -> None:
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.add(7)
                    self.assertEqual(await c.get(), 7)
            # New connection; new server-side state.
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    self.assertEqual(await c.get(), 0)
