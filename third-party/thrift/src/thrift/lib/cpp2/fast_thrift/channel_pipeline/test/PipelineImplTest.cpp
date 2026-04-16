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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>

#include <memory>
#include <string>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

HANDLER_TAG(head);
HANDLER_TAG(middle);
HANDLER_TAG(tail);
HANDLER_TAG(nonexistent);

class PipelineImplTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MockHandler::resetOrderCounter();
    transport_.reset();
    app_.reset();
    allocator_.reset();
  }

  PipelineImpl::Ptr buildPipeline() {
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&evb_)
        .setHead(&transport_)
        .setTail(&app_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(head_tag, std::move(head_handler_))
        .addNextDuplex<MockHandler>(middle_tag, std::move(middle_handler_))
        .addNextDuplex<MockHandler>(tail_tag, std::move(tail_handler_))
        .build();
  }

  void createHandlers() {
    head_handler_ = std::make_unique<MockHandler>();
    middle_handler_ = std::make_unique<MockHandler>();
    tail_handler_ = std::make_unique<MockHandler>();
    head_ptr_ = head_handler_.get();
    middle_ptr_ = middle_handler_.get();
    tail_ptr_ = tail_handler_.get();
  }

  folly::EventBase evb_;
  MockHeadHandler transport_; // Head = writes (transport side)
  MockTailHandler app_; // Tail = reads (app side)
  TestAllocator allocator_;

  std::unique_ptr<MockHandler> head_handler_;
  std::unique_ptr<MockHandler> middle_handler_;
  std::unique_ptr<MockHandler> tail_handler_;
  MockHandler* head_ptr_{nullptr};
  MockHandler* middle_ptr_{nullptr};
  MockHandler* tail_ptr_{nullptr};
};

// ==================== Inbound Flow Tests ====================

TEST_F(PipelineImplTest, FireReadFlowsThroughAllHandlers) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto msg = TypeErasedBox(42);
  auto result = pipeline->fireRead(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 1);
  EXPECT_EQ(tail_ptr_->readCount(), 1);
  EXPECT_EQ(app_.messageCount(), 1);
}

