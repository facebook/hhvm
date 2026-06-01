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

#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>

#include <folly/io/IOBuf.h>

#include <gtest/gtest.h>

namespace apache::thrift::fast_thrift::frame::read {
namespace {

// Helper to create a frame buffer with test data
std::unique_ptr<folly::IOBuf> createFrameBuffer(
    uint32_t streamId,
    FrameType type,
    uint16_t flags,
    const std::vector<uint8_t>& extraHeader = {},
    const std::vector<uint8_t>& payload = {}) {
  size_t totalSize = 4 + 2 + extraHeader.size() + payload.size();
  auto buf = folly::IOBuf::create(totalSize);
  buf->append(totalSize);

  auto* data = buf->writableData();

  // Stream ID (big-endian)
  data[0] = (streamId >> 24) & 0xFF;
  data[1] = (streamId >> 16) & 0xFF;
  data[2] = (streamId >> 8) & 0xFF;
  data[3] = streamId & 0xFF;

  // Type (6 bits) + Flags (10 bits) = 2 bytes
  uint16_t typeAndFlags = (static_cast<uint16_t>(type) << 10) |
      (flags & ::apache::thrift::fast_thrift::frame::detail::kFlagsMask);
  data[4] = (typeAndFlags >> 8) & 0xFF;
  data[5] = typeAndFlags & 0xFF;

  // Extra header bytes
  for (size_t i = 0; i < extraHeader.size(); ++i) {
    data[6 + i] = extraHeader[i];
  }

  // Payload bytes
  for (size_t i = 0; i < payload.size(); ++i) {
    data[6 + extraHeader.size() + i] = payload[i];
  }

  return buf;
}

// Helper to create a ParsedFrame from test data
ParsedFrame createParsedFrame(
    uint32_t streamId,
    FrameType type,
    uint16_t flags,
    const std::vector<uint8_t>& extraHeader = {},
    const std::vector<uint8_t>& payload = {}) {
  auto buf = createFrameBuffer(streamId, type, flags, extraHeader, payload);
  const auto& desc = getDescriptor(type);

  ParsedFrame frame;
  frame.metadata.descriptor = &desc;
  frame.metadata.streamId = streamId;
  frame.metadata.flags_ = flags;
  frame.metadata.payloadOffset = desc.headerSize;
  frame.metadata.payloadSize = static_cast<uint32_t>(payload.size());
  frame.buffer = std::move(buf);

  return frame;
}

// === FrameView Base Tests ===

TEST(FrameViewTest, BasicAccessors) {
  auto frame = createParsedFrame(
      42, // streamId
      FrameType::PAYLOAD,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit |
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit,
      {}, // no extra header
      {0xDE, 0xAD, 0xBE, 0xEF} // payload
  );

  FrameView view(frame);

  EXPECT_EQ(view.streamId(), 42);
  EXPECT_EQ(view.type(), FrameType::PAYLOAD);
  EXPECT_STREQ(view.typeName(), "PAYLOAD");
  EXPECT_TRUE(view.hasMetadata());
  EXPECT_TRUE(view.isComplete());
  EXPECT_FALSE(view.hasFollows());
}

TEST(FrameViewTest, CursorAccess) {
  auto frame = createParsedFrame(
      1,
      FrameType::PAYLOAD,
      0,
      {}, // no extra header
      {0xCA, 0xFE, 0xBA, 0xBE} // payload
  );
  frame.metadata.metadataSize = 0;

  FrameView view(frame);

  auto cursor = view.payloadCursor();
  EXPECT_EQ(cursor.read<uint8_t>(), 0xCA);
  EXPECT_EQ(cursor.read<uint8_t>(), 0xFE);
}

// === RequestStreamView Tests ===

TEST(RequestStreamViewTest, InitialRequestN) {
  // REQUEST_STREAM has 4-byte initialRequestN after base header
  uint32_t initialN = 12345;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((initialN >> 24) & 0xFF),
      static_cast<uint8_t>((initialN >> 16) & 0xFF),
      static_cast<uint8_t>((initialN >> 8) & 0xFF),
      static_cast<uint8_t>(initialN & 0xFF),
  };

