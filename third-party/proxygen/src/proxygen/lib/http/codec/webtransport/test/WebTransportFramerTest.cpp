/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>
#include <quic/folly_utils/Utils.h>

using namespace proxygen;
using namespace testing;

class WebTransportFramerTest : public Test {};

uint64_t parseCapsuleHeader(folly::io::Cursor& cursor, uint64_t type) {
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor);
  EXPECT_EQ(typeOpt->first, type);
  auto length = quic::follyutils::decodeQuicInteger(cursor);
  return length->first;
}

uint64_t parseQmuxFrameHeader(folly::io::Cursor& cursor,
                              size_t totalLen,
                              uint64_t expectedType) {
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, totalLen);
  EXPECT_TRUE(typeOpt.has_value());
  EXPECT_EQ(typeOpt->first, expectedType);
  return totalLen - typeOpt->second;
}

//////// Parameterized tests for shared frame types ////////

class WebTransportFramerProtocolTest : public TestWithParam<FrameProtocol> {
 protected:
  // Parse the frame header and return the payload length.
  // For WT: parses type + length from capsule TLV.
  // For QMUX: parses just the type varint.
  uint64_t parseHeader(folly::io::Cursor& cursor,
                       size_t totalLen,
                       uint64_t wtType,
                       uint64_t qmuxType) {
    if (GetParam() == FrameProtocol::QMUX) {
      return parseQmuxFrameHeader(cursor, totalLen, qmuxType);
    }
    return parseCapsuleHeader(cursor, wtType);
  }
};

INSTANTIATE_TEST_SUITE_P(Protocols,
                         WebTransportFramerProtocolTest,
                         Values(FrameProtocol::WT_CAPSULE,
                                FrameProtocol::QMUX));

TEST_P(WebTransportFramerProtocolTest, ResetStream) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = 0x1,
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D39, 0x04);

  auto parsed = parseWTResetStream(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->streamId, capsule.streamId);
  EXPECT_EQ(parsed->appProtocolErrorCode, capsule.appProtocolErrorCode);
  EXPECT_EQ(parsed->reliableSize, capsule.reliableSize);
}

TEST_P(WebTransportFramerProtocolTest, StopSending) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStopSendingCapsule capsule{.streamId = 0x01,
                               .appProtocolErrorCode = 0x12345678};
  auto writeRes = writeWTStopSending(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D3A, 0x05);

  auto parsed = parseWTStopSending(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->streamId, capsule.streamId);
  EXPECT_EQ(parsed->appProtocolErrorCode, capsule.appProtocolErrorCode);
}

TEST_P(WebTransportFramerProtocolTest, Stream) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x01,
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = true};
  auto writeRes = writeWTStream(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());

  if (protocol == FrameProtocol::QMUX) {
    // QMUX: type(0x0b) + streamId + dataLen(varint) + data
    auto payloadLen = parseQmuxFrameHeader(cursor, *writeRes, 0x0b);
    auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(streamIdOpt.has_value());
    payloadLen -= streamIdOpt->second;
    EXPECT_EQ(streamIdOpt->first, capsule.streamId);

    auto dataLenOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(dataLenOpt.has_value());

    std::unique_ptr<folly::IOBuf> data;
    cursor.cloneAtMost(data, dataLenOpt->first);
    EXPECT_TRUE(folly::IOBufEqualTo()(data, capsule.streamData));
  } else {
    // WT: capsule header + streamId + data
    auto payloadLen = parseCapsuleHeader(cursor, 0x190B4D3C);
    auto parsed = parseWTStream(cursor, payloadLen, capsule.fin);
    ASSERT_TRUE(parsed.hasValue());
    EXPECT_EQ(parsed->streamId, capsule.streamId);
    EXPECT_TRUE(folly::IOBufEqualTo()(parsed->streamData, capsule.streamData));
    EXPECT_EQ(parsed->fin, capsule.fin);
  }
}

