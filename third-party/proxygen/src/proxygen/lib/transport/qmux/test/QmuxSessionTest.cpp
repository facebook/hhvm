/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxSession.h>

#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportFramer.h>
#include <proxygen/lib/http/coro/transport/test/TestCoroTransport.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <proxygen/lib/transport/qmux/QmuxCodec.h>
#include <proxygen/lib/transport/qmux/QmuxFramer.h>

using namespace proxygen;
using namespace proxygen::qmux;
using proxygen::coro::test::TestCoroTransport;

namespace {

//////// Wire-bytes parser (decodes what the session wrote) ////////
//
// A self-contained QmuxCodec + Callback that captures every dispatched event
// into vectors, so tests can inspect what the session emitted on the wire.

struct StubSm {
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
    [[nodiscard]] const folly::SocketAddress& getLocalAddress() const override {
      return addr;
    }
    [[nodiscard]] const folly::SocketAddress& getPeerAddress() const override {
      return addr;
    }
    folly::SocketAddress addr;
  };
  EgressCb egressCb;
  IngressCb ingressCb;
  quic::HTTPPriorityQueue pq;
  // The peer parser tracks its own offsets so the OffsetValidator returns the
  // bytes-already-received per stream.
  std::unordered_map<uint64_t, uint64_t> rxOffsets;
  detail::WtStreamManager sm{
      detail::WtDir::Client, {}, egressCb, ingressCb, pq};
  StubSession session{nullptr, sm};
};

class WireCallback : public QmuxCodec::Callback {
 public:
  explicit WireCallback(StubSm& infra)
      : QmuxCodec::Callback(infra.sm, infra.session) {
  }

  void onStream(WTStreamCapsule c) noexcept override {
    streams.push_back(std::move(c));
  }
  void onDatagram(DatagramCapsule c) noexcept override {
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

  std::vector<WTStreamCapsule> streams;
  std::vector<DatagramCapsule> datagrams;
  std::vector<QxConnectionClose> connectionCloses;
  std::vector<QxTransportParams> transportParams;
  std::vector<QxPing> pings;
  std::vector<QxPing> pongs;
  std::vector<QmuxErrorCode> connectionErrors;
};

class WireParser {
 public:
  WireParser()
      : cb(infra), codec(&cb, [this](uint64_t id, uint64_t off) {
          // Validator must accept contiguous offsets; track per-stream.
          return off == infra.rxOffsets[id];
        }) {
    codec.setMaxRecordSize(kDefaultMaxRecordSize);
  }

  void feed(std::unique_ptr<folly::IOBuf> bytes) {
    codec.onIngress(std::move(bytes));
    // Update tracked offsets so subsequent STREAM frames pass the validator.
    for (auto& s : cb.streams) {
      auto len =
          s.streamData ? s.streamData->computeChainDataLength() : uint64_t{0};
      infra.rxOffsets[s.streamId] += len;
    }
  }

