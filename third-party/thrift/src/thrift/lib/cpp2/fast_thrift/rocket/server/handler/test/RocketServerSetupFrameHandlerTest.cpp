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

#include <cstring>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator
    g_allocator;

apache::thrift::fast_thrift::channel_pipeline::BytesPtr allocate(size_t size) {
  return g_allocator.allocate(size);
}

apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
    folly::StringPiece s) {
  auto buf = g_allocator.allocate(s.size());
  std::memcpy(buf->writableData(), s.data(), s.size());
  buf->append(s.size());
  return buf;
}

/**
 * MockContext for testing RocketServerSetupFrameHandler.
 *
 * Captures frames and messages fired via fireRead() and fireWrite(),
 * and exceptions via fireException().
 */
class MockContext {
 public:
  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (readResult_ != Result::Success) {
      return readResult_;
    }
    readMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (writeResult_ != Result::Success) {
      return writeResult_;
    }
    writeMessages_.push_back(std::move(msg));
    return Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exception_ = std::move(e);
  }

  void deactivate() noexcept { disconnectCalled_ = true; }

  void close() noexcept { closeCalled_ = true; }

  // Setup completion is announced as an event carrying a pointer, so a
  // subscriber can fill its reject slot in place. The hook lets a test stand
  // in for that subscriber.
  using OnEventFn =
      std::function<void(RocketServerEventId, const TypeErasedBox&)>;

  void setOnEvent(OnEventFn fn) { onEvent_ = std::move(fn); }

  void fireEvent(RocketServerEventId ev, TypeErasedBox&& evt) noexcept {
    firedEvents_.push_back(ev);
    if (onEvent_) {
      onEvent_(ev, evt);
    }
  }

  const std::vector<RocketServerEventId>& firedEvents() const {
    return firedEvents_;
  }

  folly::EventBase* eventBase() noexcept { return &evb_; }

  // NOLINTNEXTLINE(clang-diagnostic-unused-member-function)
  apache::thrift::fast_thrift::channel_pipeline::BytesPtr allocate(
      size_t size) {
    return g_allocator.allocate(size);
  }

  apache::thrift::fast_thrift::channel_pipeline::BytesPtr copyBuffer(
      const void* data, size_t len) noexcept {
    auto buf = g_allocator.allocate(len);
    std::memcpy(buf->writableData(), data, len);
    buf->append(len);
    return buf;
  }

  void setReadResult(Result result) { readResult_ = result; }
  void setWriteResult(Result result) { writeResult_ = result; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  bool hasException() const { return static_cast<bool>(exception_); }

  bool disconnectCalled() const { return disconnectCalled_; }

  bool closeCalled() const { return closeCalled_; }

  bool writeReadyCalled() const { return writeReadyCalled_; }

  void reset() {
    readMessages_.clear();
    writeMessages_.clear();
    exception_ = folly::exception_wrapper();
    readResult_ = Result::Success;
    writeResult_ = Result::Success;
    disconnectCalled_ = false;
    closeCalled_ = false;
    firedEvents_.clear();
    onEvent_ = nullptr;
    writeReadyCalled_ = false;
  }

 private:
  std::vector<TypeErasedBox> readMessages_;
  std::vector<TypeErasedBox> writeMessages_;
  folly::exception_wrapper exception_;
  Result readResult_{Result::Success};
  Result writeResult_{Result::Success};
  bool disconnectCalled_{false};
  bool closeCalled_{false};
  std::vector<RocketServerEventId> firedEvents_;
  OnEventFn onEvent_;
  folly::EventBase evb_;
  bool writeReadyCalled_{false};
};

