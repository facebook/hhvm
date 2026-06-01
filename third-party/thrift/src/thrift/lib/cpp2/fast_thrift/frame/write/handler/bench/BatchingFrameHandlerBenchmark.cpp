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
 * BatchingFrameHandler Performance Benchmarks
 *
 * Measures batching handler throughput and overhead.
 */

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>

#include <memory>
#include <vector>

using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::channel_pipeline;

namespace {

// =============================================================================
// Mock Context for Benchmarking
// =============================================================================

class BenchContext {
 public:
  explicit BenchContext(folly::EventBase* evb) : evb_(evb) {}

  folly::EventBase* eventBase() const { return evb_; }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    auto batch = msg.take<std::unique_ptr<folly::IOBuf>>();
    if (batch) {
      ++writeCount_;
      bytesWritten_ += batch->computeChainDataLength();
    }
    return Result::Success;
  }

  void awaitWriteReady() noexcept {}
  void cancelAwaitWriteReady() noexcept {}

  size_t writeCount() const { return writeCount_; }
  size_t bytesWritten() const { return bytesWritten_; }

 private:
  folly::EventBase* evb_;
  size_t writeCount_{0};
  size_t bytesWritten_{0};
};

// =============================================================================
// Benchmark Helpers
// =============================================================================

TypeErasedBox makeFrame(size_t size) {
  auto buf = folly::IOBuf::create(size);
  buf->append(size);
  return TypeErasedBox(std::move(buf));
}

// Pre-allocate frames to avoid allocation in timing loop
std::vector<TypeErasedBox> preallocateFrames(size_t count, size_t frameSize) {
  std::vector<TypeErasedBox> frames;
  frames.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    frames.push_back(makeFrame(frameSize));
  }
  return frames;
}

// =============================================================================
// Benchmarks: Throughput with Different Frame Sizes
// =============================================================================

void BM_BatchingHandler_SmallFrames(size_t iters, size_t frameSize) {
  folly::BenchmarkSuspender suspender;

  folly::EventBase evb;
  BenchContext ctx(&evb);

  BatchingHandlerConfig config{
      .maxPendingBytes = 64 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(ctx);

  auto frames = preallocateFrames(iters, frameSize);

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }

  // Final flush
  evb.loopOnce(EVLOOP_NONBLOCK);

  folly::doNotOptimizeAway(ctx.writeCount());
  folly::doNotOptimizeAway(ctx.bytesWritten());
}

BENCHMARK_PARAM(BM_BatchingHandler_SmallFrames, 64)
BENCHMARK_PARAM(BM_BatchingHandler_SmallFrames, 256)
BENCHMARK_PARAM(BM_BatchingHandler_SmallFrames, 1024)
BENCHMARK_PARAM(BM_BatchingHandler_SmallFrames, 4096)

BENCHMARK_DRAW_LINE();

// =============================================================================
// Benchmarks: Batching Efficiency
// =============================================================================

void BM_BatchingHandler_BatchSize(size_t iters, size_t maxPendingFrames) {
  folly::BenchmarkSuspender suspender;

  folly::EventBase evb;
  BenchContext ctx(&evb);

  BatchingHandlerConfig config{
      .maxPendingBytes = 1024 * 1024, // 1MB - won't hit this
      .maxPendingFrames = maxPendingFrames,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kFrameSize = 256;
  auto frames = preallocateFrames(iters, kFrameSize);

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }

  // Final flush
  evb.loopOnce(EVLOOP_NONBLOCK);

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_BatchingHandler_BatchSize, 1) // Immediate flush
BENCHMARK_PARAM(BM_BatchingHandler_BatchSize, 4)
BENCHMARK_PARAM(BM_BatchingHandler_BatchSize, 16)
BENCHMARK_PARAM(BM_BatchingHandler_BatchSize, 32)
BENCHMARK_PARAM(BM_BatchingHandler_BatchSize, 64)
BENCHMARK_PARAM(BM_BatchingHandler_BatchSize, 128)

BENCHMARK_DRAW_LINE();

// =============================================================================
// Benchmarks: Byte Threshold Flushing
// =============================================================================

void BM_BatchingHandler_ByteThreshold(size_t iters, size_t maxPendingKB) {
  folly::BenchmarkSuspender suspender;

  folly::EventBase evb;
  BenchContext ctx(&evb);

  BatchingHandlerConfig config{
      .maxPendingBytes = maxPendingKB * 1024,
      .maxPendingFrames = 1000, // Won't hit this
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kFrameSize = 1024; // 1KB frames
  auto frames = preallocateFrames(iters, kFrameSize);

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }

  // Final flush
  evb.loopOnce(EVLOOP_NONBLOCK);

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_BatchingHandler_ByteThreshold, 1) // 1KB - frequent flush
BENCHMARK_PARAM(BM_BatchingHandler_ByteThreshold, 8) // 8KB
BENCHMARK_PARAM(BM_BatchingHandler_ByteThreshold, 32) // 32KB
BENCHMARK_PARAM(BM_BatchingHandler_ByteThreshold, 64) // 64KB
BENCHMARK_PARAM(BM_BatchingHandler_ByteThreshold, 256) // 256KB

BENCHMARK_DRAW_LINE();

// =============================================================================
// Benchmarks: Loop Callback Overhead
// =============================================================================

BENCHMARK(BM_BatchingHandler_LoopCallbackFlush, iters) {
  folly::BenchmarkSuspender suspender;

  folly::EventBase evb;
  BenchContext ctx(&evb);

  BatchingHandlerConfig config{
      .maxPendingBytes = 1024 * 1024, // Large thresholds
      .maxPendingFrames = 1000,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kFrameSize = 256;
  auto frames = preallocateFrames(iters, kFrameSize);

  suspender.dismiss();

  // Each iteration: write one frame, flush via loop callback
  for (size_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
    evb.loopOnce(EVLOOP_NONBLOCK);
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Benchmarks: Comparison - Batched vs Unbatched
// =============================================================================

BENCHMARK(BM_DirectWrite_NoBatching, iters) {
  folly::BenchmarkSuspender suspender;

  folly::EventBase evb;
  BenchContext ctx(&evb);

  constexpr size_t kFrameSize = 256;
  auto frames = preallocateFrames(iters, kFrameSize);

  suspender.dismiss();

  // Direct writes without batching handler
  for (size_t i = 0; i < iters; ++i) {
    (void)ctx.fireWrite(std::move(frames[i]));
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_RELATIVE(BM_BatchedWrite_32Frames, iters) {
  folly::BenchmarkSuspender suspender;

  folly::EventBase evb;
  BenchContext ctx(&evb);

  BatchingHandlerConfig config{
      .maxPendingBytes = 1024 * 1024,
      .maxPendingFrames = 32,
  };
  BatchingFrameHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kFrameSize = 256;
  auto frames = preallocateFrames(iters, kFrameSize);

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }

  evb.loopOnce(EVLOOP_NONBLOCK);

  folly::doNotOptimizeAway(ctx.writeCount());
}

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
