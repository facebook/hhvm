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

#include <algorithm>
#include <cstring>
#include <utility>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#define private public
#define protected public
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;

namespace {

/**
 * Helper to create a RocketRequestMessage with REQUEST_RESPONSE frame type.
 */
RocketRequestMessage makeClientRequest(
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<folly::IOBuf> metadata,
    void* requestContext = nullptr) {
  return RocketRequestMessage{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedFrame{
              .frameType = apache::thrift::fast_thrift::frame::FrameType::
                  REQUEST_RESPONSE,
              .streamId = kInvalidStreamId,
              .metadata = std::move(metadata),
              .data = std::move(data),
          },
      .requestContext = borrow(requestContext),
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };
}

/**
 * Helper to get the streamId from a RocketRequestMessage.
 */
uint32_t getStreamId(const RocketRequestMessage& msg) {
  return msg.frame.streamId;
}

/**
 * MockStreamContext for testing RocketClientStreamStateHandler.
 *
 * Captures frames and messages fired via fireRead() and fireWrite(),
 * and exceptions via fireException().
 */
class MockStreamContext {
 public:
  Result fireRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (readReturnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  Result fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (writeReturnError_) {
      return Result::Error;
    }
    writeMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void fireEvent(
      RocketClientStreamStateHandler::EventId ev,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox
          box) noexcept {
    firedEvents_.emplace_back(ev, std::move(box));
  }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  // Sets BOTH read and write error flags. Use the directional setters
  // when you need fireWrite to fail but fireRead (e.g., rollback fan-out)
  // to succeed.
  void setReturnError(bool value) {
    readReturnError_ = value;
    writeReturnError_ = value;
  }
  void setReadReturnError(bool value) { readReturnError_ = value; }
  void setWriteReturnError(bool value) { writeReturnError_ = value; }

  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>&
  readMessages() {
    return readMessages_;
  }

  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>&
  writeMessages() {
    return writeMessages_;
  }

  bool hasException() const { return static_cast<bool>(exception_); }
  const folly::exception_wrapper& exception() const { return exception_; }

  std::vector<std::pair<
      RocketClientStreamStateHandler::EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>>&
  firedEvents() {
    return firedEvents_;
  }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    firedEvents_.clear();
    exception_ = folly::exception_wrapper();
    returnBackpressure_ = false;
    readReturnError_ = false;
    writeReturnError_ = false;
  }

 private:
  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
      readMessages_;
  std::vector<apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>
      writeMessages_;
  std::vector<std::pair<
      RocketClientStreamStateHandler::EventId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox>>
      firedEvents_;
  folly::exception_wrapper exception_;
  bool returnBackpressure_{false};
  bool readReturnError_{false};
  bool writeReturnError_{false};
};

std::unique_ptr<folly::IOBuf> buildFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  const auto& desc = apache::thrift::fast_thrift::frame::getDescriptor(type);
  size_t headerSize = desc.headerSize > 0
      ? desc.headerSize
      : apache::thrift::fast_thrift::frame::kBaseHeaderSize;

  auto buf = folly::IOBuf::create(headerSize);
  auto* data = buf->writableData();
  std::memset(data, 0, headerSize);

  // Write streamId (big-endian)
  data[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  data[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  data[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  data[3] = static_cast<uint8_t>(streamId & 0xFF);

  // Write typeAndFlags (big-endian)
  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type)
       << ::apache::thrift::fast_thrift::frame::detail::kFlagsBits) |
      flags;
  data[4] = static_cast<uint8_t>((typeAndFlags >> 8) & 0xFF);
  data[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  buf->append(headerSize);
  return buf;
}

apache::thrift::fast_thrift::frame::read::ParsedFrame parseTestFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(
      buildFrame(type, streamId, flags));
}

/**
 * Helper to wrap a ParsedFrame into RocketResponseMessage for onRead testing.
 */
RocketResponseMessage makeRocketResponse(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  return RocketResponseMessage{
      .payload = parseTestFrame(type, streamId, flags),
      .requestContext = {},
  };
}

} // namespace

class ClientStreamStateHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockStreamContext ctx_;
  RocketClientStreamStateHandler handler_;
};

