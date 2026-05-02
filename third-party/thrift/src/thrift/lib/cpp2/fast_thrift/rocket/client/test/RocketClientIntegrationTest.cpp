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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>

namespace apache::thrift::fast_thrift::rocket::client::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::test::TestAllocator;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

// Bring message types into scope
using rocket::kInvalidStreamId;
using rocket::kNoRequestHandle;
using rocket::RocketRequestMessage;
using rocket::RocketResponseMessage;
using ParsedFrame = apache::thrift::fast_thrift::frame::read::ParsedFrame;

// Bring handler types into scope
using rocket::client::handler::RocketClientErrorFrameHandler;
using rocket::client::handler::RocketClientFrameCodecHandler;
using rocket::client::handler::RocketClientRequestResponseHandler;
using rocket::client::handler::RocketClientSetupFrameHandler;
using rocket::client::handler::RocketClientStreamStateHandler;

// RSocket error codes
constexpr uint32_t kErrorCodeApplicationError = 0x00000201;
constexpr uint32_t kErrorCodeRejected = 0x00000202;
constexpr uint32_t kErrorCodeCanceled = 0x00000203;
constexpr uint32_t kErrorCodeInvalid = 0x00000204;

// Handler tags for pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(error_injection_inbound_handler);

using channel_pipeline::TypeErasedBox;

// Test-only handler: replaces the ParsedFrame payload of any inbound
// RocketResponseMessage whose streamId is in failStreamIds with a
// RocketResponseError carrying failWith. Simulates the codec's outbound
// serialize-throw path, which is structurally hard to trigger directly.
class ErrorInjectionInboundHandler {
 public:
  ErrorInjectionInboundHandler(
      uint32_t failStreamId, folly::exception_wrapper failWith)
      : failStreamId_(failStreamId), failWith_(std::move(failWith)) {}

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();
    if (response.payload.is<ParsedFrame>()) {
      uint32_t sid = response.payload.get<ParsedFrame>().streamId();
      if (sid == failStreamId_) {
        response.payload = rocket::RocketResponseError{
            .ew = folly::exception_wrapper(failWith_),
            .streamId = sid,
        };
      }
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  uint32_t failStreamId_;
  folly::exception_wrapper failWith_;
};

class RocketClientIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override { allocator_.reset(); }

  void TearDown() override {
    pipeline_.reset();
    if (transportHandler_) {
      transportHandler_->onClose(folly::exception_wrapper{});
    }
    testTransport_ = nullptr;
  }

  /**
   * Set up the rocket-only pipeline with TestAsyncTransport.
   * No thrift handlers - just the rocket client handlers.
   * The test treats the adapter as a black box: writes requests to
   * appAdapter_ and validates responses via callbacks.
   */
  void setupPipeline() {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    transportHandler_ =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    appAdapter_->setResponseHandlers(
        [this](TypeErasedBox&& msg) noexcept -> Result {
          responseCount_++;
          responses_.push_back(std::move(msg));
          return Result::Success;
        },
        [this](folly::exception_wrapper&& e) noexcept {
          exceptionReceived_ = true;
          exception_ = std::move(e);
        });

    pipeline_ = PipelineBuilder<
                    apache::thrift::fast_thrift::transport::TransportHandler,
                    rocket::client::RocketClientAppAdapter,
                    TestAllocator>()
                    .setEventBase(&evb_)
                    .setHead(transportHandler_.get())
                    .setTail(appAdapter_.get())
                    .setAllocator(&allocator_)
                    .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                        handler::FrameLengthParserHandler>(
                        frame_length_parser_handler_tag)
                    .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                         handler::FrameLengthEncoderHandler>(
                        frame_length_encoder_handler_tag)
                    .addNextDuplex<RocketClientFrameCodecHandler>(
                        rocket_client_frame_codec_handler_tag)
                    .addNextDuplex<RocketClientSetupFrameHandler>(
                        rocket_client_setup_handler_tag,
                        []() {
                          return std::make_pair(
                              folly::IOBuf::copyBuffer("setup"),
                              std::unique_ptr<folly::IOBuf>());
                        })
                    .addNextInbound<RocketClientErrorFrameHandler>(
                        rocket_client_error_frame_handler_tag)
                    .addNextDuplex<RocketClientStreamStateHandler>(
                        rocket_client_stream_state_handler_tag)
                    .addNextInbound<RocketClientRequestResponseHandler>(
                        rocket_client_request_response_handler_tag)
                    .build();

    appAdapter_->setPipeline(pipeline_.get());
    transportHandler_->setPipeline(*pipeline_);
    transportHandler_->onConnect();

    evb_.loopOnce();

    discardSetupFrame();
  }

