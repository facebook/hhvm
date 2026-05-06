/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxCodec.h>

#include <folly/io/IOBufQueue.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <quic/codec/QuicInteger.h>
#include <quic/folly_utils/Utils.h>
#include <quic/priority/HTTPPriorityQueue.h>

using namespace proxygen;
using namespace proxygen::qmux;

namespace {

struct TestSmInfra {
  struct EgressCb : detail::WtStreamManager::EgressCallback {
    void eventsAvailable() noexcept override {
    }
  };
  struct IngressCb : detail::WtStreamManager::IngressCallback {
    void onNewPeerStream(uint64_t) noexcept override {
    }
  };

  struct StubSession : detail::WtSessionBase {
    using WtSessionBase::WtSessionBase;
    const folly::SocketAddress& getLocalAddress() const override {
      return addr_;
    }
    const folly::SocketAddress& getPeerAddress() const override {
      return addr_;
    }
    folly::SocketAddress addr_;
  };

  EgressCb egressCb;
  IngressCb ingressCb;
  quic::HTTPPriorityQueue priorityQueue;
  detail::WtStreamManager sm{
      detail::WtDir::Client, {}, egressCb, ingressCb, priorityQueue};
  StubSession sess{nullptr, sm};
};

class TestCallback : public QmuxCodec::Callback {
 public:
  TestCallback() : QmuxCodec::Callback(infra_.sm, infra_.sess) {
  }

  void onStream(proxygen::WTStreamCapsule c) noexcept override {
    streams.push_back(std::move(c));
  }
  void onResetStream(proxygen::WTResetStreamCapsule c) noexcept override {
    resetStreams.push_back(std::move(c));
  }
  void onStopSending(proxygen::WTStopSendingCapsule c) noexcept override {
    stopSendings.push_back(std::move(c));
  }
  void onMaxData(proxygen::WTMaxDataCapsule c) noexcept override {
    maxDatas.push_back(c);
  }
  void onMaxStreamData(proxygen::WTMaxStreamDataCapsule c) noexcept override {
    maxStreamDatas.push_back(c);
  }
  void onMaxStreamsBidi(proxygen::WTMaxStreamsCapsule c) noexcept override {
    maxStreamsBidis.push_back(c);
  }
  void onMaxStreamsUni(proxygen::WTMaxStreamsCapsule c) noexcept override {
    maxStreamsUnis.push_back(c);
  }
  void onDataBlocked(proxygen::WTDataBlockedCapsule c) noexcept override {
    dataBlockeds.push_back(c);
  }
  void onStreamDataBlocked(
      proxygen::WTStreamDataBlockedCapsule c) noexcept override {
    streamDataBlockeds.push_back(c);
  }
  void onStreamsBlockedBidi(
      proxygen::WTStreamsBlockedCapsule c) noexcept override {
    streamsBlockedBidis.push_back(c);
  }
  void onStreamsBlockedUni(
      proxygen::WTStreamsBlockedCapsule c) noexcept override {
    streamsBlockedUnis.push_back(c);
  }
  void onDatagram(proxygen::DatagramCapsule c) noexcept override {
    datagrams.push_back(std::move(c));
  }
  void onConnectionClose(QxConnectionClose c) noexcept override {
    connectionCloses.push_back(std::move(c));
  }
  void onTransportParameters(QxTransportParams p) noexcept override {
    transportParams.push_back(std::move(p));
  }
  void onPing(QxPing p) noexcept override {
    pings.push_back(p);
  }
  void onPong(QxPing p) noexcept override {
    pongs.push_back(p);
  }
  void onConnectionError(QmuxErrorCode err) noexcept override {
    connectionErrors.push_back(err);
  }

