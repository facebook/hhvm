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

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FragmentCompletionTracker.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write::handler {
namespace {

// Test event types mirroring the rocket-pipeline shape: one message per event
// id, no shared discriminator.
struct TestBatchWriteCompleteEvent {
  transport::WriteCompletionStatus status;
  size_t frameCount;
  size_t bytes;
};

struct TestFrameWriteCompleteEvent {
  uint32_t streamId;
  transport::WriteCompletionStatus status;
};

// Event enum mirroring a rocket pipeline's EventId — one value per message.
enum class TestEventId : std::uint32_t {
  BatchWriteComplete,
  FrameWriteComplete,
  Count,
};

// Declares status first, streamId second — the same convention as the real
// RocketClientEventFactory. The tracker must call it in that order.
struct TestEventFactory {
  using EventId = TestEventId;
  using BatchWriteCompleteEventType = TestBatchWriteCompleteEvent;

  static std::pair<EventId, channel_pipeline::TypeErasedBox>
  makeFrameWriteComplete(
      transport::WriteCompletionStatus status, uint32_t streamId) noexcept {
    return {
        EventId::FrameWriteComplete,
        channel_pipeline::TypeErasedBox(
            TestFrameWriteCompleteEvent{
                .streamId = streamId,
                .status = status,
            })};
  }
};

static_assert(
    FrameWriteCompleteEventFactory<TestEventFactory>,
    "TestEventFactory must satisfy FrameWriteCompleteEventFactory concept");

// Minimal Context — captures fireEvent boxes so the test can inspect what the
// tracker fired upstream.
class CapturingContext {
 public:
  void fireEvent(
      TestEventId /*ev*/, channel_pipeline::TypeErasedBox box) noexcept {
    events_.push_back(std::move(box).take<TestFrameWriteCompleteEvent>());
  }

  const std::vector<TestFrameWriteCompleteEvent>& events() const noexcept {
    return events_;
  }

 private:
  std::vector<TestFrameWriteCompleteEvent> events_;
};

// Helper: build a BatchWriteComplete box (what the downstream batch tracker
// would fire).
channel_pipeline::TypeErasedBox batchWriteComplete(
    transport::WriteCompletionStatus status,
    size_t frameCount,
    size_t bytes) noexcept {
  return channel_pipeline::TypeErasedBox(
      TestBatchWriteCompleteEvent{
          .status = status,
          .frameCount = frameCount,
          .bytes = bytes,
      });
}

} // namespace

TEST(FragmentCompletionTrackerTest, FragmentedFrameFiresOnceOnLastFragment) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // One frame split into three fragments: only the last completes the frame.
  tracker.onFragment(7, /*isLastFragment=*/false);
  tracker.onFragment(7, /*isLastFragment=*/false);
  tracker.onFragment(7, /*isLastFragment=*/true);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 3, 0));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].streamId, 7u);
  EXPECT_EQ(ctx.events()[0].status, transport::WriteCompletionStatus::Success);
}

TEST(FragmentCompletionTrackerTest, UnfragmentedFramesEachFireOnce) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Two unfragmented frames batched together: each is its own completed frame.
  tracker.onFragment(1, /*isLastFragment=*/true);
  tracker.onFragment(2, /*isLastFragment=*/true);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 2, 0));

  ASSERT_EQ(ctx.events().size(), 2u);
  EXPECT_EQ(ctx.events()[0].streamId, 1u);
  EXPECT_EQ(ctx.events()[1].streamId, 2u);
}

TEST(FragmentCompletionTrackerTest, MixedFragmentsResolveToOriginalFrames) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Batch of 4 fragments = 2 original frames: stream 1 (split in two) and
  // stream 2 (split in two). Two FrameWriteCompletes, in stream order.
  tracker.onFragment(1, /*isLastFragment=*/false);
  tracker.onFragment(1, /*isLastFragment=*/true);
  tracker.onFragment(2, /*isLastFragment=*/false);
  tracker.onFragment(2, /*isLastFragment=*/true);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 4, 0));

  ASSERT_EQ(ctx.events().size(), 2u);
  EXPECT_EQ(ctx.events()[0].streamId, 1u);
  EXPECT_EQ(ctx.events()[1].streamId, 2u);
}

