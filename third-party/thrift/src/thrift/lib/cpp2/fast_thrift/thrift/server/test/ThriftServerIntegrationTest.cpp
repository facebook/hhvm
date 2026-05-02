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
 * ThriftServer Integration Tests
 *
 * Exercises the full server-side two-pipeline architecture end-to-end:
 *
 *   Rocket pipeline:
 *   Transport -> FrameLengthParser -> FrameLengthEncoder -> FrameCodec ->
 *   Setup -> RequestResponse -> StreamState -> RocketServerAppAdapter
 *
 *   Thrift pipeline:
 *   ThriftServerTransportAdapter -> ThriftServerChannel -> AsyncProcessor
 *
 * Uses TestAsyncTransport to inject client frames containing serialized
 * RequestRpcMetadata and capture server response frames.
 *
 * Note: TestAsyncTransport defers writeSuccess() via evb_->runInLoop(),
 * so pipeline writes return Backpressure. Response tests use (void) on
 * send results and call evb_.loopOnce() to drain deferred callbacks.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/io/Cursor.h>
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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
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
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::server::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;
using apache::thrift::fast_thrift::rocket::server::RocketServerAppAdapter;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

namespace {

// =============================================================================
// Mock AsyncProcessor — captures incoming requests and sends replies
// =============================================================================

struct CapturedRequest {
  std::string methodName;
  apache::thrift::protocol::PROTOCOL_TYPES protocolId;
  apache::thrift::ResponseChannelRequest::UniquePtr request;
  std::unique_ptr<folly::IOBuf> data;
  bool isOneway{false};
};

class MockAsyncProcessor : public apache::thrift::AsyncProcessor {
 public:
  void processSerializedCompressedRequestWithMetadata(
      apache::thrift::ResponseChannelRequest::UniquePtr req,
      apache::thrift::SerializedCompressedRequest&& serializedRequest,
      const apache::thrift::AsyncProcessorFactory::MethodMetadata&,
      apache::thrift::protocol::PROTOCOL_TYPES protocolId,
      apache::thrift::Cpp2RequestContext* ctx,
      folly::EventBase*,
      apache::thrift::concurrency::ThreadManager*) override {
    CapturedRequest captured;
    captured.methodName = ctx->getMethodName();
    captured.protocolId = protocolId;
    captured.isOneway = req->isOneway();
    captured.data = std::move(serializedRequest).uncompress().buffer;
    captured.request = std::move(req);
    requests_.push_back(std::move(captured));
  }

  void processInteraction(apache::thrift::ServerRequest&&) override {}

  std::vector<CapturedRequest>& requests() { return requests_; }

  void sendReply(size_t idx, std::unique_ptr<folly::IOBuf> payload) {
    if (idx < requests_.size() && requests_[idx].request) {
      requests_[idx].request->sendReply(
          apache::thrift::ResponsePayload::create(std::move(payload)),
          nullptr,
          folly::none);
    }
  }

  void sendError(
      size_t idx, const std::string& message, std::string exCode = "0") {
    if (idx < requests_.size() && requests_[idx].request) {
      requests_[idx].request->sendErrorWrapped(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              apache::thrift::TApplicationException::INTERNAL_ERROR, message),
          std::move(exCode));
    }
  }

  void reset() { requests_.clear(); }

 private:
  std::vector<CapturedRequest> requests_;
};

class MockAsyncProcessorFactory : public apache::thrift::AsyncProcessorFactory {
 public:
  explicit MockAsyncProcessorFactory(
      std::shared_ptr<MockAsyncProcessor> processor)
      : processor_(std::move(processor)) {}

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::unique_ptr<apache::thrift::AsyncProcessor>(
        new ForwardingProcessor(processor_));
  }

  std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
      override {
    return {};
  }

  CreateMethodMetadataResult createMethodMetadata() override { return {}; }

 private:
  // Forwards all calls to the shared processor so tests can inspect it.
  class ForwardingProcessor : public apache::thrift::AsyncProcessor {
   public:
    explicit ForwardingProcessor(std::shared_ptr<MockAsyncProcessor> inner)
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
    std::shared_ptr<MockAsyncProcessor> inner_;
  };

  std::shared_ptr<MockAsyncProcessor> processor_;
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