  auto frame = createParsedFrame(
      100, // streamId
      FrameType::REQUEST_STREAM,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit,
      extraHeader,
      {0x01, 0x02} // payload
  );

  RequestStreamView view(frame);

  EXPECT_EQ(view.streamId(), 100);
  EXPECT_EQ(view.type(), FrameType::REQUEST_STREAM);
  EXPECT_EQ(view.initialRequestN(), 12345);
  EXPECT_TRUE(view.hasMetadata());
}

TEST(RequestStreamViewTest, MaxInitialRequestN) {
  uint32_t maxN = 0x7FFFFFFF; // Max int32
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((maxN >> 24) & 0xFF),
      static_cast<uint8_t>((maxN >> 16) & 0xFF),
      static_cast<uint8_t>((maxN >> 8) & 0xFF),
      static_cast<uint8_t>(maxN & 0xFF),
  };

  auto frame = createParsedFrame(1, FrameType::REQUEST_STREAM, 0, extraHeader);

  RequestStreamView view(frame);
  EXPECT_EQ(view.initialRequestN(), 0x7FFFFFFF);
}

// === RequestChannelView Tests ===

TEST(RequestChannelViewTest, InitialRequestN) {
  uint32_t initialN = 999;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((initialN >> 24) & 0xFF),
      static_cast<uint8_t>((initialN >> 16) & 0xFF),
      static_cast<uint8_t>((initialN >> 8) & 0xFF),
      static_cast<uint8_t>(initialN & 0xFF),
  };

  auto frame = createParsedFrame(
      200, // streamId
      FrameType::REQUEST_CHANNEL,
      ::apache::thrift::fast_thrift::frame::detail::kCompleteBit,
      extraHeader);

  RequestChannelView view(frame);

  EXPECT_EQ(view.streamId(), 200);
  EXPECT_EQ(view.initialRequestN(), 999);
  EXPECT_TRUE(view.isComplete());
}

// === RequestNView Tests ===

TEST(RequestNViewTest, RequestN) {
  uint32_t requestN = 500;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((requestN >> 24) & 0xFF),
      static_cast<uint8_t>((requestN >> 16) & 0xFF),
      static_cast<uint8_t>((requestN >> 8) & 0xFF),
      static_cast<uint8_t>(requestN & 0xFF),
  };

  auto frame = createParsedFrame(300, FrameType::REQUEST_N, 0, extraHeader);

  RequestNView view(frame);

  EXPECT_EQ(view.streamId(), 300);
  EXPECT_EQ(view.requestN(), 500);
}

// === ErrorView Tests ===

TEST(ErrorViewTest, ErrorCode) {
  uint32_t errorCode = 0x00000201; // APPLICATION_ERROR
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((errorCode >> 24) & 0xFF),
      static_cast<uint8_t>((errorCode >> 16) & 0xFF),
      static_cast<uint8_t>((errorCode >> 8) & 0xFF),
      static_cast<uint8_t>(errorCode & 0xFF),
  };

  auto frame = createParsedFrame(
      400, // streamId
      FrameType::ERROR,
      0,
      extraHeader,
      {0x65, 0x72, 0x72, 0x6F, 0x72} // "error" payload
  );

  ErrorView view(frame);

  EXPECT_EQ(view.streamId(), 400);
  EXPECT_EQ(view.errorCode(), 0x00000201);
}

// === KeepAliveView Tests ===

