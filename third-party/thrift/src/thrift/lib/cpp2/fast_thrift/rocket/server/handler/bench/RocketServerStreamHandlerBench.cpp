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
 * RocketServerStreamHandler microbenchmarks.
 *
 * Stateful duplex handler for REQUEST_STREAM. Interesting paths:
 *   - Read hot:  REQUEST_STREAM registration — parse initialRequestN,
 *                insert into DirectStreamMap, fireRead.
 *   - Read hot:  REQUEST_N credit increment — lookup in DirectStreamMap,
 *                parse requestN, add credits, fireRead.
 *   - Read pass: non-stream frame type — early-out before any stream work.
 *   - Write:     PAYLOAD with credit decrement — lookup, decrement, fireWrite.
 *   - Write:     terminal PAYLOAD (complete) — lookup, erase, fireWrite.
 */

#include <cstring>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamHandler.h>

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

// Extends the shared bench context with the pipeline-level RocketStreamContexts
// the handler reaches through state<T>(). RocketServerStreamHandler only acts
// on a stream whose entry already exists (StreamStateHandler owns that
// lifecycle in the real pipeline); openStream() stands in for it so the
// benchmarks exercise the credit hot paths rather than the unknown-stream
// passthrough.
struct StreamBenchContext : BenchContext {
  template <typename T>
  T& state() noexcept {
    return contexts_;
  }

  void openStream(uint32_t streamId) {
    contexts_.streams.emplace(
        streamId,
        rocket::RocketStreamContext{
            .streamType = FrameType::REQUEST_STREAM, .credits = 0});
  }

  rocket::RocketStreamContexts contexts_;
};

RocketRequestMessage makeRequestStreamRequest(
    uint32_t streamId, uint32_t initialRequestN = 10) {
  auto buf = serialize(
      RequestStreamHeader{
          .streamId = streamId, .initialRequestN = initialRequestN},
      nullptr,
      copyBuffer("payload"));
  return RocketRequestMessage{
      .frame = parseFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketRequestMessage makeRequestN(uint32_t streamId, uint32_t requestN = 1) {
  auto buf =
      serialize(RequestNHeader{.streamId = streamId, .requestN = requestN});
  return RocketRequestMessage{
      .frame = parseFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_STREAM,
  };
}

// Read hot path: REQUEST_STREAM registration — parse initialRequestN,
// insert stream entry, fireRead.
BENCHMARK(Read_RequestStream_HotPath, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamHandler handler;
  StreamBenchContext ctx;
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    ctx.openStream(static_cast<uint32_t>(2 * i + 1));
    requests.push_back(
        makeRequestStreamRequest(static_cast<uint32_t>(2 * i + 1)));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(requests[i])));
    doNotOptimizeAway(result);
  }
}

// Read hot path: REQUEST_N credit increment on active streams.
BENCHMARK(Read_RequestN_CreditIncrement, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamHandler handler;
  StreamBenchContext ctx;
  // Pre-register streams
  for (size_t i = 0; i < iters; ++i) {
    ctx.openStream(static_cast<uint32_t>(2 * i + 1));
    auto req = makeRequestStreamRequest(static_cast<uint32_t>(2 * i + 1));
    (void)handler.onRead(ctx, erase_and_box(std::move(req)));
  }
  // Prepare REQUEST_N messages
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(makeRequestN(static_cast<uint32_t>(2 * i + 1)));
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(requests[i])));
    doNotOptimizeAway(result);
  }
}

// Read pass-through: non-stream frame type — early-out before any
// stream-specific work.
BENCHMARK(Read_NonStreamFrame_Passthrough, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamHandler handler;
  StreamBenchContext ctx;
  // No entries opened: the unknown-stream lookup misses and the frame
  // early-outs before any stream-specific work.
  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    // Create a REQUEST_RESPONSE frame — will pass through the stream handler
    auto buf = serialize(
        RequestResponseHeader{.streamId = static_cast<uint32_t>(2 * i + 1)},
        nullptr,
        copyBuffer("payload"));
    requests.push_back(
        RocketRequestMessage{
            .frame = parseFrame(std::move(buf)),
            .streamId = static_cast<uint32_t>(2 * i + 1),
            .streamType = FrameType::REQUEST_RESPONSE,
        });
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(requests[i])));
    doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// Write: PAYLOAD with credit decrement on active streams.
BENCHMARK(Write_Payload_CreditDecrement, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamHandler handler;
  StreamBenchContext ctx;
  // Register streams with enough credits
  for (size_t i = 0; i < iters; ++i) {
    ctx.openStream(static_cast<uint32_t>(2 * i + 1));
    auto req = makeRequestStreamRequest(
        static_cast<uint32_t>(2 * i + 1), static_cast<uint32_t>(iters));
    (void)handler.onRead(ctx, erase_and_box(std::move(req)));
  }
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                ComposedFrame{
                    .frameType = FrameType::PAYLOAD,
                    .streamId = static_cast<uint32_t>(2 * i + 1),
                    .data = copyBuffer("response"),
                    .next = true,
                },
            .streamType = FrameType::REQUEST_STREAM,
        });
  }
  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onWrite(ctx, erase_and_box(std::move(responses[i])));
    doNotOptimizeAway(result);
  }
}

// Write: terminal PAYLOAD (next+complete) — erase stream entry on close.
BENCHMARK(Write_PayloadComplete_StreamClose, iters) {
  BenchmarkSuspender suspender;
  RocketServerStreamHandler handler;
  StreamBenchContext ctx;
  for (size_t i = 0; i < iters; ++i) {
    ctx.openStream(static_cast<uint32_t>(2 * i + 1));
    auto req = makeRequestStreamRequest(static_cast<uint32_t>(2 * i + 1));
    (void)handler.onRead(ctx, erase_and_box(std::move(req)));
  }
  std::vector<RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        RocketResponseMessage{
            .frame =
                ComposedFrame{
                    .frameType = FrameType::PAYLOAD,
                    .streamId = static_cast<uint32_t>(2 * i + 1),
                    .data = copyBuffer("response"),
                    .complete = true,
                    .next = true,
                },
            .streamType = FrameType::REQUEST_STREAM,
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