apache::thrift::RequestRpcMetadata createMinimalRequestMetadata(
    const std::string& methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

apache::thrift::RequestRpcMetadata createOnewayRequestMetadata(
    const std::string& methodName) {
  apache::thrift::RequestRpcMetadata metadata;
  metadata.name() = methodName;
  metadata.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_NO_RESPONSE;
  metadata.protocol() = apache::thrift::ProtocolId::BINARY;
  return metadata;
}

} // namespace

class ThriftServerIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    rocketAllocator_.reset();
    thriftAllocator_.reset();
    processor_ = std::make_shared<MockAsyncProcessor>();
  }

  void TearDown() override {
    // Drain pending write callbacks before destroying
    evb_.loopOnce();

    serverChannel_.reset();
    thriftPipeline_.reset();
    transportAdapter_.reset();
    appAdapter_.reset();
    rocketPipeline_.reset();
    transportHandler_.reset();
    testTransport_ = nullptr;
  }

  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    transportHandler_ =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    auto processorFactory =
        std::make_shared<MockAsyncProcessorFactory>(processor_);
    serverChannel_ =
        std::make_shared<ThriftServerChannel>(std::move(processorFactory));

    appAdapter_.reset(new RocketServerAppAdapter());

    // 1. Build rocket pipeline: TransportHandler → ... → RocketServerAppAdapter
    rocketPipeline_ =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            RocketServerAppAdapter,
            TestAllocator>()
            .setEventBase(&evb_)
            .setHead(transportHandler_.get())
            .setTail(appAdapter_.get())
            .setAllocator(&rocketAllocator_)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerFrameCodecHandler>(
                rocket_server_frame_codec_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerSetupFrameHandler>(
                server_setup_frame_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerStreamStateHandler>(
                server_stream_state_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerRequestResponseHandler>(
                server_request_response_frame_handler_tag)
            .build();

    appAdapter_->setPipeline(rocketPipeline_.get());

    // 2. Build thrift pipeline: ThriftServerTransportAdapter →
    // ThriftServerChannel
    transportAdapter_ =
        std::make_unique<ThriftServerTransportAdapter>(*appAdapter_);

    thriftPipeline_ = PipelineBuilder<
                          ThriftServerTransportAdapter,
                          ThriftServerChannel,
                          TestAllocator>()
                          .setEventBase(&evb_)
                          .setHead(transportAdapter_.get())
                          .setTail(serverChannel_.get())
                          .setAllocator(&thriftAllocator_)
                          .build();

    transportAdapter_->setPipeline(thriftPipeline_.get());
    serverChannel_->setPipelineRef(*thriftPipeline_);
    serverChannel_->setWorker(apache::thrift::Cpp2Worker::createDummy(&evb_));

    transportHandler_->setPipeline(*rocketPipeline_);
    transportHandler_->onConnect();
  }

  void injectSetupFrame() {
    auto setupFrame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::SetupHeader{
            .majorVersion = 1,
            .minorVersion = 0,
            .keepaliveTime = 30000,
            .maxLifetime = 60000},
        nullptr,
        nullptr);
    injectFrame(std::move(setupFrame));
  }

  void setupPipelineWithSetup() {
    setupPipeline();
    injectSetupFrame();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    size_t frameLength = frame->computeChainDataLength();
    auto lengthPrefix = folly::IOBuf::create(
        apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    uint8_t* data = lengthPrefix->writableData();
    data[0] = static_cast<uint8_t>((frameLength >> 16) & 0xFF);
    data[1] = static_cast<uint8_t>((frameLength >> 8) & 0xFF);
    data[2] = static_cast<uint8_t>(frameLength & 0xFF);
    lengthPrefix->append(
        apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    lengthPrefix->appendChain(std::move(frame));
    testTransport_->injectReadData(std::move(lengthPrefix));
  }

  std::unique_ptr<folly::IOBuf> createRequestFrame(
      uint32_t streamId,
      const apache::thrift::RequestRpcMetadata& rpcMetadata,
      std::unique_ptr<folly::IOBuf> payloadData) {
    auto metadata = serializeRequestMetadata(rpcMetadata);
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
            .streamId = streamId},
        std::move(metadata),
        std::move(payloadData));
  }

  std::unique_ptr<folly::IOBuf> getWrittenFrame() {
    return testTransport_->getWrittenData();
  }

  apache::thrift::fast_thrift::frame::read::ParsedFrame parseWrittenFrame(
      std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufQueue queue;
    queue.append(std::move(data));
    queue.trimStart(apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    return apache::thrift::fast_thrift::frame::read::parseFrame(queue.move());
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  RocketServerAppAdapter::Ptr appAdapter_;
  std::unique_ptr<ThriftServerTransportAdapter> transportAdapter_;
  std::shared_ptr<ThriftServerChannel> serverChannel_;
  PipelineImpl::Ptr rocketPipeline_;
  PipelineImpl::Ptr thriftPipeline_;
  TestAllocator rocketAllocator_;
  TestAllocator thriftAllocator_;
  std::shared_ptr<MockAsyncProcessor> processor_;
};

// =============================================================================
// Request Dispatch Tests
// =============================================================================

TEST_F(ThriftServerIntegrationTest, RequestDispatchedToProcessor) {
  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("testMethod");
  auto request =
      createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("test payload"));
  injectFrame(std::move(request));

  ASSERT_EQ(processor_->requests().size(), 1u);
  EXPECT_EQ(processor_->requests()[0].methodName, "testMethod");
  EXPECT_EQ(
      processor_->requests()[0].protocolId,
      apache::thrift::protocol::T_BINARY_PROTOCOL);
  EXPECT_FALSE(processor_->requests()[0].isOneway);
}

TEST_F(ThriftServerIntegrationTest, MultipleRequestsDispatched) {
  setupPipelineWithSetup();

  auto rpc1 = createMinimalRequestMetadata("method1");
  injectFrame(
      createRequestFrame(1, rpc1, folly::IOBuf::copyBuffer("payload1")));

  auto rpc2 = createMinimalRequestMetadata("method2");
  injectFrame(
      createRequestFrame(3, rpc2, folly::IOBuf::copyBuffer("payload2")));

  auto rpc3 = createMinimalRequestMetadata("method3");
  injectFrame(
      createRequestFrame(5, rpc3, folly::IOBuf::copyBuffer("payload3")));

  ASSERT_EQ(processor_->requests().size(), 3u);
  EXPECT_EQ(processor_->requests()[0].methodName, "method1");
  EXPECT_EQ(processor_->requests()[1].methodName, "method2");
  EXPECT_EQ(processor_->requests()[2].methodName, "method3");
}

TEST_F(ThriftServerIntegrationTest, RequestWithPayloadData) {
  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("echo");
  auto request =
      createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("hello world"));
  injectFrame(std::move(request));

  ASSERT_EQ(processor_->requests().size(), 1u);
  auto& data = processor_->requests()[0].data;
  ASSERT_NE(data, nullptr);

  auto dataStr = data->moveToFbString().toStdString();
  EXPECT_EQ(dataStr, "hello world");
}

