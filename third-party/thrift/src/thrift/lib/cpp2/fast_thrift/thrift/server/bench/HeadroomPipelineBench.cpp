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
 * Headroom Pipeline Integration Benchmark
 *
 * Measures the end-to-end benefit of headroom pre-allocation in
 * ThriftServerChannel through to frame serialization in
 * RocketServerRequestResponseFrameHandler.
 *
 * Compares two paths:
 * - NoHeadroom: metadata serialized without headroom -> FrameWriter slow path
 *   (allocates separate IOBuf for frame header)
 * - WithHeadroom: metadata serialized with headroom -> FrameWriter fast path
 *   (writes header into headroom, zero alloc)
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

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
// Helpers
// =============================================================================

SimpleBufferAllocator g_allocator;

BytesPtr copyBuffer(folly::StringPiece s) {
  auto buf = g_allocator.allocate(s.size());
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

// Serialize metadata WITHOUT headroom (old behavior, forces FrameWriter slow
// path).
std::unique_ptr<folly::IOBuf> serializeMetadataNoHeadroom(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

// Serialize metadata WITH headroom (new behavior, enables FrameWriter fast
// path). Matches the serializeResponseMetadata helper in ThriftServerChannel.
constexpr size_t kMetadataHeadroomBytes = 16;

std::unique_ptr<folly::IOBuf> serializeMetadataWithHeadroom(
    const apache::thrift::ResponseRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  auto serializedSize = metadata.serializedSizeZC(&writer);
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  auto buf = folly::IOBuf::create(kMetadataHeadroomBytes + serializedSize);
  buf->advance(kMetadataHeadroomBytes);
  queue.append(std::move(buf));
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

ParsedFrame makeRequestResponseFrame(uint32_t streamId) {
  auto buf = serialize(
      RequestResponseHeader{.streamId = streamId}, nullptr, copyBuffer("req"));
  return parseFrame(std::move(buf));
}

apache::thrift::ResponseRpcMetadata createMinimalResponseMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});
  return metadata;
}

apache::thrift::ResponseRpcMetadata createTypicalResponseMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});
  auto& headers = metadata.otherMetadata().ensure();
  headers["server_id"] = "server123";
  headers["latency_ms"] = "42";
  return metadata;
}

apache::thrift::ResponseRpcMetadata createLargeResponseMetadata() {
  apache::thrift::ResponseRpcMetadata metadata;
  metadata.payloadMetadata().ensure().set_responseMetadata(
      apache::thrift::PayloadResponseMetadata{});
  auto& headers = metadata.otherMetadata().ensure();
  for (int i = 0; i < 20; ++i) {
    headers[fmt::format("header_{}", i)] = fmt::format("value_{}", i);
  }
  return metadata;
}

// =============================================================================
// Contexts
// =============================================================================

// Captures the final serialized frame IOBuf from
// RocketServerRequestResponseFrameHandler.
class FrameSinkContext {
 public:
  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  Result fireRead(TypeErasedBox&& msg) noexcept {
    lastMsg_ = std::move(msg);
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    lastMsg_ = std::move(msg);
    return Result::Success;
  }

  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  Result fireException(folly::exception_wrapper&& /*e*/) noexcept {
    return Result::Success;
  }

  void deactivate() noexcept {}

 private:
  TypeErasedBox lastMsg_;
};

// =============================================================================
// Pipeline Benchmarks: NoHeadroom vs WithHeadroom
//
// Both paths serialize metadata in the hot loop so the comparison is fair.
//
// NoHeadroom: serialize metadata (no headroom) -> frame handler
//   -> apache::thrift::fast_thrift::frame::write::serialize() slow path
//   (allocates separate header IOBuf)
//
// WithHeadroom: serialize metadata (with headroom) -> frame handler
//   -> apache::thrift::fast_thrift::frame::write::serialize() fast path (writes
//   header into headroom, zero alloc)
// =============================================================================

struct PreparedResponse {
  std::unique_ptr<folly::IOBuf> payload;
  apache::thrift::ResponseRpcMetadata metadata;
  uint32_t streamId;
};