TEST_P(WebTransportFramerProtocolTest, StreamNoFin) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x01,
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = false};
  auto writeRes = writeWTStream(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());

  if (protocol == FrameProtocol::QMUX) {
    auto payloadLen = parseQmuxFrameHeader(cursor, *writeRes, 0x0a);
    auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(streamIdOpt.has_value());
    payloadLen -= streamIdOpt->second;

    auto dataLenOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(dataLenOpt.has_value());

    std::unique_ptr<folly::IOBuf> data;
    cursor.cloneAtMost(data, dataLenOpt->first);
    EXPECT_TRUE(folly::IOBufEqualTo()(data, capsule.streamData));
  } else {
    auto payloadLen = parseCapsuleHeader(cursor, 0x190B4D3B);
    auto parsed = parseWTStream(cursor, payloadLen, capsule.fin);
    ASSERT_TRUE(parsed.hasValue());
    EXPECT_EQ(parsed->streamId, capsule.streamId);
    EXPECT_TRUE(folly::IOBufEqualTo()(parsed->streamData, capsule.streamData));
    EXPECT_FALSE(parsed->fin);
  }
}

TEST_P(WebTransportFramerProtocolTest, StreamNullData) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x01, .streamData = nullptr, .fin = true};
  auto writeRes = writeWTStream(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());

  if (protocol == FrameProtocol::QMUX) {
    auto payloadLen = parseQmuxFrameHeader(cursor, *writeRes, 0x0b);
    auto streamIdOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(streamIdOpt.has_value());
    payloadLen -= streamIdOpt->second;

    auto dataLenOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(dataLenOpt.has_value());
    EXPECT_EQ(dataLenOpt->first, 0);
  } else {
    auto payloadLen = parseCapsuleHeader(cursor, 0x190B4D3C);
    auto parsed = parseWTStream(cursor, payloadLen, capsule.fin);
    ASSERT_TRUE(parsed.hasValue());
    EXPECT_EQ(parsed->streamId, capsule.streamId);
    EXPECT_EQ(parsed->streamData->computeChainDataLength(), 0);
    EXPECT_EQ(parsed->fin, capsule.fin);
  }
}

TEST_P(WebTransportFramerProtocolTest, MaxData) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxDataCapsule capsule{100};
  auto writeRes = writeWTMaxData(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D3D, 0x10);

  auto parsed = parseWTMaxData(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->maximumData, capsule.maximumData);
}

TEST_P(WebTransportFramerProtocolTest, MaxStreamData) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamDataCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTMaxStreamData(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D3E, 0x11);

  auto parsed = parseWTMaxStreamData(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->streamId, capsule.streamId);
  EXPECT_EQ(parsed->maximumStreamData, capsule.maximumStreamData);
}

TEST_P(WebTransportFramerProtocolTest, MaxStreamsBidi) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{100};
  auto writeRes = writeWTMaxStreams(queue, capsule, true, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D3F, 0x12);

  auto parsed = parseWTMaxStreams(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->maximumStreams, capsule.maximumStreams);
}

TEST_P(WebTransportFramerProtocolTest, MaxStreamsUni) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{100};
  auto writeRes = writeWTMaxStreams(queue, capsule, false, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D40, 0x13);

  auto parsed = parseWTMaxStreams(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->maximumStreams, capsule.maximumStreams);
}

TEST_P(WebTransportFramerProtocolTest, DataBlocked) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTDataBlockedCapsule capsule{100};
  auto writeRes = writeWTDataBlocked(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D41, 0x14);

  auto parsed = parseWTDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->maximumData, capsule.maximumData);
}

TEST_P(WebTransportFramerProtocolTest, StreamDataBlocked) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamDataBlockedCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTStreamDataBlocked(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D42, 0x15);

  auto parsed = parseWTStreamDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->streamId, capsule.streamId);
  EXPECT_EQ(parsed->maximumStreamData, capsule.maximumStreamData);
}

TEST_P(WebTransportFramerProtocolTest, StreamsBlockedBidi) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{100};
  auto writeRes = writeWTStreamsBlocked(queue, capsule, true, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D43, 0x16);

  auto parsed = parseWTStreamsBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->maximumStreams, capsule.maximumStreams);
}

TEST_P(WebTransportFramerProtocolTest, StreamsBlockedUni) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{100};
  auto writeRes = writeWTStreamsBlocked(queue, capsule, false, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseHeader(cursor, *writeRes, 0x190B4D44, 0x17);

  auto parsed = parseWTStreamsBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_EQ(parsed->maximumStreams, capsule.maximumStreams);
}

