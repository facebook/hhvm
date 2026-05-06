/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <proxygen/lib/transport/qmux/QmuxFramer.h>
#include <quic/codec/QuicInteger.h>
#include <quic/folly_utils/Utils.h>

using namespace proxygen::qmux;
using proxygen::QmuxFrameType;

namespace {

folly::io::Cursor cursorFromQueue(folly::IOBufQueue& queue) {
  return folly::io::Cursor(queue.front());
}

// Helper to build a raw QMUX frame payload with varints
class FrameBuilder {
 public:
  void writeVar(uint64_t val) {
    folly::io::QueueAppender appender(&queue_, 64);
    (void)quic::encodeQuicInteger(val, [&appender](auto v) {
      appender.writeBE(folly::tag<decltype(v)>, v);
    });
  }

  void appendData(const std::string& data) {
    queue_.append(folly::IOBuf::copyBuffer(data));
  }

  folly::io::Cursor cursor() {
    return cursorFromQueue(queue_);
  }

  size_t length() const {
    return queue_.chainLength();
  }

  folly::IOBufQueue moveQueue() {
    return {std::move(queue_)};
  }

  folly::IOBufQueue queue_{folly::IOBufQueue::cacheChainLength()};
};

} // namespace

//////// STREAM frame (QMUX-specific: offset tracking, flags) ////////

TEST(QmuxFramerTest, StreamRoundTrip) {
  // Use the shared writer with QMUX protocol
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 4;
  frame.streamData = folly::IOBuf::copyBuffer("hello");
  frame.fin = false;

  auto written =
      proxygen::writeWTStream(queue, frame, proxygen::FrameProtocol::QMUX);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  available -= typeOpt->second;
  // writeWTStream with QMUX uses LEN bit (0x0a)
  EXPECT_EQ(typeOpt->first, qval(QmuxFrameType::STREAM_BASE) | kStreamFlagLen);

  auto flags = static_cast<uint8_t>(typeOpt->first & 0x07);
  auto result = parseStream(cursor, flags, available);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->streamId, 4);
  EXPECT_FALSE(result->fin);
  ASSERT_TRUE(result->streamData != nullptr);
  EXPECT_TRUE(folly::IOBufEqualTo()(result->streamData, frame.streamData));
}

TEST(QmuxFramerTest, StreamRoundTripWithFin) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 8;
  frame.streamData = folly::IOBuf::copyBuffer("world");
  frame.fin = true;

  auto written =
      proxygen::writeWTStream(queue, frame, proxygen::FrameProtocol::QMUX);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  available -= typeOpt->second;
  EXPECT_EQ(typeOpt->first,
            qval(QmuxFrameType::STREAM_BASE) | kStreamFlagLen | kStreamFlagFin);

  auto flags = static_cast<uint8_t>(typeOpt->first & 0x07);
  auto result = parseStream(cursor, flags, available);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->streamId, 8);
  EXPECT_TRUE(result->fin);
  ASSERT_TRUE(result->streamData != nullptr);
  EXPECT_TRUE(folly::IOBufEqualTo()(result->streamData, frame.streamData));
}

TEST(QmuxFramerTest, StreamEmptyData) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 12;
  frame.streamData = nullptr;
  frame.fin = true;

  auto written =
      proxygen::writeWTStream(queue, frame, proxygen::FrameProtocol::QMUX);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  available -= typeOpt->second;
  auto flags = static_cast<uint8_t>(typeOpt->first & 0x07);
  auto result = parseStream(cursor, flags, available);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->streamId, 12);
  EXPECT_TRUE(result->fin);
}

TEST(QmuxFramerTest, ParseStreamWithOffset) {
  FrameBuilder fb;
  fb.writeVar(4); // Stream ID
  fb.writeVar(0); // Offset = 0
  fb.writeVar(5); // Length = 5
  fb.appendData("hello");

  auto cursor = fb.cursor();
  uint8_t flags = kStreamFlagOff | kStreamFlagLen | kStreamFlagFin;
  size_t len = fb.length();
  auto result = parseStream(
      cursor, flags, len, [](uint64_t /*streamId*/, uint64_t offset) {
        return offset == 0;
      });
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->streamId, 4);
  EXPECT_TRUE(result->fin);
  ASSERT_TRUE(result->streamData != nullptr);
  EXPECT_EQ(result->streamData->moveToFbString().toStdString(),
            std::string("hello"));
}

