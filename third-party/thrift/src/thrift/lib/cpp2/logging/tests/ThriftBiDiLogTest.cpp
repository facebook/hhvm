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

#include <thrift/lib/cpp2/logging/ThriftBiDiLog.h>

#include <thread>

#include <gtest/gtest.h>

namespace apache::thrift {

namespace {

class RecordingCounters : public IThriftServerCounters {
 public:
  std::string lastMethodName;
  int subscribes = 0;
  int sinkNexts = 0;
  int sinkCredits = 0;
  int streamNexts = 0;
  int sinkFirstChunkLatencies = 0;
  int streamFirstChunkLatencies = 0;
  int streamGenerationIntervals = 0;
  std::chrono::milliseconds lastGenerationInterval{0};
  int streamPauses = 0;
  int streamResumes = 0;
  detail::StreamPauseReason lastPauseReason{};
  std::chrono::milliseconds lastResumePauseDuration{0};
  detail::StreamPauseReason lastResumeReason{};
  int completes = 0;
  uint32_t lastSinkCredits = 0;
  detail::BiDiEndReason lastEndReason{};
  uint32_t lastSinkTotalChunks = 0;
  uint32_t lastStreamTotalChunks = 0;
  std::chrono::milliseconds lastDuration{0};

  void onBiDiSubscribe(std::string_view methodName) override {
    ++subscribes;
    lastMethodName = std::string(methodName);
  }
  void onBiDiSinkNext(std::string_view /*methodName*/) override { ++sinkNexts; }
  void onBiDiSinkCredit(
      std::string_view /*methodName*/, uint32_t credits) override {
    ++sinkCredits;
    lastSinkCredits = credits;
  }
  void onBiDiStreamNext(std::string_view /*methodName*/) override {
    ++streamNexts;
  }
  void onBiDiSinkFirstChunkLatency(
      std::string_view /*methodName*/,
      std::chrono::milliseconds /*latency*/) override {
    ++sinkFirstChunkLatencies;
  }
  void onBiDiStreamFirstChunkLatency(
      std::string_view /*methodName*/,
      std::chrono::milliseconds /*latency*/) override {
    ++streamFirstChunkLatencies;
  }
  void onBiDiStreamPause(
      std::string_view /*methodName*/,
      detail::StreamPauseReason reason) override {
    ++streamPauses;
    lastPauseReason = reason;
  }
  void onBiDiStreamResume(
      std::string_view /*methodName*/,
      detail::StreamPauseReason reason,
      std::chrono::milliseconds pauseDuration) override {
    ++streamResumes;
    lastResumeReason = reason;
    lastResumePauseDuration = pauseDuration;
  }
  void onBiDiStreamGenerationInterval(
      std::string_view /*methodName*/,
      std::chrono::milliseconds interval) override {
    ++streamGenerationIntervals;
    lastGenerationInterval = interval;
  }
  void onBiDiComplete(
      std::string_view /*methodName*/,
      detail::BiDiEndReason reason,
      uint32_t sinkTotalChunks,
      uint32_t streamTotalChunks,
      std::chrono::milliseconds duration) override {
    ++completes;
    lastEndReason = reason;
    lastSinkTotalChunks = sinkTotalChunks;
    lastStreamTotalChunks = streamTotalChunks;
    lastDuration = duration;
  }
};

} // namespace

class ThriftBiDiLogTest : public ::testing::Test {
 protected:
  RecordingCounters counters_;
};

TEST_F(ThriftBiDiLogTest, SubscribeDispatchesCounter) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});
  EXPECT_EQ(counters_.subscribes, 1);
}

TEST_F(ThriftBiDiLogTest, SinkNextTracksChunksAndFirstLatency) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiSinkNextEvent{});
  EXPECT_EQ(counters_.sinkNexts, 1);
  EXPECT_EQ(counters_.sinkFirstChunkLatencies, 1);

  log.log(detail::BiDiSinkNextEvent{});
  EXPECT_EQ(counters_.sinkNexts, 2);
  // First chunk latency should only be reported once
  EXPECT_EQ(counters_.sinkFirstChunkLatencies, 1);
}

