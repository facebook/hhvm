/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/test/HTTPCoroSessionTests.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/coro/util/WtStreamManager.h>

using namespace proxygen;
using namespace testing;
using folly::coro::co_awaitTry;

namespace proxygen::coro::test {

constexpr auto kWtSettings = {SettingsId::ENABLE_CONNECT_PROTOCOL,
                              SettingsId::H2_WT_MAX_SESSIONS};

class HttpWtUpstreamSessionTest : public HTTPCoroSessionTest {
 public:
  HttpWtUpstreamSessionTest()
      : HTTPCoroSessionTest(TransportDirection::UPSTREAM),
        serverCodec_(peerCodec_.get()) {
  }

  void SetUp() override {
    HTTPCoroSessionTest::setUp();
    setWtSupport(true);
  }

 protected:
  // hacks to enable/disable wt
  std::array<HTTPSettings*, 2> getHttpSettings() {
    return {const_cast<HTTPSettings*>(codec_->getIngressSettings()),
            codec_->getEgressSettings()};
  }

  void setWtSupport(bool enabled) {
    for (auto httpSettings : getHttpSettings()) {
      for (const auto wtSetting : kWtSettings) {
        httpSettings->setSetting(wtSetting, int(enabled));
      }
    }
  }

  // TODO(@damlaj): derive from HTTPUpstreamSessionTest to reduce code
  // duplication
  HTTPMessage makeResponse(uint16_t statusCode) {
    HTTPMessage resp;
    resp.setHTTPVersion(1, 1);
    resp.setStatusCode(statusCode);
    return resp;
  }

  void deliverRespHeaders(HTTPCodec::StreamID id,
                          const HTTPMessage& resp,
                          bool eom = true) {
    serverCodec_->generateHeader(writeBuf_, id, resp, eom);
    // @lint-ignore CLANGTIDY
    transport_->addReadEvent(id, writeBuf_.move(), /*eof=*/false);
  }

  void deliverRstStream(HTTPCodec::StreamID id) {
    serverCodec_->generateRstStream(writeBuf_, id, ErrorCode::CANCEL);
    // @lint-ignore CLANGTIDY
    transport_->addReadEvent(id, writeBuf_.move(), /*eof=*/false);
  }

  HTTPCodec* serverCodec_{nullptr};
};

using H2WtUpstreamSessionTest = HttpWtUpstreamSessionTest;

CO_TEST_P_X(H2WtUpstreamSessionTest, Simple) {
  // valid wt req
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::CONNECT);
  msg.setUpgradeProtocol("webtransport");

  auto reservation = session_->reserveRequest();
  auto fut =
      co_withExecutor(&evb_, session_->sendWtReq(std::move(*reservation), msg))
          .start();

  MockLifecycleObserver obs;
  session_->addLifecycleObserver(&obs);
  folly::coro::Baton waitForHeaders;
  EXPECT_CALL(obs, onIngressMessage(_, _)).WillOnce([&]() {
    waitForHeaders.post();
    session_->removeLifecycleObserver(&obs);
  });

  // serialize non-final 1xx continue
  deliverRespHeaders(/*id=*/1, makeResponse(100), /*eom=*/false);
  co_await waitForHeaders;     // wait for session to parse 1xx resp headers
  co_await rescheduleN(2);     // for good measure
  EXPECT_FALSE(fut.isReady()); // fut only resolves once final headers are rx'd

  // serialize final 2xx
  deliverRespHeaders(/*id=*/1, makeResponse(200), /*eom=*/true);
  auto res = co_await co_awaitTry(std::move(fut));

  EXPECT_TRUE(res->resp->is2xxResponse());
  EXPECT_EQ(res->wt, nullptr);
}