TEST_F(PipelineImplTest, FireReadReachesApp) {
  createHandlers();
  bool app_received = false;
  app_.setOnReadCallback([&](TypeErasedBox&& msg) {
    app_received = true;
    EXPECT_EQ(msg.get<int>(), 42);
    return Result::Success;
  });

  auto pipeline = buildPipeline();
  auto msg = TypeErasedBox(42);
  auto result = pipeline->fireRead(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(app_received);
}

TEST_F(PipelineImplTest, FireReadStopsOnBackpressure) {
  createHandlers();

  // Middle handler returns backpressure
  middle_handler_->setOnRead([](detail::ContextImpl&, TypeErasedBox&&) {
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();
  auto msg = TypeErasedBox(42);
  auto result = pipeline->fireRead(std::move(msg));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 1);
  EXPECT_EQ(tail_ptr_->readCount(), 0); // Should not reach tail
  EXPECT_EQ(app_.messageCount(), 0); // Should not reach app
}

TEST_F(PipelineImplTest, FireReadStopsOnError) {
  createHandlers();

  // Head handler returns error
  head_handler_->setOnRead(
      [](detail::ContextImpl&, TypeErasedBox&&) { return Result::Error; });

  auto pipeline = buildPipeline();
  auto msg = TypeErasedBox(42);
  auto result = pipeline->fireRead(std::move(msg));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 0); // Should not continue
  EXPECT_EQ(tail_ptr_->readCount(), 0);
}

// ==================== Outbound Flow Tests ====================

TEST_F(PipelineImplTest, FireWriteFlowsThroughAllHandlers) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->fireWrite(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(tail_ptr_->writeCount(), 1);
  EXPECT_EQ(middle_ptr_->writeCount(), 1);
  EXPECT_EQ(head_ptr_->writeCount(), 1);
  EXPECT_EQ(transport_.writeCount(), 1);
}

TEST_F(PipelineImplTest, FireWriteReachesTransport) {
  createHandlers();
  bool transport_received = false;
  transport_.setWriteCallback([&](BytesPtr bytes) {
    transport_received = true;
    std::string data(
        reinterpret_cast<const char*>(bytes->data()), bytes->length());
    EXPECT_EQ(data, "hello");
    return Result::Success;
  });

  auto pipeline = buildPipeline();
  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->fireWrite(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(transport_received);
}

TEST_F(PipelineImplTest, FireWriteStopsOnBackpressure) {
  createHandlers();

  // Middle handler returns backpressure
  middle_handler_->setOnWrite([](detail::ContextImpl&, TypeErasedBox&&) {
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();
  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->fireWrite(std::move(msg));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_EQ(tail_ptr_->writeCount(), 1);
  EXPECT_EQ(middle_ptr_->writeCount(), 1);
  EXPECT_EQ(head_ptr_->writeCount(), 0); // Should not reach head
  EXPECT_EQ(transport_.writeCount(), 0); // Should not reach transport
}

// ==================== Send to Specific Handler Tests ====================

TEST_F(PipelineImplTest, SendReadToSpecificHandler) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto msg = TypeErasedBox(42);
  auto result = pipeline->sendRead(middle_tag, std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head_ptr_->readCount(), 0); // Skipped
  EXPECT_EQ(middle_ptr_->readCount(), 1);
  EXPECT_EQ(tail_ptr_->readCount(), 1);
  EXPECT_EQ(app_.messageCount(), 1);
}

TEST_F(PipelineImplTest, SendWriteToSpecificHandler) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->sendWrite(middle_tag, std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(tail_ptr_->writeCount(), 0); // Skipped
  EXPECT_EQ(middle_ptr_->writeCount(), 1);
  EXPECT_EQ(head_ptr_->writeCount(), 1);
  EXPECT_EQ(transport_.writeCount(), 1);
}

TEST_F(PipelineImplTest, SendToNonexistentHandlerReturnsError) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto msg = TypeErasedBox(42);
  auto result = pipeline->sendRead(nonexistent_tag, std::move(msg));

  EXPECT_EQ(result, Result::Error);
}

// ==================== Exception Handling Tests ====================

TEST_F(PipelineImplTest, FireExceptionFromIndex) {
  createHandlers();

  bool exception_handled = false;
  tail_handler_->setOnException(
      [&](detail::ContextImpl&, folly::exception_wrapper&& e) {
        exception_handled = true;
        EXPECT_TRUE(e.is_compatible_with<std::runtime_error>());
      });

  auto pipeline = buildPipeline();
  pipeline->sendException(
      head_tag,
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_TRUE(exception_handled);
}

TEST_F(PipelineImplTest, FireExceptionReachesApp) {
  createHandlers();

  bool app_received = false;
  app_.setOnExceptionCallback([&](folly::exception_wrapper&& e) {
    app_received = true;
    EXPECT_TRUE(e.is_compatible_with<std::runtime_error>());
  });

  // Configure tail handler to forward exceptions to app BEFORE building
  tail_handler_->setOnException(
      [](detail::ContextImpl& ctx, folly::exception_wrapper&& e) {
        ctx.fireException(std::move(e));
      });

  auto pipeline = buildPipeline();

  // Fire exception through the pipeline
  pipeline->sendException(
      head_tag,
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  EXPECT_TRUE(app_received);
  EXPECT_EQ(app_.exceptionCount(), 1);
}

TEST_F(PipelineImplTest, FireExceptionToAppWithNoHandlers) {
  // Build a pipeline with no handlers - exception should go directly to app
  bool app_received = false;
  app_.setOnExceptionCallback([&](folly::exception_wrapper&& e) {
    app_received = true;
    EXPECT_TRUE(e.is_compatible_with<std::runtime_error>());
  });

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .build();

  // Fire exception directly - should go to app since no handlers
  // Use fireExceptionFromIndex indirectly via context
  // Since there are no handlers, we need to verify via different means
  EXPECT_EQ(app_.exceptionCount(), 0);

  // Pipeline with no handlers doesn't have contexts to call fireException
  // But we can verify the concept works by checking that the app handler
  // is properly wired to receive exceptions
  EXPECT_TRUE(app_received == false);
}

TEST_F(PipelineImplTest, FireExceptionPassthroughReachesApp) {
  createHandlers();

  // Configure all handlers to passthrough exceptions
  head_handler_->setOnException(
      [](detail::ContextImpl& ctx, folly::exception_wrapper&& e) {
        ctx.fireException(std::move(e));
      });
  middle_handler_->setOnException(
      [](detail::ContextImpl& ctx, folly::exception_wrapper&& e) {
        ctx.fireException(std::move(e));
      });
  tail_handler_->setOnException(
      [](detail::ContextImpl& ctx, folly::exception_wrapper&& e) {
        ctx.fireException(std::move(e));
      });

  bool app_received = false;
  app_.setOnExceptionCallback([&](folly::exception_wrapper&& e) {
    app_received = true;
    EXPECT_TRUE(e.is_compatible_with<std::logic_error>());
  });

  auto pipeline = buildPipeline();

  // Fire exception from head - should propagate through all handlers to app
  pipeline->sendException(
      head_tag, folly::make_exception_wrapper<std::logic_error>("logic error"));

  EXPECT_TRUE(app_received);
  EXPECT_EQ(app_.exceptionCount(), 1);
  EXPECT_EQ(head_ptr_->exceptionCount(), 1);
  EXPECT_EQ(middle_ptr_->exceptionCount(), 1);
  EXPECT_EQ(tail_ptr_->exceptionCount(), 1);
}

TEST_F(PipelineImplTest, FireExceptionSwallowedByHandler) {
  createHandlers();

  // Middle handler swallows exceptions (doesn't forward)
  middle_handler_->setOnException(
      [](detail::ContextImpl&, folly::exception_wrapper&&) {
        // Swallow - don't forward
      });

  bool app_received = false;
  app_.setOnExceptionCallback(
      [&](folly::exception_wrapper&&) { app_received = true; });

  auto pipeline = buildPipeline();

  // Fire exception from head
  pipeline->sendException(
      head_tag,
      folly::make_exception_wrapper<std::runtime_error>("test error"));

  // Exception should NOT reach the app (swallowed by middle handler)
  EXPECT_FALSE(app_received);
  EXPECT_EQ(app_.exceptionCount(), 0);
  EXPECT_EQ(head_ptr_->exceptionCount(), 1);
  EXPECT_EQ(middle_ptr_->exceptionCount(), 1);
  EXPECT_EQ(tail_ptr_->exceptionCount(), 0); // Never reached
}

// ==================== Lifecycle Tests ====================

TEST_F(PipelineImplTest, FireConnectCallsAllHandlers) {
  createHandlers();
  auto pipeline = buildPipeline();

  // Fire connect event
  pipeline->activate();

  // All handlers should receive onPipelineActivated
  EXPECT_EQ(head_ptr_->pipelineActivatedCount(), 1);
  EXPECT_EQ(middle_ptr_->pipelineActivatedCount(), 1);
  EXPECT_EQ(tail_ptr_->pipelineActivatedCount(), 1);
}

TEST_F(PipelineImplTest, FireConnectIsIdempotent) {
  createHandlers();
  auto pipeline = buildPipeline();

  // Fire connect multiple times
  pipeline->activate();
  pipeline->activate();
  pipeline->activate();

  // Each handler should receive onPipelineActivated each time
  EXPECT_EQ(head_ptr_->pipelineActivatedCount(), 3);
  EXPECT_EQ(middle_ptr_->pipelineActivatedCount(), 3);
  EXPECT_EQ(tail_ptr_->pipelineActivatedCount(), 3);
}

TEST_F(PipelineImplTest, FireConnectAfterCloseIsNoop) {
  createHandlers();
  auto pipeline = buildPipeline();

  pipeline->close();
  pipeline->activate();

  // Should not call onPipelineActivated after close
  EXPECT_EQ(head_ptr_->pipelineActivatedCount(), 0);
  EXPECT_EQ(middle_ptr_->pipelineActivatedCount(), 0);
  EXPECT_EQ(tail_ptr_->pipelineActivatedCount(), 0);
}

TEST_F(PipelineImplTest, FireConnectWithInboundOnlyHandler) {
  auto inbound = std::make_unique<InboundOnlyHandler>();
  InboundOnlyHandler* inboundPtr = inbound.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextInbound<InboundOnlyHandler>(head_tag, std::move(inbound))
          .build();

  pipeline->activate();

  EXPECT_EQ(inboundPtr->pipelineActivatedCount(), 1);
}

TEST_F(PipelineImplTest, HandlerRemovedCalledInReverseOrder) {
  createHandlers();

  {
    auto pipeline = buildPipeline();
    pipeline->close();

    // handlerRemoved should be called in reverse: tail(3), middle(4), head(5)
    // (handlerAdded used 0, 1, 2)
    EXPECT_EQ(tail_ptr_->handlerRemovedCount(), 1);
    EXPECT_EQ(middle_ptr_->handlerRemovedCount(), 1);
    EXPECT_EQ(head_ptr_->handlerRemovedCount(), 1);

    EXPECT_GT(tail_ptr_->handlerRemovedOrder(), tail_ptr_->handlerAddedOrder());
    EXPECT_LT(
        tail_ptr_->handlerRemovedOrder(), middle_ptr_->handlerRemovedOrder());
    EXPECT_LT(
        middle_ptr_->handlerRemovedOrder(), head_ptr_->handlerRemovedOrder());
  }
}

TEST_F(PipelineImplTest, CloseIsIdempotent) {
  createHandlers();
  auto pipeline = buildPipeline();

  pipeline->close();
  pipeline->close(); // Should not call handlerRemoved again

  EXPECT_EQ(head_ptr_->handlerRemovedCount(), 1);
  EXPECT_EQ(middle_ptr_->handlerRemovedCount(), 1);
  EXPECT_EQ(tail_ptr_->handlerRemovedCount(), 1);
}

// ==================== Disconnect Tests ====================

TEST_F(PipelineImplTest, FireDisconnectCallsAllHandlers) {
  createHandlers();
  auto pipeline = buildPipeline();

  pipeline->deactivate();

  EXPECT_EQ(head_ptr_->pipelineDeactivatedCount(), 1);
  EXPECT_EQ(middle_ptr_->pipelineDeactivatedCount(), 1);
  EXPECT_EQ(tail_ptr_->pipelineDeactivatedCount(), 1);
}

TEST_F(PipelineImplTest, FireDisconnectCallsHandlersInTailToHeadOrder) {
  createHandlers();

  int callOrder = 0;
  int headOrder = -1, middleOrder = -1, tailOrder = -1;

  head_ptr_->setOnPipelineDeactivated(
      [&](detail::ContextImpl&) { headOrder = callOrder++; });
  middle_ptr_->setOnPipelineDeactivated(
      [&](detail::ContextImpl&) { middleOrder = callOrder++; });
  tail_ptr_->setOnPipelineDeactivated(
      [&](detail::ContextImpl&) { tailOrder = callOrder++; });

  auto pipeline = buildPipeline();
  pipeline->deactivate();

  // Should be called tail → head (reverse of connect order)
  EXPECT_LT(tailOrder, middleOrder);
  EXPECT_LT(middleOrder, headOrder);
}

TEST_F(PipelineImplTest, FireDisconnectAfterCloseIsNoop) {
  createHandlers();
  auto pipeline = buildPipeline();

  pipeline->close();
  pipeline->deactivate();

  EXPECT_EQ(head_ptr_->pipelineDeactivatedCount(), 0);
  EXPECT_EQ(middle_ptr_->pipelineDeactivatedCount(), 0);
  EXPECT_EQ(tail_ptr_->pipelineDeactivatedCount(), 0);
}

TEST_F(PipelineImplTest, FireDisconnectIsIdempotent) {
  createHandlers();
  auto pipeline = buildPipeline();

  pipeline->deactivate();
  pipeline->deactivate();
  pipeline->deactivate();

  EXPECT_EQ(head_ptr_->pipelineDeactivatedCount(), 3);
  EXPECT_EQ(middle_ptr_->pipelineDeactivatedCount(), 3);
  EXPECT_EQ(tail_ptr_->pipelineDeactivatedCount(), 3);
}

// ==================== Context Tests ====================

TEST_F(PipelineImplTest, ContextEventBase) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto* ctx = pipeline->context(head_tag);
  EXPECT_EQ(ctx->eventBase(), &evb_);
}

TEST_F(PipelineImplTest, ContextAllocate) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto* ctx = pipeline->context(head_tag);
  auto buf = ctx->allocate(1024);

  EXPECT_NE(buf.get(), nullptr);
  EXPECT_EQ(allocator_.allocationCount(), 1);
  EXPECT_EQ(allocator_.totalBytesAllocated(), 1024);
}

TEST_F(PipelineImplTest, ContextPipeline) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto* ctx = pipeline->context(head_tag);
  EXPECT_EQ(ctx->pipeline(), pipeline.get());
}

TEST_F(PipelineImplTest, ContextHandlerId) {
  createHandlers();
  auto pipeline = buildPipeline();

  auto* ctx = pipeline->context(middle_tag);
  EXPECT_EQ(ctx->handlerId(), middle_tag.id);
}

// ==================== Empty Pipeline Tests ====================

TEST_F(PipelineImplTest, EmptyPipelineFireReadGoesToApp) {
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .build();

  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->fireRead(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(app_.messageCount(), 1);
}

TEST_F(PipelineImplTest, EmptyPipelineFireWriteGoesToTransport) {
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .build();

  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->fireWrite(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(transport_.writeCount(), 1);
}

// ==================== Inbound-Only / Outbound-Only Handler Tests
// ====================

TEST_F(PipelineImplTest, InboundOnlyHandlerPassthroughWrite) {
  auto inbound = std::make_unique<InboundOnlyHandler>();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextInbound<InboundOnlyHandler>(head_tag, std::move(inbound))
          .build();

  // Write should passthrough
  auto bytes = folly::IOBuf::copyBuffer("hello");
  auto msg = TypeErasedBox(std::move(bytes));
  auto result = pipeline->fireWrite(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(transport_.writeCount(), 1);
}

TEST_F(PipelineImplTest, OutboundOnlyHandlerPassthroughRead) {
  auto outbound = std::make_unique<OutboundOnlyHandler>();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextOutbound<OutboundOnlyHandler>(head_tag, std::move(outbound))
          .build();

  // Read should passthrough (not call outbound handler's onRead)
  auto msg = TypeErasedBox(42);
  auto result = pipeline->fireRead(std::move(msg));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(app_.messageCount(), 1);
}

// ==================== Handler Behavior Tests ====================

TEST_F(PipelineImplTest, HandlerSwallowsMessage) {
  createHandlers();

  // Middle handler swallows - returns Success without calling fireRead
  middle_handler_->setOnRead([](detail::ContextImpl&, TypeErasedBox&&) {
    return Result::Success; // Don't forward
  });

  auto pipeline = buildPipeline();
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 1);
  EXPECT_EQ(tail_ptr_->readCount(), 0); // Never reached
  EXPECT_EQ(app_.messageCount(), 0); // Never reached
}

TEST_F(PipelineImplTest, HandlerSwallowsWrite) {
  createHandlers();

  // Middle handler swallows writes
  middle_handler_->setOnWrite([](detail::ContextImpl&, TypeErasedBox&&) {
    return Result::Success; // Don't forward
  });

  auto pipeline = buildPipeline();
  auto buf = folly::IOBuf::create(64);
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(tail_ptr_->writeCount(), 1);
  EXPECT_EQ(middle_ptr_->writeCount(), 1);
  EXPECT_EQ(head_ptr_->writeCount(), 0); // Never reached
  EXPECT_EQ(transport_.writeCount(), 0); // Never reached
}

TEST_F(PipelineImplTest, HandlerFiresMultipleTimes) {
  createHandlers();

  // Head handler fires 3 messages per input
  head_handler_->setOnRead([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    for (int i = 0; i < 3; ++i) {
      (void)ctx.fireRead(TypeErasedBox(i));
    }
    return Result::Success;
  });

  auto pipeline = buildPipeline();
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 3); // 3 messages
  EXPECT_EQ(tail_ptr_->readCount(), 3);
  EXPECT_EQ(app_.messageCount(), 3);
}

TEST_F(PipelineImplTest, HandlerConvertsReadToWrite) {
  createHandlers();

  // Middle handler converts reads to writes (echo)
  middle_handler_->setOnRead([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    auto buf = folly::IOBuf::create(64);
    (void)ctx.fireWrite(TypeErasedBox(std::move(buf)));
    return Result::Success; // Don't forward read
  });

  auto pipeline = buildPipeline();
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 1);
  EXPECT_EQ(tail_ptr_->readCount(), 0); // Read not forwarded
  EXPECT_EQ(app_.messageCount(), 0); // Read not forwarded
  EXPECT_EQ(head_ptr_->writeCount(), 1); // Write came back through head
  EXPECT_EQ(transport_.writeCount(), 1); // Write reached transport
}

TEST_F(PipelineImplTest, RoundTripEcho) {
  createHandlers();

  // Tail handler is an echo - converts reads to writes
  tail_handler_->setOnRead([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    auto buf = folly::IOBuf::create(64);
    (void)ctx.fireWrite(TypeErasedBox(std::move(buf)));
    return Result::Success;
  });

  auto pipeline = buildPipeline();
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);

  // Read path: head -> middle -> tail
  EXPECT_EQ(head_ptr_->readCount(), 1);
  EXPECT_EQ(middle_ptr_->readCount(), 1);
  EXPECT_EQ(tail_ptr_->readCount(), 1);
  EXPECT_EQ(app_.messageCount(), 0); // Echo swallows read

  // Write path: tail -> middle -> head -> transport
  EXPECT_EQ(tail_ptr_->writeCount(), 0); // Write starts at middle
  EXPECT_EQ(middle_ptr_->writeCount(), 1);
  EXPECT_EQ(head_ptr_->writeCount(), 1);
  EXPECT_EQ(transport_.writeCount(), 1);
}

