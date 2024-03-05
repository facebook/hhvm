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


from __future__ import annotations

import typing
import unittest

import testing.thrift_types

from apache.thrift.type.standard.thrift_types import StandardProtocol, TypeName
from apache.thrift.type.type.thrift_types import Protocol
from folly.iobuf import IOBuf
from thrift.python.any.any_registry import AnyRegistry
from thrift.python.any.typestub import PrimitiveType, SerializableType

# @manual=//thrift/test/testset:testset-python-types
from thrift.test.testset import thrift_types


TEST_STRUCT = thrift_types.struct_map_string_i32(
    field_1={
        "Answer to the Ultimate Question of Life, the Universe, and Everything.": 42
    }
)
TEST_UNION = thrift_types.union_map_string_string(field_2={"foo": "bar"})
TEST_EXCEPTION = thrift_types.exception_map_string_i64(field_1={"code": 404})
TEST_PRIMITIVES: typing.List[PrimitiveType] = [
    True,
    42,
    123456.789,
    "thrift-python",
    b"raw bytes",
    testing.thrift_types.Color.blue,
]
TEST_CONTAINERS: typing.List[
    typing.Union[
        typing.Sequence[SerializableType],
        typing.AbstractSet[SerializableType],
        typing.Mapping[str, SerializableType],
    ]
] = [
    [1, 1, 2, 3, 5],
    list(testing.thrift_types.Color),
    [TEST_STRUCT, TEST_STRUCT],
    {b"hello", b"world"},
    {TEST_UNION},
    {"foo": TEST_EXCEPTION, "bar": TEST_EXCEPTION},
]


class AnyRegistryTest(unittest.TestCase):
    def test_register_type(self) -> None:
        registry = AnyRegistry()
        self.assertTrue(registry.register_type(thrift_types.struct_empty))
        self.assertFalse(registry.register_type(thrift_types.struct_empty))
        self.assertFalse(registry.register_type(bool))  # pyre-ignore

        class MyPreciousClass:
            pass

        with self.assertRaises(AttributeError):
            registry.register_type(MyPreciousClass)  # pyre-ignore

    def test_register_module(self) -> None:
        registry = AnyRegistry()
        self.assertTrue(registry.register_module(thrift_types))
        self.assertFalse(registry.register_module(thrift_types))

    def test_struct_round_trip(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)

        for standard_protocol in [
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ]:
            with self.subTest(standard_protocol=standard_protocol):
                any_obj = registry.store(
                    TEST_STRUCT, protocol=Protocol(standard=standard_protocol)
                )
                self.assertEqual(TypeName.Type.structType, any_obj.type.name.type)
                loaded = registry.load(any_obj)
                self.assertEqual(TEST_STRUCT, loaded)

    def test_union_round_trip(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)

        for standard_protocol in [
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ]:
            with self.subTest(standard_protocol=standard_protocol):
                any_obj = registry.store(
                    TEST_UNION, protocol=Protocol(standard=standard_protocol)
                )
                self.assertEqual(TypeName.Type.unionType, any_obj.type.name.type)
                loaded = registry.load(any_obj)
                self.assertEqual(TEST_UNION, loaded)

    def test_exception_round_trip(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)

        for standard_protocol in [
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ]:
            with self.subTest(standard_protocol=standard_protocol):
                any_obj = registry.store(
                    TEST_EXCEPTION, protocol=Protocol(standard=standard_protocol)
                )
                self.assertEqual(TypeName.Type.exceptionType, any_obj.type.name.type)
                loaded = registry.load(any_obj)
                self.assertEqual(TEST_EXCEPTION, loaded)

    def test_primitive_round_trip(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)
        registry.register_module(testing.thrift_types)

        for standard_protocol in [
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ]:
            with self.subTest(standard_protocol=standard_protocol):
                for primitive in TEST_PRIMITIVES:
                    with self.subTest(primitive=primitive):
                        any_obj = registry.store(
                            primitive, protocol=Protocol(standard=standard_protocol)
                        )
                        loaded = registry.load(any_obj)
                        self.assertIs(type(primitive), type(loaded))
                        if isinstance(primitive, float):
                            # pyre-fixme[6]: For 2nd positional argument, expected
                            #  `SupportsRSub[Variable[_T], SupportsAbs[SupportsRound[object]]]`
                            #  but got `Union[Enum, IOBuf, bool, bytes, float, int, str,
                            #  GeneratedError, StructOrUnion]`
                            self.assertAlmostEqual(primitive, loaded, places=3)
                        else:
                            self.assertEqual(primitive, loaded)

    def test_containers_round_trip(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)
        registry.register_module(testing.thrift_types)

        for standard_protocol in [
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ]:
            with self.subTest(standard_protocol=standard_protocol):
                for container in TEST_CONTAINERS:
                    with self.subTest(container=container):
                        any_obj = registry.store(
                            container, protocol=Protocol(standard=standard_protocol)
                        )
                        loaded = registry.load(any_obj)
                        self.assertEqual(container, loaded)

    def test_iobuf_comes_back_as_bytes(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)

        for standard_protocol in (
            StandardProtocol.Binary,
            StandardProtocol.Compact,
            StandardProtocol.SimpleJson,
        ):
            with self.subTest(standard_protocol=standard_protocol):
                buf = IOBuf(b"iobuf")
                any_obj = registry.store(
                    buf, protocol=Protocol(standard=standard_protocol)
                )
                loaded = registry.load(any_obj)
                self.assertEqual(b"".join(buf), loaded)

    def test_unsupported_protocol(self) -> None:
        registry = AnyRegistry()
        registry.register_module(thrift_types)

        for protocol in [
            Protocol(standard=StandardProtocol.Json),
            Protocol(custom="my.custom.protocol"),
            Protocol(id=42),
        ]:
            with self.subTest(protocol=protocol):
                with self.assertRaises(NotImplementedError):
                    registry.store(TEST_STRUCT, protocol=protocol)