TEST_F(ThriftServerIntegrationTest, SetupFrameConsumedBeforeThriftLayer) {
  setupPipeline();

  injectSetupFrame();

  EXPECT_EQ(processor_->requests().size(), 0u)
      << "SETUP should be consumed by rocket SetupHandler, not dispatched";
}

TEST_F(ThriftServerIntegrationTest, OnewayRequestNoResponse) {
  setupPipelineWithSetup();

  auto rpcMeta = createOnewayRequestMetadata("onewayMethod");
  auto request =
      createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("fire"));
  injectFrame(std::move(request));

  ASSERT_EQ(processor_->requests().size(), 1u);
  EXPECT_EQ(processor_->requests()[0].methodName, "onewayMethod");
  EXPECT_TRUE(processor_->requests()[0].isOneway);
}

// =============================================================================
// Response Path Tests
// =============================================================================

TEST_F(ThriftServerIntegrationTest, ResponseFlowsFromProcessorToSocket) {
  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("testMethod");
  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));
  ASSERT_EQ(processor_->requests().size(), 1u);

  processor_->sendReply(0, folly::IOBuf::copyBuffer("response data"));
  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr) << "Expected response frame";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 1u);
  EXPECT_GT(parsed.dataSize(), 0u);
}

TEST_F(ThriftServerIntegrationTest, ThriftErrorProducesErrorFrame) {
  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("testMethod");
  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));
  ASSERT_EQ(processor_->requests().size(), 1u);

  // exCode "5" = kQueueOverloadedErrorCode → should produce ERROR frame
  processor_->sendError(0, "server overloaded", "5");
  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr) << "Expected error response frame";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);

  // Verify the error code is REJECTED (0x202) for loadshedding
  apache::thrift::fast_thrift::frame::read::ErrorView errorView(parsed);
  EXPECT_EQ(
      errorView.errorCode(),
      static_cast<uint32_t>(apache::thrift::rocket::ErrorCode::REJECTED));

  // Verify the payload contains a valid ResponseRpcError
  ASSERT_GT(parsed.dataSize(), 0u);
  auto dataBuf = std::move(parsed).extractData();
  ASSERT_NE(dataBuf, nullptr);
  apache::thrift::ResponseRpcError rpcError;
  apache::thrift::CompactProtocolReader reader;
  reader.setInput(dataBuf.get());
  rpcError.read(&reader);
  EXPECT_EQ(
      *rpcError.code(), apache::thrift::ResponseRpcErrorCode::QUEUE_OVERLOADED);
  EXPECT_THAT(*rpcError.what_utf8(), ::testing::HasSubstr("server overloaded"));
}

