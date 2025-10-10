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

} // namespace proxygen::coro::test
