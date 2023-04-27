/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/AsyncTimeoutSet.h>

#include <boost/container/flat_map.hpp>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventUtil.h>
#include <folly/io/async/test/MockTimeoutManager.h>
#include <folly/io/async/test/UndelayedDestruction.h>
#include <folly/io/async/test/Util.h>
#include <folly/portability/GTest.h>
#include <vector>

using namespace proxygen;
using namespace testing;
using folly::AsyncTimeout;
using folly::test::MockTimeoutManager;
using std::chrono::milliseconds;

using StackTimeoutSet = folly::UndelayedDestruction<AsyncTimeoutSet>;

class MockTimeoutClock : public AsyncTimeoutSet::TimeoutClock {
 public:
  MockTimeoutClock() {
  }

  MOCK_METHOD(std::chrono::milliseconds, millisecondsSinceEpoch, ());
};

class TestTimeout : public AsyncTimeoutSet::Callback {
 public:
  template <typename... Args>
  explicit TestTimeout(Args&&... args) {
    addTimeout(std::forward<Args>(args)...);
    _scheduleNext();
  }
  TestTimeout() {
  }

  void addTimeout(AsyncTimeoutSet* set) {
    nextSets_.push_back(set);
  }

  template <typename... Args>
  void addTimeout(AsyncTimeoutSet* set, Args&&... args) {
    addTimeout(set);
    addTimeout(std::forward<Args>(args)...);
  }

  void timeoutExpired() noexcept override {
    timestamps.emplace_back(clock_->millisecondsSinceEpoch());
    _scheduleNext();
    if (fn) {
      fn();
    }
  }

  void _scheduleNext() {
    if (nextSets_.empty()) {
      return;
    }
    AsyncTimeoutSet* nextSet = nextSets_.front();
    nextSets_.pop_front();
    nextSet->scheduleTimeout(this);
  }

  std::deque<milliseconds> timestamps;
  std::function<void()> fn;

  static void setTimeoutClock(MockTimeoutClock& clock) {
    clock_ = &clock;
  }

 private:
  static MockTimeoutClock* clock_;
  std::deque<AsyncTimeoutSet*> nextSets_;
};

MockTimeoutClock* TestTimeout::clock_ = nullptr;

class TimeoutTest : public testing::Test {
 public:
  void SetUp() override {
    TestTimeout::setTimeoutClock(timeoutClock_);
    setClock(milliseconds(0));

    EXPECT_CALL(timeoutManager_, attachTimeoutManager(_, _))
        .WillRepeatedly(Return());

    EXPECT_CALL(timeoutManager_, scheduleTimeout(_, _))
        .WillRepeatedly(Invoke([this](AsyncTimeout* p, milliseconds t) {
          timeoutManager_.cancelTimeout(p);
          folly::event_ref_flags(p->getEvent()->getEvent()) |= EVLIST_TIMEOUT;
          timeouts_.emplace(t + timeoutClock_.millisecondsSinceEpoch(), p);
          return true;
        }));

    EXPECT_CALL(timeoutManager_, cancelTimeout(_))
        .WillRepeatedly(Invoke([this](AsyncTimeout* p) {
          for (auto it = timeouts_.begin(); it != timeouts_.end(); it++) {
            if (it->second == p) {
              timeouts_.erase(it);
              break;
            }
          }
        }));
  }

  void loop() {
    for (auto t = timeoutClock_.millisecondsSinceEpoch() + milliseconds(1);
         !timeouts_.empty();
         t++) {
      setClock(t);
    }
  }

  void setClock(milliseconds ms) {
    EXPECT_CALL(timeoutClock_, millisecondsSinceEpoch())
        .WillRepeatedly(Return(ms));

    while (!timeouts_.empty() &&
           timeoutClock_.millisecondsSinceEpoch() >= timeouts_.begin()->first) {
      AsyncTimeout* timeout = timeouts_.begin()->second;
      timeouts_.erase(timeouts_.begin());
      folly::event_ref_flags(timeout->getEvent()->getEvent()) &=
          ~EVLIST_TIMEOUT;
      timeout->timeoutExpired();
    }
  }

