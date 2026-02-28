#!/usr/bin/env python
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

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

import glob
import sys

sys.path.insert(0, "./gen-py")
lib_path = glob.glob("../../lib/py/build/lib.*")
if lib_path:
    sys.path.insert(0, lib_path[0])

if sys.version_info[0] >= 3:
    xrange = range

from ThriftTest import ThriftTest
from ThriftTest.ttypes import *
import random
import socket
import time
import unittest
from optparse import OptionParser

from thrift.protocol import TBinaryProtocol
from thrift.transport import TSocket, TTransport


class TimeoutTest(unittest.TestCase):
    def setUp(self):
        for i in xrange(50):
            try:
                # find a port we can use
                self.listen_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.port = random.randint(10000, 30000)
                self.listen_sock.bind(("localhost", self.port))
                self.listen_sock.listen(5)
                break
            except Exception:
                if i == 49:
                    raise

    def testConnectTimeout(self):
        starttime = time.time()

        try:
            leaky = []
            for _ in xrange(100):
                socket = TSocket.TSocket("localhost", self.port)
                socket.setTimeout(10)
                socket.open()
                leaky.append(socket)
        except Exception:
            self.assertTrue(time.time() - starttime < 5.0)

    def testWriteTimeout(self):
        starttime = time.time()

        try:
            socket = TSocket.TSocket("localhost", self.port)
            socket.setTimeout(10)
            socket.open()
            self.listen_sock.accept()
            while True:
                socket.write("hi" * 100)

        except Exception:
            self.assertTrue(time.time() - starttime < 5.0)


def suite():
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    suite.addTest(loader.loadTestsFromTestCase(TimeoutTest))
    return suite


if __name__ == "__main__":
    testRunner = unittest.TextTestRunner(verbosity=2)
    testRunner.run(suite())
