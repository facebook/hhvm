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

import glob, sys

sys.path.insert(0, "./gen-py")
lib_path = glob.glob("../../lib/py/build/lib.*")
if lib_path:
    sys.path.insert(0, lib_path[0])

if sys.version_info[0] >= 3:
    xrange = range

import time
import unittest

from thrift.protocol import TBinaryProtocol
from thrift.transport import TSocket, TTransport
from ThriftTest import ThriftTest
from ThriftTest.ttypes import Xtruct


class TestEof(unittest.TestCase):
    def setUp(self):
        trans = TTransport.TMemoryBuffer()
        prot = TBinaryProtocol.TBinaryProtocol(trans)

        x = Xtruct()
        x.string_thing = "Zero"
        x.byte_thing = 0

        x.write(prot)

        x = Xtruct()
        x.string_thing = "One"
        x.byte_thing = 1

        x.write(prot)

        self.data = trans.getvalue()

    def testTransportReadAll(self):
        """Test that readAll on any type of transport throws a
        TTransportException"""
        trans = TTransport.TMemoryBuffer(self.data)
        trans.readAll(1)

        try:
            trans.readAll(10000)
        except TTransport.TTransportException as e:
            if e.type == TTransport.TTransportException.END_OF_FILE:
                return
            raise

        self.fail("Should have gotten TTransportException")

    def eofTestHelper(self, pfactory):
        trans = TTransport.TMemoryBuffer(self.data)
        prot = pfactory.getProtocol(trans)

        x = Xtruct()
        x.read(prot)
        self.assertEqual(x.string_thing, "Zero")
        self.assertEqual(x.byte_thing, 0)

        x = Xtruct()
        x.read(prot)
        self.assertEqual(x.string_thing, "One")
        self.assertEqual(x.byte_thing, 1)

        try:
            x = Xtruct()
            x.read(prot)
        except TTransport.TTransportException as e:
            if e.type == TTransport.TTransportException.END_OF_FILE:
                return
            raise

        self.fail("Should have gotten TTransportException")

    def eofTestHelperStress(self, pfactory):
        """Test the ability of TBinaryProtocol to deal with the removal
        of every byte in the file"""
        # TODO: we should make sure this covers more of the code paths

        for i in xrange(0, len(self.data) + 1):
            trans = TTransport.TMemoryBuffer(self.data[0:i])
            prot = pfactory.getProtocol(trans)
            try:
                x = Xtruct()
                x.read(prot)
                x.read(prot)
                x.read(prot)
            except TTransport.TTransportException as e:
                if e.type == TTransport.TTransportException.END_OF_FILE:
                    continue
                raise

            self.fail("Should have gotten a TTransportException")

    def testBinaryProtocolEof(self):
        """Test that TBinaryProtocol throws an TTransportException when it
        reaches the end of the stream"""
        self.eofTestHelper(TBinaryProtocol.TBinaryProtocolFactory())
        self.eofTestHelperStress(TBinaryProtocol.TBinaryProtocolFactory())

    def testBinaryProtocolAcceleratedEof(self):
        """Test that TBinaryProtocolAccelerated throws a TTransportException
        when it reaches the end of the stream"""
        self.eofTestHelper(TBinaryProtocol.TBinaryProtocolAcceleratedFactory())
        self.eofTestHelperStress(TBinaryProtocol.TBinaryProtocolAcceleratedFactory())


def suite():
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    suite.addTest(loader.loadTestsFromTestCase(TestEof))
    return suite


if __name__ == "__main__":
    unittest.main(defaultTest="suite", testRunner=unittest.TextTestRunner(verbosity=2))
