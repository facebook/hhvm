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
 * RocketClient Integration Microbenchmarks
 *
 * Measures the full end-to-end pipeline overhead including:
 * - Request path (outbound): App -> all handlers -> transport
 * - Response path (inbound): transport -> all handlers -> app callback
 *
 * This benchmarks the rocket-only pipeline without thrift-specific handlers.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::rocket;
using namespace apache::thrift::fast_thrift::rocket::client::handler;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

// =============================================================================
// Constants
// =============================================================================

constexpr size_t kPayloadSize = 1'024;

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

// RSocket error codes
constexpr uint32_t kErrorCodeApplicationError = 0x00000201;
constexpr uint32_t kErrorCodeConnectionClose = 0x00000102;

// Handler tags for pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_frame_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);

namespace {

using AppAdapter =
    apache::thrift::fast_thrift::rocket::client::RocketClientAppAdapter;

// =============================================================================
// Helper Functions
// =============================================================================

std::unique_ptr<folly::IOBuf> createPayloadResponse(
    uint32_t streamId, std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      PayloadHeader{.streamId = streamId, .complete = true, .next = true},
      nullptr,
      std::move(data));
}

std::unique_ptr<folly::IOBuf> createErrorFrame(
    uint32_t streamId, uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      ErrorHeader{.streamId = streamId, .errorCode = errorCode},
      nullptr,
      std::move(data));
}

// Prepend 3-byte length prefix to a frame (for injection)
std::unique_ptr<folly::IOBuf> prependLengthPrefix(
    std::unique_ptr<folly::IOBuf> frame) {
  size_t frameLength = frame->computeChainDataLength();
  auto lengthPrefix = folly::IOBuf::create(kMetadataLengthSize);
  uint8_t* data = lengthPrefix->writableData();
  data[0] = static_cast<uint8_t>((frameLength >> 16) & 0xFF);
  data[1] = static_cast<uint8_t>((frameLength >> 8) & 0xFF);
  data[2] = static_cast<uint8_t>(frameLength & 0xFF);
  lengthPrefix->append(kMetadataLengthSize);
  lengthPrefix->appendChain(std::move(frame));
  return lengthPrefix;
}

RocketRequestMessage createRocketRequest(size_t payloadSize) {
  return RocketRequestMessage{
      .frame =
          RocketFramePayload{
              .metadata = nullptr,
              .data = makePayloadData(payloadSize),
              .streamId = kInvalidStreamId,
          },
      .requestHandle = 1,
      .frameType = FrameType::REQUEST_RESPONSE,
  };
}

// =============================================================================
// Benchmark Fixture
// =============================================================================

struct BenchmarkFixture {
  BenchmarkFixture() = default;
  ~BenchmarkFixture() {
    pipeline.reset();
    if (transportHandler) {
      transportHandler->onClose(folly::exception_wrapper{});
    }
  }
  BenchmarkFixture(const BenchmarkFixture&) = delete;
  BenchmarkFixture& operator=(const BenchmarkFixture&) = delete;
  BenchmarkFixture(BenchmarkFixture&&) = delete;
  BenchmarkFixture& operator=(BenchmarkFixture&&) = delete;

  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  AppAdapter::Ptr appAdapter{new AppAdapter()};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler;
  PipelineImpl::Ptr pipeline;
  TestAllocator allocator;

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    appAdapter->setResponseHandlers(
        [](TypeErasedBox&& /*msg*/) noexcept -> Result {
          return Result::Success;
        },
        [](folly::exception_wrapper&& /*e*/) noexcept {});

    pipeline = PipelineBuilder<
                   apache::thrift::fast_thrift::transport::TransportHandler,
                   AppAdapter,
                   TestAllocator>()
                   .setEventBase(&evb)
                   .setHead(transportHandler.get())
                   .setTail(appAdapter.get())
                   .setAllocator(&allocator)
                   .addNextInbound<FrameLengthParserHandler>(
                       frame_length_parser_handler_tag)
                   .addNextOutbound<FrameLengthEncoderHandler>(
                       frame_length_encoder_handler_tag)
                   .addNextDuplex<RocketClientFrameCodecHandler>(
                       rocket_client_frame_codec_handler_tag)
                   .addNextInbound<RocketClientErrorFrameHandler>(
                       rocket_client_error_frame_handler_tag)
                   .addNextDuplex<RocketClientSetupFrameHandler>(
                       rocket_client_setup_handler_tag,
                       []() {
                         return std::make_pair(
                             folly::IOBuf::copyBuffer("setup"),
                             std::unique_ptr<folly::IOBuf>());
                       })
                   .addNextDuplex<RocketClientRequestResponseFrameHandler>(
                       rocket_client_request_response_frame_handler_tag)
                   .addNextDuplex<RocketClientStreamStateHandler>(
                       rocket_client_stream_state_handler_tag)
                   .build();

