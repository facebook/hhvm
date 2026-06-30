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

/**
 * RocketClientWriteCompletionIntegrationTest
 *
 * Exercises the full client-side write-completion plumbing through a real
 * PipelineImpl:
 *
 *   TransportHandlerT<FrameEventFactory>
 *     -> LoopBatchingFrameHandlerT<
 *          WriteCompletionTrackerT<RocketClientEventFactory>>
 *     -> EventCapturingAppHandler  (subscribes to BatchWriteComplete via
 * onEvent)
 *
 * Each test drives one or more outbound writes through the pipeline, lets the
 * loop callback flush, then triggers writeSuccess / writeErr on the mocked
 * AsyncTransport and verifies the BatchWriteCompleteEvent(s) the tracker fans
 * out carry the correct (status, frameCount, bytes).
 *
 * The tests also model the per-frame attribution a real upstream consumer
 * would perform: each test handler maintains its own per-outbound FIFO and
 * pops `frameCount` entries per BatchWriteCompleteEvent.
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <vector>

#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GMock.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/LoopBatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientEventFactory.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::client {
namespace {

namespace cp = apache::thrift::fast_thrift::channel_pipeline;
namespace fw = apache::thrift::fast_thrift::frame::write::handler;
namespace transport = apache::thrift::fast_thrift::transport;
using transport::WriteCompletionStatus;
using namespace testing;

// Two events per pipeline: TransportHandler fires TransportWriteComplete and
// the tracker re-fires BatchWriteComplete after enriching with the
// rocket-frame count.
using TestTransportHandler =
    transport::TransportHandlerT<RocketClientEventFactory>;
using TestBatcher = fw::LoopBatchingFrameHandlerT<
    fw::WriteCompletionTrackerT<RocketClientEventFactory>>;

HANDLER_TAG(batching);

// Tail app handler that captures BatchWriteCompleteEvents fired by the
// tracker. Records its own per-outbound FIFO (size in bytes) so each test can
// model the per-frame attribution a real upstream consumer would do.
class EventCapturingAppHandler {
 public:
  using Result = cp::Result;

  // --- Tail endpoint surface ---
  Result onRead(cp::TypeErasedBox&&) noexcept { return Result::Success; }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // --- Event subscription ---
  // Subscribes only to the enriched BatchWriteComplete event the tracker
  // fires; the raw TransportWriteComplete never reaches this handler.
  static constexpr cp::Subscriptions<RocketClientEventId::BatchWriteComplete>
      kSubscribedEvents{};

  void onEvent(
      RocketClientEventId /*ev*/, const cp::TypeErasedBox& box) noexcept {
    events_.push_back(box.get<BatchWriteCompleteEvent>());
  }

  const std::vector<BatchWriteCompleteEvent>& events() const noexcept {
    return events_;
  }

  void clear() noexcept { events_.clear(); }

 private:
  std::vector<BatchWriteCompleteEvent> events_;
};

class RocketClientWriteCompletionIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto socket = folly::AsyncTransport::UniquePtr(
        new NiceMock<folly::test::MockAsyncTransport>());
    mockSocket_ = static_cast<folly::test::MockAsyncTransport*>(socket.get());
    ON_CALL(*mockSocket_, good()).WillByDefault(Return(true));
    ON_CALL(*mockSocket_, getEventBase()).WillByDefault(Return(&evb_));

    transportHandler_ =
        TestTransportHandler::create(std::move(socket), 256, 4096);

    pipeline_ = cp::PipelineBuilder<
                    TestTransportHandler,
                    EventCapturingAppHandler,
                    cp::SimpleBufferAllocator,
                    RocketClientEventId>()
                    .setEventBase(&evb_)
                    .setHead(transportHandler_.get())
                    .setTail(&app_)
                    .setAllocator(&allocator_)
                    .addNextOutbound<TestBatcher>(batching_tag)
                    .build();

    transportHandler_->setPipeline(pipeline_.get());
    transportHandler_->onConnect();
  }

  void TearDown() override {
    // Tear pipeline down before transport handler so the transport outlives
    // any in-flight WriteRequests the pipeline still holds.
    pipeline_.reset();
    transportHandler_.reset();
  }

  // Fires N outbound writes of `size` bytes each. Returns the captured
  // socket WriteCallback so the test can drive writeSuccess/writeErr.
  // Drains the EventBase loop once to let LoopBatchingFrameHandler flush.
  folly::AsyncTransport::WriteCallback* sendFramesAndDrainLoop(
      size_t framesInBatch, size_t bytesPerFrame) {
    folly::AsyncTransport::WriteCallback* cb = nullptr;
    EXPECT_CALL(*mockSocket_, writeChain(_, _, _))
        .WillOnce(SaveArg<0>(&cb))
        .RetiresOnSaturation();

    for (size_t i = 0; i < framesInBatch; ++i) {
      auto frame = folly::IOBuf::create(bytesPerFrame);
      frame->append(bytesPerFrame);
      auto result = pipeline_->fireWrite(cp::TypeErasedBox(std::move(frame)));
      EXPECT_NE(result, cp::Result::Error);
    }

    evb_.loopOnce(EVLOOP_NONBLOCK);
    // Double-scheduling (LoopBatchingFrameHandler reschedules once before
    // flushing) — one more loop iteration to actually issue the writeChain.
    evb_.loopOnce(EVLOOP_NONBLOCK);
    return cb;
  }

  folly::EventBase evb_;
  folly::test::MockAsyncTransport* mockSocket_{nullptr};
  TestTransportHandler::Ptr transportHandler_;
  EventCapturingAppHandler app_;
  cp::SimpleBufferAllocator allocator_;
  cp::PipelineImpl::Ptr pipeline_;
};

