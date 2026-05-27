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
 * WriteBufferBackpressureHandler microbenchmarks.
 *
 * The hot path is `onWrite` in the unsaturated case — a single flag
 * check + forward. This bench guards that hot path against regressions
 * (e.g. accidentally adding allocation or virtual dispatch). It also
 * measures the buffering and drain paths to track the cost of
 * absorbing/releasing backpressure.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

#include <cstdint>
#include <utility>
#include <vector>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/WriteBufferBackpressureHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::thrift;

namespace {

// Stand-in for PipelineImpl exposing the single method the handler
// reaches for via ctx.pipeline()->onReadReady() on full drain.
struct BenchPipeline {
  void onReadReady() noexcept {}
};

// Local bench context with a configurable fireWrite result so we can
// arm the handler's backpressure flag from outside. fireRead/fireException
// are unused by the bench surface but kept so BenchCtx remains a complete
// stand-in for the ContextApi concept.
class BenchCtx {
 public:
  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  Result fireRead(TypeErasedBox&&) noexcept { return Result::Success; }
  Result fireWrite(TypeErasedBox&&) noexcept { return nextWriteResult; }
  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  void fireException(folly::exception_wrapper&&) noexcept {}

  // Backpressure / ready-signal stubs — no-ops so the bench measures
  // only the handler's own work, not the registration plumbing.
  void awaitWriteReady() noexcept {}
  void cancelAwaitWriteReady() noexcept {}
  BenchPipeline* pipeline() noexcept { return &pipeline_; }

  Result nextWriteResult{Result::Success};
  BenchPipeline pipeline_;
};

ThriftServerResponseMessage makeResponse(uint32_t streamId) {
  return ThriftServerResponseMessage{
      .payload = ThriftInitialResponsePayload{
          .data = nullptr,
          .metadata = nullptr,
          .streamId = streamId,
      }};
}

// Hot path: handler is unsaturated; onWrite is a single flag check
// followed by ctx.fireWrite. Measures the per-write overhead the
// handler adds to a normal response.
BENCHMARK(OnWrite_PassThrough, iters) {
  BenchmarkSuspender suspender;

  WriteBufferBackpressureHandler<BenchCtx> handler;
  BenchCtx ctx;

  std::vector<TypeErasedBox> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(erase_and_box(makeResponse(static_cast<uint32_t>(i))));
  }

  suspender.dismiss();

  for (auto& req : requests) {
    auto result = handler.onWrite(ctx, std::move(req));
    doNotOptimizeAway(result);
  }
}

// Buffered path: handler is saturated; onWrite enqueues into the FIFO
// and returns Success without forwarding. Measures the cost of
// absorbing a single response under backpressure (queue push +
// TypeErasedBox::take + ResponseMessage move).
BENCHMARK(OnWrite_BufferWhileBackpressured, iters) {
  BenchmarkSuspender suspender;

  WriteBufferBackpressureHandler<BenchCtx> handler;
  BenchCtx ctx;

  // Arm the backpressure flag with one priming write.
  ctx.nextWriteResult = Result::Backpressure;
  (void)handler.onWrite(ctx, erase_and_box(makeResponse(0)));

  std::vector<TypeErasedBox> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(
        erase_and_box(makeResponse(static_cast<uint32_t>(i + 1))));
  }

  suspender.dismiss();

  for (auto& req : requests) {
    auto result = handler.onWrite(ctx, std::move(req));
    doNotOptimizeAway(result);
  }
}

// Drain path: cycles of (queue 8, flush). Measures amortized per-message
// drain cost — the FIFO pop + erase_and_box + ctx.fireWrite.
//
// kDrainBatch tracks a typical interval-batching window; tune if real
// backpressure batches diverge meaningfully from this.
constexpr size_t kDrainBatch = 8;

BENCHMARK(OnWriteReady_DrainBatched, iters) {
  BenchmarkSuspender suspender;

  WriteBufferBackpressureHandler<BenchCtx> handler;
  BenchCtx ctx;

  const size_t cycles = (iters + kDrainBatch - 1) / kDrainBatch;

  suspender.dismiss();

  for (size_t c = 0; c < cycles; ++c) {
    // Saturate + queue kDrainBatch behind it.
    ctx.nextWriteResult = Result::Backpressure;
    (void)handler.onWrite(
        ctx, erase_and_box(makeResponse(static_cast<uint32_t>(c))));
    for (size_t i = 0; i < kDrainBatch - 1; ++i) {
      (void)handler.onWrite(
          ctx, erase_and_box(makeResponse(static_cast<uint32_t>(c))));
    }

    // Downstream signals room; drain the full batch.
    ctx.nextWriteResult = Result::Success;
    handler.onWriteReady(ctx);
    doNotOptimizeAway(handler.pendingResponseCount());
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
