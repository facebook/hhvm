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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

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

std::unique_ptr<folly::IOBuf> makeRawKeepAliveFrame(bool respond = false) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::KeepAliveHeader{
          .lastReceivedPosition = 0, .respond = respond},
      nullptr);
}

std::unique_ptr<folly::IOBuf> makeRawErrorFrame(
    uint32_t streamId, uint32_t errorCode, std::unique_ptr<folly::IOBuf> data) {
  return apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = errorCode},
      nullptr,
      std::move(data));
}

} // namespace

class RocketServerFrameCodecHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
  RocketServerFrameCodecHandler handler_;
};

// =============================================================================
// Inbound (Read) - Basic Decoding Tests
// =============================================================================

TEST_F(RocketServerFrameCodecHandlerTest, DecodesPayloadFrame) {
  auto rawFrame =
      makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("test payload"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& request =
      ctx_.readMessages()[0].get<rocket::server::RocketRequestMessage>();
  auto& frame =
      request.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_EQ(frame.streamId(), 1);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
}

TEST_F(RocketServerFrameCodecHandlerTest, DecodesRequestResponseFrame) {
  auto rawFrame = makeRawRequestResponseFrame(
      3,
      folly::IOBuf::copyBuffer("metadata"),
      folly::IOBuf::copyBuffer("data"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& request =
      ctx_.readMessages()[0].get<rocket::server::RocketRequestMessage>();
  auto& frame =
      request.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(frame.streamId(), 3);
  EXPECT_TRUE(frame.hasMetadata());
}

TEST_F(RocketServerFrameCodecHandlerTest, DecodesKeepAliveFrame) {
  auto rawFrame = makeRawKeepAliveFrame(true);

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& request =
      ctx_.readMessages()[0].get<rocket::server::RocketRequestMessage>();
  auto& frame =
      request.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(
      frame.type(), apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE);
  EXPECT_EQ(frame.streamId(), 0);
}

TEST_F(RocketServerFrameCodecHandlerTest, DecodesErrorFrame) {
  auto rawFrame =
      makeRawErrorFrame(5, 0x00000201, folly::IOBuf::copyBuffer("error"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& request =
      ctx_.readMessages()[0].get<rocket::server::RocketRequestMessage>();
  auto& frame =
      request.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(frame.type(), apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(frame.streamId(), 5);
}

// =============================================================================
// Inbound (Read) - Invalid Frame Tests
// =============================================================================

TEST_F(RocketServerFrameCodecHandlerTest, RejectsInvalidFrame) {
  // A 1-byte buffer is too short for a valid frame header
  auto result =
      handler_.onRead(ctx_, erase_and_box(folly::IOBuf::copyBuffer("x")));

  EXPECT_EQ(result, Result::Error);
  EXPECT_TRUE(ctx_.hasException());
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

// =============================================================================
// Inbound (Read) - Backpressure Tests
// =============================================================================

TEST_F(RocketServerFrameCodecHandlerTest, PropagatesBackpressure) {
  ctx_.setReadResult(Result::Backpressure);
  auto rawFrame = makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("test"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(RocketServerFrameCodecHandlerTest, PropagatesError) {
  ctx_.setReadResult(Result::Error);
  auto rawFrame = makeRawPayloadFrame(1, folly::IOBuf::copyBuffer("test"));

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(rawFrame)));

  EXPECT_EQ(result, Result::Error);
}

// =============================================================================
// Inbound (Read) - Exception Handling Tests
// =============================================================================

TEST_F(RocketServerFrameCodecHandlerTest, ForwardsException) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");

  handler_.onException(ctx_, ex);

  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Outbound (Write) - Passthrough Tests
// =============================================================================

TEST_F(RocketServerFrameCodecHandlerTest, WriteSerializesPayload) {
  const std::string dataStr = "hello";
  auto expectedFrame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1, .complete = true, .next = true},
      nullptr,
      folly::IOBuf::copyBuffer(dataStr));

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = folly::IOBuf::copyBuffer(dataStr),
              .header = {.streamId = 1, .complete = true, .next = true},
          },
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& writtenFrame =
      ctx_.writeMessages()[0].get<std::unique_ptr<folly::IOBuf>>();
  EXPECT_TRUE(folly::IOBufEqualTo{}(*writtenFrame, *expectedFrame));
}

TEST_F(RocketServerFrameCodecHandlerTest, PropagatesWriteError) {
  ctx_.setWriteResult(Result::Error);

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = folly::IOBuf::copyBuffer("test"),
              .header = {.streamId = 1, .complete = true, .next = true},
          },
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
}

TEST_F(RocketServerFrameCodecHandlerTest, PropagatesWriteBackpressure) {
  ctx_.setWriteResult(Result::Backpressure);

  RocketResponseMessage response{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = folly::IOBuf::copyBuffer("test"),
              .header = {.streamId = 1, .complete = true, .next = true},
          },
  };

  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Backpressure);
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
