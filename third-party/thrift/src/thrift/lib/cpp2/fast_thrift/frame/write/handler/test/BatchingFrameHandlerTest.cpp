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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>

#include <gtest/gtest.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>

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

// Drains a flush list the way a caller-owned poller does: move the list out so
// re-armed callbacks land on a fresh list, then pop-and-run each entry.
// Returns the number of batchers flushed.
size_t drainFlushList(BatchingFrameHandler::FlushList& list) {
  size_t drained = 0;
  auto callbacks = std::move(list);
  while (!callbacks.empty()) {
    auto& cb = callbacks.front();
    callbacks.pop_front();
    cb.runLoopCallback();
    ++drained;
  }
  return drained;
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
    auto batch = msg.take<std::unique_ptr<folly::IOBuf>>();
    if (batch) {
      writtenBatches_.push_back(std::move(batch));
    }
    if (forceWriteError_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }
    if (writtenBatches_.size() > backpressureAt_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::
          Backpressure;
    }
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    ++exceptionCount_;
    lastException_ = std::move(e);
  }

  void awaitWriteReady() { awaitWriteReadyCalled_ = true; }
  void cancelAwaitWriteReady() { awaitWriteReadyCalled_ = false; }
  void deactivate() {}

  void setBackpressureAt(size_t n) { backpressureAt_ = n; }
  void setForceWriteError(bool b) { forceWriteError_ = b; }

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

  size_t exceptionCount() const { return exceptionCount_; }
  const folly::exception_wrapper& lastException() const {
    return lastException_;
  }

 private:
  folly::EventBase* evb_;
  std::vector<std::unique_ptr<folly::IOBuf>> writtenBatches_;
  size_t backpressureAt_{SIZE_MAX}; // Default: never backpressure
  bool awaitWriteReadyCalled_{false};
  bool forceWriteError_{false};
  size_t exceptionCount_{0};
  folly::exception_wrapper lastException_;
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

  ctx_->setBackpressureAt(0);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(50)));
  auto result = handler.onWrite(*ctx_, wrapFrame(makePayload(50)));

  EXPECT_EQ(
      result,
      apache::thrift::fast_thrift::channel_pipeline::Result::Backpressure);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
  EXPECT_TRUE(handler.isBackpressured());
  EXPECT_FALSE(handler.isScheduled());
}

TEST_F(BatchingFrameHandlerTest, BackpressureBuffersFramesUntilWriteReady) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 10,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  ctx_->setBackpressureAt(0);

  // Both writes together exceed the byte threshold, triggering a flush.
  // The transport receives the 120-byte batch but signals backpressure.
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(60)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(60)));
  EXPECT_TRUE(handler.isBackpressured());
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_EQ(ctx_->writtenBatches().size(), 1);

  // New frames must buffer without flushing or scheduling loop callback
  auto r = handler.onWrite(*ctx_, wrapFrame(makePayload(30)));
  EXPECT_EQ(
      r, apache::thrift::fast_thrift::channel_pipeline::Result::Backpressure);
  EXPECT_TRUE(handler.hasPendingData());
  EXPECT_EQ(handler.pendingFrames(), 1);
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_EQ(ctx_->writtenBatches().size(), 1);

  // Write ready: flush the buffered data
  ctx_->setBackpressureAt(SIZE_MAX);
  handler.onWriteReady(*ctx_);

  EXPECT_FALSE(handler.hasPendingData());
  EXPECT_FALSE(handler.isBackpressured());
  ASSERT_EQ(ctx_->writtenBatches().size(), 2);
  EXPECT_EQ(ctx_->totalBytesWritten(), 150);
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
  handler.onPipelineInactive(*ctx_);

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

// ============================================================================
// Error Propagation Tests
// ============================================================================

TEST_F(BatchingFrameHandlerTest, LoopTickErrorFiresException) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Queue a frame below threshold so flush happens via loop callback
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_EQ(ctx_->exceptionCount(), 0);

  // Transport errors on the next fireWrite
  ctx_->setForceWriteError(true);
  runEventBaseLoop();

  EXPECT_EQ(ctx_->exceptionCount(), 1);
  EXPECT_TRUE(bool(ctx_->lastException()));
}

TEST_F(BatchingFrameHandlerTest, WriteReadyErrorFiresException) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Buffer data without flushing
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));

  // Transport errors when onWriteReady drives the flush
  ctx_->setForceWriteError(true);
  handler.onWriteReady(*ctx_);

  EXPECT_EQ(ctx_->exceptionCount(), 1);
  EXPECT_TRUE(bool(ctx_->lastException()));
}

