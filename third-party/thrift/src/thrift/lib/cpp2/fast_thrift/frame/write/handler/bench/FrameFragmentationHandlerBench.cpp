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
 * FrameFragmentationHandler Microbenchmarks
 *
 * Measures SRPT scheduling + fast-path bypass overhead.
 * All message allocation happens outside the timing loop.
 */

#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>

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

// ============================================================================
// Benchmark Mock Context
// ============================================================================
// Minimal context that swallows writes - measures handler overhead only

class BenchContext {
 public:
  explicit BenchContext(folly::EventBase* evb) : evb_(evb) {}

  folly::EventBase* getEventBase() const { return evb_; }

  Result fireWrite(TypeErasedBox&&) {
    ++writeCount_;
    return Result::Success;
  }

  void awaitWriteReady() {}
  void cancelAwaitWriteReady() {}
  void deactivate() {}

  size_t writeCount() const { return writeCount_; }
  void resetWriteCount() { writeCount_ = 0; }

 private:
  folly::EventBase* evb_;
  size_t writeCount_{0};
};

// ============================================================================
// Pre-allocated Frame Factory
// ============================================================================

std::unique_ptr<folly::IOBuf> makePayload(size_t size) {
  auto buf = folly::IOBuf::create(size);
  buf->append(size);
  return buf;
}

TypeErasedBox makeFrame(uint32_t streamId, size_t payloadSize) {
  OutboundFrame frame;
  frame.streamId = streamId;
  frame.frameType = FrameType::PAYLOAD;
  frame.flags = 0;
  frame.payload = makePayload(payloadSize);
  return TypeErasedBox(std::move(frame));
}

// ============================================================================
// Benchmark: Fast-Path Bypass (Direct fireWrite, zero queueing)
// ============================================================================
// Frames ≤ minSizeToFragment (1024) go straight through ctx.fireWrite()
// without touching the deque or SrptHeap. This is the hot path for ~99%
// of traffic at 200+ QPS.

void BM_FastPathDirectBypass(uint32_t iters, size_t frameSize) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  std::vector<TypeErasedBox> frames;
  frames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    frames.push_back(makeFrame(i % 1000, frameSize));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }
  // No evb.loopOnce() needed — frames are sent inline, never queued.

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_FastPathDirectBypass, 64) // 64B — tiny RPC
BENCHMARK_PARAM(BM_FastPathDirectBypass, 256) // 256B — small response
BENCHMARK_PARAM(BM_FastPathDirectBypass, 512) // 512B
BENCHMARK_PARAM(
    BM_FastPathDirectBypass,
    1024) // 1KB — exactly minSizeToFragment

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Small Frame, Heap Empty (second fast-path tier)
// ============================================================================
// Frames > minSizeToFragment but ≤ maxFragmentSize with empty heap also
// bypass the queue and go directly through fireWrite.

void BM_SmallFrameHeapEmpty(uint32_t iters, size_t frameSize) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  std::vector<TypeErasedBox> frames;
  frames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    frames.push_back(makeFrame(i % 1000, frameSize));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }
  // No evb.loopOnce() — heap is always empty, so fast-path fires inline.

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(
    BM_SmallFrameHeapEmpty,
    2048) // 2KB — just above minSizeToFragment
BENCHMARK_PARAM(BM_SmallFrameHeapEmpty, 8192) // 8KB
BENCHMARK_PARAM(BM_SmallFrameHeapEmpty, 32768) // 32KB
BENCHMARK_PARAM(BM_SmallFrameHeapEmpty, 65536) // 64KB — exactly maxFragmentSize

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Medium Frame, Heap Non-Empty (immediateQueue path)
// ============================================================================
// When the heap has pending fragments, frames ≤ maxFragmentSize go to
// the immediateQueue to preserve ordering. Measures deque overhead.

