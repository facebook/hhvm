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
 * FastThriftServer Integration Microbenchmarks
 *
 * Server-side counterpart to FastThriftClientIntegrationBench. Compares the
 * generated server-side dispatch paths through the rocket pipeline with a
 * mock socket:
 *
 *   BENCHMARK(FastThriftChannel_*)        — ThriftServerChannel +
 *                                           apache::thrift::ServiceHandler<
 *                                               FastThriftChannelServer>
 *                                           (legacy generated handler)
 *   BENCHMARK_RELATIVE(FastThriftHandler_*) — FastThriftServerAppAdapter +
 *                                             FastServiceHandler<FastThriftServer>
 *                                             (new generated handler)
 *
 * Both fixtures share the same rocket-server pipeline +
 * ThriftServerTransportAdapter and use BenchAsyncTransport so the comparison
 * isolates the dispatch path (parse → route → invoke handler → write reply).
 *
 * Mirrors the Request_* / Response_* / RequestResponse_RoundTrip layout from
 * ThriftServerIntegrationBench:
 *   Request_*  — handler drops the callback. Both paths incur an
 *                INTERNAL_ERROR reply on destruct; comparing the dispatch
 *                cost ahead of the reply.
 *   Response_* — handler echoes the payload back through the typed callback.
 *   RoundTrip  — Response_* with `clearWrittenData` per iteration.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallback.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftChannelServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.tcc>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::transport::bench;
using namespace apache::thrift::fast_thrift::thrift::test::integration;

