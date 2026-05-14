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
 * ThriftClientMetadataPushHandler Microbenchmarks
 *
 * Measures the overhead of METADATA_PUSH frame processing:
 * - SetupResponse parsing (version, zstd support)
 * - DrainComplete notification handling
 * - StreamHeaders processing
 * - Passthrough path for non-METADATA_PUSH frames
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::thrift;
using namespace apache::thrift::fast_thrift::thrift::client::handler;
using namespace apache::thrift::fast_thrift::frame;

namespace {

// =============================================================================
// Minimal Benchmark Context
// =============================================================================

class BenchContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    lastReadMsg_ = std::move(msg);
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& /*e*/) noexcept {}

 private:
  TypeErasedBox lastReadMsg_;
};

// =============================================================================
// Helper Functions - ServerPushMetadata Serialization
// =============================================================================

std::unique_ptr<folly::IOBuf> serializeServerPushMetadata(
    const apache::thrift::ServerPushMetadata& metadata) {
  apache::thrift::CompactProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::ServerPushMetadata createSetupResponseMetadata() {
  apache::thrift::SetupResponse setupResponse;
  setupResponse.version() = 8;
  setupResponse.zstdSupported() = false;

  apache::thrift::ServerPushMetadata metadata;
  metadata.set_setupResponse(std::move(setupResponse));
  return metadata;
}

apache::thrift::ServerPushMetadata createSetupResponseWithZstdMetadata() {
  apache::thrift::SetupResponse setupResponse;
  setupResponse.version() = 8;
  setupResponse.zstdSupported() = true;

  apache::thrift::ServerPushMetadata metadata;
  metadata.set_setupResponse(std::move(setupResponse));
  return metadata;
}

apache::thrift::ServerPushMetadata createDrainCompleteMetadata() {
  apache::thrift::DrainCompletePush drainPush;

  apache::thrift::ServerPushMetadata metadata;
  metadata.set_drainCompletePush(std::move(drainPush));
  return metadata;
}

apache::thrift::ServerPushMetadata createDrainCompleteExceededMemMetadata() {
  apache::thrift::DrainCompletePush drainPush;
  drainPush.drainCompleteCode() =
      apache::thrift::DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT;

  apache::thrift::ServerPushMetadata metadata;
  metadata.set_drainCompletePush(std::move(drainPush));
  return metadata;
}

apache::thrift::ServerPushMetadata createStreamHeadersMetadata() {
  apache::thrift::StreamHeadersPush headersPush;
  headersPush.streamId() = 1;

  apache::thrift::ServerPushMetadata metadata;
  metadata.set_streamHeadersPush(std::move(headersPush));
  return metadata;
}

// =============================================================================
// Helper Functions - Typed Payload Creation
// =============================================================================

ThriftResponseMessage makeMetadataPushResponse(
    const apache::thrift::ServerPushMetadata& serverMeta) {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftMetadataPushPayload{
          .metadata = serializeServerPushMetadata(serverMeta)},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  response.streamType = FrameType::METADATA_PUSH;
  return response;
}

ThriftResponseMessage makePayloadResponse() {
  ThriftResponseMessage response;
  response.payload = ThriftClientInboundPayloadVariant{
      ThriftFirstResponsePayload{
          .data = folly::IOBuf::copyBuffer("response"),
          .metadata = std::make_unique<apache::thrift::ResponseRpcMetadata>(),
          .streamId = 1,
          .complete = true,
          .next = true},
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE};
  response.requestContext =
      apache::thrift::fast_thrift::rocket::borrow(reinterpret_cast<void*>(0x1));
  response.streamType = FrameType::REQUEST_RESPONSE;
  return response;
}

// =============================================================================
// METADATA_PUSH Processing Benchmarks
// =============================================================================

BENCHMARK(Read_MetadataPush_SetupResponse, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftClientMetadataPushHandler handler;
  BenchContext ctx;

  auto serverMeta = createSetupResponseMetadata();
  std::vector<ThriftResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makeMetadataPushResponse(serverMeta));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_MetadataPush_SetupResponse_WithZstd, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftClientMetadataPushHandler handler;
  BenchContext ctx;

  auto serverMeta = createSetupResponseWithZstdMetadata();
  std::vector<ThriftResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makeMetadataPushResponse(serverMeta));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_MetadataPush_DrainComplete, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftClientMetadataPushHandler handler;
  BenchContext ctx;

  auto serverMeta = createDrainCompleteMetadata();
  std::vector<ThriftResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makeMetadataPushResponse(serverMeta));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_MetadataPush_DrainComplete_ExceededMem, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftClientMetadataPushHandler handler;
  BenchContext ctx;

  auto serverMeta = createDrainCompleteExceededMemMetadata();
  std::vector<ThriftResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makeMetadataPushResponse(serverMeta));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK(Read_MetadataPush_StreamHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftClientMetadataPushHandler handler;
  BenchContext ctx;

  auto serverMeta = createStreamHeadersMetadata();
  std::vector<ThriftResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makeMetadataPushResponse(serverMeta));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Passthrough Path Benchmarks
// =============================================================================

BENCHMARK(Read_NonMetadataPush_Passthrough, iters) {
  folly::BenchmarkSuspender suspender;
  ThriftClientMetadataPushHandler handler;
  BenchContext ctx;

  std::vector<ThriftResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(makePayloadResponse());
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    auto result = handler.onRead(ctx, erase_and_box(std::move(responses[i])));
    folly::doNotOptimizeAway(result);
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
