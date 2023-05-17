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

#include <thrift/lib/cpp2/type/Any.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/test/gen-cpp2/AnyTest1_types.h>

namespace apache::thrift::type {
namespace {

TEST(AnyTest, BaseApi) {
  SemiAny builder;
  EXPECT_THROW(AnyData{builder}, std::runtime_error);
  builder.type() = i16_t{};
  EXPECT_THROW(AnyData{builder}, std::runtime_error);
  builder.protocol() = StandardProtocol::Compact;
  builder.data() = folly::IOBuf::wrapBufferAsValue("hi", 2);

  AnyData any(builder);
  EXPECT_EQ(any.type(), Type::get<i16_t>());
  EXPECT_EQ(any.protocol(), Protocol::get<StandardProtocol::Compact>());
  EXPECT_EQ(any.data().data(), builder.data()->data());
  EXPECT_EQ(any.data().length(), 2);

  builder.type() = {};
  EXPECT_THROW(AnyData{builder}, std::runtime_error);
}

template <typename>
class AnyTestFixture : public ::testing::Test {};

using Tags = ::testing::Types<
    bool_t,
    byte_t,
    i16_t,
    i32_t,
    i64_t,
    float_t,
    double_t,
    string_t,
    binary_t,
    list<i32_t>,
    set<i32_t>,
    map<i32_t, float_t>,
    struct_t<test::AnyTestStruct>>;
TYPED_TEST_SUITE(AnyTestFixture, Tags);

template <class T>
const native_type<T> tagToValue = 42;

template <>
const std::string tagToValue<string_t> = "42";

template <>
const std::string tagToValue<binary_t> = "42";

template <>
const std::vector<std::int32_t> tagToValue<list<i32_t>> = {4, 2};

template <>
const std::set<std::int32_t> tagToValue<set<i32_t>> = {4, 2};

template <>
const std::map<std::int32_t, float> tagToValue<map<i32_t, float_t>> = {
    {4, 2}, {2, 4}};

template <>
const test::AnyTestStruct tagToValue<struct_t<test::AnyTestStruct>> = [] {
  test::AnyTestStruct ret;
  ret.foo() = 42;
  return ret;
}();

TYPED_TEST(AnyTestFixture, ToAny) {
  const auto& value = tagToValue<TypeParam>;
  AnyData any;
  native_type<TypeParam> v1, v2;

  if constexpr (
      !std::is_same_v<TypeParam, string_t> &&
      !std::is_same_v<TypeParam, binary_t>) {
    // Rely on infer_tag if TypeParam is not string_t or binary_t
    any = AnyData::toAny(value);
    std::as_const(any).get(v1);
  } else {
    any = AnyData::toAny<TypeParam>(value);
    std::as_const(any).get<TypeParam>(v1);
  }
  EXPECT_EQ(v1, value);

  EXPECT_EQ(any.type(), Type{TypeParam{}});
  EXPECT_EQ(any.protocol(), Protocol::get<StandardProtocol::Compact>());

  CompactProtocolReader reader;
  reader.setInput(&any.data());
  op::decode<TypeParam>(reader, v2);
  EXPECT_EQ(v2, value);
}

bool contains(std::string_view s, std::string_view pattern) {
  return s.find(pattern) != std::string_view::npos;
}

TEST(AnyTest, GetTypeMismatch) {
  auto any = AnyData::toAny(tagToValue<i32_t>);
  int16_t i = 0;
  // We don't use EXPECT_THROW since we want to check the content
  try {
    any.get<i16_t>(i);
    EXPECT_TRUE(false);
  } catch (std::runtime_error& e) {
    EXPECT_TRUE(contains(e.what(), "Type mismatch"));
    EXPECT_TRUE(contains(e.what(), "i16Type"));
    EXPECT_TRUE(contains(e.what(), "i32Type"));
  }
}

TEST(AnyTest, UnsupportedProtocol) {
  SemiAny builder;
  builder.type() = struct_t<test::AnyTestStruct>{};
  builder.protocol() = StandardProtocol::SimpleJson;
  builder.data() = folly::IOBuf::wrapBufferAsValue("{}", 2);
  AnyData any(builder);
  test::AnyTestStruct strct;
  // We don't use EXPECT_THROW since we want to check the content
  try {
    any.get(strct);
    EXPECT_TRUE(false);
  } catch (std::runtime_error& e) {
    EXPECT_TRUE(contains(e.what(), "Unsupported protocol"));
  }
}

TYPED_TEST(AnyTestFixture, BinaryProtocol) {
  const auto& value = tagToValue<TypeParam>;
  AnyData any;

  if constexpr (
      !std::is_same_v<TypeParam, string_t> &&
      !std::is_same_v<TypeParam, binary_t>) {
    // Rely on infer_tag if TypeParam is not string_t or binary_t
    any = AnyData::toAny<StandardProtocol::Binary>(value);
  } else {
    any = AnyData::toAny<TypeParam, StandardProtocol::Binary>(value);
  }
  EXPECT_EQ(any.type(), Type{TypeParam{}});
  EXPECT_EQ(any.protocol(), Protocol::get<StandardProtocol::Binary>());

  native_type<TypeParam> v;
  any.get<TypeParam>(v);
  EXPECT_EQ(v, value);
}
} // namespace
} // namespace apache::thrift::type
