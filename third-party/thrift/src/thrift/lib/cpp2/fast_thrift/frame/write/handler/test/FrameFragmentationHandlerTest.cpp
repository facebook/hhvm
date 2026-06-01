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

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <map>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write::handler {
namespace {

// ============================================================================
// Standalone Mock Context
// ============================================================================

struct CapturedFrame {
  uint32_t streamId;
  FrameType frameType;
  uint16_t flags;
  size_t payloadSize;

  bool hasFollows() const {
    return (flags &
            ::apache::thrift::fast_thrift::frame::detail::kFollowsBit) != 0;
  }
};

class MockContext {
 public:
  explicit MockContext(folly::EventBase* evb) : evb_(evb) {}

  folly::EventBase* getEventBase() const { return evb_; }

  apache::thrift::fast_thrift::channel_pipeline::Result fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&& msg) {
    if (backpressureAt_ > 0 && written_.size() >= backpressureAt_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::
          Backpressure;
    }

    auto& frame = msg.get<OutboundFrame>();
    written_.push_back(
        CapturedFrame{
            .streamId = frame.streamId,
            .frameType = frame.frameType,
            .flags = frame.flags,
            .payloadSize = frame.payloadSize(),
        });
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void awaitWriteReady() { awaitWriteReadyCalled_ = true; }
  void cancelAwaitWriteReady() { awaitWriteReadyCalled_ = false; }
  void deactivate() {}

  void setBackpressureAt(size_t n) { backpressureAt_ = n; }
  const std::vector<CapturedFrame>& written() const { return written_; }
  bool awaitWriteReadyCalled() const { return awaitWriteReadyCalled_; }

 private:
  folly::EventBase* evb_;
  std::vector<CapturedFrame> written_;
  size_t backpressureAt_{0};
  bool awaitWriteReadyCalled_{false};
};

// ============================================================================
// Test Helpers
// ============================================================================

std::unique_ptr<folly::IOBuf> makePayload(size_t size) {
  auto buf = folly::IOBuf::create(size);
  buf->append(size);
  std::memset(buf->writableData(), 'X', size);
  return buf;
}

apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox makeFrame(
    uint32_t streamId,
    size_t payloadSize,
    FrameType type = FrameType::PAYLOAD) {
  OutboundFrame frame;
  frame.streamId = streamId;
  frame.frameType = type;
  frame.flags = 0;
  frame.payload = makePayload(payloadSize);
  return apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
      std::move(frame));
}

// ============================================================================
// Test Fixture
// ============================================================================

class FrameFragmentationHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evb_ = std::make_unique<folly::EventBase>();
    ctx_ = std::make_unique<MockContext>(evb_.get());
  }

  void TearDown() override {
    ctx_.reset();
    evb_.reset();
  }

  void runEventBaseLoop() { evb_->loopOnce(); }

  std::unique_ptr<folly::EventBase> evb_;
  std::unique_ptr<MockContext> ctx_;
};

// ============================================================================
// Single Stream Tests
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, SmallFrameBypassesFragmentation) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Frame fits in one fragment AND heap is empty → fast-path direct fireWrite.
  auto result = handler.onWrite(*ctx_, makeFrame(1, 32 * 1024));
  EXPECT_EQ(
      result, apache::thrift::fast_thrift::channel_pipeline::Result::Success);

  // Fast-path: sent immediately, nothing queued.
  EXPECT_EQ(handler.immediateQueueSize(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 0);

  ASSERT_EQ(ctx_->written().size(), 1);
  EXPECT_EQ(ctx_->written()[0].streamId, 1);
  EXPECT_EQ(ctx_->written()[0].payloadSize, 32 * 1024);
  EXPECT_FALSE(ctx_->written()[0].hasFollows());
}

