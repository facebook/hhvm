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

"""Hand-written server handlers for the thrift-python interaction runtime test.

Commit 1 ships the interaction *runtime* without compiler support, so the
wire-dispatch wrappers (`_fbthrift__handler_<method>`) and the interaction
entries in `getFunctionTable()` are written by hand here. Commit 2 makes the
compiler generate all of this (an `<Interaction>Interface` ABC), at which point
this file collapses to just the business-logic methods.

The hand-written dispatch mirrors exactly what the service-side codegen already
emits for ordinary methods: deserialize the generated args struct, call the
business method, serialize the generated result struct.
"""

from __future__ import annotations

from collections.abc import Callable

import folly.iobuf as _fbthrift_iobuf
from calculator import thrift_types as _types
from calculator.thrift_services import CalculatorInterface
from calculator.thrift_types import NegativeError, Point
from thrift.python.exceptions import ApplicationError, ApplicationErrorType
from thrift.python.protocol import RpcKind
from thrift.python.serializer import deserialize, Protocol, serialize_iobuf
from thrift.python.server import FunctionEntry, Interaction, PythonUserException


class CounterHandler(Interaction):
    """Per-interaction state. One instance lives per client interaction session
    (the C++ ``PythonTile`` owns it). Subclasses ``Interaction`` directly because
    Commit 1 has no generated ``CounterInterface`` yet."""

    def __init__(self, start: int = 0) -> None:
        self._value: int = start
        self._point: Point = Point(x=0, y=0)

    # -- business logic --

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

    # -- hand-written wire dispatch (Commit 2 will codegen these) --

    async def _fbthrift__handler_add(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        args_struct = deserialize(_types._fbthrift_Counter_add_args, args, protocol)
        await self.add(args_struct.n)
        return serialize_iobuf(_types._fbthrift_Counter_add_result(), protocol)

    async def _fbthrift__handler_get(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        deserialize(_types._fbthrift_Counter_get_args, args, protocol)
        value = await self.get()
        return serialize_iobuf(
            _types._fbthrift_Counter_get_result(success=value), protocol
        )

    async def _fbthrift__handler_reset(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        deserialize(_types._fbthrift_Counter_reset_args, args, protocol)
        await self.reset()
        return serialize_iobuf(_types._fbthrift_Counter_reset_result(), protocol)

    async def _fbthrift__handler_noop(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> None:
        deserialize(_types._fbthrift_Counter_noop_args, args, protocol)
        await self.noop()
        # oneway: no response

    async def _fbthrift__handler_getPoint(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        deserialize(_types._fbthrift_Counter_getPoint_args, args, protocol)
        value = await self.getPoint()
        return serialize_iobuf(
            _types._fbthrift_Counter_getPoint_result(success=value), protocol
        )

    async def _fbthrift__handler_accumulatePoint(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        args_struct = deserialize(
            _types._fbthrift_Counter_accumulatePoint_args, args, protocol
        )
        await self.accumulatePoint(args_struct.p)
        return serialize_iobuf(
            _types._fbthrift_Counter_accumulatePoint_result(), protocol
        )

    async def _fbthrift__handler_addChecked(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        args_struct = deserialize(
            _types._fbthrift_Counter_addChecked_args, args, protocol
        )
        try:
            value = await self.addChecked(args_struct.n)
            return_struct = _types._fbthrift_Counter_addChecked_result(success=value)
        except NegativeError as e:
            return_struct = _types._fbthrift_Counter_addChecked_result(_ex0__err=e)
            buf = serialize_iobuf(return_struct, protocol)
            raise PythonUserException("NegativeError", str(e), buf)
        return serialize_iobuf(return_struct, protocol)

    async def _fbthrift__handler_raiseUnexpected(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        # No declared exceptions: an undeclared raise propagates to the runtime,
        # which reports it as ApplicationError(UNKNOWN).
        deserialize(_types._fbthrift_Counter_raiseUnexpected_args, args, protocol)
        await self.raiseUnexpected()
        return serialize_iobuf(
            _types._fbthrift_Counter_raiseUnexpected_result(), protocol
        )

    async def _fbthrift__handler_raiseAppError(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        deserialize(_types._fbthrift_Counter_raiseAppError_args, args, protocol)
        await self.raiseAppError()
        return serialize_iobuf(
            _types._fbthrift_Counter_raiseAppError_result(), protocol
        )


class HeartbeatHandler(Interaction):
    """Per-interaction state for ``Heartbeat`` -- exercises the
    request/response-less factory path in isolation from ``Counter``."""

    async def ping(self) -> None:
        pass

    async def _fbthrift__handler_ping(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        deserialize(_types._fbthrift_Heartbeat_ping_args, args, protocol)
        await self.ping()
        return serialize_iobuf(_types._fbthrift_Heartbeat_ping_result(), protocol)


class BoomHandler(Interaction):
    """Never actually constructed -- `createBoom` raises before this is used.
    Exists to give the throwing factory a wire method to dispatch against."""

    async def noop(self) -> None:
        pass

    async def _fbthrift__handler_noop(
        self, args: _fbthrift_iobuf.IOBuf, protocol: Protocol
    ) -> _fbthrift_iobuf.IOBuf:
        deserialize(_types._fbthrift_Boom_noop_args, args, protocol)
        await self.noop()
        return serialize_iobuf(_types._fbthrift_Boom_noop_result(), protocol)


class CalculatorHandler(CalculatorInterface):
    # -- baseline non-interaction method --

    async def echo(self, n: int) -> int:
        return n

    # -- tile factories: the canonical per-session constructors, used by BOTH
    # the implicit `performs Counter` path (framework calls createInteraction ->
    # createInteractionImpl) and the explicit factory methods below. --

    def createCounter(self) -> CounterHandler:
        return CounterHandler()

    def createHeartbeat(self) -> HeartbeatHandler:
        return HeartbeatHandler()

    # Intentionally-failing factory: exercises factory-exception propagation
    # (the client should observe an error, not a swallowed/generic failure).
    def createBoom(self) -> BoomHandler:
        raise RuntimeError("boom: factory intentionally failed")

    # -- explicit wire factory methods. The Tile is installed by the runtime via
    # the create* factory above (no leading response), so these bodies are
    # no-ops that just satisfy the wire contract. --

    async def newCounter(self) -> None:
        return None

    async def newHeartbeat(self) -> None:
        return None

    async def newBoom(self) -> None:
        return None

    def getFunctionTable(self) -> dict[bytes, FunctionEntry]:
        # Start from the generated table (echo + the factory methods as plain
        # entries) and layer in interaction routing by hand. Commit 2 makes the
        # compiler emit all of this directly.
        table: dict[bytes, FunctionEntry] = dict(super().getFunctionTable())
        rr = RpcKind.SINGLE_REQUEST_SINGLE_RESPONSE
        oneway = RpcKind.SINGLE_REQUEST_NO_RESPONSE

        # Explicit factory methods: tag the created interaction + register the
        # tile constructor so the runtime can fulfill the parked promise.
        table[b"newCounter"] = FunctionEntry(
            rr,
            self._fbthrift__handler_newCounter,
            interaction=b"Counter",
            creates_interaction=True,
            interaction_factory=self.createCounter,
        )
        table[b"newHeartbeat"] = FunctionEntry(
            rr,
            self._fbthrift__handler_newHeartbeat,
            interaction=b"Heartbeat",
            creates_interaction=True,
            interaction_factory=self.createHeartbeat,
        )
        # Explicit factory whose `createBoom` raises.
        table[b"newBoom"] = FunctionEntry(
            rr,
            self._fbthrift__handler_newBoom,
            interaction=b"Boom",
            creates_interaction=True,
            interaction_factory=self.createBoom,
        )

        # Inside-interaction methods (`creates_interaction` defaults False).
        # `handler` is the UNBOUND dispatch wrapper (referenced directly off the
        # class -- no string getattr; the runtime binds it to the per-session
        # instance at dispatch time). `interaction_factory` lets the framework
        # lazily construct the Tile for `performs Counter`.
        def counter(
            method: bytes,
            handler: Callable[..., object],
            kind: RpcKind = rr,
        ) -> None:
            table[b"Counter." + method] = FunctionEntry(
                kind,
                handler,
                interaction=b"Counter",
                interaction_factory=self.createCounter,
            )

        counter(b"add", CounterHandler._fbthrift__handler_add)
        counter(b"get", CounterHandler._fbthrift__handler_get)
        counter(b"reset", CounterHandler._fbthrift__handler_reset)
        counter(b"noop", CounterHandler._fbthrift__handler_noop, oneway)
        counter(b"getPoint", CounterHandler._fbthrift__handler_getPoint)
        counter(b"accumulatePoint", CounterHandler._fbthrift__handler_accumulatePoint)
        counter(b"addChecked", CounterHandler._fbthrift__handler_addChecked)
        counter(b"raiseUnexpected", CounterHandler._fbthrift__handler_raiseUnexpected)
        counter(b"raiseAppError", CounterHandler._fbthrift__handler_raiseAppError)
        table[b"Heartbeat.ping"] = FunctionEntry(
            rr,
            HeartbeatHandler._fbthrift__handler_ping,
            interaction=b"Heartbeat",
            interaction_factory=self.createHeartbeat,
        )
        # `createBoom` raises; this entry lets the runtime find that factory for
        # the implicit `performs Boom` path.
        table[b"Boom.noop"] = FunctionEntry(
            rr,
            BoomHandler._fbthrift__handler_noop,
            interaction=b"Boom",
            interaction_factory=self.createBoom,
        )
        return table
