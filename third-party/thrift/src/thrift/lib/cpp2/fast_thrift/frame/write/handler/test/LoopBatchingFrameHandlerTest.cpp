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

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/LoopBatchingFrameHandler.h>

#include <gtest/gtest.h>
#include <folly/ExceptionWrapper.h>
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

channel_pipeline::TypeErasedBox wrapFrame(std::unique_ptr<folly::IOBuf> buf) {
  return channel_pipeline::TypeErasedBox(std::move(buf));
}

// Drains a flush list the way a caller-owned poller does: move the list out so
// re-armed callbacks land on a fresh list, then pop-and-run each entry.
// Returns the number of batchers flushed.
size_t drainFlushList(LoopBatchingFrameHandler::FlushList& list) {
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

  channel_pipeline::Result fireWrite(channel_pipeline::TypeErasedBox&& msg) {
    auto batch = msg.take<std::unique_ptr<folly::IOBuf>>();
    if (batch) {
      writtenBatches_.push_back(std::move(batch));
    }
    if (forceWriteError_) {
      return channel_pipeline::Result::Error;
    }
    return channel_pipeline::Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    ++exceptionCount_;
    lastException_ = std::move(e);
  }

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

  void setForceWriteError(bool b) { forceWriteError_ = b; }
  size_t exceptionCount() const { return exceptionCount_; }
  const folly::exception_wrapper& lastException() const {
    return lastException_;
  }

 private:
  folly::EventBase* evb_;
  std::vector<std::unique_ptr<folly::IOBuf>> writtenBatches_;
  bool forceWriteError_{false};
  size_t exceptionCount_{0};
  folly::exception_wrapper lastException_;
};

// ============================================================================
// Test Fixture
// ============================================================================

class LoopBatchingFrameHandlerTest : public ::testing::Test {
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

TEST_F(LoopBatchingFrameHandlerTest, SingleFrameFlushOnLoopTick) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));

  EXPECT_TRUE(handler.isScheduled());
  EXPECT_FALSE(handler.empty());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // Double-scheduling: first loop reschedules, second loop flushes
  runEventBaseLoop();
  runEventBaseLoop();

  EXPECT_FALSE(handler.isScheduled());
  EXPECT_TRUE(handler.empty());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(LoopBatchingFrameHandlerTest, MultipleFramesBatched) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100, 'A')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200, 'B')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(300, 'C')));

  EXPECT_TRUE(handler.isScheduled());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  runEventBaseLoop();
  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 600);
}

TEST_F(LoopBatchingFrameHandlerTest, NoThresholds_LargeWriteStillBatched) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  // Even very large writes are deferred to loop callback — no threshold flush
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10 * 1024 * 1024)));
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  runEventBaseLoop();
  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 10 * 1024 * 1024);
}

// ============================================================================
// Drain Tests
// ============================================================================

TEST_F(LoopBatchingFrameHandlerTest, DrainFlushesImmediately) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));

  EXPECT_FALSE(handler.empty());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  handler.drain();

  EXPECT_TRUE(handler.empty());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 300);
}

TEST_F(LoopBatchingFrameHandlerTest, DrainOnEmptyIsNoop) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  handler.drain();

  EXPECT_TRUE(handler.empty());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(LoopBatchingFrameHandlerTest, EmptyFrameIgnored) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  std::unique_ptr<folly::IOBuf> nullFrame;
  (void)handler.onWrite(
      *ctx_, channel_pipeline::TypeErasedBox(std::move(nullFrame)));

  EXPECT_TRUE(handler.empty());
  EXPECT_FALSE(handler.isScheduled());
}

TEST_F(LoopBatchingFrameHandlerTest, MultipleBatchesAcrossFlushes) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  // First batch
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(60)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(60)));

  runEventBaseLoop();
  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->writtenBatches()[0]->computeChainDataLength(), 120);

  // Second batch
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(30)));
  runEventBaseLoop();
  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 2);
  EXPECT_EQ(ctx_->writtenBatches()[1]->computeChainDataLength(), 30);
}

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST_F(LoopBatchingFrameHandlerTest, DeactivateDrainsPendingData) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_FALSE(handler.empty());
  EXPECT_TRUE(handler.isScheduled());

  handler.onPipelineInactive(*ctx_);

  EXPECT_TRUE(handler.empty());
  EXPECT_FALSE(handler.isScheduled());
}