void BM_MediumFrameWithPending(uint32_t iters, size_t numMediumFrames) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kMediumSize = 4 * 1024; // 4KB — above minSizeToFragment
  constexpr size_t kLargeSize = 256 * 1024; // 256KB — needs fragmentation

  // Pre-allocate: one large frame + N medium frames per iteration.
  struct IterFrames {
    TypeErasedBox largeFrame;
    std::vector<TypeErasedBox> mediumFrames;
  };
  std::vector<IterFrames> allFrames;
  allFrames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    IterFrames f;
    f.largeFrame = makeFrame(999, kLargeSize);
    f.mediumFrames.reserve(numMediumFrames);
    for (size_t j = 0; j < numMediumFrames; ++j) {
      f.mediumFrames.push_back(
          makeFrame(static_cast<uint32_t>(j), kMediumSize));
    }
    allFrames.push_back(std::move(f));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    // Populate heap with a large frame.
    (void)handler.onWrite(ctx, std::move(allFrames[i].largeFrame));
    // Medium frames now go to immediateQueue.
    for (size_t j = 0; j < numMediumFrames; ++j) {
      (void)handler.onWrite(ctx, std::move(allFrames[i].mediumFrames[j]));
    }
    evb.loopOnce();
    ctx.resetWriteCount();
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_MediumFrameWithPending, 1) // 1 medium + 1 large
BENCHMARK_PARAM(BM_MediumFrameWithPending, 4) // 4 medium + 1 large
BENCHMARK_PARAM(BM_MediumFrameWithPending, 16) // 16 medium + 1 large
BENCHMARK_PARAM(BM_MediumFrameWithPending, 64) // 64 medium + 1 large

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Large Frame Fragmentation (Single Stream)
// ============================================================================
// Measures fragmentation + SrptHeap overhead for varying payload sizes.

void BM_LargeFrameFragmentation(uint32_t iters, size_t frameSize) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  std::vector<TypeErasedBox> frames;
  frames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    frames.push_back(makeFrame(1, frameSize));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
    evb.loopOnce();
    ctx.resetWriteCount();
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(
    BM_LargeFrameFragmentation,
    65537) // Just over threshold (2 frags)
BENCHMARK_PARAM(BM_LargeFrameFragmentation, 131072) // 128KB (2 frags)
BENCHMARK_PARAM(BM_LargeFrameFragmentation, 262144) // 256KB (4 frags)
BENCHMARK_PARAM(BM_LargeFrameFragmentation, 524288) // 512KB (8 frags)
BENCHMARK_PARAM(BM_LargeFrameFragmentation, 1048576) // 1MB (16 frags)

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Multi-Stream SRPT Scheduling
// ============================================================================
// Measures SRPT scheduling overhead with multiple concurrent streams.
// Unlike round-robin which blindly cycles, SRPT always picks the stream
// with the fewest remaining bytes → SrptHeap extractMin/update on each
// fragment.

void BM_MultiStreamSRPT(uint32_t iters, size_t numStreams) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kPayloadSize = 256 * 1024; // 4 fragments each

  std::vector<std::vector<TypeErasedBox>> iterFrames;
  iterFrames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    std::vector<TypeErasedBox> frames;
    frames.reserve(numStreams);
    for (size_t s = 0; s < numStreams; ++s) {
      frames.push_back(makeFrame(static_cast<uint32_t>(s), kPayloadSize));
    }
    iterFrames.push_back(std::move(frames));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    for (size_t s = 0; s < numStreams; ++s) {
      (void)handler.onWrite(ctx, std::move(iterFrames[i][s]));
    }
    evb.loopOnce(); // SRPT flush — peekMin/update/erase per fragment
    ctx.resetWriteCount();
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_MultiStreamSRPT, 1) // Baseline: single stream
BENCHMARK_PARAM(BM_MultiStreamSRPT, 2)
BENCHMARK_PARAM(BM_MultiStreamSRPT, 4)
BENCHMARK_PARAM(BM_MultiStreamSRPT, 8)
BENCHMARK_PARAM(BM_MultiStreamSRPT, 16)
BENCHMARK_PARAM(BM_MultiStreamSRPT, 32)

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Mixed Workload — Fast-Path vs SRPT
// ============================================================================
// Realistic production traffic: mostly small frames hitting the fast-path,
// with occasional large frames needing fragmentation + SRPT scheduling.
// Measures blended throughput.

void BM_MixedWorkload(uint32_t iters, size_t largeFramePercent) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kSmallSize = 512; // 512B — fast-path (≤ minSizeToFragment)
  constexpr size_t kLargeSize = 256 * 1024; // 256KB — SRPT path

  std::vector<TypeErasedBox> frames;
  frames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    bool isLarge = (i % 100) < largeFramePercent;
    size_t size = isLarge ? kLargeSize : kSmallSize;
    frames.push_back(makeFrame(i % 100, size));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }
  evb.loopOnce(); // Flush remaining

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_MixedWorkload, 0) // 100% small — all fast-path
BENCHMARK_PARAM(BM_MixedWorkload, 1) // 99% small — realistic production
BENCHMARK_PARAM(BM_MixedWorkload, 5) // 95% small
BENCHMARK_PARAM(BM_MixedWorkload, 10) // 90% small
BENCHMARK_PARAM(BM_MixedWorkload, 25) // 75% small
BENCHMARK_PARAM(BM_MixedWorkload, 50) // 50/50

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Threshold-Triggered Flush
// ============================================================================
// Measures overhead when maxPendingBytes triggers inline flush during onWrite.