// ============================================================================
// Flush List Tests
// ============================================================================

// With a flush list set, a below-threshold write enqueues onto the caller's
// list instead of the EventBase: the loop tick does nothing, and only draining
// the list flushes the batch.
TEST_F(BatchingFrameHandlerTest, FlushListRedirectsDeferredFlush) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  BatchingFrameHandler::FlushList flushList;
  handler.setFlushList(&flushList);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));

  // Enqueued on the caller's list, not the EventBase.
  EXPECT_TRUE(handler.isScheduled());
  EXPECT_FALSE(flushList.empty());

  // The EventBase has nothing scheduled — the loop tick must not flush.
  runEventBaseLoop();
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);
  EXPECT_TRUE(handler.hasPendingData());

  // Draining the list runs the deferred flush.
  EXPECT_EQ(drainFlushList(flushList), 1);
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_FALSE(handler.hasPendingData());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

// A threshold flush fires inline even when a flush list is set, and unlinks the
// batcher so no stale entry is left behind for the drain.
TEST_F(BatchingFrameHandlerTest, FlushListThresholdFlushUnlinks) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 1024 * 1024,
      .maxPendingFrames = 3,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  BatchingFrameHandler::FlushList flushList;
  handler.setFlushList(&flushList);

  // First two writes defer onto the list.
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));
  EXPECT_FALSE(flushList.empty());

  // Third write crosses the frame threshold and flushes inline.
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 30);

  // The entry was removed from the list by the inline flush.
  EXPECT_TRUE(flushList.empty());
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_EQ(drainFlushList(flushList), 0);
}

// A single flush list drives multiple batchers: one entry per batcher, and one
// drain flushes them all.
TEST_F(BatchingFrameHandlerTest, FlushListSharedAcrossBatchers) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler::FlushList flushList;

  BatchingFrameHandler handlerA(config);
  BatchingFrameHandler handlerB(config);
  handlerA.handlerAdded(*ctx_);
  MockContext ctxB(evb_.get());
  handlerB.handlerAdded(ctxB);
  handlerA.setFlushList(&flushList);
  handlerB.setFlushList(&flushList);

  (void)handlerA.onWrite(*ctx_, wrapFrame(makePayload(100)));
  (void)handlerB.onWrite(ctxB, wrapFrame(makePayload(200)));

  EXPECT_EQ(drainFlushList(flushList), 2);
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
  ASSERT_EQ(ctxB.writtenBatches().size(), 1);
  EXPECT_EQ(ctxB.totalBytesWritten(), 200);
}

TEST_F(BatchingFrameHandlerTest, FlushListSwitchUnlinksPreviousList) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  BatchingFrameHandler::FlushList flushListA;
  BatchingFrameHandler::FlushList flushListB;
  handler.setFlushList(&flushListA);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  ASSERT_FALSE(flushListA.empty());

  handler.setFlushList(&flushListB);

  EXPECT_TRUE(flushListA.empty());
  EXPECT_FALSE(flushListB.empty());
  EXPECT_TRUE(handler.isScheduled());
  EXPECT_EQ(drainFlushList(flushListA), 0);
  EXPECT_EQ(drainFlushList(flushListB), 1);
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(BatchingFrameHandlerTest, FlushListClearReschedulesOnEventBase) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  BatchingFrameHandler::FlushList flushList;
  handler.setFlushList(&flushList);
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  ASSERT_FALSE(flushList.empty());

  handler.setFlushList(nullptr);

  EXPECT_TRUE(flushList.empty());
  EXPECT_TRUE(handler.isScheduled());
  runEventBaseLoop();
  EXPECT_FALSE(handler.isScheduled());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(BatchingFrameHandlerTest, HandlerRemovedUnlinksFlushList) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  BatchingFrameHandler::FlushList flushList;
  handler.setFlushList(&flushList);
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  ASSERT_FALSE(flushList.empty());

  handler.handlerRemoved(*ctx_);

  EXPECT_TRUE(flushList.empty());
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_EQ(drainFlushList(flushList), 0);
}

// A batcher destroyed while enqueued unlinks itself, leaving the caller's list
// safe to drain.
TEST_F(BatchingFrameHandlerTest, FlushListUnlinksOnHandlerTeardown) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler::FlushList flushList;

  {
    BatchingFrameHandler handler(config);
    handler.handlerAdded(*ctx_);
    handler.setFlushList(&flushList);
    (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
    EXPECT_FALSE(flushList.empty());
  }

  EXPECT_TRUE(flushList.empty());
}

// ============================================================================
// Backpressure disabled
// ============================================================================