    appAdapter->setPipeline(pipeline.get());
    transportHandler->setPipeline(*pipeline);
    transportHandler->onConnect();

    // Drive event loop and discard SETUP frame
    evb.loopOnce();
    testTransport->getWrittenData();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// Request Path Benchmarks
// =============================================================================

BENCHMARK(Rocket_Request, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  std::vector<RocketRequestMessage> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRocketRequest(kPayloadSize));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)fixture.appAdapter->write(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmarks
// =============================================================================

BENCHMARK(Rocket_Response_Payload, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-send requests to populate stream state
  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    auto request = createRocketRequest(10);
    (void)fixture.appAdapter->write(std::move(request));
    fixture.evb.loopOnce();
    streamIds.push_back(i * 2 + 1);
  }
  fixture.testTransport->clearWrittenData();

  // Prepare responses
  std::vector<std::unique_ptr<folly::IOBuf>> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(
        createPayloadResponse(streamIds[i], makePayloadData(kPayloadSize)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(responses[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK(Rocket_Response_StreamError, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-send requests
  std::vector<uint32_t> streamIds;
  streamIds.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    auto request = createRocketRequest(10);
    (void)fixture.appAdapter->write(std::move(request));
    fixture.evb.loopOnce();
    streamIds.push_back(i * 2 + 1);
  }
  fixture.testTransport->clearWrittenData();

  // Prepare error responses (stream-level APPLICATION_ERROR)
  std::vector<std::unique_ptr<folly::IOBuf>> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    responses.push_back(createErrorFrame(
        streamIds[i],
        kErrorCodeApplicationError,
        folly::IOBuf::copyBuffer("error")));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(responses[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Setup Frame Benchmark
// =============================================================================

BENCHMARK(Rocket_SetupFrame, iters) {
  folly::BenchmarkSuspender suspender;

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    folly::BenchmarkSuspender innerSuspender;
    BenchmarkFixture fixture;

    // Measure only the setup which sends the SETUP frame
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&fixture.evb));
    fixture.testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    fixture.transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));
    fixture.appAdapter->setResponseHandlers(
        [](TypeErasedBox&& /*msg*/) noexcept -> Result {
          return Result::Success;
        },
        [](folly::exception_wrapper&& /*e*/) noexcept {});

    fixture.pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            AppAdapter,
            TestAllocator>()
            .setEventBase(&fixture.evb)
            .setHead(fixture.transportHandler.get())
            .setTail(fixture.appAdapter.get())
            .setAllocator(&fixture.allocator)
            .addNextInbound<FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<RocketClientFrameCodecHandler>(
                rocket_client_frame_codec_handler_tag)
            .addNextInbound<RocketClientErrorFrameHandler>(
                rocket_client_error_frame_handler_tag)
            .addNextDuplex<RocketClientSetupFrameHandler>(
                rocket_client_setup_handler_tag,
                []() {
                  return std::make_pair(
                      folly::IOBuf::copyBuffer("setup"),
                      std::unique_ptr<folly::IOBuf>());
                })
            .addNextDuplex<RocketClientRequestResponseFrameHandler>(
                rocket_client_request_response_frame_handler_tag)
            .addNextDuplex<RocketClientStreamStateHandler>(
                rocket_client_stream_state_handler_tag)
            .build();

    fixture.appAdapter->setPipeline(fixture.pipeline.get());
    fixture.transportHandler->setPipeline(*fixture.pipeline);

    innerSuspender.dismiss();

    // This triggers onConnect which sends SETUP frame
    fixture.transportHandler->onConnect();
    fixture.evb.loopOnce();

    folly::doNotOptimizeAway(fixture.testTransport->getWrittenData());
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Connection-Level Error Benchmark (triggers exception and connection close)
// =============================================================================

BENCHMARK(Rocket_ConnectionError, iters) {
  folly::BenchmarkSuspender suspender;

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    folly::BenchmarkSuspender innerSuspender;
    BenchmarkFixture fixture;
    fixture.setup();

    // Create connection-level error frame (streamId = 0)
    auto errorFrame = createErrorFrame(
        0, // streamId 0 = connection-level
        kErrorCodeConnectionClose,
        folly::IOBuf::copyBuffer("Connection closed"));

    innerSuspender.dismiss();

    fixture.injectFrame(std::move(errorFrame));
    fixture.evb.loopOnce();
  }
}

} // namespace

} // namespace

// =============================================================================
// Main
// =============================================================================

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
