/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/test/TestUtils.h>

#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/server/samples/fwdproxy/ConnectSource.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include "proxygen/lib/http/session/HTTPSessionStats.h"
#include <proxygen/lib/http/session/test/MockHTTPSessionStats.h>

using folly::coro::co_awaitTry;
using folly::coro::co_error;
using folly::coro::co_result;

namespace {

constexpr std::chrono::milliseconds kConnectTimeout{100};
constexpr std::string_view kSuspendEgressMs = "suspend_egress_ms";

auto parseQueryParamAsInt(const proxygen::HTTPMessage& message,
                          std::string_view queryParamKey) {
  const auto& queryParamVal = message.getQueryParam(std::string(queryParamKey));
  return folly::tryTo<uint64_t>(queryParamVal);
}

} // namespace

namespace proxygen::coro::test {

class ConnectHandler : public HTTPHandler {
 public:
  ConnectHandler(folly::SocketAddress serverAddress)
      : serverAddress_(std::move(serverAddress)) {
  }

  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override {
    XLOG(DBG6) << "ConnectHandler connecting to upstream";
    auto transport =
        co_await co_awaitTry(folly::coro::Transport::newConnectedSocket(
            evb, serverAddress_, std::chrono::milliseconds(50)));
    // loopback connect should never fail?
    XCHECK(!transport.hasException()) << "loopback connect failed!";
    auto connectSource = std::make_unique<ConnectSource>(
        std::make_unique<folly::coro::Transport>(std::move(transport).value()),
        std::move(requestSource));
    co_withExecutor(evb, connectSource->readRequestSendUpstream()).start();
    co_return connectSource.release();
  }

 private:
  folly::SocketAddress serverAddress_;
};

class Handler : public TestHandler {
 public:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override {
    auto headerEvent = co_await co_awaitTry(requestSource.readHeaderEvent());
    XLOG(DBG6) << " Handler::handleRequest; got headerEvent; ex="
               << int(headerEvent.hasException());
    if (headerEvent.hasException()) {
      co_yield co_error(headerEvent.exception());
    }

    auto method = headerEvent->headers->getMethod();
    XCHECK(method);
    if (method == HTTPMethod::CONNECT) {
      XLOG(DBG6) << "Handler::handleRequest HTTPMethod::CONNECT";
      if (headerEvent->eom) {
        XLOG(DBG4) << "eom in header event for CONNECT request";
        co_yield co_error(
            HTTPError{HTTPErrorCode::REFUSED_STREAM, "eom in connect request"});
      }
      auto connectRes = co_await co_awaitTry(
          ConnectHandler(serverAddress_)
              .handleRequest(evb, ctx, std::move(requestSource)));
      co_yield co_result(std::move(connectRes));
    }

    XLOG(DBG6) << "header event query string ="
               << headerEvent->headers->getQueryString();
    uint64_t suspendEgressMs =
        parseQueryParamAsInt(*headerEvent->headers, kSuspendEgressMs)
            .value_or(0);
    if (suspendEgressMs > 0) {
      XLOG(DBG6) << folly::to<std::string>(
          "suspending ingress for ", suspendEgressMs, "ms");
      co_await folly::coro::sleep(std::chrono::milliseconds(suspendEgressMs));
    }

    co_return HTTPFixedSource::makeFixedResponse(200);
  }

  folly::SocketAddress serverAddress_;
};

/**
 * Constructs a server; this server's handler is special in that it acts as a
 * fwdproxy in the case of CONNECT requests; it establishes a tunnel back to
 * itself, allowing us to test end-to-end client<->proxy<->server HTTP connect
 * tunnels (with HTTP being the tunneled protocol).
 *
 *
 * This effectively looks something like:
 *
 * +--------+   HTTP/TLS   +--------+   TCP    +--------+
 * | client | <----------> | proxy  | <------> | server |
 * +--------+              +--------+          +--------+
 *     ^                                           ^
 *     |        HTTP/TLS via proxy<->server        |
 *     |-------------------------------------------|
 */
class HTTPConnectIntegrationTest : public ::testing::Test {
 public:
  void SetUp() override {
    server_ = HTTPClientTests::constructServer(
        "127.0.0.1", 0, TransportType::TLS_FIZZ, handler_);
    handler_->serverAddress_ = *server_->address();
  }

  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

 protected:
  const folly::SocketAddress& getServAddr() {
    return handler_->serverAddress_;
  }

  HTTPCoroConnector::ConnectionParams getConnParams() {
    return HTTPClient::getConnParams(HTTPClient::SecureTransportImpl::TLS);
  }

  // ::getProxySess should never fail, sanity checks no exception is yielded
  folly::coro::Task<HTTPCoroSession*> getProxySess() {
    auto proxySess = co_await co_awaitTry(HTTPCoroConnector::connect(
        &evb_, getServAddr(), kConnectTimeout, getConnParams()));
    XCHECK(!proxySess.hasException())
        << "proxySess ex=" << proxySess.exception().what();
    co_return proxySess.value();
  }

  folly::coro::Task<HTTPCoroSession*> proxyConnect(
      HTTPCoroSession* proxySess,
      std::string authority,
      HTTPCoroConnector::SessionParams sessParams =
          HTTPCoroConnector::defaultSessionParams()) {
    auto res = co_await co_awaitTry(
        HTTPCoroConnector::proxyConnect(proxySess,
                                        *proxySess->reserveRequest(),
                                        /*authority=*/authority,
                                        /*connectUnique=*/false,
                                        /*timeout=*/kConnectTimeout,
                                        getConnParams(),
                                        sessParams));
    co_return res;
  }