TEST_F(FrameFragmentationHandlerTest, LargeFrameFragmented) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  auto result = handler.onWrite(*ctx_, makeFrame(1, 192 * 1024));
  EXPECT_EQ(
      result, apache::thrift::fast_thrift::channel_pipeline::Result::Success);

  EXPECT_EQ(handler.immediateQueueSize(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 1);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 3);

  EXPECT_TRUE(ctx_->written()[0].hasFollows());
  EXPECT_TRUE(ctx_->written()[1].hasFollows());
  EXPECT_FALSE(ctx_->written()[2].hasFollows());
}

// ============================================================================
// Multi-Stream Round-Robin Tests
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, MultiStreamRoundRobinInterleaving) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeFrame(1, 192 * 1024)); // 3 fragments
  (void)handler.onWrite(*ctx_, makeFrame(2, 128 * 1024)); // 2 fragments
  (void)handler.onWrite(*ctx_, makeFrame(3, 65 * 1024)); // 2 fragments

  EXPECT_EQ(handler.pendingStreamCount(), 3);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 7);

  std::map<uint32_t, std::vector<size_t>> fragmentsByStream;
  for (size_t i = 0; i < ctx_->written().size(); ++i) {
    fragmentsByStream[ctx_->written()[i].streamId].push_back(i);
  }

  EXPECT_EQ(fragmentsByStream[1].size(), 3);
  EXPECT_EQ(fragmentsByStream[2].size(), 2);
  EXPECT_EQ(fragmentsByStream[3].size(), 2);

  // Per-stream ordering preserved
  for (const auto& [streamId, indices] : fragmentsByStream) {
    for (size_t i = 1; i < indices.size(); ++i) {
      EXPECT_GT(indices[i], indices[i - 1])
          << "Stream " << streamId << " fragments out of order";
    }
  }
}

TEST_F(FrameFragmentationHandlerTest, SmallFramesSentBeforeLargeFragments) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeFrame(1, 192 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(2, 32 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(3, 128 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(4, 16 * 1024));

  EXPECT_EQ(handler.immediateQueueSize(), 2);
  EXPECT_EQ(handler.pendingStreamCount(), 2);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 7);

  // Small frames first
  EXPECT_EQ(ctx_->written()[0].streamId, 2);
  EXPECT_EQ(ctx_->written()[1].streamId, 4);
}

// ============================================================================
// Backpressure Tests
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, BackpressurePausesFlush) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(2, 128 * 1024));

  ctx_->setBackpressureAt(2);

  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 2);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(FrameFragmentationHandlerTest, BackpressureResumeCompletes) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Use 3 fragments so we can verify resume works
  (void)handler.onWrite(*ctx_, makeFrame(1, 192 * 1024)); // 3 fragments

  ctx_->setBackpressureAt(2); // Allow 2 fragments, backpressure on 3rd

  runEventBaseLoop();
  EXPECT_EQ(ctx_->written().size(), 2);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());

  // Clear backpressure and resume
  ctx_->setBackpressureAt(0);
  handler.onWriteReady(*ctx_);

  // After resume, stream should be fully drained
  EXPECT_EQ(handler.pendingStreamCount(), 0);
  // At least 2 fragments were written before backpressure,
  // and the stream is now complete (drained)
  EXPECT_GE(ctx_->written().size(), 2);
}

// ============================================================================
// Threshold Tests
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, MaxPendingBytesForceFlush) {
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = 256 * 1024,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  (void)handler.onWrite(*ctx_, makeFrame(2, 192 * 1024));
  EXPECT_GT(ctx_->written().size(), 0);
}