TEST_P(WebTransportFramerProtocolTest, Datagram) {
  auto protocol = GetParam();
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  DatagramCapsule capsule{folly::IOBuf::copyBuffer("Hello, World!")};
  auto writeRes = writeDatagram(queue, capsule, protocol);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());

  uint64_t dataLen = 0;
  if (protocol == FrameProtocol::QMUX) {
    // QMUX: type(0x31) + varint(dataLen) + data
    auto payloadLen = parseQmuxFrameHeader(cursor, *writeRes, 0x31);
    auto dataLenOpt = quic::follyutils::decodeQuicInteger(cursor, payloadLen);
    ASSERT_TRUE(dataLenOpt.has_value());
    dataLen = dataLenOpt->first;
  } else {
    // WT: capsule header + data
    dataLen = parseCapsuleHeader(cursor, 0x00);
  }

  auto parsed = parseDatagram(cursor, dataLen);
  ASSERT_TRUE(parsed.hasValue());
  EXPECT_TRUE(folly::IOBufEqualTo()(parsed->httpDatagramPayload,
                                    capsule.httpDatagramPayload));
}

//////// WT-only capsule types ////////

TEST(WebTransportFramerTest, Padding) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  PaddingCapsule capsule{10};
  auto writeRes = writePadding(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen =
      parseCapsuleHeader(cursor, static_cast<uint64_t>(CapsuleType::PADDING));
  size_t cursorPositionBefore = cursor.getCurrentPosition();

  auto parsedCapsule = parsePadding(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->paddingLength, capsule.paddingLength);

  size_t cursorPositionAfter = cursor.getCurrentPosition();
  EXPECT_EQ(cursorPositionAfter - cursorPositionBefore, capsule.paddingLength);
}

TEST(WebTransportFramerTest, CloseWebTransportSession) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 0x12345678,
                                          .applicationErrorMessage = "BAD!"};
  auto writeRes = writeCloseWebTransportSession(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::CLOSE_WEBTRANSPORT_SESSION));

  auto parsedCapsule = parseCloseWebTransportSession(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->applicationErrorCode, capsule.applicationErrorCode);
  ASSERT_EQ(parsedCapsule->applicationErrorMessage,
            capsule.applicationErrorMessage);
}

TEST(WebTransportFramerTest, DrainWebTransportSession) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  auto writeRes = writeDrainWebTransportSession(queue);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::DRAIN_WEBTRANSPORT_SESSION));

  auto parsedCapsule = parseDrainWebTransportSession(payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
}

//////// WT error / truncation tests ////////

