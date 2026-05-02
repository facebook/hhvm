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
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

/**
 * MockContext for testing RocketClientFrameCodecHandler.
 *
 * Captures messages fired via fireRead()/fireWrite() and exceptions via
 * fireException().
 */
class MockContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return readResult_;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return writeResult_;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  void close() noexcept { closed_ = true; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  void setReadResult(Result result) { readResult_ = result; }

  void setWriteResult(Result result) { writeResult_ = result; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    readResult_ = Result::Success;
    writeResult_ = Result::Success;
    disconnected_ = false;
    closed_ = false;
    writeReady_ = false;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  Result readResult_{Result::Success};
  Result writeResult_{Result::Success};
  bool disconnected_{false};
  bool closed_{false};
  bool writeReady_{false};
};

// Helper to create a raw PAYLOAD frame (without length prefix)
std::unique_ptr<folly::IOBuf> makeRawPayloadFrame(
    uint32_t streamId, std::unique_ptr<folly::IOBuf> data) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = streamId,
          .follows = false,
          .complete = true,
          .next = true},
      nullptr,
      std::move(data));
}

// Helper to create a raw REQUEST_RESPONSE frame (without length prefix)
std::unique_ptr<folly::IOBuf> makeRawRequestResponseFrame(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> metadata,
    std::unique_ptr<folly::IOBuf> data) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId, .follows = false},
      std::move(metadata),
      std::move(data));
}

// Helper to create a raw KEEPALIVE frame (without length prefix)
std::unique_ptr<folly::IOBuf> makeRawKeepAliveFrame(bool respond = false) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::KeepAliveHeader{
          .lastReceivedPosition = 0, .respond = respond},
      nullptr);
}

// Helper to create a raw ERROR frame (without length prefix)
std::unique_ptr<folly::IOBuf> makeRawErrorFrame(
    uint32_t streamId, uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = errorCode},
      nullptr,
      std::move(data));
}

} // namespace

class RocketClientFrameCodecHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
  RocketClientFrameCodecHandler handler_;
};

// =============================================================================
// Inbound (Read) - Basic Decoding Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, DecodesPayloadFrame) {
  auto rawFrame =
      makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("test payload"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& frame =
      response.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(frame.streamId(), 1);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
}

TEST_F(RocketClientFrameCodecHandlerTest, DecodesRequestResponseFrame) {
  auto rawFrame = makeRawRequestResponseFrame(
      3,
      folly::IOBuf::copyBuffer("metadata"),
      folly::IOBuf::copyBuffer("data"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& frame =
      response.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(frame.streamId(), 3);
  EXPECT_TRUE(frame.hasMetadata());
}

TEST_F(RocketClientFrameCodecHandlerTest, DecodesKeepAliveFrame) {
  auto rawFrame = makeRawKeepAliveFrame(true);

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& frame =
      response.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE);
  EXPECT_EQ(frame.streamId(), 0); // Connection-level frame
}

TEST_F(RocketClientFrameCodecHandlerTest, DecodesErrorFrame) {
  auto rawFrame =
      makeRawErrorFrame(5, 0x00000201, folly::IOBuf::copyBuffer("error"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& frame =
      response.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(frame.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(frame.streamId(), 5);
}

// =============================================================================
// Inbound (Read) - Frame Data Preservation Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, PreservesPayloadData) {
  std::string testData = "Hello, World! This is test data.";
  auto rawFrame = makeRawPayloadFrame(1, folly::IOBuf::copyBuffer(testData));

  (void)handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();

  // Verify the data can be extracted from the parsed frame
  EXPECT_GT(
      response.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>()
          .dataSize(),
      0);
}

TEST_F(RocketClientFrameCodecHandlerTest, PreservesMetadata) {
  std::string metadata = "request metadata";
  std::string data = "request data";
  auto rawFrame = makeRawRequestResponseFrame(
      1, folly::IOBuf::copyBuffer(metadata), folly::IOBuf::copyBuffer(data));

  (void)handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();

  auto& frame =
      response.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_GT(frame.metadataSize(), 0);
}

// =============================================================================
// Inbound (Read) - Multiple Frame Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, DecodesMultipleFrames) {
  auto frame1 = makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("payload 1"));
  auto frame2 = makeRawPayloadFrame(3, folly::IOBuf::copyBuffer("payload 2"));
  auto frame3 = makeRawKeepAliveFrame(false);

  (void)handler_.onRead(ctx_, erase_and_box(std::move(frame1)));
  (void)handler_.onRead(ctx_, erase_and_box(std::move(frame2)));
  (void)handler_.onRead(ctx_, erase_and_box(std::move(frame3)));

  ASSERT_EQ(ctx_.readMessages().size(), 3);

  using PF = apache::thrift::fast_thrift::frame::read::ParsedFrame;
  auto& response1 = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(
      response1.payload.get<PF>().type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(response1.payload.get<PF>().streamId(), 1);

  auto& response2 = ctx_.readMessages()[1].get<RocketResponseMessage>();
  EXPECT_EQ(
      response2.payload.get<PF>().type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(response2.payload.get<PF>().streamId(), 3);

  auto& response3 = ctx_.readMessages()[2].get<RocketResponseMessage>();
  EXPECT_EQ(
      response3.payload.get<PF>().type(),
      apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE);
}

// =============================================================================
// Inbound (Read) - Backpressure Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, PropagatesBackpressure) {
  ctx_.setReadResult(Result::Backpressure);
  auto rawFrame = makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("test"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(RocketClientFrameCodecHandlerTest, PropagatesError) {
  ctx_.setReadResult(Result::Error);
  auto rawFrame = makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("test"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Error);
}

// =============================================================================
// Inbound (Read) - Exception Handling Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, ForwardsException) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");

  handler_.onException(ctx_, ex);

  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Outbound (Write) - Basic Encoding Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, SerializesRequestResponsePayload) {
  auto data = folly::IOBuf::copyBuffer("data");
  auto expectedFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = 1},
      nullptr,
      data->clone());

  RocketRequestMessage request{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = std::move(data),
              .header = {.streamId = 1},
          },
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& writtenFrame =
      ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  EXPECT_TRUE(folly::IOBufEqualTo{}(*writtenFrame, *expectedFrame));
}

TEST_F(RocketClientFrameCodecHandlerTest, PropagatesWriteError) {
  ctx_.setWriteResult(Result::Error);

  RocketRequestMessage request{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = folly::IOBuf::copyBuffer("test"),
              .header = {.streamId = 1},
          },
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Error);
}

TEST_F(RocketClientFrameCodecHandlerTest, PropagatesWriteBackpressure) {
  ctx_.setWriteResult(Result::Backpressure);

  RocketRequestMessage request{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = folly::IOBuf::copyBuffer("test"),
              .header = {.streamId = 1},
          },
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Backpressure);
}

// =============================================================================
// Lifecycle Tests
// =============================================================================

TEST_F(RocketClientFrameCodecHandlerTest, HandlerAddedNoOp) {
  // handlerAdded should be a no-op
  handler_.handlerAdded(ctx_);

  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_FALSE(ctx_.hasException());
}

TEST_F(RocketClientFrameCodecHandlerTest, HandlerRemovedNoOp) {
  // handlerRemoved should be a no-op
  handler_.handlerRemoved(ctx_);

  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_FALSE(ctx_.hasException());
}

TEST_F(RocketClientFrameCodecHandlerTest, OnConnectNoOp) {
  // onConnect should be a no-op for this handler
  handler_.onPipelineActive(ctx_);

  EXPECT_EQ(ctx_.readMessages().size(), 0);
  EXPECT_FALSE(ctx_.hasException());
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
