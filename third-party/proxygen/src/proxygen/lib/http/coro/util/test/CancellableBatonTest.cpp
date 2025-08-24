/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/CancellableBaton.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"

#include <folly/coro/BlockingWait.h>
#include <folly/coro/Sleep.h>
#include <folly/coro/Task.h>
#include <folly/futures/ThreadWheelTimekeeper.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <folly/portability/GTest.h>

namespace proxygen::coro::test {

TEST(CancellableBatonTest, Simple) {
  folly::ManualExecutor executor;
  detail::CancellableBaton baton;

  auto fut = co_withExecutor(&executor, baton.wait()).startInlineUnsafe();

  // no work enqueued; indefinitely stuck until baton is posted
  EXPECT_FALSE(fut.isReady());
  EXPECT_EQ(executor.step(), 0);
  EXPECT_EQ(executor.step(), 0);
  EXPECT_EQ(executor.step(), 0);
  EXPECT_EQ(executor.step(), 0);

  // signal should produce a value
  baton.signal();
  EXPECT_GT(executor.drain(), 0);

  EXPECT_TRUE(fut.hasValue());
  EXPECT_EQ(fut.value(), TimedBaton::Status::signalled);
}

TEST(CancellableBatonTest, AlreadySignalled) {
  folly::ManualExecutor executor;
  detail::CancellableBaton baton;
  baton.signal();

  auto fut = co_withExecutor(&executor, baton.wait()).startInlineUnsafe();

  // future should already be ready
  EXPECT_TRUE(fut.isReady());
  executor.drain();

  EXPECT_TRUE(fut.hasValue());
  EXPECT_EQ(fut.value(), TimedBaton::Status::signalled);

  baton.reset();
}

TEST(CancellableBatonTest, CancelViaAnotherThread) {
  folly::ManualExecutor executor;
  folly::CancellationSource cs{};
  folly::Optional<detail::CancellableBaton> cancellableBaton{std::in_place_t{}};

  auto t = std::thread([&]() { cs.requestCancellation(); });

  auto waitTask = folly::coro::co_withCancellation(
      cs.getToken(), cancellableBaton.value().wait());
  auto res = folly::coro::blockingWait(std::move(waitTask), &executor);
  EXPECT_EQ(res, TimedBaton::Status::cancelled);

  t.join();
}

TEST(CancellableBatonTest, Timeout) {
  folly::EventBase evb;
  detail::CancellableBaton baton;

  // timeeout baton after 250ms
  auto res = folly::coro::blockingWait(
      baton.timedWait(&evb, std::chrono::milliseconds(250)), &evb);

  EXPECT_EQ(res, TimedBaton::Status::timedout);
}

TEST(CancellableBatonTest, TimeoutDetach) {
  folly::EventBase evb;
  detail::DetachableCancellableBaton baton;

  // invoked timedWait 10ms => detach => verify future not fulfilled after 10ms
  auto fut = co_withExecutor(
                 &evb, baton.timedWait(&evb, std::chrono::milliseconds(10)))
                 .startInlineUnsafe();
  baton.detach();

  folly::EventBaseThreadTimekeeper tk{evb};
  folly::coro::blockingWait(folly::coro::sleep(std::chrono::milliseconds(20)));

  // verify ::detach cancelled scheduled timer
  EXPECT_FALSE(fut.isReady());

  baton.signal();
  auto res = folly::coro::blockingWait(std::move(fut), &evb);

  EXPECT_EQ(res, TimedBaton::Status::signalled);
}

TEST(CancellableBatonTest, PostBeforeTimeoutExpires) {
  folly::EventBase evb;
  detail::CancellableBaton baton;

  // post before 250ms timeout
  evb.runAfterDelay([&]() { baton.signal(); },
                    /*milliseconds=*/25);

  // timeeout baton after 250ms
  auto res = folly::coro::blockingWait(
      baton.timedWait(&evb, std::chrono::milliseconds(250)), &evb);

  EXPECT_EQ(res, TimedBaton::Status::signalled);

  // timer should be cancelled
  EXPECT_EQ(evb.timer().count(), 0);
}

} // namespace proxygen::coro::test
