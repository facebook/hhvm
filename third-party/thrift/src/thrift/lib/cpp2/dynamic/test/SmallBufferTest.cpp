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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/detail/SmallBuffer.h>

#include <string>
#include <type_traits>

namespace apache::thrift::dynamic::detail {

// Trivial type for testing
struct TrivialType {
  int value;
  double data;
};
static_assert(std::is_trivially_copyable_v<TrivialType>);
static_assert(std::is_trivially_destructible_v<TrivialType>);

// Non-trivial type for testing
struct NonTrivialType {
  std::string value;

  explicit NonTrivialType(std::string v) : value(std::move(v)) {}
  NonTrivialType(const NonTrivialType& other) : value(other.value) {}
  NonTrivialType& operator=(const NonTrivialType& other) {
    value = other.value;
    return *this;
  }
  ~NonTrivialType() = default;
};
static_assert(!std::is_trivially_copyable_v<NonTrivialType>);

// Test SmallBuffer with SupportsNonTrivial = false (only trivial types)
class SmallBufferTrivialOnlyTest : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), false>;
};

TEST_F(SmallBufferTrivialOnlyTest, EmplaceAndAccess) {
  Buffer buf;
  auto& val = buf.emplace<TrivialType>(TrivialType{42, 3.14});
  EXPECT_EQ(val.value, 42);
  EXPECT_DOUBLE_EQ(val.data, 3.14);
}

TEST_F(SmallBufferTrivialOnlyTest, AsAccess) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{100, 2.71});
  EXPECT_EQ(buf.as<TrivialType>().value, 100);
  EXPECT_DOUBLE_EQ(buf.as<TrivialType>().data, 2.71);
}

TEST_F(SmallBufferTrivialOnlyTest, ConstAsAccess) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{200, 1.41});
  const Buffer& constBuf = buf;
  EXPECT_EQ(constBuf.as<TrivialType>().value, 200);
  EXPECT_DOUBLE_EQ(constBuf.as<TrivialType>().data, 1.41);
}

TEST_F(SmallBufferTrivialOnlyTest, CopyConstruct) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2(buf1);
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf2.as<TrivialType>().data, 3.14);
}

TEST_F(SmallBufferTrivialOnlyTest, CopyAssign) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2;
  buf2.emplace<TrivialType>(TrivialType{0, 0.0});

  buf2 = buf1;
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf2.as<TrivialType>().data, 3.14);
}

TEST_F(SmallBufferTrivialOnlyTest, SelfAssign) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer* ptr = &buf;
  buf = *ptr; // Use pointer indirection to avoid self-assign warning
  EXPECT_EQ(buf.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf.as<TrivialType>().data, 3.14);
}

// Test SmallBuffer with SupportsNonTrivial = true using trivial types
class SmallBufferNonTrivialSupportWithTrivialTypesTest
    : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferNonTrivialSupportWithTrivialTypesTest, EmplaceAndAccess) {
  Buffer buf;
  auto& val = buf.emplace<TrivialType>(TrivialType{42, 3.14});
  EXPECT_EQ(val.value, 42);
  EXPECT_DOUBLE_EQ(val.data, 3.14);
}

TEST_F(SmallBufferNonTrivialSupportWithTrivialTypesTest, AsAccess) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{100, 2.71});
  EXPECT_EQ(buf.as<TrivialType>().value, 100);
  EXPECT_DOUBLE_EQ(buf.as<TrivialType>().data, 2.71);
}

TEST_F(SmallBufferNonTrivialSupportWithTrivialTypesTest, CopyConstruct) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2(buf1);
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf2.as<TrivialType>().data, 3.14);
}

TEST_F(SmallBufferNonTrivialSupportWithTrivialTypesTest, CopyAssign) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2;
  buf2.emplace<TrivialType>(TrivialType{0, 0.0});

  buf2 = buf1;
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf2.as<TrivialType>().data, 3.14);
}