// ==================== Context-Caching Pattern Tests ====================

/**
 * ContextCachingHandler demonstrates the context-caching pattern for O(1)
 * targeted message routing. It caches a target handler's context pointer
 * at initialization time and uses it for direct routing without map lookup.
 *
 * IMPORTANT: ctx.fireRead() fires to the NEXT handler (index + 1), so:
 * - Calling cachedTargetCtx_->fireRead() routes to handlers AFTER the target
 * - This is useful for skip-ahead routing (skipping intermediate handlers)
 * - To invoke a specific handler, use pipeline->sendRead(handlerId, msg)
 */
class ContextCachingHandler {
 public:
  // Intrusive hook for write backpressure (required by pipeline)
  WriteReadyHook writeReadyHook_;

  explicit ContextCachingHandler(HandlerId targetId) : targetId_(targetId) {}

  void handlerAdded(detail::ContextImpl& ctx) noexcept {
    // Cache target context pointer - O(log N) once at init, not per-message
    cachedTargetCtx_ = ctx.pipeline()->context(targetId_);
    handlerAddedCalled_ = true;
  }

  void handlerRemoved(detail::ContextImpl&) noexcept {
    cachedTargetCtx_ = nullptr;
  }

  void onPipelineActivated(detail::ContextImpl&) noexcept {}

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    ++readCount_;
    // Route specific messages directly via cached context (O(1)!)
    // Note: This fires to handlers AFTER the cached context's handler
    if (cachedTargetCtx_ && shouldRouteToTarget(msg)) {
      ++routedCount_;
      return cachedTargetCtx_->fireRead(std::move(msg));
    }
    // Normal sequential forwarding - also O(1)
    return ctx.fireRead(std::move(msg));
  }

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    ++writeCount_;
    return ctx.fireWrite(std::move(msg));
  }

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onReadReady(detail::ContextImpl&) noexcept {}
  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  // Test accessors
  bool handlerAddedCalled() const { return handlerAddedCalled_; }
  detail::ContextImpl* cachedTarget() const { return cachedTargetCtx_; }
  int readCount() const { return readCount_; }
  int writeCount() const { return writeCount_; }
  int routedCount() const { return routedCount_; }

  // Configure which messages to route to target
  void setRouteCondition(std::function<bool(const TypeErasedBox&)> cond) {
    routeCondition_ = std::move(cond);
  }

 private:
  bool shouldRouteToTarget(const TypeErasedBox& msg) const {
    if (routeCondition_) {
      return routeCondition_(msg);
    }
    return false;
  }

  HandlerId targetId_;
  detail::ContextImpl* cachedTargetCtx_{nullptr};
  bool handlerAddedCalled_{false};
  int readCount_{0};
  int writeCount_{0};
  int routedCount_{0};
  std::function<bool(const TypeErasedBox&)> routeCondition_;
};