 protected:
  MockTimeoutManager timeoutManager_;
  MockTimeoutClock timeoutClock_;
  boost::container::flat_multimap<milliseconds, AsyncTimeout*> timeouts_;
};

/*
 * Test firing some simple timeouts that are fired once and never rescheduled
 */
TEST_F(TimeoutTest, FireOnce) {
  StackTimeoutSet ts10(
      &timeoutManager_, milliseconds(10), milliseconds(0), &timeoutClock_);
  StackTimeoutSet ts5(
      &timeoutManager_, milliseconds(5), milliseconds(0), &timeoutClock_);

  const AsyncTimeoutSet::Callback* nullCallback = nullptr;
  ASSERT_EQ(ts10.front(), nullCallback);
  ASSERT_EQ(ts5.front(), nullCallback);

  TestTimeout t1;
  TestTimeout t2;
  TestTimeout t3;

  ts5.scheduleTimeout(&t1); // fires at time=5

  // tick forward to time=2 and schedule another 5ms (@7ms) and a
  // 10ms (12ms) timeout

  setClock(milliseconds(2));

  ts5.scheduleTimeout(&t2);
  ts10.scheduleTimeout(&t3);

  ASSERT_EQ(ts10.front(), &t3);
  ASSERT_EQ(ts5.front(), &t1);

  setClock(milliseconds(5));
  ASSERT_EQ(ts5.front(), &t2);

  setClock(milliseconds(7));
  ASSERT_EQ(ts5.front(), nullCallback);

  ASSERT_EQ(t1.timestamps.size(), 1);
  ASSERT_EQ(t2.timestamps.size(), 1);

  setClock(milliseconds(12));

  ASSERT_EQ(t3.timestamps.size(), 1);

  ASSERT_EQ(ts10.front(), nullCallback);

  ASSERT_EQ(t1.timestamps[0], milliseconds(5));
  ASSERT_EQ(t2.timestamps[0], milliseconds(7));
  ASSERT_EQ(t3.timestamps[0], milliseconds(12));
}

/*
 * Test some timeouts that are scheduled on one timeout set, then moved to
 * another timeout set.
 */
TEST_F(TimeoutTest, SwitchTimeoutSet) {
  StackTimeoutSet ts10(
      &timeoutManager_, milliseconds(10), milliseconds(0), &timeoutClock_);
  StackTimeoutSet ts5(
      &timeoutManager_, milliseconds(5), milliseconds(0), &timeoutClock_);

  TestTimeout t1(&ts5, &ts10, &ts5);
  TestTimeout t2(&ts10, &ts10, &ts5);
  TestTimeout t3(&ts5, &ts5, &ts10, &ts5);

  ts5.scheduleTimeout(&t1);

  loop();

  ASSERT_EQ(t1.timestamps.size(), 3);
  ASSERT_EQ(t2.timestamps.size(), 3);
  ASSERT_EQ(t3.timestamps.size(), 4);

  ASSERT_EQ(t1.timestamps[0], milliseconds(5));
  ASSERT_EQ(t1.timestamps[1] - t1.timestamps[0], milliseconds(10));
  ASSERT_EQ(t1.timestamps[2] - t1.timestamps[1], milliseconds(5));

  ASSERT_EQ(t2.timestamps[0], milliseconds(10));
  ASSERT_EQ(t2.timestamps[1] - t2.timestamps[0], milliseconds(10));
  ASSERT_EQ(t2.timestamps[2] - t2.timestamps[1], milliseconds(5));

  ASSERT_EQ(t3.timestamps[0], milliseconds(5));
  ASSERT_EQ(t3.timestamps[1] - t3.timestamps[0], milliseconds(5));
  ASSERT_EQ(t3.timestamps[2] - t3.timestamps[1], milliseconds(10));
  ASSERT_EQ(t3.timestamps[3] - t3.timestamps[2], milliseconds(5));
  ASSERT_EQ(timeoutClock_.millisecondsSinceEpoch(), milliseconds(25));
}

