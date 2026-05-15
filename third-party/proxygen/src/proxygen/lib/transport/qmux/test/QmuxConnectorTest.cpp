/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxConnector.h>

#include <folly/coro/Task.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/coro/transport/test/TestCoroTransport.h>
#include <proxygen/lib/transport/qmux/QmuxFramer.h>

using namespace proxygen;
using namespace proxygen::qmux;
using proxygen::coro::test::TestCoroTransport;

namespace {

QxTransportParams sampleSelfParams() {
  QxTransportParams p;
  p.initialMaxData = 1 << 20;
  p.initialMaxStreamsBidi = 10;
  p.initialMaxStreamsUni = 10;
  p.initialMaxStreamDataBidiLocal = 1 << 16;
  p.initialMaxStreamDataBidiRemote = 1 << 16;
  p.initialMaxStreamDataUni = 1 << 16;
  return p;
}

QxTransportParams samplePeerParams() {
  QxTransportParams p;
  p.initialMaxData = 1 << 21;
  p.initialMaxStreamsBidi = 7;
  p.initialMaxStreamsUni = 3;
  p.initialMaxStreamDataBidiLocal = 1 << 14;
  p.initialMaxStreamDataBidiRemote = 1 << 15;
  p.initialMaxStreamDataUni = 1 << 13;
  p.maxRecordSize = 32768;
  return p;
}

std::unique_ptr<folly::IOBuf> tpRecord(const QxTransportParams& params) {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writeTransportParams(frames, params);
  folly::IOBufQueue out{folly::IOBufQueue::cacheChainLength()};
  writeRecord(out, frames.move());
  return out.move();
}

std::unique_ptr<folly::IOBuf> connectionCloseRecord() {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writeConnectionClose(frames,
                       QxConnectionClose{.errorCode = 7,
                                         .frameType = 0,
                                         .isAppError = true,
                                         .reasonPhrase = "nope"});
  folly::IOBufQueue out{folly::IOBufQueue::cacheChainLength()};
  writeRecord(out, frames.move());
  return out.move();
}

// Builds a single record containing the peer's TPs followed by a QX_PING.
// Used to verify the connector hands off mid-record frames cleanly to the
// session's callback.
std::unique_ptr<folly::IOBuf> tpAndPingRecord(const QxTransportParams& params,
                                              uint64_t pingSeq) {
  folly::IOBufQueue frames{folly::IOBufQueue::cacheChainLength()};
  writeTransportParams(frames, params);
  writePing(frames, QxPing{.sequenceNumber = pingSeq});
  folly::IOBufQueue out{folly::IOBufQueue::cacheChainLength()};
  writeRecord(out, frames.move());
  return out.move();
}

// Free coroutine: arguments are coroutine-frame members, so the pointer
// out-params remain valid for the lifetime of the coroutine. Wrapping this
// in a capturing lambda would dangle the captures once the lambda temporary
// destructs at the call site.
folly::coro::Task<void> connectAndCapture(
    folly::EventBase* evb,
    WtDir dir,
    QxTransportParams selfParams,
    std::unique_ptr<folly::coro::TransportIf> transport,
    std::chrono::milliseconds timeout,
    folly::Try<QmuxSession::Ptr>* out,
    bool* done) {
  *out = co_await folly::coro::co_awaitTry(QmuxConnector::connect(
      evb, dir, std::move(selfParams), std::move(transport), timeout));
  *done = true;
}

class QmuxConnectorTest : public ::testing::Test {
 protected:
  void drain(int budgetMs = 50) {
    bool fired = false;
    evb_.runAfterDelay([&] { fired = true; }, budgetMs);
    while (!fired) {
      evb_.loopOnce();
    }
  }

  // Spawns the connector coroutine on evb_, drives the EventBase forward, and
  // returns the resulting Try once the coroutine completes.
  folly::Try<QmuxSession::Ptr> runConnector(
      std::unique_ptr<folly::coro::TransportIf> transport,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(500),
      QxTransportParams selfParams = sampleSelfParams()) {
    folly::Try<QmuxSession::Ptr> result;
    bool done = false;
    co_withExecutor(&evb_,
                    connectAndCapture(&evb_,
                                      WtDir::Client,
                                      std::move(selfParams),
                                      std::move(transport),
                                      timeout,
                                      &result,
                                      &done))
        .start();
    while (!done) {
      drain(20);
    }
    return result;
  }

