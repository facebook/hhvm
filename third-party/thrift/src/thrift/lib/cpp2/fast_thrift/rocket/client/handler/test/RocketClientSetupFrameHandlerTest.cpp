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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

/**
 * MockContext for testing RocketClientSetupFrameHandler.
 *
 * Captures messages fired via fireRead() and fireWrite(),
 * and exceptions via fireException().
 */
class MockContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    writeMessages_.push_back(std::move(msg));
    return writeResult_;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void close() noexcept { closeCalled_ = true; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  bool closeCalled() const { return closeCalled_; }

  void setWriteResult(Result result) { writeResult_ = result; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    closeCalled_ = false;
    writeResult_ = Result::Success;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  bool closeCalled_{false};
  Result writeResult_{Result::Success};
};

// Helper to create a KEEPALIVE frame
apache::thrift::fast_thrift::frame::read::ParsedFrame makeKeepAliveFrame(
    bool respond = false) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::KeepAliveHeader{
          .lastReceivedPosition = 0, .respond = respond},
      nullptr);
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
}

// Helper to create a PAYLOAD frame on a specific stream
apache::thrift::fast_thrift::frame::read::ParsedFrame makePayloadFrame(
    uint32_t streamId, std::unique_ptr<folly::IOBuf> data) {
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = streamId,
          .follows = false,
          .complete = true,
          .next = true},
      nullptr,
      std::move(data));
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));
}

// Default factory function for tests
auto makeDefaultFactory() {
  return []() {
    return std::make_pair(
        folly::IOBuf::copyBuffer("setup metadata"),
        folly::IOBuf::copyBuffer("setup data"));
  };
}

// Helper to serialize the SETUP payload held in the variant for wire-byte
// inspection. After this call the held payload's data/metadata are moved out.
std::unique_ptr<folly::IOBuf> serializeSetupFrame(RocketRequestMessage& msg) {
  return std::move(
             msg.frame
                 .get<apache::thrift::fast_thrift::frame::ComposedSetupFrame>())
      .serialize();
}

} // namespace

class ClientSetupFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  MockContext ctx_;
};

// =============================================================================
// onConnect Behavior
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, OnConnectWritesSetupFrame) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_NE(serializeSetupFrame(msg), nullptr);
  EXPECT_TRUE(
      msg.frame.is<apache::thrift::fast_thrift::frame::ComposedSetupFrame>());
}

TEST_F(ClientSetupFrameHandlerTest, OnConnectClosesOnWriteFailure) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());
  ctx_.setWriteResult(Result::Error);

  handler.onPipelineActive(ctx_);

  EXPECT_TRUE(ctx_.closeCalled());
}

TEST_F(ClientSetupFrameHandlerTest, OnConnectIsIdempotent) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // First call should send setup frame
  handler.onPipelineActive(ctx_);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  // Second call should not send another frame
  handler.onPipelineActive(ctx_);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);

  // Third call should also be a no-op
  handler.onPipelineActive(ctx_);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
}

TEST_F(ClientSetupFrameHandlerTest, FactoryIsCalledOnConnect) {
  bool factoryCalled = false;
  RocketClientSetupFrameHandler handler([&factoryCalled]() {
    factoryCalled = true;
    return std::make_pair(
        folly::IOBuf::copyBuffer("meta"), folly::IOBuf::copyBuffer("data"));
  });

  EXPECT_FALSE(factoryCalled);
  handler.onPipelineActive(ctx_);
  EXPECT_TRUE(factoryCalled);
}

// =============================================================================
// Setup Frame Format Verification
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, SetupFrameHasCorrectHeaderFormat) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  auto coalesced = serializeSetupFrame(msg)->cloneCoalesced();
  folly::io::Cursor cursor(coalesced.get());

  // Stream ID (4 bytes)
  uint32_t streamId = cursor.readBE<uint32_t>();
  EXPECT_EQ(streamId, 0);

  // Type + Flags (2 bytes)
  uint16_t typeAndFlags = cursor.readBE<uint16_t>();
  uint8_t frameType = typeAndFlags >> 10;
  EXPECT_EQ(
      static_cast<apache::thrift::fast_thrift::frame::FrameType>(frameType),
      apache::thrift::fast_thrift::frame::FrameType::SETUP);

  // Major version (2 bytes)
  uint16_t majorVersion = cursor.readBE<uint16_t>();
  EXPECT_EQ(majorVersion, RocketClientSetupFrameHandler::kRSocketMajorVersion);

  // Minor version (2 bytes)
  uint16_t minorVersion = cursor.readBE<uint16_t>();
  EXPECT_EQ(minorVersion, RocketClientSetupFrameHandler::kRSocketMinorVersion);

  // Keepalive time (4 bytes)
  uint32_t keepaliveTime = cursor.readBE<uint32_t>();
  EXPECT_EQ(keepaliveTime, RocketClientSetupFrameHandler::kMaxKeepaliveTime);

  // Max lifetime (4 bytes)
  uint32_t maxLifetime = cursor.readBE<uint32_t>();
  EXPECT_EQ(maxLifetime, RocketClientSetupFrameHandler::kMaxLifetime);

  // Metadata MIME type (length + string)
  uint8_t metadataMimeLen = cursor.read<uint8_t>();
  EXPECT_EQ(metadataMimeLen, 36u);
  std::string metadataMime;
  metadataMime.resize(metadataMimeLen);
  cursor.pull(metadataMime.data(), metadataMimeLen);
  EXPECT_EQ(metadataMime, "application/x-rocket-metadata+binary");

  // Data MIME type (length + string)
  uint8_t dataMimeLen = cursor.read<uint8_t>();
  EXPECT_EQ(dataMimeLen, 28u);
  std::string dataMime;
  dataMime.resize(dataMimeLen);
  cursor.pull(dataMime.data(), dataMimeLen);
  EXPECT_EQ(dataMime, "application/x-rocket-payload");
}

