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

#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/ProtocolTruncatedData_types.h>

using namespace apache::thrift;
using namespace thrift::test;

template <class Serializer, class T>
void testPartialDataHandling(
    const T& val, size_t bytesToPassTheCheck, bool isStringOrBinary = false) {
  auto buf = Serializer::template serialize<folly::IOBufQueue>(val).move();
  buf->coalesce();

  // Check that deserializing doesn't throw.
  EXPECT_NO_THROW(
      protocol::parseObject<typename Serializer::ProtocolReader>(*buf));
  EXPECT_NO_THROW(Serializer::template deserialize<T>(buf.get()));

  // Trim the buffer to the point that is *just enough* to pass the check for
  // minimum required bytes.
  buf->trimEnd(buf->length() - bytesToPassTheCheck);
  // We'll hit underflow exception when pulling yet another element.
  EXPECT_THROW(
      protocol::parseObject<typename Serializer::ProtocolReader>(*buf),
      std::out_of_range);
  EXPECT_THROW(
      Serializer::template deserialize<T>(buf.get()), std::out_of_range);

  // Trim one more byte.
  buf->trimEnd(1);
  // We'll fail the deserialization straight when we read the length.

  // We don't check remaining bytes in buffer for binary type during
  // deserialization for both Compact and Binary Protocol. Since `parseObject`
  // always treat string fields as binary type by default, there is no check.
  // TODO(dokwon): Consider checking remaining bytes in buffer for binary type
  // during dersialization for both Compact and Binary Protocol instead of
  // throwing std::out_of_range.
  if (isStringOrBinary) {
    EXPECT_THROW(
        protocol::parseObject<typename Serializer::ProtocolReader>(*buf),
        std::out_of_range);
  } else {
    EXPECT_THROW(
        protocol::parseObject<typename Serializer::ProtocolReader>(*buf),
        protocol::TProtocolException);
  }
  EXPECT_THROW(
      Serializer::template deserialize<T>(buf.get()),
      protocol::TProtocolException);
}

template <class Adapter>
struct ProtocolTruncatedDataTest : ::testing::Test {};
using ProtocolTruncatedDataTestTypes =
    ::testing::Types<TestStruct, OpEncodeTestStruct>;
TYPED_TEST_CASE(ProtocolTruncatedDataTest, ProtocolTruncatedDataTestTypes);
TYPED_TEST(ProtocolTruncatedDataTest, TruncatedList) {
  TypeParam s;
  s.i64_list() = {};
  for (size_t i = 0; i < 30; ++i) {
    s.i64_list()->emplace_back((1ull << i));
  }

  testPartialDataHandling<CompactSerializer>(
      s, 3 /* headers */ + 30 /* 1b / element */);
}

TYPED_TEST(ProtocolTruncatedDataTest, TruncatedSet) {
  TypeParam s;
  s.i32_set() = {};
  for (size_t i = 0; i < 30; ++i) {
    s.i32_set()->emplace((1ull << i));
  }

  testPartialDataHandling<CompactSerializer>(
      s, 3 /* headers */ + 30 /* 1b / element */);
}

TYPED_TEST(ProtocolTruncatedDataTest, TruncatedMap) {
  TypeParam s;
  s.i32_i16_map() = {};
  for (size_t i = 0; i < 30; ++i) {
    s.i32_i16_map()->emplace((1ull << i), i);
  }

  testPartialDataHandling<CompactSerializer>(
      s, 3 /* headers */ + 30 * 2 /* 2b / kv pair */);
}

TYPED_TEST(ProtocolTruncatedDataTest, TuncatedString_Compact) {
  TypeParam s;
  s.a_string() = "foobarbazstring";

  testPartialDataHandling<CompactSerializer>(
      s,
      2 /* field & length header */ + s.a_string()->size(),
      true /* isStringOrBinary */);
}

TYPED_TEST(ProtocolTruncatedDataTest, TuncatedString_Binary) {
  TypeParam s;
  s.a_string() = "foobarbazstring";

  testPartialDataHandling<BinarySerializer>(
      s,
      7 /* field & length header */ + s.a_string()->size(),
      true /* isStringOrBinary */);
}
