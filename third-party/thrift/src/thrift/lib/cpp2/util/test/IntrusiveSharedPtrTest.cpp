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

#include <folly/synchronization/RelaxedAtomic.h>
#include <thrift/lib/cpp2/util/IntrusiveSharedPtr.h>

// We test self assignment/move in this file, suppress warnings on them
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wself-move"
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif

using apache::thrift::util::BasicIntrusiveSharedPtrControlBlock;
using apache::thrift::util::IntrusiveSharedPtr;

namespace {

struct LifetimeTracker {
 public:
  struct Counts {
    int constructor = 0;
    int destructor = 0;
  };

  struct InstrusivePtrAccess {
    static void acquireRef(LifetimeTracker& obj) noexcept { obj.acquireRef(); }
    static BasicIntrusiveSharedPtrControlBlock::RefCount releaseRef(
        LifetimeTracker& obj) noexcept {
      return obj.releaseRef();
    }
    static BasicIntrusiveSharedPtrControlBlock::RefCount useCount(
        const LifetimeTracker& obj) noexcept {
      return obj.controlBlock_.useCount();
    }
  };
  using Ptr = IntrusiveSharedPtr<LifetimeTracker, InstrusivePtrAccess>;
  static_assert(
      sizeof(Ptr) == sizeof(Ptr::pointer),
      "IntrusiveSharedPtr should not waste space");

  explicit LifetimeTracker(Counts* counts = nullptr) noexcept
      : counts_(counts) {
    if (counts_) {
      counts_->constructor++;
    }
  }
  virtual ~LifetimeTracker() noexcept {
    if (counts_) {
      counts_->destructor++;
    }
  }
  LifetimeTracker(LifetimeTracker&&) = delete;
  LifetimeTracker& operator=(LifetimeTracker&&) = delete;

  auto numAcquires() const noexcept { return acquireRefs_.load(); }
  auto numReleases() const noexcept { return releaseRefs_.load(); }

 private:
  BasicIntrusiveSharedPtrControlBlock controlBlock_;

  Counts* counts_;
  folly::relaxed_atomic<std::int32_t> acquireRefs_{0};
  folly::relaxed_atomic<std::int32_t> releaseRefs_{0};

  void acquireRef() noexcept {
    acquireRefs_.fetch_add(1);
    controlBlock_.acquireRef();
  }
  BasicIntrusiveSharedPtrControlBlock::RefCount releaseRef() noexcept {
    releaseRefs_.fetch_add(1);
    return controlBlock_.releaseRef();
  }
};

struct LifetimeTrackerDerived : public LifetimeTracker {
  struct DerivedAccess : public InstrusivePtrAccess {};
  using Ptr = IntrusiveSharedPtr<LifetimeTrackerDerived, DerivedAccess>;

  using LifetimeTracker::LifetimeTracker;
};

} // namespace

TEST(IntrusiveSharedPtrTest, Basic) {
  LifetimeTracker::Counts counts;
  {
    LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
    EXPECT_EQ(std::addressof(*p1), p1.get());
    EXPECT_EQ(p1.operator->(), p1.get());
    EXPECT_EQ(static_cast<bool>(p1), true);
  }
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, CopyConstruct) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  LifetimeTracker::Ptr p2 = p1;
  EXPECT_EQ(p1->numAcquires(), 2);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, CopyConstructDerivedType) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);
  LifetimeTracker::Ptr p2 = p1;
  EXPECT_EQ(p1->numAcquires(), 2);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, MoveConstruct) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  LifetimeTracker::Ptr p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, MoveConstructDerivedType) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);
  LifetimeTracker::Ptr p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, CopyAssign) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  LifetimeTracker::Ptr p2;
  EXPECT_EQ(p2, nullptr);
  p2 = p1;
  EXPECT_EQ(p1->numAcquires(), 2);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, CopyAssignDerivedType) {
  LifetimeTracker::Counts counts;
  LifetimeTrackerDerived::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);

  LifetimeTracker::Ptr p2;
  p2 = p1;
  EXPECT_EQ(p1->numAcquires(), 2);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, CopyReassign) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 0);

  p2 = p1;
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);

  EXPECT_EQ(p1, p2);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);

  p2.reset();
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 2);
}