TEST(KeepAliveViewTest, LastReceivedPositionAndRespond) {
  uint64_t lastPos = 0x123456789ABCDEF0;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((lastPos >> 56) & 0xFF),
      static_cast<uint8_t>((lastPos >> 48) & 0xFF),
      static_cast<uint8_t>((lastPos >> 40) & 0xFF),
      static_cast<uint8_t>((lastPos >> 32) & 0xFF),
      static_cast<uint8_t>((lastPos >> 24) & 0xFF),
      static_cast<uint8_t>((lastPos >> 16) & 0xFF),
      static_cast<uint8_t>((lastPos >> 8) & 0xFF),
      static_cast<uint8_t>(lastPos & 0xFF),
  };

  // KEEPALIVE with respond flag (bit 7, same as follows)
  auto frame = createParsedFrame(
      0, // streamId must be 0 for KEEPALIVE
      FrameType::KEEPALIVE,
      ::apache::thrift::fast_thrift::frame::detail::kRespondBit,
      extraHeader);

  KeepAliveView view(frame);

  EXPECT_EQ(view.streamId(), 0);
  EXPECT_TRUE(view.shouldRespond());
  EXPECT_EQ(view.lastReceivedPosition(), 0x123456789ABCDEF0);
}

TEST(KeepAliveViewTest, NoRespond) {
  std::vector<uint8_t> extraHeader(8, 0); // Zero position

  auto frame = createParsedFrame(0, FrameType::KEEPALIVE, 0, extraHeader);

  KeepAliveView view(frame);

  EXPECT_FALSE(view.shouldRespond());
  EXPECT_EQ(view.lastReceivedPosition(), 0);
}

// === ExtView Tests ===

TEST(ExtViewTest, ExtendedTypeAndIgnore) {
  uint32_t extType = 0xABCD1234;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((extType >> 24) & 0xFF),
      static_cast<uint8_t>((extType >> 16) & 0xFF),
      static_cast<uint8_t>((extType >> 8) & 0xFF),
      static_cast<uint8_t>(extType & 0xFF),
  };

  // EXT frame with ignore flag (bit 9)
  auto frame = createParsedFrame(
      500,
      FrameType::EXT,
      ::apache::thrift::fast_thrift::frame::detail::kIgnoreBit,
      extraHeader);

  ExtView view(frame);

  EXPECT_EQ(view.streamId(), 500);
  EXPECT_EQ(view.extendedType(), 0xABCD1234);
  EXPECT_TRUE(view.shouldIgnore());
}

TEST(ExtViewTest, NoIgnore) {
  std::vector<uint8_t> extraHeader = {0, 0, 0, 1};

  auto frame = createParsedFrame(501, FrameType::EXT, 0, extraHeader);

  ExtView view(frame);

  EXPECT_FALSE(view.shouldIgnore());
  EXPECT_EQ(view.extendedType(), 1);
}

// === asView Helper Tests ===

TEST(AsViewTest, TemplatedViewCreation) {
  std::vector<uint8_t> extraHeader = {0, 0, 0, 42};

  auto frame = createParsedFrame(123, FrameType::REQUEST_N, 0, extraHeader);

  auto view = asView<RequestNView>(frame);

  EXPECT_EQ(view.streamId(), 123);
  EXPECT_EQ(view.requestN(), 42);
}

// === Simple Frame View Alias Tests ===

