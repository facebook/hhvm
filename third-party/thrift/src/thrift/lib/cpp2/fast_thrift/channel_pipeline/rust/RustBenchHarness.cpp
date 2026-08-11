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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustBenchHarness.h>
#include <thrift/lib/rust/channel_pipeline/src/bench.rs.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include <folly/BenchmarkUtil.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/memory/MallctlHelper.h>
#include <folly/memory/Malloc.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustHandler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/rust/RustMessageAdapter.h>

namespace channel_pipeline_rust::bench {
namespace {

using namespace apache::thrift::fast_thrift::channel_pipeline;

HANDLER_TAG(bench_rust);
HANDLER_TAG(bench_cpp);

// Upper bound on the bytes a spawned Rust coroutine may allocate per call: one
// task cell holding the task state, scheduler, future, completion, and payload.
constexpr uint64_t kCoroTaskCellByteBudget = 256;

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
  void onReadReady(detail::ContextImpl&) noexcept {}
  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineActive(detail::ContextImpl&) noexcept {}
  void onPipelineInactive(detail::ContextImpl&) noexcept {}
  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Native baseline mirroring the Rust shim's readiness bookkeeping without FFI.
// onRead/onWrite arm the matching hook and return Backpressure, exactly like
// the shim does when a Rust handler returns Backpressure.
// onReadReady/onWriteReady cancel first (matching the shim's
// cancel-before-callback one-shot contract), then optionally re-arm to mirror a
// re-arming Rust handler.
struct NativeReadyHandler {
  ReadReadyHook readReadyHook_;
  WriteReadyHook writeReadyHook_;
  bool rearm{false};

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&&) noexcept {
    ctx.awaitReadReady();
    return Result::Backpressure;
  }
  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&&) noexcept {
    ctx.awaitWriteReady();
    return Result::Backpressure;
  }
  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
  void onReadReady(detail::ContextImpl& ctx) noexcept {
    ctx.cancelAwaitReadReady();
    if (rearm) {
      ctx.awaitReadReady();
    }
  }
  void onWriteReady(detail::ContextImpl& ctx) noexcept {
    ctx.cancelAwaitWriteReady();
    if (rearm) {
      ctx.awaitWriteReady();
    }
  }
  void onPipelineActive(detail::ContextImpl&) noexcept {}
  void onPipelineInactive(detail::ContextImpl&) noexcept {}
  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

struct BenchTransport {
  uint64_t writeCount{0};
  Result onWrite(detail::ContextImpl&, TypeErasedBox&&) noexcept {
    ++writeCount;
    return Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void pauseRead() noexcept {}
  void resumeRead() noexcept {}
  void onReadReady() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
};

struct BenchApp {
  uint64_t readCount{0};
  uint64_t exceptionCount{0};
  Result onRead(detail::ContextImpl&, TypeErasedBox&&) noexcept {
    ++readCount;
    return Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept { ++exceptionCount; }
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}
};

struct BenchAllocator {
  BytesPtr allocate(std::size_t size) noexcept {
    return folly::IOBuf::create(size);
  }
  BytesPtr copyBuffer(const void* data, std::size_t size) noexcept {
    return folly::IOBuf::copyBuffer(data, size);
  }
};

class Watchdog {
 public:
  explicit Watchdog(uint64_t timeoutMs)
      : timeoutMs_{timeoutMs}, thread_{[this] { run(); }} {}

  ~Watchdog() {
    {
      std::lock_guard lock{mutex_};
      done_ = true;
    }
    cv_.notify_one();
    thread_.join();
  }

  void operation(const char* name) noexcept {
    operation_.store(name, std::memory_order_release);
  }

 private:
  void run() {
    std::unique_lock lock{mutex_};
    if (cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs_), [this] {
          return done_;
        })) {
      return;
    }
    const auto* operation = operation_.load(std::memory_order_acquire);
    std::fprintf(
        stderr,
        "channel_pipeline benchmark watchdog: operation '%s' exceeded %llu ms\n",
        operation,
        static_cast<unsigned long long>(timeoutMs_));
    std::fflush(stderr);
    std::abort();
  }

