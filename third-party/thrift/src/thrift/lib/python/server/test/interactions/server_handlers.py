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

"""Server handlers for the thrift-python interaction test.

As of Commit 2 the compiler generates the ``<Interaction>Interface`` ABCs with
the wire-dispatch wrappers and the interaction routing in ``getFunctionTable``,
so the handlers below only implement business logic -- the hand-written
dispatch from Commit 1 is gone.

``create<Interaction>`` is the per-session Tile constructor the runtime invokes
(for both the implicit ``performs`` path and the explicit factory methods). The
explicit factory method bodies (``newCounter``/``newHeartbeat``) are no-ops
because the Tile is installed via ``create<Interaction>``.
"""

from __future__ import annotations

import asyncio
from collections.abc import AsyncGenerator, AsyncIterator, Awaitable, Callable

from calculator.thrift_services import (
    BoomInterface,
    CalculatorInterface,
    CounterInterface,
    HeartbeatInterface,
    SinkOnlyInterface,
)
from calculator.thrift_types import CounterSnapshot, NegativeError, Point
from thrift.python.exceptions import ApplicationError, ApplicationErrorType

# Sentinel `start` for `initializedCounter` that makes the factory raise an
# *undeclared* RuntimeError, exercising translation to ApplicationError(UNKNOWN)
# on the factory-with-initial-response path. A negative `start` instead raises
# the *declared* NegativeError.
INITIALIZED_COUNTER_BOOM_START: int = 0x7F37


class CounterHandler(CounterInterface):
    """Per-session interaction state (one instance per client interaction)."""

    def __init__(
        self,
        start: int = 0,
        on_terminate: Callable[[], Awaitable[None]] | None = None,
    ) -> None:
        self._value: int = start
        self._point: Point = Point(x=0, y=0)
        self._on_terminate = on_terminate

    async def onInteractionTermination(self) -> None:
        # Records into the owning service handler so the test can observe that
        # the termination hook fired. The recorder is a coroutine with a real
        # suspension point, so awaiting it here lets the tests verify the hook is
        # driven to completion (not dropped after first suspension).
        if self._on_terminate is not None:
            await self._on_terminate()

    async def add(self, n: int) -> None:
        self._value += n

    async def get(self) -> int:
        return self._value

    async def reset(self) -> None:
        self._value = 0
        self._point = Point(x=0, y=0)

    async def noop(self) -> None:
        pass

    async def getPoint(self) -> Point:
        return self._point

    async def accumulatePoint(self, p: Point) -> None:
        self._point = Point(x=self._point.x + p.x, y=self._point.y + p.y)

    async def addChecked(self, n: int) -> int:
        if n < 0:
            raise NegativeError(reason=f"refused negative value: {n}")
        self._value += n
        return self._value

    async def raiseUnexpected(self) -> None:
        raise RuntimeError("unexpected handler failure")

    async def raiseAppError(self) -> None:
        raise ApplicationError(
            ApplicationErrorType.INTERNAL_ERROR, "deliberate app error"
        )

    async def ticks(self, count: int) -> tuple[int, AsyncIterator[int]]:
        # Initial response and stream values both derive from per-session state,
        # so a stream bound to the wrong Counter instance would be observable.
        start: int = self._value

        async def gen() -> AsyncGenerator[int, None]:
            for i in range(count):
                yield start + i

        return start, gen()

    def drain(self) -> AsyncIterator[int]:
        # No-initial-response stream; yields 0..value-1 from per-session state.
        value: int = self._value

        async def gen() -> AsyncGenerator[int, None]:
            for i in range(value):
                yield i

        return gen()

    async def ticksThenFail(self, count: int) -> tuple[int, AsyncIterator[int]]:
        async def gen() -> AsyncGenerator[int, None]:
            for i in range(count):
                yield i
            raise NegativeError(reason="stream exhausted")

        return count, gen()


class HeartbeatHandler(HeartbeatInterface):
    async def ping(self) -> None:
        pass


class BoomHandler(BoomInterface):
    """Never actually constructed -- `createBoom` raises before this is used."""

    async def noop(self) -> None:
        pass


class SinkOnlyHandler(SinkOnlyInterface):
    """Subclasses the generated empty (`pass`) interface for the sink-only
    interaction. Its sole method is a sink, gated out of codegen, so there is
    nothing to implement -- the value is proving the empty interface is a valid,
    importable, subclassable base."""