TEST_F(FrameFragmentationHandlerTest, MaxPendingFramesForceFlush) {
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = 10 * 1024 * 1024,
      .maxPendingFrames = 3,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // First: add a large frame that needs fragmentation to fill the heap.
  (void)handler.onWrite(*ctx_, makeFrame(1, 192 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 1);

  // Second large frame.
  (void)handler.onWrite(*ctx_, makeFrame(2, 192 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 2);

  // Third large frame — at limit, not exceeded.
  (void)handler.onWrite(*ctx_, makeFrame(3, 192 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  // Fourth large frame exceeds limit — should force flush.
  (void)handler.onWrite(*ctx_, makeFrame(4, 192 * 1024));
  EXPECT_GT(ctx_->written().size(), 0);
}

TEST_F(FrameFragmentationHandlerTest, BackpressureAtExactByteLimit) {
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = 128 * 1024,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Large frame goes to SrptHeap — at limit but not exceeded.
  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);
  EXPECT_EQ(handler.pendingBytes(), 128 * 1024);

  // Another large frame exceeds limit — should force flush.
  (void)handler.onWrite(*ctx_, makeFrame(2, 128 * 1024));
  EXPECT_GT(ctx_->written().size(), 0);
}

TEST_F(FrameFragmentationHandlerTest, BackpressureAtExactFrameLimit) {
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = 10 * 1024 * 1024,
      .maxPendingFrames = 2,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Large frames go to SrptHeap.
  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  // Second large frame — at exact frame limit, not exceeded.
  (void)handler.onWrite(*ctx_, makeFrame(2, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  // Third large frame exceeds limit — should force flush.
  (void)handler.onWrite(*ctx_, makeFrame(3, 128 * 1024));
  EXPECT_GT(ctx_->written().size(), 0);
}

TEST_F(FrameFragmentationHandlerTest, BackpressureOnImmediateQueueDrain) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // First: add a large frame to populate the heap.
  (void)handler.onWrite(*ctx_, makeFrame(10, 192 * 1024));
  EXPECT_EQ(handler.pendingStreamCount(), 1);

  // Now add medium frames (above minSizeToFragment but below maxFragmentSize).
  // Heap is non-empty, so they go to immediateQueue instead of fast-path.
  (void)handler.onWrite(*ctx_, makeFrame(1, 2 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(2, 2 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(3, 2 * 1024));

  EXPECT_EQ(handler.immediateQueueSize(), 3);

  // Set backpressure to trigger on second frame.
  ctx_->setBackpressureAt(1);

  runEventBaseLoop();

  // Only first frame should be written before backpressure.
  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(FrameFragmentationHandlerTest, BackpressureOnFirstFragment) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Add a large frame that will be fragmented
  (void)handler.onWrite(*ctx_, makeFrame(1, 192 * 1024)); // 3 fragments

  EXPECT_EQ(handler.pendingStreamCount(), 1);

  // Set backpressure to trigger on first fragment
  // backpressureAt(1) means backpressure after 1 frame written
  ctx_->setBackpressureAt(1);

  runEventBaseLoop();

  // First fragment should be written, then backpressure
  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_TRUE(ctx_->written()[0].hasFollows());
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(FrameFragmentationHandlerTest, BackpressureOnContinuationFragment) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Add a large frame that will be fragmented into 3 fragments
  (void)handler.onWrite(*ctx_, makeFrame(1, 192 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);

  // Set backpressure to trigger on second fragment (continuation)
  ctx_->setBackpressureAt(1);

  runEventBaseLoop();

  // First fragment written, backpressure on second
  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_TRUE(ctx_->written()[0].hasFollows());
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());

  // Clear backpressure and resume
  ctx_->setBackpressureAt(0);
  handler.onWriteReady(*ctx_);

  // Remaining fragments should now be written
  EXPECT_GE(ctx_->written().size(), 2);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, ExactFragmentSizeNotFragmented) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Exactly maxFragmentSize AND heap empty → fast-path direct fireWrite.
  (void)handler.onWrite(*ctx_, makeFrame(1, 64 * 1024));

  EXPECT_EQ(handler.immediateQueueSize(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 0);

  ASSERT_EQ(ctx_->written().size(), 1);
  EXPECT_FALSE(ctx_->written()[0].hasFollows());
}

TEST_F(FrameFragmentationHandlerTest, StreamCompletionRemovesFromMap) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeFrame(1, 65 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);

  runEventBaseLoop();

  EXPECT_EQ(handler.pendingStreamCount(), 0);
}

TEST_F(FrameFragmentationHandlerTest, UnevenFragmentSizes) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeFrame(1, 100 * 1024));

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 2);

  EXPECT_EQ(ctx_->written()[0].payloadSize, 64 * 1024);
  EXPECT_TRUE(ctx_->written()[0].hasFollows());

  EXPECT_EQ(ctx_->written()[1].payloadSize, 36 * 1024);
  EXPECT_FALSE(ctx_->written()[1].hasFollows());
}

TEST_F(FrameFragmentationHandlerTest, CancelClearsStreamFragments) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Add a large frame that will require fragmentation
  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));

  // Verify stream is pending
  EXPECT_EQ(handler.pendingStreamCount(), 1);
  EXPECT_TRUE(handler.hasPendingStream(1));

  // Send CANCEL for this stream
  OutboundFrame cancelFrame;
  cancelFrame.streamId = 1;
  cancelFrame.frameType = FrameType::CANCEL;
  cancelFrame.flags = 0;
  cancelFrame.payload = nullptr;
  (void)handler.onWrite(
      *ctx_,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(cancelFrame)));

  // Stream should be removed from pending
  EXPECT_EQ(handler.pendingStreamCount(), 0);
  EXPECT_FALSE(handler.hasPendingStream(1));

  // Run event loop - should only get the CANCEL frame, not fragments
  runEventBaseLoop();

  // Verify CANCEL was forwarded (it's in immediate queue)
  bool cancelSent = false;
  for (const auto& f : ctx_->written()) {
    if (f.frameType == FrameType::CANCEL && f.streamId == 1) {
      cancelSent = true;
    }
  }
  EXPECT_TRUE(cancelSent);
}

