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

import sys
import unittest

from thrift.protocol import TBinaryProtocol, TCompactProtocol
from thrift.transport import TSocket, TTransport
from thrift.util import Serializer
from ThriftTest.ttypes import ListTypeVersioningV2

if sys.version_info[0] < 3:
    chr = unichr  # noqa: F821 - Python 2 built-in, dead code in Python 3


class AbstractTest:
    def setUp(self):
        self.obj = ListTypeVersioningV2(
            strings=["plain thing", chr(40960) + "fun" + chr(1972)],
            hello="hello\xac\u1234\u20ac\U00008000",
        )

    def _serialize(self, obj):
        return Serializer.serialize(self.protocol_factory, obj)

    def _deserialize(self, objtype, data):
        return Serializer.deserialize(self.protocol_factory, data, objtype())

    def testUnicodeString(self):
        obj2 = self._deserialize(ListTypeVersioningV2, self._serialize(self.obj))
        self.assertEqual(obj2.strings[0], self.obj.strings[0])
        self.assertEqual(obj2.strings[1], self.obj.strings[1])
        self.assertEqual(obj2.hello, self.obj.hello)


class NormalBinaryTest(AbstractTest, unittest.TestCase):
    protocol_factory = TBinaryProtocol.TBinaryProtocolFactory()


class AcceleratedBinaryTest(AbstractTest, unittest.TestCase):
    protocol_factory = TBinaryProtocol.TBinaryProtocolAcceleratedFactory()


class NormalCompactTest(AbstractTest, unittest.TestCase):
    protocol_factory = TCompactProtocol.TCompactProtocolFactory()


class AcceleratedCompactTest(AbstractTest, unittest.TestCase):
    protocol_factory = TCompactProtocol.TCompactProtocolAcceleratedFactory()


def suite():
    suite = unittest.TestSuite()
    loader = unittest.TestLoader()

    suite.addTest(loader.loadTestsFromTestCase(NormalBinaryTest))
    suite.addTest(loader.loadTestsFromTestCase(AcceleratedBinaryTest))
    suite.addTest(loader.loadTestsFromTestCase(NormalCompactTest))
    suite.addTest(loader.loadTestsFromTestCase(AcceleratedCompactTest))

    return suite


if __name__ == "__main__":
    unittest.main(defaultTest="suite", testRunner=unittest.TextTestRunner(verbosity=2))