TEST(SimpleFrameViewTest, RequestResponseView) {
  auto frame = createParsedFrame(
      1,
      FrameType::REQUEST_RESPONSE,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit,
      {},
      {0x01, 0x02});

  RequestResponseView view(frame);

  EXPECT_EQ(view.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(view.hasMetadata());
}

TEST(SimpleFrameViewTest, PayloadView) {
  auto frame = createParsedFrame(
      2,
      FrameType::PAYLOAD,
      ::apache::thrift::fast_thrift::frame::detail::kNextBit |
          ::apache::thrift::fast_thrift::frame::detail::kCompleteBit,
      {},
      {0x03, 0x04});

  PayloadView view(frame);

  EXPECT_EQ(view.type(), FrameType::PAYLOAD);
  EXPECT_TRUE(view.hasNext());
  EXPECT_TRUE(view.isComplete());
}

TEST(SimpleFrameViewTest, CancelView) {
  auto frame = createParsedFrame(3, FrameType::CANCEL, 0);

  CancelView view(frame);

  EXPECT_EQ(view.type(), FrameType::CANCEL);
  EXPECT_EQ(view.streamId(), 3);
}

TEST(SimpleFrameViewTest, RequestFnfView) {
  auto frame = createParsedFrame(
      4,
      FrameType::REQUEST_FNF,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit,
      {}, // no extra header (6-byte base)
      {0x01, 0x02, 0x03} // payload
  );

  RequestFnfView view(frame);

  EXPECT_EQ(view.type(), FrameType::REQUEST_FNF);
  EXPECT_EQ(view.streamId(), 4);
  EXPECT_TRUE(view.hasMetadata());
  EXPECT_FALSE(view.hasFollows());
}

TEST(SimpleFrameViewTest, RequestFnfViewWithFollows) {
  auto frame = createParsedFrame(
      5,
      FrameType::REQUEST_FNF,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit |
          ::apache::thrift::fast_thrift::frame::detail::kFollowsBit,
      {},
      {0x01, 0x02});

  RequestFnfView view(frame);

  EXPECT_EQ(view.type(), FrameType::REQUEST_FNF);
  EXPECT_TRUE(view.hasMetadata());
  EXPECT_TRUE(view.hasFollows());
}

TEST(SimpleFrameViewTest, MetadataPushView) {
  auto frame = createParsedFrame(
      0, // METADATA_PUSH must have streamId = 0
      FrameType::METADATA_PUSH,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit,
      {}, // no extra header
      {0xDE, 0xAD, 0xBE, 0xEF} // metadata payload
  );

  MetadataPushView view(frame);

  EXPECT_EQ(view.type(), FrameType::METADATA_PUSH);
  EXPECT_EQ(view.streamId(), 0);
  EXPECT_TRUE(view.hasMetadata());
}

// === Additional Flag Combination Tests ===

TEST(PayloadViewTest, AllFlagCombinations) {
  // PAYLOAD can have: M (metadata), F (follows), C (complete), N (next)

  // Just next
  {
    auto frame = createParsedFrame(
        10,
        FrameType::PAYLOAD,
        ::apache::thrift::fast_thrift::frame::detail::kNextBit,
        {},
        {0x01});
    PayloadView view(frame);
    EXPECT_TRUE(view.hasNext());
    EXPECT_FALSE(view.isComplete());
    EXPECT_FALSE(view.hasFollows());
    EXPECT_FALSE(view.hasMetadata());
  }

  // Next + Complete (common for final payload)
  {
    auto frame = createParsedFrame(
        11,
        FrameType::PAYLOAD,
        ::apache::thrift::fast_thrift::frame::detail::kNextBit |
            ::apache::thrift::fast_thrift::frame::detail::kCompleteBit,
        {},
        {0x01});
    PayloadView view(frame);
    EXPECT_TRUE(view.hasNext());
    EXPECT_TRUE(view.isComplete());
  }

  // Next + Follows (fragmented payload)
  {
    auto frame = createParsedFrame(
        12,
        FrameType::PAYLOAD,
        ::apache::thrift::fast_thrift::frame::detail::kNextBit |
            ::apache::thrift::fast_thrift::frame::detail::kFollowsBit,
        {},
        {0x01});
    PayloadView view(frame);
    EXPECT_TRUE(view.hasNext());
    EXPECT_TRUE(view.hasFollows());
    EXPECT_FALSE(view.isComplete());
  }

  // Metadata + Next + Complete
  {
    auto frame = createParsedFrame(
        13,
        FrameType::PAYLOAD,
        ::apache::thrift::fast_thrift::frame::detail::kMetadataBit |
            ::apache::thrift::fast_thrift::frame::detail::kNextBit |
            ::apache::thrift::fast_thrift::frame::detail::kCompleteBit,
        {},
        {0x01});
    PayloadView view(frame);
    EXPECT_TRUE(view.hasMetadata());
    EXPECT_TRUE(view.hasNext());
    EXPECT_TRUE(view.isComplete());
  }
}

TEST(RequestResponseViewTest, WithAllFlags) {
  // REQUEST_RESPONSE can have: M (metadata), F (follows)

  // No flags
  {
    auto frame =
        createParsedFrame(20, FrameType::REQUEST_RESPONSE, 0, {}, {0x01});
    RequestResponseView view(frame);
    EXPECT_FALSE(view.hasMetadata());
    EXPECT_FALSE(view.hasFollows());
  }

  // Metadata only
  {
    auto frame = createParsedFrame(
        21,
        FrameType::REQUEST_RESPONSE,
        ::apache::thrift::fast_thrift::frame::detail::kMetadataBit,
        {},
        {0x01});
    RequestResponseView view(frame);
    EXPECT_TRUE(view.hasMetadata());
    EXPECT_FALSE(view.hasFollows());
  }

  // Metadata + Follows (fragmented request)
  {
    auto frame = createParsedFrame(
        22,
        FrameType::REQUEST_RESPONSE,
        ::apache::thrift::fast_thrift::frame::detail::kMetadataBit |
            ::apache::thrift::fast_thrift::frame::detail::kFollowsBit,
        {},
        {0x01});
    RequestResponseView view(frame);
    EXPECT_TRUE(view.hasMetadata());
    EXPECT_TRUE(view.hasFollows());
  }
}

TEST(RequestStreamViewTest, WithFollows) {
  uint32_t initialN = 100;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((initialN >> 24) & 0xFF),
      static_cast<uint8_t>((initialN >> 16) & 0xFF),
      static_cast<uint8_t>((initialN >> 8) & 0xFF),
      static_cast<uint8_t>(initialN & 0xFF),
  };

  auto frame = createParsedFrame(
      30,
      FrameType::REQUEST_STREAM,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit |
          ::apache::thrift::fast_thrift::frame::detail::kFollowsBit,
      extraHeader,
      {0x01, 0x02});

  RequestStreamView view(frame);

  EXPECT_EQ(view.streamId(), 30);
  EXPECT_EQ(view.initialRequestN(), 100);
  EXPECT_TRUE(view.hasMetadata());
  EXPECT_TRUE(view.hasFollows());
}

