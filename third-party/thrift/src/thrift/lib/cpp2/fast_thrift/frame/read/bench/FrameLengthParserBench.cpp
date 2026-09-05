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

#include <cassert>
#include <cstring>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameLengthParser.h>

using namespace folly;
using apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::frame::kMetadataLengthSize;
using apache::thrift::fast_thrift::frame::read::FrameLengthParser;

namespace {

constexpr size_t kSmallPayloadSize = 100;
constexpr size_t kMediumPayloadSize = 1024;
constexpr size_t kLargePayloadSize = 64 * 1024;
constexpr size_t kMultipleFrameCount = 10;

void writeFrameLength(uint8_t* buf, size_t length) {
  buf[0] = static_cast<uint8_t>((length >> 16) & 0xFF);
  buf[1] = static_cast<uint8_t>((length >> 8) & 0xFF);
  buf[2] = static_cast<uint8_t>(length & 0xFF);
}

std::unique_ptr<IOBuf> buildFrame(size_t payloadSize) {
  auto buf = IOBuf::create(kMetadataLengthSize + payloadSize);
  writeFrameLength(buf->writableData(), payloadSize);
  std::memset(buf->writableData() + kMetadataLengthSize, 'x', payloadSize);
  buf->append(kMetadataLengthSize + payloadSize);
  return buf;
}

std::unique_ptr<IOBuf> buildMultipleFrames(size_t payloadSize, size_t count) {
  IOBufQueue queue{IOBufQueue::cacheChainLength()};
  for (size_t i = 0; i < count; ++i) {
    queue.append(buildFrame(payloadSize));
  }
  return queue.move();
}

// Discards frames without storing them, so the measurement is the parser
// rather than a growing container.
auto nullSink() noexcept {
  return [](BytesPtr&& frame) noexcept {
    folly::doNotOptimizeAway(frame.get());
    return Result::Success;
  };
}

struct BenchmarkFrames {
  std::unique_ptr<IOBuf> small;
  std::unique_ptr<IOBuf> medium;
  std::unique_ptr<IOBuf> large;
  std::unique_ptr<IOBuf> multipleSmall;
  std::unique_ptr<IOBuf> multipleMedium;
};

const BenchmarkFrames& benchmarkFrames() {
  static const BenchmarkFrames frames{
      buildFrame(kSmallPayloadSize),
      buildFrame(kMediumPayloadSize),
      buildFrame(kLargePayloadSize),
      buildMultipleFrames(kSmallPayloadSize, kMultipleFrameCount),
      buildMultipleFrames(kMediumPayloadSize, kMultipleFrameCount),
  };
  return frames;
}

void setupBenchmarks() {
  (void)benchmarkFrames();
}

// Drives the socket-facing path: request a buffer, fill it, report the length.
// This is the arm that exercises buffer reuse, which the movable path skips.
void runReadBufferPath(FrameLengthParser& parser, const IOBuf& frame) {
  void* buf = nullptr;
  size_t avail = 0;
  parser.getReadBuffer(&buf, &avail);
  auto len = frame.length();
  assert(buf != nullptr);
  assert(len <= avail);
  std::memcpy(buf, frame.data(), len);
  auto result = parser.consume(len, nullSink());
  folly::doNotOptimizeAway(result);
}

} // namespace

// ============================================================================
// Single frame, fresh parser (includes first-buffer allocation)
// ============================================================================

BENCHMARK(SingleFrame_Small, n) {
  const auto& frames = benchmarkFrames();
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParser parser;
    auto result = parser.consumeBuffer(frames.small->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(SingleFrame_Medium, n) {
  const auto& frames = benchmarkFrames();
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParser parser;
    auto result = parser.consumeBuffer(frames.medium->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(SingleFrame_Large, n) {
  const auto& frames = benchmarkFrames();
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParser parser;
    auto result = parser.consumeBuffer(frames.large->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Multiple frames per call
// ============================================================================

BENCHMARK(MultipleFrames_Small_10, n) {
  const auto& frames = benchmarkFrames();
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParser parser;
    auto result =
        parser.consumeBuffer(frames.multipleSmall->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(MultipleFrames_Medium_10, n) {
  const auto& frames = benchmarkFrames();
  for (size_t i = 0; i < n; ++i) {
    FrameLengthParser parser;
    auto result =
        parser.consumeBuffer(frames.multipleMedium->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Reused parser (amortizes construction)
// ============================================================================

BENCHMARK(ReusedParser_SingleFrame_Small, n) {
  const auto& frames = benchmarkFrames();
  FrameLengthParser parser;
  for (size_t i = 0; i < n; ++i) {
    auto result = parser.consumeBuffer(frames.small->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(ReusedParser_SingleFrame_Medium, n) {
  const auto& frames = benchmarkFrames();
  FrameLengthParser parser;
  for (size_t i = 0; i < n; ++i) {
    auto result = parser.consumeBuffer(frames.medium->clone(), nullSink());
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Socket read path — the arm buffer reuse actually shows up in
// ============================================================================

BENCHMARK(ReadBufferPath_SingleFrame_Small, n) {
  const auto& frames = benchmarkFrames();
  FrameLengthParser parser;
  for (size_t i = 0; i < n; ++i) {
    runReadBufferPath(parser, *frames.small);
  }
}

BENCHMARK(ReadBufferPath_SingleFrame_Medium, n) {
  const auto& frames = benchmarkFrames();
  FrameLengthParser parser;
  for (size_t i = 0; i < n; ++i) {
    runReadBufferPath(parser, *frames.medium);
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  setupBenchmarks();
  folly::runBenchmarks();
  return 0;
}
