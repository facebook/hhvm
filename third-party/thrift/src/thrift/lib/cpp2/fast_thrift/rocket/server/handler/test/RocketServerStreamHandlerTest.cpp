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

#include <limits>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;
using apache::thrift::fast_thrift::frame::ErrorCode;
using apache::thrift::fast_thrift::frame::FrameType;
using apache::thrift::fast_thrift::rocket::RocketStreamContext;
using apache::thrift::fast_thrift::rocket::RocketStreamContexts;

namespace {

apache::thrift::fast_thrift::frame::read::ParsedFrame makeParsedFrame(
    std::unique_ptr<folly::IOBuf> buf) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
}

RocketRequestMessage makeRequestResponseRequest(uint32_t streamId) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
          .streamId = streamId},
      nullptr,
      nullptr);
  return RocketRequestMessage{
      .frame = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_RESPONSE,
  };
}

RocketRequestMessage makeRequestStreamRequest(
    uint32_t streamId, uint32_t initialRequestN) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestStreamHeader{
          .streamId = streamId, .initialRequestN = initialRequestN},
      nullptr,
      nullptr);
  return RocketRequestMessage{
      .frame = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketRequestMessage makeRequestN(uint32_t streamId, uint32_t requestN) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::RequestNHeader{
          .streamId = streamId, .requestN = requestN});
  return RocketRequestMessage{
      .frame = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketRequestMessage makeCancel(uint32_t streamId) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::CancelHeader{
          .streamId = streamId});
  return RocketRequestMessage{
      .frame = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketRequestMessage makeExt(uint32_t streamId, bool ignore) {
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::ExtHeader{
          .streamId = streamId,
          .extendedType = 0,
          .ignore = ignore,
      },
      nullptr,
      nullptr);
  return RocketRequestMessage{
      .frame = makeParsedFrame(std::move(buf)),
      .streamId = streamId,
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makePayloadResponse(
    uint32_t streamId, bool next = true, bool complete = false) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = FrameType::PAYLOAD,
              .streamId = streamId,
              .complete = complete,
              .next = next,
          },
      .streamType = FrameType::REQUEST_STREAM,
  };
}

RocketResponseMessage makeErrorResponse(uint32_t streamId) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = FrameType::ERROR,
              .streamId = streamId,
              .data = folly::IOBuf::copyBuffer("err"),
              .errorCode = static_cast<uint32_t>(ErrorCode::APPLICATION_ERROR),
          },
      .streamType = FrameType::REQUEST_STREAM,
  };
}

// A response carrying a frame type that is never valid outbound on a streaming
// stream (neither PAYLOAD nor ERROR).
RocketResponseMessage makeUnexpectedResponse(uint32_t streamId) {
  return RocketResponseMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = FrameType::REQUEST_N,
              .streamId = streamId,
          },
      .streamType = FrameType::REQUEST_STREAM,
  };
}

/**
 * MockContext for RocketServerStreamHandler tests.
 *
 * Owns the pipeline-level RocketStreamContexts that the handler reaches through
 * state<T>(), so tests observe per-stream credit state directly on the shared
 * map. Entry lifecycle (insert on open, erase on terminal) is
 * RocketServerStreamStateHandler's job; tests stand in for it via openStream().
 */
class MockContext {
 public:
  template <typename T>
  T& state() noexcept {
    return contexts_;
  }

  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return nextWriteResult_;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }
  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }
  bool hasException() const { return static_cast<bool>(exception_); }

  void setNextWriteResult(Result r) { nextWriteResult_ = r; }

  // Shared-state helpers -----------------------------------------------------

  // Stand in for StreamStateHandler opening a stream: insert an entry with the
  // given streamType (default REQUEST_STREAM) and initial credits.
  void openStream(
      uint32_t streamId,
      FrameType streamType = FrameType::REQUEST_STREAM,
      uint64_t credits = 0) {
    contexts_.streams.emplace(
        streamId,
        RocketStreamContext{.streamType = streamType, .credits = credits});
  }

  bool hasStream(uint32_t streamId) const {
    return contexts_.streams.contains(streamId);
  }

  uint64_t credits(uint32_t streamId) {
    auto it = contexts_.streams.find(streamId);
    return it == contexts_.streams.end() ? 0 : it->second.credits;
  }

  void setCredits(uint32_t streamId, uint64_t value) {
    contexts_.streams.find(streamId)->second.credits = value;
  }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    nextWriteResult_ = Result::Success;
  }

 private:
  RocketStreamContexts contexts_;
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  Result nextWriteResult_{Result::Success};
};

} // namespace

class ServerStreamHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
  RocketServerStreamHandler handler_;
};

// =============================================================================
// Pattern filter: act only on streaming streams whose entry exists.
// =============================================================================

TEST_F(ServerStreamHandlerTest, Read_UnknownStream_PassesThrough) {
  // No entry registered — REQUEST_N passes through untouched.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/99, /*requestN=*/5))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

TEST_F(ServerStreamHandlerTest, Read_NonStreamingEntry_PassesThrough) {
  // A REQUEST_RESPONSE stream is owned by another handler; pass through.
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_RESPONSE);

  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/1, /*requestN=*/5))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

TEST_F(ServerStreamHandlerTest, Write_UnknownStream_PassesThrough) {
  auto response = makePayloadResponse(99, /*next=*/true, /*complete=*/false);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_EQ(
      ctx_.writeMessages()[0].get<RocketResponseMessage>().frame.frameType,
      FrameType::PAYLOAD);
}

TEST_F(ServerStreamHandlerTest, Write_NonStreamingEntry_PassesThrough) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_RESPONSE);
  // Even at zero credits, a non-streaming stream is not gated by this handler.
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(makePayloadResponse(1))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_EQ(
      ctx_.writeMessages()[0].get<RocketResponseMessage>().frame.frameType,
      FrameType::PAYLOAD);
}

// =============================================================================
// Inbound: REQUEST_STREAM -- seeds credits on the existing entry.
// =============================================================================

TEST_F(ServerStreamHandlerTest, Read_RequestStream_SeedsCredits) {
  ctx_.openStream(/*streamId=*/1);

  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1, 10))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  EXPECT_EQ(ctx_.credits(1), 10u);
  auto& forwarded = ctx_.readMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(forwarded.frame.type(), FrameType::REQUEST_STREAM);
}

TEST_F(
    ServerStreamHandlerTest,
    Read_RequestStreamInvalidInitialRequestN_SynthesizesError) {
  ctx_.openStream(/*streamId=*/1);

  // initialRequestN=0 → ERROR(INVALID), credits left unseeded.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1, 0))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(errorMsg.frame.streamId, 1u);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
  EXPECT_EQ(ctx_.credits(1), 0u);
}

TEST_F(
    ServerStreamHandlerTest,
    Read_RequestStreamNegativeInitialRequestN_SynthesizesError) {
  ctx_.openStream(/*streamId=*/1);

  // 0x80000000 is negative as int32_t → ERROR(INVALID).
  EXPECT_EQ(
      handler_.onRead(
          ctx_,
          erase_and_box(makeRequestStreamRequest(/*streamId=*/1, 0x80000000))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
}

// =============================================================================
// Inbound: REQUEST_N -- credit increments on streaming streams.
// =============================================================================

TEST_F(ServerStreamHandlerTest, Read_RequestNOnActiveStream_IncrementsCredits) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/1);

  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/1, /*requestN=*/1))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  EXPECT_EQ(ctx_.credits(1), 2u);
}

TEST_F(ServerStreamHandlerTest, Read_RequestNOnUnknownStream_PassesThrough) {
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/99, /*requestN=*/5))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

TEST_F(ServerStreamHandlerTest, Read_RequestNWithZero_DroppedSilently) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/1);

  // REQUEST_N(0) → silently dropped (not forwarded, no error), credits intact.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/1, /*requestN=*/0))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  EXPECT_TRUE(ctx_.writeMessages().empty());
  EXPECT_EQ(ctx_.credits(1), 1u);
}

TEST_F(ServerStreamHandlerTest, Read_RequestNAccumulates) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/1);

  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/1, /*requestN=*/2))),
      Result::Success);
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/1, /*requestN=*/3))),
      Result::Success);
  EXPECT_EQ(ctx_.credits(1), 6u);
}

TEST_F(ServerStreamHandlerTest, Read_RequestNOverflow_SynthesizesError) {
  ctx_.openStream(/*streamId=*/1);
  ctx_.setCredits(1, std::numeric_limits<uint64_t>::max());

  // Any positive REQUEST_N would overflow → ERROR(INVALID).
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestN(/*streamId=*/1, /*requestN=*/1))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
}

