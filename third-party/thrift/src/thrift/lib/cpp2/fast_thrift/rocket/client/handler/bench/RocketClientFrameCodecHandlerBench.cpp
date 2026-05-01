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
 * RocketClientFrameCodecHandler Microbenchmarks
 *
 * This handler does:
 * - onWrite: Visits the RocketRequestMessage::frame variant and serializes
 *   the held Composed*Frame alternative into wire bytes.
 * - onRead: Parses raw IOBuf into ParsedFrame using parseFrame()
 *
 * Meaningful benchmarks:
 * - Read path: Frame parsing overhead for different frame types
 * - Write path: Payload serialization overhead per frame type
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::rocket;
using namespace apache::thrift::fast_thrift::rocket::client::handler;

namespace {

using rocket::bench::BenchContext;

// =============================================================================
// Helper Functions - Create different frame types for parsing benchmarks
// =============================================================================

std::unique_ptr<folly::IOBuf> createPayloadFrame(uint32_t streamId) {
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      folly::IOBuf::copyBuffer("data"));
}

std::unique_ptr<folly::IOBuf> createPayloadFrameWithMetadata(
    uint32_t streamId) {
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      folly::IOBuf::copyBuffer("metadata"),
      folly::IOBuf::copyBuffer("data"));
}

std::unique_ptr<folly::IOBuf> createErrorFrame(uint32_t streamId) {
  return serialize(
      ErrorHeader{.streamId = streamId, .errorCode = 0x00000201},
      nullptr,
      folly::IOBuf::copyBuffer("error"));
}

RocketRequestMessage createRocketRequestResponseRequest(uint32_t streamId) {
  return RocketRequestMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = folly::IOBuf::copyBuffer("data"),
              .header = {.streamId = streamId},
          },
  };
}

// =============================================================================
// Read Path Benchmarks - Frame parsing is the actual work
// =============================================================================

BENCHMARK(Read_FrameCodec_PayloadFrame, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createPayloadFrame(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_PayloadFrameWithMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createPayloadFrameWithMetadata(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_FrameCodec_ErrorFrame, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<std::unique_ptr<folly::IOBuf>> frames;
  frames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    frames.push_back(createErrorFrame(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(frames[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Write Path Benchmarks - Codec serializes the held Composed*Frame
// =============================================================================

BENCHMARK(Write_FrameCodec_SerializeRequestResponse, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientFrameCodecHandler handler;
  BenchContext ctx;

  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRocketRequestResponseRequest(1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
