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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/CoroContextHandle.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/CancellationToken.h>
#include <folly/coro/Baton.h>
#include <folly/coro/CurrentExecutor.h>
#include <folly/coro/Task.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {
namespace {

HANDLER_TAG(coro_source);
HANDLER_TAG(coro_target);
HANDLER_TAG(coro_chain_a);
HANDLER_TAG(coro_chain_b);
HANDLER_TAG(coro_chain_c);

struct TraceMessage {
  std::vector<std::string> trace;
};

enum class CoroCompletionMode { EventBase, Worker };

template <typename Awaitable>
concept TrustedAwaitable = requires(Awaitable&& awaitable) {
  {
    std::move(awaitable).viaIfAsync(folly::Executor::KeepAlive<>{})
  } noexcept -> std::same_as<Awaitable&&>;
};

static_assert(TrustedAwaitable<coro::ReadAwaitable>);
static_assert(TrustedAwaitable<coro::WriteAwaitable>);
static_assert(TrustedAwaitable<coro::ExceptionAwaitable>);

folly::coro::Task<Result> asyncFireRead(
    coro::ContextHandle handle,
    TypeErasedBox message,
    std::thread::id& coroutineThread) {
  co_await folly::coro::co_reschedule_on_current_executor;
  coroutineThread = std::this_thread::get_id();
  co_return co_await std::move(handle).co_fireRead(std::move(message));
}

folly::coro::Task<Result> asyncFireReadStarted(
    coro::ContextHandle handle,
    TypeErasedBox message,
    folly::Baton<>& started) {
  started.post();
  co_return co_await std::move(handle).co_fireRead(std::move(message));
}

folly::coro::Task<Result> asyncFireWrite(
    coro::ContextHandle handle, TypeErasedBox message) {
  co_await folly::coro::co_reschedule_on_current_executor;
  co_return co_await std::move(handle).co_fireWrite(std::move(message));
}

folly::coro::Task<void> asyncFireException(
    coro::ContextHandle handle, folly::exception_wrapper exception) {
  co_await folly::coro::co_reschedule_on_current_executor;
  co_await std::move(handle).co_fireException(std::move(exception));
}

folly::coro::Task<Result> suspendedFireRead(
    coro::ContextHandle handle,
    TypeErasedBox message,
    folly::Baton<>& suspended,
    folly::coro::Baton& resume) {
  suspended.post();
  co_await resume;
  co_return co_await std::move(handle).co_fireRead(std::move(message));
}

class ContextHandleCoroTest : public ::testing::Test {
 protected:
  void SetUp() override { eventBase_ = eventBaseThread_.getEventBase(); }

  PipelineImpl::Ptr buildPipeline(
      std::unique_ptr<MockHandler> source,
      std::unique_ptr<MockHandler> target) {
    PipelineImpl::Ptr pipeline;
    eventBase_->runInEventBaseThreadAndWait([&] {
      pipeline =
          PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
              .setEventBase(eventBase_)
              .setHead(&head_)
              .setTail(&tail_)
              .setAllocator(&allocator_)
              .addNextDuplex<MockHandler>(coro_source_tag, std::move(source))
              .addNextDuplex<MockHandler>(coro_target_tag, std::move(target))
              .build();
    });
    return pipeline;
  }

  PipelineImpl::Ptr buildChain(
      std::unique_ptr<MockHandler> a,
      std::unique_ptr<MockHandler> b,
      std::unique_ptr<MockHandler> c) {
    PipelineImpl::Ptr pipeline;
    eventBase_->runInEventBaseThreadAndWait([&] {
      pipeline =
          PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
              .setEventBase(eventBase_)
              .setHead(&head_)
              .setTail(&tail_)
              .setAllocator(&allocator_)
              .addNextDuplex<MockHandler>(coro_chain_a_tag, std::move(a))
              .addNextDuplex<MockHandler>(coro_chain_b_tag, std::move(b))
              .addNextDuplex<MockHandler>(coro_chain_c_tag, std::move(c))
              .build();
    });
    return pipeline;
  }

  folly::Executor* executorFor(CoroCompletionMode mode) {
    return mode == CoroCompletionMode::EventBase
        ? static_cast<folly::Executor*>(eventBase_)
        : static_cast<folly::Executor*>(&executor_);
  }

  void destroyPipeline(PipelineImpl::Ptr pipeline) {
    eventBase_->runInEventBaseThreadAndWait(
        [pipeline = std::move(pipeline)]() mutable { pipeline.reset(); });
  }

