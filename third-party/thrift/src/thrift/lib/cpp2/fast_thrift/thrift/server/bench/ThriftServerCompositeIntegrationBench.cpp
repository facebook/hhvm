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
 * ThriftServerCompositeAppAdapter integration microbenchmarks.
 *
 * Measures the wire-level cost the composite layer adds when sitting at
 * the thrift pipeline tail under the full rocket+thrift handler stack.
 * Parallels ThriftServerIntegrationBench.cpp's two-pipeline fixture pattern;
 * the BareAdapter baseline uses a single ThriftServerAppAdapter directly as
 * the thrift tail, the Composite variants wrap one or more children behind
 * the composite.
 *
 * Companion integration test: ThriftServerCompositeIntegrationTest.
 *
 * All benches send request-only (no response capture) so that
 * BenchAsyncTransport's deferred writeSuccess interaction with synchronous
 * response writes doesn't constrain multi-request iteration. Composite-
 * level dispatch-only microbenchmarks live in
 * ThriftServerCompositeAppAdapterBench.cpp.
 */

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>

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
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/bench/BenchAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace folly;
using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read::handler;
using namespace apache::thrift::fast_thrift::frame::write::handler;
using namespace apache::thrift::fast_thrift::frame::write;
using namespace apache::thrift::fast_thrift::transport::bench;

namespace {

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
// BenchAppAdapter — exposes addMethodHandler so the bench can register
// no-op method handlers without going through codegen.
// =============================================================================

class BenchAppAdapter : public thrift::ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<BenchAppAdapter, Destructor>;

  void registerNoOp(std::string_view name) {
    addMethodHandler(
        name,
        +[](thrift::ThriftServerAppAdapter*,
            uint32_t,
            std::unique_ptr<folly::IOBuf>,
            apache::thrift::ProtocolId,
            std::unique_ptr<thrift::ThriftRequestContext>) noexcept -> Result {
          return Result::Success;
        });
  }
};

// =============================================================================
// Frame helpers
// =============================================================================

std::unique_ptr<folly::IOBuf> serializeRequestMetadata(
    const apache::thrift::RequestRpcMetadata& metadata) {
  apache::thrift::BinaryProtocolWriter writer;
  folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  writer.setOutput(&queue);
  metadata.write(&writer);
  return queue.move();
}

apache::thrift::RequestRpcMetadata createMinimalRequestMetadata(
    const std::string& methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
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
// Rocket pipeline (shared between fixtures)
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

// =============================================================================
// Baseline fixture: bare ThriftServerAppAdapter as thrift pipeline tail
// (no composite layer).
// =============================================================================

struct BareAdapterFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  RocketPipelineResources rocketResources;

  std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
      transportAdapter;
  BenchAppAdapter::Ptr adapter;
  PipelineImpl::Ptr thriftPipeline;
  SimpleBufferAllocator thriftAllocator;

  void setup() {
    testTransport = rocketResources.setup(&evb);

    adapter.reset(new BenchAppAdapter());
    adapter->registerNoOp("benchMethod");

    transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            *rocketResources.appAdapter);

    thriftPipeline = PipelineBuilder<
                         thrift::server::ThriftServerTransportAdapter,
                         BenchAppAdapter,
                         SimpleBufferAllocator>()
                         .setEventBase(&evb)
                         .setHead(transportAdapter.get())
                         .setTail(adapter.get())
                         .setAllocator(&thriftAllocator)
                         .build();

    transportAdapter->setPipeline(thriftPipeline.get());
    adapter->setPipeline(thriftPipeline.get());
    thriftPipeline->activate();

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
// Composite fixture: ThriftServerCompositeAppAdapter as thrift pipeline
// tail, wrapping N children each owning a single registered method.
// =============================================================================

struct CompositeFixture {
  folly::EventBase evb;
  BenchAsyncTransport* testTransport{nullptr};
  RocketPipelineResources rocketResources;

  std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
      transportAdapter;
  std::vector<BenchAppAdapter::Ptr> children;
  thrift::ThriftServerCompositeAppAdapter::Ptr composite;
  PipelineImpl::Ptr thriftPipeline;
  SimpleBufferAllocator thriftAllocator;

  // Builds N children with method names "benchMethod0".."benchMethod{N-1}".
  void setup(size_t numChildren) {
    testTransport = rocketResources.setup(&evb);

    for (size_t i = 0; i < numChildren; ++i) {
      BenchAppAdapter::Ptr child{new BenchAppAdapter()};
      child->registerNoOp("benchMethod" + std::to_string(i));
      children.push_back(std::move(child));
    }

    composite.reset(new thrift::ThriftServerCompositeAppAdapter());
    for (auto& child : children) {
      composite->addChild(child.get());
    }

    transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            *rocketResources.appAdapter);

    thriftPipeline = PipelineBuilder<
                         thrift::server::ThriftServerTransportAdapter,
                         thrift::ThriftServerCompositeAppAdapter,
                         SimpleBufferAllocator>()
                         .setEventBase(&evb)
                         .setHead(transportAdapter.get())
                         .setTail(composite.get())
                         .setAllocator(&thriftAllocator)
                         .build();

    transportAdapter->setPipeline(thriftPipeline.get());
    // composite's setPipeline fans out to children.
    composite->setPipeline(thriftPipeline.get());
    thriftPipeline->activate();

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

} // namespace

// =============================================================================
// Bare-adapter baseline — establishes the full-pipeline floor.
// =============================================================================

BENCHMARK(BareAdapter_Request, iters) {
  folly::BenchmarkSuspender suspender;
  BareAdapterFixture fixture;
  fixture.setup();

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata("benchMethod"));
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

// =============================================================================
// Composite with one child — measures the composite layer's added cost when
// no fan-out is needed (single child, single method).
// =============================================================================

BENCHMARK_RELATIVE(Composite_OneChild_Request, iters) {
  folly::BenchmarkSuspender suspender;
  CompositeFixture fixture;
  fixture.setup(/*numChildren=*/1);

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata("benchMethod0"));
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

// =============================================================================
// Composite with four children, hitting the last child — measures fan-out
// + methodMap lookup cost at integration level. F14 is O(1) so first-vs-last
// difference is negligible in microbench; "last" is the worst case.
// =============================================================================

BENCHMARK_RELATIVE(Composite_FourChildren_Request, iters) {
  folly::BenchmarkSuspender suspender;
  CompositeFixture fixture;
  fixture.setup(/*numChildren=*/4);

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata("benchMethod3"));
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

// =============================================================================
// Composite, unknown method — measures the wire-level cost of the
// framework-error fire path. Each request triggers a ResponseRpcError
// write at the thrift tail; ceiling for the failure path.
// =============================================================================

BENCHMARK_RELATIVE(Composite_UnknownMethod_Request, iters) {
  folly::BenchmarkSuspender suspender;
  CompositeFixture fixture;
  fixture.setup(/*numChildren=*/4);

  auto metadataTemplate =
      serializeRequestMetadata(createMinimalRequestMetadata("method.nobody"));
  auto payloadTemplate = makePayloadData(kPayloadSize);

  std::vector<std::unique_ptr<folly::IOBuf>> requests;
  requests.reserve(iters);
  for (uint32_t i = 0; i < iters; ++i) {
    requests.push_back(createRequestFrame(
        i * 2 + 1, metadataTemplate->clone(), payloadTemplate->clone()));
  }

  suspender.dismiss();

  for (uint32_t i = 0; i < iters; ++i) {
    fixture.injectFrame(std::move(requests[i]));
    fixture.evb.loopOnce();
  }
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