TEST_F(ClientSetupFrameHandlerTest, SetupFrameIncludesMetadataAndData) {
  const std::string testMetadata = "test metadata content";
  const std::string testData = "test data content";

  RocketClientSetupFrameHandler handler([&]() {
    return std::make_pair(
        folly::IOBuf::copyBuffer(testMetadata),
        folly::IOBuf::copyBuffer(testData));
  });

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  // Frame should be larger than just metadata + data (header overhead)
  EXPECT_GT(
      serializeSetupFrame(msg)->computeChainDataLength(),
      testMetadata.size() + testData.size());
}

// =============================================================================
// Frames Pass Through
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, KeepAliveFramePassesThrough) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  auto frame = makeKeepAliveFrame();
  auto result = handler.onRead(ctx_, erase_and_box(std::move(frame)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ClientSetupFrameHandlerTest, PayloadFramePassesThrough) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  auto frame = makePayloadFrame(1, folly::IOBuf::copyBuffer("payload data"));
  auto result = handler.onRead(ctx_, erase_and_box(std::move(frame)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);
}

// =============================================================================
// Exception Handling
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, OnExceptionPassesThrough) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler.onException(ctx_, std::move(ex));

  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Null/Empty Metadata and Data
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, NullMetadataStillSendsFrame) {
  RocketClientSetupFrameHandler handler([]() {
    return std::make_pair(nullptr, folly::IOBuf::copyBuffer("data only"));
  });

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_NE(serializeSetupFrame(msg), nullptr);
}

TEST_F(ClientSetupFrameHandlerTest, NullDataStillSendsFrame) {
  RocketClientSetupFrameHandler handler([]() {
    return std::make_pair(folly::IOBuf::copyBuffer("metadata only"), nullptr);
  });

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_NE(serializeSetupFrame(msg), nullptr);
}

TEST_F(ClientSetupFrameHandlerTest, NullMetadataAndDataStillSendsFrame) {
  RocketClientSetupFrameHandler handler(
      []() { return std::make_pair(nullptr, nullptr); });

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& msg = ctx_.writeMessages()[0].get<RocketRequestMessage>();
  EXPECT_NE(serializeSetupFrame(msg), nullptr);
}

TEST_F(ClientSetupFrameHandlerTest, EmptyMetadataStillSendsFrame) {
  RocketClientSetupFrameHandler handler([]() {
    return std::make_pair(
        folly::IOBuf::create(0), folly::IOBuf::copyBuffer("data"));
  });

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
}

TEST_F(ClientSetupFrameHandlerTest, EmptyDataStillSendsFrame) {
  RocketClientSetupFrameHandler handler([]() {
    return std::make_pair(
        folly::IOBuf::copyBuffer("metadata"), folly::IOBuf::create(0));
  });

  handler.onPipelineActive(ctx_);

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
}

// =============================================================================
// Handler Lifecycle
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, HandlerAddedIsNoOp) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // Should not throw or crash
  handler.handlerAdded(ctx_);
}

TEST_F(ClientSetupFrameHandlerTest, HandlerRemovedIsNoOp) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // Should not throw or crash
  handler.handlerRemoved(ctx_);
}

// =============================================================================
// Disconnect and Reconnection Behavior
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, OnDisconnectResetsSetupState) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // First connect sends setup
  handler.onPipelineActive(ctx_);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  // Second connect should be no-op (idempotent)
  handler.onPipelineActive(ctx_);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);

  // Disconnect resets state
  handler.onPipelineInactive(ctx_);

  // After disconnect, next connect should send setup again
  handler.onPipelineActive(ctx_);
  EXPECT_EQ(ctx_.writeMessages().size(), 2);
}

TEST_F(ClientSetupFrameHandlerTest, OnDisconnectAllowsMultipleReconnections) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // Simulate multiple connect/disconnect cycles
  for (int i = 0; i < 3; ++i) {
    handler.onPipelineActive(ctx_);
    handler.onPipelineInactive(ctx_);
  }

  // Should have sent 3 setup frames
  EXPECT_EQ(ctx_.writeMessages().size(), 3);

  // Verify all are SETUP frames
  for (auto& msg : ctx_.writeMessages()) {
    auto& rocketMsg = msg.get<RocketRequestMessage>();
    EXPECT_TRUE(
        rocketMsg.frame
            .is<apache::thrift::fast_thrift::frame::ComposedSetupFrame>());
  }
}

TEST_F(ClientSetupFrameHandlerTest, OnDisconnectBeforeConnectIsNoOp) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // Disconnect before any connect should not crash
  handler.onPipelineInactive(ctx_);

  // First connect should still work
  handler.onPipelineActive(ctx_);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
}

// =============================================================================
// OutboundHandler Pass-Through
// =============================================================================

TEST_F(ClientSetupFrameHandlerTest, OnWritePassesThrough) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  auto data = folly::IOBuf::copyBuffer("test data");
  RocketRequestMessage msg{
      .frame =
          apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame{
              .data = std::move(data),
          },
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = handler.onWrite(ctx_, erase_and_box(std::move(msg)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
}

TEST_F(ClientSetupFrameHandlerTest, OnWriteReadyIsNoOp) {
  RocketClientSetupFrameHandler handler(makeDefaultFactory());

  // Should not throw or crash
  handler.onWriteReady(ctx_);
}

} // namespace apache::thrift::fast_thrift::rocket::client::handler