  uint64_t timeoutMs_;
  std::atomic<const char*> operation_{"benchmark setup"};
  std::mutex mutex_;
  std::condition_variable cv_;
  bool done_{false};
  std::thread thread_;
};

template <typename F>
double measureNs(F&& fn) {
  const auto start = std::chrono::steady_clock::now();
  fn();
  return static_cast<double>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now() - start)
          .count());
}

std::vector<TypeErasedBox> makeMessages(uint64_t iterations) {
  std::vector<TypeErasedBox> messages;
  messages.reserve(iterations);
  for (uint64_t i = 0; i < iterations; ++i) {
    auto bytes = folly::IOBuf::create(64);
    bytes->append(64);
    messages.emplace_back(std::move(bytes));
  }
  return messages;
}

template <typename Pipeline>
double measureRead(Pipeline& pipeline, uint64_t iterations) {
  auto messages = makeMessages(iterations);
  return measureNs([&] {
           for (auto& message : messages) {
             folly::doNotOptimizeAway(pipeline.fireRead(std::move(message)));
           }
         }) /
      static_cast<double>(iterations);
}

template <typename Pipeline>
double measureWrite(Pipeline& pipeline, uint64_t iterations) {
  auto messages = makeMessages(iterations);
  return measureNs([&] {
           for (auto& message : messages) {
             folly::doNotOptimizeAway(pipeline.fireWrite(std::move(message)));
           }
         }) /
      static_cast<double>(iterations);
}

template <typename Pipeline>
double measureException(Pipeline& pipeline, uint64_t iterations) {
  return measureNs([&] {
           for (uint64_t i = 0; i < iterations; ++i) {
             pipeline.fireException(
                 folly::make_exception_wrapper<std::runtime_error>(
                     "benchmark"));
           }
         }) /
      static_cast<double>(iterations);
}

void requireCount(uint64_t actual, uint64_t expected, const char* path) {
  if (actual != expected) {
    throw std::runtime_error(
        std::string(path) + " did not execute expected callbacks");
  }
}

void requireZero(uint64_t actual, const char* path) {
  if (actual != 0) {
    throw std::runtime_error(
        std::string(path) + " expected zero on the normal synchronous path");
  }
}

void requireAtMostPerIteration(
    uint64_t actual, uint64_t iterations, uint64_t budget, const char* path) {
  const uint64_t perIteration = actual / iterations;
  if (perIteration > budget) {
    throw std::runtime_error(
        std::string(path) + " used " + std::to_string(perIteration) +
        " bytes per iteration, over its " + std::to_string(budget) +
        " byte budget");
  }
}

TypeErasedBox makeBox() {
  auto bytes = folly::IOBuf::create(64);
  bytes->append(64);
  return TypeErasedBox{std::move(bytes)};
}

// Measures one-shot ready dispatch with immediate re-arm: the hook is armed
// once, then each onReadReady/onWriteReady delivers exactly one notification
// and the handler re-arms for the next. Isolates the readiness dispatch +
// re-arm cost with no message movement.
template <typename Pipeline>
double measureReadyRearm(Pipeline& pipeline, uint64_t iterations, bool read) {
  if (read) {
    folly::doNotOptimizeAway(pipeline.fireRead(makeBox()));
  } else {
    folly::doNotOptimizeAway(pipeline.fireWrite(makeBox()));
  }
  return measureNs([&] {
           for (uint64_t i = 0; i < iterations; ++i) {
             if (read) {
               pipeline.onReadReady();
             } else {
               pipeline.onWriteReady();
             }
           }
         }) /
      static_cast<double>(iterations);
}