TEST(IntrusiveSharedPtrTest, CopyReassignDerived) {
  LifetimeTracker::Counts counts;
  LifetimeTrackerDerived::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);
  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 0);

  p2 = p1;
  EXPECT_EQ(p1->numAcquires(), 2);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);
  EXPECT_EQ(p1, p2);

  p1.reset();
  EXPECT_EQ(p2->numAcquires(), 2);
  EXPECT_EQ(p2->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);

  p2.reset();
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 2);
}

TEST(IntrusiveSharedPtrTest, MoveAssign) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  LifetimeTracker::Ptr p2;
  EXPECT_EQ(p2, nullptr);
  p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  EXPECT_NE(p1, p2);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, MoveAssignDerivedType) {
  LifetimeTracker::Counts counts;
  LifetimeTrackerDerived::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);

  LifetimeTracker::Ptr p2;
  p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  EXPECT_NE(p1, p2);

  p2.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrTest, MoveReassign) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 0);

  p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);

  p2.reset();
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 2);
}

TEST(IntrusiveSharedPtrTest, MoveReassignDerivedType) {
  LifetimeTracker::Counts counts;
  LifetimeTrackerDerived::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);
  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 0);

  p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);

  p2.reset();
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 2);
}

TEST(IntrusiveSharedPtrTest, SelfAssign) {
  LifetimeTracker::Counts counts;
  LifetimeTrackerDerived::Ptr p1 = LifetimeTrackerDerived::Ptr::make(&counts);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1 = p1;
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p1 = std::move(p1);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);
}

TEST(IntrusiveSharedPtrTest, ConstructFromUniquePtr) {
  {
    LifetimeTracker::Counts counts;
    LifetimeTracker::Ptr p1 =
        LifetimeTracker::Ptr(std::make_unique<LifetimeTracker>(&counts));
    EXPECT_EQ(counts.constructor, 1);
    EXPECT_EQ(counts.destructor, 0);

    p1.reset();
    EXPECT_EQ(counts.constructor, 1);
    EXPECT_EQ(counts.destructor, 1);
  }

  {
    LifetimeTracker::Counts counts;
    LifetimeTracker::Ptr p1 =
        LifetimeTracker::Ptr(std::make_unique<LifetimeTrackerDerived>(&counts));
    EXPECT_EQ(counts.constructor, 1);
    EXPECT_EQ(counts.destructor, 0);

    p1.reset();
    EXPECT_EQ(counts.constructor, 1);
    EXPECT_EQ(counts.destructor, 1);
  }
}

TEST(IntrusiveSharedPtrTest, AssignFromUniquePtr) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);

  p1 = std::make_unique<LifetimeTracker>(&counts);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 1);

  p1.reset();
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 2);

  p1 = std::make_unique<LifetimeTrackerDerived>(&counts);
  EXPECT_EQ(counts.constructor, 3);
  EXPECT_EQ(counts.destructor, 2);

  p1.reset();
  EXPECT_EQ(counts.constructor, 3);
  EXPECT_EQ(counts.destructor, 3);
}

TEST(IntrusiveSharedPtrTest, Swap) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make(&counts);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 0);

  auto p1ptr = p1.get();
  auto p2ptr = p2.get();
  swap(p1, p2);
  EXPECT_EQ(p1.get(), p2ptr);
  EXPECT_EQ(p2.get(), p1ptr);
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 0);
  EXPECT_EQ(p1->numAcquires(), 1);
  EXPECT_EQ(p1->numReleases(), 0);
  EXPECT_EQ(p2->numAcquires(), 1);
  EXPECT_EQ(p2->numReleases(), 0);

  p1 = nullptr;
  p2 = nullptr;
  EXPECT_EQ(counts.constructor, 2);
  EXPECT_EQ(counts.destructor, 2);
}

