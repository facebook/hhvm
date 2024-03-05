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


from __future__ import annotations

import asyncio
import unittest
from typing import Optional, Sequence

from testing.thrift_services import TestingServiceInterface
from testing.thrift_types import Color, easy
from thrift.py3.server import SocketAddress
from thrift.python.server import ThriftServer


class Handler(TestingServiceInterface):
    initalized = False

    def __init__(self) -> None:
        self.on_start_serving = False
        self.on_stop_requested = False

    async def __aenter__(self) -> "Handler":
        self.initalized = True
        return self

    async def onStartServing(self) -> None:
        self.on_start_serving = True

    async def onStopRequested(self) -> None:
        self.on_stop_requested = True

    async def invert(self, value: bool) -> bool:
        return not value

    async def getName(self) -> str:
        return "Testing"

    async def getMethodName(self) -> str:
        return "Testing"

    async def getRequestTimeout(self) -> float:
        return 100.0

    async def shutdown(self) -> None:
        pass

    async def complex_action(
        self, first: str, second: str, third: int, fourth: str
    ) -> int:
        return third

    async def takes_a_list(self, ints: Sequence[int]) -> None:
        pass

    async def take_it_easy(self, how: int, what: easy) -> None:
        pass

    async def pick_a_color(self, color: Color) -> None:
        pass

    async def int_sizes(self, one: int, two: int, three: int, four: int) -> None:
        pass

    async def hard_error(self, valid: bool) -> None:
        pass

    async def renamed_func(self, ret: bool) -> bool:
        return ret


class ServicesTests(unittest.TestCase):
    def test_handler_acontext(self) -> None:
        loop = asyncio.get_event_loop()

        async def inner() -> None:
            async with Handler() as h:
                self.assertTrue(h.initalized)

        loop.run_until_complete(inner())

    def test_get_service_name(self) -> None:
        self.assertEqual(Handler.service_name(), b"TestingService")

    def test_get_address(self) -> None:
        loop = asyncio.get_event_loop()
        coro = self.get_address(loop)
        self.assertIsInstance(loop.run_until_complete(coro), SocketAddress)

    async def get_address(
        self, loop: asyncio.AbstractEventLoop, handler: Optional[Handler] = None
    ) -> SocketAddress:
        server = ThriftServer(Handler() if handler is None else handler, port=0)
        serve_task = loop.create_task(server.serve())
        addy = await server.get_address()
        server.stop()
        await serve_task
        return addy

    def test_unittest_call(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        call = 5
        ret = loop.run_until_complete(h.complex_action("", "", call, ""))
        self.assertEqual(call, ret)

    def test_unittest_call_renamed_func(self) -> None:
        h = Handler()
        loop = asyncio.get_event_loop()
        ret = loop.run_until_complete(h.renamed_func(True))
        self.assertTrue(ret)

    def test_server_manipulate_config(self) -> None:
        MAX_REQUESTS = 142
        MAX_CONNECTIONS = 132
        LISTEN_BACKLOG = 167
        NUM_IO_WORKERS = 10
        IDLE_TIMEOUT = 19.84
        QUEUE_TIMEOUT = 20.19

        server = ThriftServer(Handler(), port=0)
        server.set_max_requests(MAX_REQUESTS)
        server.set_max_connections(MAX_CONNECTIONS)
        server.set_listen_backlog(LISTEN_BACKLOG)
        server.set_io_worker_threads(NUM_IO_WORKERS)
        server.set_idle_timeout(IDLE_TIMEOUT)
        server.set_queue_timeout(QUEUE_TIMEOUT)
        self.assertEqual(server.get_max_requests(), MAX_REQUESTS)
        self.assertEqual(server.get_max_connections(), MAX_CONNECTIONS)
        self.assertEqual(server.get_listen_backlog(), LISTEN_BACKLOG)
        self.assertEqual(server.get_io_worker_threads(), NUM_IO_WORKERS)
        self.assertEqual(server.get_idle_timeout(), IDLE_TIMEOUT)
        self.assertEqual(server.get_queue_timeout(), QUEUE_TIMEOUT)

        self.assertFalse(server.is_plaintext_allowed_on_loopback())
        server.set_allow_plaintext_on_loopback(True)
        self.assertTrue(server.is_plaintext_allowed_on_loopback())

    def test_server_get_stats(self) -> None:
        server = ThriftServer(Handler(), port=0)

        active_requests = server.get_active_requests()
        self.assertGreaterEqual(active_requests, 0)
        self.assertLess(active_requests, 10)

    def test_lifecycle_hooks(self) -> None:
        handler = Handler()
        loop = asyncio.get_event_loop()
        coro = self.get_address(loop, handler)
        loop.run_until_complete(coro)
        self.assertTrue(handler.on_start_serving)
        self.assertTrue(handler.on_stop_requested)