TEST_F(ThriftServerIntegrationTest, AppErrorProducesPayloadFrame) {
  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("testMethod");
  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));
  ASSERT_EQ(processor_->requests().size(), 1u);

  // exCode "23" = kAppClientErrorCode → no mapping → PAYLOAD frame
  processor_->sendError(0, "application error", "23");
  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr) << "Expected error response frame";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 1u);
  EXPECT_TRUE(parsed.hasMetadata());
}

TEST_F(ThriftServerIntegrationTest, BadMetadataProducesErrorFrame) {
  setupPipelineWithSetup();

  // Create a request with garbage metadata to trigger deserialization failure
  auto garbageMetadata = folly::IOBuf::copyBuffer("not valid thrift metadata");
  auto requestFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = 1},
      std::move(garbageMetadata),
      folly::IOBuf::copyBuffer("payload"));
  injectFrame(std::move(requestFrame));

  // Should not dispatch to processor
  EXPECT_EQ(processor_->requests().size(), 0u);

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr)
      << "Expected error response for bad metadata";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);

  apache::thrift::fast_thrift::frame::read::ErrorView errorView(parsed);
  EXPECT_EQ(
      errorView.errorCode(),
      static_cast<uint32_t>(apache::thrift::rocket::ErrorCode::INVALID));
}

TEST_F(ThriftServerIntegrationTest, UnsupportedRpcKindProducesErrorFrame) {
  setupPipelineWithSetup();

  apache::thrift::RequestRpcMetadata rpcMeta;
  rpcMeta.name() = "streamMethod";
  rpcMeta.kind() = apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE;
  rpcMeta.protocol() = apache::thrift::ProtocolId::BINARY;

  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));

  // Should not dispatch to processor
  EXPECT_EQ(processor_->requests().size(), 0u);

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr)
      << "Expected error response for unsupported RPC kind";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);

  apache::thrift::fast_thrift::frame::read::ErrorView errorView(parsed);
  EXPECT_EQ(
      errorView.errorCode(),
      static_cast<uint32_t>(apache::thrift::rocket::ErrorCode::INVALID));
}

