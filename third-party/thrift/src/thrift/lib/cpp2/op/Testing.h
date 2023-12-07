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

// gtest matchers for thrift ops.
#pragma once

#include <utility>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp/Field.h>
#include <thrift/lib/cpp2/op/Clear.h>
#include <thrift/lib/cpp2/op/Compare.h>
#include <thrift/lib/cpp2/op/Serializer.h>
#include <thrift/lib/cpp2/type/Protocol.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/Testing.h>
#include <thrift/lib/cpp2/type/detail/Runtime.h>

namespace apache::thrift::test {

template <typename Tag, typename T = type::native_type<Tag>>
struct IdenticalToMatcher {
  T expected;
  template <typename U>
  bool MatchAndExplain(const U& actual, ::testing::MatchResultListener*) const {
    return op::identical<Tag>(actual, expected);
  }
  void DescribeTo(std::ostream* os) const {
    *os << "is identical to " << ::testing::PrintToString(expected);
  }
  void DescribeNegationTo(std::ostream* os) const {
    *os << "is not identical to " << ::testing::PrintToString(expected);
  }
};

template <typename Tag, typename T = type::native_type<Tag>>
auto IsIdenticalTo(T expected) {
  return ::testing::MakePolymorphicMatcher(
      IdenticalToMatcher<Tag, T>{std::move(expected)});
}

template <typename Tag, typename T>
struct EqualToMatcher {
  T expected;
  template <typename U>
  bool MatchAndExplain(const U& actual, ::testing::MatchResultListener*) const {
    return op::equal<Tag>(actual, expected);
  }
  void DescribeTo(std::ostream* os) const {
    *os << "is equal to " << ::testing::PrintToString(expected);
  }
  void DescribeNegationTo(std::ostream* os) const {
    *os << "is not equal to " << ::testing::PrintToString(expected);
  }
};

template <typename Tag, typename T = type::native_type<Tag>>
auto IsEqualTo(T expected) {
  return ::testing::MakePolymorphicMatcher(
      EqualToMatcher<Tag, T>{std::move(expected)});
}

template <typename Tag>
struct EmptyMatcher {
  template <typename U>
  bool MatchAndExplain(const U& actual, ::testing::MatchResultListener*) const {
    return op::isEmpty<Tag>(actual);
  }
  void DescribeTo(std::ostream* os) const { *os << "is empty"; }
  void DescribeNegationTo(std::ostream* os) const { *os << "is not empty"; }
};

template <typename Tag>
auto IsEmpty() {
  return ::testing::MakePolymorphicMatcher(EmptyMatcher<Tag>{});
}

// Applies a patch twice and checks the results.
template <
    typename P,
    typename T1 = typename P::value_type,
    typename T2 = T1,
    typename T3 = T2>
void expectPatch(
    P patch, const T1& actual, const T2& expected1, const T3& expected2) {
  { // Applying twice produces the expected results.
    auto actual1 = actual;
    patch.apply(actual1);
    EXPECT_EQ(actual1, expected1);
    if (actual1 != expected1) {
      EXPECT_FALSE(patch.empty());
    }
    patch.apply(actual1);
    EXPECT_EQ(actual1, expected2);
  }
  { // Merging with self, is the same as applying twice.
    patch.merge(patch);
    auto actual2 = actual;
    patch.apply(actual2);
    EXPECT_EQ(actual2, expected2);
  }
  { // Reset should be a noop patch.
    patch.reset();
    EXPECT_TRUE(patch.empty());
    auto actual3 = actual;
    patch.apply(actual3);
    EXPECT_EQ(actual3, actual);
  }
}

template <typename P>
void expectPatch(
    P patch,
    const folly::IOBuf& actual,
    const folly::IOBuf& expected1,
    const folly::IOBuf& expected2) {
  { // Applying twice produces the expected results.
    auto actual1 = actual;
    patch.apply(actual1);
    EXPECT_TRUE(folly::IOBufEqualTo{}(actual1, expected1))
        << "actual " << actual1.to<std::string>() << " expected "
        << expected1.to<std::string>();
    if (!folly::IOBufEqualTo{}(actual1, expected1)) {
      EXPECT_FALSE(patch.empty());
    }
    patch.apply(actual1);
    EXPECT_TRUE(folly::IOBufEqualTo{}(actual1, expected2))
        << "actual " << actual1.to<std::string>() << " expected "
        << expected2.to<std::string>();
    ;
  }
  { // Merging with self, is the same as applying twice.
    patch.merge(patch);
    auto actual2 = actual;
    patch.apply(actual2);
    EXPECT_TRUE(folly::IOBufEqualTo{}(actual2, expected2))
        << "actual " << actual2.to<std::string>() << " expected "
        << expected2.to<std::string>();
    ;
  }
  { // Reset should be a noop patch.
    patch.reset();
    EXPECT_TRUE(patch.empty());
    auto actual3 = actual;
    patch.apply(actual3);
    EXPECT_TRUE(folly::IOBufEqualTo{}(actual3, actual))
        << "actual " << actual3.to<std::string>() << " expected "
        << actual.to<std::string>();
    ;
  }
}

template <typename P, typename T1 = typename P::value_type, typename T2 = T1>
void expectPatch(P patch, const T1& actual, const T2& expected) {
  expectPatch(std::move(patch), actual, expected, expected);
}

// Checks round trips using the given serializer.
template <typename Tag, typename S, typename T = type::native_type<Tag>>
void expectRoundTrip(const S& seralizer, const T& expected) {
  folly::IOBufQueue queue;
  seralizer.encode(expected, folly::io::QueueAppender{&queue, 2 << 4});
  folly::io::Cursor cursor(queue.front());
  T actual = seralizer.template decode<Tag>(cursor);
  EXPECT_EQ(actual, expected);
}

// Always serializes integers to the number 1.
class Number1Serializer
    : public op::TagSerializer<type::i32_t, Number1Serializer> {
  using Base = op::TagSerializer<type::i32_t, Number1Serializer>;

 public:
  const type::Protocol& getProtocol() const override;

  using Base::encode;
  void encode(const int&, folly::io::QueueAppender&& appender) const {
    std::string data = "number 1!!";
    appender.push(reinterpret_cast<const uint8_t*>(data.data()), data.size());
  }

  using Base::decode;
  void decode(folly::io::Cursor& cursor, int& value) const {
    cursor.readFixedString(10);
    value = 1;
  }
};

extern const type::Protocol kFollyToStringProtocol;

// A serializer based on `folly::to<std::string>`.
template <typename Tag>
class FollyToStringSerializer
    : public op::TagSerializer<Tag, FollyToStringSerializer<Tag>> {
  using Base = op::TagSerializer<Tag, FollyToStringSerializer>;
  using T = type::native_type<Tag>;

 public:
  const type::Protocol& getProtocol() const override {
    return kFollyToStringProtocol;
  }
  using Base::encode;
  void encode(const T& value, folly::io::QueueAppender&& appender) const {
    std::string data = folly::to<std::string>(value);
    appender.push(reinterpret_cast<const uint8_t*>(data.data()), data.size());
  }
  using Base::decode;
  void decode(folly::io::Cursor& cursor, T& value) const {
    value = folly::to<T>(cursor.readFixedString(cursor.totalLength()));
  }
};

class MultiSerializer : public op::Serializer {
  using Base = op::Serializer;

