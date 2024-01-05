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

TEST(AnyTest, any_struct_fields) {
  auto any = type::toAnyData<type::i16_t>();
  auto ret = anyDebugString(any);
  EXPECT_NE(ret.find("3: data (i16) = \"Unrecognized type\""), ret.npos) << ret;
}

template <typename>
class AnyTestFixture : public ::testing::Test {};

TYPED_TEST_SUITE(AnyTestFixture, type::Tags);

void verifyDebugString(const type::AnyData& any) {
  auto ret = anyDebugString(any);
  EXPECT_NE(ret.find("Unrecognized type"), ret.npos) << ret;
}

TYPED_TEST(AnyTestFixture, unregistered_compact) {
  auto any = type::toAnyData<TypeParam>();
  verifyDebugString(any);
}

TYPED_TEST(AnyTestFixture, unregistered_binary) {
  auto any = type::toAnyData<TypeParam, type::StandardProtocol::Binary>();
  verifyDebugString(any);
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