  /**
   * Same as setupPipeline() but inserts an ErrorInjectionInboundHandler
   * between the codec and the connection-level error handler. Inbound
   * RocketResponseMessages with a ParsedFrame payload whose streamId
   * matches failStreamId get rewritten to a RocketResponseError carrying
   * `failWith`. Used to exercise the StreamStateHandler cold-path
   * end-to-end without depending on a real serialize() throw.
   */
  void setupPipelineWithErrorInjection(
      uint32_t failStreamId, folly::exception_wrapper failWith) {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    transportHandler_ =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(transport));

    appAdapter_->setResponseHandlers(
        [this](TypeErasedBox&& msg) noexcept -> Result {
          responseCount_++;
          responses_.push_back(std::move(msg));
          return Result::Success;
        },
        [this](folly::exception_wrapper&& e) noexcept {
          exceptionReceived_ = true;
          exception_ = std::move(e);
        });

    pipeline_ = PipelineBuilder<
                    apache::thrift::fast_thrift::transport::TransportHandler,
                    rocket::client::RocketClientAppAdapter,
                    TestAllocator>()
                    .setEventBase(&evb_)
                    .setHead(transportHandler_.get())
                    .setTail(appAdapter_.get())
                    .setAllocator(&allocator_)
                    .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                        handler::FrameLengthParserHandler>(
                        frame_length_parser_handler_tag)
                    .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                         handler::FrameLengthEncoderHandler>(
                        frame_length_encoder_handler_tag)
                    .addNextDuplex<RocketClientFrameCodecHandler>(
                        rocket_client_frame_codec_handler_tag)
                    .addNextDuplex<RocketClientSetupFrameHandler>(
                        rocket_client_setup_handler_tag,
                        []() {
                          return std::make_pair(
                              folly::IOBuf::copyBuffer("setup"),
                              std::unique_ptr<folly::IOBuf>());
                        })
                    .addNextInbound<ErrorInjectionInboundHandler>(
                        error_injection_inbound_handler_tag,
                        failStreamId,
                        std::move(failWith))
                    .addNextInbound<RocketClientErrorFrameHandler>(
                        rocket_client_error_frame_handler_tag)
                    .addNextDuplex<RocketClientStreamStateHandler>(
                        rocket_client_stream_state_handler_tag)
                    .addNextInbound<RocketClientRequestResponseHandler>(
                        rocket_client_request_response_handler_tag)
                    .build();

    appAdapter_->setPipeline(pipeline_.get());
    transportHandler_->setPipeline(*pipeline_);
    transportHandler_->onConnect();

    evb_.loopOnce();

