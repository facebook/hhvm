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
 * Transport Handler Microbenchmarks
 *
 * Measures the transport adapter overhead including:
 * - DelayedDestruction guard overhead
 * - Pipeline dispatch
 * - Write state tracking
 */

#include <folly/Benchmark.h>
#include <folly/CppAttributes.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/test/MockAsyncTransport.h>
#include <folly/portability/GMock.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using namespace testing;

namespace fast_transport = apache::thrift::fast_thrift::transport;

namespace {

// =============================================================================
// Test Payload Sizes (matching framing benchmarks)
// =============================================================================

constexpr size_t kSmallPayloadSize = 256;
constexpr size_t kMediumPayloadSize = 4 * 1024;
constexpr size_t kLargePayloadSize = 8 * 1024 * 1024;

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

// =============================================================================
// Minimal benchmark helpers
// =============================================================================

class BenchSocket : public folly::AsyncTransport {
 public:
  void writeChain(
      WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&&,
      WriteFlags) override {
    callback->writeSuccess();
  }

  void write(WriteCallback*, const void*, size_t, WriteFlags) override {}

  void writev(WriteCallback*, const iovec*, size_t, WriteFlags) override {}

  void setReadCB(ReadCallback*) override {}
  ReadCallback* FOLLY_NULLABLE getReadCallback() const override {
    return nullptr;
  }

  void close() override {}
  void closeNow() override {}
  void closeWithReset() override {}
  void shutdownWrite() override {}
  void shutdownWriteNow() override {}

  bool good() const override { return true; }
  bool readable() const override { return true; }
  bool connecting() const override { return false; }
  bool error() const override { return false; }

  void attachEventBase(EventBase*) override {}
  void detachEventBase() override {}
  bool isDetachable() const override { return true; }
  EventBase* FOLLY_NULLABLE getEventBase() const override { return nullptr; }

  void setSendTimeout(uint32_t) override {}
  uint32_t getSendTimeout() const override { return 0; }

  void getLocalAddress(folly::SocketAddress*) const override {}
  void getPeerAddress(folly::SocketAddress*) const override {}

  size_t getAppBytesWritten() const override { return 0; }
  size_t getRawBytesWritten() const override { return 0; }
  size_t getAppBytesReceived() const override { return 0; }
  size_t getRawBytesReceived() const override { return 0; }
  size_t getAppBytesBuffered() const override { return 0; }
  size_t getRawBytesBuffered() const override { return 0; }

  bool isEorTrackingEnabled() const override { return false; }
  void setEorTracking(bool) override {}
};

class BenchEndpointHandler {
 public:
  BenchEndpointHandler() = default;

  // TailEndpointHandler interface
  Result onRead(TypeErasedBox&&) noexcept { return Result::Success; }

  void onException(folly::exception_wrapper&&) noexcept {}

  // Lifecycle methods
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
};

class BenchAllocator {
 public:
  BytesPtr allocate(size_t) { return nullptr; }

  // NOLINTNEXTLINE(clang-diagnostic-unneeded-member-function)
  BytesPtr copyBuffer(const void*, size_t) { return nullptr; }
};

// =============================================================================
// Create Handler and Pipeline Helper
// =============================================================================
auto createHandlerAndPipeline(
    folly::EventBase& evb,
    BenchEndpointHandler& appHandler,
    BenchAllocator& allocator) {
  auto socket = folly::AsyncTransport::UniquePtr(new BenchSocket());
  auto handler =
      fast_transport::TransportHandler::create(std::move(socket), 256, 4096);

  auto pipeline = PipelineBuilder<
                      fast_transport::TransportHandler,
                      BenchEndpointHandler,
                      BenchAllocator>()
                      .setEventBase(&evb)
                      .setHead(handler.get())
                      .setTail(&appHandler)
                      .setAllocator(&allocator)
                      .build();

  handler->setPipeline(*pipeline);

  return std::make_pair(std::move(handler), std::move(pipeline));
}

// =============================================================================
// TransportHandler Write Benchmarks
// =============================================================================
BENCHMARK(Write_TransportHandler_Small, iters) {
  folly::BenchmarkSuspender susp;

  EventBase evb;
  BenchEndpointHandler appHandler;
  BenchAllocator allocator;
  auto [handler, pipeline] =
      createHandlerAndPipeline(evb, appHandler, allocator);
  auto bytes = makePayloadData(kSmallPayloadSize);

  susp.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto result = handler->onWrite(
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            bytes->clone()));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Write_TransportHandler_Medium, iters) {
  folly::BenchmarkSuspender susp;

  EventBase evb;
  BenchEndpointHandler appHandler;
  BenchAllocator allocator;
  auto [handler, pipeline] =
      createHandlerAndPipeline(evb, appHandler, allocator);
  auto bytes = makePayloadData(kMediumPayloadSize);

  susp.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto result = handler->onWrite(
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            bytes->clone()));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Write_TransportHandler_Large, iters) {
  folly::BenchmarkSuspender susp;

  EventBase evb;
  BenchEndpointHandler appHandler;
  BenchAllocator allocator;
  auto [handler, pipeline] =
      createHandlerAndPipeline(evb, appHandler, allocator);
  auto bytes = makePayloadData(kLargePayloadSize);

  susp.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    auto result = handler->onWrite(
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            bytes->clone()));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Read Path Benchmarks (readBufferAvailable -> pipeline dispatch)
// =============================================================================
BENCHMARK(Read_TransportHandler_Small, iters) {
  folly::BenchmarkSuspender susp;

  EventBase evb;
  BenchEndpointHandler appHandler;
  BenchAllocator allocator;
  auto [handler, pipeline] =
      createHandlerAndPipeline(evb, appHandler, allocator);
  auto bytes = makePayloadData(kSmallPayloadSize);

  susp.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    handler->readBufferAvailable(bytes->clone());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Read_TransportHandler_Medium, iters) {
  folly::BenchmarkSuspender susp;

  EventBase evb;
  BenchEndpointHandler appHandler;
  BenchAllocator allocator;
  auto [handler, pipeline] =
      createHandlerAndPipeline(evb, appHandler, allocator);
  auto bytes = makePayloadData(kMediumPayloadSize);

  susp.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    handler->readBufferAvailable(bytes->clone());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Read_TransportHandler_Large, iters) {
  folly::BenchmarkSuspender susp;

  EventBase evb;
  BenchEndpointHandler appHandler;
  BenchAllocator allocator;
  auto [handler, pipeline] =
      createHandlerAndPipeline(evb, appHandler, allocator);
  auto bytes = makePayloadData(kLargePayloadSize);

  susp.dismiss();
  for (size_t i = 0; i < iters; ++i) {
    handler->readBufferAvailable(bytes->clone());
  }
}

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