CO_TEST_P_X(H2WtUpstreamSessionTest, WtUpgradeReqRstErr) {
  // receiving a rst_stream when waiting for 2xx should propagate the
  // ::readHeaderEvent error
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::CONNECT);
  msg.setUpgradeProtocol("webtransport");

  auto reservation = session_->reserveRequest();
  evb_.runAfterDelay([this]() { deliverRstStream(/*id=*/1); },
                     /*milliseconds=*/50);
  auto res =
      co_await co_awaitTry(session_->sendWtReq(std::move(*reservation), msg));
  auto* ex = res.tryGetExceptionObject<HTTPError>();
  EXPECT_TRUE(ex && ex->code == HTTPErrorCode::CANCEL);
}

CO_TEST_P_X(H2WtUpstreamSessionTest, SendInvalidWtReq) {
  HTTPMessage msg;
  msg.setMethod(HTTPMethod::GET);

  {
    // invalid reservation
    HTTPCoroSession::RequestReservation reservation;
    auto res =
        co_await co_awaitTry(session_->sendWtReq(std::move(reservation), msg));
    auto* ex = res.tryGetExceptionObject<HTTPError>();
    EXPECT_TRUE(ex && ex->code == HTTPErrorCode::INTERNAL_ERROR);
  }

  {
    // invalid msg
    auto reservation = session_->reserveRequest();
    auto res =
        co_await co_awaitTry(session_->sendWtReq(std::move(*reservation), msg));
    auto* ex = res.tryGetExceptionObject<HTTPError>();
    EXPECT_TRUE(ex && ex->code == HTTPErrorCode::INTERNAL_ERROR);
  }

  {
    setWtSupport(false);
    // unsupported webtransport (settings not set)
    msg.setMethod(HTTPMethod::CONNECT);
    msg.setUpgradeProtocol("webtransport");
    auto reservation = session_->reserveRequest();
    auto res =
        co_await co_awaitTry(session_->sendWtReq(std::move(*reservation), msg));
    auto* ex = res.tryGetExceptionObject<HTTPError>();
    EXPECT_TRUE(ex && ex->code == HTTPErrorCode::INTERNAL_ERROR);
  }
}

// only http/2 WebTransport tests for now
INSTANTIATE_TEST_SUITE_P(
    HttpWtUpstreamSessionTest,
    H2WtUpstreamSessionTest,
    Values(TestParams({.codecProtocol = CodecProtocol::HTTP_2})),
    paramsToTestName);

struct WtStreamManagerCb : detail::WtStreamManager::Callback {
  WtStreamManagerCb() = default;
  ~WtStreamManagerCb() override = default;
  void eventsAvailable() noexcept override {
    evAvail_ = true;
  }
  bool evAvail_{false};
};

TEST(WtStreamManager, BasicSelfBidi) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 2, .uni = 1};
  WtStreamManagerCb cb;

  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  // 0x00 is the next expected bidi stream id for client
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x00);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // 0x08 is not the next expected bidi stream id for client
  bidiRes = streamManager.getOrCreateBidiHandle(0x08);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x04 is the next expected bidi stream id for client
  bidiRes = streamManager.getOrCreateBidiHandle(0x04);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicSelfUni) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 2};
  WtStreamManagerCb cb;

  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  // 0x02 is the next expected uni stream id for client
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x02);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // 0x0a is not the next expected uni stream id for client
  bidiRes = streamManager.getOrCreateBidiHandle(0x0a);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x06 is the next expected uni stream id for client
  bidiRes = streamManager.getOrCreateBidiHandle(0x06);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_NE(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicPeerBidi) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 2, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;

  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  // 0x01 is the next expected bidi stream for server
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x01);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // 0x09 is not the next expected bidi stream for server
  bidiRes = streamManager.getOrCreateBidiHandle(0x09);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x05 is the next expected bidi stream for server
  bidiRes = streamManager.getOrCreateBidiHandle(0x05);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicPeerUni) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 2};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;

  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  // 0x03 is the next expected uni stream for server
  auto bidiRes = streamManager.getOrCreateBidiHandle(0x03);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only

  // 0x0b is not the next expected uni stream for server
  bidiRes = streamManager.getOrCreateBidiHandle(0x0b);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x07 is the next expected bidi stream for server
  bidiRes = streamManager.getOrCreateBidiHandle(0x07);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only
}

