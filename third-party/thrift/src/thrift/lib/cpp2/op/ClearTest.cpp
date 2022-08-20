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

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Testing.h>
#include <thrift/lib/cpp2/type/Name.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/cpp2/type/Traits.h>
#include <thrift/lib/thrift/gen-cpp2/type_types.h>
#include <thrift/test/testset/gen-cpp2/testset_fatal_all.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

namespace apache::thrift::op {
namespace {
using namespace apache::thrift::test;

template <typename Tag, bool IsField = false>
void testClearImpl(
    const type::native_type<Tag>& expected,
    type::native_type<Tag> unexpected,
    bool emptiable) {
  // TODO(afuller): Remove dep on fatal.
  // SCOPED_TRACE(type::getName<Tag>());

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

} // namespace
} // namespace apache::thrift::op
