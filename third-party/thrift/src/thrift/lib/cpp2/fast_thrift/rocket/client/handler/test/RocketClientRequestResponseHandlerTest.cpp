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
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::frame::ErrorCode;
using apache::thrift::fast_thrift::frame::FrameType;

namespace {

RocketResponseMessage makePayloadResponse(
    uint32_t streamId,
    bool complete,
    bool next,
    FrameType streamType = FrameType::REQUEST_RESPONSE,
    std::unique_ptr<folly::IOBuf> data = nullptr) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = streamId, .complete = complete, .next = next},
      nullptr,
      std::move(data));
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .streamType = streamType,
  };
}

RocketResponseMessage makeErrorResponse(
    uint32_t streamId,
    uint32_t errorCode,
    FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ErrorHeader{
          .streamId = streamId, .errorCode = errorCode},
      nullptr,
      folly::IOBuf::copyBuffer("error"));
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .streamType = streamType,
  };
}

RocketResponseMessage makeRequestNResponse(
    uint32_t streamId,
    uint32_t requestN,
    FrameType streamType = FrameType::REQUEST_RESPONSE) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestNHeader{
          .streamId = streamId, .requestN = requestN});
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
      .streamType = streamType,
  };
}

RocketResponseMessage makeExtResponse(
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
  return RocketResponseMessage{
      .payload =
          apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf)),
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

class ClientRequestResponseHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
  RocketClientRequestResponseHandler handler_;
};

// =============================================================================
// Pattern filter: only act on RR streams; let other patterns pass through.
// =============================================================================

TEST_F(ClientRequestResponseHandlerTest, Read_NonRRStreamType_PassesThrough) {
  // A PAYLOAD with COMPLETE-only would normally be a violation on an RR
  // stream (synthesized to ERROR), but here streamType=REQUEST_STREAM means
  // RR handler must not interpret it. Verifies the filter fires before any
  // RR-specific validation.
  auto response = makePayloadResponse(
      /*streamId=*/3,
      /*complete=*/true,
      /*next=*/false,
      /*streamType=*/FrameType::REQUEST_STREAM);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.streamType, FrameType::REQUEST_STREAM);
  EXPECT_EQ(
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>()
          .type(),
      FrameType::PAYLOAD);
}

// =============================================================================
// PAYLOAD: must carry NEXT.
// =============================================================================

TEST_F(ClientRequestResponseHandlerTest, Read_PayloadWithNext_Forwarded) {
  auto response = makePayloadResponse(
      /*streamId=*/1,
      /*complete=*/true,
      /*next=*/true,
      FrameType::REQUEST_RESPONSE,
      folly::IOBuf::copyBuffer("response"));

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& parsed =
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
  EXPECT_TRUE(parsed.hasNext());
  EXPECT_EQ(parsed.streamId(), 1u);
}

TEST_F(ClientRequestResponseHandlerTest, Read_PayloadWithNextOnly_Forwarded) {
  // NEXT-only (no COMPLETE) is unusual for RR but spec-legal — pass through.
  // The single-shot termination signal is the App's contract, not the wire.
  auto response = makePayloadResponse(1, /*complete=*/false, /*next=*/true);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(
    ClientRequestResponseHandlerTest,
    Read_PayloadCompleteOnly_SynthesizedAsError) {
  auto response = makePayloadResponse(7, /*complete=*/true, /*next=*/false);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& parsed =
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 7u);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(static_cast<ErrorCode>(errorView.errorCode()), ErrorCode::INVALID);
}

// =============================================================================
// ERROR: terminal failure response, forwarded to App.
// =============================================================================

TEST_F(ClientRequestResponseHandlerTest, Read_Error_Forwarded) {
  auto response =
      makeErrorResponse(1, static_cast<uint32_t>(ErrorCode::APPLICATION_ERROR));

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& parsed =
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(
      static_cast<ErrorCode>(errorView.errorCode()),
      ErrorCode::APPLICATION_ERROR);
}

// =============================================================================
// EXT: silently dropped when ignore=true; synthesized to ERROR otherwise.
// =============================================================================

TEST_F(ClientRequestResponseHandlerTest, Read_ExtWithIgnore_SilentlyDropped) {
  auto response = makeExtResponse(1, /*ignore=*/true);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  // Critical: dropped, NOT forwarded upstream. Stream stays alive for the
  // server to follow up with a real response.
  EXPECT_TRUE(ctx_.readMessages().empty());
}

TEST_F(
    ClientRequestResponseHandlerTest,
    Read_ExtWithoutIgnore_SynthesizedAsError) {
  auto response = makeExtResponse(1, /*ignore=*/false);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& parsed =
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(static_cast<ErrorCode>(errorView.errorCode()), ErrorCode::INVALID);
}

// =============================================================================
// Unexpected frame types: synthesize ERROR(INVALID), still forward (do not
// return Result::Error — the connection stays up).
// =============================================================================

TEST_F(
    ClientRequestResponseHandlerTest,
    Read_UnexpectedRequestN_SynthesizedAsError) {
  auto response = makeRequestNResponse(1, /*requestN=*/5);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  auto& parsed =
      forwarded.payload
          .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 1u);
  apache::thrift::fast_thrift::frame::read::ErrorView errorView{parsed};
  EXPECT_EQ(static_cast<ErrorCode>(errorView.errorCode()), ErrorCode::INVALID);
}

// =============================================================================
// onException: forwards exception unchanged (handler is stateless — no
// per-stream state to clear).
// =============================================================================

TEST_F(ClientRequestResponseHandlerTest, OnException_Forwarded) {
  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("connection error");
  handler_.onException(ctx_, std::move(exception));
  EXPECT_TRUE(ctx_.hasException());
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