TEST(RequestChannelViewTest, WithCompleteAndFollows) {
  // REQUEST_CHANNEL can have: M (metadata), F (follows), C (complete)
  uint32_t initialN = 50;
  std::vector<uint8_t> extraHeader = {
      static_cast<uint8_t>((initialN >> 24) & 0xFF),
      static_cast<uint8_t>((initialN >> 16) & 0xFF),
      static_cast<uint8_t>((initialN >> 8) & 0xFF),
      static_cast<uint8_t>(initialN & 0xFF),
  };

  // Complete only (single message channel)
  {
    auto frame = createParsedFrame(
        40,
        FrameType::REQUEST_CHANNEL,
        ::apache::thrift::fast_thrift::frame::detail::kCompleteBit,
        extraHeader,
        {0x01});
    RequestChannelView view(frame);
    EXPECT_TRUE(view.isComplete());
    EXPECT_FALSE(view.hasFollows());
  }

  // Follows only (more messages coming)
  {
    auto frame = createParsedFrame(
        41,
        FrameType::REQUEST_CHANNEL,
        ::apache::thrift::fast_thrift::frame::detail::kFollowsBit,
        extraHeader,
        {0x01});
    RequestChannelView view(frame);
    EXPECT_FALSE(view.isComplete());
    EXPECT_TRUE(view.hasFollows());
  }
}

