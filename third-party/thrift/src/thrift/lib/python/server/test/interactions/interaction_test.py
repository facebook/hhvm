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
from collections.abc import AsyncIterator

from calculator.thrift_clients import Calculator
from calculator.thrift_types import NegativeError, Point
from thrift.py3.server import SocketAddress
from thrift.python.client import ClientType, get_client
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.server import ServiceInterface, ThriftServer

from .server_handlers import (
    CalculatorHandler,
    CONCURRENCY_OP_DELAY,
    INITIALIZED_COUNTER_BOOM_START,
    SlowCalculatorHandler,
)


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

    async def test_handshake_factory_with_initial_response(self) -> None:
        """Explicit factory that returns `tuple[Counter, CounterSnapshot]`. The
        client unpacks `(interaction, initial_response)`; the per-session Tile is
        the handler-returned instance, so its state derives from the factory
        argument (`start`)."""
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                counter, snapshot = await calc.initializedCounter(7)
                self.assertEqual(snapshot.value, 7)
                async with counter as c:
                    # Tile state derived from `start`, not the zero-arg default.
                    self.assertEqual(await c.get(), 7)
                    await c.add(3)
                    self.assertEqual(await c.get(), 10)

    async def test_factory_with_initial_response_isolation(self) -> None:
        # Two factory-built Counters with diverged start values keep independent
        # per-session state.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                c1, s1 = await calc.initializedCounter(100)
                c2, s2 = await calc.initializedCounter(5)
                self.assertEqual((s1.value, s2.value), (100, 5))
                async with c1 as a, c2 as b:
                    self.assertEqual(await a.get(), 100)
                    self.assertEqual(await b.get(), 5)
                    await a.add(1)
                    self.assertEqual(await a.get(), 101)
                    self.assertEqual(await b.get(), 5)

    async def test_factory_with_initial_response_declared_exception(self) -> None:
        # A *declared* exception raised by the factory-with-initial-response
        # handler surfaces to the client as that exception (not ApplicationError),
        # and no interaction Tile is installed.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                with self.assertRaises(NegativeError):
                    await calc.initializedCounter(-1)

    async def test_factory_with_initial_response_unexpected_exception(self) -> None:
        # An *undeclared* exception (RuntimeError) raised by the
        # factory-with-initial-response handler is translated to
        # ApplicationError(UNKNOWN); no Tile is installed.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                with self.assertRaises(ApplicationError) as cm:
                    await calc.initializedCounter(INITIALIZED_COUNTER_BOOM_START)
                self.assertEqual(cm.exception.type, ApplicationErrorType.UNKNOWN)
                self.assertIn(
                    "initializedCounter failed unexpectedly", cm.exception.message
                )

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

    async def test_stream_interaction_factory(self) -> None:
        # A stream factory that *creates* an interaction (`Counter, i32,
        # stream<i32> streamingCounter`). Regression guard for the codegen bug
        # where the factory-with-initial-response Tile-install block was emitted
        # for stream factories too: there `value` is still an unawaited coroutine
        # when the unpack runs, raising `TypeError: cannot unpack non-iterable
        # coroutine object`. The fix gates that block to the request/response
        # path; a stream factory's Tile comes from the zero-arg `createCounter`.
        async with _InProcessServer(CalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                counter, initial, stream = await calc.streamingCounter(3)
                # Initial response and stream are delivered (no TypeError).
                self.assertEqual(initial, 3)
                self.assertEqual([v async for v in stream], [0, 1, 2])
                # The created interaction Tile (zero-arg create) is usable and
                # starts from the default per-session state.
                async with counter as c:
                    self.assertEqual(await c.get(), 0)
                    await c.add(5)
                    self.assertEqual(await c.get(), 5)

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

    # -- async termination hook --

    async def test_termination_hook_fires_once_on_terminate(self) -> None:
        # Exiting the interaction's `async with` sends the terminate signal; the
        # server-side `onInteractionTermination` hook must run exactly once -- and
        # must not fire again when the connection later closes and the tile is
        # destroyed. The recorder awaits a real suspension point, so reaching the
        # count also proves the hook is driven to completion (not dropped after
        # the first await).
        handler = CalculatorHandler()
        async with _InProcessServer(handler) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as c:
                    await c.add(1)
                # interaction terminated here; wait (no fixed sleep) for the hook
                await handler.wait_for_counter_terminations(1)
                self.assertEqual(handler.counter_terminations, 1)
            # The connection is now closed and the tile destroyed. A buggy
            # destroy-path double-fire would bump the count; `wait_for` must
            # never observe a second termination.
            with self.assertRaises(asyncio.TimeoutError):
                await handler.wait_for_counter_terminations(2, timeout=0.5)
            self.assertEqual(handler.counter_terminations, 1)

    async def test_termination_hook_fires_per_interaction(self) -> None:
        # Each per-session interaction handler gets its own termination hook.
        handler = CalculatorHandler()
        async with _InProcessServer(handler) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as a:
                    await a.add(1)
                async with calc.createCounter() as b:
                    await b.add(2)
                # Wait (no fixed sleep) until both hooks have fired.
                await handler.wait_for_counter_terminations(2)
                self.assertEqual(handler.counter_terminations, 2)

    async def test_termination_hook_exception_is_swallowed(self) -> None:
        # A hook that raises must be logged-and-swallowed: the interaction still
        # tears down cleanly (no client-visible error, no hang on the awaited
        # terminate path) and a *subsequent* interaction's hook still fires -- one
        # raising hook must not wedge the loop or teardown. A traceback from the
        # swallowed error is expected on stderr.
        handler = CalculatorHandler(raise_in_hook=True)
        async with _InProcessServer(handler) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as a:
                    await a.add(1)
                async with calc.createCounter() as b:
                    await b.add(2)
                # Both hooks raised after recording; both are still observed and
                # the second interaction tore down despite the first one raising.
                await handler.wait_for_counter_terminations(2)
                self.assertEqual(handler.counter_terminations, 2)

    async def test_termination_hook_cancellation_does_not_wedge_loop(self) -> None:
        # If the termination coroutine is cancelled mid-hook, CancelledError (a
        # BaseException) bypasses `termination_coro`'s `except Exception`, but its
        # `finally` must still complete the C++ promise so `co_onTermination`
        # cannot hang -- and the event loop must stay healthy for later
        # interactions. Each hook here records, then cancels its own task.
        handler = CalculatorHandler(cancel_in_hook=True)
        async with _InProcessServer(handler) as addr:
            async with await self._client(addr) as calc:
                async with calc.createCounter() as a:
                    await a.add(1)
                await handler.wait_for_counter_terminations(1)
                # A subsequent interaction still terminates normally: the
                # cancelled task neither wedged the loop nor broke teardown.
                async with calc.createCounter() as b:
                    await b.add(2)
                await handler.wait_for_counter_terminations(2)
                self.assertEqual(handler.counter_terminations, 2)


class ConcurrentInteractionTest(unittest.IsolatedAsyncioTestCase):
    """Concurrent-dispatch coverage for the thrift-python *server* handler.

    Every test drives a ``SlowCalculatorHandler`` whose operations each take
    ``CONCURRENCY_OP_DELAY`` seconds, fires three of them in-flight at once
    against *separate* interactions via ``asyncio.gather``, and asserts the
    batch finishes in well under the serialized time. A serialized server would
    take ~3x the delay and fail ``_assert_concurrent``; a concurrent one
    finishes in ~one delay. Correctness (per-session isolation) is checked
    alongside timing so a test can't pass by collapsing the interactions.
    """

    # Concurrent dispatch should finish in ~one delay; allow generous slack for
    # scheduling/CI jitter while still being far below the serialized ~3x delay.
    _CONCURRENT_DEADLINE: float = CONCURRENCY_OP_DELAY * 2

    async def _client(self, addr: SocketAddress) -> Calculator.Async:
        return get_client(
            Calculator,
            host="::1",
            port=addr.port,
            client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        )

    def _assert_concurrent(self, elapsed: float) -> None:
        self.assertLess(
            elapsed,
            self._CONCURRENT_DEADLINE,
            f"batch took {elapsed:.2f}s (>= {self._CONCURRENT_DEADLINE:.2f}s): "
            "requests appear to be serialized, not dispatched concurrently",
        )

    async def test_concurrent_factory_creation(self) -> None:
        # Three explicit factory creations (no initial response) in flight at
        # once. Each `newCounter` sleeps server-side; concurrent dispatch keeps
        # the batch at ~one delay. The created interactions are then verified to
        # be independent, usable Tiles.
        async with _InProcessServer(SlowCalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                loop = asyncio.get_running_loop()
                start = loop.time()
                counters = await asyncio.gather(
                    calc.newCounter(), calc.newCounter(), calc.newCounter()
                )
                self._assert_concurrent(loop.time() - start)

                async with counters[0] as a, counters[1] as b, counters[2] as c:
                    # Fresh, independent state.
                    self.assertEqual(
                        await asyncio.gather(a.get(), b.get(), c.get()), [0, 0, 0]
                    )
                    await a.add(1)
                    self.assertEqual(
                        [await a.get(), await b.get(), await c.get()], [1, 0, 0]
                    )

    async def test_concurrent_factory_creation_with_initial_response(self) -> None:
        # Three factory-with-initial-response creations in flight at once.
        # Each `initializedCounter` sleeps server-side; concurrent dispatch keeps
        # the batch at ~one delay. The initial responses and per-session Tile
        # state must each derive from their own `start` argument (isolation).
        async with _InProcessServer(SlowCalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                loop = asyncio.get_running_loop()
                start = loop.time()
                results = await asyncio.gather(
                    calc.initializedCounter(10),
                    calc.initializedCounter(20),
                    calc.initializedCounter(30),
                )
                self._assert_concurrent(loop.time() - start)

                self.assertEqual([snap.value for _, snap in results], [10, 20, 30])
                (c1, _), (c2, _), (c3, _) = results
                async with c1 as a, c2 as b, c3 as c:
                    self.assertEqual(
                        await asyncio.gather(a.get(), b.get(), c.get()), [10, 20, 30]
                    )

    async def test_concurrent_requests_across_interactions(self) -> None:
        # Three separate interactions, then a concurrent request/response on
        # each. Each `addChecked` sleeps server-side; concurrent dispatch keeps
        # the batch at ~one delay. Distinct return values prove per-session state
        # isolation (each counter started at 0).
        async with _InProcessServer(SlowCalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with (
                    calc.createCounter() as c1,
                    calc.createCounter() as c2,
                    calc.createCounter() as c3,
                ):
                    loop = asyncio.get_running_loop()
                    start = loop.time()
                    results = await asyncio.gather(
                        c1.addChecked(11), c2.addChecked(22), c3.addChecked(33)
                    )
                    self._assert_concurrent(loop.time() - start)
                    self.assertEqual(results, [11, 22, 33])

    async def test_concurrent_streams_across_interactions(self) -> None:
        # Three separate interactions, then a concurrent stream request on
        # each. Each `ticks` sleeps before its initial response; concurrent
        # dispatch keeps the batch of stream openings at ~one delay. Each stream
        # must reflect its own per-session state (isolation).
        async with _InProcessServer(SlowCalculatorHandler()) as addr:
            async with await self._client(addr) as calc:
                async with (
                    calc.createCounter() as c1,
                    calc.createCounter() as c2,
                    calc.createCounter() as c3,
                ):
                    # Diverge per-session state first (also concurrently).
                    await asyncio.gather(c1.add(100), c2.add(200), c3.add(300))

                    loop = asyncio.get_running_loop()
                    start = loop.time()
                    (i1, s1), (i2, s2), (i3, s3) = await asyncio.gather(
                        c1.ticks(2), c2.ticks(2), c3.ticks(2)
                    )
                    self._assert_concurrent(loop.time() - start)
                    self.assertEqual([i1, i2, i3], [100, 200, 300])

                    async def collect(stream: AsyncIterator[int]) -> list[int]:
                        return [v async for v in stream]

                    r1, r2, r3 = await asyncio.gather(
                        collect(s1), collect(s2), collect(s3)
                    )
                    self.assertEqual(r1, [100, 101])
                    self.assertEqual(r2, [200, 201])
                    self.assertEqual(r3, [300, 301])
