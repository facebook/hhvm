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

# pyre-strict

import unittest

import test.fixtures.hidden_test.thrift_types as python_types
import test.fixtures.hidden_test.ttypes as py_types
import thrift.python.serializer as python_serializer
from thrift.protocol import TCompactProtocol
from thrift.util import Serializer


class HiddenTest(unittest.TestCase):
    def test_hidden_field(self) -> None:
        s1 = py_types.S1(normalField=1)

        # Make sure hidden field is not visible
        self.assertFalse(hasattr(s1, "mapField"))

        # Make sure hidden field cannot be set
        with self.assertRaises(TypeError):
            py_types.S1(normalField=1, mapField={py_types.StructuredKey(): 1})

    def test_serde_py_to_py(self) -> None:
        s1 = py_types.S1(normalField=1)
        dst = py_types.S1()
        bytes = Serializer.serialize(TCompactProtocol.TCompactProtocolFactory(), s1)
        Serializer.deserialize(TCompactProtocol.TCompactProtocolFactory(), bytes, dst)
        self.assertEqual(s1, dst)

    def test_serde_py_to_python(self) -> None:
        s1 = py_types.S1(normalField=1)
        bytes = Serializer.serialize(TCompactProtocol.TCompactProtocolFactory(), s1)
        self.assertEqual(
            python_types.S1(normalField=1),
            python_serializer.deserialize(python_types.S1, bytes),
        )

    def test_serde_python_to_py(self) -> None:
        s1 = python_types.S1(normalField=1)
        bytes = python_serializer.serialize(s1)
        dst = py_types.S1()
        Serializer.deserialize(TCompactProtocol.TCompactProtocolFactory(), bytes, dst)
        self.assertEqual(py_types.S1(normalField=1), dst)

    def test_json_serde(self) -> None:
        s1 = py_types.S1(normalField=1)
        # make sure we can deserialize S1 from JSON
        dst = py_types.S1()
        dst.readFromJson('{"normalField": 1}')
        self.assertEqual(s1, dst)

        dst.readFromJson('{"normalField": 1, "mapField": {}}')
        self.assertEqual(s1, dst)
        self.assertFalse(hasattr(dst, "mapField"))

    def test_py_to_python(self) -> None:
        s1 = py_types.S1(normalField=1)
        dst = s1._to_python()
        self.assertEqual(python_types.S1(normalField=1), dst)

    def test_python_to_py(self) -> None:
        s1 = python_types.S1(normalField=1, mapField={python_types.StructuredKey(): 1})
        dst = s1._to_py_deprecated()
        self.assertEqual(py_types.S1(normalField=1), dst)
        with self.assertRaisesRegex(
            AttributeError, "'S1' object has no attribute 'mapField'"
        ):
            getattr(dst, "mapField")  # noqa
