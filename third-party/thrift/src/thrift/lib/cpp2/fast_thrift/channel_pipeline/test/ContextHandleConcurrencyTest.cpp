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

#include <algorithm>
#include <array>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {
namespace {

HANDLER_TAG(concurrency_source);
HANDLER_TAG(concurrency_middle);
HANDLER_TAG(concurrency_target);
HANDLER_TAG(chain_x);
HANDLER_TAG(chain_y);
HANDLER_TAG(chain_z);

enum class PropagationKind { Success, Backpressure, Error };

folly::coro::Task<Result> namedCoroFireRead(
    coro::ContextHandle handle, TypeErasedBox message) {
  co_return co_await std::move(handle).co_fireRead(std::move(message));
}

class ContextHandleConcurrencyTest : public ::testing::Test {
 protected:
  void SetUp() override { eventBase_ = eventBaseThread_.getEventBase(); }

  PipelineImpl::Ptr buildChain(
      std::unique_ptr<MockHandler> x,
      std::unique_ptr<MockHandler> y,
      std::unique_ptr<MockHandler> z) {
    PipelineImpl::Ptr pipeline;
    eventBase_->runInEventBaseThreadAndWait([&] {
      pipeline =
          PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
              .setEventBase(eventBase_)
              .setHead(&head_)
              .setTail(&tail_)
              .setAllocator(&allocator_)
              .addNextDuplex<MockHandler>(chain_x_tag, std::move(x))
              .addNextDuplex<MockHandler>(chain_y_tag, std::move(y))
              .addNextDuplex<MockHandler>(chain_z_tag, std::move(z))
              .build();
    });
    return pipeline;
  }

  PipelineImpl::Ptr buildPipeline(
      std::unique_ptr<MockHandler> source,
      std::unique_ptr<MockHandler> middle,
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
                  concurrency_source_tag, std::move(source))
              .addNextDuplex<MockHandler>(
                  concurrency_middle_tag, std::move(middle))
              .addNextDuplex<MockHandler>(
                  concurrency_target_tag, std::move(target))
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
  folly::CPUThreadPoolExecutor executor_{4};
};

TEST_F(
    ContextHandleConcurrencyTest,
    BaseHandlePropagatesSuccessBackpressureError) {
  for (PropagationKind kind :
       {PropagationKind::Success,
        PropagationKind::Backpressure,
        PropagationKind::Error}) {
    head_.reset();
    tail_.reset();
    auto source = std::make_unique<MockHandler>();
    auto middle = std::make_unique<MockHandler>();
    auto target = std::make_unique<MockHandler>();

    std::atomic<int> endpointCount{0};
    folly::Baton<> endpointDelivered;
    tail_.setOnReadCallback([&](TypeErasedBox&&) {
      endpointCount.fetch_add(1, std::memory_order_relaxed);
      endpointDelivered.post();
      if (kind == PropagationKind::Backpressure) {
        return Result::Backpressure;
      }
      return kind == PropagationKind::Error ? Result::Error : Result::Success;
    });

    middle->setOnRead(
        [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
          ContextHandle{ctx}.fireRead(std::move(message));
          return Result::Success;
        });

    auto pipeline =
        buildPipeline(std::move(source), std::move(middle), std::move(target));

    eventBase_->runInEventBaseThreadAndWait(
        [&] { (void)pipeline->fireRead(TypeErasedBox(1)); });

    endpointDelivered.wait();
    EXPECT_EQ(endpointCount.load(), 1);
    EXPECT_EQ(tail_.readCount(), 1);
    destroyPipeline(std::move(pipeline));
  }
}

TEST_F(
    ContextHandleConcurrencyTest,
    CoroHandleReturnsResultForSuccessBackpressureError) {
  for (PropagationKind kind :
       {PropagationKind::Success,
        PropagationKind::Backpressure,
        PropagationKind::Error}) {
    head_.reset();
    tail_.reset();
    auto source = std::make_unique<MockHandler>();
    auto middle = std::make_unique<MockHandler>();
    auto target = std::make_unique<MockHandler>();
    const auto expected = kind == PropagationKind::Backpressure
        ? Result::Backpressure
        : (kind == PropagationKind::Error ? Result::Error : Result::Success);
    tail_.setReadResult(expected);

    auto pipeline =
        buildPipeline(std::move(source), std::move(middle), std::move(target));

    std::optional<folly::SemiFuture<Result>> completion;
    eventBase_->runInEventBaseThreadAndWait([&] {
      auto* ctx = pipeline->context(concurrency_middle_tag);
      completion =
          folly::coro::co_withExecutor(
              &executor_,
              namedCoroFireRead(coro::ContextHandle{*ctx}, TypeErasedBox(1)))
              .start();
    });

    ASSERT_TRUE(completion.has_value());
    EXPECT_EQ(std::move(*completion).get(), expected);
    destroyPipeline(std::move(pipeline));
  }
}

TEST_F(ContextHandleConcurrencyTest, FifoOrderPreservedAcrossWorkers) {
  head_.reset();
  tail_.reset();
  constexpr int kCount = 8;
  std::vector<int> observed;
  std::mutex observedMutex;
  folly::Baton<> allDelivered;

  auto x = std::make_unique<MockHandler>();
  auto y = std::make_unique<MockHandler>();
  auto z = std::make_unique<MockHandler>();

  tail_.setOnReadCallback([&](TypeErasedBox&& message) {
    {
      std::lock_guard<std::mutex> lock(observedMutex);
      observed.push_back(message.get<int>());
      if (observed.size() == kCount) {
        allDelivered.post();
      }
    }
    return Result::Success;
  });

  y->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
    int seq = message.get<int>();
    std::thread worker([handle = ContextHandle{ctx}, seq]() mutable {
      std::move(handle).fireRead(TypeErasedBox(seq));
    });
    worker.join();
    return Result::Success;
  });

  auto pipeline = buildChain(std::move(x), std::move(y), std::move(z));

  for (int i = 0; i < kCount; ++i) {
    eventBase_->runInEventBaseThreadAndWait(
        [&] { (void)pipeline->fireRead(TypeErasedBox(i)); });
  }

  allDelivered.wait();

  const std::vector<int> expected{0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_EQ(observed, expected);
  destroyPipeline(std::move(pipeline));
}

