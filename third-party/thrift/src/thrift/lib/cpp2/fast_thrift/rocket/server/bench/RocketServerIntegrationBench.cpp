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
 * RocketServer Integration Microbenchmarks
 *
 * Measures the full end-to-end server pipeline overhead including:
 * - Request path (inbound): Transport -> all handlers -> app callback
 * - Response path (outbound): App -> all handlers -> transport
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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
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
using namespace apache::thrift::fast_thrift::rocket::server::handler;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

// =============================================================================
// Constants
// =============================================================================

constexpr size_t kPayloadSize = 1'024;

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

constexpr uint32_t kErrorCodeApplicationError = 0x00000201;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(rocket_server_setup_handler);
HANDLER_TAG(rocket_server_request_response_frame_handler);
HANDLER_TAG(rocket_server_stream_state_handler);

namespace {

// =============================================================================
// Rocket Server App Adapter - Minimal adapter for benchmarks
// =============================================================================

class RocketServerAppAdapter {
 public:
  using BytesPtr = apache::thrift::fast_thrift::channel_pipeline::BytesPtr;
  using TypeErasedBox =
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

  explicit RocketServerAppAdapter(folly::AsyncTransport::UniquePtr socket)
      : transportHandler_(
            apache::thrift::fast_thrift::transport::TransportHandler::create(
                std::move(socket))) {}

  ~RocketServerAppAdapter() {
    if (transportHandler_) {
      transportHandler_->onClose(folly::exception_wrapper{});
    }
  }

  RocketServerAppAdapter(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter& operator=(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter(RocketServerAppAdapter&&) = delete;
  RocketServerAppAdapter& operator=(RocketServerAppAdapter&&) = delete;

  apache::thrift::fast_thrift::transport::TransportHandler* transportHandler()
      const {
    return transportHandler_.get();
  }

  void setPipeline(PipelineImpl::Ptr pipeline) {
    pipeline_ = std::move(pipeline);
    if (transportHandler_ && pipeline_) {
      transportHandler_->setPipeline(*pipeline_);
    }
  }

  Result send(server::RocketResponseMessage&& msg) {
    if (!pipeline_) {
      return Result::Error;
    }
    return pipeline_->fireWrite(erase_and_box(std::move(msg)));
  }

  Result onMessage(TypeErasedBox&& /*msg*/) noexcept {
    requestCount_++;
    return Result::Success;
  }

  void onException(folly::exception_wrapper&& /*e*/) noexcept {
    exceptionReceived_ = true;
  }

  int requestCount() const { return requestCount_; }

 private:
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  PipelineImpl::Ptr pipeline_;
  int requestCount_{0};
  bool exceptionReceived_{false};
};

// =============================================================================
// Helper Functions
// =============================================================================

std::unique_ptr<folly::IOBuf> createSetupFrame() {
  return serialize(
      SetupHeader{
          .majorVersion = 1,
          .minorVersion = 0,
          .keepaliveTime = 30000,
          .maxLifetime = 60000},
      nullptr,
      nullptr);
}

std::unique_ptr<folly::IOBuf> createRequestResponseFrame(
    uint32_t streamId, std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      RequestResponseHeader{.streamId = streamId}, nullptr, std::move(data));
}

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

// =============================================================================
// Benchmark Fixture
// =============================================================================

struct BenchmarkFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  std::unique_ptr<RocketServerAppAdapter> appAdapter;
  PipelineImpl::Ptr pipeline;
  TestAllocator allocator;

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    appAdapter = std::make_unique<RocketServerAppAdapter>(std::move(transport));

    pipeline = PipelineBuilder<
                   RocketServerAppAdapter,
                   apache::thrift::fast_thrift::transport::TransportHandler,
                   TestAllocator>()
                   .setEventBase(&evb)
                   .setTail(appAdapter->transportHandler())
                   .setHead(appAdapter.get())
                   .setAllocator(&allocator)
                   .addNextInbound<FrameLengthParserHandler>(
                       frame_length_parser_handler_tag)
                   .addNextOutbound<FrameLengthEncoderHandler>(
                       frame_length_encoder_handler_tag)
                   .addNextDuplex<RocketServerFrameCodecHandler>(
                       rocket_server_frame_codec_handler_tag)
                   .addNextDuplex<RocketServerSetupFrameHandler>(
                       rocket_server_setup_handler_tag)
                   .addNextDuplex<RocketServerRequestResponseFrameHandler>(
                       rocket_server_request_response_frame_handler_tag)
                   .addNextDuplex<RocketServerStreamStateHandler>(
                       rocket_server_stream_state_handler_tag)
                   .build();

