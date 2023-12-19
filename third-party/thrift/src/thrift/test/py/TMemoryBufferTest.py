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

import unittest

from thrift.transport.TTransport import TMemoryBuffer


class TestTMemoryBuffer(unittest.TestCase):
    def testReadWrite(self):
        buf = TMemoryBuffer(b"hello")
        data = buf.read(5)
        buf.write(b"world")
        self.assertEqual(data, b"hello")
        self.assertEqual(buf.getvalue(), b"world")

    def testNoInitialValue(self):
        buf = TMemoryBuffer()
        data = buf.read(5)
        buf.write(b"world")
        self.assertEqual(data, b"")
        self.assertEqual(buf.getvalue(), b"world")

    def testClose(self):
        buf = TMemoryBuffer(b"hello")
        buf.close()
        self.assertRaises(RuntimeError, buf.read, 5)
        self.assertRaises(RuntimeError, buf.write, b"world")


if __name__ == "__main__":
    unittest.main()