TEST(WtStreamManager, NextBidiUniHandle) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 2, .uni = 2};
  WtStreamManagerCb cb;

  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  // next egress handle tests
  auto uni = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_EQ(uni->getID(), 0x02);
  uni = streamManager.createEgressHandle();
  EXPECT_EQ(uni->getID(), 0x06);

  // next bidi handle test
  auto bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
  EXPECT_EQ(bidi.readHandle->getID(), bidi.writeHandle->getID());
  EXPECT_EQ(bidi.readHandle->getID(), 0x00);

  bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
  EXPECT_EQ(bidi.readHandle->getID(), bidi.writeHandle->getID());
  EXPECT_EQ(bidi.readHandle->getID(), 0x04);
}

TEST(WtStreamManager, StreamLimits) {
  using WtStreamManager = detail::WtStreamManager;
  using MaxStreamsBidi = WtStreamManager::MaxStreamsBidi;
  using MaxStreamsUni = WtStreamManager::MaxStreamsUni;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;

  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  // a single egress handle should succeed
  auto uni = streamManager.createEgressHandle();
  EXPECT_NE(uni, nullptr);

  // a single bidi handle should succeed
  auto bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);

  // next egress handle should fail
  uni = streamManager.createEgressHandle();
  EXPECT_EQ(uni, nullptr);

  // next bidi handle should fail
  bidi = streamManager.createBidiHandle();
  EXPECT_EQ(bidi.readHandle, nullptr);
  EXPECT_EQ(bidi.writeHandle, nullptr);

  // peer grants one additional credit for each of bidi and uni
  EXPECT_TRUE(streamManager.onMaxStreams(MaxStreamsBidi{2}));
  EXPECT_TRUE(streamManager.onMaxStreams(MaxStreamsUni{2}));

  // next egress handle should succeed
  uni = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_NE(uni, nullptr);

  // next bidi handle should succeed
  bidi = streamManager.createBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
}

TEST(WtStreamManager, EnqueueIngressData) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 2, .uni = 1};

  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  // next createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  auto two = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle && two.readHandle && two.writeHandle);

  constexpr auto kBufLen = 65'535;

  // both conn & stream recv window exactly full, expect success
  EXPECT_TRUE(
      streamManager.enqueue(*one.readHandle, {makeBuf(kBufLen), false}));
  // enqueuing a additional byte in one will fail (stream recv window full)
  EXPECT_FALSE(
      streamManager.enqueue(*one.readHandle, {makeBuf(kBufLen), false}));

  // enqueuing a single byte in two will fail (conn recv window full)
  EXPECT_FALSE(streamManager.enqueue(*two.readHandle, {makeBuf(1), false}));
  auto twoFut = two.readHandle->readStreamData();
  EXPECT_FALSE(twoFut.isReady()); // no data buffered, ::enqueue failed

  // reading stream data should succeed
  auto oneFut = one.readHandle->readStreamData();
  EXPECT_TRUE(oneFut.isReady()); // enqueue should fulfill promise
  EXPECT_EQ(oneFut.value().data->computeChainDataLength(), kBufLen);
  EXPECT_FALSE(oneFut.value().fin);
}

