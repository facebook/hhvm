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

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>

#include <memory>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

HANDLER_TAG(handler1);
HANDLER_TAG(handler2);
HANDLER_TAG(handler3);

/**
 * Tests for delayed destruction behavior in PipelineImpl.
 *
 * These tests verify that:
 * 1. The pipeline uses DelayedDestruction pattern correctly
 * 2. Destruction is deferred when callbacks are executing
 * 3. Handlers can safely trigger close() during callbacks
 * 4. Nested callbacks are safe from use-after-free
 */
class DelayedDestructionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    MockHandler::resetOrderCounter();
    transport_.reset();
    app_.reset();
    allocator_.reset();
  }

  PipelineImpl::Ptr buildSingleHandlerPipeline(
      std::unique_ptr<MockHandler> handler) {
    handler_ptr_ = handler.get();
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&evb_)
        .setHead(&transport_)
        .setTail(&app_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(handler1_tag, std::move(handler))
        .build();
  }

  PipelineImpl::Ptr buildTwoHandlerPipeline(
      std::unique_ptr<MockHandler> handler1,
      std::unique_ptr<MockHandler> handler2) {
    handler1_ptr_ = handler1.get();
    handler2_ptr_ = handler2.get();
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&evb_)
        .setHead(&transport_)
        .setTail(&app_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(handler1_tag, std::move(handler1))
        .addNextDuplex<MockHandler>(handler2_tag, std::move(handler2))
        .build();
  }

  folly::EventBase evb_;
  MockHeadHandler transport_; // Head = writes (transport side)
  MockTailHandler app_; // Tail = reads (app side)
  TestAllocator allocator_;

  MockHandler* handler_ptr_{nullptr};
  MockHandler* handler1_ptr_{nullptr};
  MockHandler* handler2_ptr_{nullptr};
};

// ==================== Basic DestructorGuard Behavior ====================

TEST_F(DelayedDestructionTest, PipelineInheritsFromDelayedDestruction) {
  auto handler = std::make_unique<MockHandler>();
  auto pipeline = buildSingleHandlerPipeline(std::move(handler));

  // Verify that PipelineImpl is a DelayedDestruction
  folly::DelayedDestruction* dd =
      static_cast<folly::DelayedDestruction*>(pipeline.get());
  EXPECT_NE(dd, nullptr);
}

TEST_F(DelayedDestructionTest, DestructorGuardPreventsImmediateDestruction) {
  // Use external counter since handler is destroyed with pipeline
  int handlerRemovedCount = 0;

  auto handler = std::make_unique<MockHandler>();
  handler->setHandlerRemoved(
      [&handlerRemovedCount](detail::ContextImpl&) { ++handlerRemovedCount; });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  PipelineImpl* rawPtr = pipeline.get();

  {
    // Create a DestructorGuard to defer destruction
    folly::DelayedDestructionBase::DestructorGuard guard(rawPtr);

    // Release the unique_ptr and call destroy() - this should NOT
    // immediately destroy the pipeline because DestructorGuard is holding it
    pipeline.release()->destroy();

    // Pipeline should still be accessible (not destroyed yet)
    EXPECT_EQ(guard.get(), rawPtr);

    // handlerRemoved should NOT be called yet - destruction is deferred
    // Note: destroy() sets destroyPending but doesn't call onDelayedDestroy
    // until all guards are released
    EXPECT_EQ(handlerRemovedCount, 0);
  }

  // When guard goes out of scope, pipeline is destroyed and handlerRemoved
  // called
  EXPECT_EQ(handlerRemovedCount, 1);
}

// ==================== Close During Callback Tests ====================

TEST_F(DelayedDestructionTest, CloseDuringOnReadIsSafe) {
  auto handler = std::make_unique<MockHandler>();
  bool callbackCompleted = false;

  // Handler calls close() during onRead, then continues processing
  handler->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.pipeline()->close();
    // Continue with more operations after close - should be safe
    callbackCompleted = true;
    return Result::Success;
  });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(callbackCompleted);
  EXPECT_EQ(handler_ptr_->handlerRemovedCount(), 1);
}

