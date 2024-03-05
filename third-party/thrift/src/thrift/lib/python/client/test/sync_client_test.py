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

import time
import unittest

from thrift.lib.python.client.test.event_handler_helper import (
    addEventHandler,
    client_handler_that_throws,
)

from thrift.python.client import ClientType, get_sync_client
from thrift.python.common import RpcOptions
from thrift.python.exceptions import (
    ApplicationError,
    ApplicationErrorType,
    TransportError,
    TransportErrorType,
)
from thrift.python.leaf.thrift_clients import LeafService
from thrift.python.serializer import Protocol
from thrift.python.test.test_server import server_in_another_process
from thrift.python.test.thrift_clients import EchoService, TestService
from thrift.python.test.thrift_types import ArithmeticException, EmptyException

from .exceptions_helper import HijackTestException, HijackTestHelper


TEST_HEADER_KEY = "headerKey"
TEST_HEADER_VALUE = "headerValue"


class SyncClientTests(unittest.TestCase):
    def test_basic(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertEqual(3, client.add(1, 2))

    def test_client_type_and_protocol(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(
                TestService,
                path=path,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
                protocol=Protocol.BINARY,
            ) as client:
                sum = client.add(1, 2)
                self.assertEqual(3, sum)

    def test_void_return(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertIsNone(client.noop())

    def test_exception(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertAlmostEqual(2, client.divide(6, 3))
                with self.assertRaises(ArithmeticException):
                    client.divide(1, 0)

    def test_void_return_with_exception(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                with self.assertRaises(EmptyException):
                    client.oops()

    def test_oneway(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertIsNone(client.oneway())
                time.sleep(1)  # wait for server to clear the queue

    def test_oneway_with_rocket(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(
                TestService, path=path, client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE
            ) as client:
                self.assertIsNone(client.oneway())
                time.sleep(1)  # wait for server to clear the queue

    def test_unexpected_exception(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                with self.assertRaisesRegex(ApplicationError, "Surprise!") as ex:
                    client.surprise()
                self.assertEqual(ApplicationErrorType.UNKNOWN, ex.exception.type)

    def test_derived_service(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(EchoService, path=path) as client:
                self.assertEqual("hello", client.echo("hello"))
                self.assertEqual(3, client.add(1, 2))

    def test_deriving_from_external_service(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(LeafService, path=path) as client:
                self.assertEqual([3, 2, 1], list(client.reverse([1, 2, 3])))
                self.assertEqual("hello", client.echo("hello"))
                self.assertEqual(3, client.add(1, 2))

    def test_transport_error(self) -> None:
        with get_sync_client(TestService, path="/no/where") as client:
            with self.assertRaises(TransportError) as ex:
                client.add(1, 2)
            self.assertEqual(TransportErrorType.UNKNOWN, ex.exception.type)

    def test_persistent_header(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                client.set_persistent_header(TEST_HEADER_KEY, TEST_HEADER_VALUE)
                self.assertEqual(TEST_HEADER_VALUE, client.readHeader(TEST_HEADER_KEY))

    def test_reuse_client(self) -> None:
        with server_in_another_process() as path:
            client = get_sync_client(TestService, path=path)
            with client:
                self.assertEqual(3, client.add(1, 2))
            with self.assertRaises(RuntimeError):
                with client:
                    pass
            with self.assertRaises(RuntimeError):
                client.add(1, 2)

    def test_add_test_handler_with_rpc_options_should_hijack_transport_error_and_use_rpc_options(
        self,
    ) -> None:
        with HijackTestHelper():
            with get_sync_client(TestService, path="/no/where") as client:
                with self.assertRaises(HijackTestException) as context:
                    options = RpcOptions()
                    options.timeout = 12.5
                    client.add(1, 2, rpc_options=options)
                self.assertEqual(context.exception.timeout, 12.5)

    def test_add_test_handler_without_rpc_options_should_hijack_transport_error(
        self,
    ) -> None:
        with HijackTestHelper():
            with get_sync_client(TestService, path="/no/where") as client:
                with self.assertRaises(HijackTestException) as context:
                    client.add(1, 2)
                self.assertEqual(context.exception.timeout, 0.0)

    def test_exit_callback(self) -> None:
        class Callback:
            def __init__(self):
                self.triggered = False

            def trigger(self):
                self.triggered = True

        cb1 = Callback()
        cb2 = Callback()

        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                client._at_exit(cb1.trigger)
                client._at_exit(cb2.trigger)

        self.assertTrue(cb1.triggered)
        self.assertTrue(cb2.triggered)

    def test_exception_in_client_event_handler(self) -> None:
        with self.assertRaises(RuntimeError):
            with client_handler_that_throws():
                with server_in_another_process() as path:
                    with get_sync_client(TestService, path=path) as client:
                        self.assertEqual(3, client.add(1, 2))

    def test_no_exception_when_clear_event_handler(self) -> None:
        addEventHandler()
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                client.clear_event_handlers()
                self.assertEqual(3, client.add(1, 2))