/*
 * Test cancelling a timeout when it is scheduled to be fired right away.
 */
TEST_F(TimeoutTest, CancelTimeout) {
  StackTimeoutSet ts5(
      &timeoutManager_, milliseconds(5), milliseconds(0), &timeoutClock_);
  StackTimeoutSet ts10(
      &timeoutManager_, milliseconds(10), milliseconds(0), &timeoutClock_);
  StackTimeoutSet ts20(
      &timeoutManager_, milliseconds(20), milliseconds(0), &timeoutClock_);

  // Create several timeouts that will all fire in 5ms.
  TestTimeout t5_1(&ts5);
  TestTimeout t5_2(&ts5);
  TestTimeout t5_3(&ts5);
  TestTimeout t5_4(&ts5);
  TestTimeout t5_5(&ts5);

  // Also create a few timeouts to fire in 10ms
  TestTimeout t10_1(&ts10);
  TestTimeout t10_2(&ts10);
  TestTimeout t10_3(&ts10);

  TestTimeout t20_1(&ts20);
  TestTimeout t20_2(&ts20);

  // Have t5_1 cancel t5_2 and t5_4.
  //
  // Cancelling t5_2 will test cancelling a timeout that is at the head of the
  // list and ready to be fired.
  //
  // Cancelling t5_4 will test cancelling a timeout in the middle of the list
  t5_1.fn = [&] {
    t5_2.cancelTimeout();
    t5_4.cancelTimeout();
  };

  // Have t5_3 cancel t5_5.
  // This will test cancelling the last remaining timeout.
  //
  // Then have t5_3 reschedule itself.
  t5_3.fn = [&] {
    t5_5.cancelTimeout();
    // Reset our function so we won't continually reschedule ourself
    auto fn = std::move(t5_3.fn);
    ts5.scheduleTimeout(&t5_3);

    // Also test cancelling timeouts in another timeset that isn't ready to
    // fire yet.
    //
    // Cancel the middle timeout in ts10.
    t10_2.cancelTimeout();
    // Cancel both the timeouts in ts20.
    t20_1.cancelTimeout();
    t20_2.cancelTimeout();
  };

  loop();

  ASSERT_EQ(t5_1.timestamps.size(), 1);
  ASSERT_EQ(t5_1.timestamps[0], milliseconds(5));

  ASSERT_EQ(t5_3.timestamps.size(), 2);
  ASSERT_EQ(t5_3.timestamps[0], milliseconds(5));
  ASSERT_EQ(t5_3.timestamps[1] - t5_3.timestamps[0], milliseconds(5));

  ASSERT_EQ(t10_1.timestamps.size(), 1);
  ASSERT_EQ(t10_1.timestamps[0], milliseconds(10));

  ASSERT_EQ(t10_3.timestamps.size(), 1);
  ASSERT_EQ(t10_3.timestamps[0], milliseconds(10));

  // Cancelled timeouts
  ASSERT_EQ(t5_2.timestamps.size(), 0);
  ASSERT_EQ(t5_4.timestamps.size(), 0);
  ASSERT_EQ(t5_5.timestamps.size(), 0);
  ASSERT_EQ(t10_2.timestamps.size(), 0);
  ASSERT_EQ(t20_1.timestamps.size(), 0);
  ASSERT_EQ(t20_2.timestamps.size(), 0);
  ASSERT_EQ(timeoutClock_.millisecondsSinceEpoch(), milliseconds(10));
}

/*
 * Test destroying a AsyncTimeoutSet with timeouts outstanding
 */
