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
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::frame::ErrorCode;
using apache::thrift::fast_thrift::frame::FrameType;

namespace {

apache::thrift::fast_thrift::frame::read::ParsedFrame makeParsedFrame(
    std::unique_ptr<folly::IOBuf> buf) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
}

RocketRequestMessage makeRequestResponseRequest(
    uint32_t streamId,
    FrameType streamType = FrameType::REQUEST_RESPONSE,
    std::unique_ptr<folly::IOBuf> data = nullptr) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId},
      nullptr,
      std::move(data));
  return RocketRequestMessage{
      .payload = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = streamType,
  };
}

RocketRequestMessage makeCancelRequest(
    uint32_t streamId, FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::CancelHeader{
          .streamId = streamId});
  return RocketRequestMessage{
      .payload = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = streamType,
  };
}

RocketRequestMessage makeExtRequest(
    uint32_t streamId,
    bool ignore,
    FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ExtHeader{
          .streamId = streamId,
          .extendedType = 0,
          .ignore = ignore,
      },
      nullptr,
      nullptr);
  return RocketRequestMessage{
      .payload = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = streamType,
  };
}

RocketRequestMessage makeRequestNRequest(
    uint32_t streamId,
    uint32_t requestN,
    FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestNHeader{
          .streamId = streamId, .requestN = requestN});
  return RocketRequestMessage{
      .payload = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = streamType,
  };
}

RocketResponseMessage makePayloadResponse(
    uint32_t streamId,
    FrameType streamType = FrameType::REQUEST_RESPONSE,
    std::unique_ptr<folly::IOBuf> data = nullptr) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = std::move(data),
              .metadata = nullptr,
              .header = {.streamId = streamId},
          },
      .streamType = streamType,
  };
}

RocketResponseMessage makeErrorResponse(
    uint32_t streamId,
    uint32_t errorCode,
    FrameType streamType = FrameType::REQUEST_RESPONSE) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedErrorFrame{
              .data = folly::IOBuf::copyBuffer("err"),
              .metadata = nullptr,
              .header = {.streamId = streamId, .errorCode = errorCode},
          },
      .streamType = streamType,
  };
}

class MockContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }
  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  bool hasException() const { return static_cast<bool>(exception_); }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
};

} // namespace

class ServerRequestResponseHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
  RocketServerRequestResponseHandler handler_;
};

// =============================================================================
// Inbound: Pattern filter — only act on RR streams.
// =============================================================================

TEST_F(ServerRequestResponseHandlerTest, Read_NonRRStreamType_PassesThrough) {
  // A REQUEST_N on a STREAM streamType would normally synthesize ERROR on an
  // RR stream, but here streamType=REQUEST_STREAM means RR handler must not
  // interpret it. Verifies the filter fires before any RR-specific logic.
  auto request = makeRequestNRequest(
      /*streamId=*/3, /*requestN=*/5, FrameType::REQUEST_STREAM);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  auto& forwarded = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(forwarded.streamType, FrameType::REQUEST_STREAM);
}

// =============================================================================
// Inbound: REQUEST_RESPONSE — hot path.
// =============================================================================

TEST_F(ServerRequestResponseHandlerTest, Read_RequestResponse_Forwarded) {
  auto request = makeRequestResponseRequest(
      1, FrameType::REQUEST_RESPONSE, folly::IOBuf::copyBuffer("payload"));

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  auto& forwarded = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>()
          .type(),
      FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(forwarded.streamId, 1u);
}

// =============================================================================
// Inbound: CANCEL — forward to App so it can abort in-flight work.
// =============================================================================

TEST_F(ServerRequestResponseHandlerTest, Read_Cancel_Forwarded) {
  auto request = makeCancelRequest(7);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  auto& forwarded = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>()
          .type(),
      FrameType::CANCEL);
}

// =============================================================================
// Inbound: EXT — silently dropped when ignore=true; ERROR otherwise.
// =============================================================================

