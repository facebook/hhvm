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

#include <thrift/lib/cpp2/logging/ThriftStreamLog.h>

#include <gtest/gtest.h>

namespace apache::thrift {

namespace {

class RecordingCounters : public IThriftServerCounters {
 public:
  int subscribes = 0;
  int nexts = 0;
  int completes = 0;
  int pauses = 0;
  int resumes = 0;
  int credits = 0;
  int generationIntervals = 0;
  int sendDelays = 0;
  uint32_t lastCredits = 0;
  uint32_t lastCreditsAfter = 0;
  detail::StreamEndReason lastEndReason{};
  uint32_t lastTotalPauseEvents = 0;
  std::chrono::milliseconds lastTotalPauseDuration{0};

  void onStreamSubscribe(std::string_view /*methodName*/) override {
    ++subscribes;
  }
  void onStreamNext(std::string_view /*methodName*/) override { ++nexts; }
  void onStreamCredit(
      std::string_view /*methodName*/, uint32_t c, uint32_t after) override {
    ++credits;
    lastCredits = c;
    lastCreditsAfter = after;
  }
  void onStreamPause(
      std::string_view /*methodName*/,
      detail::StreamPauseReason /*reason*/) override {
    ++pauses;
  }
  void onStreamResume(
      std::string_view /*methodName*/,
      detail::StreamPauseReason /*reason*/,
      std::chrono::milliseconds /*pauseDuration*/) override {
    ++resumes;
  }
  void onStreamComplete(
      std::string_view /*methodName*/,
      detail::StreamEndReason reason,
      uint32_t totalPauseEvents,
      std::chrono::milliseconds totalPauseDuration) override {
    ++completes;
    lastEndReason = reason;
    lastTotalPauseEvents = totalPauseEvents;
    lastTotalPauseDuration = totalPauseDuration;
  }
  void onStreamChunkGenerationInterval(
      std::string_view /*methodName*/,
      std::chrono::milliseconds /*interval*/) override {
    ++generationIntervals;
  }
  void onStreamChunkSendDelay(
      std::string_view /*methodName*/,
      std::chrono::milliseconds /*delay*/) override {
    ++sendDelays;
  }
};

class RecordingLogging : public IThriftRequestLogging {
 public:
  int streamCompletes = 0;
  StreamSummary lastSummary{};

  void onStreamComplete(const StreamSummary& summary) override {
    ++streamCompletes;
    lastSummary = summary;
  }
};

} // namespace

class ThriftStreamLogTest : public ::testing::Test {
 protected:
  RecordingCounters counters_;
  RecordingLogging logging_;
};

TEST_F(ThriftStreamLogTest, SubscribeDispatchesToCounters) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  EXPECT_EQ(counters_.subscribes, 1);
}

TEST_F(ThriftStreamLogTest, NextDispatchesToCounters) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamNextEvent{});
  EXPECT_EQ(counters_.nexts, 1);
}

TEST_F(ThriftStreamLogTest, CreditAccumulation) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCreditEvent{10});
  EXPECT_EQ(counters_.credits, 1);
  EXPECT_EQ(counters_.lastCredits, 10);
  EXPECT_EQ(counters_.lastCreditsAfter, 10);

  // Generate some chunks to consume credits
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamNextEvent{});

  log.log(detail::StreamCreditEvent{5});
  EXPECT_EQ(counters_.lastCreditsAfter, 13); // 10 - 2 + 5
}

TEST_F(ThriftStreamLogTest, PauseAndResume) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  EXPECT_EQ(counters_.pauses, 1);

  log.log(detail::StreamResumeEvent{});
  EXPECT_EQ(counters_.resumes, 1);
}

TEST_F(ThriftStreamLogTest, DuplicatePauseIgnored) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::StreamPauseEvent{detail::StreamPauseReason::EXPLICIT_PAUSE});
  EXPECT_EQ(counters_.pauses, 1);
}

TEST_F(ThriftStreamLogTest, ResumeWithoutPauseIgnored) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamResumeEvent{});
  EXPECT_EQ(counters_.resumes, 0);
}

TEST_F(ThriftStreamLogTest, CompleteEvent) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});

  EXPECT_EQ(counters_.completes, 1);
  EXPECT_EQ(counters_.lastEndReason, detail::StreamEndReason::COMPLETE);
  EXPECT_EQ(logging_.streamCompletes, 1);
  EXPECT_EQ(logging_.lastSummary.chunksGenerated, 1);
}

TEST_F(ThriftStreamLogTest, ErrorEvent) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::ERROR});
  EXPECT_EQ(counters_.lastEndReason, detail::StreamEndReason::ERROR);
}

TEST_F(ThriftStreamLogTest, CancelEvent) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::CANCEL});
  EXPECT_EQ(counters_.lastEndReason, detail::StreamEndReason::CANCEL);
}

TEST_F(ThriftStreamLogTest, DestructorCleansUp) {
  {
    ThriftStreamLog log("myMethod", &counters_, &logging_);
    log.log(detail::StreamSubscribeEvent{});
    log.log(detail::StreamNextEvent{});
    // No explicit complete - destructor should finish with ERROR
  }
  EXPECT_EQ(counters_.completes, 1);
  EXPECT_EQ(counters_.lastEndReason, detail::StreamEndReason::ERROR);
  EXPECT_EQ(logging_.streamCompletes, 1);
}

