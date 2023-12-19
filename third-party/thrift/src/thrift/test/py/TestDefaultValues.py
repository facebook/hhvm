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

from DefaultValuesTest import *
from DefaultValuesTest.ttypes import *

from thrift.protocol.TBinaryProtocol import *
from thrift.transport.TTransport import *


class TestDefaultValues(unittest.TestCase):
    def testReadDefaults(self):
        w = DefaultValues(def_reg=11, req_reg=22)
        r = DefaultValues()
        write_to_read(w, r)
        self.assertEqual(r.def_reg, 11)
        self.assertEqual(r.req_reg, 22)
        self.assertEqual(r.def_val, 12)
        self.assertEqual(r.req_val, 34)
        self.assertEqual(r.opt_val, 56)
        self.assertIsNone(r.opt_list_reg)
        self.assertEqual(r.opt_list_val, [56, 78, 90])

    def testDefaultsAreNotOnTheWire(self):
        w = DefaultValues(def_reg=11, req_reg=22)
        r = DefaultValues(def_reg=0, opt_reg=33, opt_val=44)
        write_to_read(w, r)
        self.assertEqual(r.def_reg, 11)
        self.assertEqual(r.opt_reg, 33)
        self.assertEqual(r.opt_val, 44, "defaults are transmitted on the wire")

    def testCompoundDefaults(self):
        w = DefaultValues(def_reg=11, req_reg=22)
        r = DefaultValues(def_reg=0, req_reg=33, opt_list_val=[11, 22, 33])
        write_to_read(w, r)
        self.assertEqual(
            r.opt_list_val, [11, 22, 33], "defaults are transmitted on the wire"
        )


def write_to_read(write_struct, read_struct):
    write_buffer = TMemoryBuffer()
    write_protocol = TBinaryProtocol(write_buffer)
    write_struct.write(write_protocol)

    # The implementation of TMemoryBuffer is slightly different from C++
    # the read/write buffer is not shared, thus we have to create another
    # TMemoryBuffer
    read_buffer = TMemoryBuffer(write_buffer.getvalue())
    read_protocol = TBinaryProtocol(read_buffer)
    read_struct.read(read_protocol)


if __name__ == "__main__":
    unittest.main()
