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

#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>

#include <gtest/gtest.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <array>
#include <cstring>
#include <map>
#include <stdexcept>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write::handler {
namespace {

// ============================================================================
// Standalone Mock Context
// ============================================================================

struct CapturedFrame {
  uint32_t streamId;
  FrameType frameType;
  size_t dataSize;
  bool follows;
};

class MockContext {
 public:
  explicit MockContext(folly::EventBase* evb) : evb_(evb) {}

  folly::EventBase* eventBase() const { return evb_; }

  apache::thrift::fast_thrift::channel_pipeline::Result fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (errorEnabled_ && written_.size() >= errorAt_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }
    if (backpressureAt_ > 0 && written_.size() >= backpressureAt_) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::
          Backpressure;
    }

    auto& f = msg.get<ComposedFrame>();
    CapturedFrame cap{};
    cap.streamId = f.streamId;
    cap.frameType = f.frameType;
    cap.dataSize = f.data ? f.data->computeChainDataLength() : 0;
    cap.follows = f.follows;
    written_.push_back(cap);
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    ++fireExceptionCount_;
    lastException_ = std::move(e);
  }

  void awaitWriteReady() { awaitWriteReadyCalled_ = true; }
  void cancelAwaitWriteReady() { awaitWriteReadyCalled_ = false; }
  void deactivate() {}

  void setBackpressureAt(size_t n) { backpressureAt_ = n; }
  void setErrorAt(size_t n) {
    errorEnabled_ = true;
    errorAt_ = n;
  }
  const std::vector<CapturedFrame>& written() const { return written_; }
  bool awaitWriteReadyCalled() const { return awaitWriteReadyCalled_; }
  size_t fireExceptionCount() const { return fireExceptionCount_; }
  const folly::exception_wrapper& lastException() const {
    return lastException_;
  }

 private:
  folly::EventBase* evb_;
  std::vector<CapturedFrame> written_;
  size_t backpressureAt_{0};
  size_t errorAt_{0};
  bool errorEnabled_{false};
  bool awaitWriteReadyCalled_{false};
  size_t fireExceptionCount_{0};
  folly::exception_wrapper lastException_;
};

// ============================================================================
// Recording tracker — captures the handler's onFragment / onFlush hook calls so
// handler-level tests can assert the doFlush wiring (per-fragment isLast flags,
// onFlush only on the success path, no onFragment for a fragment that errored).
// ============================================================================

struct RecordingTracker {
  using EventId = apache::thrift::fast_thrift::channel_pipeline::NoEvent;
  static constexpr std::array<EventId, 0> kSubscribedEvents{};

  struct FragmentCall {
    uint32_t streamId;
    bool isLastFragment;
  };
  std::vector<FragmentCall> fragments;
  size_t flushCount{0};

  void onFragment(uint32_t streamId, bool isLastFragment) noexcept {
    fragments.push_back({streamId, isLastFragment});
  }
  void onFlush() noexcept { ++flushCount; }

  template <typename Context>
  void onEvent(
      Context& /*ctx*/,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
      /*box*/) noexcept {}
};

static_assert(
    FragmentCompletionTracker<RecordingTracker>,
    "RecordingTracker must satisfy FragmentCompletionTracker concept");

// ============================================================================
// Test Helpers
// ============================================================================

std::unique_ptr<folly::IOBuf> makePayload(size_t size) {
  auto buf = folly::IOBuf::create(size);
  buf->append(size);
  std::memset(buf->writableData(), 'X', size);
  return buf;
}

apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox makePayloadFrame(
    uint32_t streamId, size_t dataSize) {
  ComposedFrame f;
  f.frameType = FrameType::PAYLOAD;
  f.streamId = streamId;
  f.data = makePayload(dataSize);
  f.next = true;
  return apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
      std::move(f));
}

apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox makeCancelFrame(
    uint32_t streamId) {
  ComposedFrame f;
  f.frameType = FrameType::CANCEL;
  f.streamId = streamId;
  return apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
      std::move(f));
}

apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox makeErrorFrame(
    uint32_t streamId) {
  ComposedFrame f;
  f.frameType = FrameType::ERROR;
  f.streamId = streamId;
  f.errorCode = 0x00000201;
  return apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
      std::move(f));
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

  auto result = handler.onWrite(*ctx_, makePayloadFrame(1, 32 * 1024));
  EXPECT_EQ(
      result, apache::thrift::fast_thrift::channel_pipeline::Result::Success);

  EXPECT_EQ(handler.immediateQueueSize(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 0);

  ASSERT_EQ(ctx_->written().size(), 1);
  EXPECT_EQ(ctx_->written()[0].streamId, 1);
  EXPECT_EQ(ctx_->written()[0].dataSize, 32 * 1024);
  EXPECT_FALSE(ctx_->written()[0].follows);
}