TEST(InstrusiveSharedPtrTest, Release) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  auto oldPtr = p1.get();

  LifetimeTracker* ptr = p1.unsafeRelease();

  EXPECT_EQ(p1, nullptr);
  EXPECT_EQ(ptr, oldPtr);

  EXPECT_EQ(ptr->numAcquires(), 1);
  EXPECT_EQ(ptr->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  delete ptr;
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(InstrusiveSharedPtrTest, ReleaseReclaim) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);
  LifetimeTracker* ptr = p1.unsafeRelease();
  LifetimeTracker::Ptr p2 =
      LifetimeTracker::Ptr(LifetimeTracker::Ptr::UnsafelyFromRawPointer(), ptr);
  LifetimeTracker::Ptr p3 =
      LifetimeTracker::Ptr(LifetimeTracker::Ptr::UnsafelyFromRawPointer(), ptr);

  EXPECT_EQ(ptr->numAcquires(), 3);
  EXPECT_EQ(ptr->numReleases(), 1);
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 0);

  p2.reset();
  p3.reset();
  EXPECT_EQ(counts.constructor, 1);
  EXPECT_EQ(counts.destructor, 1);
}

TEST(IntrusiveSharedPtrDeathTest, NonUniqueRelease) {
  EXPECT_DEATH(
      ({
        LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
        LifetimeTracker::Ptr p2 = p1;
        EXPECT_EQ(p1->numAcquires(), 2);
        EXPECT_EQ(p1->numReleases(), 0);

        p1.unsafeRelease();
      }),
      "Tried to release non-unique InstrusiveSharedPtr");
}

TEST(IntrusiveSharedPtrTest, Leak) {
  LifetimeTracker::Counts counts;
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make(&counts);

  auto ptr = std::move(p1).leak();
  EXPECT_EQ(ptr->numAcquires(), 1);
  EXPECT_EQ(ptr->numReleases(), 0);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1, nullptr);

  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::fromLeaked(ptr);
  EXPECT_EQ(ptr->numAcquires(), 1);
  EXPECT_EQ(ptr->numReleases(), 0);
  EXPECT_EQ(p2.get(), ptr);
}

TEST(IntrusiveSharedPtrTest, Hash) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(
      std::hash<LifetimeTracker::Ptr>()(p1),
      std::hash<LifetimeTracker*>()(p1.get()));

  p1.reset();
  EXPECT_EQ(
      std::hash<LifetimeTracker::Ptr>()(p1),
      std::hash<LifetimeTracker*>()(nullptr));
}

TEST(IntrusiveSharedPtrTest, UseCountBasic) {
  // Test use_count() for null pointer
  LifetimeTracker::Ptr p1;
  EXPECT_EQ(p1.use_count(), 0);

  // Test use_count() for single owner
  p1 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(p1.use_count(), 1);

  // Test use_count() after reset
  p1.reset();
  EXPECT_EQ(p1.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountCopyConstruct) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(p1.use_count(), 1);

  // Copy construction should increase use_count
  LifetimeTracker::Ptr p2 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);

  // Reset one copy, use_count should decrease
  p1.reset();
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);

  // Reset the last copy, use_count should be 0
  p2.reset();
  EXPECT_EQ(p2.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountMoveConstruct) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(p1.use_count(), 1);

  // Move construction should transfer ownership, not increase use_count
  LifetimeTracker::Ptr p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);

  p2.reset();
  EXPECT_EQ(p2.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountCopyAssign) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2;
  EXPECT_EQ(p1.use_count(), 1);
  EXPECT_EQ(p2.use_count(), 0);

  // Copy assignment should increase use_count
  p2 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);

  // Reset one copy
  p1.reset();
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);
}

