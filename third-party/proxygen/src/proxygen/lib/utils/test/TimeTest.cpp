/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/Time.h>
#include <proxygen/lib/utils/test/MockTime.h>

#include <folly/String.h>
#include <folly/portability/GTest.h>

using namespace proxygen;

TEST(TimeTest, GetDateTimeStr) {
  ASSERT_FALSE(getDateTimeStr(getCurrentTime()).empty());

  SystemClock::time_point sys_tp{}; // epoch timepoint
  SteadyClock::time_point tp =
      SteadyClock::now() + std::chrono::duration_cast<SteadyClock::duration>(
                               sys_tp - SystemClock::now());
  std::string time;
  std::string timeZone;
  folly::split(' ', getDateTimeStr(tp), time, timezone);
  // getDateTimeStr return value includes the local timezone:
  // e.g 1970-01-01T00:00:00 +0000
  // Do not compare the timezone
  ASSERT_EQ("1970-01-01T00:00:00", time);
}

TEST(StopWatchTest, StartStopReset) {
  auto mockTime = std::make_shared<MockTimeUtil>();
  StopWatch<std::chrono::microseconds> stopWatch(mockTime);

  stopWatch.start();
  mockTime->advance(std::chrono::milliseconds(1));
  stopWatch.stop();

  EXPECT_EQ(stopWatch.getElapsedTime().count(),
            std::chrono::microseconds(1000).count());

  stopWatch.reset();
  EXPECT_EQ(stopWatch.getElapsedTime().count(),
            std::chrono::microseconds(0).count());
}

TEST(StopWatchTest, StartTwiceReset) {
  auto mockTime = std::make_shared<MockTimeUtil>();
  StopWatch<std::chrono::microseconds> stopWatch(mockTime);

  stopWatch.start();
  mockTime->advance(std::chrono::milliseconds(1));
  stopWatch.start();
  stopWatch.stop();

  EXPECT_EQ(stopWatch.getElapsedTime().count(),
            std::chrono::microseconds(0).count());
}

TEST(StopWatchTest, ContinueWithoutReset) {
  auto mockTime = std::make_shared<MockTimeUtil>();
  StopWatch<std::chrono::microseconds> stopWatch(mockTime);

  stopWatch.start();
  mockTime->advance(std::chrono::milliseconds(1));
  stopWatch.stop();

  mockTime->advance(std::chrono::milliseconds(1));

  stopWatch.start();
  mockTime->advance(std::chrono::milliseconds(1));
  stopWatch.stop();

  EXPECT_EQ(stopWatch.getElapsedTime().count(),
            std::chrono::microseconds(2000).count());
}

TEST(StopWatchTest, StopWatchTimedScope) {
  auto mockTime = std::make_shared<MockTimeUtil>();
  StopWatch<std::chrono::microseconds> stopWatch(mockTime);

  {
    auto timedScope = stopWatch.createTimedScope();
    mockTime->advance(std::chrono::milliseconds(1));
  }

  EXPECT_EQ(stopWatch.getElapsedTime().count(),
            std::chrono::microseconds(1000).count());
}
