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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

/**
 * Helper to create a RocketRequestMessage with REQUEST_RESPONSE frame type.
 */
RocketRequestMessage makeStreamRequest(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<folly::IOBuf> metadata = nullptr) {
  return RocketRequestMessage{
      .frame =
          RocketFramePayload{
              .metadata = std::move(metadata),
              .data = std::move(data),
              .streamId = streamId,
          },
      .frameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE};
}

/**
 * Helper to get the serialized frame from the variant.
 */
std::unique_ptr<folly::IOBuf>& getSerializedFrame(RocketRequestMessage& msg) {
  return msg.frame.get<std::unique_ptr<folly::IOBuf>>();
}

/**
 * Helper to create a RocketResponseMessage with a parsed payload frame for
 * inbound testing.
 */
RocketResponseMessage makePayloadResponse(
    uint32_t streamId,
    bool complete,
    bool next,
    std::unique_ptr<folly::IOBuf> data = nullptr) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = streamId, .complete = complete, .next = next},
      nullptr,
      std::move(data));
  return RocketResponseMessage{
      .frame = apache::thrift::fast_thrift::frame::read::parseFrame(
          std::move(frame)),
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };
}

/**
 * Helper to read string content from a ParsedFrame's data section.
 */
std::string readFrameData(
    const apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) {
  auto cursor = frame.dataCursor();
  std::string data;
  data.resize(frame.dataSize());
  cursor.pull(data.data(), frame.dataSize());
  return data;
}

/**
 * MockContext for testing RocketClientRequestResponseFrameHandler.
 */
class MockContext {
 public:
  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    writeMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  void reset() {
    writeMessages_.clear();
    readMessages_.clear();
    exception_ = folly::exception_wrapper();
    returnBackpressure_ = false;
    returnError_ = false;
  }

 private:
  std::vector<TypeErasedBox> writeMessages_;
  std::vector<TypeErasedBox> readMessages_;
  folly::exception_wrapper exception_;
  bool returnBackpressure_{false};
  bool returnError_{false};
};

} // namespace

class ClientRequestResponseFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  /**
   * Helper to send a request and verify it's tracked.
   */
  void sendRequest(uint32_t streamId) {
    auto request =
        makeStreamRequest(streamId, folly::IOBuf::copyBuffer("request"));
    EXPECT_EQ(
        handler_.onWrite(ctx_, erase_and_box(std::move(request))),
        Result::Success);
    EXPECT_TRUE(handler_.hasPendingRequestResponse(streamId));
    ctx_.reset();
  }

  MockContext ctx_;
  RocketClientRequestResponseFrameHandler handler_;
};

// =============================================================================
// Outbound: Frame Serialization
// =============================================================================

TEST_F(ClientRequestResponseFrameHandlerTest, Write_DataOnly) {
  const std::string dataStr = "test data content";
  auto request = makeStreamRequest(42, folly::IOBuf::copyBuffer(dataStr));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      getSerializedFrame(msg)->cloneCoalesced());

  EXPECT_EQ(frame.streamId(), 42);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(frame).type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_FALSE(frame.hasMetadata());
  EXPECT_EQ(readFrameData(frame), dataStr);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Write_MetadataOnly) {
  const std::string metadataStr = "test metadata";
  auto request =
      makeStreamRequest(1, nullptr, folly::IOBuf::copyBuffer(metadataStr));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      getSerializedFrame(msg)->cloneCoalesced());

  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), metadataStr.size());
  EXPECT_EQ(frame.dataSize(), 0u);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Write_DataAndMetadata) {
  const std::string dataStr = "the data";
  const std::string metadataStr = "the metadata";
  auto request = makeStreamRequest(
      99,
      folly::IOBuf::copyBuffer(dataStr),
      folly::IOBuf::copyBuffer(metadataStr));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  auto serializedFrame = getSerializedFrame(msg)->cloneCoalesced();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      std::move(serializedFrame));

  EXPECT_EQ(frame.streamId(), 99);
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), metadataStr.size());
  EXPECT_EQ(readFrameData(frame), dataStr);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Write_EmptyPayload) {
  auto request = makeStreamRequest(1, nullptr, nullptr);

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  auto frame = apache::thrift::fast_thrift::frame::read::parseFrame(
      getSerializedFrame(msg)->cloneCoalesced());
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(frame).type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Write_LargePayload) {
  constexpr size_t dataSize = 1024 * 1024; // 1MB
  auto largeData = folly::IOBuf::create(dataSize);
  std::memset(largeData->writableData(), 'D', dataSize);
  largeData->append(dataSize);

  auto request = makeStreamRequest(1, std::move(largeData));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_GT(getSerializedFrame(msg)->computeChainDataLength(), dataSize);
}

