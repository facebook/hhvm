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

import json
import unittest

from thrift.protocol import TBinaryProtocol, TSimpleJSONProtocol
from thrift.transport import TTransport
from thrift.test.UnionTest.ttypes import *


class TestUnionStructs(unittest.TestCase):
    def test_init(self):
        u = TestUnion()
        self.assertEqual(TestUnion.__EMPTY__, u.getType())

        v = TestUnion(string_field="test")
        self.assertEqual(TestUnion.STRING_FIELD, v.getType())
        self.assertEqual("test", v.value)
        self.assertNotEqual(u, v)

        try:
            TestUnion(string_field="test", i32_field=100)
            self.assertTrue(False, "Cannot initialize union with 1+ fields")
        except Exception:
            pass

    def test_get_set(self):
        u = TestUnion()
        u.set_i32_field(10)
        self.assertEqual(10, u.get_i32_field())

        v = TestUnion(i32_field=10)
        self.assertEqual(u, v)

        self.assertRaises(AssertionError, u.get_other_i32_field)

    def _test_json(self, j, v):
        u = TestUnion()
        u.readFromJson(json.dumps(j))
        self.assertEqual(v, u)

    def test_json(self):
        v = TestUnion(i32_field=123)
        j = {"i32_field": 123}
        self._test_json(j, v)

        v = TestUnion(string_field="test")
        j = {"string_field": "test"}
        self._test_json(j, v)

    def test_repr(self):
        """Ensure that __repr__() return a valid expression that can be
        used to construct the original object
        """
        v = TestUnion(i32_field=123)
        self.assertEqual(v, eval(v.__repr__()))

        v = TestUnion()
        self.assertEqual(v, eval(v.__repr__()))

        v = TestUnion(string_field="test")
        self.assertEqual(v, eval(v.__repr__()))

    def _test_read_write(self, u, j):
        protocol_factory = TBinaryProtocol.TBinaryProtocolAcceleratedFactory()
        databuf = TTransport.TMemoryBuffer()
        prot = protocol_factory.getProtocol(databuf)
        u.write(prot)

        ndatabuf = TTransport.TMemoryBuffer(databuf.getvalue())
        prot = protocol_factory.getProtocol(ndatabuf)
        v = u.__class__()
        v.read(prot)
        self.assertEqual(v, j)

    def test_read_write(self):
        l = [
            (TestUnion(string_field="test"), TestUnion(string_field="test")),
            (TestUnion(), TestUnion()),
            (TestUnion(i32_field=100), TestUnion(i32_field=100)),
            (
                StructWithUnionAndOther(TestUnion(i32_field=100), "test"),
                StructWithUnionAndOther(TestUnion(i32_field=100), "test"),
            ),
        ]

        for i, j in l:
            self._test_read_write(i, j)

    def _test_json_output(self, u, j):
        protocol_factory = TSimpleJSONProtocol.TSimpleJSONProtocolFactory()
        databuf = TTransport.TMemoryBuffer()
        prot = protocol_factory.getProtocol(databuf)
        u.write(prot)

        self.assertEqual(j, json.loads(databuf.getvalue().decode()))

    def test_json_output(self):
        l = [
            (TestUnion(), {}),
            (TestUnion(i32_field=10), {"i32_field": 10}),
            (TestUnion(string_field="test"), {"string_field": "test"}),
            (
                StructWithUnionAndOther(TestUnion(i32_field=10), "test"),
                {"test_union": {"i32_field": 10}, "string_field": "test"},
            ),
        ]

        for i, j in l:
            self._test_json_output(i, j)

    def testIsUnion(self):
        self.assertFalse(OneOfEach.isUnion())
        self.assertTrue(TestUnion.isUnion())


if __name__ == "__main__":
    unittest.main()