TEST(IntrusiveSharedPtrTest, UseCountMoveAssign) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2;
  EXPECT_EQ(p1.use_count(), 1);
  EXPECT_EQ(p2.use_count(), 0);

  // Move assignment should transfer ownership, not increase use_count
  p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);

  p2.reset();
  EXPECT_EQ(p2.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountReassign) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(p1.use_count(), 1);
  EXPECT_EQ(p2.use_count(), 1);

  // Copy reassignment should decrease old object's count and increase new
  // object's count
  p2 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);

  // Reset both copies
  p1.reset();
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);

  p2.reset();
  EXPECT_EQ(p2.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountMoveReassign) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(p1.use_count(), 1);
  EXPECT_EQ(p2.use_count(), 1);

  // Move reassignment should destroy old object and transfer ownership of new
  // object
  p2 = std::move(p1);
  // @lint-ignore CLANGTIDY bugprone-use-after-move
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);

  p2.reset();
  EXPECT_EQ(p2.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountMultipleCopies) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(p1.use_count(), 1);

  // Create multiple copies
  LifetimeTracker::Ptr p2 = p1;
  LifetimeTracker::Ptr p3 = p1;
  LifetimeTracker::Ptr p4 = p2;
  EXPECT_EQ(p1.use_count(), 4);
  EXPECT_EQ(p2.use_count(), 4);
  EXPECT_EQ(p3.use_count(), 4);
  EXPECT_EQ(p4.use_count(), 4);

  // Reset copies one by one
  p1.reset();
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 3);
  EXPECT_EQ(p3.use_count(), 3);
  EXPECT_EQ(p4.use_count(), 3);

  p2.reset();
  EXPECT_EQ(p2.use_count(), 0);
  EXPECT_EQ(p3.use_count(), 2);
  EXPECT_EQ(p4.use_count(), 2);

  p3.reset();
  EXPECT_EQ(p3.use_count(), 0);
  EXPECT_EQ(p4.use_count(), 1);

  p4.reset();
  EXPECT_EQ(p4.use_count(), 0);
}

TEST(IntrusiveSharedPtrTest, UseCountSwap) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p3 = p1; // p1 has use_count 2, p2 has use_count 1

  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 1);
  EXPECT_EQ(p3.use_count(), 2);

  // Swap should preserve use_counts for the objects
  swap(p1, p2);
  EXPECT_EQ(p1.use_count(), 1); // Now points to what p2 pointed to
  EXPECT_EQ(p2.use_count(), 2); // Now points to what p1 pointed to
  EXPECT_EQ(p3.use_count(), 2); // Still points to original p1 object
}

TEST(IntrusiveSharedPtrTest, UseCountSelfAssign) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);

  // Self assignment should not change use_count
  p1 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);

  // Self move assignment should not change use_count
  p1 = std::move(p1);
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);
}

TEST(IntrusiveSharedPtrTest, UseCountFromUniquePtr) {
  // Construction from unique_ptr should have use_count 1
  LifetimeTracker::Ptr p1 = std::make_unique<LifetimeTracker>();
  EXPECT_EQ(p1.use_count(), 1);

  // Assignment from unique_ptr should have use_count 1
  LifetimeTracker::Ptr p2;
  p2 = std::make_unique<LifetimeTracker>();
  EXPECT_EQ(p2.use_count(), 1);

  // Copy should increase use_count
  LifetimeTracker::Ptr p3 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p3.use_count(), 2);
}

TEST(IntrusiveSharedPtrTest, UseCountNullAssignment) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  LifetimeTracker::Ptr p2 = p1;
  EXPECT_EQ(p1.use_count(), 2);
  EXPECT_EQ(p2.use_count(), 2);

  // Assignment to nullptr should reset use_count
  p1 = nullptr;
  EXPECT_EQ(p1.use_count(), 0);
  EXPECT_EQ(p2.use_count(), 1);

  p2 = nullptr;
  EXPECT_EQ(p2.use_count(), 0);
}
