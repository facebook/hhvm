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

/**
 * Pipeline Throughput Microbenchmarks
 *
 * Measures pure pipeline dispatch overhead with pre-allocated messages.
 * All message allocation happens outside the timing loop.
 */

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

#include <folly/Benchmark.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <memory>
#include <vector>

using namespace apache::thrift::fast_thrift::channel_pipeline;

namespace {

// =============================================================================
// Benchmark Handlers and Adapters
// =============================================================================

// Handler tags for benchmarks
HANDLER_TAG(bench_passthrough);
HANDLER_TAG(bench_multifire);
HANDLER_TAG(bench_echo);
HANDLER_TAG(bench_backpressure);
HANDLER_TAG(bench_backpressure2);
HANDLER_TAG(bench_backpressure3);
HANDLER_TAG(bench_backpressure4);

// Minimal passthrough handler - measures dispatch overhead
struct PassthroughHandler {
  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onPipelineActivated(detail::ContextImpl&) noexcept {}
  void onReadReady(detail::ContextImpl&) noexcept {}

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Multi-fire handler

// Swallowing handler - doesn't forward messages (measures skip overhead)
struct SwallowingHandler {
  uint64_t swallowed_count{0};

  Result onRead(detail::ContextImpl&, TypeErasedBox&&) noexcept {
    ++swallowed_count;
    return Result::Success; // Don't forward
  }

  void onException(detail::ContextImpl&, folly::exception_wrapper&&) noexcept {
    // Swallowed, don't forward
  }

  void onPipelineActivated(detail::ContextImpl&) noexcept {}
  void onReadReady(detail::ContextImpl&) noexcept {}

  Result onWrite(detail::ContextImpl&, TypeErasedBox&&) noexcept {
    ++swallowed_count;
    return Result::Success; // Don't forward
  }

  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Multi-fire handler - fires N messages per input (measures per-fire overhead)
// Uses pre-allocated template buffer with clone() to avoid allocation overhead
template <int N>
struct MultiFireHandler {
  BytesPtr template_buf;

  MultiFireHandler() : template_buf(folly::IOBuf::create(64)) {
    template_buf->append(64); // Make non-empty for realistic clone
  }

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&&) noexcept {
    for (int i = 0; i < N; ++i) {
      (void)ctx.fireRead(TypeErasedBox(i));
    }
    return Result::Success;
  }

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onPipelineActivated(detail::ContextImpl&) noexcept {}
  void onReadReady(detail::ContextImpl&) noexcept {}

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&&) noexcept {
    for (int i = 0; i < N; ++i) {
      // Clone is cheap - just refcount increment for shared data
      auto buf = template_buf->clone();
      (void)ctx.fireWrite(TypeErasedBox(std::move(buf)));
    }
    return Result::Success;
  }

  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Echo handler - converts reads to writes (round-trip: read → process → write)
// Uses pre-allocated template buffer with clone() to avoid allocation overhead
struct EchoHandler {
  uint64_t echo_count{0};
  BytesPtr template_buf;