    appAdapter->setPipeline(std::move(pipeline));
    appAdapter->transportHandler()->onConnect();

    // Inject SETUP frame to bring pipeline into ready state
    injectFrame(createSetupFrame());
    evb.loopOnce();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// Request Path Benchmark (inbound: Transport -> App)
// =============================================================================

BENCHMARK(Rocket_Server_Request, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(
        createRequestResponseFrame(i * 2 + 1, makePayloadData(kPayloadSize)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmark (outbound: App -> Transport)
// =============================================================================

BENCHMARK(Rocket_Server_Response_Payload, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-inject requests to register streams
  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(
        createRequestResponseFrame(i * 2 + 1, makePayloadData(10)));
    fixture.evb.loopOnce();
  }
  fixture.testTransport->clearWrittenData();

  // Prepare responses
  std::vector<server::RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    server::RocketResponseMessage resp;
    resp.streamId = static_cast<uint32_t>(i * 2 + 1);
    resp.payload = makePayloadData(kPayloadSize);
    resp.complete = true;
    responses.push_back(std::move(resp));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)fixture.appAdapter->send(std::move(responses[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK(Rocket_Server_Response_Error, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-inject requests to register streams
  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(
        createRequestResponseFrame(i * 2 + 1, makePayloadData(10)));
    fixture.evb.loopOnce();
  }
  fixture.testTransport->clearWrittenData();

  // Prepare error responses
  std::vector<server::RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    server::RocketResponseMessage resp;
    resp.streamId = static_cast<uint32_t>(i * 2 + 1);
    resp.payload = folly::IOBuf::copyBuffer("error");
    resp.errorCode = kErrorCodeApplicationError;
    resp.complete = true;
    responses.push_back(std::move(resp));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)fixture.appAdapter->send(std::move(responses[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Setup Frame Benchmark
// =============================================================================

BENCHMARK(Rocket_Server_SetupFrame, iters) {
  folly::BenchmarkSuspender suspender;

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    folly::BenchmarkSuspender innerSuspender;
    BenchmarkFixture fixture;

    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&fixture.evb));
    fixture.testTransport = static_cast<BenchAsyncTransport*>(transport.get());
    fixture.appAdapter =
        std::make_unique<RocketServerAppAdapter>(std::move(transport));

    auto pipeline =
        PipelineBuilder<
            RocketServerAppAdapter,
            apache::thrift::fast_thrift::transport::TransportHandler,
            TestAllocator>()
            .setEventBase(&fixture.evb)
            .setTail(fixture.appAdapter->transportHandler())
            .setHead(fixture.appAdapter.get())
            .setAllocator(&fixture.allocator)
            .addNextInbound<FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<RocketServerFrameCodecHandler>(
                rocket_server_frame_codec_handler_tag)
            .addNextDuplex<RocketServerSetupFrameHandler>(
                rocket_server_setup_handler_tag)
            .addNextDuplex<RocketServerRequestResponseFrameHandler>(
                rocket_server_request_response_frame_handler_tag)
            .addNextDuplex<RocketServerStreamStateHandler>(
                rocket_server_stream_state_handler_tag)
            .build();

    fixture.appAdapter->setPipeline(std::move(pipeline));
    fixture.appAdapter->transportHandler()->onConnect();

    auto setupFrame = createSetupFrame();

    innerSuspender.dismiss();

    // Measure SETUP frame validation
    fixture.injectFrame(std::move(setupFrame));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Request-Response Round Trip Benchmark
// =============================================================================

BENCHMARK(Rocket_Server_RequestResponse_RoundTrip, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-build request frames and responses
  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(
        createRequestResponseFrame(i * 2 + 1, makePayloadData(kPayloadSize)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    // Inbound: inject request
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();

    // Outbound: send response
    server::RocketResponseMessage resp;
    resp.streamId = static_cast<uint32_t>(i * 2 + 1);
    resp.payload = makePayloadData(kPayloadSize);
    resp.complete = true;
    (void)fixture.appAdapter->send(std::move(resp));
    fixture.evb.loopOnce();

    fixture.testTransport->clearWrittenData();
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
