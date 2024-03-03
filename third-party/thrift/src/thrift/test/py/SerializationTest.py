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

from __future__ import absolute_import, division, print_function, unicode_literals

import glob, json, sys

sys.path.insert(0, "./gen-py")
lib_path = glob.glob("../../lib/py/build/lib.*")
if lib_path:
    sys.path.insert(0, lib_path[0])

from ThriftTest.ttypes import *
import unittest

from thrift.protocol import (
    TBinaryProtocol,
    TCompactProtocol,
    THeaderProtocol,
    TJSONProtocol,
    TSimpleJSONProtocol,
)
from thrift.transport import TSocket, TTransport
from thrift.util import Serializer


def bytes_comp(ut, seq1, seq2):
    if not isinstance(seq1, bytes):
        seq1 = seq1.encode("utf-8")
    if not isinstance(seq2, bytes):
        seq2 = seq2.encode("utf-8")
    ut.assertEquals(seq1, seq2)


class AbstractTest:
    def setUp(self):
        self.v1obj = VersioningTestV1(
            begin_in_both=12345,
            old_string="aaa",
            end_in_both=54321,
        )

        self.v2obj = VersioningTestV2(
            begin_in_both=12345,
            newint=1,
            newbyte=2,
            newshort=3,
            newlong=4,
            newdouble=5.5,
            newstruct=Bonk(message="Hello!", type=123),
            newlist=[7, 8, 9],
            newset=[42, 1, 8],
            newmap={1: 2, 2: 3},
            newstring="Hola!",
            # json cannot serialize bytes in python 3
            newunicodestring=(
                "any\x7f\xff".encode("utf-8")
                if sys.version_info[0] < 3
                else "any\x7f\xff"
            ),
            newbool=True,
            end_in_both=54321,
        )

        self.sjtObj = SimpleJSONTestStruct(
            m={
                1: self.v1obj,
                2: self.v2obj,
            },
        )

    def _serialize(self, obj):
        return Serializer.serialize(self.protocol_factory, obj)

    def _deserialize(self, objtype, data):
        return Serializer.deserialize(self.protocol_factory, data, objtype())

    def testSimpleJSON(self):
        if not isinstance(self, SimpleJSONTest):
            return
        json.loads(self._serialize(self.sjtObj))
        json.loads(self._serialize(self.v1obj))
        json.loads(self._serialize(self.v2obj))

    def testForwards(self):
        if isinstance(self, SimpleJSONTest):
            return
        obj = self._deserialize(VersioningTestV2, self._serialize(self.v1obj))
        self.assertEqual(obj.begin_in_both, self.v1obj.begin_in_both)
        self.assertEqual(obj.end_in_both, self.v1obj.end_in_both)

    def testUnicodeString(self):
        if isinstance(self, SimpleJSONTest):
            return
        obj = self._deserialize(VersioningTestV2, self._serialize(self.v2obj))
        bytes_comp(self, obj.newunicodestring, self.v2obj.newunicodestring)

    def testBackwards(self):
        if isinstance(self, SimpleJSONTest):
            return
        obj = self._deserialize(VersioningTestV1, self._serialize(self.v2obj))
        self.assertEqual(obj.begin_in_both, self.v2obj.begin_in_both)
        self.assertEqual(obj.end_in_both, self.v2obj.end_in_both)

    def testDouble(self):
        if isinstance(self, SimpleJSONTest):
            return
        obj = self._deserialize(VersioningTestV2, self._serialize(self.v2obj))
        self.assertEqual(obj.newdouble, self.v2obj.newdouble)


class NormalBinaryTest(AbstractTest, unittest.TestCase):
    protocol_factory = TBinaryProtocol.TBinaryProtocolFactory()


class AcceleratedBinaryTest(AbstractTest, unittest.TestCase):
    protocol_factory = TBinaryProtocol.TBinaryProtocolAcceleratedFactory()


class CompactTest(AbstractTest, unittest.TestCase):
    protocol_factory = TCompactProtocol.TCompactProtocolFactory()


class AcceleratedCompactTest(AbstractTest, unittest.TestCase):
    protocol_factory = TCompactProtocol.TCompactProtocolAcceleratedFactory()


