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

namespace apache::thrift::fast_thrift::channel_pipeline::test {

HANDLER_TAG(head);
HANDLER_TAG(middle);
HANDLER_TAG(tail);

class BackpressureTest : public ::testing::Test {
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
        .setHead(&app_)
        .setTail(&transport_)
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
  MockTailHandler transport_;
  MockHeadHandler app_;
  TestAllocator allocator_;

  std::unique_ptr<MockHandler> head_handler_;
  std::unique_ptr<MockHandler> middle_handler_;
  std::unique_ptr<MockHandler> tail_handler_;
  MockHandler* head_ptr_{nullptr};
  MockHandler* middle_ptr_{nullptr};
  MockHandler* tail_ptr_{nullptr};
  PipelineImpl::Ptr pipeline_;
};

// ==================== Explicit Backpressure API Tests ====================

TEST_F(BackpressureTest, AwaitWriteReadyRegistersHandler) {
  createHandlers();

  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();

  EXPECT_FALSE(pipeline->hasPendingWriteReady());

  auto buf = folly::IOBuf::create(64);
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_TRUE(pipeline->hasPendingWriteReady());
}

TEST_F(BackpressureTest, CancelAwaitWriteReadyUnregistersHandler) {
  createHandlers();

  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });

  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_TRUE(pipeline->hasPendingWriteReady());

  pipeline->onWriteReady();

  EXPECT_FALSE(pipeline->hasPendingWriteReady());
}

TEST_F(BackpressureTest, IsAwaitingWriteReadyReturnsCorrectState) {
  createHandlers();

  bool was_awaiting_before = false;
  bool was_awaiting_after = false;

  middle_handler_->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    was_awaiting_before = ctx.isAwaitingWriteReady();
    ctx.awaitWriteReady();
    was_awaiting_after = ctx.isAwaitingWriteReady();
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_FALSE(was_awaiting_before);
  EXPECT_TRUE(was_awaiting_after);
}

// ==================== Write Backpressure Tests ====================

TEST_F(BackpressureTest, HandlerReceivesWriteReadyAfterBackpressure) {
  createHandlers();

  bool write_ready_called = false;
  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });
  middle_handler_->setOnWriteReady([&](detail::ContextImpl& ctx) {
    write_ready_called = true;
    ctx.cancelAwaitWriteReady();
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_FALSE(write_ready_called);
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 0);

  pipeline->onWriteReady();

  EXPECT_TRUE(write_ready_called);
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 1);
}

TEST_F(BackpressureTest, OnlyRegisteredHandlerReceivesWriteReady) {
  createHandlers();

  // Only middle handler registers for write ready
  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });
  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  // Head and tail don't register
  head_handler_->setOnWrite(
      [](detail::ContextImpl&, TypeErasedBox&&) { return Result::Success; });
  tail_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
    return ctx.fireWrite(std::move(msg));
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));
  pipeline->onWriteReady();

  // Only middle registered, so only middle receives callback
  EXPECT_EQ(head_ptr_->writeReadyCount(), 0);
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 1);
  EXPECT_EQ(tail_ptr_->writeReadyCount(), 0);
}

TEST_F(BackpressureTest, MultipleHandlersCanRegisterForWriteReady) {
  createHandlers();

  // Both middle and tail register for write ready
  middle_handler_->setOnWrite(
      [](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
        ctx.awaitWriteReady();
        (void)ctx.fireWrite(std::move(msg)); // Forward to next
        return Result::Backpressure;
      });
  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  head_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });
  head_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_TRUE(pipeline->hasPendingWriteReady());

  pipeline->onWriteReady();

  // Both handlers that registered receive callback
  EXPECT_EQ(head_ptr_->writeReadyCount(), 1);
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 1);
  EXPECT_EQ(tail_ptr_->writeReadyCount(), 0);
}