TEST_F(FrameFragmentationHandlerTest, LargeFrameFragmented) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  auto result = handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  EXPECT_EQ(
      result, apache::thrift::fast_thrift::channel_pipeline::Result::Success);

  EXPECT_EQ(handler.immediateQueueSize(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 1);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 3);
  EXPECT_TRUE(ctx_->written()[0].follows);
  EXPECT_TRUE(ctx_->written()[1].follows);
  EXPECT_FALSE(ctx_->written()[2].follows);
  // First fragment keeps original PAYLOAD type; continuations are PAYLOAD too.
  EXPECT_EQ(ctx_->written()[0].frameType, FrameType::PAYLOAD);
  EXPECT_EQ(ctx_->written()[1].frameType, FrameType::PAYLOAD);
  EXPECT_EQ(ctx_->written()[2].frameType, FrameType::PAYLOAD);
}

// ============================================================================
// Non-fragmentable bypass
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, ErrorFrameBypasses) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makeErrorFrame(1));
  ASSERT_EQ(ctx_->written().size(), 1);
  EXPECT_EQ(ctx_->written()[0].frameType, FrameType::ERROR);
  EXPECT_EQ(handler.pendingStreamCount(), 0);
  EXPECT_EQ(handler.immediateQueueSize(), 0);
}

// ============================================================================
// Multi-Stream Round-Robin Tests
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, MultiStreamRoundRobinInterleaving) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024)); // 3 fragments
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 128 * 1024)); // 2 fragments
  (void)handler.onWrite(*ctx_, makePayloadFrame(3, 65 * 1024)); // 2 fragments

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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 32 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(3, 128 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(4, 16 * 1024));

  EXPECT_EQ(handler.immediateQueueSize(), 2);
  EXPECT_EQ(handler.pendingStreamCount(), 2);

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 7);

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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 128 * 1024));

  ctx_->setBackpressureAt(2);

  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 2);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(FrameFragmentationHandlerTest, BackpressureResumeCompletes) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024)); // 3 fragments

  ctx_->setBackpressureAt(2);

  runEventBaseLoop();
  EXPECT_EQ(ctx_->written().size(), 2);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());

  ctx_->setBackpressureAt(0);
  handler.onWriteReady(*ctx_);

  EXPECT_EQ(handler.pendingStreamCount(), 0);
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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 192 * 1024));
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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 1);

  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 192 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 2);

  (void)handler.onWrite(*ctx_, makePayloadFrame(3, 192 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  (void)handler.onWrite(*ctx_, makePayloadFrame(4, 192 * 1024));
  EXPECT_GT(ctx_->written().size(), 0);
}

TEST_F(FrameFragmentationHandlerTest, BackpressureAtExactByteLimit) {
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = 128 * 1024,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);
  EXPECT_EQ(handler.pendingBytes(), 128 * 1024);

  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 128 * 1024));
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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 128 * 1024));
  EXPECT_EQ(ctx_->written().size(), 0);

  (void)handler.onWrite(*ctx_, makePayloadFrame(3, 128 * 1024));
  EXPECT_GT(ctx_->written().size(), 0);
}

TEST_F(FrameFragmentationHandlerTest, BackpressureOnImmediateQueueDrain) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(10, 192 * 1024));
  EXPECT_EQ(handler.pendingStreamCount(), 1);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 2 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 2 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(3, 2 * 1024));

  EXPECT_EQ(handler.immediateQueueSize(), 3);

  ctx_->setBackpressureAt(1);

  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(FrameFragmentationHandlerTest, BackpressureOnFirstFragment) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);

  ctx_->setBackpressureAt(1);

  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_TRUE(ctx_->written()[0].follows);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
}

TEST_F(FrameFragmentationHandlerTest, BackpressureOnContinuationFragment) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);

  ctx_->setBackpressureAt(1);

  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_TRUE(ctx_->written()[0].follows);
  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());

  ctx_->setBackpressureAt(0);
  handler.onWriteReady(*ctx_);

  EXPECT_GE(ctx_->written().size(), 2);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, ExactFragmentSizeNotFragmented) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 64 * 1024));

  EXPECT_EQ(handler.immediateQueueSize(), 0);
  EXPECT_EQ(handler.pendingStreamCount(), 0);

  ASSERT_EQ(ctx_->written().size(), 1);
  EXPECT_FALSE(ctx_->written()[0].follows);
}

TEST_F(FrameFragmentationHandlerTest, StreamCompletionRemovesFromMap) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 65 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);

  runEventBaseLoop();

  EXPECT_EQ(handler.pendingStreamCount(), 0);
}

