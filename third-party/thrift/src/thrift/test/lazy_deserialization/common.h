/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/deprecated_terse_writes_types.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/simple_types.h>

namespace apache::thrift::test {

template <class Struct>
Struct gen() {
  Struct obj;
  obj.field1_ref().emplace(100, 1);
  obj.field2_ref().emplace(200, 2);
  obj.field3_ref().emplace(300, 3);
  obj.field4_ref().emplace(400, 4);
  return obj;
}

template <class Struct>
struct Structs;

template <>
struct Structs<Foo> {
  using Struct = Foo;
  using LazyStruct = LazyFoo;
  using IndexedStruct = IndexedFoo;
};

template <>
struct Structs<OptionalFoo> {
  using Struct = OptionalFoo;
  using LazyStruct = OptionalLazyFoo;
  using IndexedStruct = OptionalIndexedFoo;
};

template <>
struct Structs<TerseFoo> {
  using Struct = TerseFoo;
  using LazyStruct = TerseLazyFoo;
  using IndexedStruct = IndexedFoo;
};

template <>
struct Structs<TerseOptionalFoo> {
  using Struct = TerseOptionalFoo;
  using LazyStruct = TerseOptionalLazyFoo;
  using IndexedStruct = OptionalIndexedFoo;
};

template <>
struct Structs<FooNoChecksum> {
  using Struct = FooNoChecksum;
  using LazyStruct = LazyFooNoChecksum;
  using IndexedStruct = IndexedFoo;
};

template <class Serializer_, class Struct>
struct TypeParam : Structs<Struct> {
  using Serializer = Serializer_;
};

using TypeParams = ::testing::Types<
    TypeParam<CompactSerializer, Foo>,
    TypeParam<CompactSerializer, OptionalFoo>,
    TypeParam<CompactSerializer, TerseFoo>,
    TypeParam<CompactSerializer, TerseOptionalFoo>,
    TypeParam<CompactSerializer, FooNoChecksum>,
    TypeParam<BinarySerializer, Foo>,
    TypeParam<BinarySerializer, OptionalFoo>,
    TypeParam<BinarySerializer, TerseFoo>,
    TypeParam<BinarySerializer, TerseOptionalFoo>,
    TypeParam<BinarySerializer, FooNoChecksum>>;

template <typename T>
struct LazyDeserialization : testing::Test {
  static auto genStruct() { return gen<typename T::Struct>(); }
  static auto genLazyStruct() { return gen<typename T::LazyStruct>(); }

  template <class V>
  static std::string serialize(V&& v) {
    return T::Serializer::template serialize<std::string>(std::forward<V>(v));
  }

  template <class V>
  static V deserialize(const std::string& s) {
    return T::Serializer::template deserialize<V>(s);
  }
  template <class V>
  auto deserialize(const std::string& s, V&& v) {
    return T::Serializer::deserialize(s, std::forward<V>(v));
  }
};

TYPED_TEST_SUITE(LazyDeserialization, TypeParams);
} // namespace apache::thrift::test