    discardSetupFrame();
  }

  void discardSetupFrame() {
    auto setupFrame = testTransport_->getWrittenData();
    ASSERT_NE(setupFrame, nullptr) << "Expected SETUP frame on connect";
  }

  std::unique_ptr<folly::IOBuf> getWrittenFrame() {
    return testTransport_->getWrittenData();
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

  apache::thrift::fast_thrift::frame::read::ParsedFrame parseWrittenFrame(
      std::unique_ptr<folly::IOBuf> data) {
    folly::IOBufQueue queue;
    queue.append(std::move(data));
    queue.trimStart(apache::thrift::fast_thrift::frame::kMetadataLengthSize);
    return apache::thrift::fast_thrift::frame::read::parseFrame(queue.move());
  }

  std::unique_ptr<folly::IOBuf> createPayloadResponse(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> metadata,
      std::unique_ptr<folly::IOBuf> data) {
    return apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = streamId, .complete = true, .next = true},
        std::move(metadata),
        std::move(data));
  }

  RocketRequestMessage createRocketRequest(
      std::unique_ptr<folly::IOBuf> data,
      std::unique_ptr<folly::IOBuf> metadata = nullptr) {
    return RocketRequestMessage{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
                .data = std::move(data),
                .metadata = std::move(metadata),
                .header = {.streamId = rocket::kInvalidStreamId},
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE};
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
  rocket::client::RocketClientAppAdapter::Ptr appAdapter_{
      new rocket::client::RocketClientAppAdapter()};
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      transportHandler_;
  PipelineImpl::Ptr pipeline_;
  TestAllocator allocator_;

  // Response tracking
  int responseCount_{0};
  std::vector<TypeErasedBox> responses_;
  bool exceptionReceived_{false};
  folly::exception_wrapper exception_;
};

// =============================================================================
// Request Path Tests - Verify outbound flow
// =============================================================================

TEST_F(RocketClientIntegrationTest, RequestFlowsThroughPipelineToSocket) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test payload"));
  (void)appAdapter_->write(std::move(request));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr) << "Expected request frame to be written";

  auto parsed = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsed.isValid()) << "Failed to parse request frame";
  EXPECT_EQ(
      parsed.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(parsed.streamId(), 1u) << "First request should have stream ID 1";
  EXPECT_GT(parsed.dataSize(), 0u);
}

TEST_F(RocketClientIntegrationTest, MultipleRequestsGetDistinctStreamIds) {
  setupPipeline();

  auto request1 = createRocketRequest(folly::IOBuf::copyBuffer("payload1"));
  (void)appAdapter_->write(std::move(request1));
  evb_.loopOnce();
  auto frame1 = getWrittenFrame();
  ASSERT_NE(frame1, nullptr);
  auto parsed1 = parseWrittenFrame(std::move(frame1));
  ASSERT_TRUE(parsed1.isValid());
  EXPECT_EQ(parsed1.streamId(), 1u);

  auto request2 = createRocketRequest(folly::IOBuf::copyBuffer("payload2"));
  (void)appAdapter_->write(std::move(request2));
  evb_.loopOnce();
  auto frame2 = getWrittenFrame();
  ASSERT_NE(frame2, nullptr);
  auto parsed2 = parseWrittenFrame(std::move(frame2));
  ASSERT_TRUE(parsed2.isValid());
  EXPECT_EQ(parsed2.streamId(), 3u) << "Second request should have stream ID 3";
}

TEST_F(RocketClientIntegrationTest, RequestWithMetadataAndData) {
  setupPipeline();

  auto request = createRocketRequest(
      folly::IOBuf::copyBuffer("data"), folly::IOBuf::copyBuffer("metadata"));
  (void)appAdapter_->write(std::move(request));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr);

  auto parsed = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_GT(parsed.metadataSize(), 0u);
  EXPECT_GT(parsed.dataSize(), 0u);
}

// =============================================================================
// Response Path Tests - Verify inbound flow
// =============================================================================

TEST_F(RocketClientIntegrationTest, ResponseFlowsFromSocketThroughPipeline) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr);
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsedRequest.isValid());
  uint32_t streamId = parsedRequest.streamId();
  EXPECT_EQ(streamId, 1u);

  auto responseData = folly::IOBuf::copyBuffer("response payload");
  auto responseFrame =
      createPayloadResponse(streamId, nullptr, std::move(responseData));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1) << "App adapter should have received response";
  EXPECT_FALSE(exceptionReceived_);

  auto& response = responses_[0].get<RocketResponseMessage>();
  auto& parsed = response.payload.get<ParsedFrame>();
  EXPECT_EQ(parsed.streamId(), streamId);
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
}