TEST_F(FrameFragmentationHandlerTest, UnevenFragmentSizes) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 100 * 1024));

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 2);

  EXPECT_EQ(ctx_->written()[0].dataSize, 64 * 1024);
  EXPECT_TRUE(ctx_->written()[0].follows);

  EXPECT_EQ(ctx_->written()[1].dataSize, 36 * 1024);
  EXPECT_FALSE(ctx_->written()[1].follows);
}

TEST_F(FrameFragmentationHandlerTest, CancelClearsStreamFragments) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);
  EXPECT_TRUE(handler.hasPendingStream(1));

  (void)handler.onWrite(*ctx_, makeCancelFrame(1));

  EXPECT_EQ(handler.pendingStreamCount(), 0);
  EXPECT_FALSE(handler.hasPendingStream(1));

  runEventBaseLoop();

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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 1);
  EXPECT_TRUE(handler.hasPendingStream(1));

  (void)handler.onWrite(*ctx_, makeErrorFrame(1));

  EXPECT_EQ(handler.pendingStreamCount(), 0);
  EXPECT_FALSE(handler.hasPendingStream(1));

  runEventBaseLoop();

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

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 128 * 1024));

  EXPECT_EQ(handler.pendingStreamCount(), 2);

  (void)handler.onWrite(*ctx_, makeCancelFrame(1));

  EXPECT_EQ(handler.pendingStreamCount(), 1);
  EXPECT_FALSE(handler.hasPendingStream(1));
  EXPECT_TRUE(handler.hasPendingStream(2));

  runEventBaseLoop();

  size_t stream2Fragments = 0;
  for (const auto& f : ctx_->written()) {
    if (f.streamId == 2 && f.frameType == FrameType::PAYLOAD) {
      stream2Fragments++;
    }
  }
  EXPECT_EQ(stream2Fragments, 2);
}

// ============================================================================
// Multi-frame-per-stream regression — exercises the per-stream FIFO.
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, MultiFramesPerStreamAllSent) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  // Two large PAYLOAD frames for the same stream. With the pre-rework
  // single-payload state the second would have clobbered the first's
  // remaining bytes; both must come out intact and in order.
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 100 * 1024)); // 2 fragments
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 100 * 1024)); // 2 fragments

  runEventBaseLoop();

  ASSERT_EQ(ctx_->written().size(), 4);
  size_t totalBytes = 0;
  for (const auto& f : ctx_->written()) {
    EXPECT_EQ(f.streamId, 1);
    EXPECT_EQ(f.frameType, FrameType::PAYLOAD);
    totalBytes += f.dataSize;
  }
  EXPECT_EQ(totalBytes, 200 * 1024);
}

// ============================================================================
// Error Propagation Tests (downstream Result::Error)
// ============================================================================

TEST_F(FrameFragmentationHandlerTest, AsyncFlushStopsImmediateQueueOnError) {
  // Forces several small frames into immediateQueue_ behind a fragmented
  // stream, then errors downstream after the first write. The Step 1 loop
  // must stop after the failed write, and the async-discard path must
  // surface a single inbound fireException.
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(10, 192 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 2 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 2 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(3, 2 * 1024));
  ASSERT_EQ(handler.immediateQueueSize(), 3);

  ctx_->setErrorAt(1);
  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 1);
  EXPECT_EQ(ctx_->fireExceptionCount(), 1);
  EXPECT_NE(
      std::string(ctx_->lastException().what())
          .find("FrameFragmentationHandler: downstream write failed"),
      std::string::npos);
}

TEST_F(FrameFragmentationHandlerTest, AsyncFlushStopsSrptLoopOnError) {
  // Two streams in the SRPT heap; downstream errors after two writes. The
  // Step 2 SRPT loop must stop and the async-discard path must fire one
  // inbound exception.
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(2, 192 * 1024));

  ctx_->setErrorAt(2);
  runEventBaseLoop();

  EXPECT_EQ(ctx_->written().size(), 2);
  EXPECT_EQ(ctx_->fireExceptionCount(), 1);
  EXPECT_NE(
      std::string(ctx_->lastException().what())
          .find("FrameFragmentationHandler: downstream write failed"),
      std::string::npos);
}

TEST_F(
    FrameFragmentationHandlerTest,
    OnWriteReadyFiresExceptionOnDownstreamError) {
  // Backpressure, then resume via onWriteReady with downstream returning
  // Error. The onWriteReady async-discard path must fire inbound.
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));

  ctx_->setBackpressureAt(1);
  runEventBaseLoop();
  ASSERT_TRUE(ctx_->awaitWriteReadyCalled());
  ASSERT_EQ(ctx_->fireExceptionCount(), 0);

  ctx_->setBackpressureAt(0);
  ctx_->setErrorAt(ctx_->written().size());
  handler.onWriteReady(*ctx_);

  EXPECT_EQ(ctx_->fireExceptionCount(), 1);
  EXPECT_NE(
      std::string(ctx_->lastException().what())
          .find("FrameFragmentationHandler: downstream write failed"),
      std::string::npos);
}

