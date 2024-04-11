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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/type/AnyDebugWriter.h>
#include <thrift/test/AnyTesting.h>

namespace apache::thrift {

using apache::thrift::test::NestedAny;

std::string anyTypeDebugString(const type::Type& obj) {
  folly::IOBufQueue queue;
  detail::AnyDebugWriter proto(false);
  proto.setOutput(&queue);
  proto.write(obj);
  std::unique_ptr<folly::IOBuf> buf = queue.move();
  folly::ByteRange br = buf->coalesce();
  return std::string(reinterpret_cast<const char*>(br.data()), br.size());
}

template <typename Tag>
std::string tagToTypeString() {
  return anyTypeDebugString(type::Type::get<Tag>());
}

TEST(AnyTest, any_struct_fields) {
  auto any = type::toAnyData<type::i16_t>();
  auto ret = anyDebugString(any);
  EXPECT_NE(ret.find("1: type (struct) = \"i16\""), ret.npos) << ret;
  EXPECT_NE(ret.find("2: protocol (struct) = \"Compact\""), ret.npos) << ret;
  EXPECT_NE(
      ret.find(fmt::format("3: data (i16) = {}", type::kMagicString)), ret.npos)
      << ret;
}

TEST(AnyTest, type_str) {
  // valid TypeStruct created from Tags
  EXPECT_EQ(tagToTypeString<type::i16_t>(), "\"i16\"");
  EXPECT_EQ(tagToTypeString<type::i32_t>(), "\"i32\"");
  EXPECT_EQ(tagToTypeString<type::bool_t>(), "\"bool\"");
  EXPECT_EQ(tagToTypeString<type::byte_t>(), "\"byte\"");
  EXPECT_EQ(tagToTypeString<type::string_t>(), "\"string\"");
  EXPECT_EQ(tagToTypeString<type::binary_t>(), "\"binary\"");
  EXPECT_EQ(tagToTypeString<type::list<type::i32_t>>(), "\"list<i32>\"");
  EXPECT_EQ(tagToTypeString<type::set<type::i32_t>>(), "\"set<i32>\"");
  EXPECT_EQ(
      (tagToTypeString<type::map<type::i32_t, type::float_t>>()),
      "\"map<i32,float>\"");
  EXPECT_EQ(
      tagToTypeString<type::struct_t<test::AnyTestStruct>>(),
      fmt::format("\"struct<{}>\"", thrift::uri<test::AnyTestStruct>()));
  EXPECT_EQ(
      tagToTypeString<type::exception_t<test::AnyTestException>>(),
      fmt::format("\"exception<{}>\"", thrift::uri<test::AnyTestException>()));

  // invalid TypeStruct
  auto map_type = type::Type::get<type::map<type::i32_t, type::float_t>>();
  auto map_type_struct = map_type.toThrift();
  map_type_struct.name().value().boolType_ref() = type::Void::Unused;
  EXPECT_EQ(
      anyTypeDebugString(type::Type(map_type_struct)), "\"bool<i32,float>\"");
}

template <typename>
class AnyTestFixture : public ::testing::Test {};

TYPED_TEST_SUITE(AnyTestFixture, type::Tags);

template <typename TypeParam>
void verifyDebugString(const type::AnyData& any) {
  auto ret = anyDebugString(any, true);
  auto check = [&](auto str) {
    EXPECT_NE(ret.find(str), ret.npos) << ret << str;
  };
  if (std::is_same_v<TypeParam, type::byte_t>) {
    check("0x75");
  } else if (std::is_same_v<TypeParam, type::bool_t>) {
    check("true");
  } else {
    check(type::kMagicString);
  }
  check(anyTypeDebugString(any.type()));
  check(any.protocol().name());
}

TYPED_TEST(AnyTestFixture, unregistered_compact) {
  auto any = type::toAnyData<TypeParam>();
  verifyDebugString<TypeParam>(any);
}

TYPED_TEST(AnyTestFixture, nested_compact) {
  NestedAny obj;
  obj.any_field() = type::toAnyData<TypeParam>().toThrift();

  auto any = type::AnyData::toAny<type::struct_t<NestedAny>>(obj);
  verifyDebugString<TypeParam>(any);
}

TYPED_TEST(AnyTestFixture, unregistered_binary) {
  auto any = type::toAnyData<TypeParam, type::StandardProtocol::Binary>();
  verifyDebugString<TypeParam>(any);
}

TYPED_TEST(AnyTestFixture, nested_binary) {
  NestedAny obj;
  obj.any_field() =
      type::toAnyData<TypeParam, type::StandardProtocol::Binary>().toThrift();

  auto any = type::AnyData::
      toAny<type::struct_t<NestedAny>, type::StandardProtocol::Binary>(obj);
  verifyDebugString<TypeParam>(any);
}

TYPED_TEST(AnyTestFixture, unregistered_json) {
  SimpleJSONProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  op::encode<TypeParam>(writer, type::tagToValue<TypeParam>);

  type::AnyStruct any;
  any.data() = queue.moveAsValue();
  any.protocol() = type::StandardProtocol::SimpleJson;
  any.type() = TypeParam{};
  auto ret = anyDebugString(any);
  auto br = any.data()->coalesce();
  auto encoded_str = folly::cEscape<std::string>(
      std::string(reinterpret_cast<const char*>(br.data()), br.size()));
  EXPECT_NE(ret.find(encoded_str.data()), ret.npos) << ret << encoded_str;
  auto type_str = anyTypeDebugString(*any.type());
  EXPECT_NE(ret.find(type_str), ret.npos) << ret << type_str;
  auto protocol_str = any.protocol()->name();
  EXPECT_NE(ret.find(protocol_str), ret.npos) << ret << protocol_str;
}

} // namespace apache::thrift
