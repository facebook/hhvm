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

#include <thrift/lib/cpp2/op/Clear.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Testing.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

namespace apache::thrift::op {
namespace {
using namespace apache::thrift::test;

template <typename Tag, bool IsField = false>
void testClearImpl(
    const type::native_type<Tag>& expected,
    type::native_type<Tag> unexpected,
    bool emptiable) {
  EXPECT_EQ(isEmpty<Tag>(expected), emptiable);
  EXPECT_THAT(getIntrinsicDefault<Tag>(), IsIdenticalTo<Tag>(expected));

  EXPECT_FALSE(isEmpty<Tag>(unexpected));
  EXPECT_THAT(unexpected, ::testing::Not(IsIdenticalTo<Tag>(expected)));

  if constexpr (IsField) {
    uint8_t isset = 1;
    TestStruct obj;
    clear_field<Tag>(
        apache::thrift::detail::make_field_ref(unexpected, isset), obj);
  } else {
    clear<Tag>(unexpected);
  }

  EXPECT_EQ(isEmpty<Tag>(expected), emptiable);
  EXPECT_THAT(unexpected, IsIdenticalTo<Tag>(expected));
}

template <typename Tag>
void testClear(
    const type::native_type<Tag>& expected,
    type::native_type<Tag> unexpected,
    bool emptiable = true) {
  testClearImpl<Tag>(expected, unexpected, emptiable);
  testClearImpl<
      type::field<Tag, apache::thrift::FieldContext<TestStruct, 0>>,
      true>(expected, unexpected, emptiable);
}

TEST(ClearTest, InferTag) {
  bool val = true;
  EXPECT_FALSE(op::isEmpty<>(val));
  op::clear<>(val);
  EXPECT_TRUE(op::isEmpty<>(val));
  EXPECT_EQ(op::getIntrinsicDefault<int64_t>(), 0);
}

TEST(ClearTest, Integral) {
  testClear<type::bool_t>(false, true);
  testClear<type::byte_t>(0, 1);
  testClear<type::i16_t>(0, 1);
  testClear<type::i32_t>(0, 1);
  testClear<type::i64_t>(0, 1);
  testClear<type::enum_t<type::BaseTypeEnum>>(
      type::BaseTypeEnum::Void, type::BaseTypeEnum::Bool);
}

TEST(ClearTest, FloatingPoint) {
  testClear<type::float_t>(0, -0.0);
  testClear<type::double_t>(0, -0.0);
}

TEST(ClearTest, String) {
  testClear<type::string_t>("", "hi");
  testClear<type::binary_t>("", "bye");
}

TEST(ClearTest, Container) {
  testClear<type::list<type::string_t>>({}, {"hi"});
  testClear<type::set<type::string_t>>({}, {"hi"});
  testClear<type::map<type::string_t, type::string_t>>({}, {{"hi", "bye"}});
}

TEST(ClearTest, Structured) {
  testClear<type::struct_t<testset::struct_i64>>(
      {}, {apache::thrift::FragileConstructor(), 1}, false);
  testClear<type::struct_t<testset::struct_optional_i64>>(
      {}, {apache::thrift::FragileConstructor(), 1});
  testClear<type::exception_t<testset::exception_i64>>(
      {}, {apache::thrift::FragileConstructor(), 1}, false);
  testClear<type::exception_t<testset::exception_optional_i64>>(
      {}, {apache::thrift::FragileConstructor(), 1});
  testset::union_i64 one;
  one.field_1_ref().ensure() = 1;
  testClear<type::union_t<testset::union_i64>>({}, one);
}

TEST(ClearTest, IOBufPtr) {
  using T = std::unique_ptr<folly::IOBuf>;
  using Tag = type::cpp_type<T, type::binary_t>;
  T empty;
  EXPECT_TRUE(op::isEmpty<Tag>(empty));
  empty = folly::IOBuf::wrapBuffer("", 0);
  EXPECT_TRUE(op::isEmpty<Tag>(empty));
  T one = folly::IOBuf::wrapBuffer("one", 3);
  EXPECT_FALSE(op::isEmpty<Tag>(one));

  op::clear<Tag>(one);
  EXPECT_TRUE(op::isEmpty<Tag>(one));
  EXPECT_EQ(one, nullptr);

  // Normalizes to nullptr.
  EXPECT_NE(empty, nullptr);
  op::clear<Tag>(empty);
  EXPECT_EQ(empty, nullptr);
}

TEST(ClearTest, Adapter) {
  testClearImpl<type::adapted<TestAdapter, type::i64_t>>({}, {1}, true);
  testClearImpl<type::adapted<TestAdapter, type::list<type::i64_t>>>(
      {}, {{1}}, true);
  testClearImpl<
      type::field<
          type::adapted<FieldAdapter, type::i64_t>,
          FieldContext<TestStruct, 1>>,
      true>({}, {1}, true);
  testClearImpl<
      type::field<
          type::adapted<FieldAdapter, type::list<type::i64_t>>,
          FieldContext<TestStruct, 1>>,
      true>({}, {{1}}, true);
}

template <typename Tag, bool IsFieldAdapter = false>
void test_custom_default() {
  using Adapter = detail::get_adapter_t<Tag>;
  auto defaultObj = getDefault<Tag>();
  const auto& intrinsicDefaultObj = getIntrinsicDefault<Tag>();

  EXPECT_FALSE(apache::thrift::adapt_detail::equal<Adapter>(
      defaultObj, intrinsicDefaultObj));

  // The default of a field with field adapter is constructed with the default
  // parent struct. Meanwhile, the intrinsic default of a field with field
  // adapter is constructed with the intrinsic default parent struct.
  if constexpr (IsFieldAdapter) {
    EXPECT_EQ(*defaultObj.meta, "meta");
    EXPECT_EQ(*intrinsicDefaultObj.meta, "");
  }

  // TODO(dokwon): Fix op::clear for adapted types with custom default.
  // clear<Tag>(defaultObj);
  // EXPECT_TRUE(apache::thrift::adapt_detail::equal<Adapter>(
  //     defaultObj, intrinsicDefaultObj));
}

TEST(ClearTest, CustomDefaultTypeAdapter) {
  using Tag = type::adapted<
      TestAdapter,
      type::struct_t<testset::struct_terse_i64_custom_default>>;
  using FieldTag = type::field<Tag, FieldContext<TestStructWithContext, 1>>;

  test_custom_default<Tag>();
  test_custom_default<FieldTag>();
}

TEST(ClearTest, CustomDefaultFieldAdapter) {
  using Tag = type::field<
      type::adapted<
          FieldAdapterWithContext,
          type::struct_t<testset::struct_terse_i64_custom_default>>,
      FieldContext<TestStructWithContext, 1>>;

  test_custom_default<Tag, true>();
}

TEST(ClearTest, Default) {
  // Default of structured with no custom default returns the reference of
  // intrinsic default for the terse intern box optimization.
  using TagWithNoCustomDefault = type::struct_t<testset::struct_i64>;
  using TagWithCustomDefault =
      type::struct_t<testset::struct_i64_custom_default>;

  EXPECT_EQ(
      &getDefault<TagWithNoCustomDefault>(),
      &getIntrinsicDefault<TagWithNoCustomDefault>());
  EXPECT_NE(
      &getDefault<TagWithCustomDefault>(),
      &getIntrinsicDefault<TagWithCustomDefault>());
}

} // namespace
} // namespace apache::thrift::op