TEST_F(ServerRequestResponseHandlerTest, Read_ExtWithIgnore_SilentlyDropped) {
  auto request = makeExtRequest(1, /*ignore=*/true);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  // Critical: dropped, no fireRead, no fireWrite. Stream stays alive.
  EXPECT_TRUE(ctx_.readMessages().empty());
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

TEST_F(
    ServerRequestResponseHandlerTest, Read_ExtWithoutIgnore_SendsErrorInvalid) {
  auto request = makeExtRequest(1, /*ignore=*/false);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  ASSERT_TRUE(
      errorMsg.frame
          .is<apache::thrift::fast_thrift::frame::ComposedErrorFrame>());
  auto& errorPayload =
      errorMsg.frame
          .get<apache::thrift::fast_thrift::frame::ComposedErrorFrame>();
  EXPECT_EQ(errorPayload.header.streamId, 1u);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorPayload.header.errorCode),
      ErrorCode::INVALID);
}

// =============================================================================
// Inbound: Unexpected frame types — synthesize ERROR(INVALID) via fireWrite,
// keep connection alive (Result::Success, never Result::Error).
// =============================================================================

TEST_F(
    ServerRequestResponseHandlerTest,
    Read_UnexpectedRequestN_SendsErrorInvalid) {
  auto request = makeRequestNRequest(1, /*requestN=*/5);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  ASSERT_TRUE(
      errorMsg.frame
          .is<apache::thrift::fast_thrift::frame::ComposedErrorFrame>());
  auto& errorPayload =
      errorMsg.frame
          .get<apache::thrift::fast_thrift::frame::ComposedErrorFrame>();
  EXPECT_EQ(errorPayload.header.streamId, 1u);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorPayload.header.errorCode),
      ErrorCode::INVALID);
  EXPECT_EQ(errorMsg.streamType, FrameType::REQUEST_RESPONSE);
}

// =============================================================================
// Outbound: stamps complete=true, next=true on ComposedPayloadFrame for RR
// streams; ComposedErrorFrame pass through; non-RR pass through.
// =============================================================================

TEST_F(
    ServerRequestResponseHandlerTest,
    Write_PayloadOnRRStream_StampsCompleteAndNext) {
  auto response = makePayloadResponse(
      42, FrameType::REQUEST_RESPONSE, folly::IOBuf::copyBuffer("response"));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  auto& payload =
      forwarded.frame
          .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
  EXPECT_TRUE(payload.header.complete);
  EXPECT_TRUE(payload.header.next);
  EXPECT_EQ(payload.streamId(), 42u);
}

TEST_F(
    ServerRequestResponseHandlerTest,
    Write_ErrorOnRRStream_PassesThroughUnchanged) {
  auto response =
      makeErrorResponse(1, static_cast<uint32_t>(ErrorCode::APPLICATION_ERROR));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  auto& errorPayload =
      forwarded.frame
          .get<apache::thrift::fast_thrift::frame::ComposedErrorFrame>();
  EXPECT_EQ(errorPayload.header.streamId, 1u);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorPayload.header.errorCode),
      ErrorCode::APPLICATION_ERROR);
}

TEST_F(
    ServerRequestResponseHandlerTest,
    Write_NonRRStream_PassesThroughUnchanged) {
  auto response = makePayloadResponse(
      3, FrameType::REQUEST_STREAM, folly::IOBuf::copyBuffer("stream-frame"));

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  auto& payload =
      forwarded.frame
          .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
  // No stamping for non-RR — header.complete defaults to false.
  EXPECT_FALSE(payload.header.complete);
  EXPECT_FALSE(payload.header.next);
}

// =============================================================================
// onException: forwards exception unchanged (handler is stateless).
// =============================================================================

TEST_F(ServerRequestResponseHandlerTest, OnException_Forwarded) {
  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("connection error");
  handler_.onException(ctx_, std::move(exception));
  EXPECT_TRUE(ctx_.hasException());
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
