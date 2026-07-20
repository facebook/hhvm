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
from typing import Sequence

from derived.thrift_clients import DerivedTestingService
from derived.thrift_services import DerivedTestingServiceInterface
from folly.iobuf import IOBuf
from stack_args.thrift_clients import StackService
from stack_args.thrift_services import StackServiceInterface
from stack_args.thrift_types import simple
from test_thrift.thrift_clients import TestingService
from test_thrift.thrift_services import TestingServiceInterface
from test_thrift.thrift_types import Color, easy, SimpleError
from thrift.lib.python.test.event_handlers.helper import ThrowHelper, ThrowHelperHandler
from thrift.lib.python.test.test_server import TestServer
from thrift.py3.server import get_context, ReadHeaders, RequestContext, WriteHeaders
from thrift.python.client import get_client
from thrift.python.common import Priority, RpcOptions
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    ApplicationOverloadError,
)
from thrift.python.server import ServiceInterface


class Handler(TestingServiceInterface):
    async def invert(self, value: bool) -> bool:
        ctx = get_context()
        if "from client" in ctx.read_headers:
            ctx.set_header("from server", "with love")
        return not value

    async def getName(self) -> str:
        ctx = get_context()
        ctx.set_header("contextvar", "true")
        return "Testing"

    async def getMethodName(self) -> str:
        ctx = get_context()
        return ctx.method_name

    async def getRequestId(self) -> str:
        ctx = get_context()
        return ctx.request_id

    async def getRequestTimeout(self) -> float:
        ctx = get_context()
        return ctx.request_timeout

    async def shutdown(self) -> None:
        pass

    async def complex_action(
        self, first: str, second: str, third: int, fourth: str
    ) -> int:
        return third

    async def takes_a_list(self, ints: Sequence[int]) -> None:
        raise SimpleError(color=Color.red)

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

    async def getPriority(self) -> int:
        ctx = get_context()
        return ctx.priority.value


class DerivedHandler(Handler, DerivedTestingServiceInterface):
    async def getName(self) -> str:
        return "DerivedTesting"

    async def derived_pick_a_color(self, color: Color) -> Color:
        return color


def local_server(handler: ServiceInterface | None = None) -> TestServer:
    if handler is None:
        handler = Handler()
    return TestServer(handler=handler, ip="::1")


def default_server() -> TestServer:
    # note in this case, port is set to 0
    return TestServer(handler=Handler())


