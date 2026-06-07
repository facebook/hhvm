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

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write::handler {
namespace {

// Test event type mirroring the rocket-pipeline event shape: a nested Kind
// enum + status/frameCount/bytes payload.
struct TestEvent {
  enum class Kind : uint8_t {
    BatchWriteComplete,
    RocketWriteComplete,
  };

  Kind kind;
  transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

// Single-value event enum mirroring a rocket pipeline's EventId.
enum class TestEventId : std::uint32_t {
  WriteComplete,
  Count,
};

struct TestEventFactory {
  using EventType = TestEvent;
  using EventId = TestEventId;

  static std::pair<EventId, channel_pipeline::TypeErasedBox> make(
      transport::WriteCompletionStatus status, size_t bytes) noexcept {
    return {
        EventId::WriteComplete,
        channel_pipeline::TypeErasedBox(
            TestEvent{
                .kind = TestEvent::Kind::BatchWriteComplete,
                .status = status,
                .frameCount = 0,
                .bytes = bytes,
            })};
  }

  static std::pair<EventId, channel_pipeline::TypeErasedBox>
  makeRocketWriteComplete(
      transport::WriteCompletionStatus status,
      size_t frameCount,
      size_t bytes) noexcept {
    return {
        EventId::WriteComplete,
        channel_pipeline::TypeErasedBox(
            TestEvent{
                .kind = TestEvent::Kind::RocketWriteComplete,
                .status = status,
                .frameCount = frameCount,
                .bytes = bytes,
            })};
  }
};

// Minimal Context — captures fireEvent boxes so the test can inspect what
// the tracker fired upstream.
class CapturingContext {
 public:
  void fireEvent(
      TestEventId /*ev*/, channel_pipeline::TypeErasedBox box) noexcept {
    events_.push_back(std::move(box).take<TestEvent>());
  }

  const std::vector<TestEvent>& events() const noexcept { return events_; }

 private:
  std::vector<TestEvent> events_;
};

// Helper: build a BatchWriteComplete box (what transport would fire).
channel_pipeline::TypeErasedBox batchWriteComplete(
    transport::WriteCompletionStatus status, size_t bytes) noexcept {
  return TestEventFactory::make(status, bytes).second;
}

} // namespace

TEST(WriteCompletionTrackerTest, SingleBatchFiresOneEnrichedEvent) {
  WriteCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  tracker.onWrite();
  tracker.onWrite();
  tracker.onWrite();
  tracker.onFlush();
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].kind, TestEvent::Kind::RocketWriteComplete);
  EXPECT_EQ(ctx.events()[0].status, transport::WriteCompletionStatus::Success);
  EXPECT_EQ(ctx.events()[0].frameCount, 3u);
  EXPECT_EQ(ctx.events()[0].bytes, 0u);
}

TEST(WriteCompletionTrackerTest, MultipleInFlightBatchesPreserveFifoOrder) {
  WriteCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Batch 1: 2 frames.
  tracker.onWrite();
  tracker.onWrite();
  tracker.onFlush();

  // Batch 2: 5 frames.
  for (int i = 0; i < 5; ++i) {
    tracker.onWrite();
  }
  tracker.onFlush();

  // Batch 3: 1 frame.
  tracker.onWrite();
  tracker.onFlush();

  // writeSuccess events arrive in FIFO order (AsyncSocket guarantee).
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));

  ASSERT_EQ(ctx.events().size(), 3u);
  EXPECT_EQ(ctx.events()[0].frameCount, 2u);
  EXPECT_EQ(ctx.events()[1].frameCount, 5u);
  EXPECT_EQ(ctx.events()[2].frameCount, 1u);
}

TEST(WriteCompletionTrackerTest, ErrorStatusAndBytesPropagateToEvent) {
  WriteCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  tracker.onWrite();
  tracker.onWrite();
  tracker.onFlush();
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Error, 137));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].kind, TestEvent::Kind::RocketWriteComplete);
  EXPECT_EQ(ctx.events()[0].status, transport::WriteCompletionStatus::Error);
  EXPECT_EQ(ctx.events()[0].frameCount, 2u);
  EXPECT_EQ(ctx.events()[0].bytes, 137u);
}

TEST(WriteCompletionTrackerTest, EmptyBatchOnFlushIsIgnored) {
  WriteCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Flush with no preceding onWrite — tracker should not push a 0-count
  // batch (otherwise the next writeSuccess would fire a meaningless event).
  tracker.onFlush();

  // Subsequent real batch is the only thing in the FIFO.
  tracker.onWrite();
  tracker.onFlush();
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].frameCount, 1u);
}

TEST(WriteCompletionTrackerTest, WriteCompleteWithEmptyFifoIsNoop) {
  WriteCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Defensive: writeSuccess arriving without a corresponding flush (shouldn't
  // happen in practice) is dropped rather than UB.
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));

  EXPECT_TRUE(ctx.events().empty());
}

TEST(WriteCompletionTrackerTest, RocketWriteCompleteReFireIgnored) {
  WriteCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  tracker.onWrite();
  tracker.onFlush();

  // Tracker should ignore its own RocketWriteComplete kind — otherwise
  // a fireEvent from one tracker handled by another tracker in the same
  // pipeline would cascade or pop the wrong batch.
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      TestEventFactory::makeRocketWriteComplete(
          transport::WriteCompletionStatus::Success, 999, 0)
          .second);
  EXPECT_TRUE(ctx.events().empty());

  // The real BatchWriteComplete still drains the FIFO correctly.
  tracker.onEvent(
      ctx,
      TestEventId::WriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 0));
  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].frameCount, 1u);
}

} // namespace apache::thrift::fast_thrift::frame::write::handler