TEST_F(
    ClientRequestResponseFrameHandlerTest, Write_NonRequestResponse_Forwards) {
  const std::vector<apache::thrift::fast_thrift::frame::FrameType> otherTypes =
      {
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_FNF,
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_CHANNEL,
      };

  for (auto frameType : otherTypes) {
    ctx_.reset();
    RocketRequestMessage request{
        .frame =
            RocketFramePayload{
                .metadata = {},
                .data = folly::IOBuf::copyBuffer("data"),
                .streamId = 1,
            },
        .frameType = frameType};

    EXPECT_EQ(
        handler_.onWrite(ctx_, erase_and_box(std::move(request))),
        Result::Success);
    ASSERT_EQ(ctx_.writeMessages().size(), 1);

    // Should forward unchanged as RocketRequestMessage
    auto& forwarded = ctx_.writeMessages()[0].get<RocketRequestMessage>();
    EXPECT_EQ(forwarded.frameType, frameType);
  }
}

// =============================================================================
// Outbound: Write Result Propagation
// =============================================================================

TEST_F(ClientRequestResponseFrameHandlerTest, Write_PropagatesBackpressure) {
  ctx_.setReturnBackpressure(true);
  auto request = makeStreamRequest(1, folly::IOBuf::copyBuffer("data"));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Backpressure);
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
}

TEST_F(ClientRequestResponseFrameHandlerTest, Write_ErrorClearsTracking) {
  ctx_.setReturnError(true);
  auto request = makeStreamRequest(1, folly::IOBuf::copyBuffer("data"));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))), Result::Error);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

// =============================================================================
// Inbound: Response Handling
// =============================================================================

TEST_F(ClientRequestResponseFrameHandlerTest, Read_PayloadWithNextAndComplete) {
  sendRequest(1);

  auto response =
      makePayloadResponse(1, true, true, folly::IOBuf::copyBuffer("response"));
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));

  // Verify output is RocketResponseMessage (not raw ParsedFrame)
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.streamId(), 1);
  EXPECT_EQ(
      forwarded.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Read_PayloadWithCompleteOnly) {
  sendRequest(1);

  auto response = makePayloadResponse(1, true, false);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

TEST_F(ClientRequestResponseFrameHandlerTest, Read_PayloadWithNextOnly) {
  sendRequest(1);

  auto response = makePayloadResponse(1, false, true);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));
}

TEST_F(
    ClientRequestResponseFrameHandlerTest, Read_PayloadNoFlags_ReturnsError) {
  sendRequest(1);

  auto response = makePayloadResponse(1, false, false);
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))), Result::Error);

  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Read_ErrorFrame) {
  sendRequest(1);

  auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = 1, .errorCode = 0x00000201},
      nullptr,
      folly::IOBuf::copyBuffer("error"));
  auto response = RocketResponseMessage{
      .frame = apache::thrift::fast_thrift::frame::read::parseFrame(
          std::move(errorFrame)),
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_FALSE(handler_.hasPendingRequestResponse(1));

  // Verify output is RocketResponseMessage (not raw ParsedFrame)
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.streamId(), 1);
  EXPECT_EQ(
      forwarded.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

TEST_F(
    ClientRequestResponseFrameHandlerTest,
    Read_UnexpectedFrameType_ReturnsError) {
  sendRequest(1);

  auto requestNFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestNHeader{
          .streamId = 1, .requestN = 5});
  auto response = RocketResponseMessage{
      .frame = apache::thrift::fast_thrift::frame::read::parseFrame(
          std::move(requestNFrame)),
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))), Result::Error);

  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Read_UnknownStream_Forwards) {
  // Frame for a stream we never registered (could be a stream frame)
  auto response = makePayloadResponse(99, true, true);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);

  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.streamId(), 99);
}

// =============================================================================
// Stream Tracking
// =============================================================================

TEST_F(ClientRequestResponseFrameHandlerTest, Tracking_MultipleStreams) {
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);

  sendRequest(1);
  sendRequest(3);
  sendRequest(5);

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 3);
  EXPECT_TRUE(handler_.hasPendingRequestResponse(1));
  EXPECT_TRUE(handler_.hasPendingRequestResponse(3));
  EXPECT_TRUE(handler_.hasPendingRequestResponse(5));
}

TEST_F(
    ClientRequestResponseFrameHandlerTest,
    Tracking_ClearedOnPipelineDeactivated) {
  sendRequest(1);
  sendRequest(3);
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 2);

  handler_.onPipelineInactive(ctx_);

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);
}

TEST_F(ClientRequestResponseFrameHandlerTest, Tracking_ClearedOnException) {
  sendRequest(1);
  sendRequest(3);
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 2);

  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("connection error");
  handler_.onException(ctx_, std::move(exception));

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);
  EXPECT_TRUE(ctx_.hasException());
}

TEST_F(
    ClientRequestResponseFrameHandlerTest, Tracking_ClearedOnHandlerRemoved) {
  sendRequest(1);
  EXPECT_EQ(handler_.pendingRequestResponseCount(), 1);

  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(handler_.pendingRequestResponseCount(), 0);
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