TEST_F(TimeoutTest, DestroyTimeoutSet) {
  AsyncTimeoutSet::UniquePtr ts5(new AsyncTimeoutSet(
      &timeoutManager_, milliseconds(5), milliseconds(0), &timeoutClock_));
  AsyncTimeoutSet::UniquePtr ts10(new AsyncTimeoutSet(
      &timeoutManager_, milliseconds(10), milliseconds(0), &timeoutClock_));

  TestTimeout t5_1(ts5.get());
  TestTimeout t5_2(ts5.get());
  TestTimeout t5_3(ts5.get());

  TestTimeout t10_1(ts10.get());
  TestTimeout t10_2(ts10.get());

  // Have t5_1 destroy ts10
  t5_1.fn = [&] { ts10.reset(); };
  // Have t5_2 destroy ts5
  // Note that this will call destroy() on ts5 inside ts5's timeoutExpired()
  // method.
  t5_2.fn = [&] { ts5.reset(); };

  loop();

  ASSERT_EQ(t5_1.timestamps.size(), 1);
  ASSERT_EQ(t5_1.timestamps[0], milliseconds(5));
  ASSERT_EQ(t5_2.timestamps.size(), 1);
  ASSERT_EQ(t5_2.timestamps[0], milliseconds(5));

  ASSERT_EQ(t5_3.timestamps.size(), 0);
  ASSERT_EQ(t10_1.timestamps.size(), 0);
  ASSERT_EQ(t10_2.timestamps.size(), 0);
  ASSERT_EQ(timeoutClock_.millisecondsSinceEpoch(), milliseconds(5));
}

/*
 * Test the atMostEveryN parameter, to ensure that the timeout does not fire
 * too frequently.
 */
TEST_F(TimeoutTest, AtMostEveryN) {
  // Create a timeout set with a 25ms interval, to fire no more than once
  // every 6ms.
  milliseconds interval(25);
  milliseconds atMostEveryN(6);
  StackTimeoutSet ts25(
      &timeoutManager_, interval, atMostEveryN, &timeoutClock_);

  // Create 60 timeouts to be added to ts25 at 1ms intervals.
  uint32_t numTimeouts = 60;
  std::vector<TestTimeout> timeouts(numTimeouts);

  // Create a scheduler timeout to add the timeouts 1ms apart.
  // Note, these will start firing partway through scheduling them
  for (uint32_t index = 0; index < numTimeouts; index++) {
    setClock(milliseconds(index));
    timeouts[index].timeoutExpired();
    ts25.scheduleTimeout(&timeouts[index]);
  }

  loop();

  // We scheduled timeouts 1ms apart, when the AsyncTimeoutSet is only allowed
  // to wake up at most once every 3ms.  It will therefore wake up every 3ms
  // and fire groups of approximately 3 timeouts at a time.
  //
  // This is "approximately 3" since it may get slightly behind and fire 4 in
  // one interval, etc.  CHECK_TIMEOUT normally allows a few milliseconds of
  // tolerance.  We have to add the same into our checking algorithm here.
  for (uint32_t idx = 0; idx < numTimeouts; ++idx) {
    ASSERT_EQ(timeouts[idx].timestamps.size(), 2);

    auto scheduledTime = timeouts[idx].timestamps[0] + interval;
    auto firedTime = timeouts[idx].timestamps[1];
    // Assert that the timeout fired at roughly the right time.
    // CHECK_TIMEOUT() normally has a tolerance of 5ms.  Allow an additional
    // atMostEveryN.
    milliseconds tolerance = atMostEveryN;
    ASSERT_GE(firedTime, scheduledTime);
    ASSERT_LT(firedTime, scheduledTime + tolerance);

    // Assert that the difference between the previous timeout and now was
    // either very small (fired in the same event loop), or larger than
    // atMostEveryN.
    if (idx == 0) {
      // no previous value
      continue;
    }
    auto prev = timeouts[idx - 1].timestamps[1];

    auto delta = firedTime - prev;
    if (delta >= milliseconds(1)) {
      ASSERT_GE(delta, atMostEveryN);
    }
  }
  ASSERT_LE(timeoutClock_.millisecondsSinceEpoch(),
            milliseconds(numTimeouts) + interval + atMostEveryN);
}
