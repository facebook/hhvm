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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/CoroContextHandle.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

#include <atomic>
#include <csignal>
#include <future>
#include <thread>
#include <type_traits>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {
namespace {

class WholeBinaryWatchdog final : public ::testing::Environment {
 public:
  void SetUp() override {
    std::signal(SIGALRM, [](int) { std::abort(); });
    alarm(300);
  }

  void TearDown() override { alarm(0); }
};

const auto* const kWholeBinaryWatchdog =
    ::testing::AddGlobalTestEnvironment(new WholeBinaryWatchdog());

HANDLER_TAG(lifecycle_source);
HANDLER_TAG(lifecycle_target);
HANDLER_TAG(lifecycle_a);
HANDLER_TAG(lifecycle_b);
HANDLER_TAG(lifecycle_c);

// One-shot structural type traits
static_assert(!std::is_copy_constructible_v<ContextHandle>);
static_assert(!std::is_copy_assignable_v<ContextHandle>);
static_assert(std::is_nothrow_move_constructible_v<ContextHandle>);
static_assert(!std::is_move_assignable_v<ContextHandle>);

static_assert(!std::is_copy_constructible_v<coro::ContextHandle>);
static_assert(!std::is_copy_assignable_v<coro::ContextHandle>);
static_assert(std::is_nothrow_move_constructible_v<coro::ContextHandle>);
static_assert(!std::is_move_assignable_v<coro::ContextHandle>);

folly::coro::Task<Result> suspendedCoroFireRead(
    coro::ContextHandle handle,
    TypeErasedBox message,
    folly::Baton<>& suspended,
    folly::coro::Baton& resume) {
  suspended.post();
  co_await resume;
  co_return co_await std::move(handle).co_fireRead(std::move(message));
}

class ContextHandleLifecycleTest : public ::testing::Test {
 protected:
  PipelineImpl::Ptr buildPipeline(
      std::unique_ptr<MockHandler> source,
      std::unique_ptr<MockHandler> target) {
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&eventBase_)
        .setHead(&head_)
        .setTail(&tail_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(lifecycle_source_tag, std::move(source))
        .addNextDuplex<MockHandler>(lifecycle_target_tag, std::move(target))
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
        .addNextDuplex<MockHandler>(lifecycle_a_tag, std::move(a))
        .addNextDuplex<MockHandler>(lifecycle_b_tag, std::move(b))
        .addNextDuplex<MockHandler>(lifecycle_c_tag, std::move(c))
        .build();
  }

  folly::EventBase eventBase_;
  MockHeadHandler head_;
  MockTailHandler tail_;
  TestAllocator allocator_;
};

class ContextHandleCoroLifecycleTest : public ::testing::Test {
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
              .addNextDuplex<MockHandler>(
                  lifecycle_source_tag, std::move(source))
              .addNextDuplex<MockHandler>(
                  lifecycle_target_tag, std::move(target))
              .build();
    });
    return pipeline;
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
};