TEST_F(
    BatchingFrameHandlerTest,
    NoBackpressure_DownstreamSaturationNeitherParksNorStrands) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 2,
  };
  BatchingFrameHandlerNoBackpressure handler(config);
  handler.handlerAdded(*ctx_);

  ctx_->setBackpressureAt(0); // every flush reports saturation

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(50)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(50)));

  // The threshold flush went downstream, but the handler neither recorded
  // saturation nor registered for a write-ready callback it can never receive.
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_FALSE(ctx_->awaitWriteReadyCalled());
  EXPECT_FALSE(handler.isBackpressured());

  // Later writes keep flowing rather than being stranded behind a saturation
  // flag that nothing would ever clear.
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(50, 'B')));
  runEventBaseLoop();
  EXPECT_EQ(ctx_->writtenBatches().size(), 2);
  EXPECT_FALSE(handler.hasPendingData());
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write::handler

// ============================================================================
// Pipeline Integration Tests
// ============================================================================
//
// These tests exercise backpressure through a real PipelineImpl, verifying
// the hook registration and dispatch chain end-to-end. This catches bugs like
// writeReadyHook_ naming mismatches that unit tests above cannot detect
// (they call onWriteReady() directly, bypassing the pipeline machinery).

namespace apache::thrift::fast_thrift::frame::write::handler {

namespace cp = apache::thrift::fast_thrift::channel_pipeline;
using cp::test::MockHeadHandler;
using cp::test::MockTailHandler;
using cp::test::TestAllocator;

HANDLER_TAG(batching);

class BatchingFrameHandlerPipelineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    transport_.reset();
    app_.reset();
    allocator_.reset();
    handlerPtr_ = nullptr;
  }

  cp::PipelineImpl::Ptr buildPipeline(BatchingHandlerConfig config = {}) {
    auto handler = std::make_unique<BatchingFrameHandler>(config);
    handlerPtr_ = handler.get();
    return cp::
        PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
            .setEventBase(&evb_)
            .setHead(&transport_)
            .setTail(&app_)
            .setAllocator(&allocator_)
            .addNextOutbound<BatchingFrameHandler>(
                batching_tag, std::move(handler))
            .build();
  }

  static cp::TypeErasedBox wrapFrame(std::unique_ptr<folly::IOBuf> buf) {
    return cp::TypeErasedBox(std::move(buf));
  }

  static std::unique_ptr<folly::IOBuf> makePayload(size_t size) {
    auto buf = folly::IOBuf::create(size);
    buf->append(size);
    return buf;
  }

  folly::EventBase evb_;
  MockHeadHandler transport_;
  MockTailHandler app_;
  TestAllocator allocator_;
  BatchingFrameHandler* handlerPtr_{nullptr};
};

// Regression test: if writeReadyHook_ is misnamed, the pipeline's compile-time
// hook detection silently fails and hasPendingWriteReady() stays false even
// after backpressure is signaled by ctx.awaitWriteReady().
TEST_F(BatchingFrameHandlerPipelineTest, HookRegisteredOnBackpressure) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 10,
  };
  transport_.setWriteResult(cp::Result::Backpressure);
  auto pipeline = buildPipeline(config);

  (void)pipeline->fireWrite(wrapFrame(makePayload(60)));
  (void)pipeline->fireWrite(wrapFrame(makePayload(60)));

  EXPECT_TRUE(pipeline->hasPendingWriteReady());
  EXPECT_TRUE(handlerPtr_->isBackpressured());
}

// Verifies that pipeline->onWriteReady() walks the writeReadyList and
// dispatches to BatchingFrameHandler::onWriteReady(), which clears backpressure
// and flushes the data buffered during backpressure.
TEST_F(BatchingFrameHandlerPipelineTest, WriteReadyDispatchedThroughPipeline) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 10,
  };
  transport_.setWriteResult(cp::Result::Backpressure);
  auto pipeline = buildPipeline(config);

  // Trigger threshold flush → backpressure
  (void)pipeline->fireWrite(wrapFrame(makePayload(60)));
  (void)pipeline->fireWrite(wrapFrame(makePayload(60)));
  EXPECT_TRUE(pipeline->hasPendingWriteReady());

  // Buffer a frame during backpressure
  (void)pipeline->fireWrite(wrapFrame(makePayload(30)));
  EXPECT_TRUE(handlerPtr_->hasPendingData());

  // Simulate transport becoming writable
  transport_.setWriteResult(cp::Result::Success);
  pipeline->onWriteReady();

  EXPECT_FALSE(pipeline->hasPendingWriteReady());
  EXPECT_FALSE(handlerPtr_->isBackpressured());
  EXPECT_FALSE(handlerPtr_->hasPendingData());
  EXPECT_EQ(transport_.writeCount(), 2);
}

