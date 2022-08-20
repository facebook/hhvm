# Copyright (c) Facebook, Inc. and its affiliates.
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

import socket
import time
import unittest
from threading import Thread

from thrift.protocol import THeaderProtocol
from thrift.server import TCppServer
from thrift.transport import TSocket
from thrift.transport.THeaderTransport import MAX_BIG_FRAME_SIZE
from ThriftTest import ThriftTest


class TestHandler(ThriftTest.Iface):
    def testString(self, str):
        return str * 2**30


def create_server():
    processor = ThriftTest.Processor(TestHandler())
    server = TCppServer.TCppServer(processor)
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("0.0.0.0", 0))
    port = sock.getsockname()[1]
    server.setPort(port)

    t = Thread(name="test_tcpp_server", target=server.serve)
    t.setDaemon(True)
    t.start()

    time.sleep(2)

    return (server, port)


def create_client(port):
    socket = TSocket.TSocket("localhost", port)
    protocol = THeaderProtocol.THeaderProtocol(socket)
    protocol.trans.set_max_frame_size(MAX_BIG_FRAME_SIZE)
    protocol.trans.open()
    return ThriftTest.Client(protocol)


class BigFrameTest(unittest.TestCase):
    def testBigFrame(self):
        server, port = create_server()

        with create_client(port) as client:
            result = client.testString("a")
            self.assertEqual(len(result), 2**30)
