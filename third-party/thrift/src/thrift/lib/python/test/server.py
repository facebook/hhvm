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
import gc
import threading
import unittest
from contextlib import contextmanager
from typing import Iterator, Optional, Sequence

from test_thrift.thrift_services import TestingServiceInterface
from test_thrift.thrift_types import Color, easy
from thrift.py3.server import SocketAddress
from thrift.python.server import ThriftServer

# White-box: the runtime holds strong refs to in-flight onStopRequested lifecycle
# tasks here so asyncio's weak task bookkeeping can't GC them before they run.
# pyre-ignore[21]: Cython module has no type stub for pyre.
from thrift.python.server_impl import python_async_processor

# Typed alias for the same runtime set object (the Cython module has no stub).
_inflight_lifecycle_tasks: set[asyncio.Task[object]] = (
    python_async_processor._inflight_lifecycle_tasks  # pyre-ignore[16]
)


@contextmanager
def event_loop() -> Iterator[asyncio.AbstractEventLoop]:
    """
    If there is already running loop, yields it. Otherwise, creates a new loop,
    sets it as the current event loop, yields it, and then closes it on exit.
    """
    try:
        loop = asyncio.get_running_loop()
        yield loop
    except RuntimeError:
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        try:
            yield loop
        finally:
            loop.close()


class Handler(TestingServiceInterface):
    initialized = False

    def __init__(self) -> None:
        self.on_start_serving = False
        self.on_stop_requested = False

    async def __aenter__(self) -> "Handler":
        self.initialized = True
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


class CancelOnStopRequestedHandler(Handler):
    # onStopRequested cancels its own lifecycle task, the way the asyncio loop
    # cancels pending tasks during server/loop teardown. The C++
    # ThriftServer::callOnStopRequested collector XLOG(FATAL)s (aborting the
    # process) if the lifecycle promise resolves to an exception, so cancellation
    # here must still complete the promise.
    async def onStopRequested(self) -> None:
        self.on_stop_requested = True
        current = asyncio.current_task()
        assert current is not None
        current.cancel()
        await asyncio.sleep(0)


class BlockingOnStopRequestedHandler(Handler):
    # onStopRequested suspends mid-flight, modelling a hook still running as the
    # server tears down, so a test can observe the in-flight lifecycle task.
    def __init__(self) -> None:
        super().__init__()
        self.hook_started: asyncio.Event = asyncio.Event()
        self.release_hook: asyncio.Event = asyncio.Event()
        self.lifecycle_task: asyncio.Task[object] | None = None

    async def onStopRequested(self) -> None:
        # onStopRequested is awaited inside the runtime's `lifecycle_coro`, not
        # wrapped in a new task, so the currently running task IS the exact
        # lifecycle task the runtime tracks. Capturing it here lets the test
        # assert on that specific task rather than on shared set state.
        self.lifecycle_task = asyncio.current_task()
        self.hook_started.set()
        await self.release_hook.wait()
        self.on_stop_requested = True


