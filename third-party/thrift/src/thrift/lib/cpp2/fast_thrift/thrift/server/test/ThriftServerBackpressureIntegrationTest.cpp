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
 * ThriftServerBackpressureIntegrationTest — exercises the full production
 * thrift-server handler chain end-to-end under simulated socket
 * backpressure.
 *
 * Pipeline shape (matches ThriftServerConnectionFactory):
 *
 *   MockHeadHandler (mock socket)
 *     -> ThriftServerRequestContextHandler
 *     -> ThriftServerConnectionContextHandler
 *     -> ThriftServerConnectionCloseHandler
 *     -> WriteBufferBackpressureHandler
 *     -> MockTailHandler (mock app)
 *
 * The mock head's onWrite can be flipped to return Result::Backpressure
 * to simulate a saturated socket. Tests then verify the full chain
 * propagates the signal: writes buffer in WriteBufferBackpressureHandler,
 * reads pause via the inbound Backpressure return, and on full drain the
 * head's onReadReady is invoked to resume reads.
 *
 * Unit tests with FakeContext cannot catch wiring bugs at this layer —
 * e.g. a missing writeReadyHook_ on WriteBufferBackpressureHandler (which
 * would leave it off the pipeline's writeReadyList), or a missing
 * ctx.pipeline()->onReadReady() on full drain.
 */

#include <chrono>
#include <memory>

#include <boost/intrusive_ptr.hpp>

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionCloseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerConnectionContextHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerRequestContextHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/WriteBufferBackpressureHandler.h>

namespace apache::thrift::fast_thrift::thrift::server::test {

namespace cp = ::apache::thrift::fast_thrift::channel_pipeline;
namespace ft = ::apache::thrift::fast_thrift::thrift;

namespace {

HANDLER_TAG(req_ctx);
HANDLER_TAG(conn_ctx);
HANDLER_TAG(close);
HANDLER_TAG(write_buffer);

using Ctx = cp::detail::ContextImpl;
using ReqCtxHandler = ft::ThriftServerRequestContextHandler<Ctx>;
using ConnCtxHandler = ft::ThriftServerConnectionContextHandler<Ctx>;
using CloseHandler = ft::ThriftServerConnectionCloseHandler<Ctx>;
using WriteBufferHandler = ft::WriteBufferBackpressureHandler<Ctx>;

ft::ThriftServerRequestMessage makeRequest(uint32_t streamId) {
  return ft::ThriftServerRequestMessage{
      .requestContext = nullptr, .payload = {}, .streamId = streamId};
}

ft::ThriftServerResponseMessage makeResponse(uint32_t streamId) {
  return ft::ThriftServerResponseMessage{
      .payload = ft::ThriftInitialResponsePayload{
          .data = nullptr,
          .metadata = nullptr,
          .streamId = streamId,
      }};
}

} // namespace

class ThriftServerBackpressureIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    head_.reset();
    tail_.reset();
    allocator_.reset();

    auto reqCtx = std::make_unique<ReqCtxHandler>();
    auto connCtx = std::make_unique<ConnCtxHandler>(
        boost::intrusive_ptr<ft::ThriftConnContext>(
            new ft::ThriftConnContext{}));
    auto closeHandler = std::make_unique<CloseHandler>();
    auto writeBuffer = std::make_unique<WriteBufferHandler>();

    closeHandler_ = closeHandler.get();
    writeBuffer_ = writeBuffer.get();

    pipeline_ =
        cp::PipelineBuilder<
            cp::test::MockHeadHandler,
            cp::test::MockTailHandler,
            cp::test::TestAllocator>()
            .setEventBase(&evb_)
            .setHead(&head_)
            .setTail(&tail_)
            .setAllocator(&allocator_)
            .addNextInbound<ReqCtxHandler>(req_ctx_tag, std::move(reqCtx))
            .addNextInbound<ConnCtxHandler>(conn_ctx_tag, std::move(connCtx))
            .addNextDuplex<CloseHandler>(close_tag, std::move(closeHandler))
            .addNextDuplex<WriteBufferHandler>(
                write_buffer_tag, std::move(writeBuffer))
            .build();
    pipeline_->activate();
  }

  void TearDown() override {
    // Drain any scheduled work before the pipeline goes away.
    evb_.loopOnce(EVLOOP_NONBLOCK);
    pipeline_.reset();
  }

  // Fire N requests through the inbound path so CloseHandler.inFlight == N.
  // Each request is a ThriftServerRequestMessage (what ReqCtxHandler expects).
  void fireRequests(uint32_t count) {
    for (uint32_t i = 1; i <= count; ++i) {
      (void)pipeline_->fireRead(cp::erase_and_box(makeRequest(i)));
    }
  }

  // Fire a response from the tail outward toward the head. Pairs with a
  // prior fireRequests() call so CloseHandler's inFlight invariant holds.
  cp::Result fireResponse(uint32_t streamId) {
    return pipeline_->fireWrite(cp::erase_and_box(makeResponse(streamId)));
  }

  folly::EventBase evb_;
  cp::test::MockHeadHandler head_;
  cp::test::MockTailHandler tail_;
  cp::test::TestAllocator allocator_;
  cp::PipelineImpl::Ptr pipeline_;
  CloseHandler* closeHandler_{nullptr};
  WriteBufferHandler* writeBuffer_{nullptr};
};

// =============================================================================
// Sanity: with no backpressure, writes flow straight through the chain.
// =============================================================================