TEST(WtStreamManager, WriteEgressHandle) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 2, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  // next two ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  auto two = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle && two.readHandle && two.writeHandle);

  constexpr auto kBufLen = 65'535;
  // kBufLen will fill up both conn & stream egress windows
  auto res = one.writeHandle->writeStreamData(
      /*data=*/makeBuf(kBufLen), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() && res.value() == WebTransport::FCState::BLOCKED);
  // enqueue an additional byte into one
  res = one.writeHandle->writeStreamData(
      /*data=*/makeBuf(1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() && res.value() == WebTransport::FCState::BLOCKED);

  // each stream has an individual egress buffer of kBufLen before applying
  // backpressure => writing (kBufLen - 1) bytes into two should return
  // UNBLOCKED
  res = two.writeHandle->writeStreamData(
      makeBuf(kBufLen - 1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() &&
              res.value() == WebTransport::FCState::UNBLOCKED);

  // we should be able to dequeue kBufLen data from one.writeHandle
  EXPECT_EQ(streamManager.nextWritable(), one.writeHandle);
  auto dequeue = streamManager.dequeue(*one.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data->computeChainDataLength(), kBufLen);
  EXPECT_FALSE(dequeue.fin);
  // no connection flow control, nextWritable == nullptr
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // grant one a single byte of stream credit; dequeuing from one should yield
  // nothing since we're still blocked on conn flow control
  EXPECT_TRUE(streamManager.onMaxData(
      WtStreamManager::MaxStreamData{{kBufLen + 1}, one.writeHandle->getID()}));
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // grant one additional byte of conn credit; dequeue from one should yield
  // byte + eof
  EXPECT_TRUE(
      streamManager.onMaxData(WtStreamManager::MaxConnData{kBufLen + 1}));
  EXPECT_EQ(streamManager.nextWritable(), one.writeHandle);
  dequeue = streamManager.dequeue(*one.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data->computeChainDataLength(), 1);
  EXPECT_TRUE(dequeue.fin);

  // no next writable stream since blocked on conn egress flow control
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // grant enough conn fc credit to unblock two completely
  EXPECT_TRUE(
      streamManager.onMaxData(WtStreamManager::MaxConnData{kBufLen * 2}));
  EXPECT_EQ(streamManager.nextWritable(), two.writeHandle);
  dequeue = streamManager.dequeue(*two.writeHandle, /*atMost=*/kBufLen);
  EXPECT_EQ(dequeue.data->computeChainDataLength(), kBufLen - 1);
  EXPECT_TRUE(dequeue.fin);
}

TEST(WtStreamManager, BidiHandleCancellation) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  // next ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);

  auto res = one.writeHandle->writeStreamData(
      /*data=*/makeBuf(100), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(res.hasValue() &&
              res.value() == WebTransport::FCState::UNBLOCKED);

  // StreamManager::onStopSending should request cancellation of egress handle
  auto ct = one.writeHandle->getCancelToken();
  streamManager.onStopSending({one.writeHandle->getID(), 0x00});
  EXPECT_TRUE(ct.isCancellationRequested());
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // StreamManager::onResetStream should request cancellation of ingress handle
  auto fut = one.readHandle->readStreamData();
  ct = one.readHandle->getCancelToken();
  streamManager.onResetStream({one.writeHandle->getID(), 0x00});
  EXPECT_TRUE(ct.isCancellationRequested());
  EXPECT_TRUE(fut.isReady() && fut.hasException());
}

TEST(WtStreamManager, GrantFlowControlCredit) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 2, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  constexpr auto kBufLen = 65'535;

  // next ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);
  // fills up both conn- & stream-level flow control
  EXPECT_TRUE(streamManager.enqueue(*one.readHandle,
                                    {makeBuf(kBufLen), /*fin=*/false}));

  auto fut = one.readHandle->readStreamData();
  EXPECT_TRUE(fut.isReady() && fut.hasValue());

  EXPECT_TRUE(std::exchange(cb.evAvail_, false)); // callback should have
                                                  // triggered
  auto events = streamManager.moveEvents();
  CHECK_GE(events.size(), 2); // one conn & one stream fc
  using MaxConnData = WtStreamManager::MaxConnData;
  using MaxStreamData = WtStreamManager::MaxStreamData;
  auto maxStreamData = std::get<MaxStreamData>(events[0]);
  auto maxConnData = std::get<MaxConnData>(events[1]);
  EXPECT_EQ(maxStreamData.streamId, one.readHandle->getID());
  EXPECT_EQ(maxStreamData.maxData, kBufLen * 2);
  EXPECT_EQ(maxConnData.maxData, kBufLen * 2);

  // next ::createBidiHandle should succeed
  auto two = streamManager.createBidiHandle();
  CHECK(two.readHandle && two.writeHandle);
  // fills up both conn- & stream-level flow control
  fut = two.readHandle->readStreamData();
  EXPECT_TRUE(
      streamManager.enqueue(*two.readHandle, {makeBuf(kBufLen), /*fin=*/true}));
  // should have triggered only connection-level flow control since fin=true
  events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  EXPECT_TRUE(std::holds_alternative<MaxConnData>(events[0]));

  // validate that receiving a reset_stream releases connection-level flow
  // control
  auto* ingress = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x03));
  streamManager.enqueue(*ingress, {makeBuf(kBufLen), /*fin=*/false});
  streamManager.onResetStream(
      WtStreamManager::ResetStream{ingress->getID(), 0x00});
  events = streamManager.moveEvents();
  CHECK_EQ(events.size(), 1);
  maxConnData = std::get<MaxConnData>(events[0]);
  EXPECT_EQ(maxConnData.maxData,
            kBufLen * 4); // we've previously already issued 2x kBufLen
                          // increments, this is the 3rd
}