void BM_ThresholdFlush(uint32_t iters, size_t maxPendingKB) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{
      .maxFragmentSize = 64 * 1024,
      .maxPendingBytes = maxPendingKB * 1024,
  };
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kFrameSize = 128 * 1024; // 128KB per frame

  std::vector<TypeErasedBox> frames;
  frames.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    frames.push_back(makeFrame(i % 100, kFrameSize));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    (void)handler.onWrite(ctx, std::move(frames[i]));
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_ThresholdFlush, 128) // Flush every frame
BENCHMARK_PARAM(BM_ThresholdFlush, 256) // Flush every 2 frames
BENCHMARK_PARAM(BM_ThresholdFlush, 512) // Flush every 4 frames
BENCHMARK_PARAM(BM_ThresholdFlush, 1024) // Batch more

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Fragment Extraction Overhead
// ============================================================================
// Measures PerStreamState::nextFragment() overhead in isolation.

void BM_FragmentExtraction(uint32_t iters, size_t fragmentSize) {
  folly::BenchmarkSuspender braces;

  constexpr size_t kPayloadSize = 16 * 1024 * 1024; // 16MB
  auto payload = makePayload(kPayloadSize);

  PerStreamState state;
  state.streamId = 1;
  state.frameType = FrameType::PAYLOAD;
  state.originalFlags = 0;
  state.init(std::move(payload));

  braces.dismiss();

  size_t totalExtracted = 0;
  for (uint32_t i = 0; i < iters; ++i) {
    if (!state.hasMore()) {
      braces.rehire();
      state.init(makePayload(kPayloadSize));
      braces.dismiss();
    }
    auto [frag, follows] = state.nextFragment(fragmentSize);
    totalExtracted += frag ? frag->computeChainDataLength() : 0;
  }

  folly::doNotOptimizeAway(totalExtracted);
}

BENCHMARK_PARAM(BM_FragmentExtraction, 4096) // 4KB fragments
BENCHMARK_PARAM(BM_FragmentExtraction, 16384) // 16KB fragments
BENCHMARK_PARAM(BM_FragmentExtraction, 65536) // 64KB fragments (default)
BENCHMARK_PARAM(BM_FragmentExtraction, 131072) // 128KB fragments

BENCHMARK_DRAW_LINE();

// ============================================================================
// Benchmark: Steady-State 200 QPS/connection Simulation
// ============================================================================
// Models the hot path described by the user: 2K QPS over 10 connections
// = ~200 QPS per connection. 99% are small request-response (fast-path),
// 1% are streaming/bulk (SRPT path). Measures amortized per-frame cost.

void BM_SteadyState200QPS(uint32_t iters, size_t /*batchSize*/) {
  folly::BenchmarkSuspender braces;

  folly::EventBase evb;
  BenchContext ctx(&evb);
  FragmentationHandlerConfig config{.maxFragmentSize = 64 * 1024};
  FrameFragmentationHandler handler(config);
  handler.handlerAdded(ctx);

  constexpr size_t kBatchSize = 200; // 200 frames per "tick"
  constexpr size_t kSmallSize = 256; // 256B fast-path
  constexpr size_t kLargeSize = 512 * 1024; // 512KB SRPT path

  // Pre-allocate batches.
  std::vector<std::vector<TypeErasedBox>> batches;
  batches.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    std::vector<TypeErasedBox> batch;
    batch.reserve(kBatchSize);
    for (size_t j = 0; j < kBatchSize; ++j) {
      bool isLarge = (j % 100 == 0); // 1% large
      size_t size = isLarge ? kLargeSize : kSmallSize;
      batch.push_back(makeFrame(static_cast<uint32_t>(j), size));
    }
    batches.push_back(std::move(batch));
  }

  braces.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    for (size_t j = 0; j < kBatchSize; ++j) {
      (void)handler.onWrite(ctx, std::move(batches[i][j]));
    }
    evb.loopOnce();
    ctx.resetWriteCount();
  }

  folly::doNotOptimizeAway(ctx.writeCount());
}

BENCHMARK_PARAM(BM_SteadyState200QPS, 200)

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
