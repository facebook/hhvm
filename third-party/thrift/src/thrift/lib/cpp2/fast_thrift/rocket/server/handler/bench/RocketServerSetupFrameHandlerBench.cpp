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
 * RocketServerSetupFrameHandler Microbenchmarks
 *
 * Measures the overhead of SETUP frame validation and post-setup passthrough:
 * - Inbound: SETUP frame validation (first-frame path)
 * - Inbound: Post-setup passthrough (hot path after setup complete)
 * - Outbound: Write passthrough
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>

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

ParsedFrame makeSetupFrame(
    uint16_t majorVersion = 1,
    uint16_t minorVersion = 0,
    uint32_t keepaliveTime = 30000,
    uint32_t maxLifetime = 60000,
    bool lease = false) {
  SetupHeader header{
      .majorVersion = majorVersion,
      .minorVersion = minorVersion,
      .keepaliveTime = keepaliveTime,
      .maxLifetime = maxLifetime,
      .lease = lease,
  };
  auto buf = serialize(header, nullptr, nullptr);
  return parseFrame(std::move(buf));
}

ParsedFrame makeRequestResponseFrame(uint32_t streamId) {
  auto buf = serialize(
      RequestResponseHeader{.streamId = streamId},
      nullptr,
      copyBuffer("payload"));
  return parseFrame(std::move(buf));
}

ParsedFrame makePayloadFrame(uint32_t streamId) {
  auto buf = serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      copyBuffer("response"));
  return parseFrame(std::move(buf));
}

// Wrap a ParsedFrame in a RocketRequestMessage (matches how
// RocketServerFrameCodecHandler delivers inbound frames to SetupHandler).
RocketRequestMessage wrapRequest(ParsedFrame frame) {
  RocketRequestMessage msg;
  msg.payload = std::move(frame);
  return msg;
}

// =============================================================================
// Read Path Benchmarks
// =============================================================================

BENCHMARK(Read_Setup_ValidSetupFrame, iters) {
  BenchmarkSuspender suspender;

  std::vector<RocketRequestMessage> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(wrapRequest(makeSetupFrame()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    RocketServerSetupFrameHandler handler;
    BenchContext ctx;
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_Setup_PostSetupPassthrough, iters) {
  BenchmarkSuspender suspender;
  RocketServerSetupFrameHandler handler;
  BenchContext ctx;

  // Complete setup first
  std::ignore =
      handler.onRead(ctx, erase_and_box(wrapRequest(makeSetupFrame())));

  std::vector<RocketRequestMessage> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(wrapRequest(
        makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1))));
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

BENCHMARK(Write_Setup_Passthrough, iters) {
  BenchmarkSuspender suspender;
  RocketServerSetupFrameHandler handler;
  BenchContext ctx;

  std::vector<ParsedFrame> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(makePayloadFrame(static_cast<uint32_t>(2 * i + 1)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(frames[i])));
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