  std::vector<proxygen::WTStreamCapsule> streams;
  std::vector<proxygen::WTResetStreamCapsule> resetStreams;
  std::vector<proxygen::WTStopSendingCapsule> stopSendings;
  std::vector<proxygen::WTMaxDataCapsule> maxDatas;
  std::vector<proxygen::WTMaxStreamDataCapsule> maxStreamDatas;
  std::vector<proxygen::WTMaxStreamsCapsule> maxStreamsBidis;
  std::vector<proxygen::WTMaxStreamsCapsule> maxStreamsUnis;
  std::vector<proxygen::WTDataBlockedCapsule> dataBlockeds;
  std::vector<proxygen::WTStreamDataBlockedCapsule> streamDataBlockeds;
  std::vector<proxygen::WTStreamsBlockedCapsule> streamsBlockedBidis;
  std::vector<proxygen::WTStreamsBlockedCapsule> streamsBlockedUnis;
  std::vector<proxygen::DatagramCapsule> datagrams;
  std::vector<QxConnectionClose> connectionCloses;
  std::vector<QxTransportParams> transportParams;
  std::vector<QxPing> pings;
  std::vector<QxPing> pongs;
  std::vector<QmuxErrorCode> connectionErrors;

 private:
  TestSmInfra infra_;
};

// Wrap serialized frame bytes into a QMUX record (Size varint + frames)
void appendRecord(folly::IOBufQueue& out, folly::IOBufQueue& frameQueue) {
  writeRecord(out, frameQueue.move());
}

// Write transport params frame wrapped in a record
void writeTPRecord(folly::IOBufQueue& queue, const QxTransportParams& params) {
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  writeTransportParams(frameQueue, params);
  appendRecord(queue, frameQueue);
}

class CodecTestHelper {
 public:
  explicit CodecTestHelper(OffsetValidator validator = nullptr)
      : cb(), codec(&cb, std::move(validator)) {
  }

  void feedTransportParams(const QxTransportParams& params = {}) {
    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    writeTPRecord(queue, params);
    codec.onIngress(queue.move());
  }

  TestCallback cb;
  QmuxCodec codec;
};

} // namespace

//////// First frame must be QX_TRANSPORT_PARAMETERS ////////