TEST_F(ThriftStreamLogTest, IdempotentFinish) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::ERROR});
  EXPECT_EQ(counters_.completes, 1);
  EXPECT_EQ(logging_.streamCompletes, 1);
}

TEST_F(ThriftStreamLogTest, NullBackends) {
  // Should not crash with null backends
  ThriftStreamLog log("myMethod", nullptr, nullptr);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamCreditEvent{5});
  log.log(detail::StreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::StreamResumeEvent{});
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});
}

TEST_F(ThriftStreamLogTest, GenerationIntervalSkippedForFirstChunk) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamNextEvent{});
  EXPECT_EQ(counters_.generationIntervals, 0);

  log.log(detail::StreamNextEvent{});
  EXPECT_EQ(counters_.generationIntervals, 1);
}

TEST_F(ThriftStreamLogTest, CompleteSummaryAccumulation) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCreditEvent{10});

  for (int i = 0; i < 5; ++i) {
    log.log(detail::StreamNextEvent{});
  }

  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});

  EXPECT_EQ(logging_.lastSummary.chunksGenerated, 5);
  EXPECT_EQ(logging_.lastSummary.totalCreditsReceived, 10);
  EXPECT_EQ(logging_.lastSummary.endReason, detail::StreamEndReason::COMPLETE);
}

TEST_F(ThriftStreamLogTest, PayloadBytesAccumulation) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCreditEvent{10});

  log.log(detail::StreamNextEvent{100});
  log.log(detail::StreamNextEvent{500});
  log.log(detail::StreamNextEvent{200});

  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});

  EXPECT_EQ(logging_.lastSummary.totalBytes, 800);
  EXPECT_EQ(logging_.lastSummary.minChunkSize, 100);
  EXPECT_EQ(logging_.lastSummary.maxChunkSize, 500);
}

TEST_F(ThriftStreamLogTest, PayloadBytesZeroDoesNotAffectMinMax) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCreditEvent{10});

  // Zero-byte events (e.g., ordered headers) should not affect min/max
  log.log(detail::StreamNextEvent{0});
  log.log(detail::StreamNextEvent{300});
  log.log(detail::StreamNextEvent{0});

  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});

  EXPECT_EQ(logging_.lastSummary.totalBytes, 300);
  EXPECT_EQ(logging_.lastSummary.minChunkSize, 300);
  EXPECT_EQ(logging_.lastSummary.maxChunkSize, 300);
}

TEST_F(ThriftStreamLogTest, PayloadBytesAllZero) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamCreditEvent{10});

  log.log(detail::StreamNextEvent{0});
  log.log(detail::StreamNextEvent{0});

  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});

  EXPECT_EQ(logging_.lastSummary.totalBytes, 0);
  EXPECT_EQ(logging_.lastSummary.minChunkSize, 0);
  EXPECT_EQ(logging_.lastSummary.maxChunkSize, 0);
}

TEST_F(ThriftStreamLogTest, PauseResumeOnComplete) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  // Complete while paused should trigger resume cleanup
  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});
  EXPECT_EQ(counters_.resumes, 1);
  EXPECT_EQ(counters_.lastTotalPauseEvents, 1);
}

TEST_F(ThriftStreamLogTest, CreditTriggersResume) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});
  log.log(detail::StreamPauseEvent{detail::StreamPauseReason::NO_CREDITS});
  log.log(detail::StreamCreditEvent{5});
  EXPECT_EQ(counters_.resumes, 1);
}

TEST_F(ThriftStreamLogTest, SendDelaySamplingFirstChunk) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});

  // Chunk 1 satisfies chunkCounter_ % 10 == 1, so it is sampled
  log.log(detail::StreamNextEvent{});
  EXPECT_EQ(counters_.sendDelays, 0);

  // Matching sent event triggers the delay callback
  log.log(detail::StreamNextSentEvent{});
  EXPECT_EQ(counters_.sendDelays, 1);
}

TEST_F(ThriftStreamLogTest, SendDelaySamplingNonSampledChunks) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});

  // Generate chunk 1 (sampled) and consume its sample
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamNextSentEvent{});
  EXPECT_EQ(counters_.sendDelays, 1);

  // Chunks 2-10 are not sampled (chunkCounter_ % 10 != 1)
  for (int i = 2; i <= 10; ++i) {
    log.log(detail::StreamNextEvent{});
    log.log(detail::StreamNextSentEvent{});
  }
  EXPECT_EQ(counters_.sendDelays, 1);

  // Chunk 11 satisfies chunkCounter_ % 10 == 1, sampled again
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamNextSentEvent{});
  EXPECT_EQ(counters_.sendDelays, 2);
}

TEST_F(ThriftStreamLogTest, ChunksSentInSummary) {
  ThriftStreamLog log("myMethod", &counters_, &logging_);
  log.log(detail::StreamSubscribeEvent{});

  // Generate 3 chunks, send 2 (simulating one still in-flight)
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamNextSentEvent{});
  log.log(detail::StreamNextEvent{});
  log.log(detail::StreamNextSentEvent{});
  log.log(detail::StreamNextEvent{});

  log.log(detail::StreamCompleteEvent{detail::StreamEndReason::COMPLETE});

  EXPECT_EQ(logging_.lastSummary.chunksGenerated, 3);
  EXPECT_EQ(logging_.lastSummary.chunksSent, 2);
}

} // namespace apache::thrift