TEST_F(BackpressureTest, HandlerCanResumeWritingAfterWriteReady) {
  createHandlers();

  bool backpressure_active = true;
  int successful_writes = 0;

  middle_handler_->setOnWrite(
      [&backpressure_active, &successful_writes](
          detail::ContextImpl& ctx, TypeErasedBox&& msg) {
        if (backpressure_active) {
          ctx.awaitWriteReady();
          return Result::Backpressure;
        }
        successful_writes++;
        return ctx.fireWrite(std::move(msg));
      });

  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  auto pipeline = buildPipeline();

  auto buf1 = folly::IOBuf::create(64);
  auto result1 = pipeline->fireWrite(TypeErasedBox(std::move(buf1)));
  EXPECT_EQ(result1, Result::Backpressure);
  EXPECT_EQ(successful_writes, 0);
  EXPECT_EQ(app_.writeCount(), 0);

  backpressure_active = false;
  pipeline->onWriteReady();

  auto buf2 = folly::IOBuf::create(64);
  auto result2 = pipeline->fireWrite(TypeErasedBox(std::move(buf2)));
  EXPECT_EQ(result2, Result::Success);
  EXPECT_EQ(successful_writes, 1);
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 1);
}

// ==================== Rapid Backpressure/Resume Cycles ====================

TEST_F(BackpressureTest, RapidBackpressureResumeCycles) {
  createHandlers();

  bool backpressure_active = true;

  middle_handler_->setOnWrite(
      [&backpressure_active](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
        if (backpressure_active) {
          ctx.awaitWriteReady();
          return Result::Backpressure;
        }
        return ctx.fireWrite(std::move(msg));
      });

  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  auto pipeline = buildPipeline();

  for (int cycle = 0; cycle < 10; ++cycle) {
    backpressure_active = true;
    auto buf = folly::IOBuf::create(64);
    auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));
    EXPECT_EQ(result, Result::Backpressure);
    EXPECT_TRUE(pipeline->hasPendingWriteReady());

    backpressure_active = false;
    pipeline->onWriteReady();
    EXPECT_FALSE(pipeline->hasPendingWriteReady());
  }

  EXPECT_EQ(middle_ptr_->writeReadyCount(), 10);
}

// ==================== EventBase Integration Tests ====================

TEST_F(BackpressureTest, AsyncResumeFromEventBase) {
  createHandlers();

  bool write_ready_received = false;
  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });
  middle_handler_->setOnWriteReady([&](detail::ContextImpl& ctx) {
    write_ready_received = true;
    ctx.cancelAwaitWriteReady();
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_FALSE(write_ready_received);

  evb_.runInEventBaseThread([&pipeline]() { pipeline->onWriteReady(); });
  evb_.loopOnce();

  EXPECT_TRUE(write_ready_received);
}

TEST_F(BackpressureTest, ContextValidDuringAsyncCallback) {
  createHandlers();

  detail::ContextImpl* captured_ctx = nullptr;
  bool callback_executed = false;

  middle_handler_->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&&) {
    captured_ctx = &ctx;
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });

  middle_handler_->setOnWriteReady([&](detail::ContextImpl& ctx) {
    EXPECT_EQ(&ctx, captured_ctx);
    EXPECT_EQ(ctx.pipeline(), pipeline_.get());
    EXPECT_EQ(ctx.eventBase(), &evb_);
    callback_executed = true;
    ctx.cancelAwaitWriteReady();
  });

  pipeline_ = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline_->fireWrite(TypeErasedBox(std::move(buf)));

  evb_.runInEventBaseThread([this]() { pipeline_->onWriteReady(); });
  evb_.loopOnce();

  EXPECT_TRUE(callback_executed);
}

// ==================== Edge Cases ====================

TEST_F(BackpressureTest, ClosedPipelineIgnoresWriteReady) {
  createHandlers();

  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_TRUE(pipeline->hasPendingWriteReady());

  pipeline->close();

  pipeline->onWriteReady();

  EXPECT_EQ(middle_ptr_->writeReadyCount(), 0);
}

TEST_F(BackpressureTest, NoBackpressureDoesNotMarkPending) {
  createHandlers();

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_FALSE(pipeline->hasPendingWriteReady());
}

TEST_F(BackpressureTest, ErrorDoesNotMarkPending) {
  createHandlers();

  middle_handler_->setOnWrite(
      [](detail::ContextImpl&, TypeErasedBox&&) { return Result::Error; });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(pipeline->hasPendingWriteReady());
}

