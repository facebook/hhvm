# Copyright (c) Facebook, Inc. and its affiliates.
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

import textwrap
import unittest

# The sorted one.
from SortKeys.ttypes import NegativeId, SortedStruct
from SortSets.ttypes import SortedSetStruct
from thrift.protocol import TSimpleJSONProtocol
from thrift.transport.TTransport import TMemoryBuffer


def writeToJSON(obj):
    trans = TMemoryBuffer()
    proto = TSimpleJSONProtocol.TSimpleJSONProtocol(trans)
    obj.write(proto)
    return trans.getvalue()


def readStructFromJSON(jstr, struct_type):
    stuff = struct_type()
    trans = TMemoryBuffer(jstr)
    proto = TSimpleJSONProtocol.TSimpleJSONProtocol(trans, struct_type.thrift_spec)
    stuff.read(proto)
    return stuff


class TestSortKeys(unittest.TestCase):
    def testSorted(self):
        static_struct = SortedStruct(aMap={"b": 1.0, "a": 1.0})
        unsorted_blob = b'{\n  "aMap": {\n    "b": 1.0,\n    "a": 1.0\n  }\n}'
        sorted_blob = b'{\n  "aMap": {\n    "a": 1.0,\n    "b": 1.0\n  }\n}'
        sorted_struct = readStructFromJSON(unsorted_blob, SortedStruct)
        blob = writeToJSON(sorted_struct)
        self.assertNotEqual(blob, unsorted_blob)
        self.assertEqual(blob, sorted_blob)
        self.assertEqual(static_struct, sorted_struct)

    def testSetSorted(self):
        unsorted_set = set(["5", "4", "3", "2", "1", "0"])
        static_struct = SortedSetStruct(aSet=unsorted_set)
        unsorted_blob = (
            textwrap.dedent(
                """\
            {{
              "aSet": [
                "{}"
              ]
            }}"""
            )
            .format('",\n    "'.join(unsorted_set))
            .encode()
        )
        sorted_blob = (
            textwrap.dedent(
                """\
            {{
              "aSet": [
                "{}"
              ]
            }}"""
            )
            .format('",\n    "'.join(sorted(unsorted_set)))
            .encode()
        )
        sorted_struct = readStructFromJSON(unsorted_blob, SortedSetStruct)
        blob = writeToJSON(sorted_struct)
        self.assertNotEqual(blob, unsorted_blob)
        self.assertEqual(blob, sorted_blob)
        self.assertEqual(static_struct, sorted_struct)

    def testNegativeId(self):
        obj = NegativeId()
        self.assertEqual(obj.field1, 1)
        self.assertEqual(obj.field2, 2)
        self.assertEqual(obj.field3, 3)
