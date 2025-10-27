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

namespace apache::thrift::op {
namespace {

template <typename Tag, typename S, typename T>
void testTypeRegistryLoad(const S& seralizer, const T& value) {
  folly::IOBufQueue queue;
  seralizer.encode(value, folly::io::QueueAppender{&queue, 2 << 4});
  type::SemiAnyStruct sa;
  sa.protocol() = seralizer.getProtocol();
  sa.data() = *queue.front();
  sa.type() = type::Type::get<Tag>();
  type::AnyData anyData(sa);
  auto result = type::TypeRegistry::generated().load(anyData);
  EXPECT_EQ(result.as<Tag>(), value);
}

template <typename Tag, typename S, typename T>
void testTypeRegistryStore(const S& seralizer, const T& value) {
  auto anyData = type::TypeRegistry::generated().store(
      type::ConstRef(Tag{}, value), seralizer.getProtocol());
  folly::io::Cursor cursor(&anyData.data());
  auto actual = seralizer.template decode<Tag>(cursor);
  EXPECT_EQ(actual, value);
}

template <typename Tag, typename S, typename T>
void testSerialization(const S& seralizer, const T& value) {
  test::expectRoundTrip<Tag>(seralizer, value);
  testTypeRegistryLoad<Tag>(seralizer, value);
  testTypeRegistryStore<Tag>(seralizer, value);
}

// Checks that the value roundtrips correctly for all standard protocols.
template <typename T, typename Tag = type::infer_tag<T>>
void testRoundTrip(const T& value) {
  using type::StandardProtocol;
  FBTHRIFT_SCOPED_CHECK(
      testSerialization<Tag>(
          StdSerializer<Tag, StandardProtocol::Binary>(), value));
  FBTHRIFT_SCOPED_CHECK(
      testSerialization<Tag>(
          StdSerializer<Tag, StandardProtocol::Compact>(), value));
  FBTHRIFT_SCOPED_CHECK(
      testSerialization<Tag>(
          StdSerializer<Tag, StandardProtocol::SimpleJson>(), value));
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
  apache::thrift::op::
      StdSerializer<Tag, apache::thrift::type::StandardProtocol::Compact>
          serializer;
  auto& registry = apache::thrift::type::detail::getGeneratedTypeRegistry();
  registry.registerSerializer(serializer, Tag{});
  std::map<std::string, std::vector<std::set<std::string>>> myData;
  myData["foo"] = {{"1"}, {"2"}};
  auto anyData =
      registry.store<apache::thrift::type::StandardProtocol::Compact>(
          apache::thrift::type::ConstRef(Tag{}, myData));
  auto myData2 = registry.load(anyData).as<Tag>();
  EXPECT_EQ(myData, myData2);
}

} // namespace
} // namespace apache::thrift::op