  folly::ScopedEventBaseThread eventBaseThread_;
  folly::EventBase* eventBase_{nullptr};
  MockHeadHandler head_;
  MockTailHandler tail_;
  TestAllocator allocator_;
  folly::CPUThreadPoolExecutor executor_{1};
  std::thread::id firstCoroutineThread_;
  std::thread::id secondCoroutineThread_;
};

class ContextHandleCoroChainTest
    : public ContextHandleCoroTest,
      public ::testing::WithParamInterface<CoroCompletionMode> {};

TEST_P(ContextHandleCoroChainTest, InboundNormalAsyncNormalChain) {
  std::vector<std::string> order;
  std::vector<std::string> tailTrace;
  std::optional<folly::SemiFuture<Result>> completion;
  std::thread::id coroutineThread;
  auto a = std::make_unique<MockHandler>();
  a->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    order.push_back("A");
    message.get<std::unique_ptr<TraceMessage>>()->trace.push_back("A");
    return ctx.fireRead(std::move(message));
  });
  auto b = std::make_unique<MockHandler>();
  b->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    order.push_back("B");
    message.get<std::unique_ptr<TraceMessage>>()->trace.push_back("B");
    completion =
        folly::coro::co_withExecutor(
            executorFor(GetParam()),
            asyncFireRead(
                coro::ContextHandle{ctx}, std::move(message), coroutineThread))
            .start();
    return Result::Success;
  });
  auto c = std::make_unique<MockHandler>();
  c->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    order.push_back("C");
    message.get<std::unique_ptr<TraceMessage>>()->trace.push_back("C");
    return ctx.fireRead(std::move(message));
  });
  tail_.setOnReadCallback([&](TypeErasedBox&& message) {
    order.push_back("Tail");
    tailTrace = message.get<std::unique_ptr<TraceMessage>>()->trace;
    return Result::Backpressure;
  });
  auto pipeline = buildChain(std::move(a), std::move(b), std::move(c));

  Result initial = Result::Error;
  eventBase_->runInEventBaseThreadAndWait([&] {
    initial =
        pipeline->fireRead(TypeErasedBox(std::make_unique<TraceMessage>()));
  });

  EXPECT_EQ(initial, Result::Success);
  ASSERT_TRUE(completion.has_value());
  EXPECT_EQ(std::move(*completion).get(), Result::Backpressure);
  EXPECT_EQ(order, (std::vector<std::string>{"A", "B", "C", "Tail"}));
  EXPECT_EQ(tailTrace, (std::vector<std::string>{"A", "B", "C"}));
  destroyPipeline(std::move(pipeline));
}

TEST_P(ContextHandleCoroChainTest, OutboundNormalAsyncNormalChain) {
  std::vector<std::string> order;
  std::vector<std::string> headTrace;
  std::optional<folly::SemiFuture<Result>> completion;
  auto a = std::make_unique<MockHandler>();
  a->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    order.push_back("A");
    message.get<std::unique_ptr<TraceMessage>>()->trace.push_back("A");
    return ctx.fireWrite(std::move(message));
  });
  auto b = std::make_unique<MockHandler>();
  b->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    order.push_back("B");
    message.get<std::unique_ptr<TraceMessage>>()->trace.push_back("B");
    completion =
        folly::coro::co_withExecutor(
            executorFor(GetParam()),
            asyncFireWrite(coro::ContextHandle{ctx}, std::move(message)))
            .start();
    return Result::Success;
  });
  auto c = std::make_unique<MockHandler>();
  c->setOnWrite([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    order.push_back("C");
    message.get<std::unique_ptr<TraceMessage>>()->trace.push_back("C");
    return ctx.fireWrite(std::move(message));
  });
  head_.setOnWriteCallback([&](TypeErasedBox&& message) {
    order.push_back("Head");
    headTrace = message.get<std::unique_ptr<TraceMessage>>()->trace;
    return Result::Backpressure;
  });
  auto pipeline = buildChain(std::move(a), std::move(b), std::move(c));

  Result initial = Result::Error;
  eventBase_->runInEventBaseThreadAndWait([&] {
    initial =
        pipeline->fireWrite(TypeErasedBox(std::make_unique<TraceMessage>()));
  });

  EXPECT_EQ(initial, Result::Success);
  ASSERT_TRUE(completion.has_value());
  EXPECT_EQ(std::move(*completion).get(), Result::Backpressure);
  EXPECT_EQ(order, (std::vector<std::string>{"C", "B", "A", "Head"}));
  EXPECT_EQ(headTrace, (std::vector<std::string>{"C", "B", "A"}));
  destroyPipeline(std::move(pipeline));
}