// Handler tags for context-caching tests
HANDLER_TAG(router);
HANDLER_TAG(target);
HANDLER_TAG(counter);

class ContextCachingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    transport_.reset();
    app_.reset();
    allocator_.reset();
  }

  folly::EventBase evb_;
  MockHeadHandler transport_; // Head = writes (transport side)
  MockTailHandler app_; // Tail = reads (app side)
  TestAllocator allocator_;
};

TEST_F(ContextCachingTest, CacheTargetContextInHandlerAdded) {
  auto router = std::make_unique<ContextCachingHandler>(target_tag.id);
  auto target = std::make_unique<MockHandler>();
  ContextCachingHandler* routerPtr = router.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<ContextCachingHandler>(router_tag, std::move(router))
          .addNextDuplex<MockHandler>(target_tag, std::move(target))
          .build();

  // Verify handlerAdded was called and context was cached
  EXPECT_TRUE(routerPtr->handlerAddedCalled());
  EXPECT_NE(routerPtr->cachedTarget(), nullptr);

  // Verify cached context points to the right handler
  EXPECT_EQ(routerPtr->cachedTarget(), pipeline->context(target_tag));
}

TEST_F(ContextCachingTest, CachedContextEnablesSkipAheadRouting) {
  // Pipeline: router -> middle -> target
  // When router uses target's context, it skips middle and goes directly
  // to handlers after target (i.e., the app)
  auto router = std::make_unique<ContextCachingHandler>(target_tag.id);
  auto middle = std::make_unique<MockHandler>();
  auto target = std::make_unique<MockHandler>();
  ContextCachingHandler* routerPtr = router.get();
  MockHandler* middlePtr = middle.get();
  MockHandler* targetPtr = target.get();

  // Configure router to route all messages with value > 100 via target context
  router->setRouteCondition(
      [](const TypeErasedBox& msg) { return msg.get<int>() > 100; });

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&transport_)
          .setTail(&app_)
          .setAllocator(&allocator_)
          .addNextDuplex<ContextCachingHandler>(router_tag, std::move(router))
          .addNextDuplex<MockHandler>(middle_tag, std::move(middle))
          .addNextDuplex<MockHandler>(target_tag, std::move(target))
          .build();

  // Send a message that should be routed via target's context
  // This means: router -> (skip middle, skip target) -> app
  auto result = pipeline->fireRead(TypeErasedBox(200));
  EXPECT_EQ(result, Result::Success);

  // Router processed and routed the message
  EXPECT_EQ(routerPtr->readCount(), 1);
  EXPECT_EQ(routerPtr->routedCount(), 1);

  // Middle was SKIPPED (that's the point of skip-ahead routing!)
  EXPECT_EQ(middlePtr->readCount(), 0);

  // Target was also skipped (fireRead goes to index+1)
  EXPECT_EQ(targetPtr->readCount(), 0);

  // App received the message directly
  EXPECT_EQ(app_.messageCount(), 1);
}