TEST_F(DelayedDestructionTest, CloseDuringOnWriteIsSafe) {
  auto handler = std::make_unique<MockHandler>();
  bool callbackCompleted = false;

  // Handler calls close() during onWrite
  handler->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.pipeline()->close();
    callbackCompleted = true;
    return Result::Success;
  });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  auto bytes = folly::IOBuf::copyBuffer("test");
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(bytes)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(callbackCompleted);
  EXPECT_EQ(handler_ptr_->handlerRemovedCount(), 1);
}

TEST_F(DelayedDestructionTest, CloseDuringExceptionHandlingIsSafe) {
  auto handler = std::make_unique<MockHandler>();
  bool exceptionHandled = false;

  // Handler calls close() during exception handling
  handler->setOnException(
      [&](detail::ContextImpl& ctx, folly::exception_wrapper&&) {
        ctx.pipeline()->close();
        exceptionHandled = true;
        return Result::Success;
      });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  pipeline->sendException(
      handler1_tag, folly::make_exception_wrapper<std::runtime_error>("test"));

  EXPECT_TRUE(exceptionHandled);
  EXPECT_EQ(handler_ptr_->handlerRemovedCount(), 1);
}

// ==================== Nested Callback Safety Tests ====================

TEST_F(DelayedDestructionTest, NestedFireReadWithClosureIsSafe) {
  auto handler1 = std::make_unique<MockHandler>();
  auto handler2 = std::make_unique<MockHandler>();
  bool handler2Called = false;

  // Handler1 forwards the message
  handler1->setOnRead([](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
    return ctx.fireRead(std::move(msg));
  });

  // Handler2 calls close() during processing
  handler2->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.pipeline()->close();
    handler2Called = true;
    return Result::Success;
  });

  auto pipeline =
      buildTwoHandlerPipeline(std::move(handler1), std::move(handler2));
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler2Called);
  EXPECT_EQ(handler1_ptr_->readCount(), 1);
  EXPECT_EQ(handler2_ptr_->readCount(), 1);
  // Both handlers should have handlerRemoved called
  EXPECT_EQ(handler1_ptr_->handlerRemovedCount(), 1);
  EXPECT_EQ(handler2_ptr_->handlerRemovedCount(), 1);
}

TEST_F(DelayedDestructionTest, NestedFireWriteWithClosureIsSafe) {
  auto handler1 = std::make_unique<MockHandler>();
  auto handler2 = std::make_unique<MockHandler>();
  bool handler1Called = false;

  // Handler2 forwards the write
  handler2->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
    return ctx.fireWrite(std::move(msg));
  });

  // Handler1 calls close() during processing
  handler1->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.pipeline()->close();
    handler1Called = true;
    return Result::Success;
  });

  auto pipeline =
      buildTwoHandlerPipeline(std::move(handler1), std::move(handler2));
  auto bytes = folly::IOBuf::copyBuffer("test");
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(bytes)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler1Called);
  EXPECT_EQ(handler1_ptr_->writeCount(), 1);
  EXPECT_EQ(handler2_ptr_->writeCount(), 1);
}

// ==================== Handler Triggering Multiple Operations
// ====================

TEST_F(DelayedDestructionTest, MultipleFiresAfterCloseAreSafe) {
  // Use external counter since handler is destroyed with pipeline
  int handlerRemovedCount = 0;
  int fireReadAttempts = 0;

  auto handler = std::make_unique<MockHandler>();
  handler->setHandlerRemoved(
      [&handlerRemovedCount](detail::ContextImpl&) { ++handlerRemovedCount; });

  // Handler calls close() then tries multiple fireRead calls
  handler->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.pipeline()->close();

    // After close, the pipeline will forward to app (skipping handlers)
    // This is safe behavior - messages still flow but bypass handlers
    for (int i = 0; i < 3; ++i) {
      (void)ctx.fireRead(TypeErasedBox(i));
      ++fireReadAttempts;
    }
    return Result::Success;
  });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(fireReadAttempts, 3);
  // After close, messages are forwarded directly to app (handlers bypassed)
  EXPECT_EQ(app_.messageCount(), 3);
  EXPECT_EQ(handlerRemovedCount, 1);
}

