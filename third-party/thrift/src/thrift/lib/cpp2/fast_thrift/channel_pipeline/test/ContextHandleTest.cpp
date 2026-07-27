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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/ContextHandle.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {
namespace {

HANDLER_TAG(handle_source);
HANDLER_TAG(handle_target);
HANDLER_TAG(chain_a);
HANDLER_TAG(chain_b);
HANDLER_TAG(chain_c);

struct TraceMessage {
  std::vector<std::string> trace;
};

enum class CompletionMode { Inline, Worker };

void completeRead(
    CompletionMode mode,
    ContextHandle handle,
    TypeErasedBox message,
    std::thread& worker) {
  if (mode == CompletionMode::Inline) {
    std::move(handle).fireRead(std::move(message));
    return;
  }
  worker = std::thread(
      [handle = std::move(handle), message = std::move(message)]() mutable {
        std::move(handle).fireRead(std::move(message));
      });
}

void completeWrite(
    CompletionMode mode,
    ContextHandle handle,
    TypeErasedBox message,
    std::thread& worker) {
  if (mode == CompletionMode::Inline) {
    std::move(handle).fireWrite(std::move(message));
    return;
  }
  worker = std::thread(
      [handle = std::move(handle), message = std::move(message)]() mutable {
        std::move(handle).fireWrite(std::move(message));
      });
}

void completeException(
    CompletionMode mode,
    ContextHandle handle,
    folly::exception_wrapper exception,
    std::thread& worker) {
  if (mode == CompletionMode::Inline) {
    std::move(handle).fireException(std::move(exception));
    return;
  }
  worker = std::thread(
      [handle = std::move(handle), exception = std::move(exception)]() mutable {
        std::move(handle).fireException(std::move(exception));
      });
}

static_assert(!std::is_copy_constructible_v<ContextHandle>);
static_assert(!std::is_copy_assignable_v<ContextHandle>);
static_assert(std::is_nothrow_move_constructible_v<ContextHandle>);
static_assert(!std::is_move_assignable_v<ContextHandle>);

class ContextHandleTest : public ::testing::Test {
 protected:
  PipelineImpl::Ptr buildPipeline(
      std::unique_ptr<MockHandler> source,
      std::unique_ptr<MockHandler> target) {
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&eventBase_)
        .setHead(&head_)
        .setTail(&tail_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(handle_source_tag, std::move(source))
        .addNextDuplex<MockHandler>(handle_target_tag, std::move(target))
        .build();
  }

  PipelineImpl::Ptr buildChain(
      std::unique_ptr<MockHandler> a,
      std::unique_ptr<MockHandler> b,
      std::unique_ptr<MockHandler> c) {
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&eventBase_)
        .setHead(&head_)
        .setTail(&tail_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(chain_a_tag, std::move(a))
        .addNextDuplex<MockHandler>(chain_b_tag, std::move(b))
        .addNextDuplex<MockHandler>(chain_c_tag, std::move(c))
        .build();
  }

  folly::EventBase eventBase_;
  MockHeadHandler head_;
  MockTailHandler tail_;
  TestAllocator allocator_;
};

class ContextHandleChainTest
    : public ContextHandleTest,
      public ::testing::WithParamInterface<CompletionMode> {};

TEST_P(ContextHandleChainTest, InboundNormalAsyncNormalChain) {
  std::vector<std::string> order;
  std::vector<std::string> tailTrace;
  std::thread worker;
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
    completeRead(GetParam(), ContextHandle{ctx}, std::move(message), worker);
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
    return Result::Success;
  });
  auto pipeline = buildChain(std::move(a), std::move(b), std::move(c));

  EXPECT_EQ(
      pipeline->fireRead(TypeErasedBox(std::make_unique<TraceMessage>())),
      Result::Success);
  EXPECT_EQ(order, (std::vector<std::string>{"A", "B"}));
  if (worker.joinable()) {
    worker.join();
  }
  eventBase_.loopOnce();

  EXPECT_EQ(order, (std::vector<std::string>{"A", "B", "C", "Tail"}));
  EXPECT_EQ(tailTrace, (std::vector<std::string>{"A", "B", "C"}));
}