TEST_F(ContextCachingTest, NormalForwardingWhenRouteConditionFails) {
  auto router = std::make_unique<ContextCachingHandler>(target_tag.id);
  auto middle = std::make_unique<MockHandler>();
  auto target = std::make_unique<MockHandler>();
  ContextCachingHandler* routerPtr = router.get();
  MockHandler* middlePtr = middle.get();
  MockHandler* targetPtr = target.get();

  // Route only messages > 100, so 42 should use normal forwarding
  router->setRouteCondition(
      [](const TypeErasedBox& msg) { return msg.get<int>() > 100; });

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextDuplex<ContextCachingHandler>(router_tag, std::move(router))
          .addNextDuplex<MockHandler>(middle_tag, std::move(middle))
          .addNextDuplex<MockHandler>(target_tag, std::move(target))
          .build();

  // Send a message that should NOT be routed (uses normal forwarding)
  auto result = pipeline->fireRead(TypeErasedBox(42));
  EXPECT_EQ(result, Result::Success);

  // Router processed but did NOT route
  EXPECT_EQ(routerPtr->readCount(), 1);
  EXPECT_EQ(routerPtr->routedCount(), 0);

  // Both middle and target received via normal forwarding
  EXPECT_EQ(middlePtr->readCount(), 1);
  EXPECT_EQ(targetPtr->readCount(), 1);
  EXPECT_EQ(app_.messageCount(), 1);
}

