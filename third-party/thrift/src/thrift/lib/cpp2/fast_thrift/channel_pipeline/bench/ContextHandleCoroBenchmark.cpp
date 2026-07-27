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

#include <folly/Benchmark.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <thread>
#include <vector>

using namespace apache::thrift::fast_thrift::channel_pipeline;

namespace {

HANDLER_TAG(bench_coro_handle);

struct PassthroughHandler {
  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }
  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }
  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
  void onPipelineActive(detail::ContextImpl&) noexcept {}
  void onPipelineInactive(detail::ContextImpl&) noexcept {}
  void onReadReady(detail::ContextImpl&) noexcept {}
  void onWriteReady(detail::ContextImpl&) noexcept {}
  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

struct BenchHead {
  Result onWrite(TypeErasedBox&& message) noexcept {
    folly::doNotOptimizeAway(message);
    ++writes;
    return Result::Success;
  }
  void onReadReady() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  size_t writes{0};
};

struct BenchTail {
  Result onRead(TypeErasedBox&& message) noexcept {
    folly::doNotOptimizeAway(message);
    ++reads;
    if (completion != nullptr && reads == target) {
      completion->post();
    }
    return Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void onWriteReady() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  folly::Baton<>* completion{nullptr};
  size_t target{0};
  size_t reads{0};
};

struct BenchAllocator {
  BytesPtr allocate(size_t size) noexcept { return folly::IOBuf::create(size); }
  BytesPtr copyBuffer(const void* data, size_t size) noexcept {
    return folly::IOBuf::copyBuffer(data, size);
  }
};

using HandleVector = std::vector<ContextHandle>;
using MessageVector = std::vector<TypeErasedBox>;

folly::coro::Task<Result> coFireReadBatch(
    HandleVector handles, MessageVector messages) {
  Result result = Result::Success;
  for (size_t i = 0; i < handles.size(); ++i) {
    result = co_await coro::ContextHandle{std::move(handles[i])}.co_fireRead(
        std::move(messages[i]));
  }
  co_return result;
}

folly::coro::Task<Result> coFireWriteBatch(
    HandleVector handles, MessageVector messages) {
  Result result = Result::Success;
  for (size_t i = 0; i < handles.size(); ++i) {
    result = co_await coro::ContextHandle{std::move(handles[i])}.co_fireWrite(
        std::move(messages[i]));
  }
  co_return result;
}

struct Harness {
  Harness() {
    evb = evbThread.getEventBase();
    evb->runInEventBaseThreadAndWait([&] {
      pipeline = PipelineBuilder<BenchHead, BenchTail, BenchAllocator>()
                     .setEventBase(evb)
                     .setHead(&head)
                     .setTail(&tail)
                     .setAllocator(&allocator)
                     .addNextDuplex<PassthroughHandler>(bench_coro_handle_tag)
                     .build();
      context = pipeline->context(bench_coro_handle_tag);
    });
  }

  ~Harness() {
    evb->runInEventBaseThreadAndWait(
        [pipeline = std::move(pipeline)]() mutable { pipeline.reset(); });
  }

  HandleVector makeHandles(size_t count) {
    HandleVector handles;
    handles.reserve(count);
    evb->runInEventBaseThreadAndWait([&] {
      for (size_t i = 0; i < count; ++i) {
        handles.emplace_back(*context);
      }
    });
    return handles;
  }

  static MessageVector makeReadMessages(size_t count) {
    MessageVector messages;
    messages.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      messages.emplace_back(static_cast<int>(i));
    }
    return messages;
  }

  static MessageVector makeWriteMessages(size_t count) {
    MessageVector messages;
    messages.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      messages.emplace_back(folly::IOBuf::create(8));
    }
    return messages;
  }

  folly::ScopedEventBaseThread evbThread;
  folly::EventBase* evb{nullptr};
  BenchHead head;
  BenchTail tail;
  BenchAllocator allocator;
  PipelineImpl::Ptr pipeline;
  detail::ContextImpl* context{nullptr};
  folly::CPUThreadPoolExecutor cpu{1};
};

} // namespace

BENCHMARK(ContextHandle_BatchedFireRead, iters) {
  folly::BenchmarkSuspender suspender;
  Harness harness;
  const auto count = static_cast<size_t>(iters);
  auto handles = harness.makeHandles(count);
  auto messages = Harness::makeReadMessages(count);
  folly::Baton<> completed;
  harness.tail.completion = &completed;
  harness.tail.target = count;
  suspender.dismiss();

  harness.evb->runInEventBaseThreadAndWait([&] {
    for (size_t i = 0; i < count; ++i) {
      std::move(handles[i]).fireRead(std::move(messages[i]));
    }
  });
  completed.wait();
  folly::doNotOptimizeAway(harness.tail.reads);
}

BENCHMARK(CoroContextHandle_BatchedCoFireRead_EventBase, iters) {
  folly::BenchmarkSuspender suspender;
  Harness harness;
  const auto count = static_cast<size_t>(iters);
  auto handles = harness.makeHandles(count);
  auto messages = Harness::makeReadMessages(count);
  suspender.dismiss();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          harness.evb,
          coFireReadBatch(std::move(handles), std::move(messages))));
  folly::doNotOptimizeAway(result);
  folly::doNotOptimizeAway(harness.tail.reads);
}

BENCHMARK(CoroContextHandle_BatchedCoFireRead_CPU, iters) {
  folly::BenchmarkSuspender suspender;
  Harness harness;
  const auto count = static_cast<size_t>(iters);
  auto handles = harness.makeHandles(count);
  auto messages = Harness::makeReadMessages(count);
  suspender.dismiss();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          &harness.cpu,
          coFireReadBatch(std::move(handles), std::move(messages))));
  folly::doNotOptimizeAway(result);
  folly::doNotOptimizeAway(harness.tail.reads);
}

BENCHMARK(CoroContextHandle_BatchedCoFireWrite_EventBase, iters) {
  folly::BenchmarkSuspender suspender;
  Harness harness;
  const auto count = static_cast<size_t>(iters);
  auto handles = harness.makeHandles(count);
  auto messages = Harness::makeWriteMessages(count);
  suspender.dismiss();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          harness.evb,
          coFireWriteBatch(std::move(handles), std::move(messages))));
  folly::doNotOptimizeAway(result);
  folly::doNotOptimizeAway(harness.head.writes);
}

int main(int argc, char** argv) {
  folly::gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::atomic<bool> finished{false};
  std::thread watchdog([&] {
    constexpr auto kTimeout = std::chrono::minutes(5);
    const auto deadline = std::chrono::steady_clock::now() + kTimeout;
    while (!finished.load(std::memory_order_acquire) &&
           std::chrono::steady_clock::now() < deadline) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if (!finished.load(std::memory_order_acquire)) {
      std::abort();
    }
  });

  folly::runBenchmarks();
  finished.store(true, std::memory_order_release);
  watchdog.join();
  return 0;
}
