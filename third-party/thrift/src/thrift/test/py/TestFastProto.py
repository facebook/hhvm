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

import sys
import unittest

from FastProto.ttypes import (
    AStruct,
    NegativeFieldId,
    OneOfEach,
    Required,
    StructWithUnion,
    TestUnion,
)
from thrift.protocol import fastproto, TBinaryProtocol, TCompactProtocol
from thrift.protocol.exceptions import ThriftUnicodeDecodeError
from thrift.transport.TTransport import TMemoryBuffer

if sys.version_info[0] >= 3:
    from thrift.util.BytesStrIO import BytesStrIO

    StringIO = BytesStrIO
else:
    from cStringIO import StringIO


class ReadOnlyBufferWithRefill(TMemoryBuffer):
    def __init__(self, index, value=None):
        self._value = value
        self._index = index
        self.refill_called = 0
        if value is None:
            self._readBuffer = StringIO(b"")
        elif index < 0 or index > len(value):
            raise RuntimeError("index overflow")
        else:
            self._readBuffer = StringIO(value[:index])

    def cstringio_refill(self, partialread, reqlen):
        self.refill_called += 1
        self._readBuffer = StringIO(partialread + self._value[self._index :])
        return self._readBuffer


class AbstractTest:
    def encode_helper(self, obj):
        buf = fastproto.encode(
            obj,
            [obj.__class__, obj.thrift_spec, obj.isUnion()],
            utf8strings=0,
            protoid=self.PROTO,
        )

        trans = TMemoryBuffer(buf)
        if self.PROTO == 0:
            proto = TBinaryProtocol.TBinaryProtocol(trans)
        else:
            proto = TCompactProtocol.TCompactProtocol(trans)

        obj_new = obj.__class__()
        obj_new.read(proto)
        self.assertEqual(obj, obj_new)

    def decode_helper(self, obj, split=1.0, utf8strings=0):
        trans = TMemoryBuffer()
        if self.PROTO == 0:
            proto = TBinaryProtocol.TBinaryProtocol(trans)
        else:
            proto = TCompactProtocol.TCompactProtocol(trans)

        obj.write(proto)
        index = int(split * len(trans.getvalue()))
        trans = ReadOnlyBufferWithRefill(index, trans.getvalue())
        obj_new = obj.__class__()
        fastproto.decode(
            obj_new,
            trans,
            [obj.__class__, obj.thrift_spec, obj.isUnion()],
            utf8strings=utf8strings,
            protoid=self.PROTO,
        )
        self.assertEqual(obj, obj_new)
        # Verify the entire buffer is read
        self.assertEqual(len(trans._readBuffer.read()), 0)
        if split != 1.0:
            self.assertEqual(1, trans.refill_called)

    def encode_and_decode(self, obj):
        trans = TMemoryBuffer()
        if self.PROTO == 0:
            proto = TBinaryProtocol.TBinaryProtocol(trans)
        else:
            proto = TCompactProtocol.TCompactProtocol(trans)

        obj.write(proto)

        obj_new = obj.__class__()
        trans = TMemoryBuffer(trans.getvalue())
        proto = proto.__class__(trans)

        obj_new.read(proto)

    def buildOneOfEach(self):
        return OneOfEach(
            aBool=True,
            aByte=1,
            anInteger16=234,
            anInteger32=12345,
            anInteger64=12345678910,
            aString="\x00hello",
            aBinary=b"\x00\x01\x00",
            aDouble=1234567.901,
            aFloat=12345.0,
            aList=[12, 34, 567, 89],
            aSet={"hello", "world", "good", "bye"},
            aMap={"hello": 1, "world": 20},
            aStruct=AStruct(aString="str", anInteger=109),
        )

    def buildOneOfEachB(self):
        return OneOfEach(
            aBool=True,
            aByte=1,
            anInteger16=234,
            anInteger32=12345,
            anInteger64=12345678910,
            aString=b"\x00hello",
            aBinary=b"\x00\x01\x00",
            aDouble=1234567.901,
            aFloat=12345.0,
            aList=[12, 34, 567, 89],
            aSet={b"hello", b"world", b"good", b"bye"},
            aMap={b"hello": 1, b"world": 20},
            aStruct=AStruct(aString=b"str", anInteger=109),
        )

    def test_encode(self):
        self.encode_helper(self.buildOneOfEach())

    def test_encode_union(self):
        u = TestUnion(i32_field=12)
        self.encode_helper(u)
        u.set_string_field("hello")
        self.encode_helper(u)
        u.set_struct_field(AStruct(aString="hello"))
        self.encode_helper(u)
        swu = StructWithUnion(aUnion=u, aString="world")
        self.encode_helper(swu)

    def test_decode(self):
        self.decode_helper(self.buildOneOfEachB())
        # Test when ensureMapBegin needs to verify the buffer has
        # at least a varint and 1 more byte.
        self.decode_helper(OneOfEach(aMap={b"h": 1}), split=0.1)

    def test_decode_union(self):
        u = TestUnion(i32_field=123)
        self.decode_helper(u)
        u.set_string_field(b"world")
        self.decode_helper(u)
        u.set_struct_field(AStruct(aString=b"world"))
        self.decode_helper(u)
        swu = StructWithUnion(aUnion=u, aString=b"world")
        self.decode_helper(swu)

    def test_refill(self):
        for idx in range(1, 100):
            self.decode_helper(self.buildOneOfEachB(), split=0.01 * idx)

    def test_negative_fid(self):
        self.encode_helper(NegativeFieldId(anInteger=20, aString="hello", aDouble=1.2))
        self.decode_helper(
            NegativeFieldId(anInteger=344444, aString=b"hello again", aDouble=1.34566)
        )

    def test_empty_container(self):
        self.encode_helper(OneOfEach(aSet=set(), aList=[], aMap={}))
        self.decode_helper(OneOfEach(aSet=set(), aList=[], aMap={}))

    def test_required(self):
        # "required" fields aren't enforced anymore and should not throw any exceptions

        aStruct = AStruct(anInteger=1)
        self.encode_and_decode(aStruct)

        aStruct = AStruct(aString="")
        required = Required(aStruct=aStruct)
        self.encode_and_decode(required)

    def test_decode_failure_lists_fieldname(self):
        obj = OneOfEach(aStruct=AStruct(aString="Привет".encode("cp866")))
        # Can't use self.assertRaises since AbstractTest cannot inherit
        # from unittest.TestCase
        raised_exception = None
        try:
            self.decode_helper(obj, split=1.0, utf8strings=1)
        except UnicodeDecodeError as ex:
            raised_exception = ex
        self.assertIsNotNone(raised_exception)
        self.assertIn("when decoding field 'aStruct->aString'", str(raised_exception))
        self.assertIn(
            "('utf-8', b'\\x8f\\xe0\\xa8\\xa2\\xa5\\xe2', 0, 1, 'invalid start byte')",
            repr(raised_exception),
        )
        self.assertTrue(isinstance(raised_exception, ThriftUnicodeDecodeError))

    def createProto(self, trans):
        if self.PROTO == 0:
            return TBinaryProtocol.TBinaryProtocol(trans)
        else:
            return TCompactProtocol.TCompactProtocol(trans)


class FastBinaryTest(AbstractTest, unittest.TestCase):
    PROTO = 0


class FastCompactTest(AbstractTest, unittest.TestCase):
    PROTO = 2


if __name__ == "__main__":
    unittest.main()
