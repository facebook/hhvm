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
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>

using namespace folly;
using namespace apache::thrift::rocket;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;

namespace {

constexpr size_t kSmallPayloadSize = 100;
constexpr size_t kMediumPayloadSize = 1024;
constexpr size_t kLargePayloadSize = 64 * 1024;

std::string makePayloadData(size_t size) {
  std::string data(size, 'x');
  return data;
}

std::string gSmallData;
std::string gMediumData;
std::string gLargeData;
std::string gMetadata = "test-metadata";

constexpr size_t kHeadroomBytes = 16;

// Create a metadata IOBuf with headroom reserved for frame header.
// This enables the zero-alloc fast path in serializeFrame().
std::unique_ptr<IOBuf> copyBufferWithHeadroom(folly::StringPiece s) {
  auto buf = IOBuf::create(kHeadroomBytes + s.size());
  buf->advance(kHeadroomBytes);
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

void setupBenchmarks() {
  gSmallData = makePayloadData(kSmallPayloadSize);
  gMediumData = makePayloadData(kMediumPayloadSize);
  gLargeData = makePayloadData(kLargePayloadSize);
}

} // namespace

// ============================================================================
// REQUEST_RESPONSE Frame Serialization Benchmarks
// ============================================================================

BENCHMARK(Serialize_RequestResponse_Rocket_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    RequestResponseFrame frame(StreamId{1}, std::move(payload));
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_RequestResponse_FastThrift_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 1},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Serialize_RequestResponse_Rocket_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gMediumData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    RequestResponseFrame frame(StreamId{1}, std::move(payload));
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_RequestResponse_FastThrift_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gMediumData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 1},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Serialize_RequestResponse_Rocket_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    RequestResponseFrame frame(StreamId{1}, std::move(payload));
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_RequestResponse_FastThrift_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 1},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// PAYLOAD Frame Serialization Benchmarks
// ============================================================================

BENCHMARK(Serialize_Payload_Rocket_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    PayloadFrame frame(
        StreamId{1},
        std::move(payload),
        apache::thrift::rocket::Flags().complete(true));
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Payload_FastThrift_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Serialize_Payload_Rocket_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gMediumData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    PayloadFrame frame(
        StreamId{1},
        std::move(payload),
        apache::thrift::rocket::Flags().complete(true));
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Payload_FastThrift_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gMediumData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Serialize_Payload_Rocket_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    PayloadFrame frame(
        StreamId{1},
        std::move(payload),
        apache::thrift::rocket::Flags().complete(true));
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Payload_FastThrift_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Headroom Impact: Fast Path vs Slow Path in serializeFrame()
//
// When metadata has >= 9 bytes headroom, serializeFrame() writes the frame
// header directly into the headroom (zero allocation). Without headroom, it
// allocates a separate IOBuf for the header.
// ============================================================================

BENCHMARK(Serialize_Payload_NoHeadroom_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Payload_WithHeadroom_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = copyBufferWithHeadroom(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Serialize_Payload_NoHeadroom_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gMediumData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Payload_WithHeadroom_Medium, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gMediumData);
    auto metadata = copyBufferWithHeadroom(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(Serialize_Payload_NoHeadroom_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Payload_WithHeadroom_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = copyBufferWithHeadroom(gMetadata);
    auto serialized = serialize(
        PayloadHeader{.streamId = 1, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// REQUEST_STREAM Frame Serialization Benchmarks (with initialRequestN)
// ============================================================================

BENCHMARK(Serialize_RequestStream_Rocket, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    Payload payload =
        Payload::makeFromMetadataAndData(std::move(metadata), std::move(data));
    RequestStreamFrame frame(StreamId{1}, std::move(payload), 100);
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_RequestStream_FastThrift, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestStreamHeader{.streamId = 1, .initialRequestN = 100},
        std::move(metadata),
        std::move(data));
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// REQUEST_N Frame Serialization (header-only, no payload)
// ============================================================================

BENCHMARK(Serialize_RequestN_Rocket, iters) {
  for (size_t i = 0; i < iters; ++i) {
    RequestNFrame frame(StreamId{1}, 42);
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_RequestN_FastThrift, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto serialized = serialize(RequestNHeader{.streamId = 1, .requestN = 42});
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// CANCEL Frame Serialization (header-only, no payload)
// ============================================================================

BENCHMARK(Serialize_Cancel_Rocket, iters) {
  for (size_t i = 0; i < iters; ++i) {
    CancelFrame frame(StreamId{1});
    auto serialized = std::move(frame).serialize();
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_RELATIVE(Serialize_Cancel_FastThrift, iters) {
  for (size_t i = 0; i < iters; ++i) {
    auto serialized = serialize(CancelHeader{.streamId = 1});
    doNotOptimizeAway(serialized);
  }
}

BENCHMARK_DRAW_LINE();

// ============================================================================
// Round Trip: Serialize -> Parse (end-to-end comparison)
// Note: Using FastThrift serialize for both since Rocket's serialize includes
// a frame length prefix that Rocket's constructors don't expect (the Parser
// layer strips it in production).
// ============================================================================

BENCHMARK(RoundTrip_RequestResponse_Rocket_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    // Serialize using FastThrift (compatible with Rocket constructors)
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 42},
        std::move(metadata),
        std::move(data));

    // Parse with Rocket
    RequestResponseFrame inFrame(std::move(serialized));
    doNotOptimizeAway(inFrame.streamId());
  }
}

BENCHMARK_RELATIVE(RoundTrip_RequestResponse_FastThrift_Small, iters) {
  for (size_t i = 0; i < iters; ++i) {
    // Serialize
    auto data = IOBuf::copyBuffer(gSmallData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 42},
        std::move(metadata),
        std::move(data));

    // Parse
    auto parsed = parseFrame(std::move(serialized));
    doNotOptimizeAway(parsed.streamId());
  }
}

BENCHMARK_DRAW_LINE();

BENCHMARK(RoundTrip_RequestResponse_Rocket_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    // Serialize using FastThrift (compatible with Rocket constructors)
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 42},
        std::move(metadata),
        std::move(data));

    // Parse with Rocket
    RequestResponseFrame inFrame(std::move(serialized));
    doNotOptimizeAway(inFrame.streamId());
  }
}

BENCHMARK_RELATIVE(RoundTrip_RequestResponse_FastThrift_Large, iters) {
  for (size_t i = 0; i < iters; ++i) {
    // Serialize
    auto data = IOBuf::copyBuffer(gLargeData);
    auto metadata = IOBuf::copyBuffer(gMetadata);
    auto serialized = serialize(
        RequestResponseHeader{.streamId = 42},
        std::move(metadata),
        std::move(data));

    // Parse
    auto parsed = parseFrame(std::move(serialized));
    doNotOptimizeAway(parsed.streamId());
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  setupBenchmarks();
  runBenchmarks();
  return 0;
}