TEST(WtStreamManager, GrantConnFlowControlCreditAfterRead) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 2, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  constexpr auto kBufLen = 65'535;

  // enqueue a total of kBufLen bytes across handles one & two
  auto one = streamManager.createBidiHandle();
  auto two = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle && two.readHandle && two.writeHandle);

  EXPECT_TRUE(streamManager.enqueue(*one.readHandle,
                                    {makeBuf(kBufLen - 1), /*fin=*/true}));
  EXPECT_TRUE(
      streamManager.enqueue(*two.readHandle, {makeBuf(1), /*fin=*/true}));

  // we should release conn-fc credit only after app reading kBufLen / 2 bytes
  // => reading the single bytes from two should not release conn-fc
  auto read = two.readHandle->readStreamData();
  EXPECT_TRUE(read.isReady());
  EXPECT_FALSE(cb.evAvail_);

  // reading from one releases flow control (total bytes read > kBufLen / 2)
  read = one.readHandle->readStreamData();
  EXPECT_TRUE(read.isReady());

  EXPECT_TRUE(std::exchange(cb.evAvail_, false));
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  EXPECT_TRUE(std::holds_alternative<WtStreamManager::MaxConnData>(events[0]));
}

TEST(WtStreamManager, ResetStreamReleasesConnFlowControl) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 3};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  constexpr auto kBufLen = 65'535;
  constexpr auto kHalfBufLen = (kBufLen / 2) + 1;

  auto* one = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x03));
  auto* two = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x07));
  auto* three = CHECK_NOTNULL(streamManager.getOrCreateIngressHandle(0x0b));

  // queue kHalfBufLen across all handles; ensure the last reset releases
  // conn fc
  streamManager.enqueue(*one, {makeBuf(kHalfBufLen - 2), /*fin=*/false});
  streamManager.onResetStream({one->getID()});
  EXPECT_FALSE(cb.evAvail_);

  streamManager.enqueue(*two, {makeBuf(1), /*fin=*/false});
  streamManager.onResetStream({two->getID()});
  EXPECT_FALSE(cb.evAvail_);

  streamManager.enqueue(*three, {makeBuf(1), /*fin=*/false});
  streamManager.onResetStream({three->getID()});
  EXPECT_TRUE(cb.evAvail_);

  // validate MaxConnData value
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  auto maxConnData = std::get<WtStreamManager::MaxConnData>(events[0]);
  EXPECT_EQ(maxConnData.maxData, kBufLen + kHalfBufLen);
}

