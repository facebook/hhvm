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
 * RocketServer Stream Integration Microbenchmarks
 *
 * Measures the full end-to-end server streaming pipeline overhead including:
 * - Request path (inbound): Transport -> all handlers -> app callback
 * - Response path (outbound): App -> all handlers -> transport
 * - Flow control (REQUEST_N credit grants)
 *
 * This benchmarks the rocket-only streaming pipeline without thrift-specific
 * handlers. The test treats the adapter as a black box: injects client frames
 * via transport and validates responses via the adapter's write() method.
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
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameLengthParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/bench/BenchContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::handler;
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
constexpr uint32_t kHighCredits = 1'000'000;

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(frame_codec_handler);
HANDLER_TAG(frame_defragmentation_handler);
HANDLER_TAG(frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(rocket_server_setup_handler);
HANDLER_TAG(rocket_server_stream_state_handler);
HANDLER_TAG(rocket_server_stream_handler);

using AppAdapter =
    apache::thrift::fast_thrift::rocket::server::RocketServerAppAdapter;

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

std::unique_ptr<folly::IOBuf> createRequestStreamFrame(
    uint32_t streamId,
    uint32_t initialRequestN,
    std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      RequestStreamHeader{
          .streamId = streamId, .initialRequestN = initialRequestN},
      nullptr,
      std::move(data));
}