// Measures a full Backpressure -> ready -> recovery cycle: each iteration fires
// data (handler returns Backpressure, arming the hook) then delivers the
// matching readiness notification (recovery).
template <typename Pipeline>
double measureBackpressureCycle(
    Pipeline& pipeline, uint64_t iterations, bool read) {
  auto messages = makeMessages(iterations);
  return measureNs([&] {
           for (auto& message : messages) {
             if (read) {
               folly::doNotOptimizeAway(pipeline.fireRead(std::move(message)));
               pipeline.onReadReady();
             } else {
               folly::doNotOptimizeAway(pipeline.fireWrite(std::move(message)));
               pipeline.onWriteReady();
             }
           }
         }) /
      static_cast<double>(iterations);
}

uint64_t threadAllocatedBytes() {
  if (!folly::usingJEMalloc()) {
    return 0;
  }
  uint64_t allocated = 0;
  folly::mallctlRead("thread.allocated", &allocated);
  return allocated;
}

} // namespace

BenchResult run_bench_with_watchdog(uint64_t iterations, uint64_t timeoutMs) {
  Watchdog watchdog{timeoutMs};

  watchdog.operation("BytesPtr adapter round trip");
  auto adapterMessages = makeMessages(iterations);
  const auto adapterRoundTrip = measureNs([&] {
    for (auto& message : adapterMessages) {
      auto taken = RustMessageAdapter<BytesPtr>::tryTake(std::move(message));
      if (!taken) {
        throw std::runtime_error("adapter take failed");
      }
      folly::doNotOptimizeAway(
          RustMessageAdapter<BytesPtr>::box(std::move(*taken)));
    }
  });

  folly::EventBase eventBase;
  BenchTransport nativeTransport;
  BenchApp nativeApp;
  BenchAllocator allocator;
  auto nativePipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&nativeTransport)
          .setTail(&nativeApp)
          .setAllocator(&allocator)
          .addNextDuplex<PassthroughHandler>(bench_cpp_tag)
          .build();

  watchdog.operation("native read pipeline");
  const auto nativeRead = measureRead(*nativePipeline, iterations);
  watchdog.operation("native write pipeline");
  const auto nativeWrite = measureWrite(*nativePipeline, iterations);
  watchdog.operation("native exception pipeline");
  const auto nativeException = measureException(*nativePipeline, iterations);
  requireCount(nativeApp.readCount, iterations, "native read pipeline");
  requireCount(nativeTransport.writeCount, iterations, "native write pipeline");
  requireCount(
      nativeApp.exceptionCount, iterations, "native exception pipeline");

  rust_handler_reset_test_counts();
  BenchTransport rustTransport;
  BenchApp rustApp;
  using RustHandlerImpl = RustHandler<detail::ContextImpl>;
  auto rustPipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&rustTransport)
          .setTail(&rustApp)
          .setAllocator(&allocator)
          .addNextDuplex<RustHandlerImpl>(
              bench_rust_tag,
              std::make_unique<RustHandlerImpl>(
                  rust_handler_new_counting_test()))
          .build();

  watchdog.operation("Rust read pipeline");
  const auto rustRead = measureRead(*rustPipeline, iterations);
  watchdog.operation("Rust write pipeline");
  const auto rustWrite = measureWrite(*rustPipeline, iterations);
  requireCount(
      rust_handler_test_read_callbacks(), iterations, "Rust read pipeline");
  requireCount(
      rust_handler_test_write_callbacks(), iterations, "Rust write pipeline");
  requireCount(rustApp.readCount, iterations, "Rust read tail");
  requireCount(rustTransport.writeCount, iterations, "Rust write head");

  folly::ScopedEventBaseThread contextHandleEventBaseThread;
  auto* contextHandleEventBase = contextHandleEventBaseThread.getEventBase();
  BenchTransport contextHandleReadTransport;
  BenchApp contextHandleReadApp;
  BenchTransport contextHandleWriteTransport;
  BenchApp contextHandleWriteApp;
  BenchTransport readyCoroReadTransport;
  BenchApp readyCoroReadApp;
  BenchTransport readyCoroWriteTransport;
  BenchApp readyCoroWriteApp;
  BenchTransport pendingCoroReadTransport;
  BenchApp pendingCoroReadApp;
  PipelineImpl::Ptr contextHandleReadPipeline;
  PipelineImpl::Ptr contextHandleWritePipeline;
  PipelineImpl::Ptr readyCoroReadPipeline;
  PipelineImpl::Ptr readyCoroWritePipeline;
  PipelineImpl::Ptr pendingCoroReadPipeline;
  double contextHandleRead = 0;
  double contextHandleWrite = 0;
  double readyCoroRead = 0;
  double readyCoroWrite = 0;
  double pendingCoroSubmit = 0;
  uint64_t pendingCoroSubmitAllocBytes = 0;
  contextHandleEventBase->runInEventBaseThreadAndWait([&] {
    contextHandleReadPipeline =
        PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
            .setEventBase(contextHandleEventBase)
            .setHead(&contextHandleReadTransport)
            .setTail(&contextHandleReadApp)
            .setAllocator(&allocator)
            .addNextDuplex<RustHandlerImpl>(
                bench_rust_tag,
                std::make_unique<RustHandlerImpl>(
                    rust_handler_new_context_handle_test(8)))
            .build();
    contextHandleWritePipeline =
        PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
            .setEventBase(contextHandleEventBase)
            .setHead(&contextHandleWriteTransport)
            .setTail(&contextHandleWriteApp)
            .setAllocator(&allocator)
            .addNextDuplex<RustHandlerImpl>(
                bench_rust_tag,
                std::make_unique<RustHandlerImpl>(
                    rust_handler_new_context_handle_test(9)))
            .build();
    readyCoroReadPipeline =
        PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
            .setEventBase(contextHandleEventBase)
            .setHead(&readyCoroReadTransport)
            .setTail(&readyCoroReadApp)
            .setAllocator(&allocator)
            .addNextDuplex<RustHandlerImpl>(
                bench_rust_tag,
                std::make_unique<RustHandlerImpl>(
                    rust_handler_new_context_handle_test(20)))
            .build();
    readyCoroWritePipeline =
        PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
            .setEventBase(contextHandleEventBase)
            .setHead(&readyCoroWriteTransport)
            .setTail(&readyCoroWriteApp)
            .setAllocator(&allocator)
            .addNextDuplex<RustHandlerImpl>(
                bench_rust_tag,
                std::make_unique<RustHandlerImpl>(
                    rust_handler_new_context_handle_test(22)))
            .build();
    pendingCoroReadPipeline =
        PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
            .setEventBase(contextHandleEventBase)
            .setHead(&pendingCoroReadTransport)
            .setTail(&pendingCoroReadApp)
            .setAllocator(&allocator)
            .addNextDuplex<RustHandlerImpl>(
                bench_rust_tag,
                std::make_unique<RustHandlerImpl>(
                    rust_handler_new_context_handle_test(26)))
            .build();

    watchdog.operation("Rust ContextHandle immediate read");
    contextHandleRead = measureRead(*contextHandleReadPipeline, iterations);
    watchdog.operation("Rust ContextHandle immediate write");
    contextHandleWrite = measureWrite(*contextHandleWritePipeline, iterations);
    watchdog.operation("Rust ready coroutine read");
    readyCoroRead = measureRead(*readyCoroReadPipeline, iterations);
    watchdog.operation("Rust ready coroutine write");
    readyCoroWrite = measureWrite(*readyCoroWritePipeline, iterations);

    watchdog.operation("Rust pending coroutine submit");
    auto pendingMessages = makeMessages(iterations);
    const auto pendingBefore = threadAllocatedBytes();
    pendingCoroSubmit =
        measureNs([&] {
          for (auto& message : pendingMessages) {
            folly::doNotOptimizeAway(
                pendingCoroReadPipeline->fireRead(std::move(message)));
          }
        }) /
        static_cast<double>(iterations);
    pendingCoroSubmitAllocBytes = threadAllocatedBytes() - pendingBefore;
  });
  contextHandleEventBase->runInEventBaseThreadAndWait([] {});
  requireCount(
      contextHandleReadApp.readCount,
      iterations,
      "Rust ContextHandle read tail");
  requireCount(
      contextHandleWriteTransport.writeCount,
      iterations,
      "Rust ContextHandle write head");
  requireCount(
      readyCoroReadApp.readCount, iterations, "Rust ready coroutine read tail");
  requireCount(
      readyCoroWriteTransport.writeCount,
      iterations,
      "Rust ready coroutine write head");
  requireCount(
      pendingCoroReadApp.readCount,
      iterations,
      "Rust pending coroutine read tail");

  rust_handler_reset_test_counts();
  auto exceptionPipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&rustTransport)
          .setTail(&rustApp)
          .setAllocator(&allocator)
          .addNextDuplex<RustHandlerImpl>(
              bench_rust_tag,
              std::make_unique<RustHandlerImpl>(
                  rust_handler_new_lifecycle_test()))
          .build();
  rustApp.exceptionCount = 0;
  watchdog.operation("Rust exception pipeline");
  const auto rustException = measureException(*exceptionPipeline, iterations);
  requireCount(
      rust_handler_test_exception_callbacks(),
      iterations,
      "Rust exception pipeline");
  requireCount(rustApp.exceptionCount, iterations, "Rust exception tail");

  // === Paired readiness one-shot + re-arm ===
  BenchTransport nativeReadyHead;
  BenchApp nativeReadyApp;
  auto nativeReadyHandler = std::make_unique<NativeReadyHandler>();
  nativeReadyHandler->rearm = true;
  auto nativeReadyPipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&nativeReadyHead)
          .setTail(&nativeReadyApp)
          .setAllocator(&allocator)
          .addNextDuplex<NativeReadyHandler>(
              bench_cpp_tag, std::move(nativeReadyHandler))
          .build();
  watchdog.operation("native read ready re-arm");
  const auto nativeReadReady =
      measureReadyRearm(*nativeReadyPipeline, iterations, /*read=*/true);

  BenchTransport rustReadyHead;
  BenchApp rustReadyApp;
  auto rustReadyPipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&rustReadyHead)
          .setTail(&rustReadyApp)
          .setAllocator(&allocator)
          .addNextDuplex<RustHandlerImpl>(
              bench_rust_tag,
              std::make_unique<RustHandlerImpl>(
                  rust_handler_new_ready_rearm_bench()))
          .build();
  watchdog.operation("Rust read ready re-arm");
  const auto rustReadReady =
      measureReadyRearm(*rustReadyPipeline, iterations, /*read=*/true);

  // === Paired Backpressure -> ready -> recovery cycles ===
  BenchTransport nativeRecoveryHead;
  BenchApp nativeRecoveryApp;
  auto nativeRecoveryPipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&nativeRecoveryHead)
          .setTail(&nativeRecoveryApp)
          .setAllocator(&allocator)
          .addNextDuplex<NativeReadyHandler>(
              bench_cpp_tag, std::make_unique<NativeReadyHandler>())
          .build();
  watchdog.operation("native read recovery cycle");
  const auto nativeReadRecovery =
      measureBackpressureCycle(*nativeRecoveryPipeline, iterations, true);
  watchdog.operation("native write recovery cycle");
  const auto nativeWriteRecovery =
      measureBackpressureCycle(*nativeRecoveryPipeline, iterations, false);

  BenchTransport rustRecoveryHead;
  BenchApp rustRecoveryApp;
  auto rustRecoveryPipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&eventBase)
          .setHead(&rustRecoveryHead)
          .setTail(&rustRecoveryApp)
          .setAllocator(&allocator)
          .addNextDuplex<RustHandlerImpl>(
              bench_rust_tag,
              std::make_unique<RustHandlerImpl>(
                  rust_handler_new_backpressure_test()))
          .build();
  watchdog.operation("Rust read recovery cycle");
  const auto rustReadRecovery =
      measureBackpressureCycle(*rustRecoveryPipeline, iterations, true);
  watchdog.operation("Rust write recovery cycle");
  const auto rustWriteRecovery =
      measureBackpressureCycle(*rustRecoveryPipeline, iterations, false);

  // === Zero-allocation / zero-enqueue evidence on the normal sync path ===
  const bool jemalloc = folly::usingJEMalloc();

  watchdog.operation("ready path allocation/enqueue evidence");
  folly::EventBase readyEvidenceEb;
  BenchTransport readyEvHead;
  BenchApp readyEvApp;
  auto readyEvidencePipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&readyEvidenceEb)
          .setHead(&readyEvHead)
          .setTail(&readyEvApp)
          .setAllocator(&allocator)
          .addNextDuplex<RustHandlerImpl>(
              bench_rust_tag,
              std::make_unique<RustHandlerImpl>(
                  rust_handler_new_ready_rearm_bench()))
          .build();
  folly::doNotOptimizeAway(readyEvidencePipeline->fireRead(makeBox()));
  for (uint64_t i = 0; i < 1000; ++i) {
    readyEvidencePipeline->onReadReady(); // warm up lazy initialization
  }
  const auto readyBefore = threadAllocatedBytes();
  for (uint64_t i = 0; i < iterations; ++i) {
    readyEvidencePipeline->onReadReady();
  }
  const uint64_t readyAllocBytes = threadAllocatedBytes() - readyBefore;
  const uint64_t readyLoopCallbacks = readyEvidenceEb.getNumLoopCallbacks();

  watchdog.operation("forward path allocation/enqueue evidence");
  folly::EventBase forwardEvidenceEb;
  BenchTransport forwardEvHead;
  BenchApp forwardEvApp;
  rust_handler_reset_test_counts();
  auto forwardEvidencePipeline =
      PipelineBuilder<BenchTransport, BenchApp, BenchAllocator>()
          .setEventBase(&forwardEvidenceEb)
          .setHead(&forwardEvHead)
          .setTail(&forwardEvApp)
          .setAllocator(&allocator)
          .addNextDuplex<RustHandlerImpl>(
              bench_rust_tag,
              std::make_unique<RustHandlerImpl>(
                  rust_handler_new_counting_test()))
          .build();
  {
    auto warmup = makeMessages(1000);
    for (auto& message : warmup) {
      folly::doNotOptimizeAway(
          forwardEvidencePipeline->fireRead(std::move(message)));
    }
  }
  auto forwardMessages = makeMessages(iterations);
  const auto forwardBefore = threadAllocatedBytes();
  for (auto& message : forwardMessages) {
    folly::doNotOptimizeAway(
        forwardEvidencePipeline->fireRead(std::move(message)));
  }
  const uint64_t forwardAllocBytes = threadAllocatedBytes() - forwardBefore;
  const uint64_t forwardLoopCallbacks = forwardEvidenceEb.getNumLoopCallbacks();

  watchdog.operation("ContextHandle type erasure allocation evidence");
  auto typeErasureMessages = makeMessages(iterations);
  const auto typeErasureBefore = threadAllocatedBytes();
  for (auto& message : typeErasureMessages) {
    auto bytes = RustMessageAdapter<BytesPtr>::tryTake(std::move(message));
    if (!bytes) {
      throw std::runtime_error("ContextHandle type erasure take failed");
    }
    folly::doNotOptimizeAway(
        RustMessageAdapter<BytesPtr>::box(std::move(*bytes)));
  }
  const uint64_t contextHandleTypeErasureAllocBytes =
      threadAllocatedBytes() - typeErasureBefore;

  uint64_t contextHandlePathAllocBytes = 0;
  uint64_t contextHandlePathLoopCallbacks = 0;
  uint64_t readyCoroPathAllocBytes = 0;
  uint64_t readyCoroPathLoopCallbacks = 0;
  contextHandleEventBase->runInEventBaseThreadAndWait([&] {
    auto warmup = makeMessages(1000);
    for (auto& message : warmup) {
      folly::doNotOptimizeAway(
          contextHandleReadPipeline->fireRead(std::move(message)));
    }
    auto messages = makeMessages(iterations);
    const auto callbacksBefore = contextHandleEventBase->getNumLoopCallbacks();
    const auto before = threadAllocatedBytes();
    for (auto& message : messages) {
      folly::doNotOptimizeAway(
          contextHandleReadPipeline->fireRead(std::move(message)));
    }
    contextHandlePathAllocBytes = threadAllocatedBytes() - before;
    contextHandlePathLoopCallbacks =
        contextHandleEventBase->getNumLoopCallbacks() - callbacksBefore;

    auto coroWarmup = makeMessages(1000);
    for (auto& message : coroWarmup) {
      folly::doNotOptimizeAway(
          readyCoroReadPipeline->fireRead(std::move(message)));
    }
    auto coroMessages = makeMessages(iterations);
    const auto coroCallbacksBefore =
        contextHandleEventBase->getNumLoopCallbacks();
    const auto coroBefore = threadAllocatedBytes();
    for (auto& message : coroMessages) {
      folly::doNotOptimizeAway(
          readyCoroReadPipeline->fireRead(std::move(message)));
    }
    readyCoroPathAllocBytes = threadAllocatedBytes() - coroBefore;
    readyCoroPathLoopCallbacks =
        contextHandleEventBase->getNumLoopCallbacks() - coroCallbacksBefore;
  });

  requireZero(readyLoopCallbacks, "ready path EventBase enqueue");
  requireZero(forwardLoopCallbacks, "forward path EventBase enqueue");
  requireZero(
      contextHandlePathLoopCallbacks, "ContextHandle EventBase-local enqueue");
  requireZero(
      readyCoroPathLoopCallbacks, "ready coroutine EventBase-local enqueue");
  if (jemalloc) {
    requireZero(readyAllocBytes, "ready path heap allocation");
    requireZero(forwardAllocBytes, "forward path heap allocation");
    requireZero(
        contextHandleTypeErasureAllocBytes,
        "ContextHandle inline type erasure heap allocation");
    requireZero(
        contextHandlePathAllocBytes,
        "ContextHandle EventBase-local heap allocation");
    // A Rust future must be polled at its final address, so a spawned
    // coroutine allocates its task cell before the first poll even when that
    // poll completes inline. The cell is a single allocation holding the task
    // state, scheduler, future, completion, and payload inline; this budget
    // fails if anything else on the path starts allocating.
    requireAtMostPerIteration(
        readyCoroPathAllocBytes,
        iterations,
        kCoroTaskCellByteBudget,
        "ready coroutine task cell");
  }

  watchdog.operation("benchmark result assembly");
  return BenchResult{
      adapterRoundTrip / static_cast<double>(iterations),
      nativeRead,
      rustRead,
      nativeWrite,
      rustWrite,
      contextHandleRead,
      contextHandleWrite,
      readyCoroRead,
      readyCoroWrite,
      pendingCoroSubmit,
      nativeException,
      rustException,
      nativeReadReady,
      rustReadReady,
      nativeReadRecovery,
      rustReadRecovery,
      nativeWriteRecovery,
      rustWriteRecovery,
      readyAllocBytes,
      forwardAllocBytes,
      contextHandleTypeErasureAllocBytes,
      contextHandlePathAllocBytes,
      readyCoroPathAllocBytes,
      pendingCoroSubmitAllocBytes,
      readyLoopCallbacks,
      forwardLoopCallbacks,
      contextHandlePathLoopCallbacks,
      readyCoroPathLoopCallbacks,
      jemalloc,
  };
}

} // namespace channel_pipeline_rust::bench