TEST(QmuxCodecTest, FirstFrameMustBeTransportParams) {
  CodecTestHelper h;

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 4;
  frame.streamData = folly::IOBuf::copyBuffer("hi");
  frame.fin = false;
  writeWTStream(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
}

TEST(QmuxCodecTest, TransportParamsAccepted) {
  CodecTestHelper h;
  QxTransportParams params;
  params.initialMaxData = 65536;
  params.initialMaxStreamsBidi = 10;

  h.feedTransportParams(params);
  ASSERT_EQ(h.cb.transportParams.size(), 1);
  EXPECT_EQ(h.cb.transportParams[0].initialMaxData, 65536);
  EXPECT_EQ(h.cb.transportParams[0].initialMaxStreamsBidi, 10);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

TEST(QmuxCodecTest, DuplicateTransportParamsRejected) {
  CodecTestHelper h;
  h.feedTransportParams();
  ASSERT_EQ(h.cb.transportParams.size(), 1);
  EXPECT_TRUE(h.cb.connectionErrors.empty());

  // Sending a second QX_TRANSPORT_PARAMETERS frame must fail
  h.feedTransportParams();
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
}

//////// Stream frame dispatch ////////

TEST(QmuxCodecTest, StreamFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 4;
  frame.streamData = folly::IOBuf::copyBuffer("hello");
  frame.fin = true;
  writeWTStream(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.streams.size(), 1);
  EXPECT_EQ(h.cb.streams[0].streamId, 4);
  EXPECT_TRUE(h.cb.streams[0].fin);
  ASSERT_TRUE(h.cb.streams[0].streamData != nullptr);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// RESET_STREAM dispatch ////////

TEST(QmuxCodecTest, ResetStreamFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTResetStreamCapsule frame;
  frame.streamId = 8;
  frame.appProtocolErrorCode = 42;
  frame.reliableSize = 100;
  writeWTResetStream(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.resetStreams.size(), 1);
  EXPECT_EQ(h.cb.resetStreams[0].streamId, 8);
  EXPECT_EQ(h.cb.resetStreams[0].appProtocolErrorCode, 42);
  EXPECT_EQ(h.cb.resetStreams[0].reliableSize, 100);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// STOP_SENDING dispatch ////////

TEST(QmuxCodecTest, StopSendingFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStopSendingCapsule frame;
  frame.streamId = 12;
  frame.appProtocolErrorCode = 99;
  writeWTStopSending(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.stopSendings.size(), 1);
  EXPECT_EQ(h.cb.stopSendings[0].streamId, 12);
  EXPECT_EQ(h.cb.stopSendings[0].appProtocolErrorCode, 99);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// MAX_DATA dispatch ////////

TEST(QmuxCodecTest, MaxDataFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTMaxDataCapsule frame;
  frame.maximumData = 1048576;
  writeWTMaxData(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 1048576);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// MAX_STREAM_DATA dispatch ////////

TEST(QmuxCodecTest, MaxStreamDataFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTMaxStreamDataCapsule frame;
  frame.streamId = 4;
  frame.maximumStreamData = 32768;
  writeWTMaxStreamData(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.maxStreamDatas.size(), 1);
  EXPECT_EQ(h.cb.maxStreamDatas[0].streamId, 4);
  EXPECT_EQ(h.cb.maxStreamDatas[0].maximumStreamData, 32768);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// MAX_STREAMS dispatch ////////

TEST(QmuxCodecTest, MaxStreamsBidiFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTMaxStreamsCapsule frame;
  frame.maximumStreams = 100;
  writeWTMaxStreams(frameQueue, frame, /*isBidi=*/true, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.maxStreamsBidis.size(), 1);
  EXPECT_EQ(h.cb.maxStreamsBidis[0].maximumStreams, 100);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

TEST(QmuxCodecTest, MaxStreamsUniFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTMaxStreamsCapsule frame;
  frame.maximumStreams = 50;
  writeWTMaxStreams(frameQueue, frame, /*isBidi=*/false, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.maxStreamsUnis.size(), 1);
  EXPECT_EQ(h.cb.maxStreamsUnis[0].maximumStreams, 50);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// CONNECTION_CLOSE dispatch ////////

TEST(QmuxCodecTest, ConnectionCloseAppFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  QxConnectionClose frame;
  frame.errorCode = 77;
  frame.isAppError = true;
  frame.reasonPhrase = "bye";
  writeConnectionClose(frameQueue, frame);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionCloses.size(), 1);
  EXPECT_EQ(h.cb.connectionCloses[0].errorCode, 77);
  EXPECT_TRUE(h.cb.connectionCloses[0].isAppError);
  EXPECT_EQ(h.cb.connectionCloses[0].reasonPhrase, "bye");
}

TEST(QmuxCodecTest, ConnectionCloseTransportFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  QxConnectionClose frame;
  frame.errorCode = 0x0a;
  frame.frameType = 0x08;
  frame.isAppError = false;
  frame.reasonPhrase = "bad";
  writeConnectionClose(frameQueue, frame);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionCloses.size(), 1);
  EXPECT_EQ(h.cb.connectionCloses[0].errorCode, 0x0a);
  EXPECT_EQ(h.cb.connectionCloses[0].frameType, 0x08);
  EXPECT_FALSE(h.cb.connectionCloses[0].isAppError);
}

//////// PING/PONG dispatch ////////

TEST(QmuxCodecTest, PingFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  QxPing ping;
  ping.sequenceNumber = 42;
  writePing(frameQueue, ping);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.pings.size(), 1);
  EXPECT_EQ(h.cb.pings[0].sequenceNumber, 42);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

TEST(QmuxCodecTest, PongFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  QxPing pong;
  pong.sequenceNumber = 99;
  writePong(frameQueue, pong);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.pongs.size(), 1);
  EXPECT_EQ(h.cb.pongs[0].sequenceNumber, 99);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// DATAGRAM dispatch ////////

TEST(QmuxCodecTest, DatagramWithLenFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::DatagramCapsule frame;
  frame.httpDatagramPayload = folly::IOBuf::copyBuffer("dgram data");
  writeDatagram(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.datagrams.size(), 1);
  ASSERT_TRUE(h.cb.datagrams[0].httpDatagramPayload != nullptr);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// Prohibited frame types ////////

TEST(QmuxCodecTest, ProhibitedFrameType) {
  CodecTestHelper h;
  h.feedTransportParams();

  // Build a record containing a QUIC PING (0x01) — prohibited in QMUX
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender(&frameQueue, 32);
  auto writeVar = [&](uint64_t val) {
    auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
    (void)quic::encodeQuicInteger(val, op);
  };
  writeVar(0x01);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::FRAME_ENCODING_ERROR);
}

TEST(QmuxCodecTest, UnknownFrameTypeRejected) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender(&frameQueue, 32);
  auto writeVar = [&](uint64_t val) {
    auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
    (void)quic::encodeQuicInteger(val, op);
  };
  writeVar(0x02); // ACK

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::FRAME_ENCODING_ERROR);
}

//////// PADDING before transport params is rejected ////////

TEST(QmuxCodecTest, PaddingBeforeTransportParamsError) {
  CodecTestHelper h;

  // Wrap a PADDING byte (0x00) in a record
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  frameQueue.append(folly::IOBuf::copyBuffer("\x00", 1));

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::TRANSPORT_PARAMETER_ERROR);
}

//////// Incremental feeding ////////

TEST(QmuxCodecTest, IncrementalFeeding) {
  CodecTestHelper h;
  h.feedTransportParams();

  // Write a STREAM frame wrapped in a record
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 4;
  frame.streamData = folly::IOBuf::copyBuffer("hello world");
  frame.fin = false;
  writeWTStream(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue recordQueue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(recordQueue, frameQueue);

  auto fullBuf = recordQueue.move();
  auto totalLen = fullBuf->computeChainDataLength();

  // Feed one byte at a time
  folly::io::Cursor reader(fullBuf.get());
  for (size_t i = 0; i < totalLen; i++) {
    auto byte = folly::IOBuf::create(1);
    byte->append(1);
    reader.pull(byte->writableData(), 1);
    h.codec.onIngress(std::move(byte));
  }

  ASSERT_EQ(h.cb.streams.size(), 1);
  EXPECT_EQ(h.cb.streams[0].streamId, 4);
  EXPECT_FALSE(h.cb.streams[0].fin);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// Multiple frames in single record ////////

TEST(QmuxCodecTest, MultipleFramesInSingleRecord) {
  CodecTestHelper h;

  // TP record first
  folly::IOBufQueue tpQueue{folly::IOBufQueue::cacheChainLength()};
  writeTPRecord(tpQueue, {});
  h.codec.onIngress(tpQueue.move());
  ASSERT_EQ(h.cb.transportParams.size(), 1);

  // Build a record containing a STREAM frame + MAX_DATA frame
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};

  proxygen::WTStreamCapsule streamFrame;
  streamFrame.streamId = 4;
  streamFrame.streamData = folly::IOBuf::copyBuffer("data");
  streamFrame.fin = false;
  writeWTStream(frameQueue, streamFrame, FrameProtocol::QMUX);

  proxygen::WTMaxDataCapsule maxDataFrame;
  maxDataFrame.maximumData = 999;
  writeWTMaxData(frameQueue, maxDataFrame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);
  h.codec.onIngress(queue.move());

  ASSERT_EQ(h.cb.streams.size(), 1);
  EXPECT_EQ(h.cb.streams[0].streamId, 4);
  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 999);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// Multiple records in single onIngress ////////

TEST(QmuxCodecTest, MultipleRecordsInSingleIngress) {
  CodecTestHelper h;

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};

  // Record 1: transport params
  writeTPRecord(queue, {});

  // Record 2: STREAM frame
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    proxygen::WTStreamCapsule streamFrame;
    streamFrame.streamId = 4;
    streamFrame.streamData = folly::IOBuf::copyBuffer("data");
    streamFrame.fin = false;
    writeWTStream(frameQueue, streamFrame, FrameProtocol::QMUX);
    appendRecord(queue, frameQueue);
  }

  // Record 3: MAX_DATA frame
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    proxygen::WTMaxDataCapsule maxDataFrame;
    maxDataFrame.maximumData = 999;
    writeWTMaxData(frameQueue, maxDataFrame, FrameProtocol::QMUX);
    appendRecord(queue, frameQueue);
  }

  h.codec.onIngress(queue.move());

  ASSERT_EQ(h.cb.transportParams.size(), 1);
  ASSERT_EQ(h.cb.streams.size(), 1);
  EXPECT_EQ(h.cb.streams[0].streamId, 4);
  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 999);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// Unknown frame type ////////

TEST(QmuxCodecTest, UnknownFrameType) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender(&frameQueue, 32);
  auto writeVar = [&](uint64_t val) {
    auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
    (void)quic::encodeQuicInteger(val, op);
  };
  writeVar(0xff);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::FRAME_ENCODING_ERROR);
}