TEST_F(DelayedDestructionTest, ReadToWriteConversionAfterCloseIsSafe) {
  // Use external counter since handler is destroyed with pipeline
  int handlerRemovedCount = 0;
  bool writeAttempted = false;

  auto handler = std::make_unique<MockHandler>();
  handler->setHandlerRemoved(
      [&handlerRemovedCount](detail::ContextImpl&) { ++handlerRemovedCount; });

  // Handler calls close() then tries to convert read to write
  handler->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.pipeline()->close();

    // After close, writes are forwarded directly to transport (handlers
    // bypassed) This is safe behavior - messages still flow but bypass handlers
    auto buf = folly::IOBuf::create(64);
    (void)ctx.fireWrite(TypeErasedBox(std::move(buf)));
    writeAttempted = true;
    return Result::Success;
  });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  auto result = pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(writeAttempted);
  // After close, writes go directly to transport (handlers bypassed)
  EXPECT_EQ(transport_.writeCount(), 1);
  EXPECT_EQ(handlerRemovedCount, 1);
}

// ==================== handlerRemoved Callback Safety ====================

TEST_F(DelayedDestructionTest, HandlerRemovedCanAccessPipeline) {
  auto handler = std::make_unique<MockHandler>();
  bool handlerRemovedAccessedPipeline = false;

  handler->setHandlerRemoved([&](detail::ContextImpl& ctx) {
    // Should be safe to access pipeline during handlerRemoved
    PipelineImpl* pipeline = ctx.pipeline();
    EXPECT_NE(pipeline, nullptr);
    handlerRemovedAccessedPipeline = true;
  });

  {
    auto pipeline = buildSingleHandlerPipeline(std::move(handler));
    pipeline->close();
    EXPECT_TRUE(handlerRemovedAccessedPipeline);
  }
}

TEST_F(DelayedDestructionTest, HandlerRemovedCalledOnceEvenWithNestedClose) {
  auto handler1 = std::make_unique<MockHandler>();
  auto handler2 = std::make_unique<MockHandler>();

  // Handler2's handlerRemoved tries to call close() again
  handler2->setHandlerRemoved([](detail::ContextImpl& ctx) {
    ctx.pipeline()->close(); // Should be no-op
  });

  auto pipeline =
      buildTwoHandlerPipeline(std::move(handler1), std::move(handler2));
  pipeline->close();

  // Each handler's handlerRemoved should be called exactly once
  EXPECT_EQ(handler1_ptr_->handlerRemovedCount(), 1);
  EXPECT_EQ(handler2_ptr_->handlerRemovedCount(), 1);
}

// ==================== Ready Notification Safety ====================

TEST_F(DelayedDestructionTest, CloseInWriteReadyCallbackIsSafe) {
  auto handler = std::make_unique<MockHandler>();
  bool writeReadyCalled = false;

  handler->setOnWriteReady([&](detail::ContextImpl& ctx) {
    ctx.pipeline()->close();
    writeReadyCalled = true;
  });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));

  // Register for write ready notification
  auto* ctx = pipeline->context(handler1_tag);
  ctx->awaitWriteReady();

  // Trigger write ready
  pipeline->onWriteReady();

  EXPECT_TRUE(writeReadyCalled);
  EXPECT_EQ(handler_ptr_->handlerRemovedCount(), 1);
}

// ==================== Pipeline Destruction via destroy() ====================

TEST_F(DelayedDestructionTest, DestroyCallsHandlerRemoved) {
  // Use external counter since handler is destroyed with pipeline
  int handlerRemovedCount = 0;

  auto handler = std::make_unique<MockHandler>();
  handler->setHandlerRemoved(
      [&handlerRemovedCount](detail::ContextImpl&) { ++handlerRemovedCount; });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));

  // Get raw pointer before releasing
  PipelineImpl* rawPtr = pipeline.release();

  // Call destroy() directly - this will immediately destroy the pipeline
  // and all handlers, so we can't access handler_ptr_ afterwards
  rawPtr->destroy();

  // Verify via external counter
  EXPECT_EQ(handlerRemovedCount, 1);
}