TEST_P(ContextHandleChainTest, OutboundNormalAsyncNormalChain) {
  std::vector<std::string> order;
  std::vector<std::string> headTrace;
  std::thread worker;
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
    completeWrite(GetParam(), ContextHandle{ctx}, std::move(message), worker);
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
    return Result::Success;
  });
  auto pipeline = buildChain(std::move(a), std::move(b), std::move(c));

  EXPECT_EQ(
      pipeline->fireWrite(TypeErasedBox(std::make_unique<TraceMessage>())),
      Result::Success);
  EXPECT_EQ(order, (std::vector<std::string>{"C", "B"}));
  if (worker.joinable()) {
    worker.join();
  }
  eventBase_.loopOnce();

  EXPECT_EQ(order, (std::vector<std::string>{"C", "B", "A", "Head"}));
  EXPECT_EQ(headTrace, (std::vector<std::string>{"C", "B", "A"}));
}

TEST_P(ContextHandleChainTest, ExceptionNormalAsyncNormalChain) {
  std::vector<std::string> order;
  std::thread worker;
  auto a = std::make_unique<MockHandler>();
  a->setOnException(
      [&](detail::ContextImpl& ctx, folly::exception_wrapper&& exception) {
        order.push_back("A");
        ctx.fireException(std::move(exception));
      });
  auto b = std::make_unique<MockHandler>();
  b->setOnException(
      [&](detail::ContextImpl& ctx, folly::exception_wrapper&& exception) {
        order.push_back("B");
        completeException(
            GetParam(), ContextHandle{ctx}, std::move(exception), worker);
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

  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_EQ(order, (std::vector<std::string>{"A", "B"}));
  if (worker.joinable()) {
    worker.join();
  }
  eventBase_.loopOnce();

  EXPECT_EQ(order, (std::vector<std::string>{"A", "B", "C", "Tail"}));
}

INSTANTIATE_TEST_SUITE_P(
    InlineAndWorker,
    ContextHandleChainTest,
    ::testing::Values(CompletionMode::Inline, CompletionMode::Worker));

TEST_F(
    ContextHandleTest, InboundHandlerMovesMessageToWorkerAndResumesPipeline) {
  std::thread worker;
  auto source = std::make_unique<MockHandler>();
  source->setOnRead(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        worker = std::thread([handle = ContextHandle{ctx},
                              message = std::move(message)]() mutable {
          std::move(handle).fireRead(std::move(message));
        });
        return Result::Success;
      });
  auto target = std::make_unique<MockHandler>();
  int received = 0;
  target->setOnRead(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        received = message.get<int>();
        return ctx.fireRead(std::move(message));
      });
  auto pipeline = buildPipeline(std::move(source), std::move(target));

  EXPECT_EQ(pipeline->fireRead(TypeErasedBox(42)), Result::Success);
  EXPECT_EQ(received, 0);
  worker.join();
  EXPECT_EQ(received, 0);

  eventBase_.loopOnce();
  EXPECT_EQ(received, 42);
  EXPECT_EQ(tail_.readCount(), 1);
}

TEST_F(
    ContextHandleTest, OutboundHandlerMovesMessageToWorkerAndResumesPipeline) {
  std::thread worker;
  auto source = std::make_unique<MockHandler>();
  auto* sourcePtr = source.get();
  auto target = std::make_unique<MockHandler>();
  target->setOnWrite(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        worker = std::thread([handle = ContextHandle{ctx},
                              message = std::move(message)]() mutable {
          std::move(handle).fireWrite(std::move(message));
        });
        return Result::Success;
      });
  auto pipeline = buildPipeline(std::move(source), std::move(target));

  EXPECT_EQ(
      pipeline->fireWrite(TypeErasedBox(folly::IOBuf::create(8))),
      Result::Success);
  EXPECT_EQ(sourcePtr->writeCount(), 0);
  worker.join();
  EXPECT_EQ(sourcePtr->writeCount(), 0);

  eventBase_.loopOnce();
  EXPECT_EQ(sourcePtr->writeCount(), 1);
  EXPECT_EQ(head_.writeCount(), 1);
}

