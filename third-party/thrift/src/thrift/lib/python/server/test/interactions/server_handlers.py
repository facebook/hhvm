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

from collections.abc import AsyncGenerator, AsyncIterator

from calculator.thrift_services import (
    BoomInterface,
    CalculatorInterface,
    CounterInterface,
    HeartbeatInterface,
    SinkOnlyInterface,
)
from calculator.thrift_types import NegativeError, Point
from thrift.python.exceptions import ApplicationError, ApplicationErrorType


class CounterHandler(CounterInterface):
    """Per-session interaction state (one instance per client interaction)."""

    def __init__(self, start: int = 0) -> None:
        self._value: int = start
        self._point: Point = Point(x=0, y=0)

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
    async def echo(self, n: int) -> int:
        return n

    async def serviceTicks(self, count: int) -> tuple[int, AsyncIterator[int]]:
        async def gen() -> AsyncGenerator[int, None]:
            for i in range(count):
                yield i

        return count, gen()

    # Per-session Tile constructors invoked by the runtime.
    def createCounter(self) -> CounterHandler:
        return CounterHandler()

    def createHeartbeat(self) -> HeartbeatHandler:
        return HeartbeatHandler()

    def createSinkOnly(self) -> SinkOnlyHandler:
        return SinkOnlyHandler()

    # Intentionally-failing factory: exercises factory-exception propagation
    # (the client should observe an error, not a swallowed/generic failure).
    def createBoom(self) -> BoomHandler:
        raise RuntimeError("boom: factory intentionally failed")

    # Explicit wire factory methods; the Tile is installed via create*.
    async def newCounter(self) -> None:
        return None

    async def newHeartbeat(self) -> None:
        return None

    async def newBoom(self) -> None:
        return None
