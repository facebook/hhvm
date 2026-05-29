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
 * RocketClientRequestResponseFrameHandler Microbenchmarks
 *
 * This handler does:
 * - onWrite: Inserts streamId into F14FastSet, serializes frame via serialize()
 * - onRead: Looks up streamId in F14FastSet, validates frame flags, erases
 * entry
 *
 * Meaningful benchmarks:
 * - Write path: Frame serialization overhead (serialize() call)
 * - Read path: Stream tracking lookup/erase with varying number of pending
 * streams
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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>

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

RocketRequestMessage createRocketRequest(uint32_t streamId) {
  return RocketRequestMessage{
      .frame =
          RocketFramePayload{
              .metadata = nullptr,
              .data = folly::IOBuf::copyBuffer("data"),
              .streamId = streamId,
          },
      .frameType = FrameType::REQUEST_RESPONSE,
  };
}

RocketRequestMessage createRocketRequestWithMetadata(uint32_t streamId) {
  return RocketRequestMessage{
      .frame =
          RocketFramePayload{
              .metadata = folly::IOBuf::copyBuffer("metadata"),
              .data = folly::IOBuf::copyBuffer("data"),
              .streamId = streamId,
          },
      .frameType = FrameType::REQUEST_RESPONSE,
  };
}

RocketResponseMessage createPayloadResponse(uint32_t streamId) {
  auto frame = serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      folly::IOBuf::copyBuffer("data"));

  return RocketResponseMessage{
      .frame = parseFrame(std::move(frame)),
      .requestFrameType = FrameType::REQUEST_RESPONSE,
  };
}

// =============================================================================
// Write Path Benchmarks - Frame serialization + F14FastSet insert
// =============================================================================

BENCHMARK(Write_RequestResponse_Serialization, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientRequestResponseFrameHandler handler;
  BenchContext ctx;

  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRocketRequest(i * 2 + 1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Write_RequestResponse_WithMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  RocketClientRequestResponseFrameHandler handler;
  BenchContext ctx;

  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRocketRequestWithMetadata(i * 2 + 1));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(requests[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Read Path Benchmarks - F14FastSet lookup/erase with different pending counts
// =============================================================================

BENCHMARK(Read_RequestResponse_FewPendingStreams_10, iters) {
  folly::BenchmarkSuspender suspender;

  constexpr size_t kPendingStreams = 10;

  std::vector<RocketClientRequestResponseFrameHandler> handlers(iters);
  std::vector<BenchContext> contexts(iters);
  std::vector<std::vector<RocketResponseMessage>> responsesList;
  responsesList.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];

    // Pre-populate with pending requests
    for (size_t j = 0; j < kPendingStreams; ++j) {
      auto request = createRocketRequest(static_cast<uint32_t>(j * 2 + 1));
      (void)handler.onWrite(ctx, erase_and_box(std::move(request)));
    }

    std::vector<RocketResponseMessage> responses;
    responses.reserve(kPendingStreams);
    for (size_t j = 0; j < kPendingStreams; ++j) {
      responses.push_back(
          createPayloadResponse(static_cast<uint32_t>(j * 2 + 1)));
    }
    responsesList.push_back(std::move(responses));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];
    auto& responses = responsesList[i];

    for (size_t j = 0; j < kPendingStreams; ++j) {
      auto result = handler.onRead(ctx, erase_and_box(std::move(responses[j])));
      folly::doNotOptimizeAway(result);
    }
  }
}

BENCHMARK(Read_RequestResponse_ManyPendingStreams_100, iters) {
  folly::BenchmarkSuspender suspender;

  constexpr size_t kPendingStreams = 100;

  std::vector<RocketClientRequestResponseFrameHandler> handlers(iters);
  std::vector<BenchContext> contexts(iters);
  std::vector<std::vector<RocketResponseMessage>> responsesList;
  responsesList.reserve(iters);

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];

    for (size_t j = 0; j < kPendingStreams; ++j) {
      auto request = createRocketRequest(static_cast<uint32_t>(j * 2 + 1));
      (void)handler.onWrite(ctx, erase_and_box(std::move(request)));
    }

    std::vector<RocketResponseMessage> responses;
    responses.reserve(kPendingStreams);
    for (size_t j = 0; j < kPendingStreams; ++j) {
      responses.push_back(
          createPayloadResponse(static_cast<uint32_t>(j * 2 + 1)));
    }
    responsesList.push_back(std::move(responses));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto& handler = handlers[i];
    auto& ctx = contexts[i];
    auto& responses = responsesList[i];

    for (size_t j = 0; j < kPendingStreams; ++j) {
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
