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

namespace {
template <class T>
auto hash(T&& value) {
  return std::hash<std::remove_cvref_t<T>>{}(std::forward<T>(value));
}
} // namespace

TEST(IntrusiveSharedPtrTest, Hash) {
  LifetimeTracker::Ptr p1 = LifetimeTracker::Ptr::make();
  EXPECT_EQ(hash(p1), hash(p1.get()));

  p1.reset();
  EXPECT_EQ(hash(p1), hash(nullptr));
}
