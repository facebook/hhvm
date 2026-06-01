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
 * RocketServer Integration Tests
 *
 * Exercises the full server-side pipeline end-to-end:
 *   Transport -> FrameLengthParser -> FrameLengthEncoder -> FrameCodec ->
 *   Setup -> RequestResponse -> StreamState -> App
 *
 * Uses TestAsyncTransport to inject client frames and capture server responses.
 *
 * Note: TestAsyncTransport defers writeSuccess() via evb_->runInLoop(),
 * so TransportHandler::onMessage() returns Backpressure (not Success).
 * Response tests use (void) on send() results (matching client test pattern)
 * and call evb_.loopOnce() to drain the deferred callback.
 *
 * Setup error tests (invalid version, request-before-setup) are covered
 * in handler unit tests. They cannot be tested here because the deferred
 * writeSuccess() callback fires after closeInternal() nullifies the pipeline
 * pointer in TransportHandler.
 */

#include <gtest/gtest.h>

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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>

namespace apache::thrift::fast_thrift::rocket::server::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

using rocket::server::RocketRequestMessage;
using rocket::server::RocketResponseMessage;

using rocket::server::handler::RocketServerFrameCodecHandler;
using rocket::server::handler::RocketServerRequestResponseFrameHandler;
using rocket::server::handler::RocketServerSetupFrameHandler;
using rocket::server::handler::RocketServerStreamStateHandler;

HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(rocket_server_setup_handler);
HANDLER_TAG(rocket_server_request_response_frame_handler);
HANDLER_TAG(rocket_server_stream_state_handler);

namespace {

/**
 * RocketServerAppAdapter - Application adapter for server integration tests.
 *
 * Receives RocketRequestMessages from the pipeline (inbound requests from
 * clients) and sends RocketResponseMessages back through the pipeline.
 */
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

  Result send(RocketResponseMessage&& msg) {
    if (!pipeline_) {
      return Result::Error;
    }
    return pipeline_->fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(msg)));
  }

  Result onMessage(TypeErasedBox&& msg) noexcept {
    requestCount_++;
    requests_.push_back(std::move(msg));
    return Result::Success;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    exceptionReceived_ = true;
    exception_ = std::move(e);
  }

  int requestCount() const { return requestCount_; }

  const std::vector<TypeErasedBox>& requests() const { return requests_; }

  bool exceptionReceived() const { return exceptionReceived_; }

  const folly::exception_wrapper& exception() const { return exception_; }

  void reset() {
    requestCount_ = 0;
    requests_.clear();
    exceptionReceived_ = false;
    exception_ = folly::exception_wrapper{};
  }

 private:
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  PipelineImpl::Ptr pipeline_;
  int requestCount_{0};
  std::vector<TypeErasedBox> requests_;
  bool exceptionReceived_{false};
  folly::exception_wrapper exception_;
};

} // namespace

class RocketServerIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override { allocator_.reset(); }

  void TearDown() override {
    // Drain any pending write callbacks before destroying the adapter.
    // TestAsyncTransport defers writeSuccess() via runInLoop(), so we
    // must process those callbacks while the pipeline is still alive.
    evb_.loopOnce();

    pipeline_.reset();
    appAdapter_.reset();
    testTransport_ = nullptr;
  }

  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    appAdapter_ =
        std::make_unique<RocketServerAppAdapter>(std::move(transport));

    pipeline_ = PipelineBuilder<
                    RocketServerAppAdapter,
                    apache::thrift::fast_thrift::transport::TransportHandler,
                    TestAllocator>()
                    .setEventBase(&evb_)
                    .setTail(appAdapter_->transportHandler())
                    .setHead(appAdapter_.get())
                    .setAllocator(&allocator_)
                    .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                        handler::FrameLengthParserHandler>(
                        frame_length_parser_handler_tag)
                    .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                         handler::FrameLengthEncoderHandler>(
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

    appAdapter_->setPipeline(std::move(pipeline_));
    appAdapter_->transportHandler()->onConnect();
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

  std::unique_ptr<folly::IOBuf> createRequestResponseFrame(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
            .streamId = streamId},
        std::move(metadata),
        std::move(data));
  }

  std::unique_ptr<folly::IOBuf> createCancelFrame(uint32_t streamId) {
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::CancelHeader{
            .streamId = streamId});
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
  std::unique_ptr<RocketServerAppAdapter> appAdapter_;
  PipelineImpl::Ptr pipeline_;
  TestAllocator allocator_;
};