TEST_F(ThriftBiDiLogTest, SinkCreditForwardsCredits) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiSinkCreditEvent{5});
  EXPECT_EQ(counters_.sinkCredits, 1);
  EXPECT_EQ(counters_.lastSinkCredits, 5);
}

TEST_F(ThriftBiDiLogTest, StreamNextTracksChunksAndFirstLatencyAndInterval) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamNextEvent{});
  EXPECT_EQ(counters_.streamNexts, 1);
  EXPECT_EQ(counters_.streamFirstChunkLatencies, 1);
  EXPECT_EQ(counters_.streamGenerationIntervals, 0);

  log.log(detail::BiDiStreamNextEvent{});
  EXPECT_EQ(counters_.streamNexts, 2);
  // First chunk latency should only be reported once
  EXPECT_EQ(counters_.streamFirstChunkLatencies, 1);
  // Generation interval reported starting from second chunk
  EXPECT_EQ(counters_.streamGenerationIntervals, 1);
}

TEST_F(ThriftBiDiLogTest, FinallyReportsCompletionWithTotals) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiSinkNextEvent{});
  log.log(detail::BiDiSinkNextEvent{});
  log.log(detail::BiDiStreamNextEvent{});
  log.log(detail::BiDiStreamNextEvent{});
  log.log(detail::BiDiStreamNextEvent{});

  log.log(detail::BiDiFinallyEvent{detail::BiDiEndReason::COMPLETE});

  EXPECT_EQ(counters_.completes, 1);
  EXPECT_EQ(counters_.lastEndReason, detail::BiDiEndReason::COMPLETE);
  EXPECT_EQ(counters_.lastSinkTotalChunks, 2);
  EXPECT_EQ(counters_.lastStreamTotalChunks, 3);
}

TEST_F(ThriftBiDiLogTest, DestructorCallsFinishIfNotAlreadyDone) {
  {
    ThriftBiDiLog log("myMethod", &counters_, nullptr);
    log.log(detail::BiDiSubscribeEvent{});
  }
  EXPECT_EQ(counters_.completes, 1);
  EXPECT_EQ(counters_.lastEndReason, detail::BiDiEndReason::ERROR);
}

TEST_F(ThriftBiDiLogTest, DoubleFinishIsIgnored) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});
  log.log(detail::BiDiFinallyEvent{detail::BiDiEndReason::COMPLETE});
  log.log(detail::BiDiFinallyEvent{detail::BiDiEndReason::ERROR});
  EXPECT_EQ(counters_.completes, 1);
  EXPECT_EQ(counters_.lastEndReason, detail::BiDiEndReason::COMPLETE);
}

TEST_F(ThriftBiDiLogTest, CancelEndReason) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});
  log.log(detail::BiDiFinallyEvent{detail::BiDiEndReason::CANCEL});
  EXPECT_EQ(counters_.lastEndReason, detail::BiDiEndReason::CANCEL);
}

TEST_F(ThriftBiDiLogTest, PauseDispatchesCounter) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  EXPECT_EQ(counters_.streamPauses, 1);
  EXPECT_EQ(counters_.lastPauseReason, detail::StreamPauseReason::NO_CREDITS);
}

TEST_F(ThriftBiDiLogTest, ExplicitPauseReason) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(
      detail::BiDiStreamPauseEvent{detail::StreamPauseReason::EXPLICIT_PAUSE});
  EXPECT_EQ(counters_.streamPauses, 1);
  EXPECT_EQ(
      counters_.lastPauseReason, detail::StreamPauseReason::EXPLICIT_PAUSE);
}

TEST_F(ThriftBiDiLogTest, DuplicatePauseIgnored) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  EXPECT_EQ(counters_.streamPauses, 1);
}

