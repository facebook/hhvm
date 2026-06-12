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
 * RocketServerWriteCompletionIntegrationTest
 *
 * Mirror of the client-side integration test, using
 * IntervalBatchingFrameHandler (the server's chosen batcher) under the default
 * config.
 *
 *   TransportHandlerT<RocketServerEventFactory>
 *     -> IntervalBatchingFrameHandlerT<
 *          WriteCompletionTrackerT<RocketServerEventFactory>>
 *     -> EventCapturingAppHandler  (subscribes to RocketWriteComplete via
 * onEvent)
 *
 * Each test drives outbound writes, lets the loop / interval timer flush,
 * then triggers writeSuccess / writeErr on the mocked AsyncTransport and
 * verifies the RocketWriteCompleteEvent(s) the tracker fans out carry the
 * correct (status, frameCount, bytes). The app handler subscribes only to the
 * tracker-fired RocketWriteComplete; the raw transport TransportWriteComplete
 * never reaches it.
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
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/IntervalBatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/WriteCompletionTracker.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/RocketServerEventFactory.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::rocket::server {
namespace {

namespace cp = apache::thrift::fast_thrift::channel_pipeline;
namespace fw = apache::thrift::fast_thrift::frame::write::handler;
namespace transport = apache::thrift::fast_thrift::transport;
using transport::WriteCompletionStatus;
using namespace testing;

using TestTransportHandler =
    transport::TransportHandlerT<RocketServerEventFactory>;
using TestBatcher = fw::IntervalBatchingFrameHandlerT<
    fw::WriteCompletionTrackerT<RocketServerEventFactory>>;

HANDLER_TAG(batching);

class EventCapturingAppHandler {
 public:
  using Result = cp::Result;

  Result onRead(cp::TypeErasedBox&&) noexcept { return Result::Success; }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // Subscribes only to the enriched RocketWriteComplete event the tracker
  // fires; the raw TransportWriteComplete never reaches this handler.
  static constexpr std::array<RocketServerEventId, 1> kSubscribedEvents{
      RocketServerEventId::RocketWriteComplete};

  void onEvent(
      RocketServerEventId /*ev*/, const cp::TypeErasedBox& box) noexcept {
    events_.push_back(box.get<RocketWriteCompleteEvent>());
  }

  const std::vector<RocketWriteCompleteEvent>& events() const noexcept {
    return events_;
  }

 private:
  std::vector<RocketWriteCompleteEvent> events_;
};

class RocketServerWriteCompletionIntegrationTest : public ::testing::Test {
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
                    RocketServerEventId>()
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
    pipeline_.reset();
    transportHandler_.reset();
  }

  // Fires N outbound writes of `size` bytes each. Returns the captured
  // socket WriteCallback. IntervalBatchingFrameHandler default config:
  // batchingInterval = 0 (loop-only), so one loop iteration flushes.
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

    // IntervalBatchingFrameHandler default config uses runInLoop; drain
    // until quiescent.
    evb_.loop();
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
    RocketServerWriteCompletionIntegrationTest,
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
    RocketServerWriteCompletionIntegrationTest,
    MultiFrameSingleBatchWriteSuccess) {
  auto* cb = sendFramesAndDrainLoop(/*framesInBatch=*/4, /*bytesPerFrame=*/64);
  ASSERT_NE(cb, nullptr);

  cb->writeSuccess();

  ASSERT_EQ(app_.events().size(), 1u);
  EXPECT_EQ(app_.events()[0].status, WriteCompletionStatus::Success);
  EXPECT_EQ(app_.events()[0].frameCount, 4u);
}

TEST_F(
    RocketServerWriteCompletionIntegrationTest, SerialBatchesEachFireOwnEvent) {
  // IntervalBatchingFrameHandler is backpressure-aware: it serializes
  // writevs (only one in-flight at a time, gated on onWriteReady). The
  // test interleaves writeSuccess between batches to release backpressure.
  auto* cb1 = sendFramesAndDrainLoop(/*framesInBatch=*/2, /*bytesPerFrame=*/50);
  ASSERT_NE(cb1, nullptr);
  cb1->writeSuccess();
  evb_.loopOnce(EVLOOP_NONBLOCK);

  auto* cb2 = sendFramesAndDrainLoop(/*framesInBatch=*/3, /*bytesPerFrame=*/50);
  ASSERT_NE(cb2, nullptr);
  cb2->writeSuccess();

  ASSERT_EQ(app_.events().size(), 2u);
  EXPECT_EQ(app_.events()[0].frameCount, 2u);
  EXPECT_EQ(app_.events()[1].frameCount, 3u);
}

TEST_F(
    RocketServerWriteCompletionIntegrationTest,
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
    RocketServerWriteCompletionIntegrationTest,
    WriteErrPartialBytesReportsActualBytesWritten) {
  auto* cb = sendFramesAndDrainLoop(/*framesInBatch=*/3, /*bytesPerFrame=*/64);
  ASSERT_NE(cb, nullptr);

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
    RocketServerWriteCompletionIntegrationTest,
    SerialSuccessThenErrorPreservesOrder) {
  // Interleave writeSuccess between the two batches so the second batch is
  // not blocked by backpressure (IntervalBatchingFrameHandler is
  // single-in-flight).
  auto* cb1 = sendFramesAndDrainLoop(/*framesInBatch=*/2, /*bytesPerFrame=*/40);
  ASSERT_NE(cb1, nullptr);
  cb1->writeSuccess();
  evb_.loopOnce(EVLOOP_NONBLOCK);

  auto* cb2 = sendFramesAndDrainLoop(/*framesInBatch=*/1, /*bytesPerFrame=*/40);
  ASSERT_NE(cb2, nullptr);
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
} // namespace apache::thrift::fast_thrift::rocket::server