TEST_F(
    RocketClientIntegrationTest,
    ResponseRoutedToCorrectCallbackWithMultiplePendingRequests) {
  setupPipeline();

  auto request1 = createRocketRequest(folly::IOBuf::copyBuffer("req1"));
  (void)appAdapter_->write(std::move(request1));
  evb_.loopOnce();
  auto frame1 = getWrittenFrame();
  auto parsed1 = parseWrittenFrame(std::move(frame1));
  uint32_t streamId1 = parsed1.streamId();

  auto request2 = createRocketRequest(folly::IOBuf::copyBuffer("req2"));
  (void)appAdapter_->write(std::move(request2));
  evb_.loopOnce();
  auto frame2 = getWrittenFrame();
  auto parsed2 = parseWrittenFrame(std::move(frame2));
  uint32_t streamId2 = parsed2.streamId();

  auto response2 = createPayloadResponse(
      streamId2, nullptr, folly::IOBuf::copyBuffer("response2"));
  injectFrame(std::move(response2));
  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& resp = responses_[0].get<RocketResponseMessage>();
  EXPECT_EQ(resp.payload.get<ParsedFrame>().streamId(), streamId2);

  auto response1 = createPayloadResponse(
      streamId1, nullptr, folly::IOBuf::copyBuffer("response1"));
  injectFrame(std::move(response1));
  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 2);
}

// =============================================================================
// In-Process Per-Request Error E2E (StreamStateHandler cold path)
// =============================================================================

TEST_F(
    RocketClientIntegrationTest,
    PerRequestErrorReachesAppAndConnectionSurvives) {
  // Fail responses for streamId 1 (the first request); subsequent requests
  // get streamId 3+ which pass through normally.
  setupPipelineWithErrorInjection(
      /*failStreamId=*/1,
      folly::make_exception_wrapper<std::runtime_error>(
          "simulated serialize boom"));

  // First request: registers streamId 1, response will be rewritten to
  // RocketResponseError by the injection handler.
  auto request1 = createRocketRequest(folly::IOBuf::copyBuffer("req1"));
  (void)appAdapter_->write(std::move(request1));
  evb_.loopOnce();
  auto requestFrame1 = getWrittenFrame();
  ASSERT_NE(requestFrame1, nullptr);
  auto parsedRequest1 = parseWrittenFrame(std::move(requestFrame1));
  ASSERT_EQ(parsedRequest1.streamId(), 1u);

  // Inject any payload frame for streamId 1 — the injection handler will
  // rewrite its payload into a RocketResponseError before it reaches
  // StreamStateHandler.
  auto responseFrame1 = createPayloadResponse(
      1u, nullptr, folly::IOBuf::copyBuffer("ignored payload"));
  injectFrame(std::move(responseFrame1));
  evb_.loopOnce();
  evb_.loopOnce();

  // App receives one response carrying RocketResponseError; no
  // connection-level exception fires.
  ASSERT_EQ(responseCount_, 1);
  EXPECT_FALSE(exceptionReceived_);
  auto& errResponse = responses_[0].get<RocketResponseMessage>();
  ASSERT_TRUE(errResponse.payload.is<rocket::RocketResponseError>());
  auto& err = errResponse.payload.get<rocket::RocketResponseError>();
  EXPECT_EQ(err.streamId, 1u);
  EXPECT_EQ(
      err.ew.what().toStdString(),
      "std::runtime_error: simulated serialize boom");

  // Second request: streamId 3 doesn't match failStreamId; normal response
  // flows through untouched. This is the "connection survives" assertion.
  auto request2 = createRocketRequest(folly::IOBuf::copyBuffer("req2"));
  (void)appAdapter_->write(std::move(request2));
  evb_.loopOnce();
  auto requestFrame2 = getWrittenFrame();
  ASSERT_NE(requestFrame2, nullptr);
  auto parsedRequest2 = parseWrittenFrame(std::move(requestFrame2));
  ASSERT_EQ(parsedRequest2.streamId(), 3u);

  auto responseFrame2 = createPayloadResponse(
      3u, nullptr, folly::IOBuf::copyBuffer("real response"));
  injectFrame(std::move(responseFrame2));
  evb_.loopOnce();
  evb_.loopOnce();

  ASSERT_EQ(responseCount_, 2);
  EXPECT_FALSE(exceptionReceived_);
  auto& okResponse = responses_[1].get<RocketResponseMessage>();
  ASSERT_TRUE(okResponse.payload.is<ParsedFrame>());
  auto& parsedOk = okResponse.payload.get<ParsedFrame>();
  EXPECT_EQ(parsedOk.streamId(), 3u);
  EXPECT_EQ(
      parsedOk.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
}

// =============================================================================
// Error Response Tests
// =============================================================================

TEST_F(RocketClientIntegrationTest, ErrorFrameWithApplicationErrorCode) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));
  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeApplicationError},
      nullptr,
      folly::IOBuf::copyBuffer("Application error occurred"));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& response = responses_[0].get<RocketResponseMessage>();
  auto& parsed = response.payload.get<ParsedFrame>();
  EXPECT_EQ(parsed.streamId(), streamId);
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