  folly::EventBase evb_;
};

} // namespace

// 1) Happy path: peer sends back valid TPs; connector returns a session.
TEST_F(QmuxConnectorTest, Connect_Succeeds) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  transport->addReadEvent(tpRecord(samplePeerParams()), /*eof=*/false);

  auto result = runConnector(std::move(transport));
  ASSERT_FALSE(result.hasException()) << result.exception().what();
  ASSERT_NE(result.value(), nullptr);

  // The connector wrote our TPs to the wire before returning.
  EXPECT_FALSE(state->writeEvents.empty());
}

// 2) makeWtConfig pulls each side's advertised limits into the right slot.
TEST(MakeWtConfigTest, MapsParamsCorrectly) {
  auto self = sampleSelfParams();
  auto peer = samplePeerParams();
  auto cfg = makeWtConfig(self, peer);

  EXPECT_EQ(cfg.selfMaxStreamsBidi, self.initialMaxStreamsBidi);
  EXPECT_EQ(cfg.selfMaxConnData, self.initialMaxData);
  EXPECT_EQ(cfg.selfMaxStreamDataBidi, self.initialMaxStreamDataBidiLocal);
  EXPECT_EQ(cfg.selfMaxStreamDataUni, self.initialMaxStreamDataUni);
  EXPECT_EQ(cfg.peerMaxStreamsBidi, peer.initialMaxStreamsBidi);
  EXPECT_EQ(cfg.peerMaxConnData, peer.initialMaxData);
  EXPECT_EQ(cfg.peerMaxStreamDataBidi, peer.initialMaxStreamDataBidiRemote);
  EXPECT_EQ(cfg.peerMaxStreamDataUni, peer.initialMaxStreamDataUni);
}

// 3) Peer closes the transport before sending TPs => the connector errors.
TEST_F(QmuxConnectorTest, Connect_FailsOnEofBeforeTPs) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  transport->addReadEvent(/*ev=*/nullptr, /*eof=*/true);

  auto result = runConnector(std::move(transport));
  ASSERT_TRUE(result.hasException());
}

// 4) Peer sends a CONNECTION_CLOSE in place of TPs => the connector errors,
//    because the very first frame must be QX_TRANSPORT_PARAMETERS.
TEST_F(QmuxConnectorTest, Connect_FailsOnConnectionCloseBeforeTPs) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  transport->addReadEvent(connectionCloseRecord(), /*eof=*/false);

  auto result = runConnector(std::move(transport));
  ASSERT_TRUE(result.hasException());
}

// 5) Peer never sends TPs => the connector times out.
TEST_F(QmuxConnectorTest, Connect_FailsOnTimeout) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  // No reads queued; the connector will park on transport->read() and the
  // timeout should fire.

  auto result =
      runConnector(std::move(transport), std::chrono::milliseconds(20));
  ASSERT_TRUE(result.hasException());
}

// 6) Peer crams TPs + PING into the same record. The connector swaps the
//    codec's callback to the session's mid-record, so the session's
//    QmuxCallback receives the PING and responds with a PONG once started.
TEST_F(QmuxConnectorTest, Connect_HandsOffSameRecordFrames) {
  auto state = std::make_unique<TestCoroTransport::State>();
  auto transport = std::make_unique<TestCoroTransport>(&evb_, state.get());
  auto* rawTransport = transport.get();
  transport->addReadEvent(tpAndPingRecord(samplePeerParams(), /*pingSeq=*/77),
                          /*eof=*/false);

  auto result = runConnector(std::move(transport));
  ASSERT_FALSE(result.hasException()) << result.exception().what();
  auto session = result.value();
  ASSERT_NE(session, nullptr);

  // Snapshot writes-so-far (just our self TPs); start the session and let
  // its readLoop flush the PONG that QmuxCallback queued during handoff.
  const auto writesBeforeStart = state->writeEvents.size();
  session->start(session);
  // Wait for the readLoop's preroll-flush to fire.
  drain();

  EXPECT_GT(state->writeEvents.size(), writesBeforeStart)
      << "expected the session to flush the queued PONG on startup";

  // Clean shutdown.
  rawTransport->addReadEvent(nullptr, /*eof=*/true);
  drain();
}