TEST(WtStreamManager, StopSendingResetStreamTest) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  constexpr auto kBufLen = 65'535;

  // next ::createBidiHandle should succeed
  auto one = streamManager.createBidiHandle();
  CHECK(one.readHandle && one.writeHandle);
  auto id = one.readHandle->getID();

  // stop sending should invoke callback and resolve pending promise
  auto rp = one.readHandle->readStreamData();
  one.readHandle->stopSending(0);
  EXPECT_TRUE(rp.isReady() && rp.hasException()); // exception via stopSending
  EXPECT_TRUE(std::exchange(cb.evAvail_, false));
  auto events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  auto stopSending = std::get<WtStreamManager::StopSending>(events[0]);
  EXPECT_EQ(stopSending.streamId, id);
  EXPECT_EQ(stopSending.err, 0);

  // fill up egress buffer to ensure ::resetStream resolves pending promise
  one.writeHandle->writeStreamData(
      makeBuf(kBufLen), /*fin=*/false, /*byteEventCallback=*/nullptr);
  auto wp = one.writeHandle->awaitWritable();

  // reset stream should invoke callback and resolve pending promise
  one.writeHandle->resetStream(/*error=*/1);
  EXPECT_TRUE(wp.hasValue() && wp->isReady() && wp->hasException());
  EXPECT_TRUE(std::exchange(cb.evAvail_, false));
  events = streamManager.moveEvents();
  EXPECT_EQ(events.size(), 1);
  auto resetStream = std::get<WtStreamManager::ResetStream>(events[0]);
  EXPECT_EQ(resetStream.streamId, id);
  EXPECT_EQ(resetStream.err, 1);

  // bidirectionally reset => stream deleted
  EXPECT_FALSE(streamManager.hasStreams());

  // ::resetStream on a unidirectional egress stream should erase the stream
  auto* egress = CHECK_NOTNULL(streamManager.createEgressHandle());
  EXPECT_TRUE(streamManager.hasStreams());
  egress->resetStream(/*error=*/0); // read side is closed for an egress-only
                                    // handle; bidirectionally complete after
                                    // ::resetStream
  EXPECT_FALSE(streamManager.hasStreams());
}

TEST(WtStreamManager, AwaitWritableTest) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  constexpr auto kBufLen = 65'535;
  // next ::createBidiHandle should succeed
  auto eh = CHECK_NOTNULL(streamManager.createEgressHandle());

  // await writable future should be synchronously ready & equal to kBufLen
  // (default egress stream fc)
  auto await = eh->awaitWritable();
  EXPECT_TRUE(await.hasValue() && await.value().isReady() &&
              await.value().value() == kBufLen);

  // send kBufLen + 1 bytes
  auto fcRes = eh->writeStreamData(
      makeBuf(kBufLen + 1), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(fcRes.hasValue() &&
              fcRes.value() == WebTransport::FCState::BLOCKED);

  // await writable future; not ready awaiting egress buffer space
  await = eh->awaitWritable();
  EXPECT_TRUE(await.hasValue() && !await.value().isReady());

  // granting additional fc credit does not unblock egress buffer
  streamManager.onMaxData({{kBufLen + 2}, eh->getID()});
  EXPECT_TRUE(await.hasValue() && !await.value().isReady());

  // dequeue will resolve promise
  streamManager.dequeue(*eh, kBufLen);
  EXPECT_TRUE(await->isReady() &&
              await->value() == kBufLen - 1); // minus one because we enqueued
                                              // kBufLen + 1 bytes of data
}

