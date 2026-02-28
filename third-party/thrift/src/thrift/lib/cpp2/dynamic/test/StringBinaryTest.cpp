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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/dynamic/detail/Datum.h>

#include <gtest/gtest.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::dynamic {

// String tests
TEST(StringTest, BasicUsage) {
  String s("hello");
  EXPECT_EQ(s.view(), "hello");
  EXPECT_EQ(s.size(), 5);

  s.append(" world");
  EXPECT_EQ(s.view(), "hello world");

  String s2(s);
  EXPECT_EQ(s, s2);
}

TEST(StringTest, MemoryResource) {
  std::pmr::monotonic_buffer_resource mbr;
  String s("test", &mbr);
  EXPECT_EQ(s.view(), "test");
}

TEST(StringTest, Mutation) {
  String s("hello");
  s.resize(3);
  EXPECT_EQ(s.view(), "hel");

  s.clear();
  EXPECT_TRUE(s.empty());
}

// Binary tests
TEST(BinaryTest, BasicUsage) {
  auto buf = folly::IOBuf::copyBuffer("test data");
  Binary b(std::move(buf));
  EXPECT_EQ(b.computeChainDataLength(), 9);

  auto cursor = b.cursor();
  std::array<char, 9> data{};
  cursor.pull(data.data(), 9);
  EXPECT_EQ(std::string_view(data.data(), 9), "test data");
}

TEST(BinaryTest, CopyAndClone) {
  auto buf = folly::IOBuf::copyBuffer("test");
  Binary b1(std::move(buf));
  Binary b2(b1);
  Binary b3 = b1.clone();

  EXPECT_EQ(b1, b2);
  EXPECT_EQ(b1, b3);
}

TEST(BinaryTest, Comparison) {
  auto buf1 = folly::IOBuf::copyBuffer("test");
  auto buf2 = folly::IOBuf::copyBuffer("test");
  auto buf3 = folly::IOBuf::copyBuffer("different");

  Binary b1(std::move(buf1));
  Binary b2(std::move(buf2));
  Binary b3(std::move(buf3));
  Binary b4; // empty

  EXPECT_EQ(b1, b2);
  EXPECT_NE(b1, b3);
  EXPECT_NE(b1, b4);
}

TEST(StringTest, SerializationRoundTrip) {
  auto value = DynamicValue::makeString(String("hello world"));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  auto deserValue = deserializeValue(
      reader, type_system::TypeRef(type_system::TypeRef::String()));

  // Verify
  EXPECT_EQ(value, deserValue);
  EXPECT_EQ(deserValue.asString().view(), "hello world");
}

TEST(BinaryTest, SerializationRoundTrip) {
  auto buf = folly::IOBuf::copyBuffer("test binary data");
  auto value = DynamicValue::makeBinary(std::move(buf));

  // Serialize
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);
  serializeValue(writer, value);

  // Deserialize
  auto deserBuf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(deserBuf.get());
  auto deserValue = deserializeValue(
      reader, type_system::TypeRef(type_system::TypeRef::Binary()));

  // Verify
  EXPECT_EQ(value, deserValue);
  auto cursor = deserValue.asBinary().cursor();
  std::array<char, 16> data{};
  cursor.pull(data.data(), 16);
  EXPECT_EQ(std::string_view(data.data(), 16), "test binary data");
}

} // namespace apache::thrift::dynamic
