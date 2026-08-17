/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Covers AsyncMysqlClient::addStatsTracker against a real client and its real
// IO thread.  StatsTrackerHandlerTest checks the sampling arithmetic in
// isolation; this checks that a tracker handed to the client actually reaches
// the EventBase observer and gets driven by it.
//
// No database is involved -- these tests only need the client's event loop to
// turn over.
//

#include <gtest/gtest.h>
#include <atomic>
#include <cstdint>
#include <memory>

#include "squangle/mysql_client/AsyncMysqlClient.h"

using namespace facebook::common::mysql_client;

namespace {

class CountingTracker : public StatsTracker {
 public:
  void loopSample(int64_t /* busy_time */, int64_t /* idle_time */) override {
    ++samples;
  }

  std::atomic<uint32_t> samples{0};
};

// Wakes the IO thread repeatedly so its event loop runs enough iterations to
// produce samples.  Each hop waits, so the loop is quiescent on return.
void spinIoThread(AsyncMysqlClient& client, int iterations) {
  for (int i = 0; i < iterations; ++i) {
    client.getEventBase()->runInEventBaseThreadAndWait([]() {});
  }
}

constexpr int kIterations = 200;

} // namespace

TEST(AddStatsTrackerTest, RegisteredTrackerReceivesSamples) {
  AsyncMysqlClient client;

  auto tracker = std::make_shared<CountingTracker>();
  client.addStatsTracker(tracker, 1);

  spinIoThread(client, kIterations);

  EXPECT_GT(tracker->samples.load(), 0u);
}

TEST(AddStatsTrackerTest, RegistersWhenCalledFromTheIoThread) {
  AsyncMysqlClient client;

  auto tracker = std::make_shared<CountingTracker>();
  // folly's plain runInEventBaseThreadAndWait declines to run its callback at
  // all when already on the IO thread, which would drop the tracker silently.
  client.getEventBase()->runInEventBaseThreadAndWait(
      [&]() { client.addStatsTracker(tracker, 1); });

  spinIoThread(client, kIterations);

  EXPECT_GT(tracker->samples.load(), 0u);
}

TEST(AddStatsTrackerTest, TrackersAreDrivenAtTheirOwnRates) {
  AsyncMysqlClient client;

  // Unrelated to PerfCounters::kSampleRate -- just a rate of this test's own
  // choosing, to check that a tracker gets the cadence it asked for.
  constexpr uint32_t kSlowRate = 16;

  auto fast = std::make_shared<CountingTracker>();
  auto slow = std::make_shared<CountingTracker>();
  client.addStatsTracker(fast, 1);
  client.addStatsTracker(slow, kSlowRate);

  spinIoThread(client, kIterations);

  // Read the slow one first: if the loop somehow turns over again between
  // these two reads, it can only help the inequality below.
  const auto slowSamples = slow->samples.load();
  const auto fastSamples = fast->samples.load();

  EXPECT_GT(slowSamples, 0u);
  EXPECT_GE(fastSamples, slowSamples * kSlowRate);
}

TEST(AddStatsTrackerTest, PlainClientKeepsTheHistoricalSampleRate) {
  AsyncMysqlClient client;

  auto* observer = client.getEventBase()->getObserver().get();
  ASSERT_NE(observer, nullptr);

  // A client nobody has added a tracker to must still sample at the cadence
  // squangle has always used.  Fanning out to multiple trackers must not cost
  // every client an observer call per loop.
  EXPECT_EQ(
      observer->getSampleRate(), AsyncMysqlClient::PerfCounters::kSampleRate);

  // Only once a faster tracker is registered does the EventBase step up.
  client.addStatsTracker(std::make_shared<CountingTracker>(), 1);
  EXPECT_EQ(observer->getSampleRate(), 1);
}
