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

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

#include <memory>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace apache::thrift::fast_thrift::rocket {
namespace {

// Tracks dtor invocations so tests can assert when cleanup ran.
struct DeletionCounter {
  int* counter;
  explicit DeletionCounter(int* c) : counter(c) {}
  ~DeletionCounter() {
    if (counter != nullptr) {
      ++(*counter);
    }
  }
  DeletionCounter(DeletionCounter&&) = delete;
  DeletionCounter& operator=(DeletionCounter&&) = delete;
};

struct OtherType {};

} // namespace

// =============================================================================
// Layout / size
// =============================================================================

TEST(TypeErasedPtrTest, LayoutMatchesPtrPlusFnPtr) {
#ifdef NDEBUG
  EXPECT_EQ(sizeof(TypeErasedPtr), 2 * sizeof(void*));
#else
  EXPECT_EQ(sizeof(TypeErasedPtr), 3 * sizeof(void*));
#endif
}

// =============================================================================
// Empty / default state
// =============================================================================

TEST(TypeErasedPtrTest, DefaultConstructedIsEmpty) {
  TypeErasedPtr p;
  EXPECT_FALSE(static_cast<bool>(p));
  EXPECT_EQ(p.get(), nullptr);
}

TEST(TypeErasedPtrTest, EmptyDestructorDoesNothing) {
  // Destructor on an empty handle should be a no-op (no deleter invoked
  // on a null pointer path).
  {
    TypeErasedPtr p;
  }
  SUCCEED();
}

TEST(TypeErasedPtrTest, ResetOnEmptyIsNoOp) {
  TypeErasedPtr p;
  p.reset();
  EXPECT_FALSE(static_cast<bool>(p));
}

// =============================================================================
// from_unique_ptr — owning factory, default delete
// =============================================================================

TEST(TypeErasedPtrTest, FromUniquePtrAdoptsOwnership) {
  int deletions = 0;
  {
    auto p = from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
    EXPECT_TRUE(static_cast<bool>(p));
    EXPECT_NE(p.get(), nullptr);
    EXPECT_EQ(deletions, 0);
  }
  EXPECT_EQ(deletions, 1);
}

TEST(TypeErasedPtrTest, FromUniquePtrPreservesPointerValue) {
  auto unique = std::make_unique<std::string>("hello");
  auto* before = unique.get();
  auto p = from_unique_ptr(std::move(unique));
  EXPECT_EQ(p.get(), before);
}

TEST(TypeErasedPtrTest, FromEmptyUniquePtrProducesEmptyHandle) {
  std::unique_ptr<DeletionCounter> empty;
  auto p = from_unique_ptr(std::move(empty));
  EXPECT_FALSE(static_cast<bool>(p));
}

// =============================================================================
// borrow — non-owning view
// =============================================================================

TEST(TypeErasedPtrTest, BorrowDoesNotInvokeDeleter) {
  int deletions = 0;
  auto live = std::make_unique<DeletionCounter>(&deletions);
  {
    auto p = borrow(live.get());
    EXPECT_TRUE(static_cast<bool>(p));
    EXPECT_EQ(p.get(), live.get());
  }
  // borrow() destruction must NOT free the underlying object.
  EXPECT_EQ(deletions, 0);
  // Original unique_ptr still owns it.
  live.reset();
  EXPECT_EQ(deletions, 1);
}

TEST(TypeErasedPtrTest, BorrowOfNullIsEmpty) {
  auto p = borrow(nullptr);
  EXPECT_FALSE(static_cast<bool>(p));
}

TEST(TypeErasedPtrTest, BorrowedPointerRoundTripsThroughRelease) {
  void* const sentinel = reinterpret_cast<void*>(0xdeadbeef);
  auto p = borrow(sentinel);
  EXPECT_EQ(p.get(), sentinel);
  EXPECT_EQ(p.release(), sentinel);
  EXPECT_FALSE(static_cast<bool>(p));
}

// =============================================================================
// with_custom_deleter — escape hatch for rescue-on-drop pattern
// =============================================================================

TEST(TypeErasedPtrTest, CustomDeleterRunsExactlyOnceOnDestruction) {
  struct RescueCtx {
    int* rescueRan;
    int* deletions;
    ~RescueCtx() { ++(*deletions); }
  };
  int rescueRan = 0;
  int deletions = 0;
  {
    auto p = with_custom_deleter(
        new RescueCtx{&rescueRan, &deletions}, [](void* ptr) noexcept {
          auto* ctx = static_cast<RescueCtx*>(ptr);
          ++(*ctx->rescueRan);
          delete ctx;
        });
    EXPECT_TRUE(static_cast<bool>(p));
  }
  EXPECT_EQ(rescueRan, 1);
  EXPECT_EQ(deletions, 1);
}