TEST_F(
    ContextHandleConcurrencyTest,
    OutOfOrderCompletionDeliversExactlyOnceWithoutOrderingGuarantee) {
  head_.reset();
  tail_.reset();
  std::vector<int> observed;
  std::mutex observedMutex;
  std::array<folly::Baton<>, 3> batons;
  std::array<std::thread, 3> workers;
  folly::Baton<> allDelivered;
  std::atomic<int> delivered{0};

  auto x = std::make_unique<MockHandler>();
  auto y = std::make_unique<MockHandler>();
  auto z = std::make_unique<MockHandler>();

  tail_.setOnReadCallback([&](TypeErasedBox&& message) {
    {
      std::lock_guard<std::mutex> lock(observedMutex);
      observed.push_back(message.get<int>());
    }
    if (delivered.fetch_add(1) + 1 == 3) {
      allDelivered.post();
    }
    return Result::Success;
  });

  y->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
    int seq = message.get<int>();
    workers[seq] =
        std::thread([handle = ContextHandle{ctx}, seq, &batons]() mutable {
          batons[seq].wait();
          std::move(handle).fireRead(TypeErasedBox(seq));
        });
    return Result::Success;
  });

  auto pipeline = buildChain(std::move(x), std::move(y), std::move(z));

  for (int i = 0; i < 3; ++i) {
    eventBase_->runInEventBaseThreadAndWait(
        [&] { (void)pipeline->fireRead(TypeErasedBox(i)); });
  }

  batons[2].post();
  workers[2].join();
  batons[1].post();
  workers[1].join();
  batons[0].post();
  workers[0].join();
  allDelivered.wait();

  std::sort(observed.begin(), observed.end());
  EXPECT_EQ(observed, (std::vector<int>{0, 1, 2}));
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleConcurrencyTest, SimultaneousReadWrite) {
  head_.reset();
  tail_.reset();
  std::atomic<int> readObserved{0};
  std::atomic<int> writeObserved{0};
  folly::Baton<> bothDelivered;
  std::atomic<int> delivered{0};
  std::thread readWorker;
  std::thread writeWorker;

  auto source = std::make_unique<MockHandler>();
  auto middle = std::make_unique<MockHandler>();
  auto target = std::make_unique<MockHandler>();

  tail_.setOnReadCallback([&](TypeErasedBox&&) {
    readObserved.fetch_add(1);
    if (delivered.fetch_add(1) + 1 == 2) {
      bothDelivered.post();
    }
    return Result::Success;
  });
  head_.setOnWriteCallback([&](TypeErasedBox&&) {
    writeObserved.fetch_add(1);
    if (delivered.fetch_add(1) + 1 == 2) {
      bothDelivered.post();
    }
    return Result::Success;
  });

  middle->setOnRead(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        readWorker = std::thread(
            [handle = ContextHandle{ctx}, msg = std::move(message)]() mutable {
              std::move(handle).fireRead(std::move(msg));
            });
        return Result::Success;
      });
  middle->setOnWrite(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        writeWorker = std::thread(
            [handle = ContextHandle{ctx}, msg = std::move(message)]() mutable {
              std::move(handle).fireWrite(std::move(msg));
            });
        return Result::Success;
      });

  auto pipeline =
      buildPipeline(std::move(source), std::move(middle), std::move(target));

  eventBase_->runInEventBaseThreadAndWait([&] {
    (void)pipeline->fireRead(TypeErasedBox(1));
    (void)pipeline->fireWrite(TypeErasedBox(folly::IOBuf::create(8)));
  });

  readWorker.join();
  writeWorker.join();
  bothDelivered.wait();

  EXPECT_EQ(readObserved.load(), 1);
  EXPECT_EQ(writeObserved.load(), 1);
  destroyPipeline(std::move(pipeline));
}