// =============================================================================
// Stream ID Generation
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, GeneratesOddStreamIds) {
  EXPECT_EQ(handler_.generateStreamId(), 1);
  EXPECT_EQ(handler_.generateStreamId(), 3);
  EXPECT_EQ(handler_.generateStreamId(), 5);
  EXPECT_EQ(handler_.nextStreamId(), 7);
}

TEST_F(ClientStreamStateHandlerTest, WrapsAtMaxStreamId) {
  // RSocket 31-bit client stream ID cap. Allocating the cap value must
  // return it once, then wrap the counter back to 1.
  constexpr uint32_t kMax = (1u << 31) - 1;
  handler_.setNextStreamIdForTest(kMax);
  EXPECT_EQ(handler_.generateStreamId(), kMax);
  EXPECT_EQ(handler_.generateStreamId(), 1);
  EXPECT_EQ(handler_.generateStreamId(), 3);
}

TEST_F(ClientStreamStateHandlerTest, SkipsActiveStreamsAfterWrap) {
  // Make IDs 1, 3, 5 live via real writes, then force a wrap. The
  // post-wrap allocator must skip the live IDs and hand out the next
  // free odd ID (7).
  for (int i = 0; i < 3; ++i) {
    auto request = makeClientRequest(
        folly::IOBuf::copyBuffer("request"),
        folly::IOBuf::copyBuffer("metadata"));
    EXPECT_EQ(
        handler_.onWrite(ctx_, erase_and_box(std::move(request))),
        Result::Success);
  }
  ASSERT_EQ(handler_.activeStreamCount(), 3);
  ASSERT_TRUE(handler_.hasActiveStream(1));
  ASSERT_TRUE(handler_.hasActiveStream(3));
  ASSERT_TRUE(handler_.hasActiveStream(5));

  constexpr uint32_t kMax = (1u << 31) - 1;
  handler_.setNextStreamIdForTest(kMax);
  EXPECT_EQ(handler_.generateStreamId(), kMax);
  EXPECT_EQ(handler_.generateStreamId(), 7);
}

// =============================================================================
// Outbound Write
// =============================================================================

TEST_F(
    ClientStreamStateHandlerTest, OutboundWriteAssignsStreamIdAndStoresState) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("test request"),
      folly::IOBuf::copyBuffer("test metadata"));
  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& writtenMsg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_EQ(getStreamId(writtenMsg), 1);
  EXPECT_EQ(
      writtenMsg.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(handler_.hasActiveStream(1));
  EXPECT_EQ(handler_.activeStreamCount(), 1);
}

TEST_F(ClientStreamStateHandlerTest, MultipleWritesGenerateSequentialOddIds) {
  for (int i = 0; i < 3; ++i) {
    auto request = makeClientRequest(
        folly::IOBuf::copyBuffer("request"),
        folly::IOBuf::copyBuffer("metadata"));
    EXPECT_EQ(
        handler_.onWrite(ctx_, erase_and_box(std::move(request))),
        Result::Success);
  }

  ASSERT_EQ(ctx_.writeMessages().size(), 3);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[0].get<RocketRequestMessage>()), 1);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[1].get<RocketRequestMessage>()), 3);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[2].get<RocketRequestMessage>()), 5);
  EXPECT_EQ(handler_.activeStreamCount(), 3);
}

// =============================================================================
// Terminal Frame Handling (ERROR, CANCEL, complete PAYLOAD)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, TerminalPayloadCleansUpStream) {
  int testHook;
  void* const kTestHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestContext.get(), kTestHandle);
  EXPECT_EQ(
      response.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>())
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::PAYLOAD);
  EXPECT_TRUE(
      apache::thrift::fast_thrift::frame::read::FrameView(
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>())
          .isComplete());
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ClientStreamStateHandlerTest, ErrorFrameIsAlwaysTerminal) {
  int testHook;
  void* const kTestHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // ERROR is terminal even without complete flag
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestContext.get(), kTestHandle);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>())
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ClientStreamStateHandlerTest, CancelFrameIsAlwaysTerminal) {
  int testHook;
  void* const kTestHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // CANCEL is terminal even without complete flag
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::CANCEL, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestContext.get(), kTestHandle);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>())
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::CANCEL);
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

