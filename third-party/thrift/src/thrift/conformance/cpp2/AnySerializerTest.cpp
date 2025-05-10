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

#include <thrift/conformance/cpp2/AnySerializer.h>

#include <any>

#include <gtest/gtest.h>
#include <thrift/conformance/cpp2/Testing.h>

namespace apache::thrift::conformance {

namespace {

// Helpers for encoding/decoding using stirngs.
std::string encode(const AnySerializer& serializer, any_ref value) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  // Allocate 16KB at a time; leave some room for the IOBuf overhead
  constexpr size_t kDesiredGrowth = (1 << 14) - 64;
  serializer.encode(value, folly::io::QueueAppender(&queue, kDesiredGrowth));

  std::string result;
  queue.appendToString(result);
  return result;
}

template <typename T>
T decode(const AnySerializer& serializer, std::string_view data) {
  folly::IOBuf buf(folly::IOBuf::WRAP_BUFFER, data.data(), data.size());
  folly::io::Cursor cursor{&buf};
  return serializer.decode<T>(cursor);
}

std::any decode(
    const AnySerializer& serializer,
    const std::type_info& typeinfo,
    std::string_view data) {
  folly::IOBuf buf(folly::IOBuf::WRAP_BUFFER, data.data(), data.size());
  folly::io::Cursor cursor{&buf};
  std::any result;
  serializer.decode(typeinfo, cursor, result);
  return result;
}

TEST(AnySerializerTest, TypedSerializer) {
  FollyToStringSerializer<int> intCodec;
  EXPECT_EQ(encode(intCodec, 1), "1");
  EXPECT_EQ(decode<int>(intCodec, "1"), 1);

  AnySerializer& anyCodec(intCodec);
  EXPECT_EQ(encode(anyCodec, 1), "1");
  EXPECT_EQ(encode(anyCodec, std::any(1)), "1");
  EXPECT_THROW(encode(anyCodec, 2.5), std::bad_any_cast);
  EXPECT_THROW(encode(anyCodec, std::any(2.5)), std::bad_any_cast);

  EXPECT_EQ(decode<int>(anyCodec, "1"), 1);
  EXPECT_EQ(std::any_cast<int>(decode(anyCodec, typeid(int), "1")), 1);

  EXPECT_THROW(decode<double>(anyCodec, "1.0"), std::bad_any_cast);
  EXPECT_THROW(decode(anyCodec, typeid(double), "1.0"), std::bad_any_cast);
}

TEST(SerializerTest, MultiSerializer) {
  MultiSerializer multiSerializer;
  const AnySerializer& serializer = multiSerializer;

  THRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());

  // Can handle ints.
  EXPECT_EQ(encode(serializer, 1), "1");
  THRIFT_SCOPED_CHECK(multiSerializer.checkIntEnc());
  EXPECT_EQ(decode<int>(serializer, "1"), 1);
  THRIFT_SCOPED_CHECK(multiSerializer.checkIntDec());

  // Can handle std::any(int).
  std::any a = decode(serializer, typeid(int), "1");
  THRIFT_SCOPED_CHECK(multiSerializer.checkAnyIntDec());
  EXPECT_EQ(std::any_cast<int>(a), 1);
  EXPECT_EQ(encode(serializer, a), "1");
  THRIFT_SCOPED_CHECK(multiSerializer.checkIntEnc());

  // Can handle doubles.
  EXPECT_EQ(encode(serializer, 2.5), "2.5");
  THRIFT_SCOPED_CHECK(multiSerializer.checkDblEnc());
  EXPECT_EQ(decode<double>(serializer, "0.5"), 0.5f);
  THRIFT_SCOPED_CHECK(multiSerializer.checkDblDec());

  // Can handle std::any(double).
  a = decode(serializer, typeid(double), "1");
  THRIFT_SCOPED_CHECK(multiSerializer.checkAnyDblDec());
  EXPECT_EQ(std::any_cast<double>(a), 1.0);
  EXPECT_EQ(encode(serializer, a), "1");
  THRIFT_SCOPED_CHECK(multiSerializer.checkDblEnc());

  // Cannot handle float.
  EXPECT_THROW(encode(serializer, 1.0f), std::bad_any_cast);
  THRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());
  EXPECT_THROW(decode<float>(serializer, "1"), std::bad_any_cast);
  THRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());

  // Cannot handle std::any(float).
  a = 1.0f;
  EXPECT_THROW(decode(serializer, typeid(float), "1"), std::bad_any_cast);
  THRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAny(1));
  THRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());
  EXPECT_THROW(encode(serializer, a), std::bad_any_cast);
  THRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());
}

} // namespace
} // namespace apache::thrift::conformance
