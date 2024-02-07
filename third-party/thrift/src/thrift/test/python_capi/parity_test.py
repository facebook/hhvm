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


import unittest

import thrift.test.python_capi.parity as parity

from thrift.python.serializer import deserialize, Protocol, serialize


class ParityFixture(unittest.TestCase):
    def assert_proto_equal(self, serial: object, marshal: object) -> None:
        for proto in [Protocol.COMPACT, Protocol.BINARY, Protocol.JSON]:
            serialized_marshal = serialize(marshal, proto)
            self.assertEqual(serialized_marshal, serialize(serial, proto))
            self.assertEqual(
                serial, deserialize(serial.__class__, serialized_marshal, proto)
            )

    def assert_struct_optional(self, set_optional: bool) -> None:
        serial = parity.make_serialized_struct(set_optional)
        marshal = parity.make_marshal_struct(set_optional)
        self.assert_proto_equal(serial, marshal)

    def assert_union(self, set_string: bool) -> None:
        serial = parity.make_serialized_union(set_string)
        marshal = parity.make_marshal_union(set_string)
        self.assert_proto_equal(serial, marshal)

    def assert_error_optional(self, set_optional: bool) -> None:
        serial = parity.make_serialized_error(set_optional)
        marshal = parity.make_marshal_error(set_optional)
        self.assert_proto_equal(serial, marshal)


class ParityTest(ParityFixture):
    def test_struct_parity_unset_optional(self) -> None:
        self.assert_struct_optional(set_optional=False)

    def test_struct_parity_set_optional(self) -> None:
        self.assert_struct_optional(set_optional=True)

    def test_union_parity(self) -> None:
        self.assert_union(set_string=False)
        self.assert_union(set_string=True)

    def test_error_parity_unset_optional(self) -> None:
        self.assert_error_optional(set_optional=False)

    def test_error_parity_set_optional(self) -> None:
        self.assert_error_optional(set_optional=True)

    def test_unset_struct(self) -> None:
        serial = parity.make_unset_struct_serialized()
        marshal = parity.make_unset_struct_marshal()
        self.assert_proto_equal(serial, marshal)

    def test_unset_union(self) -> None:
        serial = parity.make_unset_union_serialized()
        marshal = parity.make_unset_union_marshal()
        self.assert_proto_equal(serial, marshal)

    def test_unset_error(self) -> None:
        serial = parity.make_unset_error_serialized()
        marshal = parity.make_unset_error_marshal()
        self.assert_proto_equal(serial, marshal)