//////// After connection error, further ingress is ignored ////////

TEST(QmuxCodecTest, AfterErrorIngressIgnored) {
  CodecTestHelper h;

  // Trigger error by sending non-TP first (inside a record)
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTMaxDataCapsule frame;
  frame.maximumData = 100;
  writeWTMaxData(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);
  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);

  // Now send valid TP - should be ignored
  folly::IOBufQueue queue2{folly::IOBufQueue::cacheChainLength()};
  writeTPRecord(queue2, {});
  h.codec.onIngress(queue2.move());
  EXPECT_TRUE(h.cb.transportParams.empty());
  EXPECT_EQ(h.cb.connectionErrors.size(), 1);
}

//////// DATA_BLOCKED, STREAM_DATA_BLOCKED, STREAMS_BLOCKED dispatch ////////

TEST(QmuxCodecTest, DataBlockedFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTDataBlockedCapsule frame;
  frame.maximumData = 2048;
  writeWTDataBlocked(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.dataBlockeds.size(), 1);
  EXPECT_EQ(h.cb.dataBlockeds[0].maximumData, 2048);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

TEST(QmuxCodecTest, StreamDataBlockedFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamDataBlockedCapsule frame;
  frame.streamId = 16;
  frame.maximumStreamData = 4096;
  writeWTStreamDataBlocked(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.streamDataBlockeds.size(), 1);
  EXPECT_EQ(h.cb.streamDataBlockeds[0].streamId, 16);
  EXPECT_EQ(h.cb.streamDataBlockeds[0].maximumStreamData, 4096);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

TEST(QmuxCodecTest, StreamsBlockedBidiFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamsBlockedCapsule frame;
  frame.maximumStreams = 10;
  writeWTStreamsBlocked(
      frameQueue, frame, /*isBidi=*/true, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.streamsBlockedBidis.size(), 1);
  EXPECT_EQ(h.cb.streamsBlockedBidis[0].maximumStreams, 10);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

TEST(QmuxCodecTest, StreamsBlockedUniFrame) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamsBlockedCapsule frame;
  frame.maximumStreams = 5;
  writeWTStreamsBlocked(
      frameQueue, frame, /*isBidi=*/false, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.streamsBlockedUnis.size(), 1);
  EXPECT_EQ(h.cb.streamsBlockedUnis[0].maximumStreams, 5);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// STREAM no-LEN uses record boundary for data extent ////////

TEST(QmuxCodecTest, StreamNoLenUsesRecordBoundary) {
  CodecTestHelper h;
  h.feedTransportParams();

  // Manually build a no-LEN STREAM frame: type=0x08 (no FIN/LEN/OFF),
  // streamId=4, data="hi"
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender(&frameQueue, 128);
  auto writeVar = [&](uint64_t val) {
    auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
    (void)quic::encodeQuicInteger(val, op);
  };

  writeVar(0x08); // STREAM_BASE, no flags
  writeVar(4);    // streamId
  appender.push(reinterpret_cast<const uint8_t*>("hi"), 2);

  // Wrap in a record — the record boundary defines the data extent
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());

  ASSERT_EQ(h.cb.streams.size(), 1);
  EXPECT_EQ(h.cb.streams[0].streamId, 4);
  ASSERT_TRUE(h.cb.streams[0].streamData != nullptr);
  EXPECT_EQ(h.cb.streams[0].streamData->moveToFbString().toStdString(),
            std::string("hi"));
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// STREAM no-LEN followed by another frame in same record ////////

TEST(QmuxCodecTest, StreamNoLenConsumesRestOfRecord) {
  CodecTestHelper h;
  h.feedTransportParams();

  // A no-LEN STREAM frame consumes all remaining bytes in the record.
  // Put STREAM no-LEN in one record, MAX_DATA in another.
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};

  // Record 1: STREAM no-LEN
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    folly::io::QueueAppender appender(&frameQueue, 128);
    auto writeVar = [&](uint64_t val) {
      auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
      (void)quic::encodeQuicInteger(val, op);
    };
    writeVar(0x08);
    writeVar(4);
    appender.push(reinterpret_cast<const uint8_t*>("hi"), 2);
    appendRecord(queue, frameQueue);
  }

  // Record 2: MAX_DATA
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    proxygen::WTMaxDataCapsule maxDataFrame;
    maxDataFrame.maximumData = 9999;
    writeWTMaxData(frameQueue, maxDataFrame, FrameProtocol::QMUX);
    appendRecord(queue, frameQueue);
  }

  h.codec.onIngress(queue.move());

  ASSERT_EQ(h.cb.streams.size(), 1);
  EXPECT_EQ(h.cb.streams[0].streamId, 4);
  ASSERT_TRUE(h.cb.streams[0].streamData != nullptr);
  EXPECT_EQ(h.cb.streams[0].streamData->moveToFbString().toStdString(),
            std::string("hi"));

  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 9999);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// DATAGRAM_NO_LEN uses record boundary ////////

