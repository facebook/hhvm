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

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/IntervalBatchingFrameHandler.h>

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

channel_pipeline::TypeErasedBox wrapFrame(std::unique_ptr<folly::IOBuf> buf) {
  return channel_pipeline::TypeErasedBox(std::move(buf));
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
    return channel_pipeline::Result::Success;
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

 private:
  folly::EventBase* evb_;
  std::vector<std::unique_ptr<folly::IOBuf>> writtenBatches_;
};

// ============================================================================
// Test Fixture
// ============================================================================

class IntervalBatchingFrameHandlerTest : public ::testing::Test {
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
// Zero-Interval Tests (LoopCallback mode)
// ============================================================================

TEST_F(
    IntervalBatchingFrameHandlerTest, ZeroInterval_SingleFrameFlushOnLoopTick) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));

  EXPECT_FALSE(handler.empty());
  EXPECT_EQ(handler.pendingBytes(), 100);
  EXPECT_EQ(handler.pendingFrames(), 1);
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  runEventBaseLoop();

  EXPECT_TRUE(handler.empty());
  EXPECT_EQ(handler.pendingBytes(), 0);
  EXPECT_EQ(handler.pendingFrames(), 0);
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(IntervalBatchingFrameHandlerTest, ZeroInterval_MultipleFramesBatched) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100, 'A')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200, 'B')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(300, 'C')));

  EXPECT_EQ(handler.pendingBytes(), 600);
  EXPECT_EQ(handler.pendingFrames(), 3);
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 600);
}

// ============================================================================
// Interval-Based Tests (HHWheelTimer mode)
// ============================================================================

TEST_F(IntervalBatchingFrameHandlerTest, Interval_FlushOnTimeout) {
  IntervalBatchingHandlerConfig config{
      .batchingInterval = std::chrono::milliseconds(10),
      .batchingSize = 1000,
      .batchingByteSize = 1024 * 1024,
  };
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));

  EXPECT_FALSE(handler.empty());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // Run the event loop until the timer fires and flushes
  evb_->loop();

  EXPECT_TRUE(handler.empty());
  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 100);
}

TEST_F(IntervalBatchingFrameHandlerTest, Interval_EarlyFlushOnByteThreshold) {
  IntervalBatchingHandlerConfig config{
      .batchingInterval = std::chrono::milliseconds(100),
      .batchingSize = 1000,
      .batchingByteSize = 500,
  };
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));

  // Not yet at byte threshold
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // This should trigger early flush (600 >= 500)
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(200)));

  // Early flush is via LoopCallback, not immediate
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 600);
  EXPECT_TRUE(handler.empty());
}

TEST_F(
    IntervalBatchingFrameHandlerTest,
    Interval_EarlyFlushOnFrameCountThreshold) {
  IntervalBatchingHandlerConfig config{
      .batchingInterval = std::chrono::milliseconds(100),
      .batchingSize = 3,
      .batchingByteSize = 1024 * 1024,
  };
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));

  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  // 3rd frame hits batchingSize threshold
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10)));

  EXPECT_EQ(ctx_->writtenBatches().size(), 0);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->writtenBatches().size(), 1);
  EXPECT_EQ(ctx_->totalBytesWritten(), 30);
  EXPECT_TRUE(handler.empty());
}

// ============================================================================
// Drain Tests
// ============================================================================

TEST_F(IntervalBatchingFrameHandlerTest, DrainFlushesImmediately) {
  IntervalBatchingHandlerConfig config{
      .batchingInterval = std::chrono::milliseconds(100),
      .batchingSize = 1000,
      .batchingByteSize = 1024 * 1024,
  };
  IntervalBatchingFrameHandler handler(config);
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

TEST_F(IntervalBatchingFrameHandlerTest, DrainOnEmptyIsNoop) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  handler.drain();

  EXPECT_TRUE(handler.empty());
  EXPECT_EQ(ctx_->writtenBatches().size(), 0);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(IntervalBatchingFrameHandlerTest, EmptyFrameIgnored) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  std::unique_ptr<folly::IOBuf> nullFrame;
  (void)handler.onWrite(
      *ctx_, channel_pipeline::TypeErasedBox(std::move(nullFrame)));

  EXPECT_TRUE(handler.empty());
  EXPECT_EQ(handler.pendingFrames(), 0);
}

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST_F(IntervalBatchingFrameHandlerTest, DeactivateDrainsPendingData) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_FALSE(handler.empty());

  handler.onPipelineDeactivated(*ctx_);

  EXPECT_TRUE(handler.empty());
  EXPECT_EQ(handler.pendingBytes(), 0);
  EXPECT_EQ(handler.pendingFrames(), 0);
}

TEST_F(IntervalBatchingFrameHandlerTest, HandlerRemovedCleansState) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(100)));
  EXPECT_FALSE(handler.empty());

  handler.handlerRemoved(*ctx_);

  EXPECT_TRUE(handler.empty());
}

// ============================================================================
// Data Integrity
// ============================================================================

TEST_F(IntervalBatchingFrameHandlerTest, BatchedDataIntegrity) {
  IntervalBatchingHandlerConfig config{};
  IntervalBatchingFrameHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(10, 'A')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(20, 'B')));
  (void)handler.onWrite(*ctx_, wrapFrame(makePayload(30, 'C')));

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

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write::handler