TEST(QmuxFramerTest, StreamOffsetCorrectSequence) {
  std::unordered_map<uint64_t, uint64_t> offsets;
  auto validator = [&](uint64_t streamId, uint64_t offset) {
    return offset == offsets[streamId];
  };

  // First frame: stream 4, offset 0, 5 bytes
  {
    FrameBuilder fb;
    fb.writeVar(4); // streamId
    fb.writeVar(0); // offset
    fb.writeVar(5); // length
    fb.appendData("hello");

    auto cursor = fb.cursor();
    uint8_t flags = kStreamFlagOff | kStreamFlagLen;
    size_t len = fb.length();
    auto result = parseStream(cursor, flags, len, validator);
    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(result->streamId, 4);
    offsets[4] += 5;
  }

  // Second frame: stream 4, offset 5 (correct), 5 more bytes
  {
    FrameBuilder fb;
    fb.writeVar(4); // streamId
    fb.writeVar(5); // offset = 5 (correct)
    fb.writeVar(5); // length
    fb.appendData("world");

    auto cursor = fb.cursor();
    uint8_t flags = kStreamFlagOff | kStreamFlagLen;
    size_t len = fb.length();
    auto result = parseStream(cursor, flags, len, validator);
    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ(result->streamId, 4);
  }
}

TEST(QmuxFramerTest, StreamOffsetWrong) {
  std::unordered_map<uint64_t, uint64_t> offsets;
  auto validator = [&](uint64_t streamId, uint64_t offset) {
    return offset == offsets[streamId];
  };

  // First frame: stream 4, offset 0, 5 bytes
  {
    FrameBuilder fb;
    fb.writeVar(4);
    fb.writeVar(0);
    fb.writeVar(5);
    fb.appendData("hello");

    auto cursor = fb.cursor();
    uint8_t flags = kStreamFlagOff | kStreamFlagLen;
    size_t len = fb.length();
    auto result = parseStream(cursor, flags, len, validator);
    ASSERT_TRUE(result.hasValue());
    offsets[4] += 5;
  }

  // Second frame: stream 4, offset 99 (wrong, should be 5)
  {
    FrameBuilder fb;
    fb.writeVar(4);
    fb.writeVar(99); // wrong offset
    fb.writeVar(5);
    fb.appendData("world");

    auto cursor = fb.cursor();
    uint8_t flags = kStreamFlagOff | kStreamFlagLen;
    size_t len = fb.length();
    auto result = parseStream(cursor, flags, len, validator);
    ASSERT_TRUE(result.hasError());
    EXPECT_EQ(result.error(), QmuxErrorCode::PROTOCOL_VIOLATION);
  }
}

TEST(QmuxFramerTest, StreamOffsetValidatorCalled) {
  // Verify the validator is called with the correct streamId and offset
  uint64_t validatedStreamId = 0;
  uint64_t validatedOffset = 0;
  auto validator = [&](uint64_t streamId, uint64_t offset) {
    validatedStreamId = streamId;
    validatedOffset = offset;
    return true;
  };

  FrameBuilder fb;
  fb.writeVar(4);  // streamId
  fb.writeVar(42); // offset
  fb.writeVar(5);  // length
  fb.appendData("hello");

  auto cursor = fb.cursor();
  uint8_t flags = kStreamFlagOff | kStreamFlagLen;
  size_t len = fb.length();
  auto result = parseStream(cursor, flags, len, validator);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(validatedStreamId, 4);
  EXPECT_EQ(validatedOffset, 42);
}

TEST(QmuxFramerTest, ParseStreamUnderflow) {
  auto buf = folly::IOBuf::create(0);
  folly::io::Cursor cursor(buf.get());
  size_t len = 0;
  auto result = parseStream(cursor, kStreamFlagLen, len);
  EXPECT_TRUE(result.hasError());
  EXPECT_EQ(result.error(), QmuxErrorCode::PARSE_UNDERFLOW);
}

