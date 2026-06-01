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

#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

using namespace folly;
using namespace apache::thrift::rocket;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;

namespace {

// Test payload sizes
constexpr size_t kSmallPayloadSize = 100;
constexpr size_t kMediumPayloadSize = 1024;
constexpr size_t kLargePayloadSize = 64 * 1024;

std::string makePayloadData(size_t size) {
  std::string data(size, 'x');
  return data;
}

// Create test frames using FastThrift's serialization
// FastThrift's format (no frame length prefix) is what both
// Rocket's constructors and our parseFrame() expect.

std::unique_ptr<IOBuf> createRequestResponseFrame(
    uint32_t streamId, size_t payloadSize) {
  auto data = IOBuf::copyBuffer(makePayloadData(payloadSize));
  auto metadata = IOBuf::copyBuffer("metadata");
  return serialize(
      RequestResponseHeader{.streamId = streamId},
      std::move(metadata),
      std::move(data));
}

std::unique_ptr<IOBuf> createPayloadFrame(
    uint32_t streamId, size_t payloadSize) {
  auto data = IOBuf::copyBuffer(makePayloadData(payloadSize));
  auto metadata = IOBuf::copyBuffer("metadata");
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true},
      std::move(metadata),
      std::move(data));
}

std::unique_ptr<IOBuf> createRequestStreamFrame(
    uint32_t streamId, size_t payloadSize, uint32_t initialN) {
  auto data = IOBuf::copyBuffer(makePayloadData(payloadSize));
  auto metadata = IOBuf::copyBuffer("metadata");
  return serialize(
      RequestStreamHeader{.streamId = streamId, .initialRequestN = initialN},
      std::move(metadata),
      std::move(data));
}

std::unique_ptr<IOBuf> createRequestNFrame(
    uint32_t streamId, uint32_t requestN) {
  return serialize(RequestNHeader{.streamId = streamId, .requestN = requestN});
}

// Pre-create frames for benchmarks (created once at setup)
std::unique_ptr<IOBuf> gRequestResponseSmall;
std::unique_ptr<IOBuf> gRequestResponseMedium;
std::unique_ptr<IOBuf> gRequestResponseLarge;
std::unique_ptr<IOBuf> gPayloadSmall;
std::unique_ptr<IOBuf> gPayloadMedium;
std::unique_ptr<IOBuf> gPayloadLarge;
std::unique_ptr<IOBuf> gRequestStreamSmall;
std::unique_ptr<IOBuf> gRequestN;

void setupBenchmarks() {
  gRequestResponseSmall = createRequestResponseFrame(1, kSmallPayloadSize);
  gRequestResponseMedium = createRequestResponseFrame(2, kMediumPayloadSize);
  gRequestResponseLarge = createRequestResponseFrame(3, kLargePayloadSize);

  gPayloadSmall = createPayloadFrame(4, kSmallPayloadSize);
  gPayloadMedium = createPayloadFrame(5, kMediumPayloadSize);
  gPayloadLarge = createPayloadFrame(6, kLargePayloadSize);

  gRequestStreamSmall = createRequestStreamFrame(7, kSmallPayloadSize, 100);
  gRequestN = createRequestNFrame(8, 42);
}

} // namespace

// ============================================================================
// REQUEST_RESPONSE Frame Parsing Benchmarks
// ============================================================================

BENCHMARK(Parse_RequestResponse_Rocket_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseSmall->clone();
    RequestResponseFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
  }
}

BENCHMARK_RELATIVE(Parse_RequestResponse_FastThrift_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseSmall->clone();
    auto parsed = parseFrame(std::move(copy));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Parse_RequestResponse_Rocket_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseMedium->clone();
    RequestResponseFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
  }
}

BENCHMARK_RELATIVE(Parse_RequestResponse_FastThrift_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseMedium->clone();
    auto parsed = parseFrame(std::move(copy));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Parse_RequestResponse_Rocket_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseLarge->clone();
    RequestResponseFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
  }
}

BENCHMARK_RELATIVE(Parse_RequestResponse_FastThrift_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseLarge->clone();
    auto parsed = parseFrame(std::move(copy));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// PAYLOAD Frame Parsing Benchmarks
// ============================================================================

BENCHMARK(Parse_Payload_Rocket_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gPayloadSmall->clone();
    PayloadFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
  }
}

BENCHMARK_RELATIVE(Parse_Payload_FastThrift_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gPayloadSmall->clone();
    auto parsed = parseFrame(std::move(copy));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Parse_Payload_Rocket_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gPayloadMedium->clone();
    PayloadFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
  }
}

BENCHMARK_RELATIVE(Parse_Payload_FastThrift_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gPayloadMedium->clone();
    auto parsed = parseFrame(std::move(copy));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Parse_Payload_Rocket_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gPayloadLarge->clone();
    PayloadFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
  }
}

BENCHMARK_RELATIVE(Parse_Payload_FastThrift_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gPayloadLarge->clone();
    auto parsed = parseFrame(std::move(copy));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// REQUEST_STREAM Frame Parsing (with initialRequestN field access)
// ============================================================================

BENCHMARK(Parse_RequestStream_Rocket, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestStreamSmall->clone();
    RequestStreamFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
    doNotOptimizeAway(frame.initialRequestN());
  }
}

BENCHMARK_RELATIVE(Parse_RequestStream_FastThrift, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestStreamSmall->clone();
    auto parsed = parseFrame(std::move(copy));
    RequestStreamView view(parsed);
    doNotOptimizeAway(parsed.streamId());
    doNotOptimizeAway(view.initialRequestN());
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// REQUEST_N Frame Parsing (header-only, no payload)
// ============================================================================

BENCHMARK(Parse_RequestN_Rocket, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestN->clone();
    RequestNFrame frame(std::move(copy));
    doNotOptimizeAway(frame.streamId());
    doNotOptimizeAway(frame.requestN());
  }
}

BENCHMARK_RELATIVE(Parse_RequestN_FastThrift, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestN->clone();
    auto parsed = parseFrame(std::move(copy));
    RequestNView view(parsed);
    doNotOptimizeAway(parsed.streamId());
    doNotOptimizeAway(view.requestN());
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Multiple Field Access (shows benefit of cached parsing)
// ============================================================================

BENCHMARK(MultiAccess_Rocket_5x, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseSmall->clone();
    RequestResponseFrame frame(std::move(copy));
    // Access fields multiple times (already cached by Rocket)
    for (int j = 0; j < 5; ++j) {
      doNotOptimizeAway(frame.streamId());
      doNotOptimizeAway(frame.hasFollows());
    }
  }
}

BENCHMARK_RELATIVE(MultiAccess_FastThrift_5x, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto copy = gRequestResponseSmall->clone();
    auto parsed = parseFrame(std::move(copy));
    // Access fields multiple times (cached in FrameMetadata)
    for (int j = 0; j < 5; ++j) {
      doNotOptimizeAway(parsed.streamId());
      doNotOptimizeAway(parsed.hasFollows());
    }
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  setupBenchmarks();
  runBenchmarks();
  return 0;
}
