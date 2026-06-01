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

#include <thrift/lib/cpp2/op/StdSerializer.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/op/Testing.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/type/Any.h>

namespace apache::thrift::op {
namespace {

template <typename Tag, typename S, typename T>
void testAnyDataGet(const S& serializer, const T& value) {
  folly::IOBufQueue queue;
  serializer.encode(value, folly::io::QueueAppender{&queue, 2 << 4});
  type::SemiAnyStruct sa;
  sa.protocol() = serializer.getProtocol();
  sa.data() = *queue.front();
  sa.type() = type::Type::get<Tag>();
  type::AnyData anyData(sa);
  EXPECT_EQ(anyData.get<Tag>(), value);
}

template <typename Tag, type::StandardProtocol P, typename T>
void testAnyDataStore(const T& value) {
  auto anyData = type::AnyData::toAny<Tag, P>(value);
  folly::io::Cursor cursor(&anyData.data());
  StdSerializer<Tag, P> serializer;
  auto actual = serializer.template decode<Tag>(cursor);
  EXPECT_EQ(actual, value);
}

template <typename Tag, type::StandardProtocol P, typename T>
void testSerialization(const T& value) {
  StdSerializer<Tag, P> serializer;
  test::expectRoundTrip<Tag>(serializer, value);
  testAnyDataStore<Tag, P>(value);
  // AnyData::get only supports Binary and Compact protocols.
  if constexpr (P != type::StandardProtocol::SimpleJson) {
    testAnyDataGet<Tag>(serializer, value);
  }
}

// Checks that the value roundtrips correctly for all standard protocols.
template <typename T, typename Tag = type::infer_tag<T>>
void testRoundTrip(const T& value) {
  using type::StandardProtocol;
  FBTHRIFT_SCOPED_CHECK(
      (testSerialization<Tag, StandardProtocol::Binary>(value)));
  FBTHRIFT_SCOPED_CHECK(
      (testSerialization<Tag, StandardProtocol::Compact>(value)));
  FBTHRIFT_SCOPED_CHECK(
      (testSerialization<Tag, StandardProtocol::SimpleJson>(value)));
}

TEST(StdSerializerTest, Struct) {
  using protocol::asValueStruct;
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::bool_t>(true)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::byte_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::i16_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::i32_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::i64_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::float_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::double_t>(1)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::string_t>("hi")));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(asValueStruct<type::binary_t>("hi")));
}

TEST(StdSerializerTest, PrimaryTypes) {
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(true));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int8_t(16)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int16_t(16)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int32_t(32)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(std::int64_t(64)));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(1.1f));
  FBTHRIFT_SCOPED_CHECK(testRoundTrip(2.2));
  FBTHRIFT_SCOPED_CHECK((testRoundTrip<std::string, type::string_t>("hi")));
  FBTHRIFT_SCOPED_CHECK((testRoundTrip<std::string, type::binary_t>("hi")));
}

TEST(StdSerializerTest, ContainerWithString) {
  using namespace type;
  using Tag = map<string_t, list<set<string_t>>>;
  std::map<std::string, std::vector<std::set<std::string>>> myData;
  myData["foo"] = {{"1"}, {"2"}};
  auto anyData = AnyData::toAny<Tag, StandardProtocol::Compact>(myData);
  auto myData2 = anyData.get<Tag>();
  EXPECT_EQ(myData, myData2);
}

} // namespace
} // namespace apache::thrift::op
