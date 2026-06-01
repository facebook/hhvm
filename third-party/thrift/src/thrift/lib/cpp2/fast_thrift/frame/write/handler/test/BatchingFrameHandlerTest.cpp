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

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <cstring>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write::handler {
namespace {

// ============================================================================
// Test Utilities
// ============================================================================

std::unique_ptr<folly::IOBuf> makePayload(size_t size, char fillChar = 'X') {
  auto buf = folly::IOBuf::create(size);
  buf->append(size);
  std::memset(buf->writableData(), fillChar, size);
  return buf;
}

apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox wrapFrame(
    std::unique_ptr<folly::IOBuf> buf) {
  return apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
      std::move(buf));
}

// ============================================================================
// Mock Context
// ============================================================================

class MockContext {
 public:
  explicit MockContext(folly::EventBase* evb) : evb_(evb) {}

  folly::EventBase* eventBase() const { return evb_; }

  apache::thrift::fast_thrift::channel_pipeline::Result fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&& msg) {
    if (writtenBatches_.size() >= backpressureAt_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::
          Backpressure;
    }

    auto batch = msg.take<std::unique_ptr<folly::IOBuf>>();
    if (batch) {
      writtenBatches_.push_back(std::move(batch));
    }
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void awaitWriteReady() { awaitWriteReadyCalled_ = true; }
  void cancelAwaitWriteReady() { awaitWriteReadyCalled_ = false; }
  void deactivate() {}

  void setBackpressureAt(size_t n) { backpressureAt_ = n; }

  const std::vector<std::unique_ptr<folly::IOBuf>>& writtenBatches() const {
    return writtenBatches_;
  }

  size_t totalBytesWritten() const {
    size_t total = 0;
    for (const auto& batch : writtenBatches_) {
      if (batch) {
        total += batch->computeChainDataLength();
      }
    }
    return total;
  }

  bool awaitWriteReadyCalled() const { return awaitWriteReadyCalled_; }

 private:
  folly::EventBase* evb_;
  std::vector<std::unique_ptr<folly::IOBuf>> writtenBatches_;
  size_t backpressureAt_{SIZE_MAX}; // Default: never backpressure
  bool awaitWriteReadyCalled_{false};
};

// ============================================================================
// Test Fixture
// ============================================================================

class BatchingFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evb_ = std::make_unique<folly::EventBase>();
    ctx_ = std::make_unique<MockContext>(evb_.get());
  }

  void runEventBaseLoop() { evb_->loopOnce(EVLOOP_NONBLOCK); }

  std::unique_ptr<folly::EventBase> evb_;
  std::unique_ptr<MockContext> ctx_;
};

// ============================================================================
// Basic Tests
// ============================================================================

TEST_F(BatchingFrameHandlerTest, SingleFrameFlushOnLoopTick) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Send single small frame
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));

  // Should be scheduled but not flushed yet
  EXPECT_TRUE(handler.isScheduled());
  EXPECT_TRUE(handler.hasPendingData());
  EXPECT_EQ(handler.pendingBytes(), 100);
  EXPECT_EQ(handler.pendingFrames(), 1);
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // Run event loop to trigger flush
  runEventBaseLoop();

  // Should have flushed
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_FALSE(handler.hasPendingData());
  EXPECT_EQ(handler.pendingBytes(), 0);
  EXPECT_EQ(handler.pendingFrames(), 0);
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(BatchingFrameHandlerTest, MultipleFramesBatched) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Send multiple frames
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100, 'A')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200, 'B')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(300, 'C')));

  // Should be scheduled with all frames accumulated
  EXPECT_TRUE(handler.isScheduled());
  EXPECT_EQ(handler.pendingBytes(), 600);
  EXPECT_EQ(handler.pendingFrames(), 3);
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // Run event loop to trigger flush
  runEventBaseLoop();

  // Should have flushed all in one batch
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 600);

  // Verify the batch is a chain
  const auto& batch = ctx_->writtenBatches()[0];
  EXPECT_TRUE(batch->isChained());
}

TEST_F(BatchingFrameHandlerTest, FlushOnByteThreshold) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 500, // Small threshold
      .maxPendingFrames = 100,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Send frames that exceed byte threshold
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));

  // Not yet flushed
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // This should trigger flush (200 + 200 + 200 = 600 >= 500)
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));

  // Should have flushed immediately
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 600);
  EXPECT_FALSE(handler.hasPendingData());
}

