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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/WriteBufferBackpressureHandler.h>

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

using channel_pipeline::erase_and_box;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;

struct FakePipeline {
  size_t onReadReadyCalls{0};
  void onReadReady() noexcept { ++onReadReadyCalls; }
};

class FakeContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    reads.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    auto& resp = msg.get<ThriftServerResponseMessage>();
    if (resp.payload.template is<ThriftInitialResponsePayload>()) {
      writtenStreamIds.push_back(
          resp.payload.template get<ThriftInitialResponsePayload>().streamId);
    } else {
      writtenStreamIds.push_back(
          resp.payload.template get<ThriftErrorPayload>().streamId);
    }
    writes.push_back(std::move(msg));
    return nextWriteResult;
  }

  // Unused by these tests but kept so FakeContext remains a complete
  // stand-in for the ContextApi concept.
  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  void fireException(folly::exception_wrapper&& e) noexcept {
    exceptions.push_back(std::move(e));
  }

  // Backpressure / ready-signal tracking.
  void awaitWriteReady() noexcept {
    ++awaitWriteReadyCalls;
    awaitingWriteReady = true;
  }
  void cancelAwaitWriteReady() noexcept {
    ++cancelAwaitWriteReadyCalls;
    awaitingWriteReady = false;
  }
  FakePipeline* pipeline() noexcept { return &fakePipeline; }

  Result nextWriteResult{Result::Success};
  std::vector<TypeErasedBox> reads;
  std::vector<TypeErasedBox> writes;
  std::vector<uint32_t> writtenStreamIds;
  std::vector<folly::exception_wrapper> exceptions;

  size_t awaitWriteReadyCalls{0};
  size_t cancelAwaitWriteReadyCalls{0};
  bool awaitingWriteReady{false};
  FakePipeline fakePipeline;
};

ThriftServerResponseMessage makeResponse(uint32_t streamId) {
  return ThriftServerResponseMessage{
      .payload = ThriftInitialResponsePayload{
          .data = nullptr,
          .metadata = nullptr,
          .streamId = streamId,
      }};
}

} // namespace

// =============================================================================
// Pass-through
// =============================================================================

TEST(WriteBufferBackpressureHandlerTest, OnWriteForwardsWhenUnsaturated) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onWrite(ctx, erase_and_box(makeResponse(1))), Result::Success);
  EXPECT_EQ(ctx.writes.size(), 1u);
  EXPECT_FALSE(handler.isBackpressured());
  EXPECT_EQ(handler.pendingResponseCount(), 0u);
}

// =============================================================================
// Backpressure flag arms / buffers
// =============================================================================

TEST(WriteBufferBackpressureHandlerTest, BackpressureFromDownstreamArmsFlag) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  // First write goes through but downstream signals "I'm full now."
  EXPECT_EQ(
      handler.onWrite(ctx, erase_and_box(makeResponse(1))),
      Result::Backpressure);
  EXPECT_EQ(ctx.writes.size(), 1u);
  EXPECT_TRUE(handler.isBackpressured());
  EXPECT_EQ(handler.pendingResponseCount(), 0u);
}

TEST(WriteBufferBackpressureHandlerTest, ResponsesBufferWhileBackpressured) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  ASSERT_EQ(
      handler.onWrite(ctx, erase_and_box(makeResponse(1))),
      Result::Backpressure);
  ASSERT_TRUE(handler.isBackpressured());
  EXPECT_EQ(ctx.writes.size(), 1u);

  // Subsequent writes buffer (no downstream traffic) and surface Success.
  EXPECT_EQ(
      handler.onWrite(ctx, erase_and_box(makeResponse(2))), Result::Success);
  EXPECT_EQ(
      handler.onWrite(ctx, erase_and_box(makeResponse(3))), Result::Success);

  EXPECT_EQ(ctx.writes.size(), 1u)
      << "No new downstream writes while backpressured";
  EXPECT_EQ(handler.pendingResponseCount(), 2u);
}

// =============================================================================
// onWriteReady drain
// =============================================================================