// Base handle: owner release while task outstanding
TEST_F(ContextHandleLifecycleTest, BaseHandleOwnerReleaseWhileTaskOutstanding) {
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
    ContextHandle handle{*pipeline->context(lifecycle_source_tag)};
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

// Coro handle: owner release while coroutine outstanding
TEST_F(
    ContextHandleCoroLifecycleTest,
    CoroHandleOwnerReleaseWhileCoroutineOutstanding) {
  std::atomic<int> removedCount{0};
  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  folly::Baton<> suspended;
  folly::coro::Baton resume;
  auto completion =
      folly::coro::co_withExecutor(
          &executor_,
          suspendedCoroFireRead(
              coro::ContextHandle{*pipeline->context(lifecycle_source_tag)},
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
  // Drain EventBase: ContextHandle destruction via DelayedDestruction is
  // dispatched back to the EventBase thread after the coro future resolves,
  // so callHandlerRemovedImpl runs on T1 concurrently with this read.
  eventBase_->runInEventBaseThreadAndWait([] {});
  EXPECT_EQ(removedCount.load(), 1);
  EXPECT_EQ(tail_.readCount(), 1);
}

// Handles in multiple handlers
TEST_F(ContextHandleLifecycleTest, HandlesInMultipleHandlers) {
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
    auto* context = pipeline->context(lifecycle_source_tag);
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

// Nested launcher scopes
TEST_F(ContextHandleLifecycleTest, NestedLauncherScopes) {
  int removedCount = 0;
  folly::Baton<> outerStarted;
  folly::Baton<> innerStarted;
  folly::Baton<> finishOuter;
  folly::Baton<> finishInner;
  std::thread outerWorker;
  std::thread innerWorker;

  {
    auto source = std::make_unique<MockHandler>();
    source->setHandlerRemoved(
        [&](detail::ContextImpl&) noexcept { ++removedCount; });
    auto pipeline =
        buildPipeline(std::move(source), std::make_unique<MockHandler>());
    auto* context = pipeline->context(lifecycle_source_tag);

    // Outer scope creates first handle
    ContextHandle outer{*context};
    outerWorker = std::thread(
        [handle = std::move(outer), &outerStarted, &finishOuter]() mutable {
          outerStarted.post();
          finishOuter.wait();
          std::move(handle).fireRead(TypeErasedBox(1));
        });

    // Inner nested scope creates second handle
    {
      ContextHandle inner{*context};
      innerWorker = std::thread(
          [handle = std::move(inner), &innerStarted, &finishInner]() mutable {
            innerStarted.post();
            finishInner.wait();
            std::move(handle).fireRead(TypeErasedBox(2));
          });
    }

    outerStarted.wait();
    innerStarted.wait();
    pipeline.reset();
  }

  EXPECT_EQ(removedCount, 0);
  finishOuter.post();
  outerWorker.join();
  eventBase_.loopOnce();
  EXPECT_EQ(removedCount, 0);
  EXPECT_EQ(tail_.readCount(), 1);

  finishInner.post();
  innerWorker.join();
  eventBase_.loopOnce();
  EXPECT_EQ(removedCount, 1);
  EXPECT_EQ(tail_.readCount(), 2);
}

// Pipeline close before completion - safe expected behavior
TEST_F(ContextHandleLifecycleTest, PipelineCloseBeforeCompletion) {
  int removedCount = 0;
  folly::Baton<> taskStarted;
  folly::Baton<> finishTask;
  std::thread worker;

  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  ContextHandle handle{*pipeline->context(lifecycle_source_tag)};
  worker = std::thread(
      [handle = std::move(handle), &taskStarted, &finishTask]() mutable {
        taskStarted.post();
        finishTask.wait();
        std::move(handle).fireRead(TypeErasedBox(42));
      });
  taskStarted.wait();

  // Close pipeline while task is outstanding
  pipeline->close();
  EXPECT_EQ(removedCount, 1);

  finishTask.post();
  worker.join();
  eventBase_.loopOnce();
  // After close, fireRead should return Error and not propagate
  EXPECT_EQ(tail_.readCount(), 0);
}

// Coro: pipeline close before completion
TEST_F(ContextHandleCoroLifecycleTest, CoroPipelineCloseBeforeCompletion) {
  int removedCount = 0;
  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  folly::Baton<> suspended;
  folly::coro::Baton resume;
  auto completion =
      folly::coro::co_withExecutor(
          &executor_,
          suspendedCoroFireRead(
              coro::ContextHandle{*pipeline->context(lifecycle_source_tag)},
              TypeErasedBox(42),
              suspended,
              resume))
          .start();
  suspended.wait();

  eventBase_->runInEventBaseThreadAndWait([&] { pipeline->close(); });
  EXPECT_EQ(removedCount, 1);

  resume.post();
  EXPECT_EQ(std::move(completion).get(), Result::Error);
  EXPECT_EQ(tail_.readCount(), 0);
  destroyPipeline(std::move(pipeline));
}

// Downstream close during resumed callback
TEST_F(ContextHandleLifecycleTest, DownstreamCloseDuringResumedCallback) {
  int removedCount = 0;
  folly::Baton<> taskStarted;
  folly::Baton<> finishTask;
  std::thread worker;

  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto target = std::make_unique<MockHandler>();
  target->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) {
    // Close pipeline during callback
    ctx.close();
    return ctx.fireRead(std::move(message));
  });
  auto pipeline = buildPipeline(std::move(source), std::move(target));
  ContextHandle handle{*pipeline->context(lifecycle_source_tag)};
  worker = std::thread(
      [handle = std::move(handle), &taskStarted, &finishTask]() mutable {
        taskStarted.post();
        finishTask.wait();
        std::move(handle).fireRead(TypeErasedBox(42));
      });
  taskStarted.wait();
  finishTask.post();
  worker.join();
  eventBase_.loopOnce();

  EXPECT_EQ(removedCount, 1);
  // The current callback explicitly forwards after close, so it completes.
  EXPECT_EQ(tail_.readCount(), 1);
}

// Endpoint teardown ordering with mocks
TEST_F(ContextHandleLifecycleTest, EndpointTeardownOrdering) {
  std::vector<std::string> teardownOrder;
  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved([&](detail::ContextImpl&) noexcept {
    teardownOrder.emplace_back("source");
  });
  auto target = std::make_unique<MockHandler>();
  target->setHandlerRemoved([&](detail::ContextImpl&) noexcept {
    teardownOrder.emplace_back("target");
  });
  head_.setOnWriteCallback([&](TypeErasedBox&&) {
    teardownOrder.emplace_back("head_write");
    return Result::Success;
  });
  tail_.setOnReadCallback([&](TypeErasedBox&&) {
    teardownOrder.emplace_back("tail_read");
    return Result::Success;
  });

  auto pipeline = buildPipeline(std::move(source), std::move(target));
  pipeline->close();

  // Mocks record internal handler removal, which runs in reverse order.
  const std::vector<std::string> expected{"target", "source"};
  EXPECT_EQ(teardownOrder, expected);
}

// Move ownership / moved-from destruction
TEST_F(ContextHandleLifecycleTest, MoveOwnershipMovedFromDestruction) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());
  auto* context = pipeline->context(lifecycle_source_tag);
  ContextHandle original{*context};
  std::promise<ContextHandle> transfer;
  auto moved = transfer.get_future();

  std::thread worker([original = std::move(original),
                      transfer = std::move(transfer)]() mutable {
    transfer.set_value(std::move(original));
    // The moved-from capture is destroyed on this worker thread.
  });
  worker.join();

  // The live handle must be destroyed on the EventBase thread.
  eventBase_.runInEventBaseThread([moved = moved.get()]() mutable {});
  eventBase_.loopOnce();
  pipeline->close();
}