std::unique_ptr<folly::IOBuf> createRequestNFrame(
    uint32_t streamId, uint32_t requestN) {
  return serialize(RequestNHeader{.streamId = streamId, .requestN = requestN});
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
  BenchmarkFixture() = default;
  ~BenchmarkFixture() {
    if (transportHandler) {
      transportHandler->close(folly::exception_wrapper{});
      transportHandler->resetPipeline();
    }
    pipeline.reset();
  }
  BenchmarkFixture(const BenchmarkFixture&) = delete;
  BenchmarkFixture& operator=(const BenchmarkFixture&) = delete;
  BenchmarkFixture(BenchmarkFixture&&) = delete;
  BenchmarkFixture& operator=(BenchmarkFixture&&) = delete;

  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  AppAdapter::Ptr appAdapter{new AppAdapter()};
  apache::thrift::fast_thrift::transport::TransportHandlerT<
      apache::thrift::fast_thrift::transport::NoOpWriteCompleteEventFactory,
      apache::thrift::fast_thrift::frame::read::FrameLengthParser>::Ptr
      transportHandler;
  PipelineImpl::Ptr pipeline;
  TestAllocator allocator;

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandlerT<
            apache::thrift::fast_thrift::transport::
                NoOpWriteCompleteEventFactory,
            apache::thrift::fast_thrift::frame::read::FrameLengthParser>::
            create(std::move(transport));

    appAdapter->setRequestHandlers(
        [](TypeErasedBox&& /*msg*/) noexcept -> Result {
          return Result::Success;
        },
        [](folly::exception_wrapper&& /*e*/) noexcept {});

    pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandlerT<
                apache::thrift::fast_thrift::transport::
                    NoOpWriteCompleteEventFactory,
                apache::thrift::fast_thrift::frame::read::FrameLengthParser>,
            AppAdapter,
            TestAllocator>()
            .setEventBase(&evb)
            .setHead(transportHandler.get())
            .setTail(appAdapter.get())
            .setAllocator(&allocator)
            .addState<
                apache::thrift::fast_thrift::rocket::RocketStreamContexts>()

            .addNextOutbound<FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<FrameCodecHandler>(frame_codec_handler_tag)
            .addNextInbound<FrameDefragmentationHandler>(
                frame_defragmentation_handler_tag)
            .addNextOutbound<FrameFragmentationHandler>(
                frame_fragmentation_handler_tag,
                frame::write::FragmentationHandlerConfig{})
            .addNextDuplex<RocketServerMessageMarshalHandler>(
                rocket_server_message_marshal_handler_tag)
            .addNextDuplex<RocketServerSetupFrameHandler>(
                rocket_server_setup_handler_tag)
            .addNextDuplex<RocketServerStreamStateHandler>(
                rocket_server_stream_state_handler_tag)
            .addNextDuplex<RocketServerStreamHandler>(
                rocket_server_stream_handler_tag)
            .build();

    appAdapter->setPipeline(pipeline.get());
    transportHandler->setPipeline(pipeline.get());
    transportHandler->onConnect();

    // Inject SETUP frame to bring pipeline into ready state
    injectFrame(createSetupFrame());
    evb.loopOnce();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// REQUEST_STREAM Inbound Benchmark
// =============================================================================

BENCHMARK(Rocket_Server_Stream_Request, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestStreamFrame(
        i * 2 + 1, kHighCredits, makePayloadData(kPayloadSize)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// REQUEST_N Inbound Benchmark
// =============================================================================

BENCHMARK(Rocket_Server_Stream_RequestN, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-register streams via REQUEST_STREAM
  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(
        createRequestStreamFrame(i * 2 + 1, 1, makePayloadData(10)));
    fixture.evb.loopOnce();
  }
  fixture.testTransport->clearWrittenData();

  // Prepare REQUEST_N frames
  std::vector<std::unique_ptr<folly::IOBuf>> requestNFrames;
  requestNFrames.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requestNFrames.push_back(createRequestNFrame(i * 2 + 1, 100));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requestNFrames[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Outbound PAYLOAD Benchmark (non-terminal)
// =============================================================================

BENCHMARK(Rocket_Server_Stream_Payload, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-register streams with high credits
  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(
        createRequestStreamFrame(i * 2 + 1, kHighCredits, makePayloadData(10)));
    fixture.evb.loopOnce();
  }
  fixture.testTransport->clearWrittenData();

  // Prepare non-terminal PAYLOAD responses
  std::vector<server::RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    server::RocketResponseMessage resp{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedFrame{
                .frameType =
                    apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
                .streamId = static_cast<uint32_t>(i * 2 + 1),
                .metadata = nullptr,
                .data = makePayloadData(kPayloadSize),
                .complete = false,
                .next = true,
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
    };
    responses.push_back(std::move(resp));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)fixture.appAdapter->write(std::move(responses[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Outbound PAYLOAD+COMPLETE Benchmark (terminal)
// =============================================================================

BENCHMARK(Rocket_Server_Stream_PayloadComplete, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-register streams with high credits
  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(
        createRequestStreamFrame(i * 2 + 1, kHighCredits, makePayloadData(10)));
    fixture.evb.loopOnce();
  }
  fixture.testTransport->clearWrittenData();

  // Prepare terminal PAYLOAD responses (next=true, complete=true)
  std::vector<server::RocketResponseMessage> responses;
  responses.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    server::RocketResponseMessage resp{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedFrame{
                .frameType =
                    apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
                .streamId = static_cast<uint32_t>(i * 2 + 1),
                .metadata = nullptr,
                .data = makePayloadData(kPayloadSize),
                .complete = true,
                .next = true,
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
    };
    responses.push_back(std::move(resp));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    (void)fixture.appAdapter->write(std::move(responses[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Full Stream Round Trip Benchmark
// =============================================================================

BENCHMARK(Rocket_Server_Stream_RoundTrip, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture fixture;
  fixture.setup();

  // Pre-build REQUEST_STREAM frames
  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(
        createRequestStreamFrame(i * 2 + 1, 1, makePayloadData(kPayloadSize)));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    uint32_t streamId = i * 2 + 1;

    // Inbound: inject REQUEST_STREAM
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();

    // Inbound: inject REQUEST_N to grant credits for the response
    fixture.injectFrame(createRequestNFrame(streamId, 1));
    fixture.evb.loopOnce();

    // Outbound: send non-terminal PAYLOAD
    server::RocketResponseMessage payloadResp{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedFrame{
                .frameType =
                    apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
                .streamId = streamId,
                .metadata = nullptr,
                .data = makePayloadData(kPayloadSize),
                .next = true,
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
    };
    (void)fixture.appAdapter->write(std::move(payloadResp));
    fixture.evb.loopOnce();

    // Outbound: send terminal PAYLOAD (complete)
    server::RocketResponseMessage completeResp{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedFrame{
                .frameType =
                    apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
                .streamId = streamId,
                .metadata = nullptr,
                .data = nullptr,
                .complete = true,
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
    };
    (void)fixture.appAdapter->write(std::move(completeResp));
    fixture.evb.loopOnce();

    fixture.testTransport->clearWrittenData();
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