TEST(
    WriteBufferBackpressureHandlerTest,
    OnWriteReadyDrainsBufferedResponsesInOrder) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  // Saturate, queue 1, 2, 3.
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(3)));
  ASSERT_EQ(handler.pendingResponseCount(), 2u);
  ASSERT_EQ(ctx.writes.size(), 1u);

  // Downstream signals room; drain should be FIFO.
  ctx.nextWriteResult = Result::Success;
  handler.onWriteReady(ctx);

  const std::vector<uint32_t> expected{1u, 2u, 3u};
  EXPECT_EQ(ctx.writtenStreamIds, expected);
  EXPECT_FALSE(handler.isBackpressured());
  EXPECT_EQ(handler.pendingResponseCount(), 0u);
}

TEST(
    WriteBufferBackpressureHandlerTest,
    OnWriteReadyHaltsAndRearmsOnBackpressure) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  // Saturate + queue two behind.
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(3)));
  ASSERT_EQ(handler.pendingResponseCount(), 2u);

  // Downstream still says Backpressure on the very first drained write.
  // Handler attempts one, re-arms the flag, and leaves the remainder.
  handler.onWriteReady(ctx);

  EXPECT_TRUE(handler.isBackpressured());
  EXPECT_EQ(handler.pendingResponseCount(), 1u)
      << "Second queued response must remain after BP re-assertion";
}

// =============================================================================
// onPipelineInactive
// =============================================================================

TEST(WriteBufferBackpressureHandlerTest, OnPipelineInactiveDropsBuffer) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(3)));
  ASSERT_EQ(handler.pendingResponseCount(), 2u);
  ASSERT_TRUE(handler.isBackpressured());

  handler.onPipelineInactive(ctx);

  EXPECT_EQ(handler.pendingResponseCount(), 0u);
  EXPECT_FALSE(handler.isBackpressured());
}

// =============================================================================
// Inbound: surfaces Backpressure while outbound is saturated
// =============================================================================

TEST(WriteBufferBackpressureHandlerTest, OnReadPassesThroughWhenUnsaturated) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;

  EXPECT_EQ(
      handler.onRead(ctx, erase_and_box(ThriftServerRequestMessage{})),
      Result::Success);
  EXPECT_EQ(ctx.reads.size(), 1u);
}

TEST(
    WriteBufferBackpressureHandlerTest,
    OnReadReturnsBackpressureWhileBuffered) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;

  // Arm the backpressure flag via an outbound saturating write.
  ctx.nextWriteResult = Result::Backpressure;
  ASSERT_EQ(
      handler.onWrite(ctx, erase_and_box(makeResponse(1))),
      Result::Backpressure);
  ASSERT_TRUE(handler.isBackpressured());

  // Read still forwards downstream but surfaces Backpressure upstream so
  // the transport pauses socket reads.
  ctx.nextWriteResult = Result::Success; // (only affects writes)
  EXPECT_EQ(
      handler.onRead(ctx, erase_and_box(ThriftServerRequestMessage{})),
      Result::Backpressure);
  EXPECT_EQ(ctx.reads.size(), 1u) << "Read must still propagate downstream";
}

TEST(WriteBufferBackpressureHandlerTest, OnReadRestoresPassThroughAfterDrain) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;

  // Arm + queue, then drain.
  ctx.nextWriteResult = Result::Backpressure;
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  ctx.nextWriteResult = Result::Success;
  handler.onWriteReady(ctx);
  ASSERT_FALSE(handler.isBackpressured());

  EXPECT_EQ(
      handler.onRead(ctx, erase_and_box(ThriftServerRequestMessage{})),
      Result::Success);
}

// =============================================================================
// Write-ready registration: handler must register with the pipeline so that
// onWriteReady() actually fires when downstream drains.
// =============================================================================

TEST(
    WriteBufferBackpressureHandlerTest,
    OnWriteBackpressureRegistersForWriteReady) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));

  EXPECT_EQ(ctx.awaitWriteReadyCalls, 1u);
  EXPECT_TRUE(ctx.awaitingWriteReady);
}