TEST(WebTransportFramerTest, WTResetStreamStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = 0x1,
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(6);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_RESET_STREAM));

  auto parsedCapsule = parseWTResetStream(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTResetStreamAppProtocolErrorCodeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = 0x1,
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(5);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_RESET_STREAM));

  auto parsedCapsule = parseWTResetStream(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTResetStreamReliableSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = 0x1,
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(1);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_RESET_STREAM));

  auto parsedCapsule = parseWTResetStream(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTResetStreamCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = std::numeric_limits<uint64_t>::max(),
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTStopSendingStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStopSendingCapsule capsule{.streamId = 0x01,
                               .appProtocolErrorCode = 0x12345678};
  auto writeRes = writeWTStopSending(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(5);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STOP_SENDING));

  auto parsedCapsule = parseWTStopSending(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStopSendingAppProtocolErrorCodeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStopSendingCapsule capsule{.streamId = 0x01,
                               .appProtocolErrorCode = 0x12345678};
  auto writeRes = writeWTStopSending(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(4);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STOP_SENDING));

  auto parsedCapsule = parseWTStopSending(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStopSendingCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStopSendingCapsule capsule{.streamId = std::numeric_limits<uint64_t>::max(),
                               .appProtocolErrorCode = 0x12345678};
  auto writeRes = writeWTStopSending(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTStreamStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x1,
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = true};
  auto writeRes = writeWTStream(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  queue.trimEnd(14);
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAM_WITH_FIN));

  auto parsedCapsule = parseWTStream(cursor, payloadLen, capsule.fin);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamDataError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x1,
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = true};
  auto writeRes = writeWTStream(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  queue.trimEnd(13);
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAM_WITH_FIN));

  auto parsedCapsule = parseWTStream(cursor, payloadLen, capsule.fin);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = std::numeric_limits<uint64_t>::max(),
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = true};
  auto writeRes = writeWTStream(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTMaxDataError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxDataCapsule capsule{100};
  auto writeRes = writeWTMaxData(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_DATA));

  auto parsedCapsule = parseWTMaxData(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTMaxDataCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxDataCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTMaxData(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTMaxStreamDataStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamDataCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTMaxStreamData(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(3);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAM_DATA));

  auto parsedCapsule = parseWTMaxStreamData(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTMaxStreamDataError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamDataCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTMaxStreamData(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAM_DATA));

  auto parsedCapsule = parseWTMaxStreamData(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTMaxStreamDataCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamDataCapsule capsule{.streamId =
                                     std::numeric_limits<uint64_t>::max(),
                                 .maximumStreamData = 100};
  auto writeRes = writeWTMaxStreamData(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTMaxStreamsError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{100};
  auto writeRes = writeWTMaxStreams(queue, capsule, true);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAMS_BIDI));

  auto parsedCapsule = parseWTMaxStreams(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTMaxStreamsCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTMaxStreams(queue, capsule, true);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTMaxStreamsUniError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{100};
  auto writeRes = writeWTMaxStreams(queue, capsule, false);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAMS_UNI));

  auto parsedCapsule = parseWTMaxStreams(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTMaxStreamsUniCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTMaxStreams(queue, capsule, false);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTDataBlockedError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTDataBlockedCapsule capsule{100};
  auto writeRes = writeWTDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_DATA_BLOCKED));

  auto parsedCapsule = parseWTDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTDataBlockedCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTDataBlockedCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTDataBlocked(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTStreamDataBlockedStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamDataBlockedCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTStreamDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(3);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAM_DATA_BLOCKED));

  auto parsedCapsule = parseWTStreamDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamDataBlockedMaxStreamDataError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamDataBlockedCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTStreamDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAM_DATA_BLOCKED));

  auto parsedCapsule = parseWTStreamDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamDataBlockedCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamDataBlockedCapsule capsule{.streamId =
                                         std::numeric_limits<uint64_t>::max(),
                                     .maximumStreamData = 100};
  auto writeRes = writeWTStreamDataBlocked(queue, capsule);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTStreamsBlockedError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{100};
  auto writeRes = writeWTStreamsBlocked(queue, capsule, true);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAMS_BLOCKED_BIDI));

  auto parsedCapsule = parseWTStreamsBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamsBlockedCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTStreamsBlocked(queue, capsule, true);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, WTStreamsBlockedUniError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{100};
  auto writeRes = writeWTStreamsBlocked(queue, capsule, false);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAMS_BLOCKED_UNI));

  auto parsedCapsule = parseWTStreamsBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamsBlockedUniCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTStreamsBlocked(queue, capsule, false);
  ASSERT_FALSE(writeRes.has_value());
}

TEST(WebTransportFramerTest, DatagramError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  DatagramCapsule capsule{folly::IOBuf::copyBuffer("Hello, World!")};
  auto writeRes = writeDatagram(queue, capsule);
  queue.trimEnd(13);
  ASSERT_TRUE(writeRes.has_value());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen =
      parseCapsuleHeader(cursor, static_cast<uint64_t>(CapsuleType::DATAGRAM));

  auto parsedCapsule = parseDatagram(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, CloseWebTransportSessionErrorCodeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 0x12345678,
                                          .applicationErrorMessage = "BAD!"};
  auto writeRes = writeCloseWebTransportSession(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  queue.trimEnd(8);
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::CLOSE_WEBTRANSPORT_SESSION));

  auto parsedCapsule = parseCloseWebTransportSession(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, CloseWebTransportSessionErrorMsgError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 0x12345678,
                                          .applicationErrorMessage = "BAD!"};
  auto writeRes = writeCloseWebTransportSession(queue, capsule);
  ASSERT_TRUE(writeRes.has_value());
  queue.trimEnd(4);
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::CLOSE_WEBTRANSPORT_SESSION));

  auto parsedCapsule = parseCloseWebTransportSession(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}