namespace {

// =============================================================================
// Constants
// =============================================================================

constexpr size_t kPayloadSize = 4'096;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

// =============================================================================
// Generated handlers — one per dispatch path. Both honor `noReply`:
//   true  → callback is dropped (destructor writes INTERNAL_ERROR)
//   false → callback completes with the echoed payload
// =============================================================================

class ChannelHandler
    : public apache::thrift::ServiceHandler<FastThriftChannelServer> {
 public:
  bool noReply{false};

  void async_tm_ping(
      apache::thrift::HandlerCallbackPtr<void> callback) override {
    if (noReply) {
      return;
    }
    callback->done();
  }

  void async_tm_echo(
      apache::thrift::HandlerCallbackPtr<std::unique_ptr<EchoResponse>>
          callback,
      std::unique_ptr<std::string> message) override {
    if (noReply) {
      return;
    }
    auto response = std::make_unique<EchoResponse>();
    response->message() = std::move(*message);
    callback->result(std::move(response));
  }
};

class FastHandler
    : public ::apache::thrift::FastServiceHandler<FastThriftServer> {
 public:
  bool noReply{false};

  void async_eb_ping(
      ::apache::thrift::fast_thrift::thrift::FastHandlerCallbackPtr<void>
          callback) override {
    if (noReply) {
      return;
    }
    callback->done();
  }

  void async_eb_echo(
      ::apache::thrift::fast_thrift::thrift::FastHandlerCallbackPtr<
          std::unique_ptr<EchoResponse>> callback,
      std::unique_ptr<std::string> message) override {
    if (noReply) {
      return;
    }
    auto response = std::make_unique<EchoResponse>();
    response->message() = std::move(*message);
    callback->result(std::move(response));
  }
};

// =============================================================================
// Helpers — frame + pargs construction
// =============================================================================

std::unique_ptr<folly::IOBuf> serializeRequestMetadata(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::RequestRpcMetadata makeRequestMetadata(
    std::string_view methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = std::string(methodName);
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

apache::thrift::RequestRpcMetadata makeRequestMetadataWithHeaders(
    std::string_view methodName) {
  auto metadata = makeRequestMetadata(methodName);
  metadata.clientTimeoutMs() = 10000;
  metadata.queueTimeoutMs() = 5000;

  auto& headers = metadata.otherMetadata().ensure();
  headers["request_id"] = "abc123";
  headers["trace_id"] = "trace-456";
  headers["client_name"] = "benchmark_client";
  headers["routing_key"] = "shard-42";
  headers["priority"] = "high";

  return metadata;
}

std::unique_ptr<folly::IOBuf> serializeEmptyPargs() {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  // ping_pargs has no fields; both legacy and fast generators emit
  // structurally identical empty pargs, so reuse one for both fixtures.
  FastThriftServer_ping_pargs pargs;
  pargs.write(&writer);
  return queue.move();
}

std::unique_ptr<folly::IOBuf> serializeEchoPargs(const std::string& payload) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  FastThriftServer_echo_pargs pargs;
  pargs.template get<0>().value = const_cast<std::string*>(&payload);
  pargs.write(&writer);
  return queue.move();
}

std::unique_ptr<folly::IOBuf> createFastThriftRequestFrame(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return serialize(
      RequestResponseHeader{.streamId = streamId},
      std::move(metadata),
      std::move(data));
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
// Rocket pipeline — shared between both fixtures
// =============================================================================

struct RocketPipelineResources {
  rocket::server::RocketServerAppAdapter::Ptr appAdapter;
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler;
  PipelineImpl::Ptr pipeline;
  SimpleBufferAllocator allocator;

  BenchAsyncTransport* setup(folly::EventBase* evb) {
    appAdapter.reset(new rocket::server::RocketServerAppAdapter());

    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(evb));
    auto* testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            rocket::server::RocketServerAppAdapter,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(transportHandler.get())
            .setTail(appAdapter.get())
            .setAllocator(&allocator)
            .addNextInbound<FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerFrameCodecHandler>(
                rocket_server_frame_codec_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerSetupFrameHandler>(
                server_setup_frame_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerStreamStateHandler>(
                server_stream_state_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerRequestResponseHandler>(
                server_request_response_frame_handler_tag)
            .build();

    appAdapter->setPipeline(pipeline.get());
    transportHandler->setPipeline(pipeline.get());
    transportHandler->onConnect();

    return testTransport;
  }
};

void injectSetupFrame(BenchAsyncTransport* transport, folly::EventBase& evb) {
  auto setupFrame = serialize(
      SetupHeader{
          .majorVersion = 1,
          .minorVersion = 0,
          .keepaliveTime = 30000,
          .maxLifetime = 60000},
      nullptr,
      nullptr);
  transport->injectReadData(prependLengthPrefix(std::move(setupFrame)));
  evb.loopOnce();
}

// =============================================================================
// FastThriftChannel fixture — ThriftServerChannel + legacy generated handler
// =============================================================================

struct ChannelBenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};

  RocketPipelineResources rocketResources;

  std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
      transportAdapter;
  std::shared_ptr<ChannelHandler> handler;
  std::shared_ptr<thrift::ThriftServerChannel> serverChannel;
  PipelineImpl::Ptr thriftPipeline;
  SimpleBufferAllocator thriftAllocator;

  void setup(bool noReply) {
    testTransport = rocketResources.setup(&evb);

    handler = std::make_shared<ChannelHandler>();
    handler->noReply = noReply;
    serverChannel = std::make_shared<thrift::ThriftServerChannel>(handler);

    transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            *rocketResources.appAdapter);

    thriftPipeline = PipelineBuilder<
                         thrift::server::ThriftServerTransportAdapter,
                         thrift::ThriftServerChannel,
                         SimpleBufferAllocator>()
                         .setEventBase(&evb)
                         .setHead(transportAdapter.get())
                         .setTail(serverChannel.get())
                         .setAllocator(&thriftAllocator)
                         .build();

    transportAdapter->setPipeline(thriftPipeline.get());
    serverChannel->setPipelineRef(*thriftPipeline);
    serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(&evb));

    injectSetupFrame(testTransport, evb);
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// FastThriftHandler fixture — FastThriftServerAppAdapter + new generated
// handler
// =============================================================================

struct FastBenchFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};

  RocketPipelineResources rocketResources;

  std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
      transportAdapter;
  std::shared_ptr<FastHandler> handler;
  std::unique_ptr<
      FastThriftServerAppAdapter,
      folly::DelayedDestruction::Destructor>
      adapter;
  PipelineImpl::Ptr thriftPipeline;
  SimpleBufferAllocator thriftAllocator;

  void setup(bool noReply) {
    testTransport = rocketResources.setup(&evb);

    handler = std::make_shared<FastHandler>();
    handler->noReply = noReply;
    adapter.reset(new FastThriftServerAppAdapter(handler));

    transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            *rocketResources.appAdapter);

    thriftPipeline = PipelineBuilder<
                         thrift::server::ThriftServerTransportAdapter,
                         FastThriftServerAppAdapter,
                         SimpleBufferAllocator>()
                         .setEventBase(&evb)
                         .setHead(transportAdapter.get())
                         .setTail(adapter.get())
                         .setAllocator(&thriftAllocator)
                         .build();

    transportAdapter->setPipeline(thriftPipeline.get());
    adapter->setPipeline(thriftPipeline.get());

    injectSetupFrame(testTransport, evb);
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// Bench bodies
// =============================================================================

template <typename Fixture>
void runRequestBench(
    Fixture& fixture,
    const apache::thrift::RequestRpcMetadata& metadataProto,
    uint32_t iters) {
  auto metadataTemplate = serializeRequestMetadata(metadataProto);
  auto pargsTemplate = serializeEmptyPargs();

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createFastThriftRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), pargsTemplate->clone()));
  }

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