  folly::EventBase evb_;
  std::shared_ptr<Handler> handler_{std::make_shared<Handler>()};
  std::unique_ptr<ScopedHTTPServer> server_;
};

CO_TEST_F_X(HTTPConnectIntegrationTest, Simple) {
  // connect to proxy
  auto proxySess = co_await getProxySess();

  // establish tunnel to server via proxy
  std::string authority = folly::to<std::string>(
      "https://localhost:", getServAddr().getPort(), "/");
  auto serverSess = co_await co_awaitTry(proxyConnect(proxySess, authority));
  XCHECK(!serverSess.hasException())
      << "serverSess ex=" << serverSess.exception().what();

  // simple GET request on the tunneled session should yield 200
  auto resp = co_await co_awaitTry(
      HTTPClient::get(/*session=*/serverSess.value(),
                      /*reservation=*/*serverSess.value()->reserveRequest(),
                      /*url=*/URL{authority}));
  XCHECK(!resp.hasException()) << "resp ex=" << resp.exception();
  XCHECK_EQ(resp->headers->getStatusCode(), 200);

  serverSess.value()->initiateDrain();
  proxySess->initiateDrain();
}

/**
 * AsyncSocketException::TIMED_OUT ex yielded from HTTPConnectTransport::read()
 * should initiate drain, and continue reading successfully
 */
CO_TEST_F_X(HTTPConnectIntegrationTest, TimeoutConnectTransportRead) {
  // connect to proxy
  auto proxySess = co_await getProxySess();

  // establish tunnel to server via proxy
  std::string authority = folly::to<std::string>(
      "https://localhost:", getServAddr().getPort(), "/");

  // set a 200ms read timeout on the connect transport
  HTTPCoroConnector::SessionParams sessParams{
      HTTPCoroConnector::defaultSessionParams()};
  sessParams.connReadTimeout = std::chrono::milliseconds(200);
  auto serverSess =
      co_await co_awaitTry(proxyConnect(proxySess, authority, sessParams));
  XCHECK(!serverSess.hasException())
      << "serverSess ex=" << serverSess.exception().what();

  // send a request with a suspend_egress_ms query param which is read by the
  // server to suspend writes for said amount, triggering the timeout
  auto url = folly::to<std::string>("/?", kSuspendEgressMs, "=2000");
  auto reqSource = HTTPFixedSource::makeFixedRequest(std::move(url));

  auto respSource =
      co_await co_awaitTry(serverSess.value()->sendRequest(reqSource));
  XCHECK(!respSource.hasException()) << respSource.exception().what();
  HTTPSourceReader reader{std::move(respSource).value()};

  auto readResult = co_await co_awaitTry(reader.read());
  XCHECK(!readResult.hasException()) << readResult.exception();

  serverSess.value()->initiateDrain();
  proxySess->initiateDrain();
}

CO_TEST_F_X(HTTPConnectIntegrationTest, CancelAfterTunnelTlsConnect) {
  /**
   * This test establishes a connection to the proxy and establishes a tls
   * connection via the HTTP CONNECT tunnel. The goal is to request cancellation
   * of token (via destructing the HTTPCoroSessionPool) immediately after the
   * TLS handshake via the CONNECT tunnel.
   */
  using PoolPtr = std::shared_ptr<HTTPCoroSessionPool>;

  // LoopCallback to destruct the HTTPCoroSessionPool
  class LoopCallback : public folly::EventBase::LoopCallback {
   public:
    LoopCallback(PoolPtr pool, PoolPtr proxyPool)
        : pool_(std::move(pool)), proxyPool_(std::move(proxyPool)) {
    }
    void runLoopCallback() noexcept override {
      proxyPool_.reset();
      pool_.reset();
      delete this;
    }
    PoolPtr pool_{nullptr}, proxyPool_{nullptr};
  };

  // heuristic to destruct the pools immediately after the TLS handshake via
  // CONNECT tunnel
  class WaitForBodyEvent : public proxygen::DummyHTTPSessionStats {
   public:
    ~WaitForBodyEvent() override = default;
    void recordPendingBufferedReadBytes(int64_t) noexcept override {
      if (++numEvents == 2) {
        baton.post();
        pool->getEventBase()->runBeforeLoop(
            new LoopCallback(std::move(pool), std::move(proxyPool)));
      }
    }
    folly::coro::Baton baton;
    std::shared_ptr<HTTPCoroSessionPool> proxyPool;
    std::shared_ptr<HTTPCoroSessionPool> pool;
    uint8_t numEvents{0};
  } waitForBodyEvent;

  const auto& servAddr = getServAddr();
  waitForBodyEvent.proxyPool = std::make_shared<HTTPCoroSessionPool>(
      &evb_,
      servAddr,
      HTTPCoroSessionPool::defaultPoolParams(),
      getConnParams());
  waitForBodyEvent.pool = std::make_unique<HTTPCoroSessionPool>(
      &evb_,
      servAddr.getAddressStr(),
      servAddr.getPort(),
      waitForBodyEvent.proxyPool,
      HTTPCoroSessionPool::defaultPoolParams(),
      getConnParams());

  auto proxySess =
      co_await waitForBodyEvent.proxyPool->getSessionWithReservation();
  proxySess.session->setSessionStats(&waitForBodyEvent);

  co_withExecutor(&evb_, waitForBodyEvent.pool->getSessionWithReservation())
      .start();
  co_await waitForBodyEvent.baton;

  proxySess.session->setSessionStats(nullptr);
}

} // namespace proxygen::coro::test
