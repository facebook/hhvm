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
 * FrameDefragmentationHandler Microbenchmarks
 *
 * Measures the overhead of fragment reassembly to ensure the handler doesn't
 * regress performance. Key scenarios:
 * - Passthrough: non-fragmented frames (fast path)
 * - Two-fragment reassembly: common case
 * - Many-fragment reassembly: scaling behavior
 * - Interleaved streams: map lookup cost
 * - Large payloads: IOBufQueue efficiency
 */

#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>

#include <cstring>

namespace apache::thrift::fast_thrift::frame::read::handler {
namespace {

// ============================================================================
// Test Utilities
// ============================================================================

/**
 * Minimal context for benchmarking - just counts operations.
 */
class BenchContext {
 public:
  apache::thrift::fast_thrift::channel_pipeline::Result fireRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&) {
    ++readCount;
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void fireException(folly::exception_wrapper&&) noexcept {}

  apache::thrift::fast_thrift::channel_pipeline::BytesPtr allocate(
      size_t size) noexcept {
    return folly::IOBuf::create(size);
  }

  size_t readCount{0};
};

/**
 * Create a ParsedFrame for benchmarking.
 */
ParsedFrame makeFrame(
    FrameType type, uint32_t streamId, bool hasFollows, size_t dataSize) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(type);
  frame.metadata.streamId = streamId;
  frame.metadata.flags_ = hasFollows
      ? ::apache::thrift::fast_thrift::frame::detail::kFollowsBit
      : 0;
  frame.metadata.payloadSize = static_cast<uint32_t>(dataSize);
  frame.metadata.metadataSize = 0;
  frame.metadata.payloadOffset = 0;

  auto buf = folly::IOBuf::create(dataSize);
  buf->append(dataSize);
  std::memset(buf->writableData(), 'X', dataSize);
  frame.buffer = std::move(buf);

  return frame;
}

/**
 * Clone a frame for reuse in iterations.
 */
ParsedFrame cloneFrame(const ParsedFrame& frame) {
  ParsedFrame result;
  result.metadata = frame.metadata;
  if (frame.buffer) {
    result.buffer = frame.buffer->clone();
  }
  return result;
}

// ============================================================================
// Passthrough Benchmarks
// ============================================================================

/**
 * Baseline: passthrough non-fragmented frames through the handler.
 * This measures the fast path where hasFollows=false and no pending fragments.
 */
BENCHMARK(Passthrough_NonFragmented, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  // Pre-create template frame
  auto templateFrame = makeFrame(
      FrameType::REQUEST_RESPONSE, 1, /*hasFollows=*/false, /*dataSize=*/64);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto frame = cloneFrame(templateFrame);
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(frame)));
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

// ============================================================================
// Fragment Reassembly Benchmarks
// ============================================================================

/**
 * Two-fragment reassembly (common case).
 * Each iteration: first fragment + final fragment = 1 complete frame.
 */
BENCHMARK(TwoFragment_Reassembly, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, /*dataSize=*/32);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, /*dataSize=*/32);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t streamId = static_cast<uint32_t>(i % 10000) + 1;

    auto first = cloneFrame(firstTemplate);
    first.metadata.streamId = streamId;

    auto last = cloneFrame(lastTemplate);
    last.metadata.streamId = streamId;

    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(first)));
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(last)));
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

/**
 * Ten-fragment reassembly.
 * Measures overhead of multiple fragment reassembly.
 */
BENCHMARK(TenFragment_Reassembly, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  constexpr int kFragments = 10;

  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, /*dataSize=*/100);
  auto midTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/true, /*dataSize=*/100);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, /*dataSize=*/100);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t streamId = static_cast<uint32_t>(i % 10000) + 1;

    // First fragment
    auto first = cloneFrame(firstTemplate);
    first.metadata.streamId = streamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(first)));

    // Middle fragments
    for (int f = 1; f < kFragments - 1; ++f) {
      auto mid = cloneFrame(midTemplate);
      mid.metadata.streamId = streamId;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(mid)));
    }

    // Final fragment
    auto last = cloneFrame(lastTemplate);
    last.metadata.streamId = streamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(last)));
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

// ============================================================================
// Interleaved Stream Benchmarks
// ============================================================================

/**
 * 10 concurrent fragmenting streams with interleaved arrival.
 * Measures map lookup cost with moderate concurrency.
 */
BENCHMARK(Interleaved_10Streams, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  constexpr int kStreams = 10;
  constexpr int kFragmentsPerStream = 3;

  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, /*dataSize=*/50);
  auto midTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/true, /*dataSize=*/50);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, /*dataSize=*/50);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t baseStreamId = static_cast<uint32_t>((i * kStreams) % 10000) + 1;

    // Start all streams
    for (int s = 0; s < kStreams; ++s) {
      auto first = cloneFrame(firstTemplate);
      first.metadata.streamId = baseStreamId + s;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(first)));
    }

    // Middle fragments (interleaved)
    for (int f = 1; f < kFragmentsPerStream - 1; ++f) {
      for (int s = 0; s < kStreams; ++s) {
        auto mid = cloneFrame(midTemplate);
        mid.metadata.streamId = baseStreamId + s;
        (void)handler.onRead(
            ctx,
            apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
                std::move(mid)));
      }
    }

    // Final fragments (interleaved)
    for (int s = 0; s < kStreams; ++s) {
      auto last = cloneFrame(lastTemplate);
      last.metadata.streamId = baseStreamId + s;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(last)));
    }
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

/**
 * 100 concurrent fragmenting streams.
 * Tests F14Map scaling behavior with higher concurrency.
 */
