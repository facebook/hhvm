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

#include <thrift/lib/cpp2/op/Serializer.h>

#include <string_view>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/op/Testing.h>

namespace apache::thrift::op {
namespace {
using namespace type;

// Helpers for encoding/decoding using string.
template <typename T>
std::string encode(const Serializer& serializer, T&& value) {
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  // Allocate 16KB at a time; leave some room for the IOBuf overhead
  constexpr size_t kDesiredGrowth = (1 << 14) - 64;
  serializer.encode(
      std::forward<T>(value), folly::io::QueueAppender(&queue, kDesiredGrowth));

  std::string result;
  queue.appendToString(result);
  return result;
}

template <typename Tag>
native_type<Tag> decode(const Serializer& serializer, std::string_view data) {
  folly::IOBuf buf(folly::IOBuf::WRAP_BUFFER, data.data(), data.size());
  folly::io::Cursor cursor{&buf};
  return serializer.decode<Tag>(cursor);
}

template <typename Tag>
native_type<Tag> decodeViaAnyRef(
    const Serializer& serializer, std::string_view data) {
  folly::IOBuf buf(folly::IOBuf::WRAP_BUFFER, data.data(), data.size());
  folly::io::Cursor cursor{&buf};
  native_type<Tag> value;
  serializer.decode(cursor, AnyRef(value));
  return value;
}

AnyValue decode(
    const Serializer& serializer, const Type& type, std::string_view data) {
  folly::IOBuf buf(folly::IOBuf::WRAP_BUFFER, data.data(), data.size());
  folly::io::Cursor cursor{&buf};
  AnyValue result;
  serializer.decode(type, cursor, result);
  return result;
}

TEST(SerializerTest, TagSerializer) {
  test::FollyToStringSerializer<type::i32_t> intCodec;
  int i = 1;
  auto ri = Ref::to<type::i32_t>(i);
  EXPECT_EQ(encode(intCodec, ri), "1");
  EXPECT_EQ(decode<type::i32_t>(intCodec, "1"), 1);

  double d = 2.5;
  auto rd = Ref::to<type::double_t>(d);
  Serializer& anyCodec(intCodec);
  EXPECT_EQ(encode(anyCodec, ri), "1");
  EXPECT_EQ(encode(anyCodec, AnyValue::create<type::i32_t>(2)), "2");
  EXPECT_THROW(encode(anyCodec, rd), std::bad_any_cast);
  EXPECT_THROW(
      encode(anyCodec, AnyValue::create<type::double_t>(2.5)),
      folly::BadPolyCast);

  EXPECT_EQ(decode<type::i32_t>(anyCodec, "1"), 1);
  EXPECT_EQ(decodeViaAnyRef<type::i32_t>(anyCodec, "1"), 1);
  EXPECT_EQ(decode(anyCodec, type::i32_t{}, "1").as<type::i32_t>(), 1);

  EXPECT_THROW(decode<type::double_t>(anyCodec, "1.0"), std::bad_any_cast);
  EXPECT_THROW(decode(anyCodec, type::double_t{}, "1.0"), std::bad_any_cast);
  EXPECT_THROW(
      decodeViaAnyRef<type::double_t>(anyCodec, "1.0"), std::bad_any_cast);
}

TEST(SerializerTest, MultiSerializer) {
  int i = 1;
  auto ri = Ref::to<type::i32_t>(i);

  test::MultiSerializer multiSerializer;
  const Serializer& serializer = multiSerializer;
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());

  // Can handle int refs.
  EXPECT_EQ(encode(serializer, ri), "1");
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkIntEnc());
  EXPECT_EQ(decode<type::i32_t>(serializer, "1"), 1);
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkIntDec());

  // Can handle int AnyValue.
  AnyValue any = decode(serializer, type::i32_t{}, "1");
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAnyIntDec());
  EXPECT_EQ(any.as<type::i32_t>(), 1);
  EXPECT_EQ(encode(serializer, any), "1");
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkIntEnc());

  double d = 2.5;
  auto rd = Ref::to<type::double_t>(d);
  // Can handle doubles.
  EXPECT_EQ(encode(serializer, rd), "2.5");
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkDblEnc());
  EXPECT_EQ(decode<type::double_t>(serializer, "0.5"), 0.5);
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkDblDec());

  // Can handle double AnyValue.
  any = decode(serializer, type::double_t{}, "1");
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAnyDblDec());
  EXPECT_EQ(any.as<type::double_t>(), 1.0);
  EXPECT_EQ(encode(serializer, any), "1");
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkDblEnc());

  // Cannot handle float.
  float f = 1.5f;
  auto rf = Ref::to<type::float_t>(f);
  EXPECT_THROW(encode(serializer, rf), std::bad_any_cast);
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());
  EXPECT_THROW(decode<type::float_t>(serializer, "1"), std::bad_any_cast);
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());

  // Cannot handle std::any(float).
  any = AnyValue::create<type::float_t>(1.5f);
  EXPECT_THROW(decode(serializer, type::float_t{}, "1"), std::bad_any_cast);
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAny(1));
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());
  EXPECT_THROW(encode(serializer, any), std::bad_any_cast);
  FBTHRIFT_SCOPED_CHECK(multiSerializer.checkAndResetAll());
}

} // namespace
} // namespace apache::thrift::op
