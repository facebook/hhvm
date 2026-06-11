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

"""Server-side interaction test for the connection-close-before-terminate path.

The standard interaction test harness cannot reach this path: leaving an
interaction's ``async with`` (or even GC'ing the interaction client) always
sends a graceful terminate first, which drives the server's ``co_onTermination``
hook. To instead exercise the connection-close path (``~PythonTile`` ->
``scheduleInteractionTermination``), this test puts a minimal TCP proxy between
the client and the in-process server and abruptly drops the connection while an
interaction is still open.
"""

from __future__ import annotations

import asyncio
import unittest

from calculator.thrift_clients import Calculator
from thrift.py3.server import SocketAddress
from thrift.python.client import ClientType, get_client
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


class _TcpProxy:
    """A minimal byte-forwarding TCP proxy sitting between the client and the
    in-process server.

    It exists so a test can *abruptly* drop the connection -- aborting the
    sockets with no graceful shutdown -- while an interaction is still open, so
    the server observes the connection closing *before* any terminate frame
    arrives. That is the only way to drive the server's connection-close
    termination path (``~PythonTile`` -> ``scheduleInteractionTermination``):
    the normal ``async with`` client teardown always sends an explicit terminate
    first, which instead drives ``co_onTermination``.
    """

    def __init__(self, target: SocketAddress) -> None:
        assert target.port is not None
        self._target_port: int = target.port
        self._server: asyncio.AbstractServer | None = None
        self._writers: list[asyncio.StreamWriter] = []
        self.port: int = 0

    async def __aenter__(self) -> _TcpProxy:
        self._server = await asyncio.start_server(
            self._handle_client, host="::1", port=0
        )
        self.port = self._server.sockets[0].getsockname()[1]
        return self

    async def __aexit__(self, *_exc_info: object) -> None:
        self.drop()
        server = self._server
        assert server is not None
        server.close()
        await server.wait_closed()

    async def _handle_client(
        self,
        client_reader: asyncio.StreamReader,
        client_writer: asyncio.StreamWriter,
    ) -> None:
        try:
            server_reader, server_writer = await asyncio.open_connection(
                host="::1", port=self._target_port
            )
        except Exception:
            client_writer.close()
            return
        self._writers.extend((client_writer, server_writer))
        await asyncio.gather(
            self._pump(client_reader, server_writer),
            self._pump(server_reader, client_writer),
            return_exceptions=True,
        )

    async def _pump(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        try:
            while True:
                data = await reader.read(65536)
                if not data:  # EOF
                    break
                writer.write(data)
                await writer.drain()
        except OSError:
            # Expected once the connection is aborted: the in-flight read/drain
            # raises a ConnectionResetError/ConnectionAbortedError/BrokenPipeError
            # (all OSError). Treat it as normal pump termination. Anything else
            # propagates.
            pass
        finally:
            try:
                writer.close()
            except Exception:
                pass

    def drop(self) -> None:
        """Abort every proxied socket (RST, no graceful flush) so both the client
        and the server immediately see the connection vanish."""
        for writer in self._writers:
            writer.transport.abort()
        self._writers.clear()


class ConnectionDropInteractionTest(unittest.IsolatedAsyncioTestCase):
    """Covers the connection-close-before-terminate path via a proxy harness that
    can abruptly drop the connection (which the standard client cannot do)."""

    def _client(self, port: int) -> Calculator.Async:
        return get_client(
            Calculator,
            host="::1",
            port=port,
            client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
        )

    async def test_termination_hook_fires_on_connection_drop(self) -> None:
        # Open an interaction and make a call so the tile exists server-side,
        # then drop the connection *without* leaving the interaction's `async
        # with` -- so no terminate is sent. The server must still run the
        # termination hook exactly once, from the tile destructor.
        handler = CalculatorHandler()
        async with _InProcessServer(handler) as addr:
            async with _TcpProxy(addr) as proxy:
                async with self._client(proxy.port) as calc:
                    counter = calc.createCounter()
                    await counter.__aenter__()
                    try:
                        await counter.add(1)
                        proxy.drop()
                        await handler.wait_for_counter_terminations(1)
                        self.assertEqual(handler.counter_terminations, 1)
                    finally:
                        # Clean up the interaction client. The channel is already
                        # dead, so the terminate this would normally send is a
                        # no-op and `__aexit__` does no network I/O, so it can't
                        # raise here.
                        await counter.__aexit__(None, None, None)
