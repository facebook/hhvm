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

import asyncio
import errno
import os
import socket
import sys
import tempfile
import traceback
import types
import unittest

from testing.clients import TestingService
from testing.types import easy, I32List
from thrift.lib.py3.test.auto_migrate.auto_migrate_util import is_auto_migrated
from thrift.lib.python.client.test.client_event_handler.helper import (
    TestHelper as ClientEventHandlerTestHelper,
)
from thrift.lib.python.client.test.event_handler_helper import (
    client_handler_that_throws,
)
from thrift.py3.client import (
    Client,
    get_client,
    get_proxy_factory,
    install_proxy_factory,
)
from thrift.py3.common import Priority, RpcOptions, WriteHeaders
from thrift.py3.exceptions import TransportError


async def bad_client_connect() -> None:
    async with get_client(TestingService, port=1) as client:
        await client.complex_action("foo", "bar", 9, "baz")


class ThriftClientTestProxy:
    inner: Client

    def __init__(self, inner: Client) -> None:
        self.inner = inner


class ClientTests(unittest.IsolatedAsyncioTestCase):
    def test_annotations(self) -> None:
        annotations = TestingService.annotations
        self.assertIsInstance(annotations, types.MappingProxyType)
        self.assertFalse(annotations.get("NotAnAnnotation"))
        self.assertEqual(annotations["fun_times"], "yes")
        with self.assertRaises(TypeError):
            # we override __setattr__ of class to prevent this
            TestingService.annotations = {}

        with self.assertRaises(TypeError):
            # pyre-ignore[16]: for test
            TestingService.annotations["new_annotation"] = "no"

    async def test_client_keyword_arguments(self) -> None:
        async with ClientEventHandlerTestHelper().get_async_client(
            TestingService, port=1
        ) as client:
            # TransportError expected because not a real handler
            with self.assertRaises(TransportError):
                await client.complex_action(
                    first="foo", second="bar", third=9, fourth="baz"
                )

            # TransportError expected because not a real handler
            with self.assertRaises(TransportError):
                await client.complex_action("foo", "bar", 9, "baz")

    async def test_none_arguments(self) -> None:
        async with ClientEventHandlerTestHelper().get_async_client(
            TestingService, port=1
        ) as client:
            with self.assertRaises(TypeError):
                await client.take_it_easy(9)  # pyre-ignore[20] testing bad behaviour

            # thrift-python doesn't TypeError on None because all args passed to struct
            # where they become kwargs
            # in thrift-py3, TransportError expected because not a real handler
            expected_err = TransportError if is_auto_migrated() else TypeError
            with self.assertRaises(expected_err):
                await client.take_it_easy(
                    9,
                    # pyre-ignore[6] testing bad behaviour: should by `easy` type
                    None,
                )
            with self.assertRaises(expected_err):
                await client.takes_a_list(None)  # pyre-ignore[6] testing bad behaviour
            with self.assertRaises(expected_err):
                await client.invert(None)  # pyre-ignore[6] testing bad behaviour
            with self.assertRaises(expected_err):
                await client.pick_a_color(None)  # pyre-ignore[6] testing bad behaviour
            with self.assertRaises(expected_err):
                await client.take_it_easy(
                    # pyre-fixme[6]: For 1st argument expected `int` but got `None`.
                    None,
                    easy(),
                )

    def test_bad_unix_domain_socket_raises_TransportError_on_connection(self) -> None:
        with (
            tempfile.TemporaryDirectory() as tempdir,
            socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as s,
        ):
            socket_path: str = os.path.join(tempdir, "socket")
            s.bind(socket_path)

            async def connect_to_unlistened_socket() -> None:
                async with get_client(TestingService, path=socket_path):
                    pass

            loop = asyncio.get_event_loop()
            with self.assertRaises(TransportError) as cm:
                loop.run_until_complete(connect_to_unlistened_socket())
            ex = cm.exception
            self.assertEqual(ex.errno, errno.ECONNREFUSED)

    def test_TransportError(self) -> None:
        """
        Are C++ TTransportException converting properly to py TransportError
        """
        loop = asyncio.get_event_loop()
        with self.assertRaises(TransportError):
            loop.run_until_complete(bad_client_connect())

        try:
            loop.run_until_complete(bad_client_connect())
        except TransportError as ex:
            # Test that we can get the errno
            self.assertEqual(ex.errno, 0)
            # The traceback should be short since it raises inside
            # the rpc call not down inside the guts of thrift-py3
            tb_length = 5 if is_auto_migrated() else 3
            tb_frames = traceback.extract_tb(sys.exc_info()[2])
            self.assertEqual(len(tb_frames), tb_length, tb_frames)

    def test_set_persistent_header(self) -> None:
        """
        This was causing a nullptr dereference and thus a segfault
        """
        loop = asyncio.get_event_loop()

        async def test() -> None:
            async with get_client(TestingService, port=1, headers={"foo": "bar"}):
                pass

        loop.run_until_complete(test())

    async def test_rpc_container_autoboxing(self) -> None:
        async with ClientEventHandlerTestHelper().get_async_client(
            TestingService, port=1
        ) as client:
            # TransportError expected because not a real handler
            with self.assertRaises(TransportError):
                await client.takes_a_list([1, 2, 3])

            with self.assertRaises(TransportError):
                await client.takes_a_list(I32List([1, 2, 3]))

            with self.assertRaises(TypeError):
                await client.takes_a_list(
                    # pyre-fixme[6]: For 1st argument expected `Sequence[int]` but
                    #  got `Sequence[Union[int, str]]`.
                    [1, "b", "three"]
                )

    async def test_rpc_non_container_types(self) -> None:
        async with ClientEventHandlerTestHelper().get_async_client(
            TestingService, port=1
        ) as client:
            with self.assertRaises(TypeError):
                await client.complex_action(
                    # pyre-fixme[6]: For 1st argument expected `str` but got `bytes`.
                    b"foo",
                    "bar",
                    # pyre-fixme[6]: For 3rd argument expected `int` but got `str`.
                    "nine",
                    fourth="baz",
                )

    async def test_rpc_int_sizes(self) -> None:
        one = 2**7 - 1
        two = 2**15 - 1
        three = 2**31 - 1
        four = 2**63 - 1
        async with ClientEventHandlerTestHelper().get_async_client(
            TestingService, port=1
        ) as client:
            # TransportError expected because not a real handler
            with self.assertRaises(TransportError):
                await client.int_sizes(one, two, three, four)

            with self.assertRaises(OverflowError):
                await client.int_sizes(two, two, three, four)

            with self.assertRaises(OverflowError):
                await client.int_sizes(one, three, three, four)

            with self.assertRaises(OverflowError):
                await client.int_sizes(one, two, four, four)

            with self.assertRaises(OverflowError):
                await client.int_sizes(one, two, three, four * 10)

    def test_proxy_get_set(self) -> None:
        # Should be empty before we assign it
        self.assertEqual(get_proxy_factory(), None)

        # Should be able to assign/get a test factory
        # pyre-fixme[6]: Expected `Optional[typing.Callable[[typing.Type[thrift.py3.c...
        install_proxy_factory(ThriftClientTestProxy)
        self.assertEqual(get_proxy_factory(), ThriftClientTestProxy)

        # Should be able to unhook a factory
        install_proxy_factory(None)
        self.assertEqual(get_proxy_factory(), None)

    def test_client_event_handler(self) -> None:
        loop = asyncio.get_event_loop()
        test_helper: ClientEventHandlerTestHelper = ClientEventHandlerTestHelper()

        async def test() -> None:
            self.assertFalse(test_helper.is_handler_called())
            async with test_helper.get_async_client(TestingService, port=1) as cli:
                try:
                    await cli.getName()
                except TransportError:
                    pass
                self.assertTrue(test_helper.is_handler_called())

        loop.run_until_complete(test())

    def test_exception_in_client_event_handler(self) -> None:
        loop = asyncio.get_event_loop()

        async def test() -> None:
            with client_handler_that_throws():
                async with get_client(TestingService, port=1) as cli:
                    with self.assertRaises(RuntimeError) as cm:
                        await cli.getName()
            self.assertEqual(cm.exception.args[0], "from postWrite")

        loop.run_until_complete(test())

    def test_mockability(self) -> None:
        self.assertTrue(hasattr(TestingService, "getName"))
        self.assertTrue(hasattr(TestingService, "__aenter__"))
        self.assertTrue(hasattr(TestingService, "__aexit__"))


