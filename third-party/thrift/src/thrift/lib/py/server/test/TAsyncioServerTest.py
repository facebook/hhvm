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
import functools
import socket
import threading
import unittest
from contextlib import contextmanager
from unittest.mock import Mock

from thrift.protocol.THeaderProtocol import THeaderProtocol
from thrift.server.TAsyncioServer import (
    ThriftAsyncServerFactory,
    ThriftClientProtocolFactory,
    ThriftHeaderClientProtocol,
    ThriftHeaderServerProtocol,
)
from thrift.Thrift import TApplicationException
from thrift.transport.TTransport import TTransportException
from thrift.util.asyncio import create_client
from thrift_asyncio.sleep import Sleep
from thrift_asyncio.tutorial import Calculator

from .handler import AsyncCalculatorHandler, AsyncSleepHandler


def server_loop_runner(loop, sock, handler, protocol_factory=None):
    return loop.run_until_complete(
        ThriftAsyncServerFactory(
            handler,
            port=None,
            loop=loop,
            sock=sock,
            protocol_factory=protocol_factory,
        ),
    )


async def test_server_with_client(sock, loop, factory=ThriftClientProtocolFactory):
    port = sock.getsockname()[1]
    (transport, protocol) = await loop.create_connection(
        factory(Calculator.Client, loop=loop),
        host="localhost",
        port=port,
    )
    client = protocol.client
    add_result = await asyncio.wait_for(
        client.add(1, 2),
        timeout=None,
    )
    transport.close()
    protocol.close()
    return add_result


async def test_echo_timeout(sock, loop, factory=ThriftClientProtocolFactory):
    port = sock.getsockname()[1]
    (transport, protocol) = await loop.create_connection(
        factory(Sleep.Client, loop=loop, timeouts={"echo": 1}),
        host="localhost",
        port=port,
    )
    client = protocol.client
    # Ask the server to delay for 30 seconds.
    # However, we told the client factory above to use a 1 second timeout
    # for the echo() function.
    await asyncio.wait_for(
        client.echo("test", 30),
        timeout=None,
    )
    transport.close()
    protocol.close()


async def test_overflow(sock, value, loop, factory=ThriftClientProtocolFactory):
    port = sock.getsockname()[1]
    (transport, protocol) = await loop.create_connection(
        factory(Sleep.Client, loop=loop, timeouts={"echo": 1}),
        host="localhost",
        port=port,
    )
    client = protocol.client

    await asyncio.wait_for(
        client.overflow(value),
        timeout=None,
    )
    transport.close()
    protocol.close()


class TestTHeaderProtocol(THeaderProtocol):
    def __init__(self, probe, *args, **kwargs):
        THeaderProtocol.__init__(self, *args, **kwargs)
        self.probe = probe

    def readMessageBegin(self):
        self.probe.touch()
        return THeaderProtocol.readMessageBegin(self)


class TestTHeaderProtocolFactory(object):
    def __init__(self, probe, *args, **kwargs):
        self.probe = probe
        self.args = args
        self.kwargs = kwargs

    def getProtocol(self, trans):
        return TestTHeaderProtocol(
            self.probe,
            trans,
            *self.args,
            **self.kwargs,
        )


class TestThriftClientProtocol(ThriftHeaderClientProtocol):
    THEADER_PROTOCOL_FACTORY = None

    def __init__(self, probe, *args, **kwargs):
        ThriftHeaderClientProtocol.__init__(self, *args, **kwargs)

        def factory(*args, **kwargs):
            return TestTHeaderProtocolFactory(probe, *args, **kwargs)

        self.THEADER_PROTOCOL_FACTORY = factory