TEST_F(FrameFragmentationHandlerTest, ErrorClearsStreamFragments) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Add a large frame that will require fragmentation
  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));

  // Verify stream is pending
  EXPECT_EQ(handler.pendingStreamCount(), 1);
  EXPECT_TRUE(handler.hasPendingStream(1));

  // Send ERROR for this stream
  OutboundFrame errorFrame;
  errorFrame.streamId = 1;
  errorFrame.frameType = FrameType::ERROR;
  errorFrame.flags = 0;
  errorFrame.payload = nullptr;
  (void)handler.onWrite(
      *ctx_,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(errorFrame)));

  // Stream should be removed from pending
  EXPECT_EQ(handler.pendingStreamCount(), 0);
  EXPECT_FALSE(handler.hasPendingStream(1));

  runEventBaseLoop();

  // Verify ERROR was forwarded
  bool errorSent = false;
  for (const auto& f : ctx_->written()) {
    if (f.frameType == FrameType::ERROR && f.streamId == 1) {
      errorSent = true;
    }
  }
  EXPECT_TRUE(errorSent);
}

TEST_F(FrameFragmentationHandlerTest, CancelOnlyAffectsTargetStream) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Add large frames for two different streams
  (void)handler.onWrite(*ctx_, makeFrame(1, 128 * 1024));
  (void)handler.onWrite(*ctx_, makeFrame(2, 128 * 1024));

  // Verify both streams are pending
  EXPECT_EQ(handler.pendingStreamCount(), 2);
  EXPECT_TRUE(handler.hasPendingStream(1));
  EXPECT_TRUE(handler.hasPendingStream(2));

  // Send CANCEL only for stream 1
  OutboundFrame cancelFrame;
  cancelFrame.streamId = 1;
  cancelFrame.frameType = FrameType::CANCEL;
  cancelFrame.flags = 0;
  cancelFrame.payload = nullptr;
  (void)handler.onWrite(
      *ctx_,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(cancelFrame)));

  // Only stream 1 should be removed
  EXPECT_EQ(handler.pendingStreamCount(), 1);
  EXPECT_FALSE(handler.hasPendingStream(1));
  EXPECT_TRUE(handler.hasPendingStream(2));

  runEventBaseLoop();

  // Verify stream 2 fragments were still sent
  size_t stream2Fragments = 0;
  for (const auto& f : ctx_->written()) {
    if (f.streamId == 2 && f.frameType == FrameType::PAYLOAD) {
      stream2Fragments++;
    }
  }
  EXPECT_EQ(stream2Fragments, 2); // 128KB = 2 fragments of 64KB each
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write::handler
