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
 * RocketServerFrameCodecHandler Microbenchmarks
 *
 * This handler does:
 * - onRead: Parses raw IOBuf into ParsedFrame using tryParseFrame()
 * - onWrite: Passthrough (no work)
 *
 * Meaningful benchmarks:
 * - Read path: Frame parsing overhead for different frame types the server
 *   receives from clients (REQUEST_RESPONSE, REQUEST_STREAM, PAYLOAD, ERROR,
 *   and with/without metadata variants)
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>

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
// Helper Functions - Create different frame types for parsing benchmarks
// =============================================================================

BytesPtr createRequestResponseFrame(uint32_t streamId) {
  return serialize(
      RequestResponseHeader{.streamId = streamId}, nullptr, copyBuffer("data"));
}

BytesPtr createRequestResponseFrameWithMetadata(uint32_t streamId) {
  return serialize(
      RequestResponseHeader{.streamId = streamId},
      copyBuffer("metadata"),
      copyBuffer("data"));
}

BytesPtr createRequestStreamFrame(uint32_t streamId) {
  return serialize(
      RequestStreamHeader{.streamId = streamId, .initialRequestN = 100},
      nullptr,
      copyBuffer("data"));
}

BytesPtr createPayloadFrame(uint32_t streamId) {
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      copyBuffer("data"));
}

BytesPtr createPayloadFrameWithMetadata(uint32_t streamId) {
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      copyBuffer("metadata"),
      copyBuffer("data"));
}

BytesPtr createErrorFrame(uint32_t streamId) {
  return serialize(
      ErrorHeader{.streamId = streamId, .errorCode = 0x00000201},
      nullptr,
      copyBuffer("error"));
}

// =============================================================================
// Read Path Benchmarks - Frame parsing is the actual work
// =============================================================================

BENCHMARK(Read_FrameCodec_RequestResponseFrame, iters) {
  BenchmarkSuspender suspender;
  RocketServerFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<BytesPtr> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createRequestResponseFrame(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_RequestResponseFrameWithMetadata, iters) {
  BenchmarkSuspender suspender;
  RocketServerFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<BytesPtr> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createRequestResponseFrameWithMetadata(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_RequestStreamFrame, iters) {
  BenchmarkSuspender suspender;
  RocketServerFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<BytesPtr> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createRequestStreamFrame(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_PayloadFrame, iters) {
  BenchmarkSuspender suspender;
  RocketServerFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<BytesPtr> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createPayloadFrame(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_PayloadFrameWithMetadata, iters) {
  BenchmarkSuspender suspender;
  RocketServerFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<BytesPtr> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createPayloadFrameWithMetadata(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_ErrorFrame, iters) {
  BenchmarkSuspender suspender;
  RocketServerFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<BytesPtr> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createErrorFrame(1));
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