TEST_P(ContextHandleCoroChainTest, ExceptionNormalAsyncNormalChain) {
  std::vector<std::string> order;
  std::optional<folly::SemiFuture<folly::Unit>> completion;
  auto a = std::make_unique<MockHandler>();
  a->setOnException(
      [&](detail::ContextImpl& ctx, folly::exception_wrapper&& exception) {
        order.push_back("A");
        ctx.fireException(std::move(exception));
      });
  auto b = std::make_unique<MockHandler>();
  b->setOnException([&](detail::ContextImpl& ctx,
                        folly::exception_wrapper&& exception) {
    order.push_back("B");
    completion =
        folly::coro::co_withExecutor(
            executorFor(GetParam()),
            asyncFireException(coro::ContextHandle{ctx}, std::move(exception)))
            .start();
  });
  auto c = std::make_unique<MockHandler>();
  c->setOnException(
      [&](detail::ContextImpl& ctx, folly::exception_wrapper&& exception) {
        order.push_back("C");
        ctx.fireException(std::move(exception));
      });
  tail_.setOnExceptionCallback(
      [&](folly::exception_wrapper&&) { order.push_back("Tail"); });
  auto pipeline = buildChain(std::move(a), std::move(b), std::move(c));

  eventBase_->runInEventBaseThreadAndWait([&] {
    pipeline->fireException(
        folly::make_exception_wrapper<std::runtime_error>("test"));
  });

  ASSERT_TRUE(completion.has_value());
  std::move(*completion).get();
  EXPECT_EQ(order, (std::vector<std::string>{"A", "B", "C", "Tail"}));
  destroyPipeline(std::move(pipeline));
}

INSTANTIATE_TEST_SUITE_P(
    InlineAndWorker,
    ContextHandleCoroChainTest,
    ::testing::Values(
        CoroCompletionMode::EventBase, CoroCompletionMode::Worker));

TEST_F(ContextHandleCoroTest, HandlerAwaitsReadCompletionAndResult) {
  std::optional<folly::SemiFuture<Result>> completion;
  std::thread::id coroutineThread;
  std::thread::id downstreamThread;
  auto source = std::make_unique<MockHandler>();
  source->setOnRead([&](detail::ContextImpl& ctx,
                        TypeErasedBox&& message) noexcept {
    completion =
        folly::coro::co_withExecutor(
            &executor_,
            asyncFireRead(
                coro::ContextHandle{ctx}, std::move(message), coroutineThread))
            .start();
    return Result::Success;
  });
  auto target = std::make_unique<MockHandler>();
  target->setOnRead(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        downstreamThread = std::this_thread::get_id();
        return ctx.fireRead(std::move(message));
      });
  tail_.setReadResult(Result::Backpressure);
  auto pipeline = buildPipeline(std::move(source), std::move(target));

  Result initial = Result::Error;
  eventBase_->runInEventBaseThreadAndWait(
      [&] { initial = pipeline->fireRead(TypeErasedBox(42)); });

  EXPECT_EQ(initial, Result::Success);
  ASSERT_TRUE(completion.has_value());
  EXPECT_EQ(std::move(*completion).get(), Result::Backpressure);
  EXPECT_NE(coroutineThread, downstreamThread);
  EXPECT_EQ(tail_.readCount(), 1);
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleCoroTest, CoFireWriteReturnsDownstreamResult) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());
  head_.setWriteResult(Result::Backpressure);
  coro::ContextHandle handle{*pipeline->context(coro_target_tag)};

  auto completion =
      folly::coro::co_withExecutor(
          &executor_,
          asyncFireWrite(
              std::move(handle), TypeErasedBox(folly::IOBuf::create(8))))
          .start();

  EXPECT_EQ(std::move(completion).get(), Result::Backpressure);
  EXPECT_EQ(head_.writeCount(), 1);
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleCoroTest, MultipleIndependentCoFireReadsComplete) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());
  auto* context = pipeline->context(coro_source_tag);

  auto first = folly::coro::co_withExecutor(
                   &executor_,
                   asyncFireRead(
                       coro::ContextHandle{*context},
                       TypeErasedBox(1),
                       firstCoroutineThread_))
                   .start();
  auto second = folly::coro::co_withExecutor(
                    &executor_,
                    asyncFireRead(
                        coro::ContextHandle{*context},
                        TypeErasedBox(2),
                        secondCoroutineThread_))
                    .start();

  EXPECT_EQ(std::move(first).get(), Result::Success);
  EXPECT_EQ(std::move(second).get(), Result::Success);
  EXPECT_EQ(tail_.readCount(), 2);
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleCoroTest, SuspendedCoroutineRetainsReleasedPipeline) {
  std::atomic<int> removedCount{0};
  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { removedCount.fetch_add(1); });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  folly::Baton<> suspended;
  folly::coro::Baton resume;
  auto completion =
      folly::coro::co_withExecutor(
          &executor_,
          suspendedFireRead(
              coro::ContextHandle{*pipeline->context(coro_source_tag)},
              TypeErasedBox(42),
              suspended,
              resume))
          .start();
  suspended.wait();

  eventBase_->runInEventBaseThreadAndWait(
      [pipeline = std::move(pipeline)]() mutable { pipeline.reset(); });
  EXPECT_EQ(removedCount.load(), 0);

  resume.post();
  EXPECT_EQ(std::move(completion).get(), Result::Success);
  EXPECT_EQ(removedCount.load(), 1);
  EXPECT_EQ(tail_.readCount(), 1);
}

