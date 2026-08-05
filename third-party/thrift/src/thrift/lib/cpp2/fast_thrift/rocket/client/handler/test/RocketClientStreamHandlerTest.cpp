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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::frame::ErrorCode;
using apache::thrift::fast_thrift::frame::FrameType;

namespace {

// =============================================================================
// Outbound helpers (client sends requests — ComposedFrame)
// =============================================================================

RocketRequestMessage makeRequestStreamRequest(
    uint32_t streamId, uint32_t initialRequestN = 10) {
  return RocketRequestMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = FrameType::REQUEST_STREAM,
              .streamId = streamId,
              .initialRequestN = initialRequestN,
          },
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketRequestMessage makeRequestResponseRequest(uint32_t streamId) {
  return RocketRequestMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = FrameType::REQUEST_RESPONSE,
              .streamId = streamId,
          },
      .requestContext = {},
      .streamType = FrameType::REQUEST_RESPONSE,
  };
}

// =============================================================================
// Inbound helpers (client receives responses — ParsedFrame from wire bytes)
// =============================================================================

RocketResponseMessage makePayloadResponse(
    uint32_t streamId, bool next = true, bool complete = false) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = streamId, .complete = complete, .next = next},
      nullptr,
      folly::IOBuf::copyBuffer("data"));
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makeErrorResponse(uint32_t streamId) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId,
          .errorCode = static_cast<uint32_t>(ErrorCode::APPLICATION_ERROR)},
      nullptr,
      folly::IOBuf::copyBuffer("error"));
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makeExt(uint32_t streamId, bool ignore) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ExtHeader{
          .streamId = streamId, .extendedType = 0, .ignore = ignore},
      nullptr,
      nullptr);
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makeRequestN(uint32_t streamId) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestNHeader{
          .streamId = streamId, .requestN = 5});
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .requestContext = {},
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makeResponseError(uint32_t streamId) {
  RocketResponseMessage resp;
  resp.payload = RocketResponseError{
      .ew = folly::make_exception_wrapper<std::runtime_error>("test error"),
      .streamId = streamId,
  };
  resp.streamType = FrameType::REQUEST_STREAM;
  return resp;
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

class ClientStreamHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
  RocketClientStreamHandler handler_;
};

// =============================================================================
// Outbound: REQUEST_STREAM registers streamId; non-stream passes through.
// =============================================================================

TEST_F(ClientStreamHandlerTest, Write_RequestStream_RegistersStreamId) {
  // REQUEST_STREAM → forwarded and streamId registered.
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/5))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  ctx_.reset();

  // Stream is active: EXT(ignore=true) is silently dropped (not forwarded).
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/5, /*ignore=*/true))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
}

TEST_F(ClientStreamHandlerTest, Write_NonStreamRequest_PassesThrough) {
  // REQUEST_RESPONSE → forwarded, streamId NOT registered.
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestResponseRequest(/*streamId=*/3))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  ctx_.reset();

  // Stream is inactive: EXT(ignore=true) passes through (not consumed).
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/3, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Inbound PAYLOAD: NEXT keeps stream alive; COMPLETE erases; malformed
// (neither NEXT nor COMPLETE) synthesizes ERROR.
// =============================================================================