// Build a raw frame buffer for non-SETUP frame types (reused from
// ServerStreamStateHandlerTest)
std::unique_ptr<folly::IOBuf> buildFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  const auto& desc = apache::thrift::fast_thrift::frame::getDescriptor(type);
  size_t headerSize = desc.headerSize > 0
      ? desc.headerSize
      : apache::thrift::fast_thrift::frame::kBaseHeaderSize;

  auto buf = allocate(headerSize);
  auto* data = buf->writableData();
  std::memset(data, 0, headerSize);

  data[0] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  data[1] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  data[2] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  data[3] = static_cast<uint8_t>(streamId & 0xFF);

  // upper 6 bits = type, lower 10 bits = flags
  uint16_t typeAndFlags =
      (static_cast<uint16_t>(type)
       << ::apache::thrift::fast_thrift::frame::detail::kFlagsBits) |
      flags;
  data[4] = static_cast<uint8_t>((typeAndFlags >> 8) & 0xFF);
  data[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  buf->append(headerSize);
  return buf;
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeTestFrame(
    apache::thrift::fast_thrift::frame::FrameType type,
    uint32_t streamId,
    uint16_t flags = 0) {
  return apache::thrift::fast_thrift::frame::read::parseFrame(
      buildFrame(type, streamId, flags));
}

apache::thrift::fast_thrift::frame::read::ParsedFrame makeSetupFrame(
    uint16_t majorVersion = 1,
    uint16_t minorVersion = 0,
    uint32_t keepaliveTime = 30000,
    uint32_t maxLifetime = 60000,
    bool lease = false) {
  apache::thrift::fast_thrift::frame::write::SetupHeader header{
      .majorVersion = majorVersion,
      .minorVersion = minorVersion,
      .keepaliveTime = keepaliveTime,
      .maxLifetime = maxLifetime,
      .lease = lease,
  };
  auto buf = apache::thrift::fast_thrift::frame::write::serialize(
      header, nullptr, nullptr);
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
}

// Build a SETUP frame with a caller-specified metadata MIME type. The
// stock writer hardcodes "+binary"; this helper exists so handler tests can
// exercise the negotiation path with arbitrary MIME strings (e.g. "+compact"
// or unknown MIMEs that should fall back to Binary).
apache::thrift::fast_thrift::frame::read::ParsedFrame
makeSetupFrameWithMetadataMimeType(std::string_view metadataMime) {
  static constexpr std::string_view kDataMime{"application/x-rocket-payload"};
  // Frame layout: streamId(4) + typeAndFlags(2) + version(4) + keepalive(4)
  // + lifetime(4) + mimeLen(1) + metadataMime + dataMimeLen(1) + dataMime.
  // No M flag — SETUP carries no payload.
  const size_t totalSize =
      6 + 12 + 1 + metadataMime.size() + 1 + kDataMime.size();
  auto buf = folly::IOBuf::create(totalSize);
  auto* p = buf->writableData();
  std::memset(p, 0, totalSize);

  // streamId = 0 (always for SETUP); upper 6 bits of next u16 = SETUP type.
  const uint16_t typeAndFlags =
      static_cast<uint16_t>(
          apache::thrift::fast_thrift::frame::FrameType::SETUP)
      << apache::thrift::fast_thrift::frame::detail::kFlagsBits;
  p[4] = static_cast<uint8_t>(typeAndFlags >> 8);
  p[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);

  // Major version = 1, minor version = 0.
  p[6] = 0;
  p[7] = 1;
  // Keepalive = 30000ms, maxLifetime = 60000ms.
  uint32_t keepalive = 30000;
  uint32_t lifetime = 60000;
  for (int i = 0; i < 4; ++i) {
    p[10 + i] = static_cast<uint8_t>((keepalive >> (24 - 8 * i)) & 0xFF);
    p[14 + i] = static_cast<uint8_t>((lifetime >> (24 - 8 * i)) & 0xFF);
  }

  // Metadata MIME (length-prefixed string).
  p[18] = static_cast<uint8_t>(metadataMime.size());
  std::memcpy(p + 19, metadataMime.data(), metadataMime.size());
  // Data MIME.
  size_t dataMimeOffset = 19 + metadataMime.size();
  p[dataMimeOffset] = static_cast<uint8_t>(kDataMime.size());
  std::memcpy(p + dataMimeOffset + 1, kDataMime.data(), kDataMime.size());

  buf->append(totalSize);
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
}

// A SETUP frame whose declared metadata length runs past the payload — the
// parser must reject it rather than read off the end.
apache::thrift::fast_thrift::frame::read::ParsedFrame
makeSetupFrameWithTruncatedMetadata() {
  static constexpr std::string_view kMetaMime{
      "application/x-rocket-metadata+compact"};
  static constexpr std::string_view kDataMime{"application/x-rocket-payload"};
  const size_t totalSize =
      6 + 12 + 1 + kMetaMime.size() + 1 + kDataMime.size() + 3;
  auto buf = folly::IOBuf::create(totalSize);
  auto* p = buf->writableData();
  std::memset(p, 0, totalSize);

  const uint16_t typeAndFlags =
      (static_cast<uint16_t>(
           apache::thrift::fast_thrift::frame::FrameType::SETUP)
       << apache::thrift::fast_thrift::frame::detail::kFlagsBits) |
      apache::thrift::fast_thrift::frame::detail::kMetadataBit;
  p[4] = static_cast<uint8_t>(typeAndFlags >> 8);
  p[5] = static_cast<uint8_t>(typeAndFlags & 0xFF);
  p[7] = 1;
  uint32_t keepalive = 30000;
  uint32_t lifetime = 60000;
  for (int i = 0; i < 4; ++i) {
    p[10 + i] = static_cast<uint8_t>((keepalive >> (24 - 8 * i)) & 0xFF);
    p[14 + i] = static_cast<uint8_t>((lifetime >> (24 - 8 * i)) & 0xFF);
  }

  size_t off = 18;
  p[off++] = static_cast<uint8_t>(kMetaMime.size());
  std::memcpy(p + off, kMetaMime.data(), kMetaMime.size());
  off += kMetaMime.size();
  p[off++] = static_cast<uint8_t>(kDataMime.size());
  std::memcpy(p + off, kDataMime.data(), kDataMime.size());
  off += kDataMime.size();
  p[off++] = 0;
  p[off++] = 0;
  p[off++] = 1;

  buf->append(totalSize);
  return apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
}

} // namespace

class ServerSetupFrameHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  Result callOnRead(
      apache::thrift::fast_thrift::frame::read::ParsedFrame frame) {
    rocket::server::RocketRequestMessage msg;
    msg.frame = std::move(frame);
    return handler_.onRead(ctx_, erase_and_box(std::move(msg)));
  }

  Result callOnWrite(TypeErasedBox msg) {
    return handler_.onWrite(ctx_, std::move(msg));
  }

  /// Complete a valid setup so tests can exercise post-setup behavior.
  void completeSetup() {
    auto result = callOnRead(makeSetupFrame());
    ASSERT_EQ(result, Result::Success);
    ASSERT_TRUE(handler_.isSetupComplete());
    ctx_.reset();
  }

  MockContext ctx_;
  RocketServerSetupFrameHandler handler_;
};

// =============================================================================
// Setup Success Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, ValidSetupFrameCompletesSetup) {
  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_FALSE(ctx_.closeCalled());

  const auto& params = handler_.setupParameters();
  EXPECT_EQ(params.majorVersion, 1);
  EXPECT_EQ(params.minorVersion, 0);
  EXPECT_EQ(params.keepaliveTime, 30000);
  EXPECT_EQ(params.maxLifetime, 60000);
  EXPECT_FALSE(params.hasLease);

  // SETUP is forwarded up: what to answer it with is not this layer's
  // decision.
  EXPECT_EQ(ctx_.readMessages().size(), 1);
}

