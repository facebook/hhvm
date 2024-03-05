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
import time
import unittest
from pathlib import Path
from typing import Optional, Sequence

from derived.thrift_clients import DerivedTestingService
from derived.thrift_services import DerivedTestingServiceInterface
from folly.iobuf import IOBuf
from stack_args.thrift_clients import StackService
from stack_args.thrift_services import StackServiceInterface
from stack_args.thrift_types import simple
from testing.thrift_clients import TestingService
from testing.thrift_services import TestingServiceInterface
from testing.thrift_types import Color, easy, SimpleError
from thrift.py3.server import get_context, SocketAddress
from thrift.python.client import get_client
from thrift.python.common import Priority, RpcOptions
from thrift.python.exceptions import ApplicationError
from thrift.python.server import ServiceInterface, ThriftServer


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


# pyre-fixme[13]: Attribute `serve_task` is never initialized.
class TestServer:
    server: ThriftServer
    serve_task: asyncio.Task

    def __init__(
        self,
        ip: Optional[str] = None,
        path: Optional["Path"] = None,
        handler: ServiceInterface = Handler(),  # noqa: B008
    ) -> None:
        self.server = ThriftServer(handler, ip=ip, path=path)

    async def __aenter__(self) -> SocketAddress:
        self.serve_task = asyncio.get_event_loop().create_task(self.server.serve())
        return await self.server.get_address()

    # pyre-fixme[2]: Parameter must be annotated.
    async def __aexit__(self, *exc_info) -> None:
        self.server.stop()
        await self.serve_task


class ClientServerTests(unittest.IsolatedAsyncioTestCase):
    """
    These are tests where a client and server talk to each other
    """

    async def test_get_context(self) -> None:

        async with TestServer(ip="::1") as sa:
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

        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                options = RpcOptions()
                options.set_header("from client", "with love")
                self.assertFalse(await client.invert(True, rpc_options=options))
                self.assertIn("from server", options.read_headers)

    async def test_server_localhost(self) -> None:

        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertTrue(await client.invert(False))
                self.assertFalse(await client.invert(True))
                # TODO (ffrancet): after RPC headers are supported, check uex and uexw
                with self.assertRaises(SimpleError):
                    await client.takes_a_list([])

    async def test_no_client_aexit(self) -> None:

        async with TestServer() as sa:
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

        async with TestServer() as sa:
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

        async with TestServer() as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            get_client(TestingService, host=ip, port=port)

    # If we do not abort here then good

    async def test_derived_service(self) -> None:
        """
        This tests calling methods from a derived service
        """

        async with TestServer(handler=DerivedHandler()) as sa:
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

        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                self.assertEqual(True, await client.renamed_func(True))

    async def test_queue_timeout(self) -> None:
        """
        This tests whether queue timeout functions properly.
        """

        class SlowDerivedHandler(Handler, DerivedTestingServiceInterface):
            async def getName(self) -> str:
                time.sleep(1)
                return "SlowDerivedTesting"

            async def derived_pick_a_color(self, color: Color) -> Color:
                return color

        testing = TestServer(handler=SlowDerivedHandler())
        testing.server.set_queue_timeout(0.01)

        async def client_call(sa: SocketAddress) -> str:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(DerivedTestingService, host=ip, port=port) as client:
                try:
                    return await client.getName()
                except ApplicationError as err:
                    if "Queue Timeout" in str(err):
                        return "Queue Timeout"
                    else:
                        return ""

        async def clients_run(server: TestServer) -> None:
            async with server as sa:
                results = await asyncio.gather(
                    client_call(sa),
                    client_call(sa),
                    client_call(sa),
                    client_call(sa),
                    client_call(sa),
                )
                self.assertIn("Queue Timeout", results)

        await clients_run(testing)

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

        async with TestServer(handler=CancelHandler(), ip="::1") as sa:
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

        async with TestServer(handler=ErrorHandler(), ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                with self.assertRaises(ApplicationError) as ex:
                    await client.getName()
                self.assertEqual(
                    ex.exception.message,
                    f"Exception('{errMessage}')",
                )

    async def test_request_with_default_rpc_options(self) -> None:

        async with TestServer(ip="::1") as sa:
            ip, port = sa.ip, sa.port
            assert ip and port
            async with get_client(TestingService, host=ip, port=port) as client:
                timeout = await client.getRequestTimeout()
                self.assertEqual(timeout, 0.0)
                priority = await client.getPriority()
                self.assertEqual(Priority(priority), Priority.N_PRIORITIES)

    async def test_request_with_specified_rpc_options(self) -> None:

        async with TestServer(ip="::1") as sa:
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

        async with TestServer(handler=StackHandler(), ip="::1") as sa:
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
