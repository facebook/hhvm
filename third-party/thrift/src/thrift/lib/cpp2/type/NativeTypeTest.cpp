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

#include <thrift/lib/cpp2/type/NativeType.h>

#include <list>
#include <map>
#include <type_traits>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

namespace apache::thrift::type {
namespace {

template <ThriftTypeTag Tag>
native_type<Tag> foo();
template <typename T>
  requires(!ThriftTypeTag<T>)
T foo();

TEST(NativeTypeTest, UndefinedOverloading) {
  static_assert(std::is_same_v<int, decltype(foo<int>())>);
  static_assert(std::is_same_v<int, decltype(foo<i32_t>())>);
}

TEST(NativeTypeTest, Void) {
  static_assert(std::is_same_v<void, native_type<void_t>>);
  static_assert(std::is_same_v<void_t, infer_tag<void>>);
  static_assert(std::is_same_v<void_t, infer_tag<std::nullptr_t>>);
}

TEST(NativeTypeTest, Bool) {
  static_assert(std::is_same_v<bool, native_type<bool_t>>);
  static_assert(std::is_same_v<bool_t, infer_tag<bool>>);
}

TEST(InferTagTest, Tag) {
  static_assert(std::is_same_v<byte_t, infer_tag<byte_t>>);
}

TEST(InferTagTest, Integer) {
  static_assert(std::is_same_v<int8_t, native_type<byte_t>>);
  static_assert(std::is_same_v<int16_t, native_type<i16_t>>);
  static_assert(std::is_same_v<int32_t, native_type<i32_t>>);
  static_assert(std::is_same_v<int64_t, native_type<i64_t>>);
  static_assert(std::is_same_v<byte_t, infer_tag<int8_t>>);
  static_assert(std::is_same_v<i16_t, infer_tag<int16_t>>);
  static_assert(std::is_same_v<i32_t, infer_tag<int32_t>>);
  static_assert(std::is_same_v<i64_t, infer_tag<int64_t>>);
  static_assert(std::is_same_v<cpp_type<uint8_t, byte_t>, infer_tag<uint8_t>>);
  static_assert(std::is_same_v<cpp_type<uint16_t, i16_t>, infer_tag<uint16_t>>);
  static_assert(std::is_same_v<cpp_type<uint32_t, i32_t>, infer_tag<uint32_t>>);
  static_assert(std::is_same_v<cpp_type<uint64_t, i64_t>, infer_tag<uint64_t>>);

  static_assert(type::is_a_v<infer_tag<char>, byte_t>);
  static_assert(type::is_a_v<infer_tag<signed char>, byte_t>);
  static_assert(type::is_a_v<infer_tag<unsigned char>, byte_t>);

  static_assert(type::is_a_v<infer_tag<short int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<unsigned short int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<unsigned int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<long int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<unsigned long int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<long long int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<unsigned long long int>, integral_c>);
  static_assert(type::is_a_v<infer_tag<size_t>, integral_c>);
}

TEST(InferTagTest, Floating) {
  static_assert(std::is_same_v<float, native_type<float_t>>);
  static_assert(std::is_same_v<double, native_type<double_t>>);
  static_assert(std::is_same_v<float_t, infer_tag<float>>);
  static_assert(std::is_same_v<double_t, infer_tag<double>>);
}

TEST(InferTagTest, Strings) {
  static_assert(std::is_same_v<std::string, native_type<string_t>>);
  static_assert(std::is_same_v<std::string, native_type<binary_t>>);
  // TODO(afuller): Allow string and binary to interoperat and infer a number of
  // string value and literals/views to be inferred as binary_t.
}

TEST(InferTagTest, Containers) {
  static_assert(
      std::is_same_v<native_type<list<i32_t>>, std::vector<std::int32_t>>);
  static_assert(
      std::is_same_v<list<i32_t>, infer_tag<std::vector<std::int32_t>>>);
  static_assert(
      std::is_same_v<native_type<set<i32_t>>, std::set<std::int32_t>>);
  static_assert(std::is_same_v<set<i32_t>, infer_tag<std::set<std::int32_t>>>);
  static_assert(std::is_same_v<
                native_type<map<i32_t, bool_t>>,
                std::map<std::int32_t, bool>>);
  static_assert(std::is_same_v<
                map<i32_t, bool_t>,
                infer_tag<std::map<std::int32_t, bool>>>);
}

TEST(InferTagTest, Wrap) {
  struct FooStruct {};
  struct Foo : detail::Wrap<FooStruct, struct_t<FooStruct>> {};
  static_assert(std::is_same_v<
                infer_tag<Foo>,
                adapted<InlineAdapter<Foo>, struct_t<FooStruct>>>);
}

} // namespace
} // namespace apache::thrift::type