// =============================================================================
// release / release_as — ownership transfer
// =============================================================================

TEST(TypeErasedPtrTest, ReleaseDisengagesDeleter) {
  int deletions = 0;
  {
    auto p = from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
    auto* raw = static_cast<DeletionCounter*>(p.release());
    EXPECT_FALSE(static_cast<bool>(p));
    EXPECT_EQ(deletions, 0);
    delete raw;
  }
  EXPECT_EQ(deletions, 1);
}

TEST(TypeErasedPtrTest, ReleaseAsRecoversConcreteType) {
  int deletions = 0;
  auto p = from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
  std::unique_ptr<DeletionCounter> recovered = p.release_as<DeletionCounter>();
  ASSERT_NE(recovered.get(), nullptr);
  EXPECT_FALSE(static_cast<bool>(p));
  EXPECT_EQ(deletions, 0);
  recovered.reset();
  EXPECT_EQ(deletions, 1);
}

#ifndef NDEBUG
TEST(TypeErasedPtrTest, ReleaseAsWrongTypeThrowsInDebug) {
  int deletions = 0;
  auto p = from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
  EXPECT_THROW({ (void)p.release_as<OtherType>(); }, TypeErasedPtrTypeMismatch);
  // The handle is left untouched; original deleter still runs on scope exit.
  EXPECT_TRUE(static_cast<bool>(p));
}

#endif

// =============================================================================
// Move semantics
// =============================================================================

TEST(TypeErasedPtrTest, MoveConstructTransfersOwnership) {
  int deletions = 0;
  TypeErasedPtr src =
      from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
  TypeErasedPtr dst{std::move(src)};
  // Verifying the moved-from-empty contract is intentional here.
  // NOLINTNEXTLINE(bugprone-use-after-move)
  EXPECT_FALSE(static_cast<bool>(src));
  EXPECT_TRUE(static_cast<bool>(dst));
  EXPECT_EQ(deletions, 0);
  dst.reset();
  EXPECT_EQ(deletions, 1);
}

TEST(TypeErasedPtrTest, MoveAssignDeletesPriorAndAdoptsNew) {
  int firstDeletions = 0;
  int secondDeletions = 0;
  TypeErasedPtr p =
      from_unique_ptr(std::make_unique<DeletionCounter>(&firstDeletions));
  p = from_unique_ptr(std::make_unique<DeletionCounter>(&secondDeletions));
  // The first allocation must have been deleted as part of move-assign.
  EXPECT_EQ(firstDeletions, 1);
  EXPECT_EQ(secondDeletions, 0);
  p.reset();
  EXPECT_EQ(secondDeletions, 1);
}

TEST(TypeErasedPtrTest, SelfMoveAssignmentIsNoOp) {
  int deletions = 0;
  auto p = from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
  auto* before = p.get();
  // Funnel self-move through a reference to avoid -Wself-move.
  TypeErasedPtr& alias = p;
  p = std::move(alias);
  EXPECT_EQ(p.get(), before);
  EXPECT_EQ(deletions, 0);
}

// =============================================================================
// Reset
// =============================================================================

TEST(TypeErasedPtrTest, ResetRunsDeleterAndLeavesEmpty) {
  int deletions = 0;
  auto p = from_unique_ptr(std::make_unique<DeletionCounter>(&deletions));
  p.reset();
  EXPECT_FALSE(static_cast<bool>(p));
  EXPECT_EQ(deletions, 1);
  // Second reset is a no-op (no double-delete).
  p.reset();
  EXPECT_EQ(deletions, 1);
}

#ifndef NDEBUG
// =============================================================================
// Debug type info
// =============================================================================

TEST(TypeErasedPtrTest, FromUniquePtrRecordsTypeInDebug) {
  auto p = from_unique_ptr(std::make_unique<std::string>("x"));
  ASSERT_NE(p.storedType(), nullptr);
  EXPECT_EQ(*p.storedType(), typeid(std::string));
}

TEST(TypeErasedPtrTest, BorrowDoesNotRecordTypeInDebug) {
  int sentinel = 0;
  auto p = borrow(&sentinel);
  EXPECT_EQ(p.storedType(), nullptr);
}

TEST(TypeErasedPtrTest, CustomDeleterDoesNotRecordTypeInDebug) {
  auto p = with_custom_deleter(
      new int(7), [](void* ptr) noexcept { delete static_cast<int*>(ptr); });
  EXPECT_EQ(p.storedType(), nullptr);
}
#endif

} // namespace apache::thrift::fast_thrift::rocket
