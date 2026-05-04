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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>

#include <memory>
#include <string>
#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>

namespace apache::thrift::fast_thrift::channel_pipeline {
namespace {

// Test helper to track destructor calls
struct DestructorTracker {
  explicit DestructorTracker(int* counter) : counter_(counter) {}
  ~DestructorTracker() {
    if (counter_) {
      ++(*counter_);
    }
  }

  // Move constructor
  DestructorTracker(DestructorTracker&& other) noexcept
      : counter_(other.counter_) {
    other.counter_ = nullptr;
  }

  DestructorTracker& operator=(DestructorTracker&& other) noexcept {
    if (this != &other) {
      counter_ = other.counter_;
      other.counter_ = nullptr;
    }
    return *this;
  }

  // Non-copyable
  DestructorTracker(const DestructorTracker&) = delete;
  DestructorTracker& operator=(const DestructorTracker&) = delete;

  int* counter_;
};

// ============================================================================
// Size and Layout Tests
// ============================================================================

TEST(TypeErasedBoxTest, InlineCapacityIs120Bytes) {
  EXPECT_EQ(TypeErasedBox::kInlineCapacity, 120);
}

TEST(TypeErasedBoxTest, InlineAlignmentIs8Bytes) {
  EXPECT_EQ(TypeErasedBox::kInlineAlign, alignof(void*));
}

// ============================================================================
// Construction Tests
// ============================================================================

TEST(TypeErasedBoxTest, DefaultConstructorCreatesEmptyBox) {
  TypeErasedBox box;
  EXPECT_TRUE(box.empty());
  EXPECT_FALSE(static_cast<bool>(box));
}

TEST(TypeErasedBoxTest, ConstructWithInt) {
  TypeErasedBox box(42);
  EXPECT_FALSE(box.empty());
  EXPECT_TRUE(static_cast<bool>(box));
  EXPECT_EQ(box.get<int>(), 42);
}

TEST(TypeErasedBoxTest, ConstructWithString) {
  std::string str = "hello world";
  TypeErasedBox box(std::move(str));
  EXPECT_FALSE(box.empty());
  EXPECT_EQ(box.get<std::string>(), "hello world");
}

TEST(TypeErasedBoxTest, ConstructWithUniquePtr) {
  auto ptr = std::make_unique<int>(123);
  TypeErasedBox box(std::move(ptr));
  EXPECT_FALSE(box.empty());
  EXPECT_EQ(*box.get<std::unique_ptr<int>>(), 123);
}

TEST(TypeErasedBoxTest, ConstructWithIOBuf) {
  auto buf = folly::IOBuf::create(64);
  buf->append(10);
  TypeErasedBox box(std::move(buf));
  EXPECT_FALSE(box.empty());
  EXPECT_EQ(box.get<BytesPtr>()->length(), 10);
}

// ============================================================================
// Get Tests
// ============================================================================

TEST(TypeErasedBoxTest, GetReturnsCorrectValue) {
  TypeErasedBox box(42);
  EXPECT_EQ(box.get<int>(), 42);
}

TEST(TypeErasedBoxTest, GetReturnsReference) {
  TypeErasedBox box(42);
  box.get<int>() = 100;
  EXPECT_EQ(box.get<int>(), 100);
}

TEST(TypeErasedBoxTest, ConstGetReturnsConstReference) {
  TypeErasedBox box(42);
  const TypeErasedBox& constBox = box;
  EXPECT_EQ(constBox.get<int>(), 42);
}

// ============================================================================
// Take Tests
// ============================================================================

TEST(TypeErasedBoxTest, TakeMovesValueOut) {
  auto ptr = std::make_unique<int>(42);
  TypeErasedBox box(std::move(ptr));

  auto result = box.take<std::unique_ptr<int>>();
  EXPECT_EQ(*result, 42);
}

TEST(TypeErasedBoxTest, TakeLeavesBoxEmpty) {
  TypeErasedBox box(42);
  auto value = box.take<int>();
  EXPECT_EQ(value, 42);
  EXPECT_TRUE(box.empty());
}

TEST(TypeErasedBoxTest, TakeIOBuf) {
  auto buf = folly::IOBuf::create(64);
  buf->append(20);
  TypeErasedBox box(std::move(buf));

  auto result = box.take<BytesPtr>();
  EXPECT_EQ(result->length(), 20);
  EXPECT_TRUE(box.empty());
}

// ============================================================================
// Move Semantics Tests
// ============================================================================

TEST(TypeErasedBoxTest, MoveConstructor) {
  TypeErasedBox box1(42);
  TypeErasedBox box2(std::move(box1));

  EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(box2.empty());
  EXPECT_EQ(box2.get<int>(), 42);
}

TEST(TypeErasedBoxTest, MoveAssignment) {
  TypeErasedBox box1(42);
  TypeErasedBox box2;

  box2 = std::move(box1);

  EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(box2.empty());
  EXPECT_EQ(box2.get<int>(), 42);
}

TEST(TypeErasedBoxTest, MoveAssignmentToNonEmpty) {
  int destruct_count = 0;
  {
    TypeErasedBox box1(42);
    TypeErasedBox box2{DestructorTracker{&destruct_count}};

    // Moving into non-empty box should destroy existing value
    box2 = std::move(box1);

    EXPECT_EQ(destruct_count, 1); // Old value destroyed
    EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
    EXPECT_FALSE(box2.empty());
    EXPECT_EQ(box2.get<int>(), 42);
  }
  // box2 destruction doesn't affect our counter since it holds an int now
}

TEST(TypeErasedBoxTest, MoveFromLeavesSourceEmpty) {
  TypeErasedBox box1(std::string("test"));
  TypeErasedBox box2 = std::move(box1);

  EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(box2.empty());
}

TEST(TypeErasedBoxTest, SelfMoveAssignmentIsNoOp) {
  TypeErasedBox box(42);
  auto* addr = &box;
  box = std::move(*addr);

  // Should not crash and value should be preserved
  EXPECT_FALSE(box.empty());
  EXPECT_EQ(box.get<int>(), 42);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST(TypeErasedBoxTest, Reset) {
  TypeErasedBox box(42);
  EXPECT_FALSE(box.empty());

  box.reset();

  EXPECT_TRUE(box.empty());
}

TEST(TypeErasedBoxTest, ResetOnEmptyBoxIsNoOp) {
  TypeErasedBox box;
  EXPECT_TRUE(box.empty());

  box.reset();

  EXPECT_TRUE(box.empty());
}

// ============================================================================
// Empty/Bool Conversion Tests
// ============================================================================

TEST(TypeErasedBoxTest, EmptyCheck) {
  TypeErasedBox empty_box;
  TypeErasedBox filled_box(42);

  EXPECT_TRUE(empty_box.empty());
  EXPECT_FALSE(filled_box.empty());
}

TEST(TypeErasedBoxTest, BoolConversion) {
  TypeErasedBox empty_box;
  TypeErasedBox filled_box(42);

  EXPECT_FALSE(static_cast<bool>(empty_box));
  EXPECT_TRUE(static_cast<bool>(filled_box));

  // Works in if statements
  if (empty_box) {
    FAIL() << "Empty box should not convert to true";
  }
  if (!filled_box) {
    FAIL() << "Filled box should not convert to false";
  }
}

// ============================================================================
// Destructor Tests
// ============================================================================

TEST(TypeErasedBoxTest, DestructorCallsDeleter) {
  int destruct_count = 0;
  {
    TypeErasedBox box{DestructorTracker{&destruct_count}};
    EXPECT_EQ(destruct_count, 0);
  }
  EXPECT_EQ(destruct_count, 1);
}

TEST(TypeErasedBoxTest, DestructorNotCalledOnEmptyBox) {
  TypeErasedBox box;
  // Should not crash when destroying empty box
}

TEST(TypeErasedBoxTest, ResetCallsDeleter) {
  int destruct_count = 0;
  TypeErasedBox box{DestructorTracker{&destruct_count}};
  EXPECT_EQ(destruct_count, 0);

  box.reset();

  EXPECT_EQ(destruct_count, 1);
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(TypeErasedBoxTest, EraseAndBox) {
  auto box = erase_and_box(42);
  EXPECT_FALSE(box.empty());
  EXPECT_EQ(box.get<int>(), 42);
}

TEST(TypeErasedBoxTest, EraseAndBoxWithString) {
  auto box = erase_and_box(std::string("hello"));
  EXPECT_FALSE(box.empty());
  EXPECT_EQ(box.get<std::string>(), "hello");
}

// ============================================================================
// Type Safety Tests (Debug Only)
// ============================================================================

#ifndef NDEBUG

TEST(TypeErasedBoxTest, TypeNameInDebug) {
  TypeErasedBox box(42);
  // Type name should contain "int" (demangled)
  EXPECT_EQ(box.typeName(), "int");
}

TEST(TypeErasedBoxTest, TypeNameOnEmptyBox) {
  TypeErasedBox box;
  EXPECT_EQ(box.typeName(), "<empty>");
}

TEST(TypeErasedBoxTest, StoredTypeInDebug) {
  TypeErasedBox box(42);
  EXPECT_NE(box.storedType(), nullptr);
  EXPECT_EQ(*box.storedType(), typeid(int));
}

TEST(TypeErasedBoxTest, StoredTypeOnEmptyBox) {
  TypeErasedBox box;
  EXPECT_EQ(box.storedType(), nullptr);
}

// ============================================================================
// Exception Tests (Debug Mode Only)
// ============================================================================

TEST(TypeErasedBoxExceptionTest, GetOnEmptyThrowsEmptyAccess) {
  TypeErasedBox box;
  EXPECT_THROW(box.get<int>(), TypeErasedBoxEmptyAccess);
}

TEST(TypeErasedBoxExceptionTest, GetOnEmptyExceptionMessage) {
  TypeErasedBox box;
  try {
    box.get<int>();
    FAIL() << "Expected TypeErasedBoxEmptyAccess exception";
  } catch (const TypeErasedBoxEmptyAccess& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("get()"), std::string::npos);
    EXPECT_NE(msg.find("empty box"), std::string::npos);
  }
}

TEST(TypeErasedBoxExceptionTest, GetWithWrongTypeThrowsTypeMismatch) {
  TypeErasedBox box(42);
  EXPECT_THROW(box.get<std::string>(), TypeErasedBoxTypeMismatch);
}

TEST(TypeErasedBoxExceptionTest, GetWithWrongTypeExceptionMessage) {
  TypeErasedBox box(42);
  try {
    box.get<std::string>();
    FAIL() << "Expected TypeErasedBoxTypeMismatch exception";
  } catch (const TypeErasedBoxTypeMismatch& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("get()"), std::string::npos);
    EXPECT_NE(msg.find("type mismatch"), std::string::npos);
    EXPECT_NE(msg.find("string"), std::string::npos); // requested type
    EXPECT_NE(msg.find("int"), std::string::npos); // actual type

    // Verify exception accessors
    EXPECT_EQ(e.requestedType(), typeid(std::string));
    EXPECT_NE(e.actualType(), nullptr);
    EXPECT_EQ(*e.actualType(), typeid(int));
  }
}

TEST(TypeErasedBoxExceptionTest, TakeOnEmptyThrowsEmptyAccess) {
  TypeErasedBox box;
  EXPECT_THROW(box.take<int>(), TypeErasedBoxEmptyAccess);
}

TEST(TypeErasedBoxExceptionTest, TakeOnEmptyExceptionMessage) {
  TypeErasedBox box;
  try {
    box.take<int>();
    FAIL() << "Expected TypeErasedBoxEmptyAccess exception";
  } catch (const TypeErasedBoxEmptyAccess& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("take()"), std::string::npos);
    EXPECT_NE(msg.find("empty box"), std::string::npos);
  }
}

TEST(TypeErasedBoxExceptionTest, TakeWithWrongTypeThrowsTypeMismatch) {
  TypeErasedBox box(42);
  EXPECT_THROW(box.take<std::string>(), TypeErasedBoxTypeMismatch);
}

TEST(TypeErasedBoxExceptionTest, TakeWithWrongTypeExceptionMessage) {
  TypeErasedBox box(42);
  try {
    box.take<std::string>();
    FAIL() << "Expected TypeErasedBoxTypeMismatch exception";
  } catch (const TypeErasedBoxTypeMismatch& e) {
    std::string msg = e.what();
    EXPECT_NE(msg.find("take()"), std::string::npos);
    EXPECT_NE(msg.find("type mismatch"), std::string::npos);
    EXPECT_NE(msg.find("string"), std::string::npos); // requested type
    EXPECT_NE(msg.find("int"), std::string::npos); // actual type

    // Verify exception accessors
    EXPECT_EQ(e.requestedType(), typeid(std::string));
    EXPECT_NE(e.actualType(), nullptr);
    EXPECT_EQ(*e.actualType(), typeid(int));
  }
}

TEST(TypeErasedBoxExceptionTest, ConstGetOnEmptyThrows) {
  const TypeErasedBox box;
  EXPECT_THROW(box.get<int>(), TypeErasedBoxEmptyAccess);
}

TEST(TypeErasedBoxExceptionTest, ConstGetWithWrongTypeThrows) {
  TypeErasedBox mutableBox(42);
  const TypeErasedBox& box = mutableBox;
  EXPECT_THROW(box.get<std::string>(), TypeErasedBoxTypeMismatch);
}

#endif // NDEBUG

// ============================================================================
// Complex Type Tests
// ============================================================================

TEST(TypeErasedBoxTest, ConstructWithVector) {
  std::vector<int> vec = {1, 2, 3, 4, 5};
  TypeErasedBox box(std::move(vec));

  EXPECT_FALSE(box.empty());
  auto& stored = box.get<std::vector<int>>();
  EXPECT_EQ(stored.size(), 5);
  EXPECT_EQ(stored[0], 1);
  EXPECT_EQ(stored[4], 5);
}

TEST(TypeErasedBoxTest, ConstructWithMap) {
  std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}};
  TypeErasedBox box(std::move(map));

  EXPECT_FALSE(box.empty());
  auto& stored = box.get<std::unordered_map<std::string, int>>();
  EXPECT_EQ(stored.size(), 2);
  EXPECT_EQ(stored["a"], 1);
  EXPECT_EQ(stored["b"], 2);
}