// Coro move ownership
TEST_F(ContextHandleCoroLifecycleTest, CoroMoveOwnershipMovedFromDestruction) {
  auto pipeline = buildPipeline(
      std::make_unique<MockHandler>(), std::make_unique<MockHandler>());
  eventBase_->runInEventBaseThreadAndWait([&] {
    auto* context = pipeline->context(lifecycle_source_tag);
    coro::ContextHandle original{*context};
    coro::ContextHandle moved{std::move(original)};
    // moved-from destroyed here on EventBase thread - safe
  });
  destroyPipeline(std::move(pipeline));
}

// Multiple handles with close interleaving
TEST_F(ContextHandleLifecycleTest, MultipleHandlesWithCloseInterleaving) {
  int removedCount = 0;
  folly::Baton<> firstStarted;
  folly::Baton<> secondStarted;
  folly::Baton<> finishFirst;
  folly::Baton<> finishSecond;
  std::thread firstWorker;
  std::thread secondWorker;

  auto source = std::make_unique<MockHandler>();
  source->setHandlerRemoved(
      [&](detail::ContextImpl&) noexcept { ++removedCount; });
  auto pipeline =
      buildPipeline(std::move(source), std::make_unique<MockHandler>());
  auto* context = pipeline->context(lifecycle_source_tag);
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

  // Close while both handles outstanding
  pipeline->close();
  EXPECT_EQ(removedCount, 1);

  finishFirst.post();
  firstWorker.join();
  finishSecond.post();
  secondWorker.join();
  eventBase_.loopOnce();
  // Both fires should return Error after close
  EXPECT_EQ(tail_.readCount(), 0);
}

} // namespace
} // namespace apache::thrift::fast_thrift::channel_pipeline::test
