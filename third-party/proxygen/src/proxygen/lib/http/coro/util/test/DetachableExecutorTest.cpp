/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/DetachableExecutor.h"
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GTest.h>

using namespace proxygen::coro::detail;

TEST(DetachableExecutor, Simple) {
  folly::EventBase evb;
  DetachableExecutor exec{&evb};

  uint8_t num_fns_executed{0};
  // invoking ::add on an attached executor should behave identically to
  // EventBase::add
  exec.add([&num_fns_executed]() { ++num_fns_executed; });
  evb.loopOnce(EVLOOP_NONBLOCK);
  EXPECT_EQ(num_fns_executed, 1);
}

TEST(DetachableExecutor, AcquireGuardTest) {
  folly::EventBase evb;
  DetachableExecutor exec{&evb};

  // default state is Undetachable
  EXPECT_EQ(exec.getState(), DetachableExecutor::State::Undetachable);
  {
    // test acquire guard construction and destruction
    auto g = exec.acquireGuard();
    EXPECT_EQ(exec.getState(), DetachableExecutor::State::Detachable);
  }
  EXPECT_EQ(exec.getState(), DetachableExecutor::State::Undetachable);
}

TEST(DetachableExecutor, TestDetachPriorToAdd) {
  // construct DetachableExecutor and immediately detach
  folly::EventBase evb;
  DetachableExecutor exec{&evb};
  {
    // must acquire guard before detach
    auto g = exec.acquireGuard();
    exec.detachEvb();
  }

  // XCHECK will fail here; invariant is that ::add must be invoked while
  // attached
  EXPECT_DEATH(exec.add([]() {}), "Check failed");
}

TEST(DetachableExecutor, TestDetachAfterAdd) {
  folly::EventBase evb;
  DetachableExecutor exec{&evb};

  class RunBeforeLoop : public folly::EventBase::LoopCallback {
   public:
    explicit RunBeforeLoop(folly::Func&& fn) : fn_(std::move(fn)) {
    }

    void runLoopCallback() noexcept override {
      fn_();
      delete this;
    }

   private:
    folly::Func fn_;
  };

  // enqueue callback, but detach the executor in runBeforeLoop callback (i.e.
  // detach after ::add but before the cob is executed)
  uint8_t num_fns_executed{0};
  exec.add([&num_fns_executed]() { ++num_fns_executed; });
  evb.runBeforeLoop(new RunBeforeLoop([&]() {
    auto g = exec.acquireGuard();
    exec.detachEvb();
  }));

  evb.loopOnce(EVLOOP_NONBLOCK);
  EXPECT_EQ(num_fns_executed, 0);

  // if attach with an already queued cob, cob will be scheduled to run in next
  // iter
  exec.attachEvb(&evb);
  evb.loopOnce(EVLOOP_NONBLOCK);
  EXPECT_EQ(num_fns_executed, 1);
}

TEST(DetachableExecutor, AddWhenScheduled) {
  folly::EventBase evb;
  DetachableExecutor exec{&evb};

  uint8_t num_fns_executed{0};
  auto incFn = [&]() { num_fns_executed++; };

  // this cob will be scheduled/executed prior to the subsequent
  // DetachableExecutor::add below
  evb.runInLoop([&]() {
    // DetachableExecutor::add here will be invoked while DetachableExecutor is
    // scheduled
    exec.add(incFn);
  });
  exec.add([&num_fns_executed]() { ++num_fns_executed; });

  // each fns should run in its own loop
  evb.loopOnce(EVLOOP_NONBLOCK);
  EXPECT_EQ(num_fns_executed, 1);
  evb.loopOnce(EVLOOP_NONBLOCK);
  EXPECT_EQ(num_fns_executed, 2);
}
