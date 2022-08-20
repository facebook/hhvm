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

import unittest

from JsonReaderTest.ttypes import (
    Container,
    StructContainingEnum,
    StructContainingOptionalList,
    StructContainingRequiredList,
)
from thrift.protocol.TProtocol import TProtocolException


class TestJSONReader(unittest.TestCase):
    def testReadNullOptionalList(self):
        struct = StructContainingOptionalList()
        struct.readFromJson('{ "data" : null }')

    def testReadNullRequiredList(self):
        # "required" fields aren't enforced anymore and should not throw any exceptions

        struct = StructContainingRequiredList()
        struct.readFromJson('{ "data" : null }')

    def testUnrecognizedKwargs(self):
        with self.assertRaises(ValueError) as ex:
            struct = StructContainingRequiredList()
            struct.readFromJson('{ "data" : null }', bad_kwarg=True)

        self.assertIn("bad_kwarg", str(ex.exception))

    def testUnrecognizedEnum(self):
        with self.assertRaises(TProtocolException) as ex:
            struct = StructContainingEnum()
            struct.readFromJson('{ "data": 100}')

        self.assertIn(
            "100 is not a recognized value of enum type EnumZeroToTen",
            str(ex.exception),
        )

    def testUnrecognizedEnumRelaxedValidation(self):
        struct = StructContainingEnum()
        struct.readFromJson('{ "data": 100}', relax_enum_validation=True)

    def testNestedStructWithUnrecognizedEnum(self):
        json_data = '{"nested_struct": {"data": 100}}'
        struct = Container()
        with self.assertRaises(TProtocolException) as ex:
            struct.readFromJson(json_data)

        self.assertIn(
            "100 is not a recognized value of enum type EnumZeroToTen",
            str(ex.exception),
        )

        struct.readFromJson(json_data, relax_enum_validation=True)


if __name__ == "__main__":
    unittest.main()
