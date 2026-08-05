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
 * RocketClientStreamHandler microbenchmarks.
 *
 * Duplex handler: the outbound (write) path observes REQUEST_STREAM frames
 * to register streamIds; the inbound (read) path filters responses by that
 * registry and validates frame ordering. The interesting paths are:
 *   - Write:  REQUEST_STREAM registration — emplace into DirectStreamSet.
 *   - Hot:    Inbound PAYLOAD with NEXT on a registered stream — map lookup
 *             + flag check + fireRead.
 *   - Close:  Inbound terminal PAYLOAD (NEXT+COMPLETE) — lookup + erase +
 *             fireRead.
 *   - Filter: Inbound response for an unknown streamId — map miss + early
 *             pass-through.
 *   - Cold:   Protocol violation (malformed PAYLOAD, no NEXT/COMPLETE) —
 *             map erase + serialize(ErrorHeader) + parseFrame to synthesize
 *             ERROR.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::rocket;
using namespace apache::thrift::fast_thrift::rocket::client;
using namespace apache::thrift::fast_thrift::rocket::client::handler;

namespace {

using rocket::bench::BenchContext;

// =============================================================================
// Helper Functions
// =============================================================================

RocketRequestMessage makeRequestStreamRequest(uint32_t streamId) {
  return RocketRequestMessage{
      .frame =
          ComposedFrame{
              .frameType = FrameType::REQUEST_STREAM,
              .streamId = streamId,
              .initialRequestN = 10,
          },
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makePayloadResponse(
    uint32_t streamId, bool next = true, bool complete = false) {
  auto buf = serialize(
      PayloadHeader{.streamId = streamId, .complete = complete, .next = next},
      nullptr,
      folly::IOBuf::copyBuffer("data"));
  return RocketResponseMessage{
      .payload = parseFrame(std::move(buf)),
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

// =============================================================================
// Benchmarks
// =============================================================================

// Outbound REQUEST_STREAM registration throughput. Each onWrite call checks
// the frame type and emplaces the streamId into the DirectStreamSet.
BENCHMARK(Write_RequestStream_Registration, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientStreamHandler handler;
  BenchContext ctx;
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(
        makeRequestStreamRequest(static_cast<uint32_t>(i * 2 + 1)));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

// Hot path: inbound PAYLOAD with NEXT on a registered stream. The handler
// looks up the streamId, checks NEXT/COMPLETE flags, and forwards via
// fireRead. Stream stays alive (complete=false).
BENCHMARK(Read_Payload_HotPath, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientStreamHandler handler;
  BenchContext ctx;
  for (size_t i = 0; i < iters; ++i) {
    auto req = makeRequestStreamRequest(static_cast<uint32_t>(i * 2 + 1));
    (void)handler.onWrite(ctx, erase_and_box(std::move(req)));
  }
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makePayloadResponse(
        static_cast<uint32_t>(i * 2 + 1), /*next=*/true, /*complete=*/false));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

// Terminal path: inbound PAYLOAD with NEXT+COMPLETE on a registered stream.
// The handler looks up the streamId, erases it from the DirectStreamSet, and
// forwards via fireRead.
BENCHMARK(Read_PayloadComplete_StreamClose, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientStreamHandler handler;
  BenchContext ctx;
  for (size_t i = 0; i < iters; ++i) {
    auto req = makeRequestStreamRequest(static_cast<uint32_t>(i * 2 + 1));
    (void)handler.onWrite(ctx, erase_and_box(std::move(req)));
  }
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makePayloadResponse(
        static_cast<uint32_t>(i * 2 + 1), /*next=*/true, /*complete=*/true));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

// Pass-through path: inbound response for an unknown streamId. No streams
// are pre-registered, so the map lookup misses and the handler immediately
// forwards via fireRead.
BENCHMARK(Read_NonStreamFrame_Passthrough, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientStreamHandler handler;
  BenchContext ctx;
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makePayloadResponse(
        static_cast<uint32_t>(i * 2 + 1), /*next=*/true, /*complete=*/false));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

// Cold path: malformed PAYLOAD (next=false, complete=false) on a registered
// stream. The handler detects the protocol violation, erases the stream, and
// synthesizes an ERROR frame via serialize + parseFrame. Bounds the worst-
// case per-frame cost.
BENCHMARK(Read_ProtocolViolation_SynthesizeError, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientStreamHandler handler;
  BenchContext ctx;
  for (size_t i = 0; i < iters; ++i) {
    auto req = makeRequestStreamRequest(static_cast<uint32_t>(i * 2 + 1));
    (void)handler.onWrite(ctx, erase_and_box(std::move(req)));
  }
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makePayloadResponse(
        static_cast<uint32_t>(i * 2 + 1),
        /*next=*/false,
        /*complete=*/false));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