// =============================================================================
// Inbound: CANCEL -- forwarded; entry lifecycle left to StreamStateHandler.
// =============================================================================

TEST_F(
    ServerStreamHandlerTest,
    Read_CancelOnActiveStream_PassesThroughWithoutErasing) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/5);

  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(makeCancel(/*streamId=*/1))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  EXPECT_EQ(
      ctx_.readMessages()[0].get<RocketRequestMessage>().frame.type(),
      FrameType::CANCEL);
  // This handler does not own entry lifecycle: the entry is untouched.
  EXPECT_TRUE(ctx_.hasStream(1));
}

TEST_F(ServerStreamHandlerTest, Read_CancelOnUnknownStream_PassesThrough) {
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(makeCancel(/*streamId=*/99))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

// =============================================================================
// Inbound: EXT -- ignore flag determines behavior.
// =============================================================================

TEST_F(ServerStreamHandlerTest, Read_ExtIgnoreOnActiveStream_DroppedSilently) {
  ctx_.openStream(/*streamId=*/1);

  // EXT(ignore=true) → silently dropped, entry stays alive.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/true))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  EXPECT_TRUE(ctx_.writeMessages().empty());
  EXPECT_TRUE(ctx_.hasStream(1));
}

TEST_F(ServerStreamHandlerTest, Read_ExtIgnoreOnUnknownStream_PassesThrough) {
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/99, /*ignore=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

TEST_F(
    ServerStreamHandlerTest, Read_ExtNoIgnoreOnActiveStream_SynthesizesError) {
  ctx_.openStream(/*streamId=*/1);

  // EXT(ignore=false) → ERROR(INVALID).
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/1, /*ignore=*/false))),
      Result::Success);
  EXPECT_TRUE(ctx_.readMessages().empty());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(errorMsg.frame.streamId, 1u);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
}

TEST_F(ServerStreamHandlerTest, Read_ExtNoIgnoreOnUnknownStream_PassesThrough) {
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeExt(/*streamId=*/99, /*ignore=*/false))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
}

// =============================================================================
// Outbound: PAYLOAD -- credit decrement and enforcement.
// =============================================================================

TEST_F(
    ServerStreamHandlerTest,
    Write_PayloadNextOnActiveStream_DecrementsCredits) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/10);

  auto response = makePayloadResponse(1, /*next=*/true, /*complete=*/false);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.frameType, FrameType::PAYLOAD);
  EXPECT_TRUE(forwarded.frame.next);
  EXPECT_EQ(ctx_.credits(1), 9u);
}

TEST_F(
    ServerStreamHandlerTest,
    Write_PayloadNextAndComplete_DecrementsAndForwards) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/10);

  // next=true consumes a credit; complete is forwarded but this handler does
  // not erase (StreamStateHandler erases on the terminal frame).
  auto response = makePayloadResponse(1, /*next=*/true, /*complete=*/true);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.frameType, FrameType::PAYLOAD);
  EXPECT_TRUE(forwarded.frame.complete);
  EXPECT_EQ(ctx_.credits(1), 9u);
}

TEST_F(ServerStreamHandlerTest, Write_PayloadCompleteOnly_NoCreditConsumed) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/0);

  // COMPLETE-only (next=false) succeeds even at 0 credits — no credit consumed.
  auto response = makePayloadResponse(1, /*next=*/false, /*complete=*/true);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(response))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.frameType, FrameType::PAYLOAD);
  EXPECT_FALSE(forwarded.frame.next);
  EXPECT_TRUE(forwarded.frame.complete);
  EXPECT_EQ(ctx_.credits(1), 0u);
}