class ServicesTests(unittest.TestCase):
    def test_handler_acontext(self) -> None:
        async def inner() -> None:
            async with Handler() as h:
                self.assertTrue(h.initialized)

        asyncio.run(inner())

    def test_get_service_name(self) -> None:
        self.assertEqual(Handler.service_name(), b"TestingService")

    def test_get_address(self) -> None:
        addy = asyncio.run(self.get_address())
        self.assertIsInstance(addy, SocketAddress)

    async def get_address(self, handler: Optional[Handler] = None) -> SocketAddress:
        loop = asyncio.get_running_loop()
        server = ThriftServer(Handler() if handler is None else handler, port=0)
        serve_task = loop.create_task(server.serve())
        addy = await server.get_address()
        server.stop()
        await serve_task
        return addy

    def test_unittest_call(self) -> None:
        h = Handler()
        call = 5
        ret = asyncio.run(h.complex_action("", "", call, ""))
        self.assertEqual(call, ret)

    def test_unittest_call_renamed_func(self) -> None:
        h = Handler()
        ret = asyncio.run(h.renamed_func(True))
        self.assertTrue(ret)

    def test_server_manipulate_config(self) -> None:
        MAX_REQUESTS = 142
        MAX_CONNECTIONS = 132
        LISTEN_BACKLOG = 167
        NUM_IO_WORKERS = 10
        IDLE_TIMEOUT = 19.84
        QUEUE_TIMEOUT = 20.19
        SOCKET_QUEUE_TIMEOUT = 21.37

        with event_loop():
            server = ThriftServer(Handler(), port=0)
            server.set_max_requests(MAX_REQUESTS)
            server.set_max_connections(MAX_CONNECTIONS)
            server.set_listen_backlog(LISTEN_BACKLOG)
            server.set_io_worker_threads(NUM_IO_WORKERS)
            server.set_idle_timeout(IDLE_TIMEOUT)
            server.set_queue_timeout(QUEUE_TIMEOUT)
            server.set_socket_queue_timeout(SOCKET_QUEUE_TIMEOUT)
            self.assertEqual(server.get_max_requests(), MAX_REQUESTS)
            self.assertEqual(server.get_max_connections(), MAX_CONNECTIONS)
            self.assertEqual(server.get_listen_backlog(), LISTEN_BACKLOG)
            self.assertEqual(server.get_io_worker_threads(), NUM_IO_WORKERS)
            self.assertEqual(server.get_idle_timeout(), IDLE_TIMEOUT)
            self.assertEqual(server.get_queue_timeout(), QUEUE_TIMEOUT)
            self.assertEqual(server.get_socket_queue_timeout(), SOCKET_QUEUE_TIMEOUT)

            self.assertFalse(server.is_plaintext_allowed_on_loopback())
            server.set_allow_plaintext_on_loopback(True)
            self.assertTrue(server.is_plaintext_allowed_on_loopback())

    def test_server_get_stats(self) -> None:
        with event_loop():
            server = ThriftServer(Handler(), port=0)

            active_requests = server.get_active_requests()
            self.assertGreaterEqual(active_requests, 0)
            self.assertLess(active_requests, 10)

    def test_lifecycle_hooks(self) -> None:
        handler = Handler()
        asyncio.run(self.get_address(handler))
        self.assertTrue(handler.on_start_serving)
        self.assertTrue(handler.on_stop_requested)

    def test_on_stop_requested_cancellation_does_not_crash(self) -> None:
        handler = CancelOnStopRequestedHandler()
        asyncio.run(self.get_address(handler))
        self.assertTrue(handler.on_stop_requested)

    def test_on_stop_requested_task_is_strongly_referenced(self) -> None:
        # Regression for the shutdown BrokenPromise crash: while an
        # onStopRequested hook is in flight, the runtime must keep a strong
        # reference to its lifecycle task so asyncio's weak bookkeeping cannot
        # GC it before it completes (which would break the C++ promise and
        # FATAL-abort the worker). Mirrors _inflight_termination_tasks.
        handler: BlockingOnStopRequestedHandler = BlockingOnStopRequestedHandler()

        async def inner() -> None:
            loop = asyncio.get_running_loop()
            server = ThriftServer(handler, port=0)
            serve_task = loop.create_task(server.serve())
            await server.get_address()
            server.stop()
            # Let the stop path schedule onStopRequested and run it up to its
            # suspension point, then grab the exact task the runtime tracks.
            await asyncio.wait_for(handler.hook_started.wait(), timeout=5)
            task = handler.lifecycle_task
            assert task is not None
            # The runtime strongly references the in-flight lifecycle task...
            self.assertIn(task, _inflight_lifecycle_tasks)
            self.assertFalse(task.done())
            # ...so it survives a GC pass while suspended.
            gc.collect()
            self.assertIn(task, _inflight_lifecycle_tasks)
            handler.release_hook.set()
            await serve_task
            # Once settled, the done-callback discards exactly this task.
            await asyncio.sleep(0)
            self.assertNotIn(task, _inflight_lifecycle_tasks)

        asyncio.run(inner())
        self.assertTrue(handler.on_stop_requested)

    def test_threaded_destruction(self) -> None:
        handler = Handler()

        async def inner(handler: Handler) -> None:
            await self.get_address(handler)

        server_runner = threading.Thread(
            target=lambda handler: asyncio.run(inner(handler)),
            args=(handler,),
        )
        server_runner.start()
        server_runner.join()
        self.assertTrue(handler.on_start_serving)
        self.assertTrue(handler.on_stop_requested)
        del server_runner
        gc.collect()