TEST_F(LoopBatchingFrameHandlerTest, HandlerRemovedCleansState) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_FALSE(handler.empty());

  handler.handlerRemoved(*ctx_);

  EXPECT_TRUE(handler.empty());
  EXPECT_FALSE(handler.isScheduled());
}

// ============================================================================
// Data Integrity
// ============================================================================

TEST_F(LoopBatchingFrameHandlerTest, BatchedDataIntegrity) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10, 'A')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(20, 'B')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(30, 'C')));

  runEventBaseLoop();
  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  const auto& batch = ctx_->writtenBatches()[0];
  EXPECT_EQ(batch->computeChainDataLength(), 60);

  auto coalesced = batch->cloneCoalescedAsValue();
  const uint8_t* data = coalesced.data();

  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(data[i], 'A') << "Byte " << i;
  }
  for (size_t i = 10; i < 30; ++i) {
    EXPECT_EQ(data[i], 'B') << "Byte " << i;
  }
  for (size_t i = 30; i < 60; ++i) {
    EXPECT_EQ(data[i], 'C') << "Byte " << i;
  }
}

// ============================================================================
// Error Propagation Tests
// ============================================================================

TEST_F(LoopBatchingFrameHandlerTest, LoopTickErrorFiresException) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_EQ(ctx_->exceptionCount(), 0);

  // Transport errors when the loop-tick flush fires
  ctx_->setForceWriteError(true);
  // Double-loop because this handler reschedules once before flushing
  runEventBaseLoop();
  runEventBaseLoop();

  EXPECT_EQ(ctx_->exceptionCount(), 1);
  EXPECT_TRUE(bool(ctx_->lastException()));
}

// ============================================================================
// Flush List Tests
// ============================================================================

// With a flush list set, a write enqueues onto the caller's list instead of the
// EventBase: the loop ticks do nothing, and a single drain flushes directly
// (no double-scheduling).
TEST_F(
    LoopBatchingFrameHandlerTest, FlushListRedirectsAndFlushesOnSingleDrain) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  LoopBatchingFrameHandler::FlushList flushList;
  handler.setFlushList(&flushList);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_TRUE(handler.isScheduled());
  EXPECT_FALSE(flushList.empty());

  // Nothing is scheduled on the EventBase, so loop ticks must not flush.
  runEventBaseLoop();
  runEventBaseLoop();
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);
  EXPECT_FALSE(handler.empty());

  // A single drain flushes directly — the double-schedule is disabled.
  EXPECT_EQ(drainFlushList(flushList), 1);
  EXPECT_FALSE(handler.isScheduled());
  EXPECT_TRUE(handler.empty());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

// A single flush list drives multiple batchers: one entry per batcher, and one
// drain flushes them all.
TEST_F(LoopBatchingFrameHandlerTest, FlushListSharedAcrossBatchers) {
  LoopBatchingFrameHandler::FlushList flushList;

  LoopBatchingFrameHandler handlerA;
  LoopBatchingFrameHandler handlerB;
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

TEST_F(LoopBatchingFrameHandlerTest, FlushListSwitchCancelsEventBaseCallback) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  ASSERT_TRUE(handler.isScheduled());

  LoopBatchingFrameHandler::FlushList flushList;
  handler.setFlushList(&flushList);

  EXPECT_FALSE(flushList.empty());
  runEventBaseLoop();
  EXPECT_FALSE(handler.empty());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  EXPECT_EQ(drainFlushList(flushList), 1);
  EXPECT_TRUE(handler.empty());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(LoopBatchingFrameHandlerTest, HandlerRemovedUnlinksFlushList) {
  LoopBatchingFrameHandler handler;
  handler.handlerAdded(*ctx_);

  LoopBatchingFrameHandler::FlushList flushList;
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
TEST_F(LoopBatchingFrameHandlerTest, FlushListUnlinksOnHandlerTeardown) {
  LoopBatchingFrameHandler::FlushList flushList;

  {
    LoopBatchingFrameHandler handler;
    handler.handlerAdded(*ctx_);
    handler.setFlushList(&flushList);
    (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
    EXPECT_FALSE(flushList.empty());
  }

  EXPECT_TRUE(flushList.empty());
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write::handler