// Covers the path where backpressure is first encountered via the loop-callback
// flush (not a threshold flush). Verifies that after the loop tick fires and
// doFlush() encounters backpressure, subsequent writes buffer correctly without
// rescheduling the loop callback.
TEST_F(BatchingFrameHandlerPipelineTest, LoopTickBackpressureRegisters) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  transport_.setWriteResult(cp::Result::Backpressure);
  auto pipeline = buildPipeline(config);

  // Write below threshold — loop callback scheduled, no flush yet
  (void)pipeline->fireWrite(wrapFrame(makePayload(100)));
  EXPECT_FALSE(pipeline->hasPendingWriteReady());
  EXPECT_FALSE(handlerPtr_->isBackpressured());

  // Loop tick fires doFlush() → encounters backpressure → hook registered
  evb_.loopOnce(EVLOOP_NONBLOCK);
  EXPECT_TRUE(pipeline->hasPendingWriteReady());
  EXPECT_TRUE(handlerPtr_->isBackpressured());
  EXPECT_FALSE(handlerPtr_->isScheduled());

  // Write during backpressure — must buffer, must not reschedule loop callback
  (void)pipeline->fireWrite(wrapFrame(makePayload(50)));
  EXPECT_TRUE(handlerPtr_->hasPendingData());
  EXPECT_FALSE(handlerPtr_->isScheduled());

  // Resume — onWriteReady flushes the buffered frame
  transport_.setWriteResult(cp::Result::Success);
  pipeline->onWriteReady();

  EXPECT_FALSE(handlerPtr_->hasPendingData());
  EXPECT_FALSE(handlerPtr_->isBackpressured());
  EXPECT_EQ(transport_.writeCount(), 2);
}

// Verifies that when the transport returns Result::Error, the exception fired
// by BatchingFrameHandler propagates through the pipeline and reaches the tail.
TEST_F(BatchingFrameHandlerPipelineTest, TransportErrorPropagatesToTail) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  transport_.setWriteResult(cp::Result::Error);
  auto pipeline = buildPipeline(config);

  // Write below threshold — flush happens on loop tick, where the transport
  // error is observed and converted to an exception.
  (void)pipeline->fireWrite(wrapFrame(makePayload(100)));
  EXPECT_EQ(app_.exceptionCount(), 0);

  evb_.loopOnce(EVLOOP_NONBLOCK);

  EXPECT_EQ(app_.exceptionCount(), 1);
}

// ============================================================================
// Tracker mixin: verify the hooks (onWrite, onFlush, onWriteComplete) fire
// when the batcher is instantiated with a non-NoOp tracker.
// ============================================================================

namespace {

// Single-value event enum mirroring a tracker's EventId.
enum class CapturingEventId : std::uint32_t {
  WriteComplete,
  Count,
};

// Tracker that captures hook invocations. Its onEvent records the raw
// TypeErasedBox arrival count — the batcher just delegates onEvent through
// without unpacking the type.
struct CapturingTracker {
  using EventId = CapturingEventId;
  static constexpr apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
      EventId::WriteComplete>
      kSubscribedEvents{};

  size_t onWriteCount{0};
  size_t onFlushCount{0};
  size_t onEventCount{0};

  void onWrite() noexcept { ++onWriteCount; }
  void onFlush() noexcept { ++onFlushCount; }

  template <typename Context>
  void onEvent(
      Context& /*ctx*/,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
      /*box*/) noexcept {
    ++onEventCount;
  }
};

static_assert(WriteCompletionTracker<CapturingTracker>);

// An event enum for a pipeline that asks its buffering handlers to flush ahead
// of a teardown. A batcher bound to it subscribes to FlushWrites whether or not
// it tracks write completions.
enum class FlushableEventId : std::uint32_t {
  WriteComplete,
  FlushWrites,
  Count,
};

static_assert(
    !HasFlushWritesEvent<CapturingEventId>,
    "an enum without FlushWrites must not opt its batcher into the flush path");
static_assert(HasFlushWritesEvent<FlushableEventId>);

class BatchingFrameHandlerTrackerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evb_ = std::make_unique<folly::EventBase>();
    ctx_ = std::make_unique<MockContext>(evb_.get());
  }

  std::unique_ptr<folly::EventBase> evb_;
  std::unique_ptr<MockContext> ctx_;
};

} // namespace

