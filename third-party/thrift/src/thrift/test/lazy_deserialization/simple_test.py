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
from typing import Any, Callable, Type

from thrift.py3 import deserialize, Protocol, serialize
from thrift.test.lazy_deserialization.simple.types import (
    Foo,
    LazyCppRef,
    LazyFoo,
    OptionalFoo,
    OptionalLazyFoo,
)


def gen(Struct: Type[Any]) -> Any:
    return Struct(
        field1=[1] * 10,
        field2=[2] * 20,
        field3=[3] * 30,
        field4=[4] * 40,
    )


def test_supported_protocols(
    func: Callable[[Any, Protocol], None],
) -> Callable[[Any], None]:
    def wrapper(self: Any) -> None:
        for protocol in [Protocol.COMPACT, Protocol.BINARY]:
            func(self, protocol)

    return wrapper


class UnitTest(unittest.TestCase):
    @test_supported_protocols
    def testFooToLazyFoo(self, protocol: Protocol) -> None:
        foo = gen(Foo)
        s = serialize(foo, protocol)
        lazyFoo = deserialize(LazyFoo, s, protocol)
        self.assertEqual(foo.field1, lazyFoo.field1)
        self.assertEqual(foo.field2, lazyFoo.field2)
        self.assertEqual(foo.field3, lazyFoo.field3)
        self.assertEqual(foo.field4, lazyFoo.field4)

    @test_supported_protocols
    def testLazyFooToFoo(self, protocol: Protocol) -> None:
        lazyFoo = gen(LazyFoo)
        s = serialize(lazyFoo, protocol)
        foo = deserialize(Foo, s, protocol)
        self.assertEqual(foo.field1, lazyFoo.field1)
        self.assertEqual(foo.field2, lazyFoo.field2)
        self.assertEqual(foo.field3, lazyFoo.field3)
        self.assertEqual(foo.field4, lazyFoo.field4)

    @test_supported_protocols
    def testLazyCppRefRoundTrip(self, protocol: Protocol) -> None:
        foo = LazyCppRef(
            field1=[1] * 10,
            field2=[2] * 20,
            field3=[3] * 30,
        )
        s = serialize(foo, protocol)
        bar = deserialize(LazyCppRef, s, protocol)
        self.assertEqual(bar.field1, [1] * 10)
        self.assertEqual(bar.field2, [2] * 20)
        self.assertEqual(bar.field3, [3] * 30)

    @test_supported_protocols
    def testEmptyLazyCppRefRoundTrip(self, protocol: Protocol) -> None:
        foo = LazyCppRef()
        s = serialize(foo, protocol)
        bar = deserialize(LazyCppRef, s, protocol)
        self.assertIsNone(bar.field1)
        self.assertIsNone(bar.field2)
        self.assertIsNone(bar.field3)

    @test_supported_protocols
    def testComparison(self, protocol: Protocol) -> None:
        foo1 = gen(LazyFoo)
        s = serialize(foo1, protocol)
        foo2 = deserialize(LazyFoo, s, protocol)
        self.assertEqual(foo1, foo2)
        foo1 = foo1(field4=[])
        self.assertLess(foo1, foo2)
        foo2 = foo2(field4=[])
        self.assertEqual(foo1, foo2)

    @test_supported_protocols
    def testOptional(self, protocol: Protocol) -> None:
        s = serialize(gen(Foo), protocol)
        foo = deserialize(OptionalFoo, s, protocol)
        lazyFoo = deserialize(OptionalLazyFoo, s, protocol)
        self.assertEqual(foo.field1, lazyFoo.field1)
        self.assertEqual(foo.field2, lazyFoo.field2)
        self.assertEqual(foo.field3, lazyFoo.field3)
        self.assertEqual(foo.field4, lazyFoo.field4)
