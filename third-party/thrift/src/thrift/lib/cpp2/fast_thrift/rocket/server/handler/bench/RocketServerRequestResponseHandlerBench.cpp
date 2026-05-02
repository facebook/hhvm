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
 * RocketServerRequestResponseHandler microbenchmarks.
 *
 * Stateless duplex handler. Interesting paths:
 *   - Read hot:    REQUEST_RESPONSE on RR stream — filter check + fireRead.
 *   - Read filter: non-RR streamType — early-out before frame inspection.
 *   - Read cold:   protocol violation (e.g. unexpected REQUEST_N) — builds
 *                  ComposedErrorFrame and fireWrites it back.
 *   - Write:       PAYLOAD on RR stream — stamp complete=true, next=true.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::rocket::server;
using namespace apache::thrift::fast_thrift::rocket::server::handler;

namespace {

SimpleBufferAllocator g_allocator;

BytesPtr copyBuffer(folly::StringPiece s) {
  auto buf = g_allocator.allocate(s.size());
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

using rocket::bench::BenchContext;

RocketRequestMessage makeRequestResponseRequest(
    uint32_t streamId, FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = serialize(
      RequestResponseHeader{.streamId = streamId},
      nullptr,
      copyBuffer("payload"));
  return RocketRequestMessage{
      .payload = parseFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = streamType,
  };
}

RocketRequestMessage makeRequestNRequest(
    uint32_t streamId, FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = serialize(RequestNHeader{.streamId = streamId, .requestN = 1});
  return RocketRequestMessage{
      .payload = parseFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = streamType,
  };
}

// Read hot path: REQUEST_RESPONSE on RR stream — pure pass-through.
BENCHMARK(Read_RequestResponse_HotPath, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(
        makeRequestResponseRequest(static_cast<uint32_t>(2 * i + 1)));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(requests[i])));
    doNotOptimizeAway(result);
  }
}

// Read filter: non-RR streamType — early return before any RR-specific work.
BENCHMARK(Read_NonRRStream_Passthrough, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(makeRequestResponseRequest(
        static_cast<uint32_t>(2 * i + 1), FrameType::REQUEST_STREAM));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(requests[i])));
    doNotOptimizeAway(result);
  }
}

// Read cold path: protocol violation — build ComposedErrorFrame + fireWrite.
BENCHMARK(Read_ProtocolViolation_SendsErrorInvalid, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(makeRequestNRequest(static_cast<uint32_t>(2 * i + 1)));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(requests[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// Write: PAYLOAD on RR stream — variant get_if + flag stamp + fireWrite.
BENCHMARK(Write_PayloadOnRRStream_StampComplete, iters) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseHandler handler;
  BenchContext ctx;
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
                    .data = copyBuffer("response"),
                    .metadata = nullptr,
                    .header = {.streamId = static_cast<uint32_t>(2 * i + 1)},
                },
            .streamType = FrameType::REQUEST_RESPONSE,
        });
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(responses[i])));
    doNotOptimizeAway(result);
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
