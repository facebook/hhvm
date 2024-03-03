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
from unittest.mock import patch

from thrift.protocol import (  # type: ignore  # noqa: F401
    fastproto,
    TBinaryProtocol,
    TCompactProtocol,
    TJSONProtocol,
    TSimpleJSONProtocol,
)
from thrift.util import Serializer

from .adapter.ttypes import Foo, FooWithoutAdapters
from .adapter_bar.ttypes import Bar


PROTOCOLS = [
    TBinaryProtocol.TBinaryProtocolFactory(),
    TBinaryProtocol.TBinaryProtocolAcceleratedFactory(),
    TCompactProtocol.TCompactProtocolFactory(),
    TCompactProtocol.TCompactProtocolAcceleratedFactory(),
    TJSONProtocol.TJSONProtocolFactory(),
    TSimpleJSONProtocol.TSimpleJSONProtocolFactory(),
]


class AdapterTest(unittest.TestCase):
    def test_roundtrip(self) -> None:
        INPUTS = {
            "empty": (Foo(), FooWithoutAdapters()),
            "default_values": (
                Foo(
                    structField={},
                    oStructField={},
                    mapField={},
                ),
                FooWithoutAdapters(
                    structField=Bar(),
                    oStructField=Bar(),
                    mapField={},
                ),
            ),
            "basic": (
                Foo(
                    structField={"field": 42},
                    oStructField={"field": 43},
                    mapField={
                        1: {"field": 44},
                        2: {"field": 45},
                    },
                ),
                FooWithoutAdapters(
                    structField=Bar(field=42),
                    oStructField=Bar(field=43),
                    mapField={
                        1: Bar(field=44),
                        2: Bar(field=45),
                    },
                ),
            ),
        }
        for protocol in PROTOCOLS:
            for name, (foo, foo_without_adapters) in INPUTS.items():
                with self.subTest(case=name, protocol=type(protocol).__name__):
                    serialized = Serializer.serialize(protocol, foo)
                    deserialized = Serializer.deserialize(protocol, serialized, Foo())
                    self.assertEqual(deserialized, foo)
                    no_adapter = Serializer.deserialize(
                        protocol, serialized, FooWithoutAdapters()
                    )
                    self.assertEqual(no_adapter, foo_without_adapters)

    def test_exception_safety(self) -> None:
        for protocol in PROTOCOLS:
            with self.subTest(protocol=type(protocol).__name__):
                foo = Foo(structField={})
                with patch(
                    "thrift.test.py.adapter_for_tests.AdapterTestStructToDict.to_thrift"
                ) as mock_to_thrift, self.assertRaises(RuntimeError):
                    mock_to_thrift.side_effect = RuntimeError()
                    Serializer.serialize(protocol, foo)

                serialized = Serializer.serialize(
                    protocol,
                    FooWithoutAdapters(structField=Bar()),
                )
                with patch(
                    "thrift.test.py.adapter_for_tests.AdapterTestStructToDict.from_thrift"
                ) as mock_from_thrift, self.assertRaises(RuntimeError):
                    mock_from_thrift.side_effect = RuntimeError()
                    Serializer.deserialize(protocol, serialized, Foo())