TEST(QmuxCodecTest, DatagramNoLenUsesRecordBoundary) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};

  // Record 1: DATAGRAM_NO_LEN
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    folly::io::QueueAppender appender(&frameQueue, 128);
    auto writeVar = [&](uint64_t val) {
      auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
      (void)quic::encodeQuicInteger(val, op);
    };
    writeVar(qval(QmuxFrameType::DATAGRAM_NO_LEN));
    appender.push(reinterpret_cast<const uint8_t*>("abc"), 3);
    appendRecord(queue, frameQueue);
  }

  // Record 2: MAX_DATA
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    proxygen::WTMaxDataCapsule maxDataFrame;
    maxDataFrame.maximumData = 7777;
    writeWTMaxData(frameQueue, maxDataFrame, FrameProtocol::QMUX);
    appendRecord(queue, frameQueue);
  }

  h.codec.onIngress(queue.move());

  ASSERT_EQ(h.cb.datagrams.size(), 1);
  ASSERT_TRUE(h.cb.datagrams[0].httpDatagramPayload != nullptr);
  EXPECT_EQ(
      h.cb.datagrams[0].httpDatagramPayload->moveToFbString().toStdString(),
      std::string("abc"));

  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 7777);
  EXPECT_TRUE(h.cb.connectionErrors.empty());
}