TEST_F(ServerSetupFrameHandlerTest, ValidSetupFrameWithLeaseFlag) {
  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000, /*lease=*/true));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_TRUE(handler_.setupParameters().hasLease);
}

// =============================================================================
// Metadata MIME-type negotiation
// =============================================================================
//
// SETUP frames advertise the wire encoding of per-RPC RpcMetadata via the
// metadata MIME type. The handler parses it once at SETUP, stores the result
// on SetupParameters, and (when wired in) publishes it to the
// RocketServerAppAdapter so the upper-layer thrift adapter can pick the
// matching reader/writer per request.

TEST_F(ServerSetupFrameHandlerTest, BinaryMimeTypeYieldsBinaryFlag) {
  auto result = callOnRead(makeSetupFrameWithMetadataMimeType(
      apache::thrift::fast_thrift::rocket::server::kMetadataBinaryMimeType));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(
      handler_.setupParameters().metadataProtocol,
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
}

TEST_F(ServerSetupFrameHandlerTest, CompactMimeTypeYieldsCompactFlag) {
  auto result = callOnRead(makeSetupFrameWithMetadataMimeType(
      apache::thrift::fast_thrift::rocket::server::kMetadataCompactMimeType));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(
      handler_.setupParameters().metadataProtocol,
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::COMPACT);
}

// Unknown MIME types must NOT silently switch to compact. Falling back to
// Binary preserves the prior hardcoded behavior for clients that don't
// advertise a recognized MIME (matches today's wire assumption).
TEST_F(ServerSetupFrameHandlerTest, UnknownMimeTypeFallsBackToBinary) {
  auto result = callOnRead(makeSetupFrameWithMetadataMimeType("text/plain"));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(
      handler_.setupParameters().metadataProtocol,
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::BINARY);
}

// A valid SETUP is forwarded up rather than consumed: deciding what to answer
// with needs the setup metadata interpreted, which is not this layer's job.
// The negotiated parameters are recorded on the way past.
TEST_F(ServerSetupFrameHandlerTest, ForwardsSetupUpWithNegotiatedParameters) {
  auto result = handler_.onRead(
      ctx_,
      erase_and_box(
          rocket::server::RocketRequestMessage{
              .frame = makeSetupFrameWithMetadataMimeType(
                  apache::thrift::fast_thrift::rocket::server::
                      kMetadataCompactMimeType),
          }));

  EXPECT_EQ(result, Result::Success);
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_EQ(
      handler_.setupParameters().metadataProtocol,
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::COMPACT);
  // Forwarded, not answered here: nothing is written at this layer.
  ASSERT_EQ(ctx_.readMessages().size(), 1);
  EXPECT_EQ(ctx_.writeMessages().size(), 0);
}

// Empty callback is the legacy code path (unwired test fixtures, etc.). Must
// not crash and must still populate setupParameters() for embedders that
// inspect them directly.
TEST_F(ServerSetupFrameHandlerTest, EmptyCallbackIsSafe) {
  RocketServerSetupFrameHandler unwired;
  auto result = unwired.onRead(
      ctx_,
      erase_and_box(
          rocket::server::RocketRequestMessage{
              .frame = makeSetupFrameWithMetadataMimeType(
                  apache::thrift::fast_thrift::rocket::server::
                      kMetadataCompactMimeType),
          }));
  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(
      unwired.setupParameters().metadataProtocol,
      apache::thrift::fast_thrift::rocket::server::MetadataProtocol::COMPACT);
}

// =============================================================================
// Version Validation Tests
// =============================================================================

TEST_F(
    ServerSetupFrameHandlerTest,
    SetupWithMajorVersionZeroReturnsUnsupportedSetup) {
  auto result = callOnRead(makeSetupFrame(0, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp = ctx_.writeMessages()[0]
                      .get<apache::thrift::fast_thrift::rocket::server::
                               RocketResponseMessage>();
  const auto& errPayload = errResp.frame;
  EXPECT_EQ(
      errPayload.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      errPayload.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::UNSUPPORTED_SETUP));
}

TEST_F(
    ServerSetupFrameHandlerTest,
    SetupWithNonZeroMinorVersionReturnsUnsupportedSetup) {
  auto result = callOnRead(makeSetupFrame(1, 1, 30000, 60000));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp = ctx_.writeMessages()[0]
                      .get<apache::thrift::fast_thrift::rocket::server::
                               RocketResponseMessage>();
  const auto& errPayload = errResp.frame;
  EXPECT_EQ(
      errPayload.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      errPayload.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::UNSUPPORTED_SETUP));
}

// =============================================================================
// Timer Validation Tests
// =============================================================================

TEST_F(
    ServerSetupFrameHandlerTest,
    SetupWithZeroKeepaliveTimeReturnsInvalidSetup) {
  auto result = callOnRead(makeSetupFrame(1, 0, 0, 60000));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp = ctx_.writeMessages()[0]
                      .get<apache::thrift::fast_thrift::rocket::server::
                               RocketResponseMessage>();
  const auto& errPayload = errResp.frame;
  EXPECT_EQ(
      errPayload.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      errPayload.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP));
}