TEST_F(ThriftServerIntegrationTest, MultipleResponsesForDifferentStreams) {
  setupPipelineWithSetup();

  auto rpc1 = createMinimalRequestMetadata("method1");
  injectFrame(createRequestFrame(1, rpc1, folly::IOBuf::copyBuffer("req1")));

  auto rpc2 = createMinimalRequestMetadata("method2");
  injectFrame(createRequestFrame(3, rpc2, folly::IOBuf::copyBuffer("req2")));

  ASSERT_EQ(processor_->requests().size(), 2u);

  // Send first reply, drain write callback, then read the frame
  processor_->sendReply(0, folly::IOBuf::copyBuffer("reply1"));
  evb_.loopOnce(); // process deferred writeSuccess
  auto frame1 = getWrittenFrame();
  ASSERT_NE(frame1, nullptr) << "Expected first response frame";
  auto parsed1 = parseWrittenFrame(std::move(frame1));
  EXPECT_EQ(parsed1.streamId(), 1u);

  evb_.loopOnce(); // restore write readiness for next write

  // Send second reply, drain write callback, then read the frame
  processor_->sendReply(1, folly::IOBuf::copyBuffer("reply2"));
  evb_.loopOnce(); // process deferred writeSuccess
  auto frame2 = getWrittenFrame();
  ASSERT_NE(frame2, nullptr) << "Expected second response frame";
  auto parsed2 = parseWrittenFrame(std::move(frame2));
  EXPECT_EQ(parsed2.streamId(), 3u);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test

// =============================================================================
// ThriftServerAppAdapter Integration Tests
// =============================================================================

namespace apache::thrift::fast_thrift::thrift::server::app_adapter_test {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);
HANDLER_TAG(rocket_thrift_interface_handler);

namespace {

/**
 * Inline handler that converts between rocket and thrift server message types.
 *
 * Read path:  RocketRequestMessage → ThriftServerRequestMessage
 * Write path: ThriftServerResponseMessage → RocketResponseMessage
 */
class RocketThriftServerInterfaceHandler {
 public:
  template <typename Context>
  void handlerAdded(Context&) noexcept {}

  template <typename Context>
  void handlerRemoved(Context&) noexcept {}

  template <typename Context>
  void onPipelineActive(Context&) noexcept {}

  template <typename Context>
  void onPipelineInactive(Context&) noexcept {}

  template <typename Context>
  void onWriteReady(Context&) noexcept {}

  template <typename Context>
  void onReadReady(Context&) noexcept {}

  template <typename Context>
  Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto rocketMsg = msg.take<
        apache::thrift::fast_thrift::rocket::server::RocketRequestMessage>();

    ThriftServerRequestMessage thriftMsg{
        .frame = std::move(rocketMsg.frame),
        .streamId = rocketMsg.streamId,
    };

    return ctx.fireRead(erase_and_box(std::move(thriftMsg)));
  }

  template <typename Context>
  Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto thriftMsg = msg.take<ThriftServerResponseMessage>();

    apache::thrift::fast_thrift::rocket::server::RocketResponseMessage
        rocketMsg;
    rocketMsg.streamType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    if (thriftMsg.errorCode != 0) {
      rocketMsg.frame = apache::thrift::fast_thrift::frame::ComposedErrorFrame{
          .data = std::move(thriftMsg.payload.data),
          .metadata = std::move(thriftMsg.payload.metadata),
          .header =
              {.streamId = thriftMsg.streamId,
               .errorCode = thriftMsg.errorCode},
      };
    } else {
      rocketMsg.frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = std::move(thriftMsg.payload.data),
              .metadata = std::move(thriftMsg.payload.metadata),
              .header =
                  {.streamId = thriftMsg.streamId,
                   .complete = thriftMsg.payload.complete,
                   .next = true},
          };
    }

    return ctx.fireWrite(erase_and_box(std::move(rocketMsg)));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
};

/**
 * Test subclass that exposes addMethodHandler for registration.
 */
class TestServerAppAdapter : public ThriftServerAppAdapter {
 public:
  using Ptr = std::unique_ptr<TestServerAppAdapter, Destructor>;

  void registerMethod(std::string_view name, ProcessFn handler) {
    addMethodHandler(name, handler);
  }

  bool handlerCalled{false};
  uint32_t capturedStreamId{0};
  apache::thrift::ProtocolId capturedProtocol{};
  int method1Count{0};
  int method2Count{0};
};

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

} // namespace

class ThriftServerAppAdapterIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    allocator_.reset();
    adapter_.reset(new TestServerAppAdapter());
  }

  void TearDown() override {
    evb_.loopOnce();

    pipeline_.reset();
    transportHandler_.reset();
    testTransport_ = nullptr;
  }

  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    transportHandler_ =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    pipeline_ =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            TestServerAppAdapter,
            TestAllocator>()
            .setEventBase(&evb_)
            .setHead(transportHandler_.get())
            .setTail(adapter_.get())
            .setAllocator(&allocator_)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerFrameCodecHandler>(
                rocket_server_frame_codec_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerSetupFrameHandler>(
                server_setup_frame_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerStreamStateHandler>(
                server_stream_state_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerRequestResponseHandler>(
                server_request_response_frame_handler_tag)
            .addNextDuplex<RocketThriftServerInterfaceHandler>(
                rocket_thrift_interface_handler_tag)
            .build();

    adapter_->setPipeline(pipeline_.get());
    transportHandler_->setPipeline(*pipeline_);
    transportHandler_->onConnect();
  }

  void injectSetupFrame() {
    auto setupFrame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::SetupHeader{
            .majorVersion = 1,
            .minorVersion = 0,
            .keepaliveTime = 30000,
            .maxLifetime = 60000},
        nullptr,
        nullptr);
    injectFrame(std::move(setupFrame));
  }

  void setupPipelineWithSetup() {
    setupPipeline();
    injectSetupFrame();
  }

  void injectFrame(std::unique_ptr<folly::IOBuf> frame) {
    size_t frameLength = frame->computeChainDataLength();
    auto lengthPrefix = folly::IOBuf::create(
        apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    uint8_t* data = lengthPrefix->writableData();
    data[0] = static_cast<uint8_t>((frameLength >> 16) & 0xFF);
    data[1] = static_cast<uint8_t>((frameLength >> 8) & 0xFF);
    data[2] = static_cast<uint8_t>(frameLength & 0xFF);
    lengthPrefix->append(
        apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    lengthPrefix->appendChain(std::move(frame));
    testTransport_->injectReadData(std::move(lengthPrefix));
  }

  std::unique_ptr<folly::IOBuf> createRequestFrame(
      uint32_t streamId,
      const apache::thrift::RequestRpcMetadata& rpcMetadata,
      std::unique_ptr<folly::IOBuf> payloadData) {
    auto metadata = serializeRequestMetadata(rpcMetadata);
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
            .streamId = streamId},
        std::move(metadata),
        std::move(payloadData));
  }

  std::unique_ptr<folly::IOBuf> getWrittenFrame() {
    return testTransport_->getWrittenData();
  }

  apache::thrift::fast_thrift::frame::read::ParsedFrame parseWrittenFrame(
      std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufQueue queue;
    queue.append(std::move(data));
    queue.trimStart(apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    return apache::thrift::fast_thrift::frame::read::parseFrame(queue.move());
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  TestServerAppAdapter::Ptr adapter_;
  PipelineImpl::Ptr pipeline_;
  TestAllocator allocator_;
};

// =============================================================================
// Request Dispatch Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterIntegrationTest,
    RequestDispatchedToRegisteredHandler) {
  adapter_->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&& request,
          apache::thrift::ProtocolId) noexcept -> Result {
        auto* t = static_cast<TestServerAppAdapter*>(self);
        t->handlerCalled = true;
        t->capturedStreamId = request.streamId;
        return Result::Success;
      });

  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("testMethod");
  auto request =
      createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("test payload"));
  injectFrame(std::move(request));

  EXPECT_TRUE(adapter_->handlerCalled);
  EXPECT_EQ(adapter_->capturedStreamId, 1u);
}

TEST_F(ThriftServerAppAdapterIntegrationTest, MultipleRequestsDispatched) {
  adapter_->registerMethod(
      "method1",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->method1Count++;
        return Result::Success;
      });

  adapter_->registerMethod(
      "method2",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->method2Count++;
        return Result::Success;
      });

  setupPipelineWithSetup();

  auto rpc1 = createMinimalRequestMetadata("method1");
  injectFrame(
      createRequestFrame(1, rpc1, folly::IOBuf::copyBuffer("payload1")));

  auto rpc2 = createMinimalRequestMetadata("method2");
  injectFrame(
      createRequestFrame(3, rpc2, folly::IOBuf::copyBuffer("payload2")));

  auto rpc3 = createMinimalRequestMetadata("method1");
  injectFrame(
      createRequestFrame(5, rpc3, folly::IOBuf::copyBuffer("payload3")));

  EXPECT_EQ(adapter_->method1Count, 2);
  EXPECT_EQ(adapter_->method2Count, 1);
}