class CalculatorHandler(CalculatorInterface):
    def __init__(
        self, raise_in_hook: bool = False, cancel_in_hook: bool = False
    ) -> None:
        # Termination-hook observability for the tests. `createCounter` wires
        # each per-session CounterHandler to bump `counter_terminations` from its
        # `onInteractionTermination`. A `Condition` lets tests wait for an exact
        # count without fixed sleeps.
        self.counter_terminations: int = 0
        self._terminations_changed: asyncio.Condition = asyncio.Condition()
        # When True, every hook raises *after* recording, to exercise the
        # log-and-swallow path: teardown must still complete and later
        # interactions must still fire their hooks.
        self._raise_in_hook: bool = raise_in_hook
        # When True, every hook cancels its own task *after* recording, to
        # exercise the CancelledError path (a BaseException, not caught by
        # `termination_coro`'s `except Exception`): the `finally` must still
        # complete the C++ promise so `co_onTermination` cannot hang, and the
        # loop must stay healthy for later interactions.
        self._cancel_in_hook: bool = cancel_in_hook

    async def _record_counter_termination(self) -> None:
        # A real suspension point before the count bump: a regression that ran
        # the hook fire-and-forget (dropping it after the first await) would
        # never reach the increment below, so `wait_for_counter_terminations`
        # would time out.
        await asyncio.sleep(0)
        async with self._terminations_changed:
            self.counter_terminations += 1
            self._terminations_changed.notify_all()
        if self._cancel_in_hook:
            # Cancel the task running this hook (the internal `termination_coro`
            # task), then yield so the CancelledError is delivered at an await
            # point inside it.
            current = asyncio.current_task()
            assert current is not None
            current.cancel()
            await asyncio.sleep(0)
        if self._raise_in_hook:
            raise RuntimeError("deliberate onInteractionTermination failure")

    async def wait_for_counter_terminations(self, n: int, timeout: float = 5) -> None:
        """Block until at least ``n`` interaction terminations have been recorded
        (or raise ``TimeoutError``). Condition-based, so it is robust to
        scheduling latency -- no fixed sleeps."""

        async def _wait() -> None:
            async with self._terminations_changed:
                await self._terminations_changed.wait_for(
                    lambda: self.counter_terminations >= n
                )

        await asyncio.wait_for(_wait(), timeout=timeout)

    async def echo(self, n: int) -> int:
        return n

    async def serviceTicks(self, count: int) -> tuple[int, AsyncIterator[int]]:
        async def gen() -> AsyncGenerator[int, None]:
            for i in range(count):
                yield i

        return count, gen()

    # Stream interaction factory (`Counter, i32, stream<i32> streamingCounter`).
    # The per-session `Counter` Tile comes from the zero-arg `createCounter`
    # (stream factories don't carry an initial-response Tile), so this handler
    # only returns the initial response and the stream. Exercises the codegen
    # path for a stream factory that creates an interaction.
    async def streamingCounter(self, count: int) -> tuple[int, AsyncIterator[int]]:
        async def gen() -> AsyncGenerator[int, None]:
            for i in range(count):
                yield i

        return count, gen()

    # Per-session Tile constructors invoked by the runtime.
    def createCounter(self) -> CounterHandler:
        return CounterHandler(on_terminate=self._record_counter_termination)

    def createHeartbeat(self) -> HeartbeatHandler:
        return HeartbeatHandler()

    def createSinkOnly(self) -> SinkOnlyHandler:
        return SinkOnlyHandler()

    # Intentionally-failing factory: exercises factory-exception propagation
    # (the client should observe an error, not a swallowed/generic failure).
    def createBoom(self) -> BoomHandler:
        raise RuntimeError("boom: factory intentionally failed")

    # Explicit wire factory *with* an initial response. Unlike `newCounter`
    # (whose Tile comes from the zero-arg `createCounter`), this handler builds
    # the Tile itself from the factory argument and returns it alongside the
    # initial response as `tuple[Counter, CounterSnapshot]`. The runtime installs
    # the returned Tile and sends the `CounterSnapshot`. Building the Tile here is
    # what lets the per-session state derive from `start` (the zero-arg factory
    # path cannot, since it never sees the arguments).
    async def initializedCounter(
        self, start: int
    ) -> tuple[CounterHandler, CounterSnapshot]:
        # Error propagation from the factory itself: a declared NegativeError
        # surfaces to the client as-is; an undeclared RuntimeError is translated
        # to ApplicationError(UNKNOWN). Neither installs a Tile.
        if start < 0:
            raise NegativeError(reason=f"refused negative start: {start}")
        if start == INITIALIZED_COUNTER_BOOM_START:
            raise RuntimeError("initializedCounter failed unexpectedly")
        counter = CounterHandler(
            start=start, on_terminate=self._record_counter_termination
        )
        return counter, CounterSnapshot(value=start)

    # Explicit wire factory methods; the Tile is installed via create*.
    async def newCounter(self) -> None:
        return None

    async def newHeartbeat(self) -> None:
        return None

    async def newBoom(self) -> None:
        return None