// =============================================================================
// In-Process RocketResponseError Handling (cold path)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, ResponseErrorRoutesViaStreamLookup) {
  int testHook;
  void* const kTestHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  RocketResponseMessage response;
  response.payload = RocketResponseError{
      .ew = folly::make_exception_wrapper<std::runtime_error>("serialize boom"),
      .streamId = 1,
  };

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& forwarded = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(forwarded.requestContext.get(), kTestHandle);
  EXPECT_EQ(
      forwarded.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  ASSERT_TRUE(forwarded.payload.is<RocketResponseError>());
  EXPECT_EQ(
      forwarded.payload.get<RocketResponseError>().ew.what().toStdString(),
      "std::runtime_error: serialize boom");
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

TEST_F(ClientStreamStateHandlerTest, ResponseErrorForUnknownStreamIsDropped) {
  RocketResponseMessage response;
  response.payload = RocketResponseError{
      .ew = folly::make_exception_wrapper<std::runtime_error>("orphan error"),
      .streamId = 999,
  };

  auto result = handler_.onRead(ctx_, erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0);
}

// =============================================================================
// Non-Terminal Frame Handling
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, NonTerminalPayloadKeepsStreamOpen) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  // PAYLOAD without complete flag is non-terminal (streaming scenario).
  // streamType must be stamped on every stream-scoped frame so downstream
  // pattern handlers can dispatch statelessly.
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(
      response.streamType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(handler_.hasActiveStream(1)); // Stream still open
}

TEST_F(
    ClientStreamStateHandlerTest,
    RequestNFramePassesThroughAndKeepsStreamOpen) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  // REQUEST_N is a flow control frame, non-terminal
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_N, 1, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          ctx_.readMessages()[0]
              .get<RocketResponseMessage>()
              .payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>())
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_N);
  EXPECT_TRUE(handler_.hasActiveStream(1)); // Stream still open
}

TEST_F(
    ClientStreamStateHandlerTest, NonTerminalFrameForUnknownStreamIsDropped) {
  // Non-terminal frame for unknown stream should be dropped (same as terminal)
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD, 999, 0)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0); // Dropped with log
}

// =============================================================================
// Unknown Stream ID Handling
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, TerminalFrameForUnknownStreamIsDropped) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          999,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 0); // Dropped with log
}

// =============================================================================
// Connection-Level Frames (streamId == 0)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, ConnectionFramesPassThrough) {
  // SETUP frame
  auto result1 = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::SETUP, 0)));
  EXPECT_EQ(result1, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);

  // KEEPALIVE frame
  auto result2 = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE, 0)));
  EXPECT_EQ(result2, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 2);

  // Connection frames don't create active streams
  EXPECT_EQ(handler_.activeStreamCount(), 0);
}

// =============================================================================
// Handler Lifecycle
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, HandlerRemovedRequiresDrainedSlotMap) {
  // Contract: by the time handlerRemoved runs, the slot map must be
  // empty — onException or onPipelineInactive should have already done
  // the fan-out. handlerRemoved DCHECKs that.
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_GT(handler_.nextStreamId(), 1);
  EXPECT_EQ(handler_.activeStreamCount(), 1);

  // Drain via the legitimate teardown path (onPipelineInactive fan-out).
  handler_.onPipelineInactive(ctx_);
  EXPECT_EQ(handler_.activeStreamCount(), 0);

  handler_.handlerRemoved(ctx_);
}

TEST_F(ClientStreamStateHandlerTest, OnPipelineActiveResetsStreamId) {
  // Stream-id reset belongs on the activate edge: a fresh connection
  // (first activate or post-disconnect reactivate) must start at 1
  // per RSocket spec.
  for (int i = 0; i < 3; ++i) {
    auto request = makeClientRequest(
        folly::IOBuf::copyBuffer("request"),
        folly::IOBuf::copyBuffer("metadata"));
    EXPECT_EQ(
        handler_.onWrite(ctx_, erase_and_box(std::move(request))),
        Result::Success);
  }
  EXPECT_EQ(handler_.nextStreamId(), 7);

  // Disconnect (drains in-flight) → reactivate (resets counter).
  handler_.onPipelineInactive(ctx_);
  EXPECT_EQ(handler_.activeStreamCount(), 0);
  handler_.onPipelineActive(ctx_);
  EXPECT_EQ(handler_.nextStreamId(), 1);

  // Next request on the reactivated connection starts at streamId 1.
  ctx_.writeMessages().clear();
  auto next = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(next))), Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  EXPECT_EQ(
      getStreamId(ctx_.writeMessages()[0].get<RocketRequestMessage>()), 1);
}