TEST_F(ThriftBiDiLogTest, ResumeDispatchesCounter) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::BiDiStreamResumeEvent{});
  EXPECT_EQ(counters_.streamResumes, 1);
  EXPECT_EQ(counters_.lastResumeReason, detail::StreamPauseReason::NO_CREDITS);
  EXPECT_GE(counters_.lastResumePauseDuration, std::chrono::milliseconds{0});
}

TEST_F(ThriftBiDiLogTest, ResumeWithoutPauseIsNoop) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamResumeEvent{});
  EXPECT_EQ(counters_.streamResumes, 0);
}

TEST_F(ThriftBiDiLogTest, MultiplePauseResumeCycles) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::BiDiStreamResumeEvent{});
  log.log(
      detail::BiDiStreamPauseEvent{detail::StreamPauseReason::EXPLICIT_PAUSE});
  log.log(detail::BiDiStreamResumeEvent{});

  EXPECT_EQ(counters_.streamPauses, 2);
  EXPECT_EQ(counters_.streamResumes, 2);
  EXPECT_EQ(
      counters_.lastResumeReason, detail::StreamPauseReason::EXPLICIT_PAUSE);
}

TEST_F(ThriftBiDiLogTest, FinishWhilePausedDoesNotResume) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::BiDiFinallyEvent{detail::BiDiEndReason::COMPLETE});

  // handleResume is NOT called during finish to avoid a data race (finish
  // runs on the IO thread while pause fields are executor-thread-only).
  EXPECT_EQ(counters_.streamResumes, 0);
}

TEST_F(ThriftBiDiLogTest, GenerationIntervalSubtractsPauseDuration) {
  ThriftBiDiLog log("myMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});

  // First chunk — no interval reported.
  log.log(detail::BiDiStreamNextEvent{});
  EXPECT_EQ(counters_.streamGenerationIntervals, 0);

  // Pause for ~50ms, then resume.
  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  std::this_thread::sleep_for(std::chrono::milliseconds(50)); // sleep override
  log.log(detail::BiDiStreamResumeEvent{});

  // Second chunk — interval should be reduced by the pause duration.
  log.log(detail::BiDiStreamNextEvent{});
  EXPECT_EQ(counters_.streamGenerationIntervals, 1);
  // The raw wall-clock interval is >= 50ms, but after subtracting ~50ms of
  // pause the adjusted interval should be small (< 50ms). Allow some slack
  // for scheduling jitter.
  EXPECT_LT(counters_.lastGenerationInterval, std::chrono::milliseconds{50});

  // Third chunk with no pause — pauseDurationSinceLastChunk_ should have
  // been reset, so the full interval is reported.
  log.log(detail::BiDiStreamNextEvent{});
  EXPECT_EQ(counters_.streamGenerationIntervals, 2);
}

TEST_F(ThriftBiDiLogTest, NullCountersDoNotCrash) {
  ThriftBiDiLog log("myMethod", nullptr, nullptr);
  log.log(detail::BiDiSubscribeEvent{});
  log.log(detail::BiDiSinkNextEvent{});
  log.log(detail::BiDiSinkCreditEvent{3});
  log.log(detail::BiDiStreamNextEvent{});
  log.log(detail::BiDiStreamCreditEvent{2});
  log.log(detail::BiDiStreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::BiDiStreamResumeEvent{});
  log.log(detail::BiDiFinallyEvent{detail::BiDiEndReason::COMPLETE});
}

TEST_F(ThriftBiDiLogTest, MethodNamePropagatedToCounters) {
  ThriftBiDiLog log("TestService.myBiDiMethod", &counters_, nullptr);
  log.log(detail::BiDiSubscribeEvent{});
  EXPECT_EQ(counters_.lastMethodName, "TestService.myBiDiMethod");
}

TEST_F(ThriftBiDiLogTest, FinishWithoutSubscribeSkipsMetrics) {
  {
    ThriftBiDiLog log("myMethod", &counters_, nullptr);
    // Destroy without calling log(BiDiSubscribeEvent{})
  }
  EXPECT_EQ(counters_.completes, 0);
}

} // namespace apache::thrift