TEST_F(DelayedDestructionTest, DestroyDeferredWithGuard) {
  // Use external counter since handler is destroyed with pipeline
  int handlerRemovedCount = 0;

  auto handler = std::make_unique<MockHandler>();
  handler->setHandlerRemoved(
      [&handlerRemovedCount](detail::ContextImpl&) { ++handlerRemovedCount; });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  PipelineImpl* rawPtr = pipeline.get();

  {
    folly::DelayedDestructionBase::DestructorGuard guard(rawPtr);

    // Release and call destroy while guard is active
    pipeline.release()->destroy();

    // Pipeline should still be valid
    EXPECT_EQ(guard.get(), rawPtr);

    // handlerRemoved should NOT be called yet - destruction is deferred
    EXPECT_EQ(handlerRemovedCount, 0);
  }
  // Guard goes out of scope, actual destruction happens
  EXPECT_EQ(handlerRemovedCount, 1);
}

// ==================== Edge Cases ====================

TEST_F(DelayedDestructionTest, CloseCalledMultipleTimesInCallback) {
  auto handler = std::make_unique<MockHandler>();
  int closeCallCount = 0;

  handler->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    // Call close multiple times
    for (int i = 0; i < 5; ++i) {
      ctx.pipeline()->close();
      ++closeCallCount;
    }
    return Result::Success;
  });

  auto pipeline = buildSingleHandlerPipeline(std::move(handler));
  (void)pipeline->fireRead(TypeErasedBox(42));

  EXPECT_EQ(closeCallCount, 5);
  // handlerRemoved should still only be called once
  EXPECT_EQ(handler_ptr_->handlerRemovedCount(), 1);
}

TEST_F(DelayedDestructionTest, PipelineOperationAfterCloseReturnsEarly) {
  auto handler = std::make_unique<MockHandler>();
  auto pipeline = buildSingleHandlerPipeline(std::move(handler));

  pipeline->close();

  // Further operations on closed pipeline should be handled gracefully
  (void)pipeline->fireRead(TypeErasedBox(42));
  auto bytes = folly::IOBuf::copyBuffer("test");
  (void)pipeline->fireWrite(TypeErasedBox(std::move(bytes)));

  // Read count should still be 0 since pipeline is closed
  EXPECT_EQ(handler_ptr_->readCount(), 0);
  EXPECT_EQ(handler_ptr_->writeCount(), 0);

  // Results go directly to app/transport since handlers are bypassed
  EXPECT_EQ(app_.messageCount(), 1);
  EXPECT_EQ(transport_.writeCount(), 1);
}

TEST_F(DelayedDestructionTest, RecursiveFireReadDuringCloseIsSafe) {
  auto handler1 = std::make_unique<MockHandler>();
  auto handler2 = std::make_unique<MockHandler>();
  int handler1Reads = 0;
  int handler2Reads = 0;

  // Handler1 fires additional reads during processing
  handler1->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
    ++handler1Reads;
    if (handler1Reads == 1) {
      // First read: fire another read and close
      (void)ctx.fireRead(TypeErasedBox(100));
      ctx.pipeline()->close();
      (void)ctx.fireRead(TypeErasedBox(200)); // After close
    }
    return ctx.fireRead(std::move(msg));
  });

  handler2->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
    ++handler2Reads;
    return ctx.fireRead(std::move(msg));
  });

  auto pipeline =
      buildTwoHandlerPipeline(std::move(handler1), std::move(handler2));
  (void)pipeline->fireRead(TypeErasedBox(42));

  // Handler1 should have processed the initial read
  EXPECT_GE(handler1Reads, 1);
  // Handler2 should have received at least some reads
  EXPECT_GE(handler2Reads, 1);
  // Both should have handlerRemoved called
  EXPECT_EQ(handler1_ptr_->handlerRemovedCount(), 1);
  EXPECT_EQ(handler2_ptr_->handlerRemovedCount(), 1);
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
