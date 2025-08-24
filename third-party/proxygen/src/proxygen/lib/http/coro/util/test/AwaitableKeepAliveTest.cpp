/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/AwaitableKeepAlive.h"
#include "proxygen/lib/http/coro/util/Refcount.h"
#include <folly/coro/GmockHelpers.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/io/async/ScopedEventBaseThread.h>

namespace proxygen::coro::test {

using namespace proxygen::coro;

class TestObject : public detail::EnableAwaitableKeepAlive<TestObject> {
 public:
  folly::coro::Task<void> run() {
    co_await zeroRefs();
    delete this;
  }
};

class AwaitableSharedPtrTest : public testing::Test {
 public:
  void SetUp() override {
    obj_ = new TestObject();
  }

 protected:
  TestObject* obj_{nullptr};
};

CO_TEST_F(AwaitableSharedPtrTest, NoRefs) {
  co_await obj_->run();
}

CO_TEST_F(AwaitableSharedPtrTest, OneRef) {
  auto ka = obj_->acquireKeepAlive();
  auto executor = co_await folly::coro::co_current_executor;
  co_withExecutor(
      executor,
      [](detail::KeepAlivePtr<TestObject> p) -> folly::coro::Task<void> {
        co_return;
      }(std::move(ka)))
      .start();
  co_await obj_->run();
}

CO_TEST_F(AwaitableSharedPtrTest, TwoRefs) {
  {
    // Ref count goes 1 -> 2, 2 -> 1
    auto ka = obj_->acquireKeepAlive();
  }
  // Back to 1
  auto ka = obj_->acquireKeepAlive();
  auto executor = co_await folly::coro::co_current_executor;
  co_withExecutor(
      executor,
      [](detail::KeepAlivePtr<TestObject> p) -> folly::coro::Task<void> {
        co_return;
      }(std::move(ka)))
      .start();
  co_await obj_->run();
}

class TestKeepAlive : public testing::Test {
  // we can't use TestObject here due to false positive with free & atomic
  // writes: https://github.com/google/sanitizers/issues/602
 public:
  class AwaitableNoop : public detail::EnableAwaitableKeepAlive<AwaitableNoop> {
   public:
    using detail::EnableAwaitableKeepAlive<AwaitableNoop>::zeroRefs;
    uint64_t val{0};
  };
};

// tests both copy constructor and copy assignment operators
TEST_F(TestKeepAlive, CopyConstructorAssignment) {
  // object holds an implicit keepalive on construction
  AwaitableNoop obj;
  EXPECT_EQ(obj.numKeepAlives(), 1);

  detail::KeepAlivePtr<AwaitableNoop> empty{};
  EXPECT_FALSE(empty);

  auto ka = obj.acquireKeepAlive();
  EXPECT_EQ(obj.numKeepAlives(), 2);

  // copy constructor
  auto ka2 = ka;
  EXPECT_EQ(obj.numKeepAlives(), 3);
  // copy assignment
  ka2 = empty;
  EXPECT_EQ(obj.numKeepAlives(), 2);
  // another copy assignment
  ka2 = ka;
  EXPECT_EQ(obj.numKeepAlives(), 3);
}

// tests both move constructor and move assignment operators
TEST_F(TestKeepAlive, MoveConstructorAssignment) {
  // object holds an implicit keepalive on construction
  AwaitableNoop obj;
  EXPECT_EQ(obj.numKeepAlives(), 1);

  detail::KeepAlivePtr<AwaitableNoop> empty{};
  EXPECT_FALSE(empty);

  auto ka = obj.acquireKeepAlive();
  EXPECT_EQ(obj.numKeepAlives(), 2);

  // move constructor
  auto ka2 = std::move(ka);
  EXPECT_EQ(obj.numKeepAlives(), 2);
  EXPECT_FALSE(ka);

  // move assignment
  ka2 = std::move(empty);
  EXPECT_EQ(obj.numKeepAlives(), 1);

  // another move assignment
  ka2 = obj.acquireKeepAlive();
  EXPECT_EQ(obj.numKeepAlives(), 2);
}

TEST_F(TestKeepAlive, ConcurrentAcquireAndReleaseKeepAlives) {
  constexpr uint8_t kNumThreads{10};
  constexpr uint8_t kNumKeepAliveCopiesPerThread{100};
  std::array<folly::ScopedEventBaseThread, kNumThreads> evbThreads{};

  AwaitableNoop obj{};
  {
    // initial keep alive
    auto keepalive = obj.acquireKeepAlive();

    // race a bunch of threads to acquire & release keepalives
    for (uint8_t idx = 0; idx < evbThreads.size(); idx++) {
      evbThreads[idx].add([ka = keepalive]() {
        for (uint8_t i = 0; i < kNumKeepAliveCopiesPerThread; i++) {
          // copy constructor test
          auto copy_construct_ka = ka;

          // move constructor test
          auto move_construct_ka = std::move(copy_construct_ka);
          EXPECT_TRUE(move_construct_ka);  // should be holding a non-null ptr
          EXPECT_FALSE(copy_construct_ka); // was moved from

          // default initialized KeepAlivePtr test
          detail::KeepAlivePtr<AwaitableNoop> assign_ka{};
          EXPECT_FALSE(assign_ka); // default initialized should return false

          // move assignment test
          assign_ka = std::move(move_construct_ka);
          EXPECT_TRUE(assign_ka && !move_construct_ka);

          // copy assignment test
          assign_ka = ka;
          EXPECT_TRUE(assign_ka && ka);
        }
      });
    }
  }

  folly::EventBase evb;
  folly::coro::blockingWait(obj.zeroRefs(), &evb);
}

// test acquiring keep alive after the implicit keepalive is released via
// co_await
TEST_F(TestKeepAlive, UseAfterFree) {
  // object holds an implicit keepalive on construction; released on the next
  // line
  AwaitableNoop obj;
  EXPECT_EQ(obj.numKeepAlives(), 1);
  folly::coro::blockingWait(obj.zeroRefs());
  EXPECT_EQ(obj.numKeepAlives(), 0);

  // invoking acquireKeepAlive again should trigger an XCHECK
  EXPECT_DEATH(obj.acquireKeepAlive(), "use after free");
}

TEST_F(TestKeepAlive, SimpleMemberAccess) {
  AwaitableNoop obj;
  // default constructed should be empty
  detail::KeepAlivePtr<AwaitableNoop> ka;
  XCHECK(!ka);

  ka = obj.acquireKeepAlive();
  EXPECT_EQ(ka->val, 0);
  ka->val++;

  // const access
  const auto& kaRef = ka;
  EXPECT_EQ(kaRef->val, 1);
}

class RefcountTest : public testing::Test {
 protected:
  folly::EventBase evb_;
  Refcount refcount_{0};
};

TEST_F(RefcountTest, IncDec) {
  refcount_.incRef();
  EXPECT_EQ(refcount_.count(), 1);
  bool zero = false;
  co_withExecutor(&evb_, refcount_.zeroRefs())
      .start()
      .via(&evb_)
      .then([&zero](folly::Try<TimedBaton::Status> status) {
        EXPECT_FALSE(status.hasException());
        EXPECT_EQ(*status, TimedBaton::Status::signalled);
        zero = true;
      });
  evb_.loopOnce();
  EXPECT_FALSE(zero);
  refcount_.decRef();
  evb_.loopOnce();
  EXPECT_TRUE(zero);
}

TEST_F(RefcountTest, AlreadyZero) {
  bool zero = false;
  co_withExecutor(&evb_, refcount_.zeroRefs())
      .start()
      .via(&evb_)
      .then([&zero](folly::Try<TimedBaton::Status> status) {
        EXPECT_FALSE(status.hasException());
        EXPECT_EQ(*status, TimedBaton::Status::signalled);
        zero = true;
      });
  evb_.loopOnce();
  EXPECT_TRUE(zero);
}

} // namespace proxygen::coro::test
