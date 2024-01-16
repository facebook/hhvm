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

from __future__ import absolute_import, division, print_function, unicode_literals

import unittest

from thrift.protocol import THeaderProtocol
from thrift.server.test.slots_throwing_service import SlotsThrowingService
from thrift.server.test.slots_throwing_service.ttypes import UserException2
from thrift.Thrift import TApplicationException
from thrift.transport import THeaderTransport, TSocket
from thrift.util.TCppServerTestManager import TCppServerTestManager


class SlotsServiceThrowsTest(unittest.TestCase):
    class Handler(SlotsThrowingService.Iface):
        def throwUserException(self):
            raise UserException2("Some message")

        def throwUncaughtException(self, msg):
            raise AssertionError(msg)

    def _perform_rpc(self, server, val):
        host, port = server.addr()
        with TSocket.TSocket(host=host, port=port) as sock:
            transport = THeaderTransport.THeaderTransport(sock)
            protocol = THeaderProtocol.THeaderProtocol(transport)
            client = SlotsThrowingService.Client(protocol, protocol)
            return client.getDataById(val)

    def test_throw_populates_headers(self):
        handler = self.Handler()
        processor = SlotsThrowingService.Processor(handler)
        server = TCppServerTestManager.make_server(processor)
        with TCppServerTestManager(server) as server:
            host, port = server.addr()
            with TSocket.TSocket(host=host, port=port) as sock:
                transport = THeaderTransport.THeaderTransport(sock)
                protocol = THeaderProtocol.THeaderProtocol(transport)
                client = SlotsThrowingService.Client(protocol, protocol)

                try:
                    client.throwUserException()
                    self.fail("Expect to throw UserException2")
                except UserException2:
                    pass

                self.assertEqual(b"UserException2", transport.get_headers()[b"uex"])
                self.assertIn(b"Some message", transport.get_headers()[b"uexw"])

                try:
                    client.throwUncaughtException("a message!")
                    self.fail("Expect to throw TApplicationException")
                except TApplicationException:
                    pass

                self.assertEqual(
                    b"TApplicationException", transport.get_headers()[b"uex"]
                )
                self.assertIn(b"a message!", transport.get_headers()[b"uexw"])
