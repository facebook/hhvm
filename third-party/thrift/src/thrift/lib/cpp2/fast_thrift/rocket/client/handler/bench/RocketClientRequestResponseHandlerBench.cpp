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
 * RocketClientRequestResponseHandler microbenchmarks.
 *
 * Inbound-only, stateless handler. The interesting paths are:
 *   - Hot:    PAYLOAD with NEXT — common single-shot RR response. Just a
 *             flag check + fireRead.
 *   - Filter: streamType != REQUEST_RESPONSE — early return before any
 *             frame inspection.
 *   - Cold:   protocol violation (e.g. unexpected REQUEST_N) — full
 *             serialize(ErrorHeader) + parseFrame to synthesize ERROR.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>

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

RocketResponseMessage createPayloadResponse(
    uint32_t streamId, FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto frame = serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      folly::IOBuf::copyBuffer("data"));
  return RocketResponseMessage{
      .frame = parseFrame(std::move(frame)),
      .streamType = streamType,
  };
}

RocketResponseMessage makeRequestNResponse(uint32_t streamId) {
  auto buf = serialize(RequestNHeader{.streamId = streamId, .requestN = 1});
  return RocketResponseMessage{
      .frame = parseFrame(std::move(buf)),
      .streamType = FrameType::REQUEST_RESPONSE,
  };
}

// Hot path: PAYLOAD with NEXT on an RR stream — flag check + fireRead.
BENCHMARK(Read_PayloadWithNext_HotPath, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        createPayloadResponse(static_cast<uint32_t>(i * 2 + 1)));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

// Filter path: non-RR streamType — early return before any RR-specific work.
BENCHMARK(Read_NonRRStream_Passthrough, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(createPayloadResponse(
        static_cast<uint32_t>(i * 2 + 1), FrameType::REQUEST_STREAM));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

// Cold path: protocol violation — synthesizeStreamError serializes a fresh
// ERROR frame and re-parses it. Bounds the worst-case per-frame cost.
BENCHMARK(Read_ProtocolViolation_SynthesizeError, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makeRequestNResponse(static_cast<uint32_t>(i * 2 + 1)));
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