TEST_F(BackpressureTest, MultipleWriteReadyCallsAreIdempotent) {
  createHandlers();

  middle_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });

  // Don't cancel on first callback - stay registered
  int callback_count = 0;
  middle_handler_->setOnWriteReady([&callback_count](detail::ContextImpl&) {
    callback_count++;
    // Note: Not canceling, so handler stays in list
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  // First onWriteReady call
  pipeline->onWriteReady();
  EXPECT_EQ(callback_count, 1);

  // Handler is still in list, so second call should also trigger callback
  pipeline->onWriteReady();
  EXPECT_EQ(callback_count, 2);
}

TEST_F(BackpressureTest, AwaitWriteReadyIsIdempotent) {
  createHandlers();

  int await_calls = 0;
  middle_handler_->setOnWrite(
      [&await_calls](detail::ContextImpl& ctx, TypeErasedBox&&) {
        await_calls++;
        ctx.awaitWriteReady();
        ctx.awaitWriteReady(); // Call twice
        ctx.awaitWriteReady(); // Call thrice
        return Result::Backpressure;
      });

  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));
  pipeline->onWriteReady();

  // Handler should only receive one callback despite multiple await calls
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 1);
}

// ==================== Handler Unlink During Callback Tests
// ====================

TEST_F(BackpressureTest, HandlerCanUnlinkDuringOnWriteReady) {
  createHandlers();

  // Both handlers register
  head_handler_->setOnWrite([](detail::ContextImpl& ctx, TypeErasedBox&&) {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  });
  head_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  middle_handler_->setOnWrite(
      [](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
        ctx.awaitWriteReady();
        (void)ctx.fireWrite(std::move(msg));
        return Result::Backpressure;
      });
  middle_handler_->setOnWriteReady(
      [](detail::ContextImpl& ctx) { ctx.cancelAwaitWriteReady(); });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  (void)pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  // Should not crash - safe iteration even when handlers unlink themselves
  pipeline->onWriteReady();

  EXPECT_EQ(head_ptr_->writeReadyCount(), 1);
  EXPECT_EQ(middle_ptr_->writeReadyCount(), 1);
  EXPECT_FALSE(pipeline->hasPendingWriteReady());
}

// ==================== Backpressure Strategy Tests ====================

TEST_F(BackpressureTest, HandlerCanDropMessageOnBackpressure) {
  createHandlers();

  int drop_count = 0;

  // Handler drops message instead of buffering
  middle_handler_->setOnWrite(
      [&drop_count](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
        auto result = ctx.fireWrite(std::move(msg));
        if (result == Result::Backpressure) {
          drop_count++;
          return Result::Success; // Drop the message, return success
        }
        return result;
      });

  // Head handler simulates downstream backpressure
  head_handler_->setOnWrite([](detail::ContextImpl&, TypeErasedBox&&) {
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_EQ(result, Result::Success); // Dropped, returned success
  EXPECT_EQ(drop_count, 1);
  EXPECT_FALSE(pipeline->hasPendingWriteReady()); // No registration
}

TEST_F(BackpressureTest, HandlerCanReturnErrorOnBackpressure) {
  createHandlers();

  int error_count = 0;

  // Handler returns error instead of buffering
  middle_handler_->setOnWrite(
      [&error_count](detail::ContextImpl& ctx, TypeErasedBox&& msg) {
        auto result = ctx.fireWrite(std::move(msg));
        if (result == Result::Backpressure) {
          error_count++;
          return Result::Error; // Signal error to caller
        }
        return result;
      });

  // Head handler simulates downstream backpressure
  head_handler_->setOnWrite([](detail::ContextImpl&, TypeErasedBox&&) {
    return Result::Backpressure;
  });

  auto pipeline = buildPipeline();

  auto buf = folly::IOBuf::create(64);
  auto result = pipeline->fireWrite(TypeErasedBox(std::move(buf)));

  EXPECT_EQ(result, Result::Error);
  EXPECT_EQ(error_count, 1);
  EXPECT_FALSE(pipeline->hasPendingWriteReady()); // No registration
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