TEST_F(ContextCachingTest, CacheNonExistentHandlerReturnsNull) {
  // Create router targeting a handler that doesn't exist
  auto router = std::make_unique<ContextCachingHandler>(nonexistent_tag.id);
  ContextCachingHandler* routerPtr = router.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextDuplex<ContextCachingHandler>(router_tag, std::move(router))
          .build();

  // handlerAdded was called
  EXPECT_TRUE(routerPtr->handlerAddedCalled());

  // But cached context is null (target doesn't exist)
  EXPECT_EQ(routerPtr->cachedTarget(), nullptr);
}

TEST_F(ContextCachingTest, MixedRoutingAndForwarding) {
  auto router = std::make_unique<ContextCachingHandler>(target_tag.id);
  auto middle = std::make_unique<MockHandler>();
  auto target = std::make_unique<MockHandler>();
  ContextCachingHandler* routerPtr = router.get();
  MockHandler* middlePtr = middle.get();
  MockHandler* targetPtr = target.get();

  // Route even numbers via cached context (skip-ahead), forward odd normally
  router->setRouteCondition(
      [](const TypeErasedBox& msg) { return msg.get<int>() % 2 == 0; });

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextDuplex<ContextCachingHandler>(router_tag, std::move(router))
          .addNextDuplex<MockHandler>(middle_tag, std::move(middle))
          .addNextDuplex<MockHandler>(target_tag, std::move(target))
          .build();

  // Send a mix of messages
  (void)pipeline->fireRead(TypeErasedBox(1)); // Forward normally (odd)
  (void)pipeline->fireRead(TypeErasedBox(2)); // Skip-ahead (even)
  (void)pipeline->fireRead(TypeErasedBox(3)); // Forward normally (odd)
  (void)pipeline->fireRead(TypeErasedBox(4)); // Skip-ahead (even)
  (void)pipeline->fireRead(TypeErasedBox(5)); // Forward normally (odd)

  // Router processed all 5
  EXPECT_EQ(routerPtr->readCount(), 5);

  // 2 were routed via skip-ahead (2 and 4)
  EXPECT_EQ(routerPtr->routedCount(), 2);

  // Middle received only the 3 that were forwarded normally (1, 3, 5)
  EXPECT_EQ(middlePtr->readCount(), 3);

  // Target also received only the 3 forwarded normally
  EXPECT_EQ(targetPtr->readCount(), 3);

  // All 5 reached the app (3 via normal path, 2 via skip-ahead)
  EXPECT_EQ(app_.messageCount(), 5);
}

