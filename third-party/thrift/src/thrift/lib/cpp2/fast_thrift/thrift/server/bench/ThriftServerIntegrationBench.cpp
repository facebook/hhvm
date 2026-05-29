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
 * ThriftServer Integration Microbenchmarks
 *
 * Measures the full end-to-end server pipeline overhead including the
 * thrift layer (ThriftServerChannel + AsyncProcessor dispatch):
 * - Request path (inbound): Transport -> handlers -> ThriftServerChannel ->
 *                           RequestRpcMetadata deserialization -> Processor
 * - Response path (outbound): Processor::sendReply -> ResponseRpcMetadata
 *                             serialization -> handlers -> Transport
 *
 * This benchmarks the complete thrift server pipeline, extending the
 * rocket-only benchmarks with the thrift metadata deserialization and
 * processor dispatch overhead.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

// =============================================================================
// Constants
// =============================================================================

constexpr size_t kPayloadSize = 4'096;

std::unique_ptr<folly::IOBuf> makePayloadData(size_t size) {
  return folly::IOBuf::copyBuffer(std::string(size, 'x'));
}

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

// =============================================================================
// Echo AsyncProcessor — synchronously echoes request payload as reply
// =============================================================================

class EchoProcessor : public apache::thrift::AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedRequest,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {
    requestCount_++;
    if (!req->isOneway()) {
      auto data = std::move(serializedRequest).uncompress().buffer;
      req->sendReply(
          apache::thrift::ResponsePayload::create(std::move(data)),
          nullptr,
          folly::none);
    }
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {}

  int requestCount() const { return requestCount_; }

 private:
  int requestCount_{0};
};

class NoopProcessor : public apache::thrift::AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr,
      apache::thrift::SerializedCompressedRequest&&,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext*,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {
    requestCount_++;
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {}

  int requestCount() const { return requestCount_; }

 private:
  int requestCount_{0};
};

template <typename ProcessorT>
class BenchProcessorFactory : public apache::thrift::AsyncProcessorFactory {
 public:
  BenchProcessorFactory() : processor_(std::make_shared<ProcessorT>()) {}

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::unique_ptr<apache::thrift::AsyncProcessor>(
        new ForwardingProcessor(processor_));
  }

  std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
      override {
    return {};
  }

  CreateMethodMetadataResult createMethodMetadata() override { return {}; }

  ProcessorT& processor() { return *processor_; }

 private:
  class ForwardingProcessor : public apache::thrift::AsyncProcessor {
   public:
    explicit ForwardingProcessor(std::shared_ptr<ProcessorT> inner)
        : inner_(std::move(inner)) {}

    void processSerializedCompressedRequestWithMetadata(
        apache::thrift::ResponseChannelRequest::UniquePtr req,
        apache::thrift::SerializedCompressedRequest&& serializedRequest,
        const apache::thrift::AsyncProcessorFactory::MethodMetadata& metadata,
        apache::thrift::protocol::PROTOCOL_TYPES protocolId,
        apache::thrift::Cpp2RequestContext* ctx,
        folly::EventBase* evb,
        apache::thrift::concurrency::ThreadManager* tm) override {
      inner_->processSerializedCompressedRequestWithMetadata(
          std::move(req),
          std::move(serializedRequest),
          metadata,
          protocolId,
          ctx,
          evb,
          tm);
    }

    void processInteraction(apache::thrift::ServerRequest&&) override {}

   private:
    std::shared_ptr<ProcessorT> inner_;
  };

  std::shared_ptr<ProcessorT> processor_;
};

// =============================================================================
// Helpers
// =============================================================================

std::unique_ptr<folly::IOBuf> serializeRequestMetadata(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::RequestRpcMetadata createMinimalRequestMetadata() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "benchMethod";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

apache::thrift::RequestRpcMetadata createRequestMetadataWithHeaders() {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = "benchMethod";
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
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

std::unique_ptr<folly::IOBuf> createRequestFrame(
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
// Benchmark Fixture
// =============================================================================

template <typename ProcessorT>
struct BenchmarkFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler;
  std::shared_ptr<apache::thrift::fast_thrift::thrift::ThriftServerChannel>
      serverChannel;
  std::shared_ptr<BenchProcessorFactory<ProcessorT>> processorFactory;
  PipelineImpl::Ptr pipeline;
  TestAllocator allocator;

  void setup() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new BenchAsyncTransport(&evb));
    testTransport = static_cast<BenchAsyncTransport*>(transport.get());

    transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    processorFactory = std::make_shared<BenchProcessorFactory<ProcessorT>>();
    serverChannel = std::make_shared<
        apache::thrift::fast_thrift::thrift::ThriftServerChannel>(
        processorFactory);

    pipeline = PipelineBuilder<
                   apache::thrift::fast_thrift::transport::TransportHandler,
                   apache::thrift::fast_thrift::thrift::ThriftServerChannel,
                   TestAllocator>()
                   .setEventBase(&evb)
                   .setHead(transportHandler.get())
                   .setTail(serverChannel.get())
                   .setAllocator(&allocator)
                   .setHeadToTailOp(HeadToTailOp::Read)
                   .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                                      handler::RocketServerStreamStateHandler>(
                       server_stream_state_handler_tag)
                   .addNextDuplex<
                       apache::thrift::fast_thrift::rocket::server::handler::
                           RocketServerRequestResponseFrameHandler>(
                       server_request_response_frame_handler_tag)
                   .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                                      handler::RocketServerSetupFrameHandler>(
                       server_setup_frame_handler_tag)
                   .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                                      handler::RocketServerFrameCodecHandler>(
                       rocket_server_frame_codec_handler_tag)
                   .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                        handler::FrameLengthEncoderHandler>(
                       frame_length_encoder_handler_tag)
                   .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                       handler::FrameLengthParserHandler>(
                       frame_length_parser_handler_tag)
                   .build();

    serverChannel->setPipelineRef(*pipeline);
    serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(&evb));

    transportHandler->setPipeline(*pipeline);
    transportHandler->onConnect();

    // Inject SETUP frame
    auto setupFrame = serialize(
        SetupHeader{
            .majorVersion = 1,
            .minorVersion = 0,
            .keepaliveTime = 30000,
            .maxLifetime = 60000},
        nullptr,
        nullptr);
    testTransport->injectReadData(prependLengthPrefix(std::move(setupFrame)));
    evb.loopOnce();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    testTransport->injectReadData(prependLengthPrefix(std::move(frame)));
  }
};

// =============================================================================
// Request Path Benchmarks
// =============================================================================

BENCHMARK(ThriftServer_Request_MinimalMetadata, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture<NoopProcessor> fixture;
  fixture.setup();

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata());
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK(ThriftServer_Request_WithHeaders, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture<NoopProcessor> fixture;
  fixture.setup();

  auto metadataTemplate =
      serializeRequestMetadata(createRequestMetadataWithHeaders());
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Response Path Benchmark (Echo processor: request in -> reply out)
// =============================================================================

BENCHMARK(ThriftServer_Response_Success, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture<EchoProcessor> fixture;
  fixture.setup();

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata());
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

BENCHMARK_DRAW_LINE();

// =============================================================================
// Request-Response Round Trip
// =============================================================================

BENCHMARK(ThriftServer_RequestResponse_RoundTrip, iters) {
  folly::BenchmarkSuspender suspender;
  BenchmarkFixture<EchoProcessor> fixture;
  fixture.setup();

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata());
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (size_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (size_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
    fixture.testTransport->clearWrittenData();
  }
}

} // namespace

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  runBenchmarks();
  return 0;
}