TEST_F(
    ContextHandleTest, ExceptionHandlerMovesErrorToWorkerAndResumesPipeline) {
  std::thread worker;
  auto source = std::make_unique<MockHandler>();
  source->setOnException([&](detail::ContextImpl& ctx,
                             folly::exception_wrapper&& exception) noexcept {
    worker = std::thread([handle = ContextHandle{ctx},
                          exception = std::move(exception)]() mutable {
      std::move(handle).fireException(std::move(exception));
    });
  });
  auto target = std::make_unique<MockHandler>();
  auto* targetPtr = target.get();
  auto pipeline = buildPipeline(std::move(source), std::move(target));

  pipeline->fireException(
      folly::make_exception_wrapper<std::runtime_error>("test"));
  EXPECT_EQ(targetPtr->exceptionCount(), 0);
  worker.join();
  EXPECT_EQ(targetPtr->exceptionCount(), 0);

  eventBase_.loopOnce();
  EXPECT_EQ(targetPtr->exceptionCount(), 1);
  EXPECT_EQ(tail_.exceptionCount(), 1);
}

TEST_F(ContextHandleTest, FireReadSchedulesFromEventBaseThread) {
  auto target = std::make_unique<MockHandler>();
  auto* targetPtr = target.get();
  auto pipeline =
      buildPipeline(std::make_unique<MockHandler>(), std::move(target));

  ContextHandle{*pipeline->context(handle_source_tag)}.fireRead(
      TypeErasedBox(42));
  EXPECT_EQ(targetPtr->readCount(), 0);

  eventBase_.loopOnce();
  EXPECT_EQ(targetPtr->readCount(), 1);
  EXPECT_EQ(tail_.readCount(), 1);
}

TEST_F(ContextHandleTest, FireReadFromWorkerRunsOnEventBase) {
  auto target = std::make_unique<MockHandler>();
  bool ranOnEventBase = false;
  target->setOnRead(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        ranOnEventBase = ctx.eventBase()->isInEventBaseThread();
        return ctx.fireRead(std::move(message));
      });
  auto pipeline =
      buildPipeline(std::make_unique<MockHandler>(), std::move(target));
  ContextHandle handle{*pipeline->context(handle_source_tag)};

  std::thread worker([handle = std::move(handle)]() mutable {
    std::move(handle).fireRead(TypeErasedBox(42));
  });
  worker.join();
  EXPECT_FALSE(ranOnEventBase);

  eventBase_.loopOnce();
  EXPECT_TRUE(ranOnEventBase);
}

TEST_F(ContextHandleTest, FireWriteFromWorkerPreservesDirection) {
  auto source = std::make_unique<MockHandler>();
  auto* sourcePtr = source.get();
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  ContextHandle handle{*pipeline->context(handle_target_tag)};

  std::thread worker([handle = std::move(handle)]() mutable {
    std::move(handle).fireWrite(TypeErasedBox(folly::IOBuf::create(8)));
  });
  worker.join();
  eventBase_.loopOnce();

  EXPECT_EQ(sourcePtr->writeCount(), 1);
  EXPECT_EQ(head_.writeCount(), 1);
}

TEST_F(ContextHandleTest, FireExceptionFromWorkerPreservesDirection) {
  auto target = std::make_unique<MockHandler>();
  auto* targetPtr = target.get();
  auto pipeline =
      buildPipeline(std::make_unique<MockHandler>(), std::move(target));
  ContextHandle handle{*pipeline->context(handle_source_tag)};

  std::thread worker([handle = std::move(handle)]() mutable {
    std::move(handle).fireException(
        folly::make_exception_wrapper<std::runtime_error>("test"));
  });
  worker.join();
  eventBase_.loopOnce();

  EXPECT_EQ(targetPtr->exceptionCount(), 1);
  EXPECT_EQ(tail_.exceptionCount(), 1);
}

