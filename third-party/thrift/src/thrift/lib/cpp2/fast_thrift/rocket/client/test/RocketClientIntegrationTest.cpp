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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
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
using rocket::RocketFramePayload;
using rocket::RocketRequestMessage;
using rocket::RocketResponseMessage;

// Bring handler types into scope
using rocket::client::handler::RocketClientErrorFrameHandler;
using rocket::client::handler::RocketClientFrameCodecHandler;
using rocket::client::handler::RocketClientRequestResponseFrameHandler;
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
HANDLER_TAG(rocket_client_request_response_frame_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);

using channel_pipeline::TypeErasedBox;

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
                    .addNextDuplex<RocketClientRequestResponseFrameHandler>(
                        rocket_client_request_response_frame_handler_tag)
                    .addNextInbound<RocketClientErrorFrameHandler>(
                        rocket_client_error_frame_handler_tag)
                    .addNextDuplex<RocketClientStreamStateHandler>(
                        rocket_client_stream_state_handler_tag)
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
            RocketFramePayload{
                .metadata = std::move(metadata),
                .data = std::move(data),
            },
        .frameType =
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
  EXPECT_EQ(response.frame.streamId(), streamId);
  EXPECT_EQ(
      response.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
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
  EXPECT_EQ(resp.frame.streamId(), streamId2);

  auto response1 = createPayloadResponse(
      streamId1, nullptr, folly::IOBuf::copyBuffer("response1"));
  injectFrame(std::move(response1));
  evb_.loopOnce();
  evb_.loopOnce();

  EXPECT_EQ(responseCount_, 2);
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
  EXPECT_EQ(response.frame.streamId(), streamId);
  EXPECT_EQ(
      response.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
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
      response.frame.type(),
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
      response.frame.type(),
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
      response.frame.type(),
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
  EXPECT_EQ(response.frame.streamId(), streamId);
  EXPECT_EQ(
      response.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
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
  EXPECT_EQ(response.frame.streamId(), streamId);
  EXPECT_TRUE(response.frame.hasMetadata());
  EXPECT_GT(response.frame.metadataSize(), 0u);
  EXPECT_GT(response.frame.dataSize(), 0u);
}

} // namespace apache::thrift::fast_thrift::rocket::client::test
