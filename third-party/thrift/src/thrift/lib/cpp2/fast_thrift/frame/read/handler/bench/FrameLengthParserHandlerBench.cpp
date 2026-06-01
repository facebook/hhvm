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

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::channel_pipeline;

namespace {
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::detail;

// Frame sizes to benchmark
constexpr size_t kSmallPayloadSize = 100;
constexpr size_t kMediumPayloadSize = 1024;
constexpr size_t kLargePayloadSize = 64 * 1024;

// Write 3-byte big-endian frame length
void writeFrameLength(uint8_t* buf, size_t length) {
  buf[0] = static_cast<uint8_t>((length >> 16) & 0xFF);
  buf[1] = static_cast<uint8_t>((length >> 8) & 0xFF);
  buf[2] = static_cast<uint8_t>(length & 0xFF);
}

// Build a frame with length prefix and payload
std::unique_ptr<IOBuf> buildFrame(size_t payloadSize) {
  auto buf = IOBuf::create(kMetadataLengthSize + payloadSize);
  writeFrameLength(buf->writableData(), payloadSize);
  std::memset(buf->writableData() + kMetadataLengthSize, 'x', payloadSize);
  buf->append(kMetadataLengthSize + payloadSize);
  return buf;
}

// Build multiple frames in a single buffer
std::unique_ptr<IOBuf> buildMultipleFrames(size_t payloadSize, size_t count) {
  IOBufQueue queue{IOBufQueue::cacheChainLength()};
  for (size_t i = 0; i < count; ++i) {
    queue.append(buildFrame(payloadSize));
  }
  return queue.move();
}

/**
 * NullContext - minimal context that accepts frames without storing them.
 * Used for benchmarking to avoid measurement noise from vector operations.
 */
class NullContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    // Take ownership to trigger move, then discard
    auto buf = msg.take<BytesPtr>();
    folly::doNotOptimizeAway(buf.get());
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&&) noexcept {}
};

// Pre-created frames for benchmarks
std::unique_ptr<IOBuf> gSmallFrame;
std::unique_ptr<IOBuf> gMediumFrame;
std::unique_ptr<IOBuf> gLargeFrame;
std::unique_ptr<IOBuf> gMultipleSmallFrames;
std::unique_ptr<IOBuf> gMultipleMediumFrames;

constexpr size_t kMultipleFrameCount = 10;

void setupBenchmarks() {
  gSmallFrame = buildFrame(kSmallPayloadSize);
  gMediumFrame = buildFrame(kMediumPayloadSize);
  gLargeFrame = buildFrame(kLargePayloadSize);
  gMultipleSmallFrames =
      buildMultipleFrames(kSmallPayloadSize, kMultipleFrameCount);
  gMultipleMediumFrames =
      buildMultipleFrames(kMediumPayloadSize, kMultipleFrameCount);
}

} // namespace

// ============================================================================
// Single Frame Benchmarks
// ============================================================================

BENCHMARK(SingleFrame_Small, n) {
  NullContext ctx;
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParserHandler handler;
    auto frame = gSmallFrame->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frame)));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(SingleFrame_Medium, n) {
  NullContext ctx;
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParserHandler handler;
    auto frame = gMediumFrame->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frame)));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(SingleFrame_Large, n) {
  NullContext ctx;
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParserHandler handler;
    auto frame = gLargeFrame->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frame)));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Multiple Frames Benchmarks (10 frames per call)
// ============================================================================

BENCHMARK(MultipleFrames_Small_10, n) {
  NullContext ctx;
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParserHandler handler;
    auto frames = gMultipleSmallFrames->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames)));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(MultipleFrames_Medium_10, n) {
  NullContext ctx;
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParserHandler handler;
    auto frames = gMultipleMediumFrames->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames)));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Reused Handler Benchmarks (amortize handler construction)
// ============================================================================

BENCHMARK(ReusedHandler_SingleFrame_Small, n) {
  NullContext ctx;
  FrameLengthParserHandler handler;
  for (size_t i = 0; i < n; ++i) {
    auto frame = gSmallFrame->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frame)));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(ReusedHandler_SingleFrame_Medium, n) {
  NullContext ctx;
  FrameLengthParserHandler handler;
  for (size_t i = 0; i < n; ++i) {
    auto frame = gMediumFrame->clone();
    auto result = handler.onRead(ctx, erase_and_box(std::move(frame)));
    folly::doNotOptimizeAway(result);
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  setupBenchmarks();
  folly::runBenchmarks();
  return 0;
}
