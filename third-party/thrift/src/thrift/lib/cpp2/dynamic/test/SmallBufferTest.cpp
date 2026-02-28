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

#include <memory>
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

// ============================================================================
// Move Semantics Tests
// ============================================================================

// Test empty() and reset() methods
class SmallBufferEmptyStateTest : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferEmptyStateTest, DefaultConstructedIsEmpty) {
  Buffer buf;
  EXPECT_TRUE(buf.empty());
}

TEST_F(SmallBufferEmptyStateTest, EmplacedIsNotEmpty) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{42, 3.14});
  EXPECT_FALSE(buf.empty());
}

TEST_F(SmallBufferEmptyStateTest, ResetMakesEmpty) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{42, 3.14});
  EXPECT_FALSE(buf.empty());
  buf.reset();
  EXPECT_TRUE(buf.empty());
}

TEST_F(SmallBufferEmptyStateTest, ResetOnEmptyIsNoOp) {
  Buffer buf;
  EXPECT_TRUE(buf.empty());
  buf.reset(); // Should not crash
  EXPECT_TRUE(buf.empty());
}

TEST_F(SmallBufferEmptyStateTest, ResetCallsDestructor) {
  static int destructorCount = 0;
  struct CountingType {
    std::string value;
    explicit CountingType(std::string v) : value(std::move(v)) {}
    CountingType(const CountingType& other) : value(other.value) {}
    CountingType(CountingType&& other) noexcept
        : value(std::move(other.value)) {}
    ~CountingType() { ++destructorCount; }
  };

  destructorCount = 0;
  Buffer buf;
  buf.emplace<CountingType>("test");
  EXPECT_EQ(destructorCount, 0);
  buf.reset();
  EXPECT_EQ(destructorCount, 1);
  EXPECT_TRUE(buf.empty());
}

// Note: Trivial-only buffers (SupportsNonTrivial=false) don't have empty() or
// reset() methods - they're only available when SupportsNonTrivial=true.

// Test move constructor
class SmallBufferMoveConstructorTest : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferMoveConstructorTest, MoveConstructTrivialType) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2(std::move(buf1));
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf2.as<TrivialType>().data, 3.14);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty()); // Source is now empty
}

TEST_F(SmallBufferMoveConstructorTest, MoveConstructNonTrivialType) {
  Buffer buf1;
  buf1.emplace<NonTrivialType>("hello world");

  Buffer buf2(std::move(buf1));
  EXPECT_EQ(buf2.as<NonTrivialType>().value, "hello world");
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty()); // Source is now empty
}

TEST_F(SmallBufferMoveConstructorTest, MoveConstructFromEmpty) {
  Buffer buf1; // Empty
  Buffer buf2(std::move(buf1));
  EXPECT_TRUE(buf2.empty());
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveConstructorTest, MoveConstructCallsMoveNotCopy) {
  static int copyCount = 0;
  static int moveCount = 0;
  struct TrackingType {
    std::string value;
    explicit TrackingType(std::string v) : value(std::move(v)) {}
    TrackingType(const TrackingType& other) : value(other.value) {
      ++copyCount;
    }
    TrackingType(TrackingType&& other) noexcept
        : value(std::move(other.value)) {
      ++moveCount;
    }
    ~TrackingType() = default;
  };

  copyCount = 0;
  moveCount = 0;

  Buffer buf1;
  buf1.emplace<TrackingType>("test");
  EXPECT_EQ(copyCount, 0);
  EXPECT_EQ(moveCount, 0);

  Buffer buf2(std::move(buf1));
  EXPECT_EQ(copyCount, 0);
  EXPECT_EQ(moveCount, 1); // Move constructor was called
}

// Test move assignment
class SmallBufferMoveAssignmentTest : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferMoveAssignmentTest, MoveAssignTrivialType) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2;
  buf2.emplace<TrivialType>(TrivialType{0, 0.0});

  buf2 = std::move(buf1);
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  EXPECT_DOUBLE_EQ(buf2.as<TrivialType>().data, 3.14);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveAssignmentTest, MoveAssignNonTrivialType) {
  Buffer buf1;
  buf1.emplace<NonTrivialType>("source");

  Buffer buf2;
  buf2.emplace<NonTrivialType>("destination");

  buf2 = std::move(buf1);
  EXPECT_EQ(buf2.as<NonTrivialType>().value, "source");
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveAssignmentTest, MoveAssignFromEmpty) {
  Buffer buf1; // Empty

  Buffer buf2;
  buf2.emplace<TrivialType>(TrivialType{42, 3.14});

  buf2 = std::move(buf1);
  EXPECT_TRUE(buf2.empty());
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveAssignmentTest, MoveAssignToEmpty) {
  Buffer buf1;
  buf1.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer buf2; // Empty

  buf2 = std::move(buf1);
  EXPECT_EQ(buf2.as<TrivialType>().value, 42);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveAssignmentTest, SelfMoveAssign) {
  Buffer buf;
  buf.emplace<TrivialType>(TrivialType{42, 3.14});

  Buffer* ptr = &buf;
  buf = std::move(*ptr); // Self-move should be safe
  // After self-move, behavior is implementation-defined but should not crash
  // In our implementation, it should remain unchanged
  EXPECT_EQ(buf.as<TrivialType>().value, 42);
}