TEST_F(ContextHandleConcurrencyTest, SimultaneousReadException) {
  head_.reset();
  tail_.reset();
  std::atomic<int> readObserved{0};
  std::atomic<int> exceptionObserved{0};
  std::atomic<int> delivered{0};
  folly::Baton<> bothDelivered;
  std::thread readWorker;
  std::thread exceptionWorker;

  auto source = std::make_unique<MockHandler>();
  auto middle = std::make_unique<MockHandler>();
  auto target = std::make_unique<MockHandler>();

  tail_.setOnReadCallback([&](TypeErasedBox&&) {
    readObserved.fetch_add(1);
    if (delivered.fetch_add(1) + 1 == 2) {
      bothDelivered.post();
    }
    return Result::Success;
  });
  tail_.setOnExceptionCallback([&](folly::exception_wrapper&&) {
    exceptionObserved.fetch_add(1);
    if (delivered.fetch_add(1) + 1 == 2) {
      bothDelivered.post();
    }
  });

  middle->setOnRead(
      [&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
        readWorker = std::thread(
            [handle = ContextHandle{ctx}, msg = std::move(message)]() mutable {
              std::move(handle).fireRead(std::move(msg));
            });
        return Result::Success;
      });
  middle->setOnException(
      [&](detail::ContextImpl& ctx, folly::exception_wrapper&& ex) noexcept {
        exceptionWorker = std::thread(
            [handle = ContextHandle{ctx}, ex = std::move(ex)]() mutable {
              std::move(handle).fireException(std::move(ex));
            });
      });

  auto pipeline =
      buildPipeline(std::move(source), std::move(middle), std::move(target));

  eventBase_->runInEventBaseThreadAndWait([&] {
    (void)pipeline->fireRead(TypeErasedBox(1));
    pipeline->fireException(
        folly::make_exception_wrapper<std::runtime_error>("test"));
  });

  readWorker.join();
  exceptionWorker.join();
  bothDelivered.wait();

  EXPECT_EQ(readObserved.load(), 1);
  EXPECT_EQ(exceptionObserved.load(), 1);
  destroyPipeline(std::move(pipeline));
}