TEST(QmuxFramerTest, ParseStreamDataLenExceedsFrameLength) {
  // Build a STREAM frame where LEN field claims more data than the frame has
  FrameBuilder fb;
  fb.writeVar(4);   // streamId
  fb.writeVar(100); // dataLen = 100, but only 5 bytes of data follow
  fb.appendData("hello");

  auto cursor = fb.cursor();
  size_t len = fb.length();
  auto result = parseStream(cursor, kStreamFlagLen, len);
  EXPECT_TRUE(result.hasError());
  EXPECT_EQ(result.error(), QmuxErrorCode::PARSE_UNDERFLOW);
}

//////// CONNECTION_CLOSE round-trip ////////

TEST(QmuxFramerTest, ConnectionCloseTransportRoundTrip) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxConnectionClose frame;
  frame.errorCode = 0x0a;
  frame.frameType = qval(QmuxFrameType::STREAM_BASE);
  frame.isAppError = false;
  frame.reasonPhrase = "bad stream";

  auto written = writeConnectionClose(queue, frame);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  EXPECT_EQ(typeOpt->first, qval(QmuxFrameType::CONNECTION_CLOSE));
  available -= typeOpt->second;

  auto result = parseConnectionClose(cursor, available, /*isAppError=*/false);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->errorCode, 0x0a);
  EXPECT_EQ(result->frameType, qval(QmuxFrameType::STREAM_BASE));
  EXPECT_FALSE(result->isAppError);
  EXPECT_EQ(result->reasonPhrase, "bad stream");
}

TEST(QmuxFramerTest, ConnectionCloseAppRoundTrip) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxConnectionClose frame;
  frame.errorCode = 77;
  frame.isAppError = true;
  frame.reasonPhrase = "app error";

  auto written = writeConnectionClose(queue, frame);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  EXPECT_EQ(typeOpt->first, qval(QmuxFrameType::CONNECTION_CLOSE_APP));
  available -= typeOpt->second;

  auto result = parseConnectionClose(cursor, available, /*isAppError=*/true);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->errorCode, 77);
  EXPECT_TRUE(result->isAppError);
  EXPECT_EQ(result->reasonPhrase, "app error");
}

TEST(QmuxFramerTest, ConnectionCloseNoReason) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxConnectionClose frame;
  frame.errorCode = 0;
  frame.isAppError = true;

  auto written = writeConnectionClose(queue, frame);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  available -= typeOpt->second;

  auto result = parseConnectionClose(cursor, available, /*isAppError=*/true);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->errorCode, 0);
  EXPECT_TRUE(result->reasonPhrase.empty());
}

TEST(QmuxFramerTest, ParseConnectionCloseUnderflow) {
  auto buf = folly::IOBuf::create(0);
  folly::io::Cursor cursor(buf.get());
  size_t len = 0;
  auto result = parseConnectionClose(cursor, len, false);
  EXPECT_TRUE(result.hasError());
  EXPECT_EQ(result.error(), QmuxErrorCode::PARSE_UNDERFLOW);
}

//////// QX_TRANSPORT_PARAMETERS round-trip ////////

TEST(QmuxFramerTest, TransportParamsRoundTrip) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxTransportParams params;
  params.maxIdleTimeout = 30000;
  params.initialMaxData = 1048576;
  params.initialMaxStreamDataBidiLocal = 65536;
  params.initialMaxStreamDataBidiRemote = 65536;
  params.initialMaxStreamDataUni = 32768;
  params.initialMaxStreamsBidi = 100;
  params.initialMaxStreamsUni = 100;
  params.maxRecordSize = 32768;
  params.maxDatagramFrameSize = 1200;

  auto written = writeTransportParams(queue, params);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  EXPECT_EQ(typeOpt->first, qval(QmuxFrameType::QX_TRANSPORT_PARAMS));
  available -= typeOpt->second;

  auto lenOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(lenOpt.has_value());
  size_t payloadLen = lenOpt->first;

  auto result = parseTransportParams(cursor, payloadLen);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->maxIdleTimeout, 30000);
  EXPECT_EQ(result->initialMaxData, 1048576);
  EXPECT_EQ(result->initialMaxStreamDataBidiLocal, 65536);
  EXPECT_EQ(result->initialMaxStreamDataBidiRemote, 65536);
  EXPECT_EQ(result->initialMaxStreamDataUni, 32768);
  EXPECT_EQ(result->initialMaxStreamsBidi, 100);
  EXPECT_EQ(result->initialMaxStreamsUni, 100);
  EXPECT_EQ(result->maxRecordSize, 32768);
  ASSERT_TRUE(result->maxDatagramFrameSize.hasValue());
  EXPECT_EQ(*result->maxDatagramFrameSize, 1200);
}