TEST_F(BatchingFrameHandlerTrackerTest, TrackerHooksFireOnWriteAndFlush) {
  BatchingFrameHandlerT<CapturingTracker> handler({
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 3,
  });
  handler.handlerAdded(*ctx_);

  // Three writes — third one triggers a flush at the frame threshold.
  ASSERT_EQ(
      handler.onWrite(*ctx_, wrapFrame(makePayload(64))),
      apache::thrift::fast_thrift::channel_pipeline::Result::Success);
  ASSERT_EQ(
      handler.onWrite(*ctx_, wrapFrame(makePayload(64))),
      apache::thrift::fast_thrift::channel_pipeline::Result::Success);
  ASSERT_EQ(
      handler.onWrite(*ctx_, wrapFrame(makePayload(64))),
      apache::thrift::fast_thrift::channel_pipeline::Result::Success);

  EXPECT_EQ(handler.tracker().onWriteCount, 3u);
  EXPECT_EQ(handler.tracker().onFlushCount, 1u);
}

// A teardown is imminent and the batch is still buffered: FlushWrites has to
// push it downstream there and then, or the frame dies with the connection.
TEST_F(BatchingFrameHandlerTrackerTest, FlushWritesEventFlushesPendingBatch) {
  BatchingFrameHandlerT<
      NoOpWriteCompletionTracker,
      BackpressureEnabled,
      FlushableEventId>
      handler;
  handler.handlerAdded(*ctx_);

  ASSERT_EQ(
      handler.onWrite(*ctx_, wrapFrame(makePayload(64))),
      apache::thrift::fast_thrift::channel_pipeline::Result::Success);
  ASSERT_TRUE(handler.hasPendingData());
  ASSERT_TRUE(ctx_->writtenBatches().empty()) << "batched, not yet downstream";

  handler.onEvent(
      *ctx_,
      FlushableEventId::FlushWrites,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox{});

  EXPECT_FALSE(handler.hasPendingData());
  EXPECT_EQ(ctx_->writtenBatches().size(), 1u)
      << "batch never reached the next handler";
}

// Nothing buffered is not an error, and must not manufacture an empty write.
TEST_F(BatchingFrameHandlerTrackerTest, FlushWritesEventOnEmptyBatchIsNoOp) {
  BatchingFrameHandlerT<
      NoOpWriteCompletionTracker,
      BackpressureEnabled,
      FlushableEventId>
      handler;
  handler.handlerAdded(*ctx_);

  handler.onEvent(
      *ctx_,
      FlushableEventId::FlushWrites,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox{});

  EXPECT_TRUE(ctx_->writtenBatches().empty());
}

TEST_F(BatchingFrameHandlerTrackerTest, OnEventDelegatesToTracker) {
  BatchingFrameHandlerT<CapturingTracker> handler;
  handler.handlerAdded(*ctx_);

  // The batcher's onEvent should pass the box through to the tracker
  // unchanged. The tracker owns the per-pipeline event type discrimination.
  // We pass a dummy int box twice and confirm the tracker received both.
  handler.onEvent(
      *ctx_,
      CapturingEventId::WriteComplete,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(0));
  handler.onEvent(
      *ctx_,
      CapturingEventId::WriteComplete,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(0));

  EXPECT_EQ(handler.tracker().onEventCount, 2u);
}

// The zero-cost claim, observed through a real pipeline. The no-backpressure
// specialization has no writeReadyHook_, so makeHandlerNode's compile-time
// detection skips it and PipelineImpl never links it into writeReadyList_ —
// hasPendingWriteReady() must stay false even while the transport reports
// saturation on every write. This is the counterpart to
// HookRegisteredOnBackpressure above.
TEST_F(BatchingFrameHandlerPipelineTest, NoBackpressure_HookNeverRegistered) {
  BatchingHandlerConfig config{
      .maxPendingBytes = 100,
      .maxPendingFrames = 10,
  };
  transport_.setWriteResult(cp::Result::Backpressure);

  auto pipeline =
      cp::PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextOutbound<BatchingFrameHandlerNoBackpressure>(
              batching_tag,
              std::make_unique<BatchingFrameHandlerNoBackpressure>(config))
          .build();

  // Threshold flush hits a saturated transport.
  (void)pipeline->fireWrite(wrapFrame(makePayload(60)));
  (void)pipeline->fireWrite(wrapFrame(makePayload(60)));

  EXPECT_FALSE(pipeline->hasPendingWriteReady());

  // And onWriteReady has nothing to dispatch, so it is inert rather than
  // crashing on a half-registered handler.
  pipeline->onWriteReady();
  EXPECT_FALSE(pipeline->hasPendingWriteReady());
}

} // namespace apache::thrift::fast_thrift::frame::write::handler