TEST_F(
    ThriftServerBackpressureIntegrationTest,
    WritesPassThroughWhenHeadAcceptsAllWrites) {
  fireRequests(3);
  ASSERT_EQ(closeHandler_->inFlight(), 3u);
  ASSERT_EQ(tail_.readCount(), 3);

  // Head accepts; mock default callback expects BytesPtr, so install a
  // typed callback that just returns Success.
  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Success; });

  EXPECT_EQ(fireResponse(1), cp::Result::Success);
  EXPECT_EQ(fireResponse(2), cp::Result::Success);
  EXPECT_EQ(fireResponse(3), cp::Result::Success);

  EXPECT_FALSE(writeBuffer_->isBackpressured());
  EXPECT_EQ(writeBuffer_->pendingResponseCount(), 0u);
  EXPECT_EQ(head_.writeCount(), 3);
  EXPECT_EQ(closeHandler_->inFlight(), 0u);
}

// =============================================================================
// Head saturated → first write reaches head and arms backpressure; subsequent
// writes buffer in WriteBufferBackpressureHandler. CloseHandler invariant
// holds because buffered writes haven't been forwarded through it yet
// (inFlight only decrements on writes that actually traverse close).
// =============================================================================

TEST_F(
    ThriftServerBackpressureIntegrationTest,
    HeadBackpressureBuffersInWriteBufferHandler) {
  fireRequests(3);
  ASSERT_EQ(closeHandler_->inFlight(), 3u);

  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });

  EXPECT_EQ(fireResponse(1), cp::Result::Backpressure);
  EXPECT_EQ(fireResponse(2), cp::Result::Success); // queued
  EXPECT_EQ(fireResponse(3), cp::Result::Success); // queued

  EXPECT_TRUE(writeBuffer_->isBackpressured());
  EXPECT_EQ(writeBuffer_->pendingResponseCount(), 2u);
  EXPECT_EQ(head_.writeCount(), 1);
  // close-handler only sees the one write that reached the head.
  EXPECT_EQ(closeHandler_->inFlight(), 2u);
  // The handler must be registered on the pipeline's writeReadyList — if
  // writeReadyHook_ is missing this stays false (the original bug).
  EXPECT_TRUE(pipeline_->hasPendingWriteReady());
}

// =============================================================================
// User-reported bug: full drain must resume the head's paused reads.
// =============================================================================

TEST_F(
    ThriftServerBackpressureIntegrationTest,
    OnWriteReadyFullDrainsAndResumesHeadReads) {
  fireRequests(3);

  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });

  (void)fireResponse(1);
  (void)fireResponse(2);
  (void)fireResponse(3);
  ASSERT_EQ(writeBuffer_->pendingResponseCount(), 2u);
  ASSERT_TRUE(pipeline_->hasPendingWriteReady());
  ASSERT_EQ(head_.onReadReadyCount(), 0);

  // Head drains.
  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Success; });
  pipeline_->onWriteReady();

  EXPECT_FALSE(writeBuffer_->isBackpressured());
  EXPECT_EQ(writeBuffer_->pendingResponseCount(), 0u);
  EXPECT_FALSE(pipeline_->hasPendingWriteReady());
  EXPECT_EQ(head_.writeCount(), 3);
  // The user-reported bug — without ctx.pipeline()->onReadReady() on full
  // drain, the head's onReadReady is never invoked and reads stay paused.
  EXPECT_EQ(head_.onReadReadyCount(), 1);
  // All writes reached close-handler on the drain path; in-flight closed
  // out.
  EXPECT_EQ(closeHandler_->inFlight(), 0u);
}

// =============================================================================
// Partial drain — head still saturated on the first drained write. Buffer
// shrinks but doesn't empty; backpressure and read-pause must persist.
// =============================================================================

TEST_F(
    ThriftServerBackpressureIntegrationTest,
    OnWriteReadyPartialDrainKeepsBackpressureAndDoesNotResumeReads) {
  fireRequests(3);

  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });

  (void)fireResponse(1);
  (void)fireResponse(2);
  (void)fireResponse(3);
  ASSERT_EQ(writeBuffer_->pendingResponseCount(), 2u);

  // Head still saturated on the very first drained write.
  pipeline_->onWriteReady();

  EXPECT_TRUE(writeBuffer_->isBackpressured());
  EXPECT_EQ(writeBuffer_->pendingResponseCount(), 1u);
  EXPECT_TRUE(pipeline_->hasPendingWriteReady())
      << "Still armed for the next drain cycle";
  EXPECT_EQ(head_.onReadReadyCount(), 0)
      << "Reads must stay paused while buffer is non-empty";
}

// =============================================================================
// Read-side: while WriteBufferBackpressureHandler is armed, inbound reads
// must surface Backpressure all the way back to the head so the transport
// pauses socket reads. Validates that NO handler in the chain
// (ConnCtxHandler / ReqCtxHandler / CloseHandler) drops the Backpressure
// signal on its way upstream.
// =============================================================================

TEST_F(
    ThriftServerBackpressureIntegrationTest,
    InboundReadSurfacesBackpressureThroughEntireChain) {
  fireRequests(1);
  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });
  (void)fireResponse(1);
  ASSERT_TRUE(writeBuffer_->isBackpressured());

  // A new inbound read should propagate Backpressure all the way upstream.
  auto result = pipeline_->fireRead(cp::erase_and_box(makeRequest(99)));
  EXPECT_EQ(result, cp::Result::Backpressure);
  // The request still reaches the tail (WriteBufferBackpressureHandler
  // forwards before overriding the result) — verify it wasn't dropped.
  EXPECT_EQ(tail_.readCount(), 2);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