TEST(FragmentCompletionTrackerTest, BatchBoundaryPopsExactlyFrameCount) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Two in-flight batches; each BatchWriteComplete must pop only its own
  // frameCount fragments, so the second frame's completion waits for the
  // second event.
  tracker.onFragment(1, /*isLastFragment=*/true); // batch 1: 1 fragment
  tracker.onFragment(2, /*isLastFragment=*/false); // batch 2: 2 fragments
  tracker.onFragment(2, /*isLastFragment=*/true);

  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 1, 0));
  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].streamId, 1u);

  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 2, 0));
  ASSERT_EQ(ctx.events().size(), 2u);
  EXPECT_EQ(ctx.events()[1].streamId, 2u);
}

TEST(FragmentCompletionTrackerTest, ErrorStatusPropagatesToFrameEvent) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  tracker.onFragment(9, /*isLastFragment=*/true);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Error, 1, 42));

  ASSERT_EQ(ctx.events().size(), 1u);
  EXPECT_EQ(ctx.events()[0].streamId, 9u);
  EXPECT_EQ(ctx.events()[0].status, transport::WriteCompletionStatus::Error);
}

TEST(FragmentCompletionTrackerTest, InterleavedFragmentsCompleteInFifoOrder) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // SRPT can alternate streams, so fragments of two frames arrive interleaved.
  // Completion keys off each record's own isLastFragment in FIFO order: here
  // stream 1's last fragment precedes stream 2's, so stream 1 completes first.
  tracker.onFragment(1, /*isLastFragment=*/false);
  tracker.onFragment(2, /*isLastFragment=*/false);
  tracker.onFragment(1, /*isLastFragment=*/true);
  tracker.onFragment(2, /*isLastFragment=*/true);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 4, 0));

  ASSERT_EQ(ctx.events().size(), 2u);
  EXPECT_EQ(ctx.events()[0].streamId, 1u);
  EXPECT_EQ(ctx.events()[1].streamId, 2u);
}

TEST(FragmentCompletionTrackerTest, InterleavedFragmentsCompleteInWireOrder) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // Same interleaving, but stream 2's last fragment lands before stream 1's:
  // completions follow wire order, so stream 2 fires first.
  tracker.onFragment(1, /*isLastFragment=*/false);
  tracker.onFragment(2, /*isLastFragment=*/false);
  tracker.onFragment(2, /*isLastFragment=*/true);
  tracker.onFragment(1, /*isLastFragment=*/true);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Success, 4, 0));

  ASSERT_EQ(ctx.events().size(), 2u);
  EXPECT_EQ(ctx.events()[0].streamId, 2u);
  EXPECT_EQ(ctx.events()[1].streamId, 1u);
}

TEST(FragmentCompletionTrackerTest, ErrorBeforeLastFragmentFiresNoCompletion) {
  FragmentCompletionTrackerT<TestEventFactory> tracker;
  CapturingContext ctx;

  // A frame that errored mid-fragmentation: only its non-last fragments were
  // ever recorded (the handler skips onFragment for the fragment that hit
  // Result::Error). The Error BatchWriteComplete pops those records but fires
  // nothing, since none is marked isLastFragment — the partially-written frame
  // receives NO FrameWriteComplete. Documents current behavior.
  tracker.onFragment(5, /*isLastFragment=*/false);
  tracker.onFragment(5, /*isLastFragment=*/false);
  tracker.onEvent(
      ctx,
      TestEventId::BatchWriteComplete,
      batchWriteComplete(transport::WriteCompletionStatus::Error, 2, 0));

  EXPECT_TRUE(ctx.events().empty());
}

} // namespace apache::thrift::fast_thrift::frame::write::handler