template <typename Fixture>
void runResponseBench(
    Fixture& fixture,
    const apache::thrift::RequestRpcMetadata& metadataProto,
    const std::string& payload,
    uint32_t iters,
    bool clearBetween) {
  auto metadataTemplate = serializeRequestMetadata(metadataProto);
  auto pargsTemplate = serializeEchoPargs(payload);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createFastThriftRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), pargsTemplate->clone()));
  }

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
    if (clearBetween) {
      fixture.testTransport->clearWrittenData();
    }
  }
}

// =============================================================================
// Request-only — minimal metadata
// =============================================================================

BENCHMARK(FastThriftChannel_Request_MinimalMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  ChannelBenchFixture fixture;
  fixture.setup(/*noReply=*/true);
  suspender.dismiss();

  runRequestBench(fixture, makeRequestMetadata("ping"), iters);
}

BENCHMARK_RELATIVE(FastThriftHandler_Request_MinimalMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  FastBenchFixture fixture;
  fixture.setup(/*noReply=*/true);
  suspender.dismiss();

  runRequestBench(fixture, makeRequestMetadata("ping"), iters);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Request-only — request metadata with headers
// =============================================================================

BENCHMARK(FastThriftChannel_Request_WithHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  ChannelBenchFixture fixture;
  fixture.setup(/*noReply=*/true);
  suspender.dismiss();

  runRequestBench(fixture, makeRequestMetadataWithHeaders("ping"), iters);
}

BENCHMARK_RELATIVE(FastThriftHandler_Request_WithHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  FastBenchFixture fixture;
  fixture.setup(/*noReply=*/true);
  suspender.dismiss();

  runRequestBench(fixture, makeRequestMetadataWithHeaders("ping"), iters);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response — handler echoes back
// =============================================================================

BENCHMARK(FastThriftChannel_Response_Success, iters) {
  folly::BenchmarkSuspender suspender;
  ChannelBenchFixture fixture;
  fixture.setup(/*noReply=*/false);
  std::string payload(kPayloadSize, 'x');
  suspender.dismiss();

  runResponseBench(
      fixture,
      makeRequestMetadata("echo"),
      payload,
      iters,
      /*clearBetween=*/false);
}

BENCHMARK_RELATIVE(FastThriftHandler_Response_Success, iters) {
  folly::BenchmarkSuspender suspender;
  FastBenchFixture fixture;
  fixture.setup(/*noReply=*/false);
  std::string payload(kPayloadSize, 'x');
  suspender.dismiss();

  runResponseBench(
      fixture,
      makeRequestMetadata("echo"),
      payload,
      iters,
      /*clearBetween=*/false);
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Round trip — handler echoes, written data cleared per iteration
// =============================================================================

BENCHMARK(FastThriftChannel_RequestResponse_RoundTrip, iters) {
  folly::BenchmarkSuspender suspender;
  ChannelBenchFixture fixture;
  fixture.setup(/*noReply=*/false);
  std::string payload(kPayloadSize, 'x');
  suspender.dismiss();

  runResponseBench(
      fixture,
      makeRequestMetadata("echo"),
      payload,
      iters,
      /*clearBetween=*/true);
}

BENCHMARK_RELATIVE(FastThriftHandler_RequestResponse_RoundTrip, iters) {
  folly::BenchmarkSuspender suspender;
  FastBenchFixture fixture;
  fixture.setup(/*noReply=*/false);
  std::string payload(kPayloadSize, 'x');
  suspender.dismiss();

  runResponseBench(
      fixture,
      makeRequestMetadata("echo"),
      payload,
      iters,
      /*clearBetween=*/true);
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