class ClientServerTests(unittest.IsolatedAsyncioTestCase):
    """
    These are tests where a client and server talk to each other
    """

    async def test_get_context(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                options = RpcOptions()
                options.timeout = 100.0

                self.assertEqual("Testing", await client.getName(rpc_options=options))
                self.assertEqual("true", options.read_headers["contextvar"])
                self.assertEqual(
                    "getMethodName",
                    await client.getMethodName(),
                )
                # requestId is a 16 char wide hex string
                self.assertEqual(
                    len(await client.getRequestId()),
                    16,
                )
                self.assertEqual(
                    100.0,
                    await client.getRequestTimeout(rpc_options=options),
                )

        handler = Handler()  # so we can call it outside the thrift server
        with self.assertRaises(LookupError):
            await handler.getName()

    async def test_rpc_headers(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                options = RpcOptions()
                options.set_header("from client", "with love")
                self.assertFalse(await client.invert(True, rpc_options=options))
                self.assertIn("from server", options.read_headers)

    async def test_server_localhost(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertTrue(await client.invert(False))
                self.assertFalse(await client.invert(True))
                # TODO (ffrancet): after RPC headers are supported, check uex and uexw
                with self.assertRaises(SimpleError):
                    await client.takes_a_list([])

    async def test_no_client_aexit(self) -> None:
        async with default_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            client = get_client(TestingService, host=ip, port=port)
            await client.__aenter__()
            self.assertTrue(await client.invert(False))
            self.assertFalse(await client.invert(True))

    # If we do not abort here then good

    async def test_client_aexit_no_await(self) -> None:
        """
        This actually handles the case if __aexit__ is not awaited
        """

        async with default_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            client = get_client(TestingService, host=ip, port=port)
            await client.__aenter__()
            self.assertTrue(await client.invert(False))
            self.assertFalse(await client.invert(True))
            # pyre-fixme[1001]: Async expression is not awaited.
            _ = client.__aexit__(None, None, None)
            del client  # If we do not abort here then good

    async def test_no_client_no_aenter(self) -> None:
        """
        This covers if aenter was canceled since those two are the same really
        """

        async with default_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            get_client(TestingService, host=ip, port=port)

    # If we do not abort here then good

    async def test_derived_service(self) -> None:
        """
        This tests calling methods from a derived service
        """

        async with local_server(handler=DerivedHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(
                DerivedTestingService,
                host=ip,
                port=port,
            ) as client:
                self.assertEqual(await client.getName(), "DerivedTesting")
                self.assertEqual(
                    await client.derived_pick_a_color(Color.red), Color.red
                )

    async def test_renamed_func(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertEqual(True, await client.renamed_func(True))

    async def test_cancelled_task(self) -> None:
        """
        This tests whether cancelled tasks are handled properly.
        """
        cancelledMessage: str = "I have been cancelled"

        class CancelHandler(Handler):
            async def getName(self) -> str:
                raise asyncio.CancelledError(
                    cancelledMessage
                )  # Pretend that this is some await call that gets cancelled

        async with local_server(handler=CancelHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                with self.assertRaises(ApplicationError) as ex:
                    await client.getName()
                self.assertEqual(
                    ex.exception.message,
                    f"Application was cancelled on the server with message: {cancelledMessage}",
                )

    async def test_unexpected_error(self) -> None:
        """
        This tests whether unexpected errors handled properly.
        """
        errMessage: str = "I am an error"

        class ErrorHandler(TestingServiceInterface):
            async def getName(self) -> str:
                raise Exception(errMessage)

        async with local_server(handler=ErrorHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                with self.assertRaises(ApplicationError) as ex:
                    await client.getName()
                self.assertEqual(
                    ex.exception.message,
                    f"Exception('{errMessage}')",
                )

    async def test_application_error_type_is_preserved(self) -> None:
        """
        A handler that raises a plain ApplicationError has its type discarded:
        the client observes ApplicationErrorType.UNKNOWN, not the type the
        handler raised.

        This is the EXPECTED, deliberate behavior -- handler-thrown exceptions
        are untrusted, so their error codes are intentionally not propagated (a
        rogue handler must not be able to forge the codes the client/SR layer
        act on). Handlers that genuinely need to shed load should raise
        ApplicationOverloadError instead (see
        test_application_overload_error_preserves_type).
        """
        errMessage: str = "shedding load"

        class ErrorHandler(TestingServiceInterface):
            async def getName(self) -> str:
                raise ApplicationError(ApplicationErrorType.LOADSHEDDING, errMessage)

        async with local_server(handler=ErrorHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                with self.assertRaises(ApplicationError) as ex:
                    await client.getName()
                # NOTE: This is the EXPECTED behavior. It prevents uncaught
                # ApplicationErrors from an upstream service from being
                # propagated to client of downstream service. A loadshedding
                # state in an upstream service does not necessarily imply
                # that all downstream services must have loadshedding state.
                # If this is the case, the ApplicationError should be caught
                # and TrustedServerException raised instead.
                self.assertEqual(
                    ex.exception.type,
                    ApplicationErrorType.UNKNOWN,
                )
                self.assertEqual(ex.exception.message, errMessage)

    async def test_application_overload_error_preserves_type(self) -> None:
        """
        A handler that raises ApplicationOverloadError propagates
        ApplicationErrorType.LOADSHEDDING to the client, unlike a plain
        ApplicationError whose type is discarded (see above). The overload error
        code is mapped to a ResponseRpcErrorCode on the server and back to an
        ApplicationErrorType on the client.
        """
        errMessage: str = "shedding load"

        class ErrorHandler(TestingServiceInterface):
            async def getName(self) -> str:
                raise ApplicationOverloadError(errMessage)

        async with local_server(handler=ErrorHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                with self.assertRaises(ApplicationError) as ex:
                    await client.getName()
                self.assertEqual(
                    ex.exception.type,
                    ApplicationErrorType.LOADSHEDDING,
                )
                self.assertEqual(ex.exception.message, errMessage)

    async def test_request_with_default_rpc_options(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                timeout = await client.getRequestTimeout()
                self.assertEqual(timeout, 0.0)
                priority = await client.getPriority()
                self.assertEqual(Priority(priority), Priority.N_PRIORITIES)

    async def test_request_with_specified_rpc_options(self) -> None:
        async with local_server() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                options = RpcOptions()
                options.timeout = 15.0
                options.priority = Priority.BEST_EFFORT
                timeout = await client.getRequestTimeout(rpc_options=options)
                self.assertEqual(timeout, 15.0)
                priority = await client.getPriority(rpc_options=options)
                self.assertEqual(Priority(priority), Priority.BEST_EFFORT)

    async def test_client_event_handler_throw(self) -> None:
        for handler in ThrowHelperHandler:
            async with local_server() as sa:
                ip, port = sa.ip, sa.port
                self.assertIsNotNone(ip)
                self.assertIsNotNone(port)
                async with get_client(TestingService, host=ip, port=port) as client:
                    with ThrowHelper(handler):
                        # pyrefly: ignore [bad-argument-type]
                        with self.assertRaisesRegex(ApplicationError, handler.value):
                            await client.complex_action("1", "2", 3, "4")

    async def test_request_context_invalidated_after_rpc(self) -> None:
        captured_ctx: RequestContext | None = None
        captured_headers: ReadHeaders | None = None

        class CapturingHandler(Handler):
            async def getName(self) -> str:
                nonlocal captured_ctx, captured_headers
                captured_ctx = get_context()
                captured_headers = captured_ctx.read_headers
                return "Testing"

        async with local_server(handler=CapturingHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertEqual("Testing", await client.getName())

        assert captured_ctx is not None
        assert captured_headers is not None

        with self.assertRaises(RuntimeError):
            dict(captured_ctx.read_headers)
        with self.assertRaises(RuntimeError):
            _ = captured_ctx.priority
        with self.assertRaises(RuntimeError):
            _ = captured_ctx.request_timeout
        with self.assertRaises(RuntimeError):
            captured_ctx.set_header("key", "value")

        with self.assertRaises(RuntimeError):
            _ = captured_headers["any_key"]

        self.assertIsNotNone(captured_ctx.connection_context)
        self.assertIsInstance(captured_ctx.request_id, str)
        self.assertIsInstance(captured_ctx.method_name, str)

    async def test_request_context_invalidated_by_default(self) -> None:
        captured_ctx: RequestContext | None = None
        captured_headers: ReadHeaders | None = None

        class CapturingHandler(Handler):
            async def getName(self) -> str:
                nonlocal captured_ctx, captured_headers
                captured_ctx = get_context()
                captured_headers = captured_ctx.read_headers
                return "Testing"

        async with local_server(handler=CapturingHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertEqual("Testing", await client.getName())

        assert captured_ctx is not None
        assert captured_headers is not None

        self.assertFalse(captured_ctx.is_valid())

        with self.assertRaises(RuntimeError):
            dict(captured_ctx.read_headers)

        self.assertIsNotNone(captured_ctx.connection_context)
        self.assertIsInstance(captured_ctx.request_id, str)
        self.assertIsInstance(captured_ctx.method_name, str)

    async def test_request_context_invalidated_in_background_task(self) -> None:
        captured_ctx: RequestContext | None = None
        background_error: Exception | None = None
        background_done: asyncio.Event = asyncio.Event()
        eviction_done: asyncio.Event = asyncio.Event()

        class CapturingHandler(Handler):
            async def getName(self) -> str:
                nonlocal captured_ctx
                if captured_ctx is None:
                    captured_ctx = get_context()
                    asyncio.get_running_loop().create_task(background_access())
                return "Testing"

        async def background_access() -> None:
            nonlocal background_error
            try:
                await eviction_done.wait()
                assert captured_ctx is not None
                _ = captured_ctx.priority
            except Exception as e:
                background_error = e
            finally:
                background_done.set()

        test_server = TestServer(handler=CapturingHandler(), ip="::1")
        test_server.server.set_io_worker_threads(1)
        async with test_server as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertEqual("Testing", await client.getName())
                # Send 11 more requests to evict the first request's DebugStub
                # from RequestsRegistry's finished list (default limit: 10),
                # freeing the underlying Cpp2RequestContext memory.
                for _ in range(11):
                    await client.getName()

            eviction_done.set()
            await asyncio.wait_for(background_done.wait(), timeout=5.0)

        self.assertIsInstance(background_error, RuntimeError)
        self.assertIn(
            "Request context is no longer valid. The Thrift request has already completed",
            str(background_error),
        )

    def test_request_context_direct_construction(self) -> None:
        ctx = RequestContext()
        self.assertFalse(ctx.is_valid())
        self.assertIsNone(ctx.connection_context)
        self.assertEqual(ctx.method_name, "")
        self.assertEqual(ctx.request_id, "")
        # actually accessing them can cause problems because cpp pointer
        # invalid, but tests that construct RequestContext directly
        # just rely on these for mocking
        self.assertIsInstance(ctx.read_headers, ReadHeaders)
        self.assertIsInstance(ctx.write_headers, WriteHeaders)
        self.assertIsNone(ctx.read_headers_or_none)
        self.assertIsNone(ctx.write_headers_or_none)

    async def test_or_none_accessors(self) -> None:
        captured_ctx: RequestContext | None = None

        class CapturingHandler(Handler):
            async def getName(self) -> str:
                nonlocal captured_ctx
                captured_ctx = get_context()
                return "Testing"

        async with local_server(handler=CapturingHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertEqual("Testing", await client.getName())

        assert captured_ctx is not None

        self.assertIsNone(captured_ctx.priority_or_none)
        self.assertIsNone(captured_ctx.read_headers_or_none)
        self.assertIsNone(captured_ctx.write_headers_or_none)
        self.assertIsNone(captured_ctx.request_timeout_or_none)

        self.assertIsNotNone(captured_ctx.connection_context)
        self.assertIsInstance(captured_ctx.request_id, str)
        self.assertIsInstance(captured_ctx.method_name, str)


class StackHandler(StackServiceInterface):
    async def add_to(self, lst: Sequence[int], value: int) -> Sequence[int]:
        return [x + value for x in lst]

    async def get_simple(self) -> simple:
        return simple(val=66)

    async def get_simple_no_sa(self) -> simple:
        return simple(val=88)

    async def take_simple(self, smpl: simple) -> None:
        if smpl.val != 10:
            raise Exception("WRONG")

    async def get_iobuf(self) -> IOBuf:
        return IOBuf(b"abc")

    async def take_iobuf(self, val: IOBuf) -> None:
        if b"".join(val) != b"cba":
            raise Exception("WRONG")

    # currently unsupported by cpp backend:
    # async def get_iobuf_ptr(self) -> IOBuf:
    #     return IOBuf(b'xyz')

    async def take_iobuf_ptr(self, val: IOBuf) -> None:
        if b"".join(val) != b"zyx":
            raise Exception("WRONG")


class ClientStackServerTests(unittest.IsolatedAsyncioTestCase):
    """
    These are tests where a client and server(stack_arguments) talk to each other
    """

    async def test_server_localhost(self) -> None:
        async with local_server(handler=StackHandler()) as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(StackService, host=ip, port=port) as client:
                self.assertEqual(
                    (3, 4, 5, 6), await client.add_to(lst=(1, 2, 3, 4), value=2)
                )
                self.assertEqual(66, (await client.get_simple()).val)
                self.assertEqual((await client.get_simple_no_sa()).val, 88)
                await client.take_simple(simple(val=10))
                self.assertEqual(b"abc", b"".join(await client.get_iobuf()))
                await client.take_iobuf(IOBuf(b"cba"))
                # currently unsupported by cpp backend:
                # self.assertEqual(b'xyz', (await client.get_iobuf_ptr()))
                await client.take_iobuf_ptr(IOBuf(b"zyx"))


class RpcOptionsPropertyTest(unittest.TestCase):
    def test_routing_key_defaults_to_empty(self) -> None:
        self.assertEqual(RpcOptions().routing_key, "")

    def test_routing_key_round_trips(self) -> None:
        options = RpcOptions()
        options.routing_key = "user-42"
        self.assertEqual(options.routing_key, "user-42")

    def test_routing_key_rejects_none(self) -> None:
        with self.assertRaises(TypeError):
            # pyre-ignore[8]: intentionally passing None to check the guard
            RpcOptions().routing_key = None