  StubSm infra;
  WireCallback cb;
  QmuxCodec codec;
};

//////// Wire-bytes builders ////////

std::unique_ptr<folly::IOBuf> wrapInRecord(folly::IOBufQueue& frames) {
  folly::IOBufQueue out{folly::IOBufQueue::cacheChainLength()};
  writeRecord(out, frames.move());
  return out.move();
}

std::unique_ptr<folly::IOBuf> streamRecord(uint64_t streamId,
                                           const std::string& data,
                                           bool fin) {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writeWTStream(frames,
                WTStreamCapsule{.streamId = streamId,
                                .streamData = folly::IOBuf::copyBuffer(data),
                                .fin = fin},
                FrameProtocol::QMUX);
  return wrapInRecord(frames);
}

std::unique_ptr<folly::IOBuf> datagramRecord(const std::string& data) {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writeDatagram(
      frames,
      DatagramCapsule{.httpDatagramPayload = folly::IOBuf::copyBuffer(data)},
      FrameProtocol::QMUX);
  return wrapInRecord(frames);
}

std::unique_ptr<folly::IOBuf> pingRecord(uint64_t seq) {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writePing(frames, QxPing{.sequenceNumber = seq});
  return wrapInRecord(frames);
}

std::unique_ptr<folly::IOBuf> pingRecords(uint64_t count) {
  folly::IOBufQueue out{folly::IOBufQueue::cacheChainLength()};
  for (uint64_t seq = 0; seq < count; ++seq) {
    folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
    writePing(frames, QxPing{.sequenceNumber = seq});
    writeRecord(out, frames.move());
  }
  return out.move();
}

std::unique_ptr<folly::IOBuf> connectionCloseRecord(uint64_t err,
                                                    std::string reason) {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writeConnectionClose(frames,
                       QxConnectionClose{.errorCode = err,
                                         .frameType = 0,
                                         .isAppError = true,
                                         .reasonPhrase = std::move(reason)});
  return wrapInRecord(frames);
}

//////// Test handler ////////

class TestWtHandler : public WebTransportHandler {
 public:
  void onWebTransportSession(
      std::shared_ptr<WebTransport> wt) noexcept override {
    sessionStarts++;
    wtSession = std::move(wt);
  }
  void onNewUniStream(WebTransport::StreamReadHandle* rh) noexcept override {
    newUniStreamIds.push_back(rh->getID());
  }
  void onNewBidiStream(WebTransport::BidiStreamHandle bh) noexcept override {
    newBidiStreamIds.push_back(bh.writeHandle->getID());
  }
  void onDatagram(std::unique_ptr<folly::IOBuf> dg) noexcept override {
    datagrams.push_back(
        dg ? dg->cloneCoalescedAsValue().moveToFbString().toStdString()
           : std::string{});
  }
  void onSessionEnd(folly::Optional<uint32_t> err) noexcept override {
    sessionEndCount++;
    sessionEndError = err;
  }
  void onSessionDrain() noexcept override {
    sessionDrains++;
  }

  std::shared_ptr<WebTransport> wtSession;
  std::vector<uint64_t> newUniStreamIds;
  std::vector<uint64_t> newBidiStreamIds;
  std::vector<std::string> datagrams;
  int sessionStarts{0};
  int sessionEndCount{0};
  int sessionDrains{0};
  folly::Optional<uint32_t> sessionEndError;
};

//////// Fixture ////////

class QmuxSessionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    state_ = std::make_unique<TestCoroTransport::State>();
    auto transport = std::make_unique<TestCoroTransport>(&evb_, state_.get());
    transport_ = transport.get();

    QxTransportParams selfParams;
    selfParams.initialMaxData = 1 << 20;
    selfParams.initialMaxStreamsBidi = kSelfMaxStreams;
    selfParams.initialMaxStreamsUni = kSelfMaxStreams;
    selfParams.initialMaxStreamDataBidiLocal = 1 << 16;
    selfParams.initialMaxStreamDataBidiRemote = 1 << 16;
    selfParams.initialMaxStreamDataUni = 1 << 16;

    // Stand in for the peer-side TPs that QmuxConnector would deliver in
    // production; the values pre-grant peer flow-control credit so the
    // session can open and accept streams immediately.
    QxTransportParams peerParams;
    peerParams.initialMaxData = 1 << 20;
    peerParams.initialMaxStreamsBidi = 5;
    peerParams.initialMaxStreamsUni = 5;
    peerParams.initialMaxStreamDataBidiRemote = 1 << 16;
    peerParams.initialMaxStreamDataUni = 1 << 16;

    WtStreamManager::WtConfig wtConfig{
        .selfMaxStreamsBidi = selfParams.initialMaxStreamsBidi,
        .selfMaxStreamsUni = selfParams.initialMaxStreamsUni,
        .selfMaxConnData = selfParams.initialMaxData,
        .selfMaxStreamDataBidi = selfParams.initialMaxStreamDataBidiLocal,
        .selfMaxStreamDataUni = selfParams.initialMaxStreamDataUni,
        .peerMaxStreamsBidi = peerParams.initialMaxStreamsBidi,
        .peerMaxStreamsUni = peerParams.initialMaxStreamsUni,
        .peerMaxConnData = peerParams.initialMaxData,
        .peerMaxStreamDataBidi = peerParams.initialMaxStreamDataBidiRemote,
        .peerMaxStreamDataUni = peerParams.initialMaxStreamDataUni};