TEST(
    WriteBufferBackpressureHandlerTest,
    OnWriteBufferedBranchDoesNotReRegister) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  // Arm.
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  ASSERT_EQ(ctx.awaitWriteReadyCalls, 1u);

  // Subsequent buffered writes are no-ops for registration — already armed.
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(3)));

  EXPECT_EQ(ctx.awaitWriteReadyCalls, 1u);
}

// =============================================================================
// Full drain releases the write-ready registration AND resumes upstream reads.
// =============================================================================

TEST(
    WriteBufferBackpressureHandlerTest,
    OnWriteReadyFullDrainCancelsAwaitAndNotifiesReadReady) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  ASSERT_TRUE(ctx.awaitingWriteReady);

  ctx.nextWriteResult = Result::Success;
  handler.onWriteReady(ctx);

  ASSERT_FALSE(handler.isBackpressured());
  EXPECT_EQ(ctx.cancelAwaitWriteReadyCalls, 1u);
  EXPECT_FALSE(ctx.awaitingWriteReady);
  // The user-reported bug: full drain must resume the head's reads.
  EXPECT_EQ(ctx.fakePipeline.onReadReadyCalls, 1u);
}

TEST(
    WriteBufferBackpressureHandlerTest,
    OnWriteReadyPartialDrainKeepsAwaitAndDoesNotNotifyReadReady) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(2)));
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(3)));
  ASSERT_TRUE(ctx.awaitingWriteReady);

  // Downstream re-asserts backpressure on the first drained write.
  handler.onWriteReady(ctx);

  EXPECT_TRUE(handler.isBackpressured());
  // Still armed — handler must remain on the writeReadyList for the next
  // onWriteReady cycle.
  EXPECT_EQ(ctx.cancelAwaitWriteReadyCalls, 0u);
  EXPECT_TRUE(ctx.awaitingWriteReady);
  // Reads must stay paused until the buffer fully drains.
  EXPECT_EQ(ctx.fakePipeline.onReadReadyCalls, 0u);
}

// =============================================================================
// Pipeline teardown clears the write-ready registration.
// =============================================================================

TEST(
    WriteBufferBackpressureHandlerTest,
    OnPipelineInactiveCancelsAwaitWriteReady) {
  WriteBufferBackpressureHandler<FakeContext> handler;
  FakeContext ctx;
  ctx.nextWriteResult = Result::Backpressure;

  (void)handler.onWrite(ctx, erase_and_box(makeResponse(1)));
  ASSERT_TRUE(ctx.awaitingWriteReady);

  handler.onPipelineInactive(ctx);

  EXPECT_FALSE(ctx.awaitingWriteReady);
  EXPECT_EQ(ctx.cancelAwaitWriteReadyCalls, 1u);
}

// =============================================================================
// Integration tests — exercise the handler embedded in a real PipelineImpl
// built via PipelineBuilder. The FakeContext-based tests above can only
// verify which methods the handler calls; they cannot catch wiring bugs
// such as a missing writeReadyHook_ member (which would leave the handler
// off the pipeline's writeReadyList so onWriteReady never fires).
// =============================================================================

namespace cp = channel_pipeline;

namespace {

HANDLER_TAG(middle);

class WriteBufferBackpressureHandlerPipelineTest : public ::testing::Test {
 protected:
  using Handler = WriteBufferBackpressureHandler<cp::detail::ContextImpl>;

  cp::PipelineImpl::Ptr buildPipeline(std::unique_ptr<Handler> handler) {
    return cp::PipelineBuilder<
               cp::test::MockHeadHandler,
               cp::test::MockTailHandler,
               cp::test::TestAllocator>()
        .setEventBase(&evb_)
        .setHead(&head_)
        .setTail(&tail_)
        .setAllocator(&allocator_)
        .addNextDuplex<Handler>(middle_tag, std::move(handler))
        .build();
  }

  folly::EventBase evb_;
  cp::test::MockHeadHandler head_;
  cp::test::MockTailHandler tail_;
  cp::test::TestAllocator allocator_;
};

} // namespace