 public:
  mutable size_t intEncCount = 0;
  mutable size_t dblEncCount = 0;

  mutable size_t intDecCount = 0;
  mutable size_t dblDecCount = 0;
  mutable size_t anyDecCount = 0;

  using Base::decode;
  using Base::encode;

  const type::Protocol& getProtocol() const override {
    return kFollyToStringProtocol;
  }
  void encode(
      type::ConstRef value, folly::io::QueueAppender&& appender) const override;
  void encode(type::AnyConstRef value, folly::io::QueueAppender&& appender)
      const override;
  void decode(folly::io::Cursor& cursor, type::Ref value) const override;
  void decode(folly::io::Cursor& cursor, type::AnyRef value) const override;
  void decode(
      const type::Type& type,
      folly::io::Cursor& cursor,
      type::AnyValue& value) const override;

  // Helper functions to check the statis
  void checkAndResetInt(size_t enc, size_t dec) const;
  void checkAndResetDbl(size_t enc, size_t dec) const;
  void checkAndResetAny(size_t dec) const;
  void checkAndResetAll() const;
  void checkAnyDec() const;
  void checkIntEnc() const;
  void checkIntDec() const;
  void checkDblEnc() const;
  void checkDblDec() const;
  void checkAnyIntDec() const;
  void checkAnyDblDec() const;
};

} // namespace apache::thrift::test