TEST_F(SmallBufferMoveAssignmentTest, MoveAssignDestroysOldValue) {
  static int destructorCount = 0;
  struct CountingType {
    std::string value;
    explicit CountingType(std::string v) : value(std::move(v)) {}
    CountingType(const CountingType& other) : value(other.value) {}
    CountingType(CountingType&& other) noexcept
        : value(std::move(other.value)) {}
    ~CountingType() { ++destructorCount; }
  };

  destructorCount = 0;
  {
    Buffer buf1;
    buf1.emplace<CountingType>("first");

    Buffer buf2;
    buf2.emplace<CountingType>("second");

    buf2 = std::move(buf1); // Should destroy "second", move "first"
    EXPECT_EQ(
        destructorCount, 2); // "second" destroyed + source destroyed by move op
  }
  // "first" (now in buf2) destroyed when buf2 goes out of scope
  EXPECT_EQ(destructorCount, 3);
}

// Test emplace destroys existing value
class SmallBufferEmplaceDestroyTest : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferEmplaceDestroyTest, DoubleEmplaceDestroysFirst) {
  static int destructorCount = 0;
  struct CountingType {
    std::string value;
    explicit CountingType(std::string v) : value(std::move(v)) {}
    CountingType(const CountingType& other) : value(other.value) {}
    CountingType(CountingType&& other) noexcept
        : value(std::move(other.value)) {}
    ~CountingType() { ++destructorCount; }
  };

  destructorCount = 0;
  {
    Buffer buf;
    buf.emplace<CountingType>("first");
    EXPECT_EQ(destructorCount, 0);

    buf.emplace<CountingType>("second"); // Should destroy "first"
    EXPECT_EQ(destructorCount, 1);
    EXPECT_EQ(buf.as<CountingType>().value, "second");
  }
  EXPECT_EQ(
      destructorCount, 2); // "second" destroyed when buf goes out of scope
}

TEST_F(SmallBufferEmplaceDestroyTest, EmplaceAfterResetWorks) {
  Buffer buf;
  buf.emplace<NonTrivialType>("first");
  buf.reset();
  EXPECT_TRUE(buf.empty());

  buf.emplace<NonTrivialType>("second");
  EXPECT_FALSE(buf.empty());
  EXPECT_EQ(buf.as<NonTrivialType>().value, "second");
}

// Test copy of empty buffer
class SmallBufferCopyEmptyTest : public ::testing::Test {
 protected:
  using Buffer = SmallBuffer<64, alignof(void*), true>;
};

TEST_F(SmallBufferCopyEmptyTest, CopyConstructFromEmpty) {
  Buffer buf1; // Empty
  Buffer buf2(buf1);
  EXPECT_TRUE(buf2.empty());
}

TEST_F(SmallBufferCopyEmptyTest, CopyAssignFromEmpty) {
  Buffer buf1; // Empty

  Buffer buf2;
  buf2.emplace<TrivialType>(TrivialType{42, 3.14});

  buf2 = buf1;
  EXPECT_TRUE(buf2.empty());
}

// ============================================================================
// MoveOnly Template Parameter Tests
// ============================================================================

// Move-only type for testing (not copyable)
struct MoveOnlyType {
  std::unique_ptr<int> value;

  explicit MoveOnlyType(int v) : value(std::make_unique<int>(v)) {}
  MoveOnlyType(MoveOnlyType&&) noexcept = default;
  MoveOnlyType& operator=(MoveOnlyType&&) noexcept = default;
  MoveOnlyType(const MoveOnlyType&) = delete;
  MoveOnlyType& operator=(const MoveOnlyType&) = delete;
  ~MoveOnlyType() = default;
};
static_assert(!std::is_copy_constructible_v<MoveOnlyType>);
static_assert(std::is_move_constructible_v<MoveOnlyType>);