TEST_F(
    ClientStreamStateHandlerTest,
    OnPipelineInactiveFansOutEndOfFileToAllStreams) {
  // Graceful close: every active stream gets a per-stream
  // RocketResponseError with TTransportException::END_OF_FILE / "Connection
  // closed", routed inbound via fireRead so the bridge / AppAdapter can
  // resolve each handler with a real transport exception (not the
  // generic auto-detach error). Mirrors legacy
  // RocketClient::onConnectionClosed semantics.
  int hook1, hook2;
  void* const kHandle1 = &hook1;
  void* const kHandle2 = &hook2;
  auto r1 = makeClientRequest(
      folly::IOBuf::copyBuffer("a"), folly::IOBuf::copyBuffer("m"), kHandle1);
  auto r2 = makeClientRequest(
      folly::IOBuf::copyBuffer("b"), folly::IOBuf::copyBuffer("m"), kHandle2);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(r1))), Result::Success);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(r2))), Result::Success);
  EXPECT_EQ(handler_.activeStreamCount(), 2);
  ctx_.readMessages().clear();

  handler_.onPipelineInactive(ctx_);

  EXPECT_EQ(handler_.activeStreamCount(), 0);
  EXPECT_FALSE(ctx_.hasException());
  ASSERT_EQ(ctx_.readMessages().size(), 2);

  std::vector<uint32_t> seenStreamIds;
  std::vector<void*> seenHandles;
  for (auto& boxed : ctx_.readMessages()) {
    auto& response = boxed.get<RocketResponseMessage>();
    ASSERT_TRUE(response.payload.is<RocketResponseError>());
    auto& err = response.payload.get<RocketResponseError>();
    auto* tex =
        err.ew.get_exception<apache::thrift::transport::TTransportException>();
    ASSERT_NE(tex, nullptr);
    EXPECT_EQ(
        tex->getType(),
        apache::thrift::transport::TTransportException::END_OF_FILE);
    EXPECT_NE(
        std::string(tex->what()).find("Connection closed"), std::string::npos);
    seenStreamIds.push_back(err.streamId);
    seenHandles.push_back(response.requestContext.get());
  }
  std::sort(seenStreamIds.begin(), seenStreamIds.end());
  std::sort(seenHandles.begin(), seenHandles.end());
  EXPECT_EQ(seenStreamIds, (std::vector<uint32_t>{1u, 3u}));
  std::vector<void*> expectedHandles{kHandle1, kHandle2};
  std::sort(expectedHandles.begin(), expectedHandles.end());
  EXPECT_EQ(seenHandles, expectedHandles);
}

TEST_F(ClientStreamStateHandlerTest, OnPipelineInactiveAfterOnExceptionIsNoOp) {
  // onException already drained the slot map; a follow-up
  // onPipelineInactive (e.g., from pipeline_->close()) finds nothing to
  // do.
  int hook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("a"), folly::IOBuf::copyBuffer("m"), &hook);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_EQ(handler_.activeStreamCount(), 1);

  auto ex =
      folly::make_exception_wrapper<std::runtime_error>("connection died");
  handler_.onException(ctx_, folly::exception_wrapper(ex));
  EXPECT_EQ(handler_.activeStreamCount(), 0);
  size_t readsBefore = ctx_.readMessages().size();

  handler_.onPipelineInactive(ctx_);
  EXPECT_EQ(ctx_.readMessages().size(), readsBefore);
}

// =============================================================================
// Exception Handling
// =============================================================================

