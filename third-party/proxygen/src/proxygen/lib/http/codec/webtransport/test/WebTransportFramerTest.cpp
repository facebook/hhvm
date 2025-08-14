/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "gtest/gtest.h"
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/CapsuleCodec.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>

using namespace proxygen;
using namespace testing;

class WebTransportFramerTest : public Test {};

uint64_t parseCapsuleHeader(folly::io::Cursor& cursor, uint64_t type) {
  // parse the type
  auto typeOpt = quic::decodeQuicInteger(cursor);
  EXPECT_EQ(typeOpt->first, type);

  // parse + return the length
  auto length = quic::decodeQuicInteger(cursor);
  return length->first;
}

TEST(WebTransportFramerTest, Padding) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  PaddingCapsule capsule{10};
  auto writeRes = writePadding(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen =
      parseCapsuleHeader(cursor, static_cast<uint64_t>(CapsuleType::PADDING));
  size_t cursorPositionBefore = cursor.getCurrentPosition();

  // parse the capsule payload
  auto parsedCapsule = parsePadding(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->paddingLength, capsule.paddingLength);

  // check that we skipped over the padding
  size_t cursorPositionAfter = cursor.getCurrentPosition();
  EXPECT_EQ(cursorPositionAfter - cursorPositionBefore, capsule.paddingLength);
}

TEST(WebTransportFramerTest, WTResetStream) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = 0x1,
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_RESET_STREAM));

  auto parsedCapsule = parseWTResetStream(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->streamId, capsule.streamId);
  ASSERT_EQ(parsedCapsule->appProtocolErrorCode, capsule.appProtocolErrorCode);
  ASSERT_EQ(parsedCapsule->reliableSize, capsule.reliableSize);
}

TEST(WebTransportFramerTest, WTResetStreamStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTResetStreamCapsule capsule{.streamId = 0x1,
                               .appProtocolErrorCode = 0x12345678,
                               .reliableSize = 0x10};
  auto writeRes = writeWTResetStream(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that streamId (1 byte), appProtocolErrorCode (4 bytes), and
  // reliableSize (1 byte) are empty
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
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that appProtocolErrorCode (4 bytes) and reliableSize (1 byte)
  // are empty
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
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that reliableSize (1 byte) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTStopSending) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStopSendingCapsule capsule{.streamId = 0x01,
                               .appProtocolErrorCode = 0x12345678};
  auto writeRes = writeWTStopSending(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STOP_SENDING));

  auto parsedCapsule = parseWTStopSending(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->streamId, capsule.streamId);
  ASSERT_EQ(parsedCapsule->appProtocolErrorCode, capsule.appProtocolErrorCode);
}

TEST(WebTransportFramerTest, WTStopSendingStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStopSendingCapsule capsule{.streamId = 0x01,
                               .appProtocolErrorCode = 0x12345678};
  auto writeRes = writeWTStopSending(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that streamId (1 byte) and appProtocolErrorCode (4 bytes) are
  // empty
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
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that appProtocolErrorCode (4 bytes) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTStream) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x01,
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = true};
  auto writeRes = writeWTStream(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAM_WITH_FIN));

  auto parsedCapsule = parseWTStream(cursor, payloadLen, capsule.fin);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->streamId, capsule.streamId);
  ASSERT_TRUE(
      folly::IOBufEqualTo()(parsedCapsule->streamData, capsule.streamData));
  ASSERT_EQ(parsedCapsule->fin, capsule.fin);
}

TEST(WebTransportFramerTest, WTStreamStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamCapsule capsule{.streamId = 0x1,
                          .streamData =
                              folly::IOBuf::copyBuffer("Hello, World!"),
                          .fin = true};
  auto writeRes = writeWTStream(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  // hack buf so that streamId (1 byte) and streamData (13 bytes) are empty
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
  ASSERT_TRUE(writeRes.hasValue());
  // hack buf so that streamData (13 bytes) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTMaxData) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxDataCapsule capsule{100};
  auto writeRes = writeWTMaxData(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_DATA));

  auto parsedCapsule = parseWTMaxData(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->maximumData, capsule.maximumData);
}

TEST(WebTransportFramerTest, WTMaxDataError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxDataCapsule capsule{100};
  auto writeRes = writeWTMaxData(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that maximumData (2 bytes) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTMaxStreamData) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamDataCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTMaxStreamData(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAM_DATA));

  auto parsedCapsule = parseWTMaxStreamData(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->streamId, capsule.streamId);
  ASSERT_EQ(parsedCapsule->maximumStreamData, capsule.maximumStreamData);
}

TEST(WebTransportFramerTest, WTMaxStreamDataStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamDataCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTMaxStreamData(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that streamId (1 byte) and maximumStreamData (2 bytes) are
  // empty
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
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that maximumStreamData (2 bytes) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTMaxStreams) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{100};
  auto writeRes = writeWTMaxStreams(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAMS));

  auto parsedCapsule = parseWTMaxStreams(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->maximumStreams, capsule.maximumStreams);
}