TEST_F(BatchingFrameHandlerTest, FlushOnFrameThreshold) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 1024 * 1024, // Large byte threshold
      .maxPendingFrames = 3, // Small frame threshold
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Send frames up to threshold
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));

  // Not yet flushed
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // This should trigger flush (3 frames >= threshold)
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));

  // Should have flushed immediately
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 30);
  EXPECT_FALSE(handler.hasPendingData());
}

TEST_F(BatchingFrameHandlerTest, EmptyFrameIgnored) {
  BatchingHandlerConfig config{};
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Send nullptr frame
  std::unique_ptr<folly::IOBuf> nullFrame;
  (void)handler.onWrite(
      *ctx_,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          std::move(nullFrame)));

  // Should not have any pending data
  EXPECT_FALSE(handler.hasPendingData());
  EXPECT_EQ(handler.pendingFrames(), 0);
}

// ============================================================================
// Backpressure Tests
// ============================================================================

TEST_F(BatchingFrameHandlerTest, BackpressureHandled) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 2,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Set backpressure on first write
  ctx_->setBackpressureAt(0);

  // Send frames that trigger flush
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(50)));
  auto result = handler.onWrite(*ctx_, wrapFrame(makePayload(50)));

  // Should have returned backpressure
  EXPECT_EQ(
      result,
      apache::thrift::fast_thrift::channel_pipeline::Result::Backpressure);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(BatchingFrameHandlerTest, WriteReadyResumesFlushing) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Accumulate some data
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_TRUE(handler.hasPendingData());

  // Simulate writeReady callback
  handler.onWriteReady(*ctx_);

  // Should have flushed
  EXPECT_FALSE(handler.hasPendingData());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
}

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST_F(BatchingFrameHandlerTest, DeactivateCleansPendingData) {
  BatchingHandlerConfig config{};
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Accumulate data
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_TRUE(handler.hasPendingData());
  EXPECT_TRUE(handler.isScheduled());

  // Deactivate
  handler.onPipelineDeactivated(*ctx_);

  // Should have cleared everything
  EXPECT_FALSE(handler.hasPendingData());
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_EQ(handler.pendingBytes(), 0);
  EXPECT_EQ(handler.pendingFrames(), 0);
}

TEST_F(BatchingFrameHandlerTest, HandlerRemovedCleansPendingData) {
  BatchingHandlerConfig config{};
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Accumulate data
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_TRUE(handler.hasPendingData());

  // Remove handler
  handler.handlerRemoved(*ctx_);

  // Should have cleared everything
  EXPECT_FALSE(handler.hasPendingData());
  EXPECT_FALSE(handler.isScheduled());
}

// ============================================================================
// Chain Integrity Tests
// ============================================================================

TEST_F(BatchingFrameHandlerTest, ChainedDataIntegrity) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Send frames with distinct content
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10, 'A')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(20, 'B')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(30, 'C')));

  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  const auto& batch = ctx_->writtenBatches()[0];

  // Verify total size
  EXPECT_EQ(batch->computeChainDataLength(), 60);

  // Coalesce and verify content
  auto coalesced = batch->cloneCoalescedAsValue();
  const uint8_t* data = coalesced.data();

  // First 10 bytes should be 'A'
  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(data[i], 'A') << "Byte " << i;
  }
  // Next 20 bytes should be 'B'
  for (size_t i = 10; i < 30; ++i) {
    EXPECT_EQ(data[i], 'B') << "Byte " << i;
  }
  // Last 30 bytes should be 'C'
  for (size_t i = 30; i < 60; ++i) {
    EXPECT_EQ(data[i], 'C') << "Byte " << i;
  }
}

TEST_F(BatchingFrameHandlerTest, MultipleBatchesAcrossFlushes) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 100,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // First batch: triggers threshold
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(60)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(60)));

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->writtenBatches()[0]->computeChainDataLength(), 120);

  // Second batch: triggers on loop tick
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(30)));
  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 2);
  EXPECT_EQ(ctx_->writtenBatches()[1]->computeChainDataLength(), 30);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write::handler