TEST_F(ServerStreamHandlerTest, Write_PayloadExceedsCredits_SynthesizesError) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/0);

  // PAYLOAD(next=true) at 0 credits → ERROR(INVALID), original not forwarded.
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makePayloadResponse(1, /*next=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
}

TEST_F(ServerStreamHandlerTest, CreditEnforcement_SeedAndDecrement) {
  ctx_.openStream(/*streamId=*/1);
  // Seed 3 credits via REQUEST_STREAM.
  EXPECT_EQ(
      handler_.onRead(
          ctx_, erase_and_box(makeRequestStreamRequest(/*streamId=*/1, 3))),
      Result::Success);
  ctx_.reset();

  // Three PAYLOADs succeed (one credit each).
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(
        handler_.onWrite(
            ctx_, erase_and_box(makePayloadResponse(1, /*next=*/true))),
        Result::Success);
    EXPECT_EQ(
        ctx_.writeMessages()[0].get<RocketResponseMessage>().frame.frameType,
        FrameType::PAYLOAD);
    ctx_.reset();
  }
  EXPECT_EQ(ctx_.credits(1), 0u);

  // Fourth PAYLOAD exceeds credits → ERROR(INVALID).
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makePayloadResponse(1, /*next=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
}

// =============================================================================
// Outbound: ERROR / unexpected frame types.
// =============================================================================

TEST_F(ServerStreamHandlerTest, Write_ErrorOnActiveStream_ForwardedNoErase) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/5);

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(makeErrorResponse(1))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& forwarded = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(forwarded.frame.streamId, 1u);
  // Terminal erase is StreamStateHandler's responsibility.
  EXPECT_TRUE(ctx_.hasStream(1));
}

TEST_F(ServerStreamHandlerTest, Write_ErrorOnUnknownStream_PassesThrough) {
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(makeErrorResponse(99))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_EQ(
      ctx_.writeMessages()[0].get<RocketResponseMessage>().frame.streamId, 99u);
}

TEST_F(
    ServerStreamHandlerTest,
    Write_UnexpectedFrameTypeOnActiveStream_SynthesizesError) {
  ctx_.openStream(/*streamId=*/1);

  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(makeUnexpectedResponse(1))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errorMsg = ctx_.writeMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(errorMsg.frame.frameType, FrameType::ERROR);
  EXPECT_EQ(
      static_cast<ErrorCode>(errorMsg.frame.errorCode), ErrorCode::INVALID);
}

// =============================================================================
// Outbound: Write failure rollback.
// =============================================================================

TEST_F(ServerStreamHandlerTest, Write_PayloadWriteFailure_ReincrementsCredits) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/1);

  // Non-terminal PAYLOAD write fails — credit is rolled back to 1.
  ctx_.setNextWriteResult(Result::Error);
  EXPECT_EQ(
      handler_.onWrite(
          ctx_, erase_and_box(makePayloadResponse(1, /*next=*/true))),
      Result::Error);
  EXPECT_EQ(ctx_.credits(1), 1u);
}

TEST_F(ServerStreamHandlerTest, Write_PayloadCompleteWriteFailure_NoRollback) {
  ctx_.openStream(/*streamId=*/1, FrameType::REQUEST_STREAM, /*credits=*/10);

  // Terminal PAYLOAD write fails — the consumed credit is not rolled back.
  ctx_.setNextWriteResult(Result::Error);
  EXPECT_EQ(
      handler_.onWrite(
          ctx_,
          erase_and_box(
              makePayloadResponse(1, /*next=*/true, /*complete=*/true))),
      Result::Error);
  EXPECT_EQ(ctx_.credits(1), 9u);
}

// =============================================================================
// Handler independence: non-stream frames pass through on both paths.
// =============================================================================

TEST_F(
    ServerStreamHandlerTest, HandlerIndependence_NonStreamFramePassesThrough) {
  // Read path: REQUEST_RESPONSE for an unregistered stream passes through.
  EXPECT_EQ(
      handler_.onRead(ctx_, erase_and_box(makeRequestResponseRequest(5))),
      Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_TRUE(ctx_.writeMessages().empty());
  ctx_.reset();

  // Write path: unregistered stream passes through.
  EXPECT_EQ(
      handler_.onWrite(
          ctx_,
          erase_and_box(
              makePayloadResponse(5, /*next=*/true, /*complete=*/true))),
      Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_TRUE(ctx_.readMessages().empty());
}

// =============================================================================
// onException: forwards exception unchanged.
// =============================================================================

TEST_F(ServerStreamHandlerTest, OnException_Forwarded) {
  auto exception =
      folly::make_exception_wrapper<std::runtime_error>("connection error");
  handler_.onException(ctx_, std::move(exception));
  EXPECT_TRUE(ctx_.hasException());
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
