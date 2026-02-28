/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPStreamSourceSink.h"
#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include "proxygen/lib/http/session/test/HTTPSessionMocks.h"

#include <folly/MoveWrapper.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Sleep.h>

using namespace proxygen::coro;
using namespace testing;
using namespace proxygen;

namespace proxygen::coro::test {

CO_TEST_P_X(HTTPClientTests, BasicUpstream) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  auto sess = co_await HTTPCoroConnector::connect(
      &evb_, serverAddress_, std::chrono::seconds(1), connParams, sessParams);

  MockHTTPHandler handler;
  HTTPStreamSourceUpstreamSink serverSink(
      &evb_, sess->acquireKeepAlive(), &handler);

  handler.expectHeaders();
  handler.expectEOM();
  handler.expectDetachTransaction();
  auto req = getGetRequest();
  serverSink.sendHeadersWithEOM(req);
  co_await serverSink.transact(sess, *sess->reserveRequest());
}

CO_TEST_P_X(HTTPClientTests, UpstreamPauseBeforeEOM) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  auto sess = co_await HTTPCoroConnector::connect(
      &evb_, serverAddress_, std::chrono::seconds(1), connParams, sessParams);

  MockHTTPHandler handler;
  HTTPStreamSourceUpstreamSink serverSink(
      &evb_, sess->acquireKeepAlive(), &handler);

  handler.expectHeaders([&serverSink] { serverSink.pauseIngress(); });
  auto req = getGetRequest();
  serverSink.sendHeadersWithEOM(req);
  co_withExecutor(
      co_await folly::coro::co_current_executor,
      folly::coro::co_invoke(
          [&serverSink, &handler]() -> folly::coro::Task<void> {
            co_await folly::coro::sleep(std::chrono::milliseconds(100));
            handler.expectEOM();
            handler.expectDetachTransaction();
            serverSink.resumeIngress();
          }))
      .start();
  co_await serverSink.transact(sess, *sess->reserveRequest());
}

CO_TEST_P_X(HTTPClientTests, FailSendUpstreamReq) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  auto sess = co_await HTTPCoroConnector::connect(
      &evb_, serverAddress_, std::chrono::seconds(1), connParams, sessParams);

  // get request before drain
  auto reservation = sess->reserveRequest();
  XCHECK(reservation.hasValue());
  // initiateDrain will cause next HTTPCoroSession::sendRequest to yield an
  // exception
  sess->initiateDrain();

  // enqueue headers to be sent to upstream
  auto* handler = new MockHTTPHandler();
  HTTPStreamSourceUpstreamSink serverSink(
      &evb_, sess->acquireKeepAlive(), handler);
  auto req = getGetRequest();
  serverSink.sendHeaders(req);

  EXPECT_CALL(*handler, _onError(_)).Times(1);
  EXPECT_CALL(*handler, _detachTransaction()).WillOnce([handler]() {
    delete handler;
  });

  co_await serverSink.transact(sess, std::move(*reservation));
}

CO_TEST_P_X(HTTPClientTests, PostWithPause) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  auto sess = co_await HTTPCoroConnector::connect(
      &evb_, serverAddress_, std::chrono::seconds(1), connParams, sessParams);

  MockHTTPHandler handler;
  HTTPStreamSourceUpstreamSink serverSink(
      &evb_, sess->acquireKeepAlive(), &handler);

  auto length = 65535;
  auto req = getPostRequest(length + 1);
  // will pause
  serverSink.sendHeaders(req);
  serverSink.sendPadding(2);
  handler.expectEgressPaused();
  serverSink.sendBody(makeBuf(length));
  handler.expectEgressResumed([&] {
    serverSink.sendPadding(10101);
    serverSink.sendBody(makeBuf(1));
    serverSink.sendPadding(12345);
    serverSink.sendEOM();
  });

  serverSink.pauseIngress();
  evb_.runAfterDelay(
      [&] {
        serverSink.resumeIngress();
        handler.expectHeaders();
        EXPECT_CALL(handler, _onBody(_));
        handler.expectEOM();
        handler.expectDetachTransaction();
      },
      50);
  co_await serverSink.transact(sess, *sess->reserveRequest());
}

CO_TEST_P_X(HTTPClientTests, SendAbortDuringPause) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  auto sess = co_await HTTPCoroConnector::connect(
      &evb_, serverAddress_, std::chrono::seconds(1), connParams, sessParams);

  MockHTTPHandler handler;
  HTTPStreamSourceUpstreamSink serverSink(
      &evb_, sess->acquireKeepAlive(), &handler);
  // send abort after 50ms
  evb_.runAfterDelay([&] { serverSink.sendAbort(); }, 50);

  handler.expectHeaders([&]() {
    // pause ingress after rx'ing headers
    serverSink.pauseIngress();
  });

  folly::coro::Baton waitForDetach;
  handler.expectDetachTransaction([&]() { waitForDetach.post(); });

  constexpr auto length = 65535;
  auto req = getPostRequest(length);
  serverSink.sendHeaders(req);
  serverSink.sendBody(makeBuf(length));
  co_await co_awaitTry(serverSink.transact(sess, *sess->reserveRequest()));
  co_await waitForDetach;
}

CO_TEST_P_X(HTTPClientTests, DetachAndAbortIfIncomplete) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  auto sess = co_await HTTPCoroConnector::connect(
      &evb_, serverAddress_, std::chrono::seconds(1), connParams, sessParams);

  StrictMock<MockHTTPHandler> handler;
  auto serverSink = std::make_unique<HTTPStreamSourceUpstreamSink>(
      &evb_, sess->acquireKeepAlive(), &handler);

  folly::coro::Baton waitUntilHeaders;
  handler.expectHeaders([&]() {
    // pause ingress after rx'ing headers
    serverSink->pauseIngress();

    // detachAndAbort; no more callbacks expected
    auto sinkPtr = serverSink.get();
    sinkPtr->detachAndAbortIfIncomplete(std::move(serverSink));

    waitUntilHeaders.post();
  });

  constexpr auto length = 65535;
  auto req = getPostRequest(length);
  serverSink->sendHeaders(req);

  EXPECT_CALL(handler, _onEgressPaused());
  serverSink->sendBody(makeBuf(length));

  EXPECT_CALL(handler, _onEgressResumed());
  co_await co_awaitTry(serverSink->transact(sess, *sess->reserveRequest()));

  co_await waitUntilHeaders;
}

INSTANTIATE_TEST_SUITE_P(HTTPClientTests,
                         HTTPClientTests,
                         Values(TransportType::TCP),
                         transportTypeToTestName);

} // namespace proxygen::coro::test