class SmallBufferMoveOnlyTest : public ::testing::Test {
 protected:
  // MoveOnly=true buffer for move-only types
  using MoveOnlyBuffer = SmallBuffer<64, alignof(void*), true, true>;
  // Regular buffer (MoveOnly=false) for comparison
  using CopyableBuffer = SmallBuffer<64, alignof(void*), true, false>;
};

// Verify MoveOnly buffer is not copyable
TEST_F(SmallBufferMoveOnlyTest, MoveOnlyBufferIsNotCopyable) {
  static_assert(
      !std::is_copy_constructible_v<MoveOnlyBuffer>,
      "MoveOnly buffer should not be copy constructible");
  static_assert(
      !std::is_copy_assignable_v<MoveOnlyBuffer>,
      "MoveOnly buffer should not be copy assignable");
}

// Verify MoveOnly buffer is movable
TEST_F(SmallBufferMoveOnlyTest, MoveOnlyBufferIsMovable) {
  static_assert(
      std::is_move_constructible_v<MoveOnlyBuffer>,
      "MoveOnly buffer should be move constructible");
  static_assert(
      std::is_move_assignable_v<MoveOnlyBuffer>,
      "MoveOnly buffer should be move assignable");
}

// Verify regular buffer is copyable
TEST_F(SmallBufferMoveOnlyTest, CopyableBufferIsCopyable) {
  static_assert(
      std::is_copy_constructible_v<CopyableBuffer>,
      "Copyable buffer should be copy constructible");
  static_assert(
      std::is_copy_assignable_v<CopyableBuffer>,
      "Copyable buffer should be copy assignable");
}

TEST_F(SmallBufferMoveOnlyTest, EmplaceAndAccessMoveOnlyType) {
  MoveOnlyBuffer buf;
  buf.emplace<MoveOnlyType>(42);
  EXPECT_EQ(*buf.as<MoveOnlyType>().value, 42);
}

TEST_F(SmallBufferMoveOnlyTest, MoveConstructMoveOnlyType) {
  MoveOnlyBuffer buf1;
  buf1.emplace<MoveOnlyType>(42);

  MoveOnlyBuffer buf2(std::move(buf1));
  EXPECT_EQ(*buf2.as<MoveOnlyType>().value, 42);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveOnlyTest, MoveAssignMoveOnlyType) {
  MoveOnlyBuffer buf1;
  buf1.emplace<MoveOnlyType>(42);

  MoveOnlyBuffer buf2;
  buf2.emplace<MoveOnlyType>(0);

  buf2 = std::move(buf1);
  EXPECT_EQ(*buf2.as<MoveOnlyType>().value, 42);
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_TRUE(buf1.empty());
}

TEST_F(SmallBufferMoveOnlyTest, MoveOnlyBufferWithCopyableType) {
  // MoveOnly buffer can also hold copyable types (just won't be able to copy
  // the buffer)
  MoveOnlyBuffer buf;
  buf.emplace<NonTrivialType>("hello");
  EXPECT_EQ(buf.as<NonTrivialType>().value, "hello");

  MoveOnlyBuffer buf2(std::move(buf));
  EXPECT_EQ(buf2.as<NonTrivialType>().value, "hello");
}

TEST_F(SmallBufferMoveOnlyTest, MoveOnlyBufferReset) {
  MoveOnlyBuffer buf;
  buf.emplace<MoveOnlyType>(42);
  EXPECT_FALSE(buf.empty());

  buf.reset();
  EXPECT_TRUE(buf.empty());
}

TEST_F(SmallBufferMoveOnlyTest, MoveOnlyBufferDoubleEmplace) {
  static int destructorCount = 0;
  struct CountingMoveOnly {
    std::unique_ptr<int> value;
    explicit CountingMoveOnly(int v) : value(std::make_unique<int>(v)) {}
    CountingMoveOnly(CountingMoveOnly&&) noexcept = default;
    CountingMoveOnly& operator=(CountingMoveOnly&&) noexcept = default;
    CountingMoveOnly(const CountingMoveOnly&) = delete;
    CountingMoveOnly& operator=(const CountingMoveOnly&) = delete;
    ~CountingMoveOnly() { ++destructorCount; }
  };

  destructorCount = 0;
  MoveOnlyBuffer buf;
  buf.emplace<CountingMoveOnly>(1);
  EXPECT_EQ(destructorCount, 0);

  buf.emplace<CountingMoveOnly>(2); // Should destroy first value
  EXPECT_EQ(destructorCount, 1);
  EXPECT_EQ(*buf.as<CountingMoveOnly>().value, 2);
}

} // namespace apache::thrift::dynamic::detail