    session_ = std::make_shared<QmuxSession>(&evb_,
                                             WtDir::Server,
                                             selfParams,
                                             std::move(transport),
                                             std::move(wtConfig),
                                             peerParams.maxRecordSize,
                                             /*effectiveMaxIdleTimeoutMs=*/0,
                                             /*initialIngress=*/nullptr,
                                             QmuxSession::Config{});
    selfParams_ = selfParams;
    handler_ = std::make_unique<TestWtHandler>();
    session_->setHandler(handler_.get());
    session_->start(session_);
    // Drive the initial scheduling so both coroutines reach their first
    // suspension point (read loop parks on transport_->read; write loop parks
    // on the WtStreamManager event baton).
    drain();
  }

  void TearDown() override {
    // Drive a clean shutdown via EOF so the read coroutine exits naturally
    // (instead of relying on cancellation propagation through the baton wait).
    if (session_ && handler_->sessionEndCount == 0) {
      transport_->addReadEvent(nullptr, /*eof=*/true);
      drain();
    }
    session_.reset();
    drain();
  }

  // Drain the EventBase for a bounded budget. We schedule a wakeup timer and
  // call loopOnce() in a loop until the timer fires; this lets every chained
  // baton-signal -> runInLoop -> coroutine-resume run to completion without
  // either hanging (plain loopOnce blocks if nothing is pending) or missing
  // callbacks (EVLOOP_NONBLOCK can race with not-yet-queued runInLoop).
  void drain(int budgetMs = 50) {
    bool fired = false;
    evb_.runAfterDelay([&] { fired = true; }, budgetMs);
    while (!fired) {
      evb_.loopOnce();
    }
  }

  void feedFromPeer(std::unique_ptr<folly::IOBuf> bytes, bool eof = false) {
    transport_->addReadEvent(std::move(bytes), eof);
    drain();
  }

  // Drain everything the session has written so far and feed it to the wire
  // parser. Subsequent calls only see newly-written bytes.
  void parseWrites() {
    auto writes = std::move(state_->writeEvents);
    for (auto& w : writes) {
      auto chunk = w.move();
      if (chunk) {
        wire_.feed(std::move(chunk));
      }
    }
  }

  static constexpr uint64_t kSelfMaxStreams = 10;
  // For a Server session, peer-initiated bidi stream IDs are 0x00, 0x04, ...
  static constexpr uint64_t kPeerBidiId = 0x00;

  folly::EventBase evb_;
  std::unique_ptr<TestCoroTransport::State> state_;
  TestCoroTransport* transport_{nullptr};
  std::unique_ptr<TestWtHandler> handler_;
  std::shared_ptr<QmuxSession> session_;
  QxTransportParams selfParams_;
  WireParser wire_;
};

} // namespace

//////// Tests ////////
//
// QmuxSession runs only the steady-state I/O loops; the QX_TRANSPORT_PARAMETERS
// handshake is QmuxConnector's job. Tests here construct the session with a
// pre-built WtConfig (as the connector would) and exercise post-handshake
// behavior.

// 1) Peer credit comes from the constructor's WtConfig — bidi streams open
//    immediately, no on-wire handshake required.
TEST_F(QmuxSessionTest, CreateBidiStream_UsesConstructorPeerCredit) {
  auto created = session_->createBidiStream();
  ASSERT_TRUE(created.hasValue());
  EXPECT_NE(created.value().writeHandle, nullptr);
  EXPECT_NE(created.value().readHandle, nullptr);
}