TEST(WebTransportFramerTest, WTMaxStreamsError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{100};
  auto writeRes = writeWTMaxStreams(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that maximumStreams (2 bytes) is empty
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_MAX_STREAMS));

  auto parsedCapsule = parseWTMaxStreams(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTMaxStreamsCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTMaxStreamsCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTMaxStreams(queue, capsule);
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTDataBlocked) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTDataBlockedCapsule capsule{100};
  auto writeRes = writeWTDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_DATA_BLOCKED));

  auto parsedCapsule = parseWTDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->maximumData, capsule.maximumData);
}

TEST(WebTransportFramerTest, WTDataBlockedError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTDataBlockedCapsule capsule{100};
  auto writeRes = writeWTDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that maximumData (2 bytes) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTStreamDataBlocked) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamDataBlockedCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTStreamDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAM_DATA_BLOCKED));

  auto parsedCapsule = parseWTStreamDataBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->streamId, capsule.streamId);
  ASSERT_EQ(parsedCapsule->maximumStreamData, capsule.maximumStreamData);
}

TEST(WebTransportFramerTest, WTStreamDataBlockedStreamIdError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamDataBlockedCapsule capsule{.streamId = 0x1, .maximumStreamData = 100};
  auto writeRes = writeWTStreamDataBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that streamId (1 byte) and maximumStreamData (2 bytes) are
  // empty
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
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that maximumStreamData (2 bytes) is empty
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
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, WTStreamsBlocked) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{100};
  auto writeRes = writeWTStreamsBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAMS_BLOCKED));

  auto parsedCapsule = parseWTStreamsBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_EQ(parsedCapsule->maximumStreams, capsule.maximumStreams);
}

TEST(WebTransportFramerTest, WTStreamsBlockedError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{100};
  auto writeRes = writeWTStreamsBlocked(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  // hack buf so that maximumStreams (2 bytes) is empty
  buf->trimEnd(2);
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::WT_STREAMS_BLOCKED));

  auto parsedCapsule = parseWTStreamsBlocked(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, WTStreamsBlockedCapsuleSizeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  WTStreamsBlockedCapsule capsule{std::numeric_limits<uint64_t>::max()};
  auto writeRes = writeWTStreamsBlocked(queue, capsule);
  ASSERT_FALSE(writeRes.hasValue());
  EXPECT_EQ(writeRes.error(), quic::TransportErrorCode::INTERNAL_ERROR);
}

TEST(WebTransportFramerTest, Datagram) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  DatagramCapsule capsule{folly::IOBuf::copyBuffer("Hello, World!")};
  auto writeRes = writeDatagram(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen =
      parseCapsuleHeader(cursor, static_cast<uint64_t>(CapsuleType::DATAGRAM));

  auto parsedCapsule = parseDatagram(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
  ASSERT_TRUE(folly::IOBufEqualTo()(parsedCapsule->httpDatagramPayload,
                                    capsule.httpDatagramPayload));
}

TEST(WebTransportFramerTest, DatagramError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  DatagramCapsule capsule{folly::IOBuf::copyBuffer("Hello, World!")};
  auto writeRes = writeDatagram(queue, capsule);
  // hack buf so that httpDatagramPayload (13 bytes) is empty
  queue.trimEnd(13);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen =
      parseCapsuleHeader(cursor, static_cast<uint64_t>(CapsuleType::DATAGRAM));

  auto parsedCapsule = parseDatagram(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, CloseWebTransportSession) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 0x12345678,
                                          .applicationErrorMessage = "BAD!"};
  auto writeRes = writeCloseWebTransportSession(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
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

TEST(WebTransportFramerTest, CloseWebTransportSessionErrorCodeError) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  CloseWebTransportSessionCapsule capsule{.applicationErrorCode = 0x12345678,
                                          .applicationErrorMessage = "BAD!"};
  auto writeRes = writeCloseWebTransportSession(queue, capsule);
  ASSERT_TRUE(writeRes.hasValue());
  // hack buf so that applicationErrorCode (4 bytes) and applicationErrorMessage
  // (4 bytes) are empty
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
  ASSERT_TRUE(writeRes.hasValue());
  // hack buf so that applicationErrorMessage (4 bytes) is empty
  queue.trimEnd(4);
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::CLOSE_WEBTRANSPORT_SESSION));

  auto parsedCapsule = parseCloseWebTransportSession(cursor, payloadLen);
  ASSERT_TRUE(parsedCapsule.hasError());
  EXPECT_EQ(parsedCapsule.error(), CapsuleCodec::ErrorCode::PARSE_UNDERFLOW);
}

TEST(WebTransportFramerTest, DrainWebTransportSession) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  auto writeRes = writeDrainWebTransportSession(queue);
  ASSERT_TRUE(writeRes.hasValue());
  auto buf = queue.move();
  auto cursor = folly::io::Cursor(buf.get());
  auto payloadLen = parseCapsuleHeader(
      cursor, static_cast<uint64_t>(CapsuleType::DRAIN_WEBTRANSPORT_SESSION));

  auto parsedCapsule = parseDrainWebTransportSession(payloadLen);
  ASSERT_TRUE(parsedCapsule.hasValue());
}