TEST(WtStreamManager, WritableStreams) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 3};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};
  constexpr auto kBufLen = 65'535;
  constexpr auto kAtMost = std::numeric_limits<uint64_t>::max();

  // next two ::createEgressHandle should succeed
  auto one = CHECK_NOTNULL(streamManager.createEgressHandle());
  auto two = CHECK_NOTNULL(streamManager.createEgressHandle());

  // 1 byte + eof; next writableStream == one
  auto writeRes = one->writeStreamData(
      makeBuf(1), /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(writeRes.hasValue() &&
              writeRes.value() == WebTransport::FCState::UNBLOCKED);
  EXPECT_TRUE(std::exchange(cb.evAvail_, false)); // nextWritable now
                                                  // not-nullptr
  EXPECT_EQ(streamManager.nextWritable(), one);

  // dequeue should yield the expected results
  auto dequeue = streamManager.dequeue(*one, kAtMost);
  EXPECT_TRUE(dequeue.data->length() == 1 && dequeue.fin);

  // no more writableStreams
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // write kBufLen data; which will exceed conn flow control by one byte)
  writeRes = two->writeStreamData(
      makeBuf(kBufLen), /*fin=*/false, /*byteEventCallback=*/nullptr);
  EXPECT_TRUE(writeRes.hasValue() &&
              writeRes.value() == WebTransport::FCState::BLOCKED);
  EXPECT_TRUE(std::exchange(cb.evAvail_, false)); // nextWritable now
                                                  // not-nullptr
  EXPECT_EQ(streamManager.nextWritable(), two);

  // dequeue will yield kBufLen - 1 (limited by conn flow control)
  dequeue = streamManager.dequeue(*two, kAtMost);
  EXPECT_EQ(dequeue.data->length(), kBufLen - 1);
  EXPECT_FALSE(dequeue.fin);
  // will return nullptr as no conn flow control available
  EXPECT_EQ(streamManager.nextWritable(), nullptr);

  // issue one additional byte of conn fc
  EXPECT_TRUE(
      streamManager.onMaxData(WtStreamManager::MaxConnData{kBufLen + 1}));

  // will return two as conn flow control is now available
  EXPECT_EQ(streamManager.nextWritable(), two);
  dequeue = streamManager.dequeue(*two, kAtMost);
  EXPECT_EQ(dequeue.data->length(), 1);
  EXPECT_FALSE(dequeue.fin);

  // ::nextWritable should return stream with only a pending fin even if
  // connection-level fc is blocked
  auto three = CHECK_NOTNULL(streamManager.createEgressHandle());
  three->writeStreamData(nullptr, /*fin=*/true, /*byteEventCallback=*/nullptr);
  EXPECT_EQ(streamManager.nextWritable(), three);
}

TEST(WtStreamManager, DrainWtSession) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  // drain session
  streamManager.onDrainSession({});

  // all self- and peer-initiated streams will fail
  auto one = streamManager.createBidiHandle();
  auto* two = streamManager.createEgressHandle();
  EXPECT_TRUE(one.readHandle == nullptr && one.writeHandle == nullptr);
  EXPECT_TRUE(two == nullptr);
}

TEST(WtStreamManager, CloseWtSession) {
  using WtStreamManager = detail::WtStreamManager;
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};
  WtStreamManagerCb cb;
  WtStreamManager streamManager{detail::WtDir::Client, self, peer, cb};

  // ensure cancellation source is cancelled when invoked ::onCloseSession
  auto one = streamManager.createBidiHandle();
  auto oneRead = one.readHandle->readStreamData();

  auto* two = streamManager.createEgressHandle();
  auto cts = {one.readHandle->getCancelToken(),
              one.writeHandle->getCancelToken(),
              two->getCancelToken()};

  streamManager.onCloseSession(WtStreamManager::CloseSession{0, ""});
  for (auto& ct : cts) {
    EXPECT_TRUE(ct.isCancellationRequested());
  }
  // read promise should have exception set
  EXPECT_TRUE(oneRead.isReady() && oneRead.hasException());

  // a locally initiated close should enqueue an event
  streamManager.shutdown(WtStreamManager::CloseSession{0, ""});
  auto events = streamManager.moveEvents();
  EXPECT_TRUE(
      std::holds_alternative<WtStreamManager::CloseSession>(events.back()));
}

} // namespace proxygen::coro::test