TEST_F(ContextHandleTest, PendingHandoffKeepsPipelineAlive) {
  int removedCount = 0;
  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  ContextHandle handle{*pipeline->context(handle_source_tag)};

  std::thread worker([handle = std::move(handle)]() mutable {
    std::move(handle).fireRead(TypeErasedBox(42));
  });
  worker.join();
  pipeline.release()->destroy();
  EXPECT_EQ(removedCount, 0);

  eventBase_.loopOnce();
  EXPECT_EQ(removedCount, 1);
}

TEST_F(ContextHandleTest, RunningTaskOutlivesPipelineOwnerScope) {
  int removedCount = 0;
  folly::Baton<> taskStarted;
  folly::Baton<> finishTask;
  std::thread worker;

  {
    auto source = std::make_unique<MockHandler>();
    source->setHandlerRemoved(
        [&](detail::ContextImpl&) noexcept { ++removedCount; });
    auto pipeline =
        buildPipeline(std::move(source), std::make_unique<MockHandler>());
    ContextHandle handle{*pipeline->context(handle_source_tag)};
    worker = std::thread(
        [handle = std::move(handle), &taskStarted, &finishTask]() mutable {
          taskStarted.post();
          finishTask.wait();
          std::move(handle).fireRead(TypeErasedBox(42));
        });
    taskStarted.wait();
    pipeline.reset();
    EXPECT_EQ(removedCount, 0);
  }

  EXPECT_EQ(removedCount, 0);
  finishTask.post();
  worker.join();
  EXPECT_EQ(removedCount, 0);
  eventBase_.loopOnce();
  EXPECT_EQ(removedCount, 1);
  EXPECT_EQ(tail_.readCount(), 1);
}

TEST_F(ContextHandleTest, PipelineWaitsForLastRunningTaskAcrossScopes) {
  int removedCount = 0;
  folly::Baton<> firstStarted;
  folly::Baton<> secondStarted;
  folly::Baton<> finishFirst;
  folly::Baton<> finishSecond;
  std::thread firstWorker;
  std::thread secondWorker;

  {
    auto source = std::make_unique<MockHandler>();
    source->setHandlerRemoved(
        [&](detail::ContextImpl&) noexcept { ++removedCount; });
    auto pipeline =
        buildPipeline(std::move(source), std::make_unique<MockHandler>());
    auto* context = pipeline->context(handle_source_tag);
    ContextHandle first{*context};
    ContextHandle second{*context};
    firstWorker = std::thread(
        [handle = std::move(first), &firstStarted, &finishFirst]() mutable {
          firstStarted.post();
          finishFirst.wait();
          std::move(handle).fireRead(TypeErasedBox(1));
        });
    secondWorker = std::thread(
        [handle = std::move(second), &secondStarted, &finishSecond]() mutable {
          secondStarted.post();
          finishSecond.wait();
          std::move(handle).fireRead(TypeErasedBox(2));
        });
    firstStarted.wait();
    secondStarted.wait();
    pipeline.reset();
  }

  EXPECT_EQ(removedCount, 0);
  finishFirst.post();
  firstWorker.join();
  eventBase_.loopOnce();
  EXPECT_EQ(removedCount, 0);
  EXPECT_EQ(tail_.readCount(), 1);

  finishSecond.post();
  secondWorker.join();
  eventBase_.loopOnce();
  EXPECT_EQ(removedCount, 1);
  EXPECT_EQ(tail_.readCount(), 2);
}

TEST_F(ContextHandleTest, MultiplePendingHandoffsReleasePipelineExactlyOnce) {
  int removedCount = 0;
  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  auto* context = pipeline->context(handle_source_tag);
  ContextHandle first{*context};
  ContextHandle second{*context};

  std::thread firstWorker([handle = std::move(first)]() mutable {
    std::move(handle).fireRead(TypeErasedBox(1));
  });
  std::thread secondWorker([handle = std::move(second)]() mutable {
    std::move(handle).fireRead(TypeErasedBox(2));
  });
  firstWorker.join();
  secondWorker.join();
  pipeline.release()->destroy();
  EXPECT_EQ(removedCount, 0);

  eventBase_.loop();
  EXPECT_EQ(removedCount, 1);
  EXPECT_EQ(tail_.readCount(), 2);
}

} // namespace
} // namespace apache::thrift::fast_thrift::channel_pipeline::test