TEST_F(
    ClientStreamStateHandlerTest, OnExceptionFansOutToAllStreamsAndPropagates) {
  // onException fans out per-stream errors with the wrapped reason
  // verbatim — matches legacy RocketClient::failAllSentWrites semantics
  // (each in-flight request resolves with the actual connection
  // exception, not the generic auto-detach error). After the fan-out it
  // propagates the exception upstream so the AppAdapter can flip state
  // to Closed.
  int hook1, hook2;
  void* const kHandle1 = &hook1;
  void* const kHandle2 = &hook2;
  auto r1 = makeClientRequest(
      folly::IOBuf::copyBuffer("a"), folly::IOBuf::copyBuffer("m"), kHandle1);
  auto r2 = makeClientRequest(
      folly::IOBuf::copyBuffer("b"), folly::IOBuf::copyBuffer("m"), kHandle2);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(r1))), Result::Success);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(r2))), Result::Success);
  EXPECT_EQ(handler_.activeStreamCount(), 2);
  ctx_.readMessages().clear();

  auto ex =
      folly::make_exception_wrapper<std::runtime_error>("connection died");
  handler_.onException(ctx_, folly::exception_wrapper(ex));

  EXPECT_EQ(handler_.activeStreamCount(), 0);
  EXPECT_TRUE(ctx_.hasException());
  EXPECT_EQ(
      ctx_.exception().what().toStdString(),
      "std::runtime_error: connection died");

  ASSERT_EQ(ctx_.readMessages().size(), 2);
  for (auto& boxed : ctx_.readMessages()) {
    auto& response = boxed.get<RocketResponseMessage>();
    ASSERT_TRUE(response.payload.is<RocketResponseError>());
    auto& err = response.payload.get<RocketResponseError>();
    EXPECT_EQ(
        err.ew.what().toStdString(), "std::runtime_error: connection died");
  }
}

TEST_F(ClientStreamStateHandlerTest, OnWriteErrorRollbackEmitsNotOpen) {
  // Rollback path: a downstream handler returns Result::Error from
  // fireWrite (e.g., codec serialize failed). The slot was inserted
  // before fireWrite, so rollback must fan out a per-stream NOT_OPEN
  // error so the in-flight handler resolves with a real transport
  // exception (not the generic auto-detach). Mirrors legacy
  // RocketClient::failAllScheduledWrites.
  int testHook;
  void* const kHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kHandle);

  // fireWrite fails, fireRead (called from the rollback fan-out) succeeds
  // so the test can capture the synthesized RocketResponseError.
  ctx_.setWriteReturnError(true);
  auto result = handler_.onWrite(ctx_, erase_and_box(std::move(request)));
  EXPECT_EQ(result, Result::Error);

  EXPECT_EQ(handler_.activeStreamCount(), 0);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestContext.get(), kHandle);
  ASSERT_TRUE(response.payload.is<RocketResponseError>());
  auto& err = response.payload.get<RocketResponseError>();
  EXPECT_EQ(err.streamId, 1);
  auto* tex =
      err.ew.get_exception<apache::thrift::transport::TTransportException>();
  ASSERT_NE(tex, nullptr);
  EXPECT_EQ(
      tex->getType(), apache::thrift::transport::TTransportException::NOT_OPEN);
  EXPECT_NE(
      std::string(tex->what()).find("Dropping unsent request"),
      std::string::npos);
  EXPECT_NE(
      std::string(tex->what()).find("Pipeline write failed"),
      std::string::npos);
}

// =============================================================================
// Backpressure and Error Propagation
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, DownstreamResultPropagatedOnRead) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  ctx_.setReturnBackpressure(true);
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(ClientStreamStateHandlerTest, DownstreamResultPropagatedOnWrite) {
  ctx_.setReturnBackpressure(true);
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("test"), folly::IOBuf::copyBuffer("metadata"));
  auto result1 = handler_.onWrite(ctx_, erase_and_box(std::move(request1)));
  EXPECT_EQ(result1, Result::Backpressure);

  ctx_.setReturnBackpressure(false);
  ctx_.setReturnError(true);
  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("test"), folly::IOBuf::copyBuffer("metadata"));
  auto result2 = handler_.onWrite(ctx_, erase_and_box(std::move(request2)));
  EXPECT_EQ(result2, Result::Error);
}

