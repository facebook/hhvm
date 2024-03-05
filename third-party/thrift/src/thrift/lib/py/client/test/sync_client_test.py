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

import unittest

from thrift.py.client.common import ClientType, Protocol
from thrift.py.client.sync_client_factory import get_client
from thrift.py.test import TestService
from thrift.py.test.ttypes import ArithmeticException
from thrift.python.test.test_server import server_in_another_process
from thrift.Thrift import TApplicationException
from thrift.transport.TTransport import TTransportException


class SyncClientTests(unittest.TestCase):
    def test_basic(self) -> None:
        with server_in_another_process() as path:
            with get_client(TestService.Client, path=path) as client:
                self.assertEqual(3, client.add(1, 2))

    def test_client_type_and_protocol(self) -> None:
        with server_in_another_process() as path:
            with get_client(
                TestService.Client,
                path=path,
                client_type=ClientType.THRIFT_ROCKET_CLIENT_TYPE,
                protocol=Protocol.BINARY,
            ) as client:
                sum = client.add(1, 2)
                self.assertEqual(3, sum)

    def test_persistent_header(self) -> None:
        with server_in_another_process() as path:
            with get_client(TestService.Client, path=path) as client:
                client.set_persistent_header(
                    "PERSISTENT_HEADER_KEY", "PERSISTENT_HEADER_VALUE"
                )
                self.assertEqual(
                    "PERSISTENT_HEADER_VALUE",
                    client.get_persistent_headers()["PERSISTENT_HEADER_KEY"],
                )
                self.assertEqual(
                    "PERSISTENT_HEADER_VALUE",
                    client.readHeader("PERSISTENT_HEADER_KEY"),
                )
                client.clear_persistent_headers()
                self.assertFalse(client.get_persistent_headers())
                self.assertEqual(
                    "",
                    client.readHeader("PERSISTENT_HEADER_KEY"),
                )

    def test_onetime_header(self) -> None:
        with server_in_another_process() as path:
            with get_client(TestService.Client, path=path) as client:
                client.set_onetime_header("ONETIME_HEADER_KEY", "ONETIME_HEADER_VALUE")
                self.assertEqual(
                    "ONETIME_HEADER_VALUE",
                    client.readHeader("ONETIME_HEADER_KEY"),
                )
                self.assertEqual(
                    "",
                    client.readHeader("ONETIME_HEADER_KEY"),
                )

    def test_onetime_header_override_persistent_header(self) -> None:
        with server_in_another_process() as path:
            with get_client(TestService.Client, path=path) as client:
                client.set_persistent_header("HEADER_KEY", "PERSISTENT_HEADER_VALUE")
                self.assertEqual(
                    "PERSISTENT_HEADER_VALUE",
                    client.readHeader("HEADER_KEY"),
                )
                client.set_onetime_header("HEADER_KEY", "ONETIME_HEADER_VALUE")
                self.assertEqual(
                    "ONETIME_HEADER_VALUE",
                    client.readHeader("HEADER_KEY"),
                )
                self.assertEqual(
                    "PERSISTENT_HEADER_VALUE",
                    client.get_persistent_headers()["HEADER_KEY"],
                )
                self.assertEqual(
                    "PERSISTENT_HEADER_VALUE",
                    client.readHeader("HEADER_KEY"),
                )

    def test_reuse_client(self) -> None:
        with server_in_another_process() as path:
            client = get_client(TestService.Client, path=path)
            with client:
                self.assertEqual(3, client.add(1, 2))
            with self.assertRaises(RuntimeError):
                with client:
                    pass
            with self.assertRaises(RuntimeError):
                client.add(1, 2)

    def test_exception(self) -> None:
        with server_in_another_process() as path:
            with get_client(TestService.Client, path=path) as client:
                self.assertAlmostEqual(2, client.divide(6, 3))
                with self.assertRaises(ArithmeticException):
                    client.divide(1, 0)

    def test_unexpected_exception(self) -> None:
        with server_in_another_process() as path:
            with get_client(TestService.Client, path=path) as client:
                with self.assertRaisesRegex(TApplicationException, "Surprise!") as ex:
                    client.surprise()
                self.assertEqual(TApplicationException.UNKNOWN, ex.exception.type)

    def test_transport_error(self) -> None:
        with get_client(TestService.Client, path="/no/where") as client:
            with self.assertRaises(TTransportException) as ex:
                client.add(1, 2)
            self.assertEqual(TTransportException.UNKNOWN, ex.exception.type)