BENCHMARK(Interleaved_100Streams, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  constexpr int kStreams = 100;

  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, /*dataSize=*/50);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, /*dataSize=*/50);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t baseStreamId = static_cast<uint32_t>((i * kStreams) % 100000) + 1;

    // Start all streams
    for (int s = 0; s < kStreams; ++s) {
      auto first = cloneFrame(firstTemplate);
      first.metadata.streamId = baseStreamId + s;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(first)));
    }

    // Complete all streams
    for (int s = 0; s < kStreams; ++s) {
      auto last = cloneFrame(lastTemplate);
      last.metadata.streamId = baseStreamId + s;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(last)));
    }
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

// ============================================================================
// Large Payload Benchmarks
// ============================================================================

/**
 * 1MB payload in 64KB fragments (16 fragments).
 * Tests IOBufQueue efficiency with larger payloads.
 */
BENCHMARK(LargePayload_1MB, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  constexpr size_t kChunkSize = 64 * 1024; // 64KB
  constexpr int kChunks = 16; // Total = 1MB

  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, kChunkSize);
  auto midTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/true, kChunkSize);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, kChunkSize);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t streamId = static_cast<uint32_t>(i % 10000) + 1;

    auto first = cloneFrame(firstTemplate);
    first.metadata.streamId = streamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(first)));

    for (int c = 1; c < kChunks - 1; ++c) {
      auto mid = cloneFrame(midTemplate);
      mid.metadata.streamId = streamId;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(mid)));
    }

    auto last = cloneFrame(lastTemplate);
    last.metadata.streamId = streamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(last)));
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

/**
 * 10MB payload in 256KB fragments (40 fragments).
 * Tests IOBufQueue efficiency with very large payloads.
 */
BENCHMARK(LargePayload_10MB, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  constexpr size_t kChunkSize = 256 * 1024; // 256KB
  constexpr int kChunks = 40; // Total = 10MB

  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, kChunkSize);
  auto midTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/true, kChunkSize);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, kChunkSize);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t streamId = static_cast<uint32_t>(i % 10000) + 1;

    auto first = cloneFrame(firstTemplate);
    first.metadata.streamId = streamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(first)));

    for (int c = 1; c < kChunks - 1; ++c) {
      auto mid = cloneFrame(midTemplate);
      mid.metadata.streamId = streamId;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(mid)));
    }

    auto last = cloneFrame(lastTemplate);
    last.metadata.streamId = streamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(last)));
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

// ============================================================================
// Mixed Workload Benchmarks
// ============================================================================

/**
 * Mixed workload: 50% complete frames, 50% fragmented (2 fragments each).
 * Measures realistic scenario where passthrough happens while fragments
 * pending. This tests the cost of map lookup for non-fragmented frames when the
 * map is NOT empty (has pending fragments from other streams).
 */
BENCHMARK(Mixed_PassthroughAndFragmented, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  auto completeTemplate = makeFrame(
      FrameType::REQUEST_FNF, 0, /*hasFollows=*/false, /*dataSize=*/64);
  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, /*dataSize=*/32);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, /*dataSize=*/32);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t fragStreamId = static_cast<uint32_t>((i * 2) % 10000) + 1;
    uint32_t completeStreamId = static_cast<uint32_t>((i * 2 + 1) % 10000) + 1;

    // First fragment (stream now pending in map)
    auto first = cloneFrame(firstTemplate);
    first.metadata.streamId = fragStreamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(first)));

    // Complete frame while fragment pending (tests map lookup with non-empty
    // map)
    auto complete = cloneFrame(completeTemplate);
    complete.metadata.streamId = completeStreamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(complete)));

    // Final fragment (completes the fragmented stream)
    auto last = cloneFrame(lastTemplate);
    last.metadata.streamId = fragStreamId;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(last)));
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

/**
 * Mixed workload with multiple pending streams.
 * Tests passthrough performance when map has many pending entries.
 */
BENCHMARK(Mixed_PassthroughWith10PendingStreams, iters) {
  folly::BenchmarkSuspender susp;
  FrameDefragmentationHandler handler;
  BenchContext ctx;

  constexpr int kPendingStreams = 10;

  auto completeTemplate = makeFrame(
      FrameType::REQUEST_FNF, 0, /*hasFollows=*/false, /*dataSize=*/64);
  auto firstTemplate = makeFrame(
      FrameType::REQUEST_RESPONSE, 0, /*hasFollows=*/true, /*dataSize=*/32);
  auto lastTemplate =
      makeFrame(FrameType::PAYLOAD, 0, /*hasFollows=*/false, /*dataSize=*/32);

  susp.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t baseStreamId =
        static_cast<uint32_t>((i * (kPendingStreams + 1)) % 10000) + 1;

    // Start multiple fragmenting streams (fill the pending map)
    for (int s = 0; s < kPendingStreams; ++s) {
      auto first = cloneFrame(firstTemplate);
      first.metadata.streamId = baseStreamId + s;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(first)));
    }

    // Send complete frame while many fragments pending
    auto complete = cloneFrame(completeTemplate);
    complete.metadata.streamId = baseStreamId + kPendingStreams;
    (void)handler.onRead(
        ctx,
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(complete)));

    // Complete all pending streams
    for (int s = 0; s < kPendingStreams; ++s) {
      auto last = cloneFrame(lastTemplate);
      last.metadata.streamId = baseStreamId + s;
      (void)handler.onRead(
          ctx,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(last)));
    }
  }

  folly::doNotOptimizeAway(ctx.readCount);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read::handler

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