  EchoHandler() : template_buf(folly::IOBuf::create(64)) {
    template_buf->append(64); // Make non-empty for realistic clone
  }

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&&) noexcept {
    ++echo_count;
    // Clone is cheap - just refcount increment for shared data
    auto buf = template_buf->clone();
    return ctx.fireWrite(TypeErasedBox(std::move(buf)));
  }

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onReadReady(detail::ContextImpl&) noexcept {}

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineActivated(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Backpressure handler

// Backpressure handler - has hook for intrusive list registration
// Simulates a handler that registers for write ready notifications
struct BackpressureHandler {
  WriteReadyHook writeReadyHook_;
  uint64_t write_count{0};
  uint64_t write_ready_count{0};
  bool register_on_write{true};

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  void onReadReady(detail::ContextImpl&) noexcept {}

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    ++write_count;
    if (register_on_write) {
      ctx.awaitWriteReady(); // Register for callback
    }
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(detail::ContextImpl& ctx) noexcept {
    ++write_ready_count;
    ctx.cancelAwaitWriteReady(); // Unregister
  }

  void onPipelineActivated(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Read backpressure handler - has hook for intrusive list registration
// Simulates a handler that registers for read ready notifications
struct ReadBackpressureHandler {
  ReadReadyHook readReadyHook_;
  uint64_t read_count{0};
  uint64_t read_ready_count{0};
  bool register_on_read{true};

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    ++read_count;
    if (register_on_read) {
      ctx.awaitReadReady(); // Register for callback
    }
    return ctx.fireRead(std::move(msg));
  }

  void onReadReady(detail::ContextImpl& ctx) noexcept {
    ++read_ready_count;
    ctx.cancelAwaitReadReady(); // Unregister
  }

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineActivated(detail::ContextImpl&) noexcept {}
  void onPipelineDeactivated(detail::ContextImpl&) noexcept {}

  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}
};

// Source handler (outbound end of pipeline)
struct BenchTransportHandler {
  uint64_t write_count{0};
  uint64_t exception_count{0};

  Result onWrite(TypeErasedBox&&) noexcept {
    ++write_count;
    return Result::Success;
  }

  void onException(folly::exception_wrapper&&) noexcept { ++exception_count; }

  void pauseRead() noexcept {}
  void resumeRead() noexcept {}
  void onReadReady() noexcept {}

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
};

// App handler (inbound end of pipeline)
struct BenchAppHandler {
  uint64_t read_count{0};
  uint64_t exception_count{0};

  Result onRead(TypeErasedBox&&) noexcept {
    ++read_count;
    return Result::Success;
  }

  void onException(folly::exception_wrapper&&) noexcept { ++exception_count; }

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}
};

// Simple buffer allocator
struct BenchAllocator {
  BytesPtr allocate(std::size_t size) noexcept {
    return folly::IOBuf::create(size);
  }