TEST_F(ClientStreamHandlerTest, Read_PayloadNextOnActiveStream_Forwarded) {
  // Register stream, receive PAYLOAD(next=true) → forwarded, stream alive.
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1))),
      Result::Success);
  ctx_.reset();

  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makePayloadResponse(/*streamId=*/1, /*next=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
  EXPECT_TRUE(parsed.hasNext());
  ctx_.reset();

  // Stream still active.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
}

TEST_F(ClientStreamHandlerTest, Read_PayloadCompleteOnly_ValidAndErases) {
  // COMPLETE-only is valid for streaming (unlike RR).
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/7))),
      Result::Success);
  ctx_.reset();

  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makePayloadResponse(
              /*streamId=*/7, /*next=*/false, /*complete=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
  ctx_.reset();

  // Stream erased: EXT passes through.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/7, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(
    ClientStreamHandlerTest, Read_PayloadNextAndComplete_ForwardedAndErases) {
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/2))),
      Result::Success);
  ctx_.reset();

  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makePayloadResponse(
              /*streamId=*/2, /*next=*/true, /*complete=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
  EXPECT_TRUE(parsed.hasNext());
  ctx_.reset();

  // Stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/2, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ClientStreamHandlerTest, Read_PayloadMalformed_SynthesizesError) {
  // PAYLOAD with neither NEXT nor COMPLETE → ERROR(INVALID), stream erased.
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/4))),
      Result::Success);
  ctx_.reset();

  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makePayloadResponse(
              /*streamId=*/4, /*next=*/false, /*complete=*/false))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 4u);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(static_cast<ErrorCode>(errorView.errorCode()), ErrorCode::INVALID);
  ctx_.reset();

  // Stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/4, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ClientStreamHandlerTest, Read_PayloadOnUnknownStream_PassesThrough) {
  // No stream registered — PAYLOAD passes through unchanged.
  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makePayloadResponse(/*streamId=*/99, /*next=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
}

// =============================================================================
// Inbound ERROR: terminal failure response — erase stream and forward.
// =============================================================================

TEST_F(ClientStreamHandlerTest, Read_ErrorOnActiveStream_ErasesAndForwards) {
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1))),
      Result::Success);
  ctx_.reset();

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(makeErrorResponse(/*streamId=*/1))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(
      static_cast<ErrorCode>(errorView.errorCode()),
      ErrorCode::APPLICATION_ERROR);
  ctx_.reset();

  // Stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Inbound EXT: silently dropped when ignore=true; synthesized to ERROR
// otherwise.
// =============================================================================

TEST_F(ClientStreamHandlerTest, Read_ExtIgnoreOnActiveStream_DroppedSilently) {
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1))),
      Result::Success);
  ctx_.reset();

  // EXT(ignore=true) → silently dropped, stream stays alive.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());

  // Stream still active — second EXT(ignore=true) also dropped.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
}

TEST_F(
    ClientStreamHandlerTest, Read_ExtNoIgnoreOnActiveStream_SynthesizesError) {
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1))),
      Result::Success);
  ctx_.reset();

  // EXT(ignore=false) → ERROR(INVALID), stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/false))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(static_cast<ErrorCode>(errorView.errorCode()), ErrorCode::INVALID);
  ctx_.reset();

  // Stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Inbound RocketResponseError: in-process error variant — erase and forward.
// =============================================================================

TEST_F(ClientStreamHandlerTest, Read_ResponseError_ErasesAndForwards) {
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/6))),
      Result::Success);
  ctx_.reset();

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(makeResponseError(/*streamId=*/6))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.readMessages()[0]
                  .get<RocketResponseMessage>()
                  .payload.is<RocketResponseError>());
  ctx_.reset();

  // Stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/6, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Unexpected frame types: synthesize ERROR(INVALID), still forward.
// =============================================================================

TEST_F(ClientStreamHandlerTest, Read_UnexpectedFrameType_SynthesizesError) {
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1))),
      Result::Success);
  ctx_.reset();

  // REQUEST_N is not a valid inbound frame for streaming → ERROR(INVALID).
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(makeRequestN(/*streamId=*/1))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(static_cast<ErrorCode>(errorView.errorCode()), ErrorCode::INVALID);
  ctx_.reset();

  // Stream erased.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Handler independence: responses for unregistered streams pass through.
// =============================================================================

TEST_F(
    ClientStreamHandlerTest,
    HandlerIndependence_NonStreamResponsePassesThrough) {
  // No stream registered — PAYLOAD passes through unchanged.
  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makePayloadResponse(/*streamId=*/42, /*next=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& parsed =
      ctx_.readMessages()[0]
          .get<RocketResponseMessage>()
          .payload.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
}

// =============================================================================
// onException: forwarded unchanged (no per-stream cleanup needed).
// =============================================================================

TEST_F(ClientStreamHandlerTest, OnException_Forwarded) {
  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("connection error");
  handler_.onException(ctx_, std::move(exception));
  EXPECT_TRUE(ctx_.hasException());
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
