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
 * RocketServerRequestResponseFrameHandler Microbenchmarks
 *
 * Measures the overhead of request-response frame handling on the server:
 * - Inbound: REQUEST_RESPONSE stream tracking (F14FastSet insert)
 * - Inbound: Non-REQUEST_RESPONSE passthrough
 * - Outbound: PAYLOAD frame serialization (success response)
 * - Outbound: ERROR frame serialization
 * - Outbound: Non-request-response forward (set miss)
 * - Outbound: Serialization under set pressure (many pending streams)
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>

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

ParsedFrame makeRequestStreamFrame(uint32_t streamId) {
  auto buf = serialize(
      RequestStreamHeader{.streamId = streamId, .initialRequestN = 100},
      nullptr,
      copyBuffer("payload"));
  return parseFrame(std::move(buf));
}

// =============================================================================
// Read Path Benchmarks
// =============================================================================

BENCHMARK(Read_RequestResponse_Track, iters) {
  BenchmarkSuspender suspender;

  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(
        makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1)));
  }

  suspender.dismiss();

  RocketServerRequestResponseFrameHandler handler;
  BenchContext ctx;

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_RequestResponse_NonRR_Passthrough, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler handler;
  BenchContext ctx;

  // Use REQUEST_STREAM frames (not REQUEST_RESPONSE)
  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(makeRequestStreamFrame(static_cast<uint32_t>(2 * i + 1)));
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

BENCHMARK(Write_RequestResponse_PayloadSerialization, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler handler;
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
                    .metadata = copyBuffer("response metadata"),
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

BENCHMARK(Write_RequestResponse_ErrorSerialization, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler handler;
  BenchContext ctx;

  // Register streams
  for (size_t i = 0; i < iters; ++i) {
    auto frame = makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1));
    std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));
  }

  // Build error responses
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                apache::thrift::fast_thrift::frame::ComposedErrorFrame{
                    .data = copyBuffer("error details"),
                    .header =
                        {.streamId = static_cast<uint32_t>(2 * i + 1),
                         .errorCode = 0x00000201},
                },
        }); // APPLICATION_ERROR
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(responses[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Write_RequestResponse_NonRR_Forward, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler handler;
  BenchContext ctx;

  // Build responses for stream IDs NOT in the tracking set
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
                    .data = copyBuffer("data"),
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

BENCHMARK_DRAW_LINE();

// =============================================================================
// Set Pressure Benchmarks
// =============================================================================

BENCHMARK(Write_RequestResponse_PayloadSerialization_ManyPending, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler handler;
  BenchContext ctx;

  // Pre-populate with 1000 pending request-response streams
  for (uint32_t i = 0; i < 1000; ++i) {
    auto frame = makeRequestResponseFrame(2 * i + 1);
    std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));
  }

  // Register additional streams for this benchmark
  for (size_t i = 0; i < iters; ++i) {
    auto frame =
        makeRequestResponseFrame(static_cast<uint32_t>(2 * (1000 + i) + 1));
    std::ignore = handler.onRead(ctx, erase_and_box(std::move(frame)));
  }

  // Build responses for the newly registered streams
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
                    .data = copyBuffer("response data"),
                    .metadata = copyBuffer("response metadata"),
                    .header =
                        {.streamId = static_cast<uint32_t>(2 * (1000 + i) + 1),
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

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