TEST_F(ThriftServerAppAdapterIntegrationTest, UnknownMethodSendsErrorResponse) {
  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("unknownMethod");
  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr)
      << "Expected error response for unknown method";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(parsed.streamId(), 1u);
}

TEST_F(ThriftServerAppAdapterIntegrationTest, SetupFrameConsumed) {
  adapter_->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->handlerCalled = true;
        return Result::Success;
      });

  setupPipeline();
  injectSetupFrame();

  EXPECT_FALSE(adapter_->handlerCalled)
      << "SETUP should be consumed by rocket SetupHandler";
}

// =============================================================================
// Response Path Tests
// =============================================================================

TEST_F(
    ThriftServerAppAdapterIntegrationTest, ResponseFlowsFromHandlerToSocket) {
  adapter_->registerMethod(
      "echo",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&& request,
          apache::thrift::ProtocolId) noexcept -> Result {
        self->writeResponse(
            ThriftServerResponseMessage{
                .payload =
                    ThriftServerResponsePayload{
                        .data = folly::IOBuf::copyBuffer("echo response"),
                        .metadata = nullptr,
                        .complete = true},
                .streamId = request.streamId});
        return Result::Success;
      });

  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("echo");
  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr) << "Expected response frame";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 1u);
}

TEST_F(
    ThriftServerAppAdapterIntegrationTest,
    MultipleResponsesForDifferentStreams) {
  adapter_->registerMethod(
      "echo",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&& request,
          apache::thrift::ProtocolId) noexcept -> Result {
        self->writeResponse(
            ThriftServerResponseMessage{
                .payload =
                    ThriftServerResponsePayload{
                        .data = folly::IOBuf::copyBuffer("reply"),
                        .metadata = nullptr,
                        .complete = true},
                .streamId = request.streamId});
        return Result::Success;
      });

  setupPipelineWithSetup();

  auto rpc1 = createMinimalRequestMetadata("echo");
  injectFrame(createRequestFrame(1, rpc1, folly::IOBuf::copyBuffer("req1")));
  evb_.loopOnce();

  auto frame1 = getWrittenFrame();
  ASSERT_NE(frame1, nullptr) << "Expected first response frame";
  auto parsed1 = parseWrittenFrame(std::move(frame1));
  EXPECT_EQ(parsed1.streamId(), 1u);

  evb_.loopOnce();

  auto rpc2 = createMinimalRequestMetadata("echo");
  injectFrame(createRequestFrame(3, rpc2, folly::IOBuf::copyBuffer("req2")));
  evb_.loopOnce();

  auto frame2 = getWrittenFrame();
  ASSERT_NE(frame2, nullptr) << "Expected second response frame";
  auto parsed2 = parseWrittenFrame(std::move(frame2));
  EXPECT_EQ(parsed2.streamId(), 3u);
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(ThriftServerAppAdapterIntegrationTest, OnExceptionInvokesCloseCallback) {
  bool closeCalled = false;
  adapter_->setCloseCallback([&] { closeCalled = true; });

  setupPipelineWithSetup();

  evb_.runInEventBaseThread([&] {
    adapter_->onException(
        folly::make_exception_wrapper<std::runtime_error>("connection lost"));
  });
  evb_.loopOnce();

  EXPECT_TRUE(closeCalled);
}

TEST_F(ThriftServerAppAdapterIntegrationTest, ProtocolIdPassedToHandler) {
  adapter_->registerMethod(
      "testMethod",
      +[](ThriftServerAppAdapter* self,
          ThriftServerRequestMessage&&,
          apache::thrift::ProtocolId protocol) noexcept -> Result {
        static_cast<TestServerAppAdapter*>(self)->capturedProtocol = protocol;
        return Result::Success;
      });

  setupPipelineWithSetup();

  auto rpcMeta = createMinimalRequestMetadata("testMethod");
  injectFrame(createRequestFrame(1, rpcMeta, folly::IOBuf::copyBuffer("req")));

  EXPECT_EQ(adapter_->capturedProtocol, apache::thrift::ProtocolId::BINARY);
}

} // namespace apache::thrift::fast_thrift::thrift::server::app_adapter_test