TEST(
    ContextHandleDirectAwaitableTest, CancellationRequestWhileSuspendedIsSafe) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<MockHandler>(
              coro_source_tag, std::make_unique<MockHandler>())
          .build();
  folly::CPUThreadPoolExecutor executor(1);
  folly::CancellationSource cancellation;
  folly::Baton<> started;
  auto task = folly::coro::co_withCancellation(
      cancellation.getToken(),
      asyncFireReadStarted(
          coro::ContextHandle{*pipeline->context(coro_source_tag)},
          TypeErasedBox(42),
          started));
  auto completion =
      folly::coro::co_withExecutor(&executor, std::move(task)).start();

  started.wait();
  cancellation.requestCancellation();
  eventBase.loopOnce();

  EXPECT_EQ(std::move(completion).get(), Result::Success);
  EXPECT_EQ(tail.readCount(), 1);
}

TEST(ContextHandleDirectAwaitableTest, DroppedFutureKeepsSuspendedFrameAlive) {
  folly::EventBase eventBase;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator allocator;
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&eventBase)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&allocator)
          .addNextDuplex<MockHandler>(
              coro_source_tag, std::make_unique<MockHandler>())
          .build();
  folly::CPUThreadPoolExecutor executor(1);
  folly::Baton<> started;

  {
    auto completion =
        folly::coro::co_withExecutor(
            &executor,
            asyncFireReadStarted(
                coro::ContextHandle{*pipeline->context(coro_source_tag)},
                TypeErasedBox(42),
                started))
            .start();
    started.wait();
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (eventBase.getNotificationQueueSize() == 0 &&
           std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    ASSERT_GT(eventBase.getNotificationQueueSize(), 0);
  }

  eventBase.loopOnce();
  EXPECT_EQ(tail.readCount(), 1);
}

TEST_F(ContextHandleCoroTest, SameEventBaseAwaitablesCompleteInline) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());
  head_.setWriteResult(Result::Backpressure);
  tail_.setReadResult(Result::Backpressure);

  eventBase_->runInEventBaseThreadAndWait([&] {
    const auto queuedBefore = eventBase_->getNotificationQueueSize();

    auto read =
        coro::ContextHandle{*pipeline->context(coro_source_tag)}.co_fireRead(
            TypeErasedBox(42));
    EXPECT_TRUE(read.await_ready());
    EXPECT_EQ(read.await_resume(), Result::Backpressure);

    auto write =
        coro::ContextHandle{*pipeline->context(coro_target_tag)}.co_fireWrite(
            TypeErasedBox(folly::IOBuf::create(8)));
    EXPECT_TRUE(write.await_ready());
    EXPECT_EQ(write.await_resume(), Result::Backpressure);

    auto exception =
        coro::ContextHandle{*pipeline->context(coro_source_tag)}
            .co_fireException(
                folly::make_exception_wrapper<std::runtime_error>("test"));
    EXPECT_TRUE(exception.await_ready());
    exception.await_resume();

    EXPECT_EQ(eventBase_->getNotificationQueueSize(), queuedBefore);
  });

  EXPECT_EQ(tail_.readCount(), 1);
  EXPECT_EQ(head_.writeCount(), 1);
  EXPECT_EQ(tail_.exceptionCount(), 1);
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleCoroTest, SameEventBaseClosedPipelineReturnsErrorInline) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());

  eventBase_->runInEventBaseThreadAndWait([&] {
    coro::ContextHandle handle{*pipeline->context(coro_source_tag)};
    pipeline->close();
    auto read = std::move(handle).co_fireRead(TypeErasedBox(42));

    EXPECT_TRUE(read.await_ready());
    EXPECT_EQ(read.await_resume(), Result::Error);
  });

  EXPECT_EQ(tail_.readCount(), 0);
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleCoroTest, CoFireExceptionAwaitsPropagation) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());
  coro::ContextHandle handle{*pipeline->context(coro_source_tag)};

  auto completion =
      folly::coro::co_withExecutor(
          &executor_,
          asyncFireException(
              std::move(handle),
              folly::make_exception_wrapper<std::runtime_error>("test")))
          .start();

  std::move(completion).get();
  EXPECT_EQ(tail_.exceptionCount(), 1);
  destroyPipeline(std::move(pipeline));
}

} // namespace
} // namespace apache::thrift::fast_thrift::channel_pipeline::test