  BytesPtr copyBuffer(const void* data, std::size_t size) noexcept {
    return folly::IOBuf::copyBuffer(data, size);
  }
};

// Helper to create a pipeline
auto makePipeline(
    folly::EventBase& evb,
    BenchTransportHandler& transport,
    BenchAppHandler& app,
    BenchAllocator& allocator) {
  return PipelineBuilder<
             BenchTransportHandler,
             BenchAppHandler,
             BenchAllocator>()
      .setEventBase(&evb)
      .setHead(&transport)
      .setTail(&app)
      .setAllocator(&allocator)
      .build();
}

} // namespace

// =============================================================================
// TypeErasedBox Microbenchmarks
// Measures the overhead of TypeErasedBox construction and access
// =============================================================================

BENCHMARK(TypeErasedBox_Construct_Int, iters) {
  for (size_t i = 0; i < iters; ++i) {
    TypeErasedBox box(static_cast<int>(i));
    folly::doNotOptimizeAway(box);
  }
}

BENCHMARK(TypeErasedBox_Get_Int, iters) {
  TypeErasedBox box(42);
  for (size_t i = 0; i < iters; ++i) {
    auto& val = box.get<int>();
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK(TypeErasedBox_Construct_IOBuf, iters) {
  for (size_t i = 0; i < iters; ++i) {
    TypeErasedBox box(folly::IOBuf::create(64));
    folly::doNotOptimizeAway(box);
  }
}

BENCHMARK(TypeErasedBox_Get_IOBuf, iters) {
  TypeErasedBox box(folly::IOBuf::create(64));
  for (size_t i = 0; i < iters; ++i) {
    auto& val = box.get<BytesPtr>();
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK(TypeErasedBox_Take_Int, iters) {
  for (size_t i = 0; i < iters; ++i) {
    TypeErasedBox box(static_cast<int>(i));
    auto val = box.take<int>();
    folly::doNotOptimizeAway(val);
  }
}

BENCHMARK(TypeErasedBox_Get_And_Move_IOBuf, iters) {
  // Pre-allocate boxes outside timing
  folly::BenchmarkSuspender susp;
  std::vector<TypeErasedBox> boxes;
  boxes.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    boxes.emplace_back(folly::IOBuf::create(64));
  }
  susp.dismiss();

  // Measure get + move (what fireWriteToSource does)
  for (size_t i = 0; i < iters; ++i) {
    auto& bytes = boxes[i].get<BytesPtr>();
    BytesPtr moved = std::move(bytes);
    folly::doNotOptimizeAway(moved);
  }
}

BENCHMARK(TypeErasedBox_Take_IOBuf, iters) {
  // Pre-allocate boxes outside timing
  folly::BenchmarkSuspender susp;
  std::vector<TypeErasedBox> boxes;
  boxes.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    boxes.emplace_back(folly::IOBuf::create(64));
  }
  susp.dismiss();

  // Measure take (alternative extraction method)
  for (size_t i = 0; i < iters; ++i) {
    BytesPtr taken = boxes[i].take<BytesPtr>();
    folly::doNotOptimizeAway(taken);
  }
}

BENCHMARK(IOBuf_Clone, iters) {
  // Measure IOBuf clone overhead
  auto template_buf = folly::IOBuf::create(64);
  template_buf->append(64);

  for (size_t i = 0; i < iters; ++i) {
    auto cloned = template_buf->clone();
    folly::doNotOptimizeAway(cloned);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Pipeline fireRead Throughput (0 handlers - baseline)
// =============================================================================

BENCHMARK(Pipeline_FireRead_0Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with write path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  // Pre-allocate messages OUTSIDE timing loop using clone
  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  // Only measure dispatch
  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

// =============================================================================
// Pipeline fireWrite Throughput (0 handlers - baseline)
// =============================================================================

BENCHMARK(Pipeline_FireWrite_0Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with read path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  // Pre-allocate IOBufs OUTSIDE timing loop using clone
  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Pipeline fire_read with varying handler counts
// Note: Currently tests 0 handlers since we can't dynamically add handlers.
// When handler addition is supported, these will show per-handler overhead.
// =============================================================================

BENCHMARK(Pipeline_FireRead_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // TODO: Add 1 handler when dynamic handler addition is supported
  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with write path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Pipeline_FireRead_4Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // TODO: Add 4 handlers when dynamic handler addition is supported
  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with write path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Pipeline_FireRead_8Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // TODO: Add 8 handlers when dynamic handler addition is supported
  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with write path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Pipeline fire_write with varying handler counts
// =============================================================================

BENCHMARK(Pipeline_FireWrite_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // TODO: Add 1 handler when dynamic handler addition is supported
  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with read path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK(Pipeline_FireWrite_4Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // TODO: Add 4 handlers when dynamic handler addition is supported
  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with read path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK(Pipeline_FireWrite_8Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // TODO: Add 8 handlers when dynamic handler addition is supported
  auto pipeline = makePipeline(evb, transport, app, allocator);

  // Create template buffer once, use clone for fair comparison with read path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Swallowing Handler Benchmarks (handler doesn't forward)
// =============================================================================

BENCHMARK(Pipeline_FireRead_Swallow_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<PassthroughHandler>(bench_passthrough_tag)
          .build();

  // Create template buffer once, use clone for fair comparison with write path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Pipeline_FireWrite_Passthrough_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<PassthroughHandler>(bench_passthrough_tag)
          .build();

  // Create template buffer once, use clone for fair comparison with read path
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Multi-Fire Handler Benchmarks (handler fires N times per input)
// =============================================================================

BENCHMARK(Pipeline_FireRead_MultiFire_3x, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<MultiFireHandler<3>>(bench_multifire_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  // App receives 3x the messages
  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Pipeline_FireRead_MultiFire_10x, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<MultiFireHandler<10>>(bench_multifire_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  // App receives 10x the messages
  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Passthrough Handler Benchmark (1 handler - baseline for comparison)
// =============================================================================

BENCHMARK(Pipeline_FireRead_Passthrough_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<PassthroughHandler>(bench_passthrough_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Round-Trip Benchmarks (read → handler → write)
// Simulates request/response flow: message comes in, handler processes and
// sends response back out
// =============================================================================

BENCHMARK(Pipeline_RoundTrip_Echo_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  // Echo handler converts reads to writes
  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<EchoHandler>(bench_echo_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  // Messages went in via read, came out via write to transport
  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Backpressure Benchmarks
// Measures overhead of intrusive list registration/unregistration and callbacks
// =============================================================================

// Measures the overhead of awaitWriteReady + cancelAwaitWriteReady cycle
// This is pure intrusive list push_back + unlink cost
BENCHMARK(Backpressure_RegisterUnregister_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<BackpressureHandler>(bench_backpressure_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

BENCHMARK(Pipeline_FireWrite_Backpressure_ResumeWrite, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<BackpressureHandler>(bench_backpressure_tag)
          .build();

  // Create template buffer for cloning (avoids allocation in timed loop)
  auto templateBuf = folly::IOBuf::create(64);

  // Register the handler once
  (void)pipeline->fireWrite(TypeErasedBox(templateBuf->clone()));

  susp.dismiss();

  // Measure repeated onWriteReady calls
  // Handler unlinks then re-registers via the fireWrite call
  for (size_t i = 0; i < iters; ++i) {
    pipeline->onWriteReady(); // Calls callback, handler unlinks
    // Re-register for next iteration (clone is cheap, no heap allocation)
    (void)pipeline->fireWrite(TypeErasedBox(templateBuf->clone()));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

// Measures onWriteReady callback dispatch with 4 registered handlers
BENCHMARK(Backpressure_OnWriteReady_4Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<BackpressureHandler>(bench_backpressure_tag)
          .addNextDuplex<BackpressureHandler>(bench_backpressure2_tag)
          .addNextDuplex<BackpressureHandler>(bench_backpressure3_tag)
          .addNextDuplex<BackpressureHandler>(bench_backpressure4_tag)
          .build();

  // Create template buffer for cloning (avoids allocation in timed loop)
  auto templateBuf = folly::IOBuf::create(64);

  // Register all 4 handlers
  (void)pipeline->fireWrite(TypeErasedBox(templateBuf->clone()));

  susp.dismiss();

  // Measure repeated onWriteReady calls with 4 handlers
  for (size_t i = 0; i < iters; ++i) {
    pipeline->onWriteReady(); // Calls 4 callbacks
    // Re-register all for next iteration (clone is cheap, no heap allocation)
    (void)pipeline->fireWrite(TypeErasedBox(templateBuf->clone()));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

// Measures full backpressure cycle: write -> backpressure -> onWriteReady
// Compares to normal passthrough to show overhead
BENCHMARK(Backpressure_FullCycle_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<BackpressureHandler>(bench_backpressure_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  // Full cycle: write (registers) -> onWriteReady (unregisters)
  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
    pipeline->onWriteReady();
  }

  folly::doNotOptimizeAway(transport.write_count);
}

// Baseline comparison: passthrough without backpressure registration
BENCHMARK_RELATIVE(Backpressure_Baseline_Passthrough, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<PassthroughHandler>(bench_passthrough_tag)
          .build();

  // Create template buffer once, use clone for fair comparison
  auto templateBuf = folly::IOBuf::create(64);
  templateBuf->append(64);

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(templateBuf->clone());
  }

  susp.dismiss();

  // Just passthrough, no backpressure overhead
  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireWrite(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(transport.write_count);
}

// =============================================================================
// Read Backpressure Benchmarks
// Measures overhead of awaitReadReady/onReadReady callback path
// =============================================================================

BENCHMARK(Backpressure_RegisterUnregisterRead_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure_tag)
          .build();

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(static_cast<int>(i));
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Pipeline_FireRead_Backpressure_ResumeRead, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure_tag)
          .build();

  // Register once.
  (void)pipeline->fireRead(TypeErasedBox(0));

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    pipeline->onReadReady(); // Calls callback, handler unlinks
    (void)pipeline->fireRead(TypeErasedBox(static_cast<int>(i))); // Re-register
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Backpressure_OnReadReady_4Handlers, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure_tag)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure2_tag)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure3_tag)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure4_tag)
          .build();

  (void)pipeline->fireRead(TypeErasedBox(0)); // Register all handlers

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    pipeline->onReadReady(); // Calls 4 callbacks
    (void)pipeline->fireRead(TypeErasedBox(static_cast<int>(i))); // Re-register
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK(Backpressure_FullCycle_Read_1Handler, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<ReadBackpressureHandler>(bench_backpressure_tag)
          .build();

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(static_cast<int>(i));
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
    pipeline->onReadReady();
  }

  folly::doNotOptimizeAway(app.read_count);
}

BENCHMARK_RELATIVE(Backpressure_Baseline_PassthroughRead, iters) {
  folly::BenchmarkSuspender susp;
  folly::EventBase evb;
  BenchTransportHandler transport;
  BenchAppHandler app;
  BenchAllocator allocator;

  auto pipeline =
      PipelineBuilder<BenchTransportHandler, BenchAppHandler, BenchAllocator>()
          .setEventBase(&evb)
          .setHead(&transport)
          .setTail(&app)
          .setAllocator(&allocator)
          .addNextDuplex<PassthroughHandler>(bench_passthrough_tag)
          .build();

  std::vector<TypeErasedBox> messages;
  messages.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    messages.emplace_back(static_cast<int>(i));
  }

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)pipeline->fireRead(std::move(messages[i]));
  }

  folly::doNotOptimizeAway(app.read_count);
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