TEST_F(SmallBufferNonTrivialSupportWithTrivialTypesTest, SelfAssign) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer* ptr = &buf;
  buf = *ptr; // Use pointer indirection to avoid self-assign warning
  EXPECT_EQ(buf.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf.as<TrivialType>().data, 3.14);
}

// Test SmallBuffer with SupportsNonTrivial = true using non-trivial types
class SmallBufferNonTrivialSupportWithNonTrivialTypesTest
    : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferNonTrivialSupportWithNonTrivialTypesTest, EmplaceAndAccess) {
  Buffer buf;
  auto& val = buf.emplace<NonTrivialType>("hello");
  EXPECT_EQ(val.value, "hello");
}

TEST_F(SmallBufferNonTrivialSupportWithNonTrivialTypesTest, AsAccess) {
  Buffer buf;
  buf.emplace<NonTrivialType>("world");
  EXPECT_EQ(buf.as<NonTrivialType>().value, "world");
}

TEST_F(SmallBufferNonTrivialSupportWithNonTrivialTypesTest, ConstAsAccess) {
  Buffer buf;
  buf.emplace<NonTrivialType>("test");
  const Buffer& constBuf = buf;
  EXPECT_EQ(constBuf.as<NonTrivialType>().value, "test");
}

TEST_F(SmallBufferNonTrivialSupportWithNonTrivialTypesTest, CopyConstruct) {
  Buffer buf1;
  buf1.emplace<NonTrivialType>("original");

  Buffer buf2(buf1);
  EXPECT_EQ(buf2.as<NonTrivialType>().value, "original");
  // Verify original is unchanged
  EXPECT_EQ(buf1.as<NonTrivialType>().value, "original");
}

TEST_F(SmallBufferNonTrivialSupportWithNonTrivialTypesTest, CopyAssign) {
  Buffer buf1;
  buf1.emplace<NonTrivialType>("source");

  Buffer buf2;
  buf2.emplace<NonTrivialType>("destination");

  buf2 = buf1;
  EXPECT_EQ(buf2.as<NonTrivialType>().value, "source");
  // Verify original is unchanged
  EXPECT_EQ(buf1.as<NonTrivialType>().value, "source");
}

TEST_F(SmallBufferNonTrivialSupportWithNonTrivialTypesTest, SelfAssign) {
  Buffer buf;
  buf.emplace<NonTrivialType>("self");

  Buffer* ptr = &buf;
  buf = *ptr; // Use pointer indirection to avoid self-assign warning
  EXPECT_EQ(buf.as<NonTrivialType>().value, "self");
}

TEST_F(
    SmallBufferNonTrivialSupportWithNonTrivialTypesTest,
    DestructorCalledOnDestroy) {
  static int destructorCount = 0;
  struct CountingType {
    std::string value;
    explicit CountingType(std::string v) : value(std::move(v)) {}
    CountingType(const CountingType& other) : value(other.value) {}
    ~CountingType() { ++destructorCount; }
  };

  destructorCount = 0;
  {
    Buffer buf;
    buf.emplace<CountingType>("test");
  }
  EXPECT_EQ(destructorCount, 1);
}

TEST_F(
    SmallBufferNonTrivialSupportWithNonTrivialTypesTest,
    DestructorCalledOnAssign) {
  static int destructorCount = 0;
  struct CountingType {
    std::string value;
    explicit CountingType(std::string v) : value(std::move(v)) {}
    CountingType(const CountingType& other) : value(other.value) {}
    ~CountingType() { ++destructorCount; }
  };

  destructorCount = 0;
  {
    Buffer buf1;
    buf1.emplace<CountingType>("first");

    Buffer buf2;
    buf2.emplace<CountingType>("second");

    buf2 = buf1; // Should destroy "second" before copying "first"
    EXPECT_EQ(destructorCount, 1);
  }
  // Both buffers destroyed
  EXPECT_EQ(destructorCount, 3);
}

} // namespace apache::thrift::dynamic::detail