TEST_F(
    ContextHandleConcurrencyTest, BoundedSixteenHandleMultiWorkerExactlyOnce) {
  head_.reset();
  tail_.reset();
  constexpr int kHandles = 16;
  std::atomic<int> deliveryCount{0};
  folly::Baton<> allDelivered;
  std::array<std::thread, kHandles> workers;
  std::array<std::atomic<bool>, kHandles> seen{};
  for (auto& s : seen) {
    s.store(false);
  }

  auto x = std::make_unique<MockHandler>();
  auto y = std::make_unique<MockHandler>();
  auto z = std::make_unique<MockHandler>();

  tail_.setOnReadCallback([&](TypeErasedBox&& message) {
    int seq = message.get<int>();
    bool expected = false;
    EXPECT_TRUE(
        seen[seq].compare_exchange_strong(
            expected, true, std::memory_order_relaxed));
    if (deliveryCount.fetch_add(1, std::memory_order_relaxed) + 1 == kHandles) {
      allDelivered.post();
    }
    return Result::Success;
  });

  y->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
    int seq = message.get<int>();
    workers[seq] = std::thread([handle = ContextHandle{ctx}, seq]() mutable {
      std::move(handle).fireRead(TypeErasedBox(seq));
    });
    return Result::Success;
  });

  auto pipeline = buildChain(std::move(x), std::move(y), std::move(z));

  for (int i = 0; i < kHandles; ++i) {
    eventBase_->runInEventBaseThreadAndWait(
        [&] { (void)pipeline->fireRead(TypeErasedBox(i)); });
  }

  for (auto& worker : workers) {
    worker.join();
  }
  allDelivered.wait();

  EXPECT_EQ(deliveryCount.load(), kHandles);
  EXPECT_TRUE(std::all_of(seen.begin(), seen.end(), [](const auto& delivered) {
    return delivered.load();
  }));
  destroyPipeline(std::move(pipeline));
}

TEST_F(
    ContextHandleConcurrencyTest,
    CoroHandlesAcrossWorkersDeliverExactlyOnceWithoutOrderingGuarantee) {
  head_.reset();
  tail_.reset();
  constexpr int kCount = 8;
  std::vector<int> observed;
  std::mutex m;
  std::vector<folly::SemiFuture<Result>> completions;

  tail_.setOnReadCallback([&](TypeErasedBox&& msg) {
    std::lock_guard<std::mutex> lock(m);
    observed.push_back(msg.get<int>());
    return Result::Success;
  });

  auto x = std::make_unique<MockHandler>();
  auto y = std::make_unique<MockHandler>();
  auto z = std::make_unique<MockHandler>();
  y->setOnRead([&](detail::ContextImpl& ctx, TypeErasedBox&& message) noexcept {
    completions.push_back(
        folly::coro::co_withExecutor(
            &executor_,
            namedCoroFireRead(coro::ContextHandle{ctx}, std::move(message)))
            .start());
    return Result::Success;
  });

  auto pipeline = buildChain(std::move(x), std::move(y), std::move(z));
  for (int i = 0; i < kCount; ++i) {
    eventBase_->runInEventBaseThreadAndWait(
        [&] { (void)pipeline->fireRead(TypeErasedBox(i)); });
  }
  ASSERT_EQ(completions.size(), kCount);
  for (auto& completion : completions) {
    EXPECT_EQ(std::move(completion).get(), Result::Success);
  }
  std::sort(observed.begin(), observed.end());
  const std::vector<int> expected{0, 1, 2, 3, 4, 5, 6, 7};
  EXPECT_EQ(observed, expected);
  destroyPipeline(std::move(pipeline));
}

} // namespace
} // namespace apache::thrift::fast_thrift::channel_pipeline::test
