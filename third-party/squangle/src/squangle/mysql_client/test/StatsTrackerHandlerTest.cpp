/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// StatsTrackerHandler multiplexes an EventBase's single observer slot across
// several StatsTrackers sampled at different rates.  These tests drive
// loopSample() directly rather than through a real event loop, since the
// cadence is what's under test.
//

#include <gtest/gtest.h>
#include <memory>
#include <utility>
#include <vector>

#include "squangle/mysql_client/AsyncMysqlClient.h"

using namespace facebook::common::mysql_client;

namespace {

// Records the timings of every sample it is handed.
class RecordingTracker : public StatsTracker {
 public:
  void loopSample(int64_t busy_time, int64_t idle_time) override {
    samples.emplace_back(busy_time, idle_time);
  }

  std::vector<std::pair<int64_t, int64_t>> samples;
};

// Runs `count` loops through the handler, feeding loop N the timings (N, -N)
// so that samples can be traced back to the loop that produced them.  Skips
// loops the handler doesn't ask for, exactly as folly's EventBase does.
void runLoops(StatsTrackerHandler& handler, int64_t count) {
  uint32_t sampleCount = 0;
  for (int64_t loop = 1; loop <= count; ++loop) {
    if (++sampleCount >= handler.getSampleRate()) {
      sampleCount = 0;
      handler.loopSample(loop, -loop);
    }
  }
}

} // namespace

TEST(StatsTrackerHandlerTest, SingleTrackerSamplesAtItsOwnRate) {
  StatsTrackerHandler handler;
  auto tracker = std::make_shared<RecordingTracker>();
  handler.addTracker(tracker, 4);

  // The EventBase itself does the downsampling; the handler asks for one loop
  // in four and forwards every one it gets.
  EXPECT_EQ(handler.getSampleRate(), 4);

  runLoops(handler, 12);

  const std::vector<std::pair<int64_t, int64_t>> expected{
      {4, -4}, {8, -8}, {12, -12}};
  EXPECT_EQ(tracker->samples, expected);
}

TEST(StatsTrackerHandlerTest, EachTrackerKeepsItsOwnCadence) {
  StatsTrackerHandler handler;
  auto slow = std::make_shared<RecordingTracker>();
  auto fast = std::make_shared<RecordingTracker>();
  handler.addTracker(slow, 4);
  handler.addTracker(fast, 1);

  // gcd(4, 1) == 1: the fast tracker forces every-loop sampling, and the slow
  // tracker is redistributed back down to one in four.
  EXPECT_EQ(handler.getSampleRate(), 1);

  runLoops(handler, 8);

  const std::vector<std::pair<int64_t, int64_t>> expectedSlow{{4, -4}, {8, -8}};
  EXPECT_EQ(slow->samples, expectedSlow);

  const std::vector<std::pair<int64_t, int64_t>> expectedFast{
      {1, -1}, {2, -2}, {3, -3}, {4, -4}, {5, -5}, {6, -6}, {7, -7}, {8, -8}};
  EXPECT_EQ(fast->samples, expectedFast);
}

TEST(StatsTrackerHandlerTest, NonDivisibleRatesSampleExactly) {
  StatsTrackerHandler handler;
  auto six = std::make_shared<RecordingTracker>();
  auto sixteen = std::make_shared<RecordingTracker>();
  handler.addTracker(six, 6);
  handler.addTracker(sixteen, 16);

  // gcd rather than min, so both rates stay exact multiples of the handler's.
  EXPECT_EQ(handler.getSampleRate(), 2);

  runLoops(handler, 48);

  const std::vector<std::pair<int64_t, int64_t>> expectedSix{
      {6, -6},
      {12, -12},
      {18, -18},
      {24, -24},
      {30, -30},
      {36, -36},
      {42, -42},
      {48, -48}};
  EXPECT_EQ(six->samples, expectedSix);

  const std::vector<std::pair<int64_t, int64_t>> expectedSixteen{
      {16, -16}, {32, -32}, {48, -48}};
  EXPECT_EQ(sixteen->samples, expectedSixteen);
}

TEST(StatsTrackerHandlerTest, AddingATrackerDoesNotStarveExistingOnes) {
  StatsTrackerHandler handler;
  auto slow = std::make_shared<RecordingTracker>();
  handler.addTracker(slow, 4);

  runLoops(handler, 4);
  EXPECT_EQ(slow->samples.size(), 1);

  // Registering a faster tracker drops the handler's rate to 1.  The slow
  // tracker must still fire every four loops afterwards, not every four of the
  // handler's now-much-more-frequent calls.
  auto fast = std::make_shared<RecordingTracker>();
  handler.addTracker(fast, 1);

  runLoops(handler, 8);

  EXPECT_EQ(slow->samples.size(), 3);
  EXPECT_EQ(fast->samples.size(), 8);
}

TEST(StatsTrackerHandlerTest, NoTrackersAsksForEveryLoop) {
  StatsTrackerHandler handler;

  // Never zero: folly reads the rate every loop and treats 0 as "every loop"
  // anyway, but the handler shouldn't hand back a rate it didn't pick.
  EXPECT_EQ(handler.getSampleRate(), 1);
}