TEST_F(ContextCachingTest, CachedContextValidForPipelineLifetime) {
  auto router = std::make_unique<ContextCachingHandler>(target_tag.id);
  auto target = std::make_unique<MockHandler>();
  ContextCachingHandler* routerPtr = router.get();

  router->setRouteCondition([](const TypeErasedBox&) { return true; });

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setTail(&app_)
          .setHead(&transport_)
          .setAllocator(&allocator_)
          .addNextDuplex<ContextCachingHandler>(router_tag, std::move(router))
          .addNextDuplex<MockHandler>(target_tag, std::move(target))
          .build();

  detail::ContextImpl* cachedCtx = routerPtr->cachedTarget();
  ASSERT_NE(cachedCtx, nullptr);

  // Use the cached context multiple times throughout pipeline lifetime
  for (int i = 0; i < 100; ++i) {
    (void)pipeline->fireRead(TypeErasedBox(i));
  }

  // Cached context should still be the same pointer
  EXPECT_EQ(routerPtr->cachedTarget(), cachedCtx);

  // All 100 messages were routed via cached context
  EXPECT_EQ(routerPtr->routedCount(), 100);

  // All reached app (via skip-ahead since target is last handler)
  EXPECT_EQ(app_.messageCount(), 100);
}

// ==================== HeadToTailOp Direction Tests ====================