// 2) A STREAM frame on a new peer-initiated bidi ID surfaces to the handler.
TEST_F(QmuxSessionTest, OnNewPeerBidiStream_NotifiesHandler) {
  feedFromPeer(streamRecord(kPeerBidiId, "hello", /*fin=*/false));

  ASSERT_EQ(handler_->newBidiStreamIds.size(), 1);
  EXPECT_EQ(handler_->newBidiStreamIds[0], kPeerBidiId);
  EXPECT_TRUE(handler_->newUniStreamIds.empty());
}

// 3) onDatagram on the codec forwards directly to the handler.
TEST_F(QmuxSessionTest, OnDatagram_ForwardsToHandler) {
  feedFromPeer(datagramRecord("dg-payload"));

  ASSERT_EQ(handler_->datagrams.size(), 1);
  EXPECT_EQ(handler_->datagrams[0], "dg-payload");
}

// 4) An inbound QX_PING is answered with a QX_PING response (same seq).
TEST_F(QmuxSessionTest, OnPing_RespondsWithPong) {
  feedFromPeer(pingRecord(/*seq=*/42));
  parseWrites();

  ASSERT_EQ(wire_.cb.pongs.size(), 1);
  EXPECT_EQ(wire_.cb.pongs[0].sequenceNumber, 42);
  EXPECT_TRUE(wire_.cb.pings.empty());
}

TEST_F(QmuxSessionTest, ManyPings_SplitsPongsIntoPeerSizedRecords) {
  constexpr uint64_t kPingCount = 4'000;

  feedFromPeer(pingRecords(kPingCount));
  parseWrites();

  EXPECT_TRUE(wire_.cb.connectionErrors.empty());
  ASSERT_EQ(wire_.cb.pongs.size(), static_cast<size_t>(kPingCount));
  for (uint64_t seq = 0; seq < kPingCount; ++seq) {
    EXPECT_EQ(wire_.cb.pongs[seq].sequenceNumber, seq);
  }
}

// 5) An inbound CONNECTION_CLOSE drives the session to onSessionEnd.
TEST_F(QmuxSessionTest, OnConnectionClose_FiresSessionEnd) {
  feedFromPeer(connectionCloseRecord(/*err=*/0x0, "bye"));
  drain();

  EXPECT_EQ(handler_->sessionEndCount, 1);
}

// 6) sendDatagram queues a payload that the writeLoop emits as a DATAGRAM
//    frame on the wire.
TEST_F(QmuxSessionTest, SendDatagram_AppearsOnWire) {
  auto res = session_->sendDatagram(folly::IOBuf::copyBuffer("ping-payload"));
  ASSERT_TRUE(res.hasValue());
  drain();
  parseWrites();

  ASSERT_EQ(wire_.cb.datagrams.size(), 1);
  auto& dg = wire_.cb.datagrams[0];
  ASSERT_NE(dg.httpDatagramPayload, nullptr);
  EXPECT_EQ(dg.httpDatagramPayload->cloneCoalescedAsValue()
                .moveToFbString()
                .toStdString(),
            "ping-payload");
}

// 7) Large stream writes are split into peer-sized QMUX records.
TEST_F(QmuxSessionTest, LargeStreamWrite_SplitsIntoPeerSizedRecords) {
  auto created = session_->createBidiStream();
  ASSERT_TRUE(created.hasValue());

  const auto streamId = created.value().writeHandle->getID();
  std::string payload(kDefaultMaxRecordSize * 2, 'x');
  auto writeResult = created.value().writeHandle->writeStreamData(
      folly::IOBuf::copyBuffer(payload), /*fin=*/false, nullptr);
  ASSERT_TRUE(writeResult.hasValue());
  drain();

  ASSERT_EQ(state_->writeEvents.size(), 1);
  parseWrites();

  EXPECT_TRUE(wire_.cb.connectionErrors.empty());
  ASSERT_GT(wire_.cb.streams.size(), 1);

  std::string observed;
  for (const auto& stream : wire_.cb.streams) {
    if (stream.streamId != streamId || !stream.streamData) {
      continue;
    }
    observed.append(stream.streamData->cloneCoalescedAsValue()
                        .moveToFbString()
                        .toStdString());
  }
  EXPECT_EQ(observed, payload);
}