class TAsyncioServerTest(unittest.TestCase):
    def test_THEADER_PROTOCOL_FACTORY_readMessageBegin(self):
        loop = asyncio.get_event_loop()
        loop.set_debug(True)
        sock = socket.socket()
        server_loop_runner(loop, sock, AsyncCalculatorHandler())

        class Probe(object):
            def __init__(self):
                self.touched = False

            def touch(self):
                self.touched = True

        probe = Probe()

        def factory(*args, **kwargs):
            return functools.partial(
                TestThriftClientProtocol,
                probe,
                *args,
                **kwargs,
            )

        add_result = loop.run_until_complete(
            test_server_with_client(
                sock,
                loop,
                factory=factory,
            )
        )
        self.assertTrue(probe.touched)
        self.assertEqual(42, add_result)

    def test_read_error(self):
        """Test the behavior if readMessageBegin() throws an exception"""
        loop = asyncio.get_event_loop()
        loop.set_debug(True)
        sock = socket.socket()
        server_loop_runner(loop, sock, AsyncCalculatorHandler())

        # A helper Probe class that will raise an exception when
        # it is invoked by readMessageBegin()
        class Probe(object):
            def touch(self):
                raise TTransportException(
                    TTransportException.INVALID_TRANSFORM, "oh noes"
                )

        probe = Probe()

        def factory(*args, **kwargs):
            return functools.partial(
                TestThriftClientProtocol,
                probe,
                *args,
                **kwargs,
            )

        try:
            add_result = loop.run_until_complete(
                test_server_with_client(
                    sock,
                    loop,
                    factory=factory,
                )
            )
            self.fail(
                "expected client method to throw; instead returned %r" % (add_result,)
            )
        except TTransportException as ex:
            self.assertEqual(str(ex), "oh noes")
            self.assertEqual(ex.type, TTransportException.INVALID_TRANSFORM)

    def _test_using_event_loop(self, loop):
        sock = socket.socket()
        server_loop_runner(loop, sock, AsyncCalculatorHandler())
        add_result = loop.run_until_complete(test_server_with_client(sock, loop))
        self.assertEqual(42, add_result)

    def test_default_event_loop(self):
        loop = asyncio.get_event_loop()
        loop.set_debug(True)
        self._test_using_event_loop(loop)

    def test_custom_event_loop(self):
        loop = asyncio.new_event_loop()
        loop.set_debug(True)
        self.assertIsNot(loop, asyncio.get_event_loop())
        self._test_using_event_loop(loop)

    def _start_server_thread(self, server, loop):
        def _run(server, loop):
            loop.run_until_complete(server.wait_closed())

        t = threading.Thread(target=functools.partial(_run, server, loop))
        t.start()
        return t

    def test_server_in_separate_thread(self):
        sock = socket.socket()
        server_loop = asyncio.new_event_loop()
        server_loop.set_debug(True)
        server = server_loop_runner(server_loop, sock, AsyncCalculatorHandler())
        server_thread = self._start_server_thread(server, server_loop)

        client_loop = asyncio.new_event_loop()
        client_loop.set_debug(True)
        add_result = client_loop.run_until_complete(
            test_server_with_client(sock, client_loop),
        )
        self.assertEqual(42, add_result)

        server_loop.call_soon_threadsafe(server.close)
        server_thread.join()

    async def _make_out_of_order_calls(self, sock, loop):
        port = sock.getsockname()[1]
        client_manager = await create_client(
            Sleep.Client,
            host="localhost",
            port=port,
            loop=loop,
        )
        with client_manager as client:
            futures = [client.echo(str(delay), delay * 0.1) for delay in [3, 2, 1]]
            results_in_arrival_order = []
            for f in asyncio.as_completed(futures):
                result = await f
                results_in_arrival_order.append(result)
            self.assertEqual(["1", "2", "3"], results_in_arrival_order)

    @contextmanager
    def server_in_background_thread(self, sock):
        server_loop = asyncio.new_event_loop()
        server_loop.set_debug(True)
        handler = AsyncSleepHandler(server_loop)
        server = server_loop_runner(server_loop, sock, handler)
        server_thread = self._start_server_thread(server, server_loop)
        try:
            yield server
        finally:
            server_loop.call_soon_threadsafe(server.close)
            server_thread.join()

    def test_out_of_order_calls(self):
        sock = socket.socket()
        with self.server_in_background_thread(sock):
            client_loop = asyncio.new_event_loop()
            client_loop.set_debug(True)
            client_loop.run_until_complete(
                self._make_out_of_order_calls(sock, client_loop),
            )

    async def _assert_transport_is_closed_on_error(self, sock, loop):
        port = sock.getsockname()[1]
        client_manager = await create_client(
            Sleep.Client,
            host="localhost",
            port=port,
            loop=loop,
        )
        try:
            with client_manager as client:
                raise Exception("expected exception from test")
        except Exception:
            self.assertFalse(client._oprot.trans.isOpen())

    def test_close_client_on_error(self):
        sock = socket.socket()
        with self.server_in_background_thread(sock):
            loop = asyncio.new_event_loop()
            loop.set_debug(True)
            loop.run_until_complete(
                self._assert_transport_is_closed_on_error(sock, loop),
            )

    def test_overflow_failure(self):
        loop = asyncio.get_event_loop()
        loop.set_debug(True)
        sock = socket.socket()
        server_loop_runner(loop, sock, AsyncSleepHandler(loop))
        with self.assertRaises(TTransportException, msg="Connection closed"):
            # This will raise an exception on the server. The
            # OverflowResult.value is byte and 0xffff will result in exception
            #
            #    struct.error('byte format requires -128 <= number <= 127',)
            loop.run_until_complete(test_overflow(sock, 0xFFFF, loop))

    def test_overflow_success(self):
        loop = asyncio.get_event_loop()
        loop.set_debug(True)
        sock = socket.socket()
        server_loop_runner(loop, sock, AsyncSleepHandler(loop))

        # This shouldn't raise any exceptions
        loop.run_until_complete(test_overflow(sock, 0x7F, loop))

    def test_timeout(self):
        loop = asyncio.get_event_loop()
        loop.set_debug(True)
        sock = socket.socket()
        server_loop_runner(loop, sock, AsyncSleepHandler(loop))
        with self.assertRaisesRegex(TApplicationException, "Call to echo timed out"):
            loop.run_until_complete(test_echo_timeout(sock, loop))

    def test_custom_protocol_factory(self):
        loop = asyncio.get_event_loop()
        sock = socket.socket()
        wrapper_protocol = None

        def wrapper_protocol_factory(*args, **kwargs):
            nonlocal wrapper_protocol
            wrapper_protocol = Mock(wraps=ThriftHeaderServerProtocol(*args, **kwargs))
            return wrapper_protocol

        server_loop_runner(
            loop, sock, AsyncCalculatorHandler(), wrapper_protocol_factory
        )
        add_result = loop.run_until_complete(test_server_with_client(sock, loop))
        self.assertEqual(42, add_result)
        wrapper_protocol.connection_made.assert_called_once()
        wrapper_protocol.data_received.assert_called()