TEST_F(FrameFragmentationHandlerTest, SyncThresholdFlushDoesNotFireException) {
  // onWrite triggers a sync flushNow via the maxPendingBytes threshold.
  // The caller (channel boundary) owns escalation on the sync path, so
  // onWrite must return Result::Error WITHOUT firing fireException — firing
  // both would double-fire at the channel.
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = 128 * 1024,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(*ctx_);

  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 128 * 1024));
  ASSERT_EQ(ctx_->written().size(), 0);

  ctx_->setErrorAt(0);

  auto result = handler.onWrite(*ctx_, makePayloadFrame(2, 192 * 1024));
  EXPECT_EQ(
      result, apache::thrift::fast_thrift::channel_pipeline::Result::Error);
  EXPECT_EQ(ctx_->fireExceptionCount(), 0);
}

// ============================================================================
// Tracker wiring — FrameFragmentationHandlerT drives the tracker from doFlush.
// ============================================================================

TEST_F(
    FrameFragmentationHandlerTest, TrackerSeesEachFragmentWithFrameDoneFlag) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandlerT<RecordingTracker> handler(config);
  handler.handlerAdded(*ctx_);

  // 192KB -> 3 fragments; only the last completes the original frame.
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  runEventBaseLoop();

  const auto& frags = handler.tracker().fragments;
  ASSERT_EQ(frags.size(), 3u);
  EXPECT_EQ(frags[0].streamId, 1u);
  EXPECT_FALSE(frags[0].isLastFragment);
  EXPECT_FALSE(frags[1].isLastFragment);
  EXPECT_TRUE(frags[2].isLastFragment);
  EXPECT_EQ(handler.tracker().flushCount, 1u);
}

TEST_F(
    FrameFragmentationHandlerTest, TrackerSeesImmediateQueueFrameAsComplete) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandlerT<RecordingTracker> handler(config);
  handler.handlerAdded(*ctx_);

  // Large frame parks a pending stream; the smaller frame lands in the
  // immediate queue and drains first as a single complete frame (isLast=true).
  (void)handler.onWrite(*ctx_, makePayloadFrame(10, 192 * 1024));
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 32 * 1024));
  ASSERT_EQ(handler.immediateQueueSize(), 1u);

  runEventBaseLoop();

  const auto& frags = handler.tracker().fragments;
  ASSERT_EQ(frags.size(), 4u);
  // Immediate queue drains first: one complete frame for stream 1.
  EXPECT_EQ(frags[0].streamId, 1u);
  EXPECT_TRUE(frags[0].isLastFragment);
  // Then the 3 SRPT fragments of stream 10.
  EXPECT_EQ(frags[1].streamId, 10u);
  EXPECT_FALSE(frags[1].isLastFragment);
  EXPECT_FALSE(frags[2].isLastFragment);
  EXPECT_TRUE(frags[3].isLastFragment);
  EXPECT_EQ(handler.tracker().flushCount, 1u);
}

TEST_F(
    FrameFragmentationHandlerTest, TrackerSkipsErroredFragmentAndDoesNotFlush) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandlerT<RecordingTracker> handler(config);
  handler.handlerAdded(*ctx_);

  // 192KB -> 3 fragments; downstream errors on the 3rd (frame-completing) one.
  // onFragment must NOT record the errored fragment, and onFlush must not run
  // on the error path.
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  ctx_->setErrorAt(2);
  runEventBaseLoop();

  const auto& frags = handler.tracker().fragments;
  ASSERT_EQ(frags.size(), 2u);
  EXPECT_FALSE(frags[0].isLastFragment);
  EXPECT_FALSE(frags[1].isLastFragment);
  EXPECT_EQ(handler.tracker().flushCount, 0u);
}

TEST_F(
    FrameFragmentationHandlerTest,
    TrackerCountsBackpressuredFragmentButDoesNotFlush) {
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandlerT<RecordingTracker> handler(config);
  handler.handlerAdded(*ctx_);

  // Backpressure on the 3rd write: the fragment was still handed downstream, so
  // onFragment records it (isLast=true), but the flush paused — onFlush must
  // not run. Contrast with the error path, which skips onFragment entirely.
  (void)handler.onWrite(*ctx_, makePayloadFrame(1, 192 * 1024));
  ctx_->setBackpressureAt(2);
  runEventBaseLoop();

  EXPECT_TRUE(ctx_->awaitWriteReadyCalled());
  const auto& frags = handler.tracker().fragments;
  ASSERT_EQ(frags.size(), 3u);
  EXPECT_TRUE(frags[2].isLastFragment);
  EXPECT_EQ(handler.tracker().flushCount, 0u);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write::handler
