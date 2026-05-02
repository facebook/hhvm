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
 * RocketClientStreamStateHandler Microbenchmarks
 *
 * This handler does:
 * - onWrite: Generates streamId, inserts into F14FastMap with
 * ClientStreamContext
 * - onRead: Looks up streamId in F14FastMap, extracts state, erases entry
 *
 * Meaningful benchmarks:
 * - Write path: Stream ID generation + map insertion
 * - Read path: Map lookup/erase with varying number of active streams
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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>

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
// Helper Functions
// =============================================================================

RocketRequestMessage createRocketRequest() {
  return RocketRequestMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = folly::IOBuf::copyBuffer("data"),
              .header = {.streamId = kInvalidStreamId},
          },
      .requestHandle = 1,
      .streamType = FrameType::REQUEST_RESPONSE,
  };
}

RocketResponseMessage createPayloadResponse(uint32_t streamId) {
  auto frame = serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      folly::IOBuf::copyBuffer("data"));

  return RocketResponseMessage{
      .payload = parseFrame(std::move(frame)),
  };
}

// =============================================================================
// Write Path Benchmarks - Stream ID generation + F14FastMap insert
// =============================================================================

BENCHMARK(Write_StreamState_StreamIdGenerationAndMapInsert, iters) {
  folly::BenchmarkSuspender suspender;
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRocketRequest());
  }

  suspender.dismiss();

  RocketClientStreamStateHandler handler;
  BenchContext ctx;

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Read Path Benchmarks - Map lookup/erase with different active stream counts
// =============================================================================

BENCHMARK(Read_StreamState_FewActiveStreams_10, iters) {
  folly::BenchmarkSuspender suspender;

  constexpr size_t kActiveStreams = 10;

  std::vector<RocketClientStreamStateHandler> handlers(iters);
  std::vector<BenchContext> contexts(iters);
  std::vector<std::vector<RocketResponseMessage>> responsesList;
  responsesList.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];

    // Pre-populate with active streams
    std::vector<uint32_t> streamIds;
    for (size_t j = 0; j < kActiveStreams; ++j) {
      auto request = createRocketRequest();
      (void)handler.onWrite(ctx, erase_and_box(std::move(request)));
      streamIds.push_back(static_cast<uint32_t>(j * 2 + 1));
    }

    std::vector<RocketResponseMessage> responses;
    responses.reserve(kActiveStreams);
    for (size_t j = 0; j < kActiveStreams; ++j) {
      responses.push_back(createPayloadResponse(streamIds[j]));
    }
    responsesList.push_back(std::move(responses));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];
    auto& responses = responsesList[i];

    for (size_t j = 0; j < kActiveStreams; ++j) {
      auto result = handler.onRead(ctx, erase_and_box(std::move(responses[j])));
      folly::doNotOptimizeAway(result);
    }
  }
}

BENCHMARK(Read_StreamState_ManyActiveStreams_100, iters) {
  folly::BenchmarkSuspender suspender;

  constexpr size_t kActiveStreams = 100;

  std::vector<RocketClientStreamStateHandler> handlers(iters);
  std::vector<BenchContext> contexts(iters);
  std::vector<std::vector<RocketResponseMessage>> responsesList;
  responsesList.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];

    std::vector<uint32_t> streamIds;
    for (size_t j = 0; j < kActiveStreams; ++j) {
      auto request = createRocketRequest();
      (void)handler.onWrite(ctx, erase_and_box(std::move(request)));
      streamIds.push_back(static_cast<uint32_t>(j * 2 + 1));
    }

    std::vector<RocketResponseMessage> responses;
    responses.reserve(kActiveStreams);
    for (size_t j = 0; j < kActiveStreams; ++j) {
      responses.push_back(createPayloadResponse(streamIds[j]));
    }
    responsesList.push_back(std::move(responses));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];
    auto& responses = responsesList[i];

    for (size_t j = 0; j < kActiveStreams; ++j) {
      auto result = handler.onRead(ctx, erase_and_box(std::move(responses[j])));
      folly::doNotOptimizeAway(result);
    }
  }
}

BENCHMARK(Read_StreamState_ManyActiveStreams_1000, iters) {
  folly::BenchmarkSuspender suspender;

  constexpr size_t kActiveStreams = 1000;

  std::vector<RocketClientStreamStateHandler> handlers(iters);
  std::vector<BenchContext> contexts(iters);
  std::vector<std::vector<RocketResponseMessage>> responsesList;
  responsesList.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];

    std::vector<uint32_t> streamIds;
    for (size_t j = 0; j < kActiveStreams; ++j) {
      auto request = createRocketRequest();
      (void)handler.onWrite(ctx, erase_and_box(std::move(request)));
      streamIds.push_back(static_cast<uint32_t>(j * 2 + 1));
    }

    std::vector<RocketResponseMessage> responses;
    responses.reserve(kActiveStreams);
    for (size_t j = 0; j < kActiveStreams; ++j) {
      responses.push_back(createPayloadResponse(streamIds[j]));
    }
    responsesList.push_back(std::move(responses));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];
    auto& responses = responsesList[i];

    for (size_t j = 0; j < kActiveStreams; ++j) {
      auto result = handler.onRead(ctx, erase_and_box(std::move(responses[j])));
      folly::doNotOptimizeAway(result);
    }
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