TEST(QmuxFramerTest, TransportParamsDefaultValues) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxTransportParams params; // all defaults

  auto written = writeTransportParams(queue, params);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  available -= typeOpt->second;
  auto lenOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(lenOpt.has_value());
  // Default values are all 0 or default, so payload should be empty
  EXPECT_EQ(lenOpt->first, 0);
}

TEST(QmuxFramerTest, TransportParamsProhibitedParam) {
  FrameBuilder fb;
  // param ID = 0x00 (prohibited), length = 1, value = 0
  fb.writeVar(0x00);
  fb.writeVar(1);
  fb.writeVar(0);

  auto cursor = fb.cursor();
  size_t len = fb.length();
  auto result = parseTransportParams(cursor, len);
  ASSERT_TRUE(result.hasError());
  EXPECT_EQ(result.error(), QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
}

TEST(QmuxFramerTest, MaxRecordSizeTooSmall) {
  auto buildTPWithMaxRecordSize = [](uint64_t value) {
    FrameBuilder fb;
    auto valSize = quic::getQuicIntegerSize(value);
    fb.writeVar(kTpMaxRecordSize);
    fb.writeVar(*valSize);
    fb.writeVar(value);
    return fb.moveQueue();
  };

  // Values below kDefaultMaxRecordSize (16382) should fail
  for (uint64_t val : {uint64_t(0), uint64_t(100), uint64_t(16381)}) {
    auto queue = buildTPWithMaxRecordSize(val);
    auto cursor = cursorFromQueue(queue);
    size_t len = queue.chainLength();
    auto result = parseTransportParams(cursor, len);
    ASSERT_TRUE(result.hasError()) << "Expected failure for value " << val;
    EXPECT_EQ(result.error(), QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
  }

  // Values >= kDefaultMaxRecordSize should succeed
  for (uint64_t val : {uint64_t(16382), uint64_t(65535)}) {
    auto queue = buildTPWithMaxRecordSize(val);
    auto cursor = cursorFromQueue(queue);
    size_t len = queue.chainLength();
    auto result = parseTransportParams(cursor, len);
    ASSERT_TRUE(result.hasValue()) << "Expected success for value " << val;
    EXPECT_EQ(result->maxRecordSize, val);
  }
}

//////// QX_PING / QX_PONG round-trip ////////

TEST(QmuxFramerTest, PingRoundTrip) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxPing ping;
  ping.sequenceNumber = 42;

  auto written = writePing(queue, ping);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  EXPECT_EQ(typeOpt->first, qval(QmuxFrameType::QX_PING));
  available -= typeOpt->second;

  auto result = parsePing(cursor, available);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->sequenceNumber, 42);
}

TEST(QmuxFramerTest, PongRoundTrip) {
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  QxPing pong;
  pong.sequenceNumber = 99;

  auto written = writePong(queue, pong);
  ASSERT_TRUE(written.has_value());

  auto cursor = cursorFromQueue(queue);
  auto available = queue.chainLength();
  auto typeOpt = quic::follyutils::decodeQuicInteger(cursor, available);
  ASSERT_TRUE(typeOpt.has_value());
  EXPECT_EQ(typeOpt->first, qval(QmuxFrameType::QX_PONG));
  available -= typeOpt->second;

  auto result = parsePing(cursor, available);
  ASSERT_TRUE(result.hasValue());
  EXPECT_EQ(result->sequenceNumber, 99);
}

TEST(QmuxFramerTest, PingUnderflow) {
  auto buf = folly::IOBuf::create(0);
  folly::io::Cursor cursor(buf.get());
  size_t len = 0;
  auto result = parsePing(cursor, len);
  EXPECT_TRUE(result.hasError());
  EXPECT_EQ(result.error(), QmuxErrorCode::PARSE_UNDERFLOW);
}