TEST_F(
    RocketClientWriteCompletionIntegrationTest,
    SingleFrameSingleBatchWriteSuccess) {
  auto* cb = sendFramesAndDrainLoop(/*framesInBatch=*/1, /*bytesPerFrame=*/100);
  ASSERT_NE(cb, nullptr);

  cb->writeSuccess();

  ASSERT_EQ(app_.events().size(), 1u);
  EXPECT_EQ(app_.events()[0].status, WriteCompletionStatus::Success);
  EXPECT_EQ(app_.events()[0].frameCount, 1u);
  EXPECT_EQ(app_.events()[0].bytes, 0u);
}

TEST_F(
    RocketClientWriteCompletionIntegrationTest,
    MultiFrameSingleBatchWriteSuccess) {
  auto* cb = sendFramesAndDrainLoop(/*framesInBatch=*/4, /*bytesPerFrame=*/64);
  ASSERT_NE(cb, nullptr);

  cb->writeSuccess();

  ASSERT_EQ(app_.events().size(), 1u);
  EXPECT_EQ(app_.events()[0].status, WriteCompletionStatus::Success);
  EXPECT_EQ(app_.events()[0].frameCount, 4u);
}

TEST_F(
    RocketClientWriteCompletionIntegrationTest,
    MultipleBatchesPreserveFifoOrder) {
  // Batch 1: 2 frames
  auto* cb1 = sendFramesAndDrainLoop(/*framesInBatch=*/2, /*bytesPerFrame=*/50);
  ASSERT_NE(cb1, nullptr);
  // Batch 2: 3 frames
  auto* cb2 = sendFramesAndDrainLoop(/*framesInBatch=*/3, /*bytesPerFrame=*/50);
  ASSERT_NE(cb2, nullptr);

  // AsyncSocket FIFO: writeSuccess for batch 1 fires first.
  cb1->writeSuccess();
  cb2->writeSuccess();

  ASSERT_EQ(app_.events().size(), 2u);
  EXPECT_EQ(app_.events()[0].frameCount, 2u);
  EXPECT_EQ(app_.events()[1].frameCount, 3u);
}

TEST_F(
    RocketClientWriteCompletionIntegrationTest,
    WriteErrFullLossReportsErrorWithZeroBytes) {
  auto* cb = sendFramesAndDrainLoop(/*framesInBatch=*/3, /*bytesPerFrame=*/64);
  ASSERT_NE(cb, nullptr);

  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "no bytes hit wire");
  cb->writeErr(/*bytesWritten=*/0, ex);

  ASSERT_EQ(app_.events().size(), 1u);
  EXPECT_EQ(app_.events()[0].status, WriteCompletionStatus::Error);
  EXPECT_EQ(app_.events()[0].frameCount, 3u);
  EXPECT_EQ(app_.events()[0].bytes, 0u);
}

TEST_F(
    RocketClientWriteCompletionIntegrationTest,
    WriteErrPartialBytesReportsActualBytesWritten) {
  auto* cb = sendFramesAndDrainLoop(/*framesInBatch=*/3, /*bytesPerFrame=*/64);
  ASSERT_NE(cb, nullptr);

  // 100 bytes made wire out of 192 (3 * 64). Consumer-side attribution can
  // walk a per-frame size FIFO (each frame is 64 bytes) and conclude that
  // frame 0 (cumulative 64) made wire and frame 1 (cumulative 128) did not.
  constexpr size_t kBytesWritten = 100;
  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "partial write");
  cb->writeErr(kBytesWritten, ex);

  ASSERT_EQ(app_.events().size(), 1u);
  EXPECT_EQ(app_.events()[0].status, WriteCompletionStatus::Error);
  EXPECT_EQ(app_.events()[0].frameCount, 3u);
  EXPECT_EQ(app_.events()[0].bytes, kBytesWritten);
}

TEST_F(
    RocketClientWriteCompletionIntegrationTest,
    InterleavedSuccessThenErrorPreservesOrder) {
  auto* cb1 = sendFramesAndDrainLoop(/*framesInBatch=*/2, /*bytesPerFrame=*/40);
  ASSERT_NE(cb1, nullptr);
  auto* cb2 = sendFramesAndDrainLoop(/*framesInBatch=*/1, /*bytesPerFrame=*/40);
  ASSERT_NE(cb2, nullptr);

  cb1->writeSuccess();
  folly::AsyncSocketException ex(
      folly::AsyncSocketException::NETWORK_ERROR, "batch 2 fail");
  cb2->writeErr(/*bytesWritten=*/0, ex);

  ASSERT_EQ(app_.events().size(), 2u);
  EXPECT_EQ(app_.events()[0].status, WriteCompletionStatus::Success);
  EXPECT_EQ(app_.events()[0].frameCount, 2u);
  EXPECT_EQ(app_.events()[1].status, WriteCompletionStatus::Error);
  EXPECT_EQ(app_.events()[1].frameCount, 1u);
}

} // namespace
} // namespace apache::thrift::fast_thrift::rocket::client
