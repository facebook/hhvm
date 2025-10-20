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

from Recursive.ttypes import CoRec, CoRec2, RecList, RecTree

from thrift.protocol.TBinaryProtocol import (
    TBinaryProtocolAcceleratedFactory,
    TBinaryProtocolFactory,
)
from thrift.util.Serializer import deserialize, serialize


class AbstractTestRecursivePythonStructs:
    def test_tree(self):
        tree = RecTree()
        child = RecTree()
        tree.children = [child]
        ser = serialize(self.fac, tree)
        result = RecTree()
        result = deserialize(self.fac, ser, result)
        self.assertEqual(result, tree)

    def test_list(self):
        l = RecList()
        l2 = RecList()
        l.next = l2
        ser = serialize(self.fac, l)
        result = RecList()
        result = deserialize(self.fac, ser, result)
        self.assertIsNotNone(result.next)
        self.assertIsNone(result.next.next)

    def test_corec(self):
        c = CoRec()
        r = CoRec2()
        c.other = r
        ser = serialize(self.fac, c)
        result = CoRec()
        result = deserialize(self.fac, ser, result)
        self.assertIsNotNone(c.other)
        self.assertIsNone(c.other.other)


class TestBinary(AbstractTestRecursivePythonStructs, unittest.TestCase):
    def setUp(self):
        self.fac = TBinaryProtocolFactory()


class TestBinaryAccelerated(AbstractTestRecursivePythonStructs, unittest.TestCase):
    def setUp(self):
        self.fac = TBinaryProtocolAcceleratedFactory()


if __name__ == "__main__":
    unittest.main()