TEST_F(
    ServerSetupFrameHandlerTest, SetupWithZeroMaxLifetimeReturnsInvalidSetup) {
  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 0));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp = ctx_.writeMessages()[0]
                      .get<apache::thrift::fast_thrift::rocket::server::
                               RocketResponseMessage>();
  const auto& errPayload = errResp.frame;
  EXPECT_EQ(
      errPayload.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      errPayload.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP));
}

// =============================================================================
// Non-SETUP First Frame Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, NonSetupFirstFrameReturnsInvalidSetup) {
  auto result = callOnRead(makeTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp = ctx_.writeMessages()[0]
                      .get<apache::thrift::fast_thrift::rocket::server::
                               RocketResponseMessage>();
  const auto& errPayload = errResp.frame;
  EXPECT_EQ(
      errPayload.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(errPayload.streamId, 0u);
  EXPECT_EQ(
      errPayload.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP));
}

// =============================================================================
// Duplicate SETUP Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, DuplicateSetupFrameReturnsInvalidSetup) {
  completeSetup();

  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Error);
  // Setup should still be complete from the first SETUP
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());

  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp = ctx_.writeMessages()[0]
                      .get<apache::thrift::fast_thrift::rocket::server::
                               RocketResponseMessage>();
  const auto& errPayload = errResp.frame;
  EXPECT_EQ(
      errPayload.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      errPayload.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP));
}

// =============================================================================
// Post-Setup Passthrough Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, PostSetupRequestFramePassesThrough) {
  completeSetup();

  auto result = callOnRead(makeTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);

  auto& request =
      ctx_.readMessages()[0].get<rocket::server::RocketRequestMessage>();
  EXPECT_EQ(
      request.frame.type(),
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);
}

// =============================================================================
// Outbound Passthrough Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, OnWritePassesThrough) {
  auto data = copyBuffer("test data");
  auto frame = apache::thrift::fast_thrift::frame::write::serialize(
      apache::thrift::fast_thrift::frame::write::PayloadHeader{
          .streamId = 1, .complete = true, .next = true},
      nullptr,
      std::move(data));
  auto parsed =
      apache::thrift::fast_thrift::frame::read::parseFrame(std::move(frame));

  auto result = callOnWrite(erase_and_box(std::move(parsed)));

  EXPECT_EQ(result, Result::Success);
  EXPECT_EQ(ctx_.writeMessages().size(), 1);
}

// =============================================================================
// Handler Lifecycle Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, HandlerRemovedResetsState) {
  completeSetup();
  EXPECT_TRUE(handler_.isSetupComplete());

  handler_.handlerRemoved(ctx_);

  EXPECT_FALSE(handler_.isSetupComplete());
  const auto& params = handler_.setupParameters();
  EXPECT_EQ(params.majorVersion, 0);
  EXPECT_EQ(params.minorVersion, 0);
  EXPECT_EQ(params.keepaliveTime, 0);
  EXPECT_EQ(params.maxLifetime, 0);
  EXPECT_FALSE(params.hasLease);
}

