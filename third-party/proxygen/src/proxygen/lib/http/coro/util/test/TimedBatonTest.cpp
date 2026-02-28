/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/TimedBaton.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GTest.h>

namespace proxygen::coro::test {

class TimedBatonTest : public testing::Test {
 public:
  folly::EventBase* getExecutor() {
    return &eventBase;
  }

  folly::EventBase eventBase;
  folly::CancellationSource cancelSource;
  TimedBaton timedBaton{&eventBase, std::chrono::milliseconds(0)};
};

CO_TEST_F_X(TimedBatonTest, TimedBatonTimeOutTest) {
  timedBaton.setTimeout(std::chrono::milliseconds(500));
  auto res = co_await timedBaton.wait();
  EXPECT_EQ(res, TimedBaton::Status::timedout);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::timedout);
  // Signalling a baton in this state doesn't change the state
  timedBaton.signal();
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::timedout);
}

CO_TEST_F_X(TimedBatonTest, TimedBatonChangeTimeOutTest) {
  // Start with 0 timeout, set timeout while waiting
  auto fut =
      co_withExecutor(&eventBase, timedBaton.wait()).start().via(&eventBase);
  co_await folly::coro::co_reschedule_on_current_executor;
  EXPECT_FALSE(timedBaton.isScheduled());
  timedBaton.setTimeout(std::chrono::milliseconds(500));
  EXPECT_TRUE(timedBaton.isScheduled());
  auto res = co_await std::move(fut);
  EXPECT_EQ(res, TimedBaton::Status::timedout);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::timedout);
  // Signalling a baton in this state doesn't change the state
  timedBaton.signal();
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::timedout);
}

CO_TEST_F_X(TimedBatonTest, TimedBatonNoTimeOutTest) {
  // When timeout is 0, there is no timeout
  auto fut = co_withExecutor(&eventBase,
                             folly::coro::co_withCancellation(
                                 cancelSource.getToken(), timedBaton.wait()))
                 .start()
                 .via(&eventBase);
  co_await folly::coro::co_reschedule_on_current_executor;
  EXPECT_FALSE(timedBaton.isScheduled());
  cancelSource.requestCancellation();
  auto res = co_await std::move(fut);
  EXPECT_EQ(res, TimedBaton::Status::cancelled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::cancelled);
}

CO_TEST_F_X(TimedBatonTest, TimedBatonDisableTimeOutTest) {
  // Start with a timeout, set to 0 while waiting
  timedBaton.setTimeout(std::chrono::milliseconds(500));
  auto fut = co_withExecutor(&eventBase,
                             folly::coro::co_withCancellation(
                                 cancelSource.getToken(), timedBaton.wait()))
                 .start()
                 .via(&eventBase);
  co_await folly::coro::co_reschedule_on_current_executor;
  timedBaton.setTimeout(std::chrono::milliseconds(0));
  EXPECT_FALSE(timedBaton.isScheduled());
  cancelSource.requestCancellation();
  auto res = co_await std::move(fut);
  EXPECT_EQ(res, TimedBaton::Status::cancelled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::cancelled);
}

// Signal from another task
CO_TEST_F_X(TimedBatonTest, TimedBatonSignalTest) {
  timedBaton.setTimeout(std::chrono::seconds(1));
  auto signalTask = [](TimedBaton* timedBaton) -> folly::coro::Task<void> {
    timedBaton->signal();
    co_return;
  };
  co_withExecutor(&eventBase, signalTask(&timedBaton)).start();
  auto res = co_await timedBaton.wait();
  EXPECT_EQ(res, TimedBaton::Status::signalled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::signalled);
}

CO_TEST_F_X(TimedBatonTest, TimedBatonCancelTest) {
  timedBaton.setTimeout(std::chrono::seconds(1));
  // Cancel directly
  auto fut = co_withExecutor(&eventBase,
                             folly::coro::co_withCancellation(
                                 cancelSource.getToken(), timedBaton.wait()))
                 .start()
                 .via(&eventBase);
  cancelSource.requestCancellation();
  auto res = co_await std::move(fut);
  EXPECT_EQ(res, TimedBaton::Status::cancelled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::cancelled);

  // Cancel via another task
  folly::CancellationSource cancelSource1;
  timedBaton.reset();
  auto fut2 = co_withExecutor(&eventBase,
                              folly::coro::co_withCancellation(
                                  cancelSource1.getToken(), timedBaton.wait()))
                  .start()
                  .via(&eventBase);

  auto cancelTask =
      [](folly::CancellationSource* cancelSource) -> folly::coro::Task<void> {
    cancelSource->requestCancellation();
    co_return;
  };

  co_withExecutor(&eventBase, cancelTask(&cancelSource1)).start();
  auto res2 = co_await std::move(fut2);
  EXPECT_EQ(res2, TimedBaton::Status::cancelled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::cancelled);
  // Signalling a baton in this state doesn't change the state
  timedBaton.signal();
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::cancelled);
}

CO_TEST_F_X(TimedBatonTest, EarlySignal) {
  timedBaton.setTimeout(std::chrono::milliseconds(10));
  timedBaton.signal();
  auto res = co_await timedBaton.wait();
  EXPECT_EQ(res, TimedBaton::Status::signalled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::signalled);
}

CO_TEST_F_X(TimedBatonTest, TimedBatonRequestCancellationOutsideEvb) {
  /**
   * The idea here is to invoke co_await timedBaton.wait() with
   * cancellationToken inside evb; and subsequently request cancellation of the
   * token via CancellationSource::requestCancellation in another thread
   * (otherEvb here). Callbacks attached to the cancellationToken (via
   * CancellationCallbacks) are invoked inline on the thread that requests
   * cancellation (otherEvb here) and can lead to race conditions if not
   * properly handled inside TimedBaton.
   */
  folly::ScopedEventBaseThread otherScopedEvb;
  folly::EventBase* otherEvb = otherScopedEvb.getEventBase();
  folly::CancellationSource cancellationSource;
  auto cancellationToken = cancellationSource.getToken();
  via(otherEvb).then(
      [otherEvb, cs = std::move(cancellationSource)](auto&&) mutable {
        // request cancellation after small 100ms delay to ensure
        // TimedBaton::wait is invoked first
        otherEvb->runAfterDelay(
            [cs = std::move(cs)]() { cs.requestCancellation(); }, 100);
      });

  // start .wait() inside evb with cancellationToken
  timedBaton.setTimeout(std::chrono::seconds(5));
  auto res = co_await folly::coro::co_withCancellation(cancellationToken,
                                                       timedBaton.wait());
  EXPECT_EQ(res, TimedBaton::Status::cancelled);
  EXPECT_EQ(timedBaton.getStatus(), TimedBaton::Status::cancelled);
}

CO_TEST_F_X(TimedBatonTest, TimedBatonDestroyTest) {
  auto timedBaton2 =
      std::make_unique<TimedBaton>(&eventBase, std::chrono::seconds(10));
  auto fut =
      co_withExecutor(&eventBase, timedBaton2->wait()).start().via(&eventBase);
  // wait() has to run before we delete the object
  co_await folly::coro::co_reschedule_on_current_executor;
  EXPECT_TRUE(timedBaton2->isScheduled());
  timedBaton2.reset();
  auto res = co_await std::move(fut);
  EXPECT_EQ(res, TimedBaton::Status::cancelled);
}

} // namespace proxygen::coro::test