// =============================================================================
// Setup Validation Tests
// =============================================================================

TEST_F(RocketServerIntegrationTest, SetupFrameAcceptedAndConsumed) {
  setupPipeline();

  injectSetupFrame();

  EXPECT_EQ(appAdapter_->requestCount(), 0)
      << "SETUP should be consumed by SetupHandler, not forwarded to app";
  EXPECT_FALSE(appAdapter_->exceptionReceived());
}

// =============================================================================
// Inbound Request Path Tests
// =============================================================================

TEST_F(RocketServerIntegrationTest, RequestFlowsThroughPipelineToApp) {
  setupPipelineWithSetup();

  auto request = createRequestResponseFrame(
      1, nullptr, folly::IOBuf::copyBuffer("test payload"));
  injectFrame(std::move(request));

  ASSERT_EQ(appAdapter_->requestCount(), 1)
      << "App should receive exactly one request";
  EXPECT_FALSE(appAdapter_->exceptionReceived());

  auto& req = appAdapter_->requests()[0].get<RocketRequestMessage>();
  EXPECT_EQ(req.streamId, 1u);
  EXPECT_EQ(
      req.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

TEST_F(RocketServerIntegrationTest, MultipleRequestsGetDistinctStreamIds) {
  setupPipelineWithSetup();

  auto request1 = createRequestResponseFrame(
      1, nullptr, folly::IOBuf::copyBuffer("payload1"));
  injectFrame(std::move(request1));

  auto request2 = createRequestResponseFrame(
      3, nullptr, folly::IOBuf::copyBuffer("payload2"));
  injectFrame(std::move(request2));

  ASSERT_EQ(appAdapter_->requestCount(), 2);

  auto& req1 = appAdapter_->requests()[0].get<RocketRequestMessage>();
  EXPECT_EQ(req1.streamId, 1u);

  auto& req2 = appAdapter_->requests()[1].get<RocketRequestMessage>();
  EXPECT_EQ(req2.streamId, 3u);
}

TEST_F(RocketServerIntegrationTest, RequestWithDataAndMetadata) {
  setupPipelineWithSetup();

  auto request = createRequestResponseFrame(
      1,
      folly::IOBuf::copyBuffer("metadata"),
      folly::IOBuf::copyBuffer("data"));
  injectFrame(std::move(request));

  ASSERT_EQ(appAdapter_->requestCount(), 1);

  auto& req = appAdapter_->requests()[0].get<RocketRequestMessage>();
  EXPECT_EQ(req.streamId, 1u);
  EXPECT_TRUE(req.frame.hasMetadata());
  EXPECT_GT(req.frame.metadataSize(), 0u);
  EXPECT_GT(req.frame.dataSize(), 0u);
}

// =============================================================================
// Outbound Response Path Tests
// =============================================================================

TEST_F(RocketServerIntegrationTest, ResponseFlowsFromAppToSocket) {
  setupPipelineWithSetup();

  auto request =
      createRequestResponseFrame(1, nullptr, folly::IOBuf::copyBuffer("test"));
  injectFrame(std::move(request));

  ASSERT_EQ(appAdapter_->requestCount(), 1);

  RocketResponseMessage response;
  response.streamId = 1;
  response.payload = folly::IOBuf::copyBuffer("response data");
  response.complete = true;
  // TestAsyncTransport defers writeSuccess(), so send() returns Backpressure.
  // Use (void) to ignore the result, matching client test pattern.
  (void)appAdapter_->send(std::move(response));

  // Drain the deferred writeSuccess() callback
  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr) << "Expected response to be written";

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 1u);
  EXPECT_GT(parsed.dataSize(), 0u);
}

TEST_F(RocketServerIntegrationTest, ErrorResponseFlowsFromAppToSocket) {
  setupPipelineWithSetup();

  auto request =
      createRequestResponseFrame(1, nullptr, folly::IOBuf::copyBuffer("test"));
  injectFrame(std::move(request));

  ASSERT_EQ(appAdapter_->requestCount(), 1);

  RocketResponseMessage response;
  response.streamId = 1;
  response.payload = folly::IOBuf::copyBuffer("error message");
  response.errorCode = 0x00000201; // APPLICATION_ERROR
  response.complete = true;
  (void)appAdapter_->send(std::move(response));

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr);

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);
}