// 8) Local closeSession fires onSessionEnd exactly once (both loops finish).
//    We also signal EOF as a belt-and-suspenders guarantee that the read
//    coroutine exits regardless of cancellation-propagation timing.
TEST_F(QmuxSessionTest, CloseSession_FiresSessionEndOnce) {
  auto res = session_->closeSession(/*error=*/folly::Optional<uint32_t>{42});
  ASSERT_TRUE(res.hasValue());
  transport_->addReadEvent(nullptr, /*eof=*/true);
  drain();

  EXPECT_EQ(handler_->sessionEndCount, 1);
}

// 9) Bytes the connector forwarded as initialIngress are drained by the
//    readLoop on its first iteration, before any new transport reads.
TEST_F(QmuxSessionTest, InitialIngress_IsDrainedOnStartup) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  auto* rawTransport = transport.get();

  WtStreamManager::WtConfig wtConfig{.selfMaxStreamsBidi = kSelfMaxStreams,
                                     .selfMaxStreamsUni = kSelfMaxStreams,
                                     .selfMaxConnData = 1 << 20,
                                     .selfMaxStreamDataBidi = 1 << 16,
                                     .selfMaxStreamDataUni = 1 << 16,
                                     .peerMaxStreamsBidi = 5,
                                     .peerMaxStreamsUni = 5,
                                     .peerMaxConnData = 1 << 20,
                                     .peerMaxStreamDataBidi = 1 << 16,
                                     .peerMaxStreamDataUni = 1 << 16};

  auto preroll = streamRecord(kPeerBidiId, "preroll", /*fin=*/false);

  auto session = std::make_shared<QmuxSession>(&evb_,
                                               WtDir::Server,
                                               selfParams_,
                                               std::move(transport),
                                               std::move(wtConfig),
                                               kDefaultMaxRecordSize,
                                               /*effectiveMaxIdleTimeoutMs=*/0,
                                               std::move(preroll),
                                               QmuxSession::Config{});
  auto handler = std::make_unique<TestWtHandler>();
  session->setHandler(handler.get());
  session->start(session);
  drain();

  EXPECT_EQ(handler->newBidiStreamIds, std::vector<uint64_t>{kPeerBidiId});

  // Clean shutdown.
  rawTransport->addReadEvent(nullptr, /*eof=*/true);
  drain();
}

TEST_F(QmuxSessionTest, IdleTimer_NotArmedWhenEffectiveTimeoutZero) {
  EXPECT_FALSE(session_->isIdleTimeoutScheduled());
}

TEST_F(QmuxSessionTest, IdleTimer_ArmedWhenEffectiveTimeoutNonzero) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  auto* rawTransport = transport.get();

  WtStreamManager::WtConfig wtConfig{.selfMaxStreamsBidi = kSelfMaxStreams,
                                     .selfMaxStreamsUni = kSelfMaxStreams,
                                     .selfMaxConnData = 1 << 20,
                                     .selfMaxStreamDataBidi = 1 << 16,
                                     .selfMaxStreamDataUni = 1 << 16,
                                     .peerMaxStreamsBidi = 5,
                                     .peerMaxStreamsUni = 5,
                                     .peerMaxConnData = 1 << 20,
                                     .peerMaxStreamDataBidi = 1 << 16,
                                     .peerMaxStreamDataUni = 1 << 16};

  auto session = std::make_shared<QmuxSession>(&evb_,
                                               WtDir::Server,
                                               selfParams_,
                                               std::move(transport),
                                               std::move(wtConfig),
                                               kDefaultMaxRecordSize,
                                               /*effectiveMaxIdleTimeoutMs=*/
                                               30'000,
                                               /*initialIngress=*/nullptr,
                                               QmuxSession::Config{});
  auto handler = std::make_unique<TestWtHandler>();
  session->setHandler(handler.get());
  session->start(session);
  drain();

  EXPECT_TRUE(session->isIdleTimeoutScheduled());

  // Clean shutdown.
  rawTransport->addReadEvent(nullptr, /*eof=*/true);
  drain();
}