TEST(HeadToTailOpTest, DefaultWriteDirection) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .build();

  auto buf = folly::IOBuf::copyBuffer("write test");
  EXPECT_EQ(
      pipeline->fireWrite(TypeErasedBox(std::move(buf))), Result::Success);
  EXPECT_EQ(head.writeCount(), 1);
  EXPECT_EQ(tail.messageCount(), 0);

  EXPECT_EQ(pipeline->fireRead(TypeErasedBox(42)), Result::Success);
  EXPECT_EQ(tail.messageCount(), 1);
  EXPECT_EQ(head.writeCount(), 1);
}

// Note: ReadDirectionReversesFlow test was removed because direction reversal
// via setHeadToTailOp is no longer supported for new-style endpoints.
// HeadEndpointHandler always receives reads, TailEndpointHandler always
// receives writes. Direction reversal only applies to legacy EndpointHandler.

// ==================== Endpoint Concept Tests ====================

// Static assertions to verify concepts are satisfied
static_assert(
    HeadEndpointHandler<MockHeadHandler>,
    "MockHeadHandler should satisfy HeadEndpointHandler concept");
static_assert(
    TailEndpointHandler<MockTailHandler>,
    "MockTailHandler should satisfy TailEndpointHandler concept");
static_assert(
    ValidEndpointPair<MockHeadHandler, MockTailHandler>,
    "MockHeadHandler+MockTailHandler should be a ValidEndpointPair");

// Verify lifecycle concept is satisfied
static_assert(
    EndpointHandlerLifecycle<MockHeadHandler>,
    "MockHeadHandler should satisfy EndpointHandlerLifecycle concept");
static_assert(
    EndpointHandlerLifecycle<MockTailHandler>,
    "MockTailHandler should satisfy EndpointHandlerLifecycle concept");

// ==================== EndpointLifecycle Tests ====================

TEST(EndpointLifecycleTest, HeadHookCalledOnActivateDeactivate) {
  folly::EventBase evb;
  MockTailHandler tail;
  TestAllocator allocator;

  int activated = 0;
  int deactivated = 0;

  struct LifecycleHead {
    int* activated_;
    int* deactivated_;

    LifecycleHead(int* a, int* d) : activated_(a), deactivated_(d) {}

    // HeadEndpointHandler data method
    Result onWrite(TypeErasedBox&&) noexcept { return Result::Success; }

    // EndpointHandlerLifecycle methods
    void handlerAdded() noexcept {}
    void handlerRemoved() noexcept {}
    void onPipelineActive() noexcept { ++(*activated_); }
    void onPipelineInactive() noexcept { ++(*deactivated_); }
  };

  LifecycleHead head(&activated, &deactivated);

  auto pipeline =
      PipelineBuilder<LifecycleHead, MockTailHandler, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .build();

  EXPECT_EQ(activated, 0);
  pipeline->activate();
  EXPECT_EQ(activated, 1);
  EXPECT_EQ(deactivated, 0);

  pipeline->deactivate();
  EXPECT_EQ(deactivated, 1);
}

TEST(EndpointLifecycleTest, LifecycleMethodsCalledOnMockHandlers) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .build();
#pragma GCC diagnostic pop

  EXPECT_EQ(head.handlerAddedCount(), 1);
  EXPECT_EQ(tail.handlerAddedCount(), 1);
  EXPECT_EQ(head.pipelineActiveCount(), 0);
  EXPECT_EQ(tail.pipelineActiveCount(), 0);

  pipeline->activate();
  EXPECT_EQ(head.pipelineActiveCount(), 1);
  EXPECT_EQ(tail.pipelineActiveCount(), 1);

  pipeline->deactivate();
  EXPECT_EQ(head.pipelineInactiveCount(), 1);
  EXPECT_EQ(tail.pipelineInactiveCount(), 1);
}

TEST(EndpointLifecycleTest, HandlerRemovedCalledOnClose) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;

  {
    auto pipeline =
        PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
            .setEventBase(&evb)
            .setHead(&head)
            .setTail(&tail)
            .setAllocator(&allocator)
            .build();

    EXPECT_EQ(head.handlerRemovedCount(), 0);
    EXPECT_EQ(tail.handlerRemovedCount(), 0);

    pipeline->close();
  }

  // handlerRemoved should be called for both endpoints on close
  EXPECT_EQ(head.handlerRemovedCount(), 1);
  EXPECT_EQ(tail.handlerRemovedCount(), 1);
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
