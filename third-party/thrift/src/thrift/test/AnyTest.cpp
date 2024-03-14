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

#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/lib/cpp2/type/Any.h>
#include <thrift/test/AnyTesting.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/test/gen-cpp2/AnyTest1_types.h>

using namespace apache::thrift::type;

namespace apache::thrift::test {
namespace {

template <conformance::StandardProtocol P>
bool isRegistered(std::string_view uri) {
  return conformance::AnyRegistry::generated().getSerializerByUri(
             uri, conformance::getStandardProtocol<P>()) != nullptr;
}

TEST(AnyTest, Registered) {
  using conformance::StandardProtocol;
  {
    constexpr auto kType = "facebook.com/thrift/test/AnyTestStruct";
    EXPECT_TRUE(isRegistered<StandardProtocol::Binary>(kType));
    EXPECT_TRUE(isRegistered<StandardProtocol::Compact>(kType));
    EXPECT_TRUE(isRegistered<StandardProtocol::SimpleJson>(kType));
    EXPECT_FALSE(isRegistered<StandardProtocol::Json>(kType));
  }

  {
    constexpr auto kType = "facebook.com/thrift/test/AnyTestException";
    EXPECT_TRUE(isRegistered<StandardProtocol::Binary>(kType));
    EXPECT_TRUE(isRegistered<StandardProtocol::Compact>(kType));
    EXPECT_TRUE(isRegistered<StandardProtocol::SimpleJson>(kType));
    EXPECT_FALSE(isRegistered<StandardProtocol::Json>(kType));
  }

  { // Has Json enabled.
    constexpr auto kType = "facebook.com/thrift/test/AnyTestUnion";
    EXPECT_TRUE(isRegistered<StandardProtocol::Binary>(kType));
    EXPECT_TRUE(isRegistered<StandardProtocol::Compact>(kType));
    EXPECT_TRUE(isRegistered<StandardProtocol::SimpleJson>(kType));
    EXPECT_FALSE(isRegistered<StandardProtocol::Json>(kType));
  }

  { // Does not have `any` in buck target options.
    constexpr auto kType = "facebook.com/thrift/test/AnyTestMissingAnyOption";
    EXPECT_FALSE(isRegistered<StandardProtocol::Binary>(kType));
    EXPECT_FALSE(isRegistered<StandardProtocol::Compact>(kType));
    EXPECT_FALSE(isRegistered<StandardProtocol::SimpleJson>(kType));
    EXPECT_FALSE(isRegistered<StandardProtocol::Json>(kType));
  }
}

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

TYPED_TEST_SUITE(AnyTestFixture, Tags);

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
    any = toAnyData<TypeParam>();
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

TYPED_TEST(AnyTestFixture, MoveToSemiAny) {
  const auto& value = tagToValue<TypeParam>;
  AnyData any;

  if constexpr (
      !std::is_same_v<TypeParam, string_t> &&
      !std::is_same_v<TypeParam, binary_t>) {
    // Rely on infer_tag if TypeParam is not string_t or binary_t
    any = AnyData::toAny(value);
  } else {
    any = toAnyData<TypeParam>();
  }
  auto anyCopy = any;
  auto semiAny = std::move(any).moveToSemiAny();
  AnyData anyFromSemiAny(semiAny);
  EXPECT_EQ(anyCopy, anyFromSemiAny);
}

bool contains(std::string_view s, std::string_view pattern) {
  return s.find(pattern) != std::string_view::npos;
}

TEST(AnyTest, GetTypeMismatch) {
  auto any = toAnyData<i32_t>();
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
    any = toAnyData<TypeParam, StandardProtocol::Binary>();
  }
  EXPECT_EQ(any.type(), Type{TypeParam{}});
  EXPECT_EQ(any.protocol(), Protocol::get<StandardProtocol::Binary>());

  native_type<TypeParam> v;
  any.get<TypeParam>(v);
  EXPECT_EQ(v, value);
}

TEST(AnyTest, isValid) {
  AnyStruct builder;
  // Any is empty, so it is valid
  EXPECT_TRUE(AnyData::isValid(builder));

  builder.data() = folly::IOBuf::wrapBufferAsValue("{}", 2);
  // type and protocol are missing
  EXPECT_FALSE(AnyData::isValid(builder));

  builder.type() = struct_t<test::AnyTestStruct>{};
  // protocol is missing
  EXPECT_FALSE(AnyData::isValid(builder));

  builder.protocol() = StandardProtocol::SimpleJson;
  // All fields have valid values
  EXPECT_TRUE(AnyData::isValid(builder));
}

} // namespace
} // namespace apache::thrift::test