class RpcOptionsTests(unittest.TestCase):
    def test_write_headers(self) -> None:
        options = RpcOptions()
        headers = options.write_headers
        self.assertIsInstance(headers, WriteHeaders)
        options.set_header("test", "test")
        self.assertTrue(options.write_headers is headers)
        self.assertIn("test", headers)
        self.assertEqual(headers["test"], "test")
        with self.assertRaises(TypeError):
            # pyre-fixme[6]: Expected `str` for 2nd param but got `int`.
            options.set_header("count", 1)

    def test_timeout(self) -> None:
        options = RpcOptions()
        self.assertEqual(0, options.timeout)
        options.timeout = 0.05
        self.assertEqual(0.05, options.timeout)
        options.chunk_timeout = options.queue_timeout = options.timeout
        self.assertEqual(options.chunk_timeout, options.queue_timeout)
        with self.assertRaises(TypeError):
            # pyre-fixme[8]: Attribute has type `float`; used as `str`.
            options.timeout = "1"

    def test_priority(self) -> None:
        options = RpcOptions()
        self.assertIsInstance(options.priority, Priority)
        options.priority = Priority.HIGH
        self.assertEqual(options.priority, Priority.HIGH)
        with self.assertRaises(TypeError):
            # pyre-fixme[8]: Attribute has type `Priority`; used as `int`.
            options.priority = 1

    def test_chunk_buffer_size(self) -> None:
        options = RpcOptions()
        self.assertEqual(options.chunk_buffer_size, 100)  # default value
        options.chunk_buffer_size = 200
        self.assertEqual(options.chunk_buffer_size, 200)
        with self.assertRaises(TypeError):
            # pyre-fixme[8]: Attribute has type `int`; used as `str`.
            options.chunk_buffer_size = "1"