//////// Record size exceeds max_record_size ////////

TEST(QmuxCodecTest, RecordExceedsMaxRecordSize) {
  CodecTestHelper h;
  h.feedTransportParams();

  h.codec.setMaxRecordSize(10);

  // Build a record with Size > 10
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTStreamCapsule frame;
  frame.streamId = 4;
  frame.streamData = folly::IOBuf::copyBuffer("this is more than ten bytes");
  frame.fin = false;
  writeWTStream(frameQueue, frame, FrameProtocol::QMUX);

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());
  ASSERT_EQ(h.cb.connectionErrors.size(), 1);
  EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::FRAME_ENCODING_ERROR);
}

//////// Empty record (Size = 0) is silently accepted ////////

TEST(QmuxCodecTest, EmptyRecordAccepted) {
  CodecTestHelper h;
  h.feedTransportParams();

  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender(&queue, 64);
  auto writeVar = [&](uint64_t val) {
    auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
    (void)quic::encodeQuicInteger(val, op);
  };

  // Empty record: Size varint = 0, no payload.
  writeVar(0);

  // A non-empty record after the empty one — proves the parser advanced
  // exactly one byte and continued processing.
  folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
  proxygen::WTMaxDataCapsule maxDataFrame;
  maxDataFrame.maximumData = 555;
  writeWTMaxData(frameQueue, maxDataFrame, FrameProtocol::QMUX);
  appendRecord(queue, frameQueue);

  h.codec.onIngress(queue.move());

  EXPECT_TRUE(h.cb.connectionErrors.empty());
  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 555);
}

