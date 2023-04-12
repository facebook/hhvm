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
#include <unordered_set>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/cpp2/type/detail/Wrap.h>

namespace apache::thrift::type {
namespace {

template <typename Tag>
native_type<Tag> foo();
template <typename T>
if_not_thrift_type_tag<T, T> foo();

TEST(NativeTypeTest, UndefinedOverloading) {
  test::same_type<int, decltype(foo<int>())>;
  test::same_type<int, decltype(foo<i32_t>())>;
}

TEST(NativeTypeTest, Void) {
  test::same_type<void, native_type<void_t>>;
  test::same_tag<void_t, infer_tag<void>>;
  test::same_tag<void_t, infer_tag<std::nullptr_t>>;
}

TEST(NativeTypeTest, Bool) {
  test::same_type<bool, native_type<bool_t>>;
  test::same_tag<bool_t, infer_tag<bool>>;
}

TEST(InferTagTest, Tag) {
  test::same_tag<byte_t, infer_tag<byte_t>>;
}

TEST(InferTagTest, Integer) {
  test::same_type<int8_t, native_type<byte_t>>;
  test::same_type<int16_t, native_type<i16_t>>;
  test::same_type<int32_t, native_type<i32_t>>;
  test::same_type<int64_t, native_type<i64_t>>;
  test::same_tag<byte_t, infer_tag<int8_t>>;
  test::same_tag<i16_t, infer_tag<int16_t>>;
  test::same_tag<i32_t, infer_tag<int32_t>>;
  test::same_tag<i64_t, infer_tag<int64_t>>;
  test::same_tag<cpp_type<uint8_t, byte_t>, infer_tag<uint8_t>>;
  test::same_tag<cpp_type<uint16_t, i16_t>, infer_tag<uint16_t>>;
  test::same_tag<cpp_type<uint32_t, i32_t>, infer_tag<uint32_t>>;
  test::same_tag<cpp_type<uint64_t, i64_t>, infer_tag<uint64_t>>;

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
  test::same_type<float, native_type<float_t>>;
  test::same_type<double, native_type<double_t>>;
  test::same_tag<float_t, infer_tag<float>>;
  test::same_tag<double_t, infer_tag<double>>;
}

TEST(InferTagTest, Strings) {
  test::same_type<std::string, native_type<string_t>>;
  test::same_type<std::string, native_type<binary_t>>;
  // TODO(afuller): Allow string and binary to interoperat and infer a number of
  // string value and literals/views to be inferred as binary_t.
}

TEST(InferTagTest, Containers) {
  test::same_tag<native_type<list<i32_t>>, std::vector<std::int32_t>>;
  test::same_tag<list<i32_t>, infer_tag<std::vector<std::int32_t>>>;
  test::same_tag<native_type<set<i32_t>>, std::set<std::int32_t>>;
  test::same_tag<set<i32_t>, infer_tag<std::set<std::int32_t>>>;
  test::same_tag<native_type<map<i32_t, bool_t>>, std::map<std::int32_t, bool>>;
  test::same_tag<map<i32_t, bool_t>, infer_tag<std::map<std::int32_t, bool>>>;
}

TEST(InferTagTest, Wrap) {
  struct FooStruct {};
  struct Foo : detail::Wrap<FooStruct, struct_t<FooStruct>> {};
  test::same_tag<
      infer_tag<Foo>,
      adapted<InlineAdapter<Foo>, struct_t<FooStruct>>>;
}

} // namespace
} // namespace apache::thrift::type