class AcceleratedFramedTest(unittest.TestCase):
    def testSplit(self):
        """Test FramedTransport and BinaryProtocolAccelerated

        Tests that TBinaryProtocolAccelerated and TFramedTransport
        play nicely together when a read spans a frame"""

        protocol_factory = TBinaryProtocol.TBinaryProtocolAcceleratedFactory()
        bigstring = "".join(chr(byte) for byte in range(ord("a"), ord("z") + 1))

        databuf = TTransport.TMemoryBuffer()
        prot = protocol_factory.getProtocol(databuf)
        prot.writeI32(42)
        prot.writeString(bigstring)
        prot.writeI16(24)
        data = databuf.getvalue()
        cutpoint = len(data) // 2
        parts = [data[:cutpoint], data[cutpoint:]]

        framed_buffer = TTransport.TMemoryBuffer()
        framed_writer = TTransport.TFramedTransport(framed_buffer)
        for part in parts:
            framed_writer.write(part)
            framed_writer.flush()
        self.assertEqual(len(framed_buffer.getvalue()), len(data) + 8)

        # Recreate framed_buffer so we can read from it.
        framed_buffer = TTransport.TMemoryBuffer(framed_buffer.getvalue())
        framed_reader = TTransport.TFramedTransport(framed_buffer)
        prot = protocol_factory.getProtocol(framed_reader)
        self.assertEqual(prot.readI32(), 42)
        bytes_comp(self, prot.readString(), bigstring)
        self.assertEqual(prot.readI16(), 24)


class SimpleJSONTest(AbstractTest):
    protocol_factory = TSimpleJSONProtocol.TSimpleJSONProtocolFactory()


class JSONProtocolTest(AbstractTest, unittest.TestCase):
    protocol_factory = TJSONProtocol.TJSONProtocolFactory()

    def testNestedStructs(self):
        nested = NestedStructs(bonk=Bonk(), bools=Bools())
        json.loads(self._serialize(nested))
        # Old protocol should be able to deserialize both valid and invalid
        # JSON.
        protocol_factory = TJSONProtocol.TJSONProtocolFactory(validJSON=False)
        Serializer.deserialize(
            protocol_factory, '{"1":{"rec":{}}"2":{"rec":{}}}', NestedStructs()
        )
        Serializer.deserialize(
            protocol_factory, '{"1":{"rec":{}},"2":{"rec":{}}}', NestedStructs()
        )


class HeaderDefaultFactory(THeaderProtocol.THeaderProtocolFactory):
    def __init__(self, default_protocol):
        super(HeaderDefaultFactory, self).__init__()
        self.defaultProtocol = default_protocol

    def getProtocol(self, trans):
        proto = super(HeaderDefaultFactory, self).getProtocol(trans)
        proto.trans.set_protocol_id(self.defaultProtocol)
        proto.reset_protocol()
        return proto


class HeaderTest(AbstractTest):
    def _serialize(self, obj):
        return Serializer.serialize(self.serialize_factory, obj)

    def _deserialize(self, objtype, data):
        return Serializer.deserialize(self.deserialize_factory, data, objtype())


class HeaderCompactToCompactTest(HeaderTest, unittest.TestCase):
    serialize_factory = deserialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL
    )


class HeaderBinaryToBinaryTest(HeaderTest, unittest.TestCase):
    serialize_factory = deserialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL
    )


class HeaderCompactToBinaryTest(HeaderTest, unittest.TestCase):
    serialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL
    )
    deserialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL
    )


class HeaderBinaryToCompactTest(HeaderTest, unittest.TestCase):
    serialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL
    )
    deserialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL
    )


class HeaderBinaryToDefault(HeaderTest, unittest.TestCase):
    serialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_BINARY_PROTOCOL
    )
    deserialize_factory = THeaderProtocol.THeaderProtocolFactory()


class HeaderCompactToDefault(HeaderTest, unittest.TestCase):
    serialize_factory = HeaderDefaultFactory(
        THeaderProtocol.THeaderProtocol.T_COMPACT_PROTOCOL
    )
    deserialize_factory = THeaderProtocol.THeaderProtocolFactory()


def suite():
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()

    test_classes = (
        NormalBinaryTest,
        AcceleratedBinaryTest,
        AcceleratedFramedTest,
        CompactTest,
        SimpleJSONTest,
        JSONProtocolTest,
        HeaderCompactToCompactTest,
        HeaderBinaryToBinaryTest,
        HeaderBinaryToCompactTest,
        HeaderCompactToBinaryTest,
    )
    for clazz in test_classes:
        suite.addTest(loader.loadTestsFromTestCase(clazz))

    return suite


if __name__ == "__main__":
    unittest.main(defaultTest="suite", testRunner=unittest.TextTestRunner(verbosity=2))