TEST_F(ServerSetupFrameHandlerTest, OnDisconnectIsNoOp) {
  handler_.onPipelineInactive(ctx_);
  EXPECT_FALSE(ctx_.disconnectCalled());
}

TEST_F(ServerSetupFrameHandlerTest, OnWriteReadyIsNoOp) {
  handler_.onWriteReady(ctx_);
  EXPECT_FALSE(ctx_.writeReadyCalled());
}

// =============================================================================
// Exception Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, OnExceptionPassesThrough) {
  auto ex = folly::make_exception_wrapper<std::runtime_error>("test error");
  handler_.onException(ctx_, ex);

  EXPECT_TRUE(ctx_.hasException());
}

// =============================================================================
// Backpressure / Rollback Tests
// =============================================================================

TEST_F(ServerSetupFrameHandlerTest, PostSetupBackpressurePropagated) {
  completeSetup();

  ctx_.setReadResult(Result::Backpressure);

  auto result = callOnRead(makeTestFrame(
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1));

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_TRUE(handler_.isSetupComplete());
}

// Backpressure on the SETUP itself means the layer above took it and only
// asked to slow down. Leaving the connection awaiting a setup would reject
// the next frame as "Expected SETUP frame" and tear down a live connection.
TEST_F(ServerSetupFrameHandlerTest, SetupBackpressureStillCompletesSetup) {
  ctx_.setReadResult(Result::Backpressure);

  auto result = callOnRead(makeSetupFrame());

  EXPECT_EQ(result, Result::Backpressure);
  EXPECT_TRUE(handler_.isSetupComplete());
  EXPECT_FALSE(ctx_.closeCalled());

  ctx_.reset();
  EXPECT_EQ(
      callOnRead(makeTestFrame(
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE, 1)),
      Result::Success);
  EXPECT_EQ(ctx_.readMessages().size(), 1);
}

// A refusal from the layer above ends the exchange. The connection is being
// torn down, so it must be left unset-up and never announced as established —
// subscribers to SetupComplete would otherwise see an established connection
// after they had already been told it closed.
TEST_F(ServerSetupFrameHandlerTest, UpstreamRefusalIsNotAnnouncedAsComplete) {
  ctx_.setReadResult(Result::Error);

  auto result = callOnRead(makeSetupFrame(1, 0, 30000, 60000));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(handler_.isSetupComplete());
  EXPECT_TRUE(ctx_.firedEvents().empty())
      << "a refused connection must not be announced as established";
}

TEST_F(ServerSetupFrameHandlerTest, InvalidSetupMetadataSendsErrorAndCloses) {
  RocketServerSetupFrameHandler wired;

  auto result = wired.onRead(
      ctx_,
      erase_and_box(
          rocket::server::RocketRequestMessage{
              .frame = makeSetupFrameWithTruncatedMetadata()}));

  EXPECT_EQ(result, Result::Error);
  EXPECT_FALSE(wired.isSetupComplete());
  EXPECT_TRUE(ctx_.closeCalled());
  ASSERT_EQ(ctx_.writeMessages().size(), 1);
  auto& errResp =
      ctx_.writeMessages()[0].get<rocket::server::RocketResponseMessage>();
  EXPECT_EQ(
      errResp.frame.frameType,
      apache::thrift::fast_thrift::frame::FrameType::ERROR);
  EXPECT_EQ(
      errResp.frame.errorCode,
      static_cast<uint32_t>(
          apache::thrift::fast_thrift::frame::ErrorCode::INVALID_SETUP));
}

} // namespace apache::thrift::fast_thrift::rocket::server::handler