void benchNoHeadroom(
    size_t iters, const apache::thrift::ResponseRpcMetadata& metadataTemplate) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler frameHandler;
  FrameSinkContext frameCtx;

  // Register streams
  for (size_t i = 0; i < iters; ++i) {
    auto frame = makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1));
    std::ignore =
        frameHandler.onRead(frameCtx, erase_and_box(std::move(frame)));
  }

  // Pre-build payloads and metadata copies, but do NOT serialize metadata yet.
  // Metadata serialization happens in the timed section below.
  std::vector<PreparedResponse> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        PreparedResponse{
            .payload = copyBuffer("response data"),
            .metadata = metadataTemplate,
            .streamId = static_cast<uint32_t>(2 * i + 1)});
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    // Serialize metadata without headroom (in timed section)
    auto serializedMeta = serializeMetadataNoHeadroom(responses[i].metadata);
    auto streamResp = RocketResponseMessage{
        .payload = std::move(responses[i].payload),
        .metadata = std::move(serializedMeta),
        .streamId = responses[i].streamId,
        .errorCode = 0};
    auto result =
        frameHandler.onWrite(frameCtx, erase_and_box(std::move(streamResp)));
    doNotOptimizeAway(result);
  }
}

void benchWithHeadroom(
    size_t iters, const apache::thrift::ResponseRpcMetadata& metadataTemplate) {
  BenchmarkSuspender suspender;
  RocketServerRequestResponseFrameHandler frameHandler;
  FrameSinkContext frameCtx;

  // Register streams in the frame handler
  for (size_t i = 0; i < iters; ++i) {
    auto frame = makeRequestResponseFrame(static_cast<uint32_t>(2 * i + 1));
    std::ignore =
        frameHandler.onRead(frameCtx, erase_and_box(std::move(frame)));
  }

  // Pre-build payloads and metadata copies, but do NOT serialize metadata yet.
  // Metadata serialization happens in the timed section below, matching
  // the NoHeadroom path.
  std::vector<PreparedResponse> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        PreparedResponse{
            .payload = copyBuffer("response data"),
            .metadata = metadataTemplate,
            .streamId = static_cast<uint32_t>(2 * i + 1)});
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    // Serialize metadata with headroom (in timed section)
    auto serializedMeta = serializeMetadataWithHeadroom(responses[i].metadata);

    // Build RocketResponseMessage directly (no handler intermediary)
    auto streamResp = RocketResponseMessage{
        .payload = std::move(responses[i].payload),
        .metadata = std::move(serializedMeta),
        .streamId = responses[i].streamId,
        .errorCode = 0};

    // RocketServerRequestResponseFrameHandler serializes the frame
    auto result =
        frameHandler.onWrite(frameCtx, erase_and_box(std::move(streamResp)));
    doNotOptimizeAway(result);
  }
}

// =============================================================================
// Minimal metadata
// =============================================================================

BENCHMARK(Pipeline_NoHeadroom_Minimal, iters) {
  benchNoHeadroom(iters, createMinimalResponseMetadata());
}

BENCHMARK_RELATIVE(Pipeline_WithHeadroom_Minimal, iters) {
  benchWithHeadroom(iters, createMinimalResponseMetadata());
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Typical metadata (2 headers)
// =============================================================================

BENCHMARK(Pipeline_NoHeadroom_Typical, iters) {
  benchNoHeadroom(iters, createTypicalResponseMetadata());
}

BENCHMARK_RELATIVE(Pipeline_WithHeadroom_Typical, iters) {
  benchWithHeadroom(iters, createTypicalResponseMetadata());
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Large metadata (20 headers)
// =============================================================================

BENCHMARK(Pipeline_NoHeadroom_ManyHeaders, iters) {
  benchNoHeadroom(iters, createLargeResponseMetadata());
}

BENCHMARK_RELATIVE(Pipeline_WithHeadroom_ManyHeaders, iters) {
  benchWithHeadroom(iters, createLargeResponseMetadata());
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
