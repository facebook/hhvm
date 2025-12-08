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

import unittest

from thrift.protocol import TBinaryProtocol
from thrift.transport import TTransport
from thrift.transport.THeaderTransport import THeaderTransport

from ThriftTest.ttypes import Xtruct


class TestEof(unittest.TestCase):
    def setUp(self):
        self.trans = TTransport.TMemoryBuffer()
        self.trans = THeaderTransport(self.trans)
        self.prot = TBinaryProtocol.TBinaryProtocol(self.trans)

        self.x = Xtruct()
        self.x.string_thing = "Zero"
        self.x.byte_thing = 0

    def testOversizeFrameRecv(self):
        """Test that an oversize frame on recv gets a TTransportException"""

        self.trans.set_max_frame_size(200)

        self.x.write(self.prot)
        self.trans.flush()

        inTrans = TTransport.TMemoryBuffer(self.trans.getTransport().getvalue())
        inTrans = THeaderTransport(inTrans)
        inProt = TBinaryProtocol.TBinaryProtocol(inTrans)

        inTrans.set_max_frame_size(2)

        try:
            self.x.read(inProt)
        except TTransport.TTransportException:
            return

        self.fail("Should have gotten TTransportException")

    def testOversizeFrameSend(self):
        """Test that an oversize frame on send gets a TTransportException"""
        self.trans.set_max_frame_size(2)

        self.x.write(self.prot)

        try:
            self.trans.flush()
        except TTransport.TTransportException:
            return

        self.fail("Should have gotten TTransportException")


def suite():
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    suite.addTest(loader.loadTestsFromTestCase(TestEof))
    return suite


if __name__ == "__main__":
    unittest.main(defaultTest="suite", testRunner=unittest.TextTestRunner(verbosity=2))