TEST_F(RocketClientIntegrationTest, ErrorFrameWithRejectedCode) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));
  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeRejected},
      nullptr,
      folly::IOBuf::copyBuffer("Request rejected"));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& response = responses_[0].get<RocketResponseMessage>();
  EXPECT_EQ(
      response.payload.get<ParsedFrame>().type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

TEST_F(RocketClientIntegrationTest, ErrorFrameWithCanceledCode) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));
  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeCanceled},
      nullptr,
      folly::IOBuf::copyBuffer("Request canceled"));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& response = responses_[0].get<RocketResponseMessage>();
  EXPECT_EQ(
      response.payload.get<ParsedFrame>().type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

TEST_F(RocketClientIntegrationTest, ErrorFrameWithInvalidCode) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));
  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = kErrorCodeInvalid},
      nullptr,
      folly::IOBuf::copyBuffer("Invalid request"));

  injectFrame(std::move(errorFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& response = responses_[0].get<RocketResponseMessage>();
  EXPECT_EQ(
      response.payload.get<ParsedFrame>().type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(RocketClientIntegrationTest, ResponseWithEmptyPayload) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));
  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto responseFrame = createPayloadResponse(streamId, nullptr, nullptr);

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& response = responses_[0].get<RocketResponseMessage>();
  auto& parsed = response.payload.get<ParsedFrame>();
  EXPECT_EQ(parsed.streamId(), streamId);
  EXPECT_EQ(
      parsed.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
}

TEST_F(RocketClientIntegrationTest, LargePayloadRequest) {
  setupPipeline();

  constexpr size_t dataSize = 64 * 1024; // 64KB
  auto largeData = folly::IOBuf::create(dataSize);
  std::memset(largeData->writableData(), 'X', dataSize);
  largeData->append(dataSize);

  auto request = createRocketRequest(std::move(largeData));
  (void)appAdapter_->write(std::move(request));

  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  ASSERT_NE(requestFrame, nullptr);

  auto parsed = parseWrittenFrame(std::move(requestFrame));
  ASSERT_TRUE(parsed.isValid());
  EXPECT_EQ(
      parsed.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_GE(parsed.dataSize(), dataSize);
}

TEST_F(RocketClientIntegrationTest, ResponseWithMetadata) {
  setupPipeline();

  auto request = createRocketRequest(folly::IOBuf::copyBuffer("test"));
  (void)appAdapter_->write(std::move(request));
  evb_.loopOnce();

  auto requestFrame = getWrittenFrame();
  auto parsedRequest = parseWrittenFrame(std::move(requestFrame));
  uint32_t streamId = parsedRequest.streamId();

  auto responseFrame = createPayloadResponse(
      streamId,
      folly::IOBuf::copyBuffer("response metadata"),
      folly::IOBuf::copyBuffer("response data"));

  injectFrame(std::move(responseFrame));

  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 1);
  auto& response = responses_[0].get<RocketResponseMessage>();
  auto& parsed = response.payload.get<ParsedFrame>();
  EXPECT_EQ(parsed.streamId(), streamId);
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_GT(parsed.metadataSize(), 0u);
  EXPECT_GT(parsed.dataSize(), 0u);
}

} // namespace apache::thrift::fast_thrift::rocket::client::test
