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
#include <thrift/lib/cpp2/type/AnyTesting.h>

namespace apache::thrift {

using apache::thrift::test::NestedAny;

TEST(AnyTest, any_struct_fields) {
  auto any = type::toAnyData<type::i16_t>();
  auto ret = anyDebugString(any);
  EXPECT_NE(
      ret.find(fmt::format("3: data (i16) = {}", type::kMagicString)), ret.npos)
      << ret;
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
}

} // namespace apache::thrift