TEST(TypeErasedBoxTest, MoveConstructorVsMoveFrom) {
  // Moving a TypeErasedBox into another invokes the move constructor,
  // not the template constructor. So we don't get nesting - we get transfer.
  TypeErasedBox inner(42);
  TypeErasedBox outer(std::move(inner));

  EXPECT_TRUE(inner.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(outer.empty());

  // outer now holds the int 42 directly, not a TypeErasedBox containing 42
  EXPECT_EQ(outer.get<int>(), 42);
}

// ============================================================================
// Compile-Time Inline Capacity Tests
// ============================================================================

TEST(TypeErasedBoxTest, FitsInlineCompileTimeCheck) {
  // Compile-time checks for common types that should fit in 120 bytes
  static_assert(TypeErasedBox::fits_inline<int>());
  static_assert(TypeErasedBox::fits_inline<long>());
  static_assert(TypeErasedBox::fits_inline<double>());
  static_assert(TypeErasedBox::fits_inline<void*>());
  static_assert(TypeErasedBox::fits_inline<std::unique_ptr<int>>());
  static_assert(TypeErasedBox::fits_inline<BytesPtr>());
  static_assert(TypeErasedBox::fits_inline<std::string>());

  // Large types should not fit inline (would fail static_assert at
  // construction)
  struct Large {
    char data[200];
  };
  static_assert(sizeof(Large) > TypeErasedBox::kInlineCapacity);
  static_assert(!TypeErasedBox::fits_inline<Large>());

  // Type that fits the new 120-byte capacity
  struct Fits120 {
    char data[120];
  };
  static_assert(TypeErasedBox::fits_inline<Fits120>());

  // Type that was too big for old 56-byte capacity but fits new 120-byte
  struct Fits120NotOld56 {
    char data[100];
  };
  static_assert(sizeof(Fits120NotOld56) == 100);
  static_assert(TypeErasedBox::fits_inline<Fits120NotOld56>());

  SUCCEED(); // If we get here, all static_asserts passed
}

TEST(TypeErasedBoxTest, BytesPtrFitsInline) {
  // BytesPtr is unique_ptr<IOBuf> which is 8 bytes - should easily fit
  static_assert(sizeof(BytesPtr) <= TypeErasedBox::kInlineCapacity);
  static_assert(TypeErasedBox::fits_inline<BytesPtr>());

  auto buf = folly::IOBuf::create(64);
  buf->append(10);
  TypeErasedBox box(std::move(buf));

  EXPECT_FALSE(box.empty());
  EXPECT_EQ(box.get<BytesPtr>()->length(), 10);
}

TEST(TypeErasedBoxTest, UniquePtrFitsInline) {
  // unique_ptr<int> should fit inline (8 bytes)
  static_assert(TypeErasedBox::fits_inline<std::unique_ptr<int>>());

  auto ptr = std::make_unique<int>(123);
  TypeErasedBox box(std::move(ptr));

  EXPECT_EQ(*box.get<std::unique_ptr<int>>(), 123);
}

TEST(TypeErasedBoxTest, StringFitsInline) {
  // std::string is typically 32 bytes - should fit in 120 bytes
  static_assert(TypeErasedBox::fits_inline<std::string>());

  TypeErasedBox box(std::string("hello world"));
  EXPECT_EQ(box.get<std::string>(), "hello world");
}

// ============================================================================
// Move Value Tests
// ============================================================================

TEST(TypeErasedBoxTest, MoveInlineValue) {
  TypeErasedBox box1(42);
  TypeErasedBox box2(std::move(box1));

  EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(box2.empty());
  EXPECT_EQ(box2.get<int>(), 42);
}

TEST(TypeErasedBoxTest, MoveInlineIOBuf) {
  auto buf = folly::IOBuf::create(64);
  buf->append(15);
  TypeErasedBox box1(std::move(buf));

  TypeErasedBox box2(std::move(box1));

  EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(box2.empty());
  EXPECT_EQ(box2.get<BytesPtr>()->length(), 15);
}

TEST(TypeErasedBoxTest, MoveAssignInlineValue) {
  TypeErasedBox box1(42);
  TypeErasedBox box2;

  box2 = std::move(box1);

  EXPECT_TRUE(box1.empty()); // NOLINT(bugprone-use-after-move)
  EXPECT_FALSE(box2.empty());
  EXPECT_EQ(box2.get<int>(), 42);
}

TEST(TypeErasedBoxTest, TakeFromInlineValue) {
  TypeErasedBox box(42);

  int value = box.take<int>();

  EXPECT_EQ(value, 42);
  EXPECT_TRUE(box.empty());
}

TEST(TypeErasedBoxTest, TakeFromInlineIOBuf) {
  auto buf = folly::IOBuf::create(64);
  buf->append(25);
  TypeErasedBox box(std::move(buf));

  BytesPtr result = box.take<BytesPtr>();

  EXPECT_EQ(result->length(), 25);
  EXPECT_TRUE(box.empty());
}

TEST(TypeErasedBoxTest, InlineDestructorCalledOnReset) {
  int destruct_count = 0;
  {
    TypeErasedBox box{DestructorTracker{&destruct_count}};
    EXPECT_EQ(destruct_count, 0);

    box.reset();

    EXPECT_EQ(destruct_count, 1);
    EXPECT_TRUE(box.empty());
  }
  // No double-destruction
  EXPECT_EQ(destruct_count, 1);
}

TEST(TypeErasedBoxTest, InlineDestructorCalledOnDestruction) {
  int destruct_count = 0;
  {
    TypeErasedBox box{DestructorTracker{&destruct_count}};
    EXPECT_EQ(destruct_count, 0);
  }
  EXPECT_EQ(destruct_count, 1);
}

// ============================================================================
// Zero-Cost Wrapper Verification Tests
// ============================================================================

TEST(TypeErasedBoxTest, ZeroCostInReleaseBuild) {
  // In release builds, TypeErasedBox should be exactly the same size
  // as the underlying SmallBuffer (120-byte inline + ops_ pointer).
  // In debug builds, it adds one type_info* pointer.
#ifdef NDEBUG
  EXPECT_EQ(sizeof(TypeErasedBox), 120 + sizeof(void*));
#else
  EXPECT_EQ(sizeof(TypeErasedBox), 120 + 2 * sizeof(void*));
#endif
}

} // namespace
} // namespace apache::thrift::fast_thrift::channel_pipeline
