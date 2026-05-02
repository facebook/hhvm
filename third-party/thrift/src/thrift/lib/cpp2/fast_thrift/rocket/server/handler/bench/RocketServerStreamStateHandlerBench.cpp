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
 * RocketServerStreamStateHandler Microbenchmarks
 *
 * Measures the overhead of server-side stream state management:
 * - Inbound: New stream registration (REQUEST_RESPONSE)
 * - Inbound: Terminal event handling (CANCEL for active stream)
 * - Inbound: Connection-level frame passthrough (streamId=0)
 * - Outbound: Response routing with complete (stream cleanup)
 * - Outbound: Response routing without complete (partial response)
 * - Inbound: New stream registration under map pressure (many active streams)
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::rocket::server;
using namespace apache::thrift::fast_thrift::rocket::server::handler;

namespace {

// =============================================================================
// Allocator & Helpers
// =============================================================================

SimpleBufferAllocator g_allocator;

BytesPtr benchAllocate(size_t size) {
  return g_allocator.allocate(size);
}

BytesPtr copyBuffer(folly::StringPiece s) {
  auto buf = g_allocator.allocate(s.size());
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

// =============================================================================
// Minimal Benchmark Context
// =============================================================================

using apache::thrift::fast_thrift::rocket::bench::BenchContext;

// =============================================================================
// Frame Builders
// =============================================================================

ParsedFrame makeRequestResponseFrame(uint32_t streamId) {
  auto buf = serialize(
      RequestResponseHeader{.streamId = streamId},
      nullptr,
      copyBuffer("payload"));
  return parseFrame(std::move(buf));
}

// Build a raw frame for types without a dedicated serialize overload (e.g.,
// CANCEL).
std::unique_ptr<folly::IOBuf> buildRawFrame(
    FrameType type, uint32_t streamId, uint16_t flags = 0) {
  const auto& desc = getDescriptor(type);
  size_t headerSize = desc.headerSize > 0 ? desc.headerSize : kBaseHeaderSize;

  auto buf = benchAllocate(headerSize);
  auto* data = buf->writableData();
  std::memset(data, 0, headerSize);

  data[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  data[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  data[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  data[3] = static_cast<uint8_t>(streamId & 0xFF);

  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type)
       << ::apache::thrift::fast_thrift::frame::detail::kFlagsBits) |
      flags;
  data[4] = static_cast<uint8_t>((typeAndFlags >> 8) & 0xFF);
  data[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  buf->append(headerSize);
  return buf;
}

ParsedFrame makeCancelFrame(uint32_t streamId) {
  return parseFrame(buildRawFrame(FrameType::CANCEL, streamId));
}

ParsedFrame makeSetupFrame() {
  SetupHeader header{
      .majorVersion = 1,
      .minorVersion = 0,
      .keepaliveTime = 30000,
      .maxLifetime = 60000,
      .lease = false,
  };
  auto buf = serialize(header, nullptr, nullptr);
  return parseFrame(std::move(buf));
}

// =============================================================================
// Read Path Benchmarks
// =============================================================================

BENCHMARK(Read_StreamState_NewStream, iters) {
  BenchmarkSuspender suspender;

  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(
        makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1)));
  }

  suspender.dismiss();

  RocketServerStreamStateHandler handler;
  BenchContext ctx;

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_StreamState_TerminalCancel, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamStateHandler handler;
  BenchContext ctx;

  // Register streams
  for (size_t i = 0; i < iters; ++i) {
    auto frame = makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1));
    std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));
  }

  // Build cancel frames
  std::vector<ParsedFrame> cancelFrames;
  cancelFrames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    cancelFrames.push_back(makeCancelFrame(static_cast<uint32_t>(2 * i + 1)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result =
        handler.onRead(ctx, erase_and_box(std::move(cancelFrames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_StreamState_ConnectionFrame, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamStateHandler handler;
  BenchContext ctx;

  // Connection-level frames have streamId=0 (e.g., SETUP)
  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(makeSetupFrame());
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Write Path Benchmarks
// =============================================================================

BENCHMARK(Write_StreamState_CompleteResponse, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamStateHandler handler;
  BenchContext ctx;

  // Register streams
  for (size_t i = 0; i < iters; ++i) {
    auto frame = makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1));
    std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));
  }

  // Build responses
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
                    .data = copyBuffer("response data"),
                    .header =
                        {.streamId = static_cast<uint32_t>(2 * i + 1),
                         .complete = true,
                         .next = true},
                },
        });
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(responses[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Write_StreamState_PartialResponse, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamStateHandler handler;
  BenchContext ctx;

  // Register a single stream for partial responses
  auto frame = makeRequestResponseFrame(1);
  std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));

  // Build partial responses (all to the same stream)
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
                    .data = copyBuffer("partial data"),
                    .header = {.streamId = 1, .complete = false, .next = true},
                },
        });
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(responses[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Map Pressure Benchmarks
// =============================================================================

BENCHMARK(Read_StreamState_NewStream_ManyActive, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamStateHandler handler;
  BenchContext ctx;

  // Pre-populate with 1000 active streams
  for (uint32_t i = 0; i < 1000; ++i) {
    auto frame = makeRequestResponseFrame(2 * i + 1);
    std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));
  }

  // Build new stream frames with IDs above the pre-populated range
  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(
        makeRequestResponseFrame(static_cast<uint32_t>(2 * (1000 + i) + 1)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
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
