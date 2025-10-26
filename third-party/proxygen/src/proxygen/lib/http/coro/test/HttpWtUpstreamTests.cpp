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

TEST(WtStreamManager, BasicSelfBidi) {
  using WtStreamManager = detail::WtStreamManager;
  // TODO(@damlaj): WtMaxStreams ignored for now
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};

  WtStreamManager streamManager{detail::WtDir::Client, self, peer};
  // 0x00 is the next expected bidi stream id for client
  auto bidiRes = streamManager.getBidiHandle(0x00);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // 0x08 is not the next expected bidi stream id for client
  bidiRes = streamManager.getBidiHandle(0x08);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x04 is the next expected bidi stream id for client
  bidiRes = streamManager.getBidiHandle(0x04);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicSelfUni) {
  using WtStreamManager = detail::WtStreamManager;
  // TODO(@damlaj): WtMaxStreams ignored for now
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};

  WtStreamManager streamManager{detail::WtDir::Client, self, peer};
  // 0x02 is the next expected uni stream id for client
  auto bidiRes = streamManager.getBidiHandle(0x02);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // 0x0a is not the next expected uni stream id for client
  bidiRes = streamManager.getBidiHandle(0x0a);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x06 is the next expected uni stream id for client
  bidiRes = streamManager.getBidiHandle(0x06);
  EXPECT_EQ(bidiRes.readHandle, nullptr); // egress only
  EXPECT_NE(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicPeerBidi) {
  using WtStreamManager = detail::WtStreamManager;
  // TODO(@damlaj): WtMaxStreams ignored for now
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};

  WtStreamManager streamManager{detail::WtDir::Client, self, peer};
  // 0x01 is the next expected bidi stream for server
  auto bidiRes = streamManager.getBidiHandle(0x01);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);

  // 0x09 is not the next expected bidi stream for server
  bidiRes = streamManager.getBidiHandle(0x09);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x05 is the next expected bidi stream for server
  bidiRes = streamManager.getBidiHandle(0x05);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_NE(bidiRes.writeHandle, nullptr);
}

TEST(WtStreamManager, BasicPeerUni) {
  using WtStreamManager = detail::WtStreamManager;
  // TODO(@damlaj): WtMaxStreams ignored for now
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};

  WtStreamManager streamManager{detail::WtDir::Client, self, peer};
  // 0x03 is the next expected uni stream for server
  auto bidiRes = streamManager.getBidiHandle(0x03);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only

  // 0x0b is not the next expected uni stream for server
  bidiRes = streamManager.getBidiHandle(0x0b);
  EXPECT_EQ(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr);

  // 0x07 is the next expected bidi stream for server
  bidiRes = streamManager.getBidiHandle(0x07);
  EXPECT_NE(bidiRes.readHandle, nullptr);
  EXPECT_EQ(bidiRes.writeHandle, nullptr); // ingress only
}

TEST(WtStreamManager, NextBidiUniHandle) {
  using WtStreamManager = detail::WtStreamManager;
  // TODO(@damlaj): WtMaxStreams ignored for now
  WtStreamManager::WtMaxStreams self{.bidi = 1, .uni = 1};
  WtStreamManager::WtMaxStreams peer{.bidi = 1, .uni = 1};

  WtStreamManager streamManager{detail::WtDir::Client, self, peer};
  // next egress handle tests
  auto uni = CHECK_NOTNULL(streamManager.nextEgressHandle());
  EXPECT_EQ(uni->getID(), 0x02);
  uni = streamManager.nextEgressHandle();
  EXPECT_EQ(uni->getID(), 0x06);

  // next bidi handle test
  auto bidi = streamManager.nextBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
  EXPECT_EQ(bidi.readHandle->getID(), bidi.writeHandle->getID());
  EXPECT_EQ(bidi.readHandle->getID(), 0x00);

  bidi = streamManager.nextBidiHandle();
  EXPECT_NE(bidi.readHandle, nullptr);
  EXPECT_NE(bidi.writeHandle, nullptr);
  EXPECT_EQ(bidi.readHandle->getID(), bidi.writeHandle->getID());
  EXPECT_EQ(bidi.readHandle->getID(), 0x04);
}

} // namespace proxygen::coro::test