//////// Truncated frame in record ////////

TEST(QmuxCodecTest, TruncatedFrameInRecord) {
  CodecTestHelper h;
  h.feedTransportParams();

  // Build a record whose Size is larger than the frame it contains,
  // leaving leftover bytes. The codec should detect this after parsing.
  folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
  folly::io::QueueAppender appender(&queue, 64);
  auto writeVar = [&](uint64_t val) {
    auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
    (void)quic::encodeQuicInteger(val, op);
  };

  // Record Size = 5, but frame only uses 2 bytes (type + varint)
  writeVar(5);                             // Size
  writeVar(qval(QmuxFrameType::MAX_DATA)); // type (1 byte)
  writeVar(100);                           // maximumData (1 byte)
  // 3 leftover bytes — these aren't a valid frame start
  appender.push(reinterpret_cast<const uint8_t*>("\x00\x00\x00"), 3);

  h.codec.onIngress(queue.move());
  // The leftover PADDING bytes (0x00) are valid, so this should succeed
  EXPECT_TRUE(h.cb.connectionErrors.empty());
  ASSERT_EQ(h.cb.maxDatas.size(), 1);
  EXPECT_EQ(h.cb.maxDatas[0].maximumData, 100);
}

//////// STREAM offset validation via codec ////////

TEST(QmuxCodecTest, StreamOffsetViolation) {
  auto offsets = std::make_shared<std::unordered_map<uint64_t, uint64_t>>();
  CodecTestHelper h([offsets](uint64_t streamId, uint64_t offset) {
    return offset == (*offsets)[streamId];
  });

  h.feedTransportParams();

  // First: send a valid STREAM frame with LEN+OFF, offset=0, 5 bytes
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    folly::io::QueueAppender appender(&frameQueue, 64);
    auto writeVar = [&](uint64_t val) {
      auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
      (void)quic::encodeQuicInteger(val, op);
    };
    writeVar(qval(QmuxFrameType::STREAM_BASE) | kStreamFlagOff |
             kStreamFlagLen);
    writeVar(4); // streamId
    writeVar(0); // offset
    writeVar(5); // length
    appender.push(reinterpret_cast<const uint8_t*>("hello"), 5);

    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    appendRecord(queue, frameQueue);

    h.codec.onIngress(queue.move());
    ASSERT_EQ(h.cb.streams.size(), 1);
    EXPECT_EQ(h.cb.streams[0].streamId, 4);
    EXPECT_TRUE(h.cb.connectionErrors.empty());
    (*offsets)[4] += 5;
  }

  // Second: send STREAM with wrong offset (99 instead of 5)
  {
    folly::IOBufQueue frameQueue{folly::IOBufQueue::cacheChainLength()};
    folly::io::QueueAppender appender(&frameQueue, 64);
    auto writeVar = [&](uint64_t val) {
      auto op = [&](auto v) { appender.writeBE(folly::tag<decltype(v)>, v); };
      (void)quic::encodeQuicInteger(val, op);
    };
    writeVar(qval(QmuxFrameType::STREAM_BASE) | kStreamFlagOff |
             kStreamFlagLen);
    writeVar(4);  // streamId
    writeVar(99); // wrong offset
    writeVar(5);  // length
    appender.push(reinterpret_cast<const uint8_t*>("world"), 5);

    folly::IOBufQueue queue{folly::IOBufQueue::cacheChainLength()};
    appendRecord(queue, frameQueue);

    h.codec.onIngress(queue.move());
    ASSERT_EQ(h.cb.connectionErrors.size(), 1);
    EXPECT_EQ(h.cb.connectionErrors[0], QmuxErrorCode::PROTOCOL_VIOLATION);
  }
}