// Original bug: handler was missing writeReadyHook_, so it was never on
// the pipeline's writeReadyList. Direct-call unit tests couldn't catch
// this — only a real pipeline reveals it.
TEST_F(
    WriteBufferBackpressureHandlerPipelineTest,
    HandlerRegistersOnPipelineWriteReadyList) {
  auto handler = std::make_unique<Handler>();
  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });

  auto pipeline = buildPipeline(std::move(handler));

  ASSERT_FALSE(pipeline->hasPendingWriteReady());

  auto result =
      pipeline->fireWrite(cp::erase_and_box(makeResponse(/*streamId=*/1)));

  EXPECT_EQ(result, cp::Result::Backpressure);
  EXPECT_TRUE(pipeline->hasPendingWriteReady())
      << "Handler must be on pipeline writeReadyList so onWriteReady fires "
         "when downstream drains";
}

// User-reported bug: full drain must resume the head's paused reads.
TEST_F(
    WriteBufferBackpressureHandlerPipelineTest,
    OnWriteReadyFullDrainsAndResumesHeadReads) {
  auto handler = std::make_unique<Handler>();
  auto* handlerPtr = handler.get();

  // Phase 1: head saturated — buffer fills.
  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });

  auto pipeline = buildPipeline(std::move(handler));

  (void)pipeline->fireWrite(cp::erase_and_box(makeResponse(1)));
  (void)pipeline->fireWrite(cp::erase_and_box(makeResponse(2)));
  (void)pipeline->fireWrite(cp::erase_and_box(makeResponse(3)));

  ASSERT_TRUE(handlerPtr->isBackpressured());
  ASSERT_EQ(handlerPtr->pendingResponseCount(), 2u)
      << "First write reached head; 2 + 3 buffered";
  ASSERT_TRUE(pipeline->hasPendingWriteReady());
  ASSERT_EQ(head_.onReadReadyCount(), 0)
      << "Reads must stay paused while buffer is non-empty";

  // Phase 2: head drains — pipeline fires onWriteReady which walks the
  // writeReadyList. If the handler isn't registered, drain never happens.
  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Success; });
  pipeline->onWriteReady();

  EXPECT_FALSE(handlerPtr->isBackpressured());
  EXPECT_EQ(handlerPtr->pendingResponseCount(), 0u);
  EXPECT_FALSE(pipeline->hasPendingWriteReady())
      << "Full drain must cancelAwaitWriteReady";
  EXPECT_EQ(head_.onReadReadyCount(), 1)
      << "Full drain must resume head reads (the user-reported bug)";
  EXPECT_EQ(head_.writeCount(), 3);
}

// Partial drain — buffer not empty — must keep backpressure and must NOT
// resume reads.
TEST_F(
    WriteBufferBackpressureHandlerPipelineTest,
    OnWriteReadyPartialDrainKeepsBackpressureAndDoesNotResumeReads) {
  auto handler = std::make_unique<Handler>();
  auto* handlerPtr = handler.get();

  head_.setOnWriteCallback(
      [](cp::TypeErasedBox&&) noexcept { return cp::Result::Backpressure; });

  auto pipeline = buildPipeline(std::move(handler));

  (void)pipeline->fireWrite(cp::erase_and_box(makeResponse(1)));
  (void)pipeline->fireWrite(cp::erase_and_box(makeResponse(2)));
  (void)pipeline->fireWrite(cp::erase_and_box(makeResponse(3)));
  ASSERT_EQ(handlerPtr->pendingResponseCount(), 2u);

  // Head re-asserts backpressure on the very first drained write.
  pipeline->onWriteReady();

  EXPECT_TRUE(handlerPtr->isBackpressured());
  EXPECT_EQ(handlerPtr->pendingResponseCount(), 1u);
  EXPECT_TRUE(pipeline->hasPendingWriteReady())
      << "Still armed for the next drain cycle";
  EXPECT_EQ(head_.onReadReadyCount(), 0)
      << "Reads must stay paused while buffer is non-empty";
}

} // namespace apache::thrift::fast_thrift::thrift