TEST_F(RocketServerIntegrationTest, ResponseWithMetadata) {
  setupPipelineWithSetup();

  auto request =
      createRequestResponseFrame(1, nullptr, folly::IOBuf::copyBuffer("test"));
  injectFrame(std::move(request));

  RocketResponseMessage response;
  response.streamId = 1;
  response.payload = folly::IOBuf::copyBuffer("response data");
  response.metadata = folly::IOBuf::copyBuffer("response metadata");
  response.complete = true;
  (void)appAdapter_->send(std::move(response));

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr);

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_GT(parsed.metadataSize(), 0u);
  EXPECT_GT(parsed.dataSize(), 0u);
}

// =============================================================================
// Stream Lifecycle Tests
// =============================================================================

TEST_F(RocketServerIntegrationTest, CancelFromClientRemovesStream) {
  setupPipelineWithSetup();

  auto request =
      createRequestResponseFrame(1, nullptr, folly::IOBuf::copyBuffer("test"));
  injectFrame(std::move(request));

  ASSERT_EQ(appAdapter_->requestCount(), 1);

  appAdapter_->reset();

  auto cancel = createCancelFrame(1);
  injectFrame(std::move(cancel));

  EXPECT_EQ(appAdapter_->requestCount(), 1)
      << "App should receive CANCEL notification";
  auto& req = appAdapter_->requests()[0].get<RocketRequestMessage>();
  EXPECT_EQ(req.streamId, 1u);
  EXPECT_EQ(
      req.frame.type(), apache::thrift::fast_thrift::frame::FrameType::CANCEL);
}

TEST_F(RocketServerIntegrationTest, ResponseForCompletedStreamFails) {
  setupPipelineWithSetup();

  auto request =
      createRequestResponseFrame(1, nullptr, folly::IOBuf::copyBuffer("test"));
  injectFrame(std::move(request));

  RocketResponseMessage response;
  response.streamId = 1;
  response.payload = folly::IOBuf::copyBuffer("response");
  response.complete = true;
  (void)appAdapter_->send(std::move(response));

  evb_.loopOnce();
  testTransport_->getWrittenData(); // consume the response

  RocketResponseMessage duplicate;
  duplicate.streamId = 1;
  duplicate.payload = folly::IOBuf::copyBuffer("duplicate");
  duplicate.complete = true;
  auto result = appAdapter_->send(std::move(duplicate));
  EXPECT_EQ(result, Result::Error)
      << "Response for completed stream should fail";
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(RocketServerIntegrationTest, LargePayloadRequest) {
  setupPipelineWithSetup();

  constexpr size_t dataSize = 64 * 1024; // 64KB
  auto largeData = folly::IOBuf::create(dataSize);
  std::memset(largeData->writableData(), 'X', dataSize);
  largeData->append(dataSize);

  auto request = createRequestResponseFrame(1, nullptr, std::move(largeData));
  injectFrame(std::move(request));

  ASSERT_EQ(appAdapter_->requestCount(), 1);

  auto& req = appAdapter_->requests()[0].get<RocketRequestMessage>();
  EXPECT_EQ(req.streamId, 1u);
  EXPECT_GE(req.frame.dataSize(), dataSize);
}

TEST_F(RocketServerIntegrationTest, EmptyPayloadResponse) {
  setupPipelineWithSetup();

  auto request =
      createRequestResponseFrame(1, nullptr, folly::IOBuf::copyBuffer("test"));
  injectFrame(std::move(request));

  RocketResponseMessage response;
  response.streamId = 1;
  response.payload = nullptr;
  response.metadata = nullptr;
  response.complete = true;
  (void)appAdapter_->send(std::move(response));

  evb_.loopOnce();

  auto responseFrame = getWrittenFrame();
  ASSERT_NE(responseFrame, nullptr);

  auto parsed = parseWrittenFrame(std::move(responseFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 1u);
}

} // namespace apache::thrift::fast_thrift::rocket::server::test