TEST_F(ClientStreamStateHandlerTest, DownstreamErrorPropagatedOnRead) {
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);

  ctx_.setReturnError(true);
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Error);
}

// =============================================================================
// Request Context Propagation
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, RequestHandleReturnedInResponse) {
  int testHook;
  void* const kTestHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // Send terminal frame and verify handle is returned
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::PAYLOAD,
          1,
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& response = ctx_.readMessages()[0].get<RocketResponseMessage>();
  EXPECT_EQ(response.requestContext.get(), kTestHandle);
  EXPECT_FALSE(handler_.hasActiveStream(1));
}

// =============================================================================
// Stream ID Collision Detection (Death Test)
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, DuplicateStreamIdCrashes) {
  // Send a request to create stream ID 1
  auto request1 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request1))),
      Result::Success);
  EXPECT_TRUE(handler_.hasActiveStream(1));

  // Manually reset the stream ID counter to create a duplicate
  handler_.nextStreamId_ = 1;

  auto request2 = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"));

#ifndef NDEBUG
  // DCHECK fires only in debug builds
  EXPECT_DEATH(
      (void)handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      "Stream ID 1 already exists in active streams");
#else
  // In opt builds, DCHECK is a no-op so the write succeeds
  EXPECT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request2))),
      Result::Success);
#endif
}

// =============================================================================
// Connection Frame Edge Cases
// =============================================================================

TEST_F(ClientStreamStateHandlerTest, ErrorFrameOnStreamZeroPassesThrough) {
  // Connection-level ERROR frame (stream ID 0) should pass through
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(makeRocketResponse(
          apache::thrift::fast_thrift::frame::FrameType::ERROR, 0, 0)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(
      apache::thrift::fast_thrift::frame::read::FrameView(
          ctx_.readMessages()[0]
              .get<RocketResponseMessage>()
              .payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>())
          .type(),
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
}

// =============================================================================
// Write Completion (onEvent: FrameWriteComplete -> RocketWriteComplete)
// =============================================================================

// A FrameWriteComplete for a live stream resolves the streamId to its request
// context and fires RocketWriteComplete carrying that context and the status.
TEST_F(
    ClientStreamStateHandlerTest,
    OnEventFiresRocketWriteCompleteForKnownStream) {
  int testHook;
  void* const kTestHandle = &testHook;
  auto request = makeClientRequest(
      folly::IOBuf::copyBuffer("request"),
      folly::IOBuf::copyBuffer("metadata"),
      kTestHandle);
  ASSERT_EQ(
      handler_.onWrite(ctx_, erase_and_box(std::move(request))),
      Result::Success);
  ASSERT_TRUE(handler_.hasActiveStream(1));

  handler_.onEvent(
      ctx_,
      RocketClientStreamStateHandler::EventId::FrameWriteComplete,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          FrameWriteCompleteEvent{
              .streamId = 1,
              .status = apache::thrift::fast_thrift::transport::
                  WriteCompletionStatus::Success,
          }));

  ASSERT_EQ(ctx_.firedEvents().size(), 1);
  EXPECT_EQ(
      ctx_.firedEvents()[0].first,
      RocketClientStreamStateHandler::EventId::RocketWriteComplete);
  auto& event = ctx_.firedEvents()[0].second.get<RocketWriteCompleteEvent>();
  EXPECT_EQ(event.requestContext, kTestHandle);
  EXPECT_EQ(
      event.status,
      apache::thrift::fast_thrift::transport::WriteCompletionStatus::Success);
  // Write completion does not terminate the stream; the slot stays live.
  EXPECT_TRUE(handler_.hasActiveStream(1));
}

// A FrameWriteComplete whose streamId has no live slot (already terminated or
// never seen) is silently dropped — no RocketWriteComplete is fired.
TEST_F(ClientStreamStateHandlerTest, OnEventForUnknownStreamFiresNothing) {
  handler_.onEvent(
      ctx_,
      RocketClientStreamStateHandler::EventId::FrameWriteComplete,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
          FrameWriteCompleteEvent{
              .streamId = 999,
              .status = apache::thrift::fast_thrift::transport::
                  WriteCompletionStatus::Success,
          }));

  EXPECT_TRUE(ctx_.firedEvents().empty());
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