TEST(ErrorViewTest, DifferentErrorCodes) {
  // Test common error codes used by Rocket

  // INVALID_SETUP (0x0001)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0, 1};
    auto frame = createParsedFrame(0, FrameType::ERROR, 0, extraHeader, {});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000001);
  }

  // UNSUPPORTED_SETUP (0x0002)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0, 2};
    auto frame = createParsedFrame(0, FrameType::ERROR, 0, extraHeader, {});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000002);
  }

  // REJECTED_SETUP (0x0003)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0, 3};
    auto frame = createParsedFrame(0, FrameType::ERROR, 0, extraHeader, {});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000003);
  }

  // APPLICATION_ERROR (0x0201)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0x02, 0x01};
    auto frame = createParsedFrame(
        100, FrameType::ERROR, 0, extraHeader, {'e', 'r', 'r'});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000201);
    EXPECT_EQ(view.streamId(), 100);
  }

  // REJECTED (0x0202)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0x02, 0x02};
    auto frame = createParsedFrame(101, FrameType::ERROR, 0, extraHeader, {});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000202);
  }

  // CANCELED (0x0203)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0x02, 0x03};
    auto frame = createParsedFrame(102, FrameType::ERROR, 0, extraHeader, {});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000203);
  }

  // INVALID (0x0204)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0x02, 0x04};
    auto frame = createParsedFrame(103, FrameType::ERROR, 0, extraHeader, {});
    ErrorView view(frame);
    EXPECT_EQ(view.errorCode(), 0x00000204);
  }
}

TEST(RequestNViewTest, VariousRequestNValues) {
  // Test boundary values for REQUEST_N

  // Minimum valid (1)
  {
    std::vector<uint8_t> extraHeader = {0, 0, 0, 1};
    auto frame = createParsedFrame(50, FrameType::REQUEST_N, 0, extraHeader);
    RequestNView view(frame);
    EXPECT_EQ(view.requestN(), 1);
  }

  // Typical value
  {
    uint32_t n = 1000;
    std::vector<uint8_t> extraHeader = {
        static_cast<uint8_t>((n >> 24) & 0xFF),
        static_cast<uint8_t>((n >> 16) & 0xFF),
        static_cast<uint8_t>((n >> 8) & 0xFF),
        static_cast<uint8_t>(n & 0xFF),
    };
    auto frame = createParsedFrame(51, FrameType::REQUEST_N, 0, extraHeader);
    RequestNView view(frame);
    EXPECT_EQ(view.requestN(), 1000);
  }

  // Max int32 (unlimited)
  {
    uint32_t n = 0x7FFFFFFF;
    std::vector<uint8_t> extraHeader = {
        static_cast<uint8_t>((n >> 24) & 0xFF),
        static_cast<uint8_t>((n >> 16) & 0xFF),
        static_cast<uint8_t>((n >> 8) & 0xFF),
        static_cast<uint8_t>(n & 0xFF),
    };
    auto frame = createParsedFrame(52, FrameType::REQUEST_N, 0, extraHeader);
    RequestNView view(frame);
    EXPECT_EQ(view.requestN(), 0x7FFFFFFF);
  }
}

// === SetupView Tests ===

TEST(SetupViewTest, BasicSetupFrame) {
  // SETUP frame layout:
  // [6-7]   Major Version
  // [8-9]   Minor Version
  // [10-13] Keepalive Time
  // [14-17] Max Lifetime
  // [18+]   MIME types and payload

  uint16_t majorVersion = 1;
  uint16_t minorVersion = 0;
  uint32_t keepaliveTime = 30000; // 30 seconds
  uint32_t maxLifetime = 60000; // 60 seconds

  std::vector<uint8_t> extraHeader = {
      // Major version (BE)
      static_cast<uint8_t>((majorVersion >> 8) & 0xFF),
      static_cast<uint8_t>(majorVersion & 0xFF),
      // Minor version (BE)
      static_cast<uint8_t>((minorVersion >> 8) & 0xFF),
      static_cast<uint8_t>(minorVersion & 0xFF),
      // Keepalive time (BE)
      static_cast<uint8_t>((keepaliveTime >> 24) & 0xFF),
      static_cast<uint8_t>((keepaliveTime >> 16) & 0xFF),
      static_cast<uint8_t>((keepaliveTime >> 8) & 0xFF),
      static_cast<uint8_t>(keepaliveTime & 0xFF),
      // Max lifetime (BE)
      static_cast<uint8_t>((maxLifetime >> 24) & 0xFF),
      static_cast<uint8_t>((maxLifetime >> 16) & 0xFF),
      static_cast<uint8_t>((maxLifetime >> 8) & 0xFF),
      static_cast<uint8_t>(maxLifetime & 0xFF),
  };

  auto frame = createParsedFrame(
      0, // SETUP must have streamId = 0
      FrameType::SETUP,
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit,
      extraHeader);

  SetupView view(frame);

  EXPECT_EQ(view.streamId(), 0);
  EXPECT_EQ(view.type(), FrameType::SETUP);
  EXPECT_EQ(view.majorVersion(), 1);
  EXPECT_EQ(view.minorVersion(), 0);
  EXPECT_EQ(view.keepaliveTime(), 30000);
  EXPECT_EQ(view.maxLifetime(), 60000);
  EXPECT_TRUE(view.hasMetadata());
  EXPECT_FALSE(view.hasLease());
  EXPECT_FALSE(view.hasResumeToken());
}

TEST(SetupViewTest, WithLeaseFlag) {
  std::vector<uint8_t> extraHeader = {
      0,
      1, // Major version = 1
      0,
      0, // Minor version = 0
      0,
      0,
      0x75,
      0x30, // Keepalive time = 30000
      0,
      0,
      0xEA,
      0x60, // Max lifetime = 60000
  };

  // Lease flag is bit 6 (same position as complete)
  auto frame = createParsedFrame(
      0,
      FrameType::SETUP,
      ::apache::thrift::fast_thrift::frame::detail::kLeaseBit,
      extraHeader);

  SetupView view(frame);

  EXPECT_TRUE(view.hasLease());
}

TEST(SetupViewTest, WithResumeTokenFlag) {
  std::vector<uint8_t> extraHeader = {
      0,
      1, // Major version = 1
      0,
      0, // Minor version = 0
      0,
      0,
      0x75,
      0x30, // Keepalive time = 30000
      0,
      0,
      0xEA,
      0x60, // Max lifetime = 60000
  };

  // Resume token flag is bit 7 (same position as follows)
  auto frame = createParsedFrame(
      0,
      FrameType::SETUP,
      ::apache::thrift::fast_thrift::frame::detail::kResumeTokenBit,
      extraHeader);

  SetupView view(frame);

  EXPECT_TRUE(view.hasResumeToken());
}

TEST(SetupViewTest, RocketTypicalValues) {
  // Test typical values used by Rocket/Thrift
  // Max keepalive and lifetime are (2^31 - 1) milliseconds

  uint32_t maxKeepalive = 0x7FFFFFFF;
  uint32_t maxLifetime = 0x7FFFFFFF;

  std::vector<uint8_t> extraHeader = {
      0,
      1, // Major version = 1
      0,
      0, // Minor version = 0
      // Max keepalive
      static_cast<uint8_t>((maxKeepalive >> 24) & 0xFF),
      static_cast<uint8_t>((maxKeepalive >> 16) & 0xFF),
      static_cast<uint8_t>((maxKeepalive >> 8) & 0xFF),
      static_cast<uint8_t>(maxKeepalive & 0xFF),
      // Max lifetime
      static_cast<uint8_t>((maxLifetime >> 24) & 0xFF),
      static_cast<uint8_t>((maxLifetime >> 16) & 0xFF),
      static_cast<uint8_t>((maxLifetime >> 8) & 0xFF),
      static_cast<uint8_t>(maxLifetime & 0xFF),
  };

  auto frame = createParsedFrame(0, FrameType::SETUP, 0, extraHeader);

  SetupView view(frame);

  EXPECT_EQ(view.majorVersion(), 1);
  EXPECT_EQ(view.minorVersion(), 0);
  EXPECT_EQ(view.keepaliveTime(), 0x7FFFFFFF);
  EXPECT_EQ(view.maxLifetime(), 0x7FFFFFFF);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read
