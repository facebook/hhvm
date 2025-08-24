/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/client/HTTPClientConnectionCache.h"
#include "proxygen/lib/http/coro/client/HTTPCoroConnector.h"
#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"
#include "proxygen/lib/http/coro/test/HTTPTestSources.h"
#include "proxygen/lib/http/coro/util/test/TestHelpers.h"
#include <folly/logging/xlog.h>
#include <quic/client/QuicClientTransport.h>

#include <folly/SocketAddress.h>
#include <folly/coro/AsyncScope.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Timeout.h>
#include <folly/coro/ViaIfAsync.h>
#include <quic/api/test/Mocks.h>
#include <quic/state/test/Mocks.h>
#include <wangle/ssl/test/MockSSLStats.h>

using namespace testing;
using namespace proxygen;
using namespace proxygen::coro;
using namespace std::chrono;
using folly::coro::blockingWait;

namespace {
using proxygen::coro::test::TransportType;

HTTPClient::SecureTransportImpl transportImpl(TransportType transportType) {
  switch (transportType) {
    case TransportType::TCP:
      return HTTPClient::SecureTransportImpl::NONE;
    case TransportType::TLS:
      return HTTPClient::SecureTransportImpl::TLS;
    case TransportType::TLS_FIZZ:
      return HTTPClient::SecureTransportImpl::FIZZ;
    default:
      XLOG(FATAL) << "Don't call this for QUIC";
  }
}

using ConnParams = boost::variant<HTTPCoroConnector::ConnectionParams,
                                  HTTPCoroConnector::QuicConnectionParams>;

ConnParams getConnParams(TransportType transportType) {
  if (transportType == TransportType::QUIC) {
    return HTTPClient::getQuicConnParams();
  } else {
    return HTTPClient::getConnParams(transportImpl(transportType));
  }
}

std::shared_ptr<const HTTPCoroConnector::QuicConnectionParams>
getQuicConnParams(const ConnParams& connParams) {
  return std::make_shared<const HTTPCoroConnector::QuicConnectionParams>(
      boost::get<HTTPCoroConnector::QuicConnectionParams>(connParams));
}

folly::coro::Task<HTTPCoroSession*> HTTPCoroConnector_connect(
    folly::EventBase* evb,
    folly::SocketAddress addr,
    std::chrono::milliseconds timeout,
    ConnParams connParams,
    HTTPCoroConnector::SessionParams sessParams =
        HTTPClient::getSessionParams()) {
  if (connParams.which() == 1) {
    return HTTPCoroConnector::connect(
        evb,
        addr,
        timeout,
        boost::get<HTTPCoroConnector::QuicConnectionParams>(connParams),
        sessParams);
  } else {
    return HTTPCoroConnector::connect(
        evb,
        addr,
        timeout,
        boost::get<HTTPCoroConnector::ConnectionParams>(connParams),
        sessParams);
  }
}

std::string transportTypeToAlpn(TransportType ttype) {
  switch (ttype) {
    case TransportType::TLS:
    case TransportType::TLS_FIZZ:
      return "h2";
    case TransportType::TCP:
      return "";
    case TransportType::QUIC:
      return "h3";
  }
}

/**
 * Unfortunately this class is needed for timeout test. This class never
 * registers the (WRITE) socket connect event which makes it appear as if
 * server syn-ack never arrives and subsequently connection timeout will fire.
 */
class EventBaseBackend : public folly::EventBaseBackendBase {
 public:
  EventBaseBackend() {
    evb_ = event_base_new();
  }
  explicit EventBaseBackend(event_base* evb) : evb_(evb) {
    CHECK(evb);
  }
  ~EventBaseBackend() override {
    event_base_free(evb_);
  }

  event_base* getEventBase() override {
    return evb_;
  }

  int eb_event_base_loop(int flags) override {
    return event_base_loop(evb_, flags);
  }
  int eb_event_base_loopbreak() override {
    return event_base_loopbreak(evb_);
  }

  int eb_event_add(Event& event, const struct timeval* timeout) override {
    // never register connect event
    if (event.getEvent()->ev_events & (EV_WRITE)) {
      return 0;
    }
    return event_add(event.getEvent(), timeout);
  }
  int eb_event_del(EventBaseBackendBase::Event& event) override {
    return event_del(event.getEvent());
  }

  bool eb_event_active(Event& event, int res) override {
    event_active(event.getEvent(), res, 1);
    return true;
  }

 private:
  event_base* evb_;
};

} // namespace

namespace proxygen::coro::test {

CO_TEST_P_X(HTTPClientTests, ConnectorConnect) {
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  sessParams.settings.emplace_back(SettingsId::HEADER_TABLE_SIZE, 8192);
  sessParams.maxConcurrentOutgoingStreams = 10;
  sessParams.connFlowControl = 128000;
  sessParams.streamReadTimeout = std::chrono::seconds(5);
  sessParams.connReadTimeout = std::chrono::seconds(3);
  sessParams.writeTimeout = std::chrono::seconds(1);
  auto sess =
      co_await co_awaitTry(HTTPCoroConnector_connect(&evb_,
                                                     serverAddress_,
                                                     seconds(1),
                                                     getConnParams(GetParam()),
                                                     sessParams));
  EXPECT_FALSE(sess.hasException());
  const auto& tinfo = (*sess)->getSetupTransportInfo();
  if (tinfo.appProtocol) {
    EXPECT_EQ(*tinfo.appProtocol, transportTypeToAlpn(GetParam()));
  }
  (*sess)->dropConnection();
}

CO_TEST_P_X(HTTPClientTests, ConnectWithCustomTimeout) {
  HTTPCoroConnector::ConnectionParams connParams;
  auto sessParams = HTTPClient::getSessionParams(std::chrono::seconds(3));
  auto sess =
      co_await co_awaitTry(HTTPCoroConnector_connect(&evb_,
                                                     serverAddress_,
                                                     seconds(1),
                                                     getConnParams(GetParam()),
                                                     sessParams));
  EXPECT_FALSE(sess.hasException());
  const auto& tinfo = (*sess)->getSetupTransportInfo();
  if (tinfo.appProtocol) {
    EXPECT_EQ(*tinfo.appProtocol, transportTypeToAlpn(GetParam()));
  }
  (*sess)->dropConnection();
}

TEST_P(HTTPClientTests, ConnectCancel) {
  folly::CancellationSource source;
  co_withExecutor(
      &evb_,
      folly::coro::co_withCancellation(
          source.getToken(), ([this]() -> folly::coro::Task<void> {
            auto res = co_await co_awaitTry(HTTPCoroConnector_connect(
                &evb_, serverAddress_, seconds(1), getConnParams(GetParam())));
            EXPECT_TRUE(res.hasException());
            co_return;
          })()))
      .start();
  evb_.loopOnce();
  source.requestCancellation();
  evb_.loop();
}

TEST_P(HTTPClientTests, CancelledConnection) {
  HTTPCoroConnector::SessionParams sessParams;
  auto observer = std::make_shared<LifecycleObserver>();
  sessParams.lifecycleObserver = observer.get();
  observer.reset();
  folly::CancellationSource source;
  co_withExecutor(
      &evb_,
      folly::coro::co_withCancellation(
          source.getToken(),
          ([this](auto sessionParams) -> folly::coro::Task<void> {
            auto res = co_await co_awaitTry(
                HTTPCoroConnector_connect(&evb_,
                                          serverAddress_,
                                          seconds(1),
                                          getConnParams(GetParam()),
                                          sessionParams));
            EXPECT_TRUE(res.hasException());
            if (GetParam() == TransportType::QUIC) {
              std::string errMsg =
                  "quic::QuicInternalException: Connection cancelled";
              EXPECT_EQ(errMsg, res.exception().what().toStdString());
            }
            co_return;
          })(sessParams)))
      .start();
  evb_.loopOnce();
  source.requestCancellation();
  evb_.loop();
}

CO_TEST_P_X(HTTPClientTests, MigrateSessionEvb) {
  auto maybeSess = co_await co_awaitTry(
      HTTPCoroConnector_connect(&evb_,
                                serverAddress_,
                                seconds(1),
                                getConnParams(GetParam()),
                                HTTPCoroConnector::SessionParams{}));

  // round robin 100 requests on a single HTTPCoroSession between 10 evbs
  std::array<folly::ScopedEventBaseThread, 10> scopedEvbs{};

  // validate we've connected successfully
  CHECK(!maybeSess.hasException());
  auto* sess = maybeSess.value();

  // we require a few evb loops until we reach the detachable suspension points
  while (!sess->isDetachable()) {
    co_await folly::coro::co_reschedule_on_current_executor;
  }

  auto ka = sess->acquireKeepAlive();
  sess->detachEvb();
  folly::EventBase* evb{nullptr};

  for (uint8_t numRequests = 0; numRequests < 100; numRequests++) {
    evb = scopedEvbs[numRequests % scopedEvbs.size()].getEventBase();

    co_await co_withExecutor(
        evb, folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
          // attach evb must be invoked from current evb
          sess->attachEvb(evb);

          auto res = co_await co_awaitTry(
              sess->sendRequest(HTTPFixedSource::makeFixedRequest("/")));
          CHECK(!res.hasException());

          HTTPSourceReader reader(std::move(res).value());
          reader.onHeaders(
              [](std::unique_ptr<HTTPMessage> headers, bool, bool eom) {
                EXPECT_EQ(headers->getStatusCode(), 200);
                EXPECT_EQ(headers->getHeaders().getSingleOrEmpty("x-method"),
                          "GET");
                return HTTPSourceReader::Cancel;
              });
          auto readRes = co_await folly::coro::co_awaitTry(reader.read());
          EXPECT_FALSE(readRes.hasException());

          XCHECK(sess->isDetachable());
          sess->detachEvb();
        }));
  }

  sess->attachEvb(&evb_);
  struct DestroyLifecycleObs : public LifecycleObserver {
    DestroyLifecycleObs() = default;
    ~DestroyLifecycleObs() override = default;
    void onDestroy(const HTTPCoroSession&) override {
      waitUntilDestroy.post();
    }
    folly::coro::Baton waitUntilDestroy;
  } obs;

  ka.reset();
  sess->addLifecycleObserver(&obs);
  sess->closeWhenIdle();
  co_await obs.waitUntilDestroy;
}

using HTTPClientTLSOnlyTests = HTTPClientTests;
CO_TEST_P_X(HTTPClientTLSOnlyTests, sslSessionCallbacks) {
  HTTPCoroConnector::ConnectionParams connParams;
  class SslSessionManager : public HTTPCoroConnector::SslSessionManagerIf {
   public:
    void onNewSslSession(SslSessionPtr) noexcept override {
    }
    SslSessionPtr getSslSession() noexcept override {
      return (sslSessionRequested = true, nullptr);
    }
    bool sslSessionRequested{false};
  } sslSessionManager;

  connParams.sslContext = std::make_shared<folly::SSLContext>();
  connParams.sslSessionManager = &sslSessionManager;
  HTTPCoroConnector::SessionParams sessParams;
  sessParams.settings.emplace_back(SettingsId::HEADER_TABLE_SIZE, 8192);
  sessParams.maxConcurrentOutgoingStreams = 10;
  sessParams.connFlowControl = 128000;
  sessParams.streamReadTimeout = std::chrono::seconds(5);
  sessParams.connReadTimeout = std::chrono::seconds(3);
  sessParams.writeTimeout = std::chrono::seconds(1);
  auto sess = co_await co_awaitTry(HTTPCoroConnector_connect(
      &evb_, serverAddress_, seconds(1), connParams, sessParams));
  EXPECT_FALSE(sess.hasException());
  (*sess)->dropConnection();

  EXPECT_EQ(sslSessionManager.sslSessionRequested, true);
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectError) {
  auto sess = co_await co_awaitTry(
      HTTPCoroConnector_connect(&evb_,
                                folly::SocketAddress("169.254.1.1", 443),
                                milliseconds(1),
                                getConnParams(GetParam())));
  CHECK(sess.hasException());
  if (useQuic()) {
    auto ex = sess.tryGetExceptionObject<quic::QuicInternalException>();
    EXPECT_TRUE(ex->errorCode() == quic::LocalErrorCode::CONNECT_FAILED ||
                ex->errorCode() == quic::LocalErrorCode::CONNECTION_ABANDONED);
  } else {
    auto ex = sess.tryGetExceptionObject<AsyncSocketException>();
    EXPECT_TRUE(ex->getType() == AsyncSocketException::NOT_OPEN ||
                ex->getType() == AsyncSocketException::TIMED_OUT);
  }
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectTimeout) {
  /**
   * a bit hacky but:
   * - create UDP server socket that does not read anything (i.e. Quic will
   *   timeout)
   *
   * - create TCP server socket that doesn't accept any connections (i.e. don't
   *   process handshake for fizz/tls conns)
   *
   * - create evb backed by fake EventBaseBackend that drops connect write event
   */
  std::unique_ptr<folly::AsyncUDPServerSocket> serverUdpSocket{nullptr};
  folly::AsyncServerSocket::UniquePtr serverTcpSocket{nullptr};
  if (useQuic()) {
    evb_.runImmediatelyOrRunInEventBaseThread([&]() {
      serverUdpSocket = std::make_unique<folly::AsyncUDPServerSocket>(&evb_);
      serverUdpSocket->bind(folly::SocketAddress("127.0.0.1", 0));
    });
  } else {
    evb_.runImmediatelyOrRunInEventBaseThread([&]() {
      serverTcpSocket.reset(new folly::AsyncServerSocket(&evb_));
      serverTcpSocket->bind(folly::SocketAddress("127.0.0.1", 0));
      serverTcpSocket->listen(0);
      serverTcpSocket->setMaxAcceptAtOnce(0);
    });
  }

  // fake EventBaseBackend to drop connect event
  folly::ScopedEventBaseThread scopedEvb(
      folly::EventBase::Options().setBackendFactory(
          []() { return std::make_unique<EventBaseBackend>(); }),
      nullptr,
      "");
  auto& connectEvb = *scopedEvb.getEventBase();

  // attempt to connect to server; expect timeout
  folly::SocketAddress serverAddr;
  serverUdpSocket ? serverUdpSocket->getAddress(&serverAddr)
                  : serverTcpSocket->getAddress(&serverAddr);

  auto sess =
      co_withExecutor(&connectEvb,
                      HTTPCoroConnector_connect(&connectEvb,
                                                serverAddr,
                                                milliseconds(1),
                                                getConnParams(GetParam())))
          .start()
          .getTry();

  CHECK(sess.hasException());
  expectException(GetParam(), sess, ExceptionType::TIMED_OUT);

  co_return;
}

using HTTPClientTLSTests = HTTPClientTests;
CO_TEST_P_X(HTTPClientTLSTests, ConnectorConnectTLSError) {
  HTTPClient::setDefaultCAPaths({"/etc/pki/tls/cert.pem"});
  HTTPClient::setDefaultFizzCertVerifier(nullptr);
  auto sess = co_await co_awaitTry(HTTPCoroConnector_connect(
      &evb_, serverAddress_, seconds(1), getConnParams(GetParam())));
  EXPECT_TRUE(sess.hasException());
  expectException(GetParam(), sess, ExceptionType::SSL_ERROR);
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectHappyEyeballs) {
  if (GetParam() == TransportType::QUIC) {
    // Not supported yet
    co_return;
  }
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  // This addr will timeout
  folly::SocketAddress badAddr("169.254.0.1", serverAddress_.getPort());
  auto sess = co_await co_awaitTry(HTTPCoroConnector::happyEyeballsConnect(
      &evb_, badAddr, serverAddress_, seconds(1), connParams));
  EXPECT_FALSE(sess.hasException());
  (*sess)->dropConnection();
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectHappyEyeballsDoubleSuccess) {
  if (GetParam() == TransportType::QUIC) {
    // Not supported yet
    co_return;
  }
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  auto sessParams = HTTPCoroConnector::defaultSessionParams();
  // Set primary and secondary to the same address and a very short delay
  auto sess = co_await co_awaitTry(
      HTTPCoroConnector::happyEyeballsConnect(&evb_,
                                              serverAddress_,
                                              serverAddress_,
                                              seconds(1),
                                              connParams,
                                              sessParams,
                                              std::chrono::milliseconds(0)));
  EXPECT_FALSE(sess.hasException());
  (*sess)->dropConnection();
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectHappyEyeballsFail) {
  if (GetParam() == TransportType::QUIC) {
    // Not supported yet
    co_return;
  }
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  folly::SocketAddress timeoutAddr("169.254.0.1", serverAddress_.getPort());
  folly::SocketAddress failAddr("127.0.0.1", 0);
  auto sess = co_await co_awaitTry(HTTPCoroConnector::happyEyeballsConnect(
      &evb_, timeoutAddr, failAddr, seconds(1), connParams));
  EXPECT_TRUE(sess.hasException());
  auto asyncSockEx = sess.tryGetExceptionObject<folly::AsyncSocketException>();
  EXPECT_NE(asyncSockEx, nullptr);
  // returns second exception
  EXPECT_TRUE(asyncSockEx->getType() ==
                  folly::AsyncSocketException::TIMED_OUT ||
              asyncSockEx->getType() == folly::AsyncSocketException::NOT_OPEN ||
              asyncSockEx->getErrno() == EHOSTUNREACH);
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectHappyEyeballsCancelFallback) {
  if (GetParam() == TransportType::QUIC) {
    // Not supported yet
    co_return;
  }
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  class SslSessionManager : public HTTPCoroConnector::SslSessionManagerIf {
   public:
    void onNewSslSession(SslSessionPtr) noexcept override {
    }
    SslSessionPtr getSslSession() noexcept override {
      return (getSslSessionCount++, nullptr);
    }
    uint8_t getSslSessionCount{0};
  } sslSessionManager;

  connParams.sslSessionManager = &sslSessionManager;
  auto sess = co_await co_awaitTry(HTTPCoroConnector::happyEyeballsConnect(
      &evb_, serverAddress_, serverAddress_, seconds(1), connParams));
  EXPECT_FALSE(sess.hasException());
  (*sess)->dropConnection();
  co_await folly::coro::sleep(HTTPCoroConnector::kHappyEyeballsDelay * 2);
  // This only increments in the SSL test, it should attempt at most one conn
  EXPECT_LE(sslSessionManager.getSslSessionCount, 1);
}

CO_TEST_P_X(HTTPClientTests, ConnectorConnectHappyEyeballsFastFail) {
  if (GetParam() == TransportType::QUIC) {
    // Not supported yet
    co_return;
  }
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  folly::SocketAddress badAddr("127.0.0.1", 0);
  auto sess = co_await co_awaitTry(folly::coro::timeout(
      HTTPCoroConnector::happyEyeballsConnect(
          &evb_, badAddr, serverAddress_, seconds(1), connParams),
      std::chrono::milliseconds(HTTPCoroConnector::kHappyEyeballsDelay)));
  EXPECT_FALSE(sess.hasException());
  (*sess)->dropConnection();
}

CO_TEST_P_X(HTTPClientTests, Get) {
  auto resp = co_await co_awaitTry(HTTPClient::get(
      &evb_, getURL("/"), seconds(1), useQuic(), {{"custom", "header"}}));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  auto& headers = resp->headers->getHeaders();
  EXPECT_EQ(headers.getSingleOrEmpty("x-method"), "GET");
  EXPECT_EQ(headers.getSingleOrEmpty("x-host").rfind("127.0.0.1:", 0), 0);
  EXPECT_EQ(headers.getSingleOrEmpty("x-custom-header-count"), "1");
  EXPECT_EQ(resp->body.chainLength(), 0);
}

CO_TEST_P_X(HTTPClientTests, SetHostHeader) {
  auto resp = co_await co_awaitTry(HTTPClient::get(
      &evb_, getURL("/"), seconds(1), useQuic(), {{"host", "foo.bar"}}));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  auto& headers = resp->headers->getHeaders();
  EXPECT_EQ(headers.getSingleOrEmpty("x-method"), "GET");
  EXPECT_EQ(headers.getSingleOrEmpty("x-host"), "foo.bar");
  EXPECT_EQ(resp->body.chainLength(), 0);
}

CO_TEST_P_X(HTTPClientTests, GetWithSessionAndReservation) {
  std::unique_ptr<HTTPCoroSessionPool> pool;
  if (GetParam() == TransportType::QUIC) {
    auto qconnParams = getQuicConnParams(getConnParams(GetParam()));
    pool =
        std::make_unique<HTTPCoroSessionPool>(&evb_,
                                              serverAddress_.getAddressStr(),
                                              serverAddress_.getPort(),
                                              HTTPCoroSessionPool::PoolParams(),
                                              std::move(qconnParams));
  } else {
    auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
        getConnParams(GetParam()));
    pool =
        std::make_unique<HTTPCoroSessionPool>(&evb_,
                                              serverAddress_.getAddressStr(),
                                              serverAddress_.getPort(),
                                              HTTPCoroSessionPool::PoolParams(),
                                              connParams);
  }
  auto res = co_await co_awaitTry(pool->getSessionWithReservation());
  EXPECT_FALSE(res.hasException());
  auto resp = co_await co_awaitTry(HTTPClient::get(res->session,
                                                   std::move(res->reservation),
                                                   proxygen::URL(getURL("/")),
                                                   seconds(1),
                                                   {{"custom", "header"}}));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  auto& headers = resp->headers->getHeaders();
  EXPECT_EQ(headers.getSingleOrEmpty("x-method"), "GET");
  EXPECT_EQ(headers.getSingleOrEmpty("x-custom-header-count"), "1");
  EXPECT_EQ(resp->body.chainLength(), 0);
}

CO_TEST_P_X(HTTPClientTests, SessionPoolCancellationTest) {
  std::unique_ptr<HTTPCoroSessionPool> pool;
  if (GetParam() == TransportType::QUIC) {
    auto qconnParams = getQuicConnParams(getConnParams(GetParam()));
    pool =
        std::make_unique<HTTPCoroSessionPool>(&evb_,
                                              serverAddress_.getAddressStr(),
                                              serverAddress_.getPort(),
                                              HTTPCoroSessionPool::PoolParams(),
                                              std::move(qconnParams));
  } else {
    auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
        getConnParams(GetParam()));
    pool =
        std::make_unique<HTTPCoroSessionPool>(&evb_,
                                              serverAddress_.getAddressStr(),
                                              serverAddress_.getPort(),
                                              HTTPCoroSessionPool::PoolParams(),
                                              connParams);
  }
  auto resTask =
      co_withExecutor(&evb_, pool->getSessionWithReservation()).start();
  co_await folly::coro::co_reschedule_on_current_executor;
  pool.reset();
  auto res = co_await folly::coro::co_awaitTry(std::move(resTask));
  EXPECT_TRUE(res.hasException());
}

CO_TEST_P_X(HTTPClientTests, GetTrailers) {
  HTTPSourceReader reader;
  reader.onTrailers([](std::unique_ptr<HTTPHeaders> trailers) {
    EXPECT_EQ(trailers->getSingleOrEmpty("Test"), "Success");
  });
  auto resp = co_await co_awaitTry(HTTPClient::get(
      &evb_, getURL("/trailers"), seconds(1), std::move(reader), useQuic()));
  EXPECT_FALSE(resp.hasException());
}

CO_TEST_P_X(HTTPClientTests, Post) {
  auto resp = co_await co_awaitTry(HTTPClient::post(
      &evb_, getURL("/"), std::string(100, 'a'), seconds(1), useQuic()));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
}

CO_TEST_P_X(HTTPClientTests, PostEarlyReturn) {
  auto resp = co_await co_awaitTry(HTTPClient::post(&evb_,
                                                    getURL("/earlyreturn"),
                                                    std::string(1000000, 'a'),
                                                    seconds(1),
                                                    useQuic()));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
}

CO_TEST_P_X(HTTPClientTests, PostCancel) {
  folly::coro::CancellableAsyncScope scope;
  URL url(getURL("/echo"));
  auto sess = co_await co_awaitTry(HTTPClient::getHTTPSession(&evb_,
                                                              url.getHost(),
                                                              url.getPort(),
                                                              url.isSecure(),
                                                              useQuic(),
                                                              seconds(1),
                                                              seconds(1)));
  // Cancel POST request before entire body has been sent
  HTTPSourceReader reader;
  reader.onError([](HTTPSourceReader::ErrorContext, const HTTPError&) {});
  scope.add(co_withExecutor(&evb_,
                            HTTPClient::post(*sess,
                                             url,
                                             std::string(1000000, 'a'),
                                             std::move(reader),
                                             seconds(1))));
  co_await folly::coro::co_reschedule_on_current_executor;
  co_await scope.cancelAndJoinAsync();
}

CO_TEST_P_X(HTTPClientTests, Connect) {
  URL url(getURL("/"));
  auto timeout = std::chrono::milliseconds(3000);

  auto sess = co_await co_awaitTry(HTTPClient::getHTTPSession(&evb_,
                                                              url.getHost(),
                                                              url.getPort(),
                                                              url.isSecure(),
                                                              useQuic(),
                                                              timeout,
                                                              timeout));
  EXPECT_FALSE(sess.hasException());

  auto sessViaProxy = co_await co_awaitTry(
      HTTPClient::getHTTPSessionViaProxy(*sess,
                                         "example.com",
                                         443,
                                         true,
                                         transportImpl(TransportType::TCP),
                                         timeout,
                                         timeout));
  EXPECT_FALSE(sessViaProxy.hasException());

  (*sessViaProxy)->initiateDrain();
  (*sess)->initiateDrain();
}

CO_TEST_P_X(HTTPClientTests, PostSession) {
  URL url(getURL("/"));
  auto sess =
      co_await co_awaitTry(HTTPClient::getHTTPSession(&evb_,
                                                      url.getHost(),
                                                      url.getPort(),
                                                      url.isSecure(),
                                                      useQuic(),
                                                      std::chrono::seconds(1),
                                                      std::chrono::seconds(1)));
  EXPECT_FALSE(sess.hasException());
  size_t bodyExpected = 100;
  HTTPSourceReader reader;
  reader
      .onHeaders([](std::unique_ptr<HTTPMessage> headers, bool, bool eom) {
        EXPECT_EQ(headers->getStatusCode(), 200);
        EXPECT_EQ(headers->getHeaders().getSingleOrEmpty("x-method"), "POST");
        EXPECT_FALSE(eom);
        return HTTPSourceReader::Continue;
      })
      .onBody([&bodyExpected](BufQueue body, bool) {
        if (!body.empty()) {
          bodyExpected -= body.chainLength();
        }
        return HTTPSourceReader::Continue;
      });
  auto maybe = co_await co_awaitTry(HTTPClient::post(
      *sess, std::move(url), std::string(100, 'a'), std::move(reader)));
  EXPECT_FALSE(maybe.hasException());
  EXPECT_EQ(bodyExpected, 0);
  (*sess)->initiateDrain();
}

CO_TEST_P_X(HTTPClientTests, PostCustomReader) {
  size_t bytes = 0;
  HTTPSourceReader reader;
  reader
      .onBody([&bytes](BufQueue body, bool) {
        bytes += body.chainLength();
        return HTTPSourceReader::Continue;
      })
      .onError([](HTTPSourceReader::ErrorContext, HTTPError) {
        EXPECT_FALSE(true);
      });
  co_await co_awaitTry(HTTPClient::post(&evb_,
                                        getURL("/"),
                                        std::string(100, 'a'),
                                        seconds(1),
                                        std::move(reader),
                                        useQuic()));
  EXPECT_EQ(bytes, 100);
}

CO_TEST_P_X(HTTPClientTests, Request) {
  URL url(getURL("/"));
  auto sess =
      co_await co_awaitTry(HTTPClient::getHTTPSession(&evb_,
                                                      url.getHost(),
                                                      url.getPort(),
                                                      url.isSecure(),
                                                      useQuic(),
                                                      std::chrono::seconds(1),
                                                      std::chrono::seconds(1)));
  EXPECT_FALSE(sess.hasException());
  HTTPSourceReader reader;
  reader.onHeaders([](std::unique_ptr<HTTPMessage> headers, bool, bool eom) {
    EXPECT_EQ(headers->getStatusCode(), 200);
    EXPECT_EQ(headers->getHeaders().getSingleOrEmpty("x-method"), "OPTIONS");
    EXPECT_TRUE(eom);
    return HTTPSourceReader::Continue;
  });
  auto request = std::make_unique<HTTPMessage>();
  request->setMethod(HTTPMethod::OPTIONS);
  request->setURL(url.makeRelativeURL());
  request->getHeaders().set(HTTP_HEADER_HOST, url.getHostAndPort());
  auto maybe = co_await co_awaitTry(
      HTTPClient::request(*sess,
                          std::move(*(*sess)->reserveRequest()),
                          HTTPFixedSource::makeFixedSource(std::move(request)),
                          std::move(reader)));
  EXPECT_FALSE(maybe.hasException());
  (*sess)->initiateDrain();
}

CO_TEST_P_X(HTTPClientTests, getHTTPSessionAddressOverride) {
  URL url(getURL("/"));
  auto sess = co_await co_awaitTry(
      HTTPClient::getHTTPSession(&evb_,
                                 "shouldnotbeused.com",
                                 url.getPort(),
                                 url.isSecure(),
                                 useQuic(),
                                 std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 "",
                                 "",
                                 std::string("169.254.1.1")));
  EXPECT_TRUE(sess.hasException());
  sess = co_await co_awaitTry(
      HTTPClient::getHTTPSession(&evb_,
                                 "shouldnotbeused.com",
                                 url.getPort(),
                                 url.isSecure(),
                                 useQuic(),
                                 std::chrono::seconds(1),
                                 std::chrono::seconds(1),
                                 "",
                                 "",
                                 serverAddress_.getIPAddress().str()));
  EXPECT_FALSE(sess.hasException());
  (*sess)->initiateDrain();
}

CO_TEST_P_X(HTTPClientTests, RequestBuildHTTPSource) {
  URL url(getURL("/"));
  auto sess =
      co_await co_awaitTry(HTTPClient::getHTTPSession(&evb_,
                                                      url.getHost(),
                                                      url.getPort(),
                                                      url.isSecure(),
                                                      useQuic(),
                                                      std::chrono::seconds(1),
                                                      std::chrono::seconds(1)));
  EXPECT_FALSE(sess.hasException());
  HTTPSourceReader reader;
  reader.onHeaders([](std::unique_ptr<HTTPMessage> headers, bool, bool) {
    EXPECT_EQ(headers->getStatusCode(), 200);
    EXPECT_EQ(headers->getHeaders().getSingleOrEmpty("x-method"), "POST");
    return HTTPSourceReader::Continue;
  });
  reader.onBody([](quic::BufQueue body, bool eom) {
    EXPECT_EQ(eom, true);
    EXPECT_EQ(body.move()->to<std::string>(), "somebody");
    return HTTPSourceReader::Continue;
  });
  HTTPClient::RequestHeaderMap headers;
  headers["host"] = url.getHostAndPort();
  std::optional<std::string> body = "somebody";

  auto maybe = co_await co_awaitTry(
      HTTPClient::request(*sess,
                          std::move(*(*sess)->reserveRequest()),
                          HTTPMethod::POST,
                          url,
                          headers,
                          std::move(reader),
                          body));
  EXPECT_FALSE(maybe.hasException());
  (*sess)->initiateDrain();
}

CO_TEST_P_X(HTTPClientTests, GetBadURL) {
  auto resp = co_await co_awaitTry(HTTPClient::get(&evb_, "abcd", seconds(1)));
  EXPECT_TRUE(resp.hasException());
}

CO_TEST_P_X(HTTPClientTests, SourceReaderErrorHandling) {
  HTTPClient::Response resp;
  auto reader = HTTPClient::makeDefaultReader(resp);
  YieldExceptionSource exceptionSource(
      YieldExceptionSource::Stage::HeaderEvent,
      YieldExceptionSource::MessageType::Request);
  reader.setSource(&exceptionSource);
  auto response = co_await co_awaitTry(reader.read());
  EXPECT_TRUE(response.hasException());
  auto error = response.exception().get_exception<HTTPError>();
  EXPECT_NE(error, nullptr);
  EXPECT_EQ(error->code, HTTPErrorCode::INTERNAL_ERROR);
  // Check for appended message from HTTPClientmakeDefaultReader
  EXPECT_TRUE(error->describe().find("Error receiving response headers"));
}

CO_TEST_P_X(HTTPClientTests, GetError) {
  auto resp = co_await co_awaitTry(HTTPClient::get(
      &evb_, "https://169.254.1.1:443/", milliseconds(1), useQuic()));
  EXPECT_TRUE(resp.hasException());
}

CO_TEST_P_X(HTTPClientTests, GetNon200) {
  auto resp = co_await co_awaitTry(
      HTTPClient::get(&evb_, getURL("/error"), seconds(1), useQuic()));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 500);
}

CO_TEST_P_X(HTTPClientTests, GetErrorHeaders) {
  auto resp = co_await co_awaitTry(
      HTTPClient::get(&evb_, getURL("/abortHeaders"), seconds(1), useQuic()));
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 500);
}

CO_TEST_P_X(HTTPClientTests, PostErrorBody) {
  auto resp = co_await co_awaitTry(HTTPClient::post(&evb_,
                                                    getURL("/abortBody"),
                                                    std::string(100, 'a'),
                                                    seconds(1),
                                                    useQuic()));
  EXPECT_TRUE(resp.hasException());
}

CO_TEST_P_X(HTTPClientTests, QuicConnParamsTestMocks) {
  // ccFactory & observers only set for quic connections
  if (GetParam() == TransportType::QUIC) {
    // setup mocked cc factory and observer
    quic::MockObserver observer;
    EXPECT_CALL(observer, attached).Times(1);
    auto ccFactory =
        std::make_shared<quic::test::MockCongestionControllerFactory>();
    EXPECT_CALL(*ccFactory, makeCongestionController(_, _))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke([](quic::QuicConnectionStateBase& conn,
                                  quic::CongestionControlType ccType) {
          return quic::DefaultCongestionControllerFactory()
              .makeCongestionController(conn, ccType);
        }));

    // connect
    HTTPCoroConnector::SessionParams sessParams;
    auto connParams = HTTPClient::getQuicConnParams();
    connParams.ccFactory = std::move(ccFactory);
    connParams.onTransportCreated =
        [obs = &observer](quic::QuicClientTransport& client) {
          client.addObserver(obs);
        };

    auto sess = co_await HTTPCoroConnector_connect(
        &evb_, serverAddress_, seconds(10), std::move(connParams), sessParams);

    // make reservation and send request
    auto reservation = *(sess->reserveRequest());
    auto respSource =
        co_await co_withExecutor(
            &evb_,
            sess->sendRequest(HTTPFixedSource::makeFixedRequest("/"),
                              std::move(reservation)))
            .start();

    HTTPSourceReader reader;
    auto resp = co_await folly::coro::co_awaitTry(
        reader.setSource(std::move(respSource)).read());
    EXPECT_FALSE(resp.hasException());
  }
}

CO_TEST_P_X(HTTPClientTests, ClientBodyError) {
  auto params = HTTPCoroConnector::defaultSessionParams();
  // For H2, only allow one stream
  HTTPCoroConnector::ConnectionParams connParams;
  HTTPCoroConnector::SessionParams sessParams;
  sessParams.streamReadTimeout = seconds(10);
  sessParams.connReadTimeout = seconds(10);
  sessParams.writeTimeout = seconds(10);
  auto sess = co_await HTTPCoroConnector_connect(&evb_,
                                                 serverAddress_,
                                                 seconds(10),
                                                 getConnParams(GetParam()),
                                                 sessParams);

  auto reservation = *(sess->reserveRequest());
  auto respSource =
      co_await co_withExecutor(
          &evb_,
          sess->sendRequest(new ErrorSource("super super long text", true, 11),
                            std::move(reservation)))
          .start();

  HTTPSourceReader reader;
  auto resp = co_await folly::coro::co_awaitTry(
      reader.setSource(std::move(respSource)).read());

  CO_ASSERT_TRUE(resp.hasException<HTTPError>());
  EXPECT_NE(
      dynamic_cast<const HTTPError*>(resp.exception().get_exception())->code,
      HTTPErrorCode::READ_TIMEOUT);
  sess->initiateDrain();
  // Need to loop one more time to ensure writeLoop flushes the TCP reset
  co_await folly::coro::co_reschedule_on_current_executor;
}

CO_TEST_P_X(HTTPClientTests, ServerBodyError) {
  // use a custom reader to make sure that we get the original error
  HTTPSourceReader reader;
  auto resp = co_await co_awaitTry(HTTPClient::get(&evb_,
                                                   getURL("/bodyError_11"),
                                                   seconds(10),
                                                   std::move(reader),
                                                   useQuic()));
  CO_ASSERT_TRUE(resp.hasException<HTTPError>());
  EXPECT_NE(
      dynamic_cast<const HTTPError*>(resp.exception().get_exception())->code,
      HTTPErrorCode::READ_TIMEOUT);
}

class HTTPCoroSessionPoolTests : public HTTPClientTests {
 protected:
  class CoroSessionPool : public HTTPCoroSessionPool {
   public:
    // protected => public visibility
    using HTTPCoroSessionPool::HTTPCoroSessionPool;

    using HTTPCoroSessionPool::detachIdleSession;
    using HTTPCoroSessionPool::insertIdleSession;
    using HTTPCoroSessionPool::setIdleSessionObserver;
  };
  std::unique_ptr<CoroSessionPool> pool_;
  NiceMock<wangle::MockSSLStats> tlsStats_;

 public:
  void SetUp() override {
    HTTPClientTests::SetUp();
    pool_ = makePool();
  }

  using PoolEx = HTTPCoroSessionPool::Exception;
  using PoolParams = HTTPCoroSessionPool::PoolParams;
  using SessParams = HTTPCoroConnector::SessionParams;
  std::unique_ptr<CoroSessionPool> makePool(
      PoolParams poolParams = PoolParams{},
      SessParams sessParams = SessParams{}) {
    if (GetParam() == TransportType::QUIC) {
      auto qconnParams = boost::get<HTTPCoroConnector::QuicConnectionParams>(
          getConnParams(GetParam()));
      qconnParams.tlsStats = &tlsStats_;
      auto qconnParamsPtr = getQuicConnParams(qconnParams);
      return std::make_unique<CoroSessionPool>(&evb_,
                                               serverAddress_.getAddressStr(),
                                               serverAddress_.getPort(),
                                               poolParams,
                                               std::move(qconnParamsPtr),
                                               sessParams);
    } else {
      auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
          getConnParams(GetParam()));
      connParams.tlsStats = &tlsStats_;
      return std::make_unique<CoroSessionPool>(&evb_,
                                               serverAddress_.getAddressStr(),
                                               serverAddress_.getPort(),
                                               poolParams,
                                               connParams,
                                               sessParams);
    }
  }
};

folly::coro::Task<uint16_t> get(HTTPCoroSessionPool& pool,
                                bool expectError = true) {
  auto res = co_await co_awaitTry(pool.getSessionWithReservation());
  if (expectError) {
    EXPECT_FALSE(res.hasException());
  }
  // Slightly jank not to pass the real URL here.  HTTPClient::get uses it to
  // set the Host header
  auto respSource = co_await res->session->sendRequest(
      HTTPFixedSource::makeFixedRequest(URL("http://localhost/")),
      std::move(res->reservation));
  HTTPSourceReader reader(std::move(respSource));
  reader.onHeaders([](std::unique_ptr<HTTPMessage> resp, bool, bool) {
    EXPECT_EQ(resp->getStatusCode(), 200);
    return HTTPSourceReader::Continue;
  });
  auto maybe = co_await co_awaitTry(reader.read());
  EXPECT_FALSE(maybe.hasException());
  co_return res->session->getLocalAddress().getPort();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, Pool) {
  // Send two requests via the same pool, ensure they use the same port
  auto localPort1 = co_await get(*pool_);
  auto localPort2 = co_await get(*pool_);
  EXPECT_EQ(localPort1, localPort2);
  pool_.reset();
}

using HTTPCoroSessionPoolTLSTests = HTTPCoroSessionPoolTests;
CO_TEST_P_X(HTTPCoroSessionPoolTLSTests, PoolSessionReuse) {
  EXPECT_CALL(tlsStats_, recordSSLUpstreamConnection(true));
  auto localPort1 = co_await get(*pool_);
  pool_->flush();
  EXPECT_CALL(tlsStats_, recordSSLUpstreamConnection(false));
  auto localPort2 = co_await get(*pool_);
  EXPECT_NE(localPort1, localPort2);
  pool_.reset();
}

using HTTPCoroSessionPoolSSLTests = HTTPCoroSessionPoolTests;
CO_TEST_P_X(HTTPCoroSessionPoolSSLTests, PoolSessionReuseCustomStore) {
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  connParams.tlsStats = &tlsStats_;
  class SslSessionManager : public HTTPCoroConnector::SslSessionManagerIf {
   public:
    void onNewSslSession(SslSessionPtr session) noexcept override {
      sslSession = std::move(session);
    }
    SslSessionPtr getSslSession() noexcept override {
      return std::move(sslSession);
    }
    std::shared_ptr<folly::ssl::SSLSession> sslSession{nullptr};
  } sslSessionManager;

  connParams.sslSessionManager = &sslSessionManager;
  pool_->setConnParams(connParams);
  EXPECT_CALL(tlsStats_, recordSSLUpstreamConnection(true));
  auto localPort1 = co_await get(*pool_);
  EXPECT_TRUE(sslSessionManager.sslSession);
  pool_->flush();
  EXPECT_CALL(tlsStats_, recordSSLUpstreamConnection(false));
  auto localPort2 = co_await get(*pool_);
  EXPECT_NE(localPort1, localPort2);
  pool_.reset();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolLateBind) {
  // Send two requests via the same pool at the same time, with max conns=1,
  // ensure they use the same port
  // Note: H1 causes the second connection to be reused after the get() finishes
  // while H2 (tls) signals both gets() in parallel
  // TODO: verify this behavior
  pool_->setMaxConnections(1);
  auto [localPort1, localPort2] =
      co_await folly::coro::collectAll(get(*pool_), get(*pool_));
  EXPECT_EQ(localPort1, localPort2);
  pool_->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolLateBindTimeout) {
  // Request a session from a pool that is already full, but don't return
  // the session in time.
  pool_->setMaxConnections(1);
  auto params = HTTPCoroConnector::defaultSessionParams();
  // For H2, only allow one stream
  params.maxConcurrentOutgoingStreams = 1;
  pool_->setSessionParams(params);
  auto res = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_FALSE(res.hasException());
  auto respSource = co_await co_awaitTry(res->session->sendRequest(
      new TimeoutSource(std::make_unique<HTTPMessage>(getPostRequest(10))),
      std::move(res->reservation)));
  EXPECT_FALSE(respSource.hasException());
  // Getting the second session fails
  auto res2 = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_TRUE(res2.hasException());
  EXPECT_EQ(res2.tryGetExceptionObject<HTTPCoroSessionPool::Exception>()->type,
            HTTPCoroSessionPool::Exception::Type::Timeout);
  respSource->stopReading();
  pool_->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolHeadersTimeout) {
  // Request a session from a pool that is already full with a session that
  // hasn't sent it's request
  pool_->setMaxConnections(2);
  auto params = HTTPCoroConnector::defaultSessionParams();
  // For H2, only allow one stream
  params.maxConcurrentOutgoingStreams = 1;
  pool_->setSessionParams(params);
  auto res = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_FALSE(res.hasException());
  folly::CancellationSource cancellationSource;
  auto sendFut =
      co_withExecutor(
          &evb_,
          folly::coro::co_withCancellation(
              cancellationSource.getToken(),
              res->session->sendRequest(
                  new TimeoutSource(
                      std::make_unique<HTTPMessage>(getGetRequest()), true),
                  std::move(res->reservation))))
          .start();
  // Getting the second session succeeds, but with a different port
  auto res2 = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_FALSE(res2.hasException());
  EXPECT_NE(res->session->getLocalAddress().getPort(),
            res2->session->getLocalAddress().getPort());
  cancellationSource.requestCancellation();
  auto respSource = co_await folly::coro::co_awaitTry(std::move(sendFut));
  EXPECT_TRUE(respSource.hasException());
  pool_->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolLateBindConnectOnClose) {
  // Return a closing session to a pool that has a waiter, verify it gets a
  // new session
  pool_->setMaxConnections(1);
  auto params = HTTPCoroConnector::defaultSessionParams();
  // For H2, only allow one stream
  params.maxConcurrentOutgoingStreams = 1;
  pool_->setSessionParams(params);
  auto port1 = co_await get(*pool_);
  EXPECT_NE(port1, 0);
  auto res = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_FALSE(res.hasException());
  EXPECT_EQ(port1, res->session->getLocalAddress().getPort());
  auto respSource = co_await co_awaitTry(res->session->sendRequest(
      HTTPFixedSource::makeFixedRequest("/"), std::move(res->reservation)));
  EXPECT_FALSE(respSource.hasException());
  res->session->initiateDrain();
  auto fut = co_withExecutor(&evb_, pool_->getSessionWithReservation()).start();
  XLOG(DBG4) << "waiting for headers";
  auto resp = co_await co_awaitTry(respSource->readHeaderEvent());
  XLOG(DBG4) << "got headers";
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  EXPECT_TRUE(resp->eom);
  auto res2 = co_await std::move(fut);
  auto port2 = res2.session->getLocalAddress().getPort();
  EXPECT_NE(port1, port2);
  pool_->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, LateBindConnFailCancelWaiters) {
  HTTPCoroSessionPool deadPool(&evb_, "0.0.0.169", 0);
  // Send two requests via the same bad pool.  When the first one runs out
  // of connection attempts, both are cancelled
  deadPool.setMaxConnections(1);
  auto [res1, res2] =
      co_await folly::coro::collectAllTry(deadPool.getSessionWithReservation(),
                                          deadPool.getSessionWithReservation());
  auto ex = res1.tryGetExceptionObject<HTTPCoroSessionPool::Exception>();
  EXPECT_EQ(ex->type, HTTPCoroSessionPool::Exception::Type::ConnectFailed);
  auto asyncSockEx =
      ex->connectException.get_exception<folly::AsyncSocketException>();
  EXPECT_EQ(asyncSockEx->getType(), folly::AsyncSocketException::NOT_OPEN);
  EXPECT_EQ(res2.tryGetExceptionObject<HTTPCoroSessionPool::Exception>()->type,
            HTTPCoroSessionPool::Exception::Type::ConnectFailed);
  auto res3 = co_await co_awaitTry(deadPool.getSessionWithReservation());
  // Another attemp will still fail, but not because it's been "cancelled"
  EXPECT_EQ(res3.tryGetExceptionObject<HTTPCoroSessionPool::Exception>()->type,
            HTTPCoroSessionPool::Exception::Type::ConnectFailed);
  deadPool.drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, DrainCancelWaiters) {
  pool_->setMaxConnections(1);
  // Start getting a session
  auto fut = co_withExecutor(&evb_, pool_->getSessionWithReservation()).start();
  // Context switch to ensure getSessionWithReservation() starts the new
  // connection
  co_await folly::coro::co_reschedule_on_current_executor;
  // Drain the pool
  pool_->drain();
  // Should return null immediately
  auto res = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_EQ(res.tryGetExceptionObject<HTTPCoroSessionPool::Exception>()->type,
            HTTPCoroSessionPool::Exception::Type::Draining);

  // The connection will come back, and get drained without ever being
  // made available to the caller
  res = co_await folly::coro::co_awaitTry(std::move(fut));
  EXPECT_EQ(res.tryGetExceptionObject<HTTPCoroSessionPool::Exception>()->type,
            HTTPCoroSessionPool::Exception::Type::Draining);
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, TwoWaitersOneConns) {
  // Issue two requests to a pool with one currently full session that supports
  // 1 txn. When it finishes it should signal both waiter.
  if (GetParam() == TransportType::TCP) {
    // TCP/H1 can't do this
    co_return;
  }
  PoolParams poolParams;
  poolParams.maxConnections = 1;
  poolParams.maxWaiters = 2;
  auto sessParams = HTTPCoroConnector::defaultSessionParams();
  // For H2, only allow one stream, at first
  sessParams.maxConcurrentOutgoingStreams = 1;
  auto pool = makePool(poolParams);
  pool->setSessionParams(sessParams);

  auto res1 = co_await co_awaitTry(pool->getSessionWithReservation());
  EXPECT_FALSE(res1.hasException());
  auto res2Fut =
      co_withExecutor(&evb_, pool->getSessionWithReservation()).start();
  auto res3Fut =
      co_withExecutor(&evb_, pool->getSessionWithReservation()).start();
  auto res4Fut =
      co_withExecutor(&evb_, pool->getSessionWithReservation()).start();
  // Make sure these get in the waiter queue
  co_await folly::coro::co_reschedule_on_current_executor;
  // Technically, this API maybe should invoke an info callback by itself?
  res1->session->setMaxConcurrentOutgoingStreams(2);
  // Finish the request, this will signal both waiters
  auto respSource = co_await res1->session->sendRequest(
      HTTPFixedSource::makeFixedRequest("/"), std::move(res1->reservation));
  auto resp = co_await co_awaitTry(respSource.readHeaderEvent());
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  EXPECT_TRUE(resp->eom);
  auto res2 = co_await folly::coro::co_awaitTry(std::move(res2Fut));
  EXPECT_EQ(res1->session, res2->session);
  auto get2Fut = co_withExecutor(&evb_,
                                 res2->session->sendRequest(
                                     HTTPFixedSource::makeFixedRequest("/"),
                                     std::move(res2->reservation)))
                     .start();
  auto res3 = co_await folly::coro::co_awaitTry(std::move(res3Fut));
  XCHECK(res3.hasValue());
  EXPECT_EQ(res1->session, res3->session);

  // maxWaiters=2 => res4Fut is cancelled
  EXPECT_TRUE(res4Fut.isReady() && res4Fut.hasException() &&
              res4Fut.result().exception().get_exception<PoolEx>()->type ==
                  PoolEx::Type::MaxWaiters);

  // meh abandon get2Fut
  pool->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, TwoWaitersTwoConns) {
  // Issue two requests to a pool with two currently full sessions that support
  // 1 txn each. When they finish they should each signal one waiter.
  pool_->setMaxConnections(2);
  auto params = HTTPCoroConnector::defaultSessionParams();
  // For H2, only allow one stream
  params.maxConcurrentOutgoingStreams = 1;
  pool_->setSessionParams(params);
  auto res1 = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_FALSE(res1.hasException());
  auto get1Fut = co_withExecutor(&evb_,
                                 res1->session->sendRequest(
                                     HTTPFixedSource::makeFixedRequest("/"),
                                     std::move(res1->reservation)))
                     .start();
  auto res2 = co_await co_awaitTry(pool_->getSessionWithReservation());
  EXPECT_FALSE(res2.hasException());
  auto get2Fut = co_withExecutor(&evb_,
                                 res2->session->sendRequest(
                                     HTTPFixedSource::makeFixedRequest("/"),
                                     std::move(res2->reservation)))
                     .start();
  auto res3Fut =
      co_withExecutor(&evb_, pool_->getSessionWithReservation()).start();
  auto res4Fut =
      co_withExecutor(&evb_, pool_->getSessionWithReservation()).start();
  // Make sure these get in the waiter queue
  co_await folly::coro::co_reschedule_on_current_executor;
  // Finish first request, will release only one session waiter
  auto respSource = co_await folly::coro::co_awaitTry(std::move(get1Fut));
  auto resp = co_await co_awaitTry(respSource->readHeaderEvent());
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  EXPECT_TRUE(resp->eom);
  auto res3 = co_await folly::coro::co_awaitTry(std::move(res3Fut));
  EXPECT_EQ(res1->session, res3->session);
  auto get3Fut = co_withExecutor(&evb_,
                                 res3->session->sendRequest(
                                     HTTPFixedSource::makeFixedRequest("/"),
                                     std::move(res3->reservation)))
                     .start();
  // Finish second request, releases second waiter
  respSource = co_await folly::coro::co_awaitTry(std::move(get2Fut));
  resp = co_await co_awaitTry(respSource->readHeaderEvent());
  EXPECT_FALSE(resp.hasException());
  EXPECT_EQ(resp->headers->getStatusCode(), 200);
  EXPECT_TRUE(resp->eom);
  auto res4 = co_await folly::coro::co_awaitTry(std::move(res4Fut));
  EXPECT_EQ(res2->session, res4->session);
  // meh abandon get3Fut
  pool_->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolMaxAge) {
  pool_->setMaxAge(std::chrono::seconds(1));
  // Send two requests via the same pool, after the first one ages out
  auto localPort1 = co_await get(*pool_);
  co_await folly::coro::sleep(std::chrono::milliseconds(1100));
  auto localPort2 = co_await get(*pool_);
  EXPECT_NE(localPort1, localPort2);
  pool_->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolConnect) {
  pool_.reset();
  std::shared_ptr<HTTPCoroSessionPool> serverPool = makePool();
  // Using the default conn params: TCP/H1
  HTTPCoroSessionPool connectPool(&evb_, "example.com", 443, serverPool);

  // Send two requests via the same pool, after the first one ages out
  auto res1 = co_await co_awaitTry(connectPool.getSessionWithReservation());
  EXPECT_FALSE(res1.hasException());
  auto localPort1 = res1->session->getLocalAddress().getPort();
  auto res2 = co_await co_awaitTry(connectPool.getSessionWithReservation());
  EXPECT_FALSE(res2.hasException());
  auto localPort2 = res2->session->getLocalAddress().getPort();
  // The connected session is H1, so full, expect a new top level session
  EXPECT_NE(res1->session, res2->session);

  if (GetParam() == TransportType::TCP) {
    // The underlying session is H1, so a new underlying session is created
    EXPECT_NE(localPort1, localPort2);
  } else {
    // The underlying session is multiplexed and re-used.
    EXPECT_EQ(localPort1, localPort2);
  }
  connectPool.drain();
  serverPool->drain();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, PoolingDisabled) {
  // maxConnections = 0 can be set via constructor or setter
  {
    // test constructor path
    auto pool = makePool(HTTPCoroSessionPool::PoolParams{.maxConnections = 0});
    auto localPort1 = co_await get(*pool);
    auto localPort2 = co_await get(*pool);
    EXPECT_NE(localPort1, localPort2);
  }

  {
    // test setter path
    auto pool = makePool();
    pool->setMaxConnections(0);
    auto localPort1 = co_await get(*pool);
    auto localPort2 = co_await get(*pool);
    EXPECT_NE(localPort1, localPort2);
  }

  {
    // test pooling disabled with existing maxConnections
    auto pool = makePool(HTTPCoroSessionPool::PoolParams{.maxConnections = 2},
                         SessParams{.maxConcurrentOutgoingStreams = 1});
    pool->setPoolingEnabled(false);

    {
      auto [res1, res2] = co_await folly::coro::collectAll(
          pool->getSessionWithReservation(), pool->getSessionWithReservation());
      // two connections => pool is full
      EXPECT_TRUE(pool->full());
    }
    // detaching both txns w/ pooling disabled => connections will drain
    EXPECT_TRUE(pool->empty());
  }
}

TEST_P(HTTPCoroSessionPoolTests, DrainViaDestructor) {
  pool_.reset();
  auto pool = makePool();
  co_withExecutor(&evb_, get(*pool, /*expectError=*/false))
      .start([](auto&& res) { EXPECT_TRUE(res.hasException()); });
  evb_.loopOnce();
  evb_.loopOnce();
  pool.reset();
  evb_.loop();
}

TEST_P(HTTPCoroSessionPoolTests, LastOutstandingConnectFailed) {
  // just test http/1.1 for simplicity
  if (GetParam() != TransportType::TCP) {
    return;
  }

  /**
   * Tests the following edgecase:
   *
   * If the last outstanding connection fails, we should not cancel waiters if a
   * session already exists. To acheive this we need a special EventBaseBackend
   * that fails AsyncSocket connect
   */
  struct FailConnectionEvb : public EventBaseBackend {
    int eb_event_add(Event& event, const struct timeval* timeout) override {
      bool fail = failWrite_ && (event.getEvent()->ev_events == EV_WRITE);
      return fail ? -1 : event_add(event.getEvent(), timeout);
    }
    bool failWrite_{false};
  };

  auto evbBackend = new FailConnectionEvb();
  folly::EventBase evb{
      folly::EventBase::Options().setBackendFactory([evbBackend]() {
        return std::unique_ptr<folly::EventBaseBackendBase>(evbBackend);
      }),
  };

  // configure pool
  auto connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  auto pool = std::make_unique<CoroSessionPool>(
      &evb,
      serverAddress_.getAddressStr(),
      serverAddress_.getPort(),
      PoolParams{.maxConnectionAttempts = 1},
      connParams,
      SessParams{.maxConcurrentOutgoingStreams = 1});

  pool->setMaxConnections(1);
  using PoolEx = HTTPCoroSessionPool::Exception;

  {
    // When the only single outstanding connection establishment fails with no
    // other sessions, it should cancel the waiting coroutine with ConnectFailed
    evbBackend->failWrite_ = true;
    auto res =
        blockingWait(co_awaitTry(pool->getSessionWithReservation()), &evb);
    auto* ex = res.tryGetExceptionObject<PoolEx>();
    EXPECT_TRUE(ex && ex->type == PoolEx::Type::ConnectFailed);
    evbBackend->failWrite_ = false;
  }

  {
    // connection establishment succeeds => one session in the pool
    auto res =
        blockingWait(co_awaitTry(pool->getSessionWithReservation()), &evb);
    EXPECT_FALSE(res.hasException());

    // pool is now full, second ::getSessionWithReservation will suspend without
    // attempting to establish a connection
    EXPECT_TRUE(pool->full());
    auto res2 = co_withExecutor(&evb, pool->getSessionWithReservation())
                    .startInlineUnsafe();
    EXPECT_FALSE(res2.isReady()); // suspended

    // allow room for one more connection => fail connection attempt => verify
    // above coroutine is still waiting
    pool->setMaxConnections(2);
    EXPECT_FALSE(pool->full());

    evbBackend->failWrite_ = true;
    auto res3 = co_withExecutor(&evb, pool->getSessionWithReservation())
                    .startInlineUnsafe();

    // few loops
    evb.loopOnce();
    evb.loopOnce();

    // validate both res2 & res3 are still suspended without exception
    EXPECT_TRUE(!res2.isReady() && !res3.isReady());

    // cancel res => res2.isReady()
    res->reservation.cancel();
    evb.loopOnce();
    EXPECT_TRUE(res2.isReady() && res2.hasValue());

    // cancel re2 => res3.isReady()
    std::move(res2).value().reservation.cancel();
    evb.loopOnce();
    EXPECT_TRUE(res3.isReady() && res3.hasValue());
  }

  pool.reset();
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, IdleSessionsTest) {
  /**
   * pool settings to simplify test:
   *   - a single reservation/request will cause the session to become full
   *   - at most two sessions
   */
  pool_->setSessionParams({.maxConcurrentOutgoingStreams = 1});
  pool_->setMaxConnections(2);

  class MockIdleSessionObserver
      : public HTTPCoroSessionPool::IdleSessionObserverIf {
   public:
    MOCK_METHOD(void,
                onIdleSessionsChanged,
                (const HTTPCoroSessionPool& pool),
                (noexcept));
  };
  EXPECT_FALSE(pool_->detachIdleSession()); // no idle sessions

  {
    /**
     * Two concurrent ::getSessionWithReservation will attempt to establish two
     * connections; both should be initially inserted in available list (i.e. no
     * idle sessions).
     */
    NiceMock<MockIdleSessionObserver> idleSessionObs;
    pool_->setIdleSessionObserver(&idleSessionObs);
    auto [res1, res2] = co_await folly::coro::collectAll(
        pool_->getSessionWithReservation(), pool_->getSessionWithReservation());
    EXPECT_TRUE(pool_->full());
    EXPECT_FALSE(pool_->hasAvailableSessions());
    EXPECT_FALSE(pool_->hasIdleSessions());

    // cancelling reservations will move backing session back into idle list
    EXPECT_CALL(idleSessionObs, onIdleSessionsChanged(_)).Times(2);
    res1.reservation.cancel();
    EXPECT_TRUE(pool_->hasAvailableSessions() && pool_->hasIdleSessions());
    res2.reservation.cancel();
    pool_->setIdleSessionObserver(nullptr);
  }

  {
    // loop for session to become detachable
    co_await folly::coro::co_reschedule_on_current_executor;
    // there should be two idle sessions; expect detach to return session
    auto idleSession = pool_->detachIdleSession();
    XCHECK(idleSession);
    EXPECT_EQ(idleSession->getEventBase(), nullptr);

    // reservation will synchronously return reservation since we still have one
    // idle session
    EXPECT_TRUE(pool_->hasIdleSessions() && !pool_->full());
    auto res1 = co_withExecutor(&evb_, pool_->getSessionWithReservation())
                    .startInlineUnsafe();
    EXPECT_TRUE(res1.isReady());

    // no available sessions => will open new connection
    EXPECT_FALSE(pool_->hasAvailableSessions());
    auto res2 = co_await co_awaitTry(pool_->getSessionWithReservation());
    XCHECK(!res2.hasException());

    // acquiring another res will indefinitely suspend since pool is full
    EXPECT_TRUE(pool_->full());
    auto res3 = co_withExecutor(&evb_, pool_->getSessionWithReservation())
                    .startInlineUnsafe();
    EXPECT_FALSE(res3.isReady());

    // inject idle session, will wake up awaiting ::getSessionWithReservation
    // coro above after one evb loop
    static_cast<HTTPCoroSession*>(idleSession.get())->attachEvb(&evb_);
    pool_->insertIdleSession(std::move(idleSession));
    co_await folly::coro::co_reschedule_on_current_executor;
    EXPECT_TRUE(res3.isReady());
  }
  {
    // verify IdleSessionObserver is invoked when an session is draining
    pool_->flush();
    NiceMock<MockIdleSessionObserver> idleSessionObs;
    pool_->setIdleSessionObserver(&idleSessionObs);
    HTTPCoroSession* session{nullptr};
    {
      auto res = co_await pool_->getSessionWithReservation();
      // destructing reservation will move to idle
      EXPECT_CALL(idleSessionObs, onIdleSessionsChanged(_)).Times(1);
      session = res.session;
    }

    EXPECT_CALL(idleSessionObs, onIdleSessionsChanged(_)).Times(1);
    // draining will invoke ::onIdleSessionsChanged
    session->initiateDrain();
  }
}

CO_TEST_P_X(HTTPCoroSessionPoolTests, FlushPool) {
  /**
   * pool settings to simplify test:
   *   - a single reservation/request will cause the session to become full
   *   - at most two sessions
   */
  pool_->setSessionParams({.maxConcurrentOutgoingStreams = 1});
  pool_->setMaxConnections(2);

  // helpers
  auto session = [this]() -> auto {
    return pool_->getSessionWithReservation();
  };
  using reschedule = folly::coro::co_reschedule_on_current_executor_t;

  // two ::getSessionWithReservations will fill pool
  auto [res1, res2] = co_await folly::coro::collectAll(session(), session());
  EXPECT_TRUE(pool_->full());

  // any additional ::getSessionWithReservation will be suspended
  auto res3 = co_withExecutor(&evb_, session()).start();
  auto res4 = co_withExecutor(&evb_, session()).start();
  co_await reschedule{};
  EXPECT_TRUE(!res3.isReady() && !res4.isReady());

  // flushing pool should cancel any waiters
  pool_->flush();
  co_await reschedule{};
  // both futs should be ready and contain exceptions
  EXPECT_TRUE(res3.isReady() && res4.isReady());
  EXPECT_TRUE(res3.hasException() && res4.hasException());
}

class HTTPClientConnectionCacheTests : public HTTPClientTests {
 public:
  void SetUp() override {
    HTTPClientTests::SetUp();
  }

  proxygen::coro::HTTPCoroSessionPool& getPool(
      const folly::SocketAddress& socket) {
    return connCache_.getPool(
        socket.getAddressStr(),
        socket.getAddressStr(),
        socket.getPort(),
        (HTTPClientConnectionCacheTests::GetParam() != TransportType::TCP));
  }

  bool poolExists(const folly::SocketAddress& socket) {
    return connCache_.poolExists(
        socket.getAddressStr(),
        socket.getPort(),
        (HTTPClientConnectionCacheTests::GetParam() != TransportType::TCP));
  }

 protected:
  // wrapper class to expose protected members fns as public
  class HTTPClientConnectionCacheWrapper : public HTTPClientConnectionCache {
   public:
    using HTTPClientConnectionCache::getPool;
    using HTTPClientConnectionCache::HTTPClientConnectionCache;
    using HTTPClientConnectionCache::poolExists;
  };

  HTTPClientConnectionCacheWrapper connCache_{evb_};
};

TEST_P(HTTPClientConnectionCacheTests, GetPool) {
  auto& pool1 = connCache_.getPool("foo.com", "1.2.3.4", 443, true);
  EXPECT_FALSE(pool1.full());
  auto& pool2 = connCache_.getPool("foo.com", "1.2.3.4", 80, false);
  EXPECT_EQ(connCache_.getNumPools(), 2);
  auto& pool3 = connCache_.getPool("foo.com", "1.2.3.4", 80, false);
  EXPECT_EQ(connCache_.getNumPools(), 2);
  EXPECT_EQ(&pool2, &pool3);

  for (size_t i = 0; i < 20; i++) {
    auto& pool = connCache_.getPool("foo.com", "1.2.3.4", 9443 + i, true);
    EXPECT_FALSE(pool.full());
    EXPECT_LE(connCache_.getNumPools(), 10);
  }
  EXPECT_EQ(connCache_.getNumPools(), 10);
}

TEST_P(HTTPClientConnectionCacheTests, GetPoolWithConnParams) {
  /**
   * ::getPool() should always prefer connParams passed in via param rather than
   * the connParams set via ::setConnParams(). Invoke ::setConnParams with a
   * dummy value because why not?
   */
  HTTPCoroConnector::ConnectionParams other{};
  other.fizzContextAndVerifier = HTTPCoroConnector::makeFizzClientContext(
      HTTPCoroConnector::defaultTLSParams());
  connCache_.setConnParams(other);

  auto& pool1 = connCache_.getPool(
      "foo.com", "1.2.3.4", 443, true, /*connParams=*/nullptr);
  auto& pool2 = connCache_.getPool(
      "foo.com", "1.2.3.4", 443, true, /*connParams=*/nullptr);

  // pools retrieved should be the same as (address, port, isSecure) tuples are
  // equivalent
  EXPECT_EQ(&pool1, &pool2);

  // passing in the additional ConnectionParams* argument should retrieve a
  // different pool
  HTTPCoroConnector::ConnectionParams connParams{};
  connParams.fizzContextAndVerifier = HTTPCoroConnector::makeFizzClientContext(
      HTTPCoroConnector::defaultTLSParams());
  auto& pool3 =
      connCache_.getPool("foo.com", "1.2.3.4", 443, true, &connParams);
  EXPECT_NE(&pool1, &pool3);

  // since we're hashing the addresses of the members of fizzContextAndVerifier,
  // creating a new one should yet again retrieve a different pool
  connParams.fizzContextAndVerifier = HTTPCoroConnector::makeFizzClientContext(
      HTTPCoroConnector::defaultTLSParams());
  auto& pool4 =
      connCache_.getPool("foo.com", "1.2.3.4", 443, true, &connParams);
  EXPECT_NE(&pool3, &pool4);

  // reusing the same connParams should retrieve the same pool
  auto& pool5 =
      connCache_.getPool("foo.com", "1.2.3.4", 443, true, &connParams);
  EXPECT_EQ(&pool4, &pool5);
}

TEST_P(HTTPClientConnectionCacheTests, PoolMaxSize) {
  const auto maxConnectionPools = 232;
  HTTPClientConnectionCache connCache{evb_, folly::none, maxConnectionPools};
  EXPECT_EQ(connCache_.getMaxNumPools(), kDefaultMaxConnectionPools);
  EXPECT_EQ(connCache.getMaxNumPools(), maxConnectionPools);
}

CO_TEST_P_X(HTTPClientConnectionCacheTests, GetSession) {
  auto res = co_await connCache_.getSessionWithReservation(
      getURL(""), std::chrono::seconds(1));
  EXPECT_EQ(res.session->getPeerAddress().getPort(), serverAddress_.getPort());
  res.reservation.cancel();
  res = co_await connCache_.getSessionWithReservation(
      serverAddress_.getAddressStr(),
      serverAddress_.getPort(),
      (GetParam() != TransportType::TCP),
      std::chrono::seconds(1));
  EXPECT_EQ(res.session->getPeerAddress().getPort(), serverAddress_.getPort());
  res.reservation.cancel();

  auto tryRes = co_await co_awaitTry(
      connCache_.getSessionWithReservation(getURL(""),
                                           80,
                                           false,
                                           std::chrono::milliseconds(10),
                                           std::string("169.254.1.1")));
  EXPECT_TRUE(tryRes.hasException());

  res = co_await connCache_.getSessionWithReservation(
      getURL("shouldnotbeused.com"),
      serverAddress_.getPort(),
      (GetParam() != TransportType::TCP),
      std::chrono::seconds(1),
      serverAddress_.getIPAddress().str());
  EXPECT_EQ(res.session->getPeerAddress().getAddressStr(),
            serverAddress_.getAddressStr());

  connCache_.drain();
}

TEST_P(HTTPClientConnectionCacheTests, EmptyPoolIsReaped) {
  auto& pool = getPool(serverAddress_);
  EXPECT_TRUE(pool.empty());
  EXPECT_EQ(connCache_.getNumPools(), 1);
  connCache_.reapAllEmptyPools();
  EXPECT_EQ(connCache_.getNumPools(), 0);
}

CO_TEST_P_X(HTTPClientConnectionCacheTests, OnlyEmptyPoolsAreReaped) {
  // Using port 0 so that the kernel can choose a random port for us
  std::unique_ptr<ScopedHTTPServer> serverA{constructServer("127.0.0.1", 0)},
      serverB{constructServer("127.0.0.1", 0)},
      serverC{constructServer("127.0.0.1", 0)};

  folly::SocketAddress socketA{*serverA->address()},
      socketB{*serverB->address()}, socketC{*serverC->address()};

  co_await connCache_.getSessionWithReservation(getURL(socketA, ""),
                                                std::chrono::seconds(1));
  EXPECT_EQ(connCache_.getNumPools(), 1);
  EXPECT_TRUE(getPool(socketB).empty());
  EXPECT_EQ(connCache_.getNumPools(), 2);

  // Adding this pool will cause the empty pool to be reaped resulting in
  // a total of 2 pools.
  co_await connCache_.getSessionWithReservation(getURL(socketC, ""),
                                                std::chrono::seconds(1));
  EXPECT_EQ(connCache_.getNumPools(), 2);
  EXPECT_TRUE(poolExists(socketA));
  EXPECT_FALSE(poolExists(socketB));
  EXPECT_TRUE(poolExists(socketC));
  connCache_.drain();
}

CO_TEST_P_X(HTTPClientConnectionCacheTests, TraverseAtMostMaxPoolsDuringReap) {
  std::vector<std::unique_ptr<ScopedHTTPServer>> servers;
  for (size_t i = 0; i < 6; ++i) {
    auto server = constructServer("127.0.0.1", 0);
    auto socket = *server->address();
    EXPECT_TRUE(getPool(socket).empty());
    EXPECT_EQ(connCache_.getNumPools(), i + 1);
    servers.push_back(std::move(server));
  }

  // When we add the 7th pool (below), we remove upto 5 pools from the
  // back as defined by kMaxPoolsToTraverseDuringReap. In this case,
  // we remove 5 pools from the back of the cache's LRU.
  co_await connCache_.getSessionWithReservation(getURL(""),
                                                std::chrono::seconds(1));
  EXPECT_EQ(connCache_.getNumPools(), 2);
  EXPECT_TRUE(poolExists(serverAddress_));
  // This is the empty pool that survived since it was most recently added.
  EXPECT_TRUE(poolExists(*servers.back()->address()));
  connCache_.drain();
}

// tests drain prior to establishing connection
CO_TEST_P_X(HTTPClientConnectionCacheTests, GetSessionDuringDrain) {
  connCache_.drain();
  auto res = co_await co_awaitTry(connCache_.getSessionWithReservation(
      getURL(""), std::chrono::seconds(1)));
  auto ex = res.tryGetExceptionObject<HTTPCoroSessionPool::Exception>();
  CHECK(ex);
  EXPECT_EQ(ex->type, HTTPCoroSessionPool::Exception::Type::Draining);
  EXPECT_EQ(std::string(ex->what()), "Pool is draining");
}

// tests cancellation prior to establishing connection
CO_TEST_P_X(HTTPClientConnectionCacheTests, GetSessionReqCancel) {
  folly::CancellationSource cancelSource;
  cancelSource.requestCancellation();

  auto res = co_await co_awaitTry(folly::coro::co_withCancellation(
      cancelSource.getToken(),
      connCache_.getSessionWithReservation(getURL(""),
                                           std::chrono::seconds(1))));

  auto ex = res.tryGetExceptionObject<HTTPError>();
  CHECK(ex);
  EXPECT_EQ(ex->code, HTTPErrorCode::CORO_CANCELLED);
  EXPECT_EQ(ex->msg, "Cancelled");
}

CO_TEST_P_X(HTTPClientConnectionCacheTests, PendingGetSessionDrain) {
  HTTPClientConnectionCache connCache{evb_};
  constexpr auto kTimeout = std::chrono::seconds(2);
  connCache.setPoolParams({.maxConnections = 1});
  connCache.setSessionParams({.maxConcurrentOutgoingStreams = 1,
                              .streamReadTimeout = kTimeout,
                              .connReadTimeout = kTimeout});

  // reserve the only session (only one due to maxConnection=1), which will
  // force the second ::getSessionWithReservation call to block waiting on the
  // session to become available (until reservation is consumed/released)
  auto sess =
      co_await connCache.getSessionWithReservation(getURL(""), kTimeout);

  // drain ConnectionCache after 100ms (to give time for second
  // ::getSessionWithReservation to execute)
  evb_.runAfterDelay(
      [&]() {
        // resume waiter
        sess.reservation.cancel();
        // drain
        connCache.drain();
      },
      100);
  // reserving the same session should yield an exception due to drain
  auto ex = co_await co_awaitTry(
      connCache.getSessionWithReservation(getURL(""), kTimeout));

  CHECK(ex.hasException());
  auto poolEx = ex.tryGetExceptionObject<HTTPCoroSessionPool::Exception>();
  CHECK(poolEx);
  EXPECT_EQ(poolEx->type, HTTPCoroSessionPool::Exception::Type::Draining);
  EXPECT_EQ(std::string(poolEx->what()), "Pool is draining");
}

// same test as above, but instead of draining while the second
// ::getSessionWithReservation call is awaiting a free session we cancel the
// request
CO_TEST_P_X(HTTPClientConnectionCacheTests, PendingGetSessionReqCancel) {
  HTTPClientConnectionCache connCache{evb_};
  constexpr auto kTimeout = std::chrono::seconds(2);
  connCache.setPoolParams({.maxConnections = 1});
  connCache.setSessionParams({.maxConcurrentOutgoingStreams = 1,
                              .streamReadTimeout = kTimeout,
                              .connReadTimeout = kTimeout});

  // reserve the only session (only one due to maxConnection=1), which will
  // force the second ::getSessionWithReservation call to block waiting on the
  // session to become available (until reservation is consumed/released)
  auto sess =
      co_await connCache.getSessionWithReservation(getURL(""), kTimeout);

  // request cancellation after 100 (to give time for
  // ::getSessionWithReservation to execute)
  folly::CancellationSource cancelSource;
  evb_.runAfterDelay(
      [&]() {
        // resume waiter
        sess.reservation.cancel();
        // request cancel
        cancelSource.requestCancellation();
      },
      100);

  auto ex = co_await co_awaitTry(co_withCancellation(
      cancelSource.getToken(),
      connCache.getSessionWithReservation(getURL(""), kTimeout)));
  CHECK(ex.hasException());
  auto poolEx = ex.tryGetExceptionObject<HTTPCoroSessionPool::Exception>();
  CHECK(poolEx);
  EXPECT_EQ(poolEx->type, HTTPCoroSessionPool::Exception::Type::Cancelled);
  EXPECT_EQ(std::string(poolEx->what()), "Cancelled");
}

using HTTPClientConnectionCacheTLSTests = HTTPClientConnectionCacheTests;
CO_TEST_P_X(HTTPClientConnectionCacheTLSTests, NonDefaultParams) {
  HTTPCoroSessionPool::PoolParams poolParams;
  poolParams.maxConnections = 1;
  auto connParams(boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam())));
  NiceMock<wangle::MockSSLStats> tlsStats_;
  connParams.tlsStats = &tlsStats_;
  HTTPCoroConnector::SessionParams sessParams;
  sessParams.maxConcurrentOutgoingStreams = 1;
  connCache_.setPoolParams(poolParams);
  connCache_.setConnParams(connParams);
  connCache_.setSessionParams(sessParams);
  EXPECT_CALL(tlsStats_, recordSSLUpstreamConnection(true));
  auto res = co_await connCache_.getSessionWithReservation(
      getURL(""), std::chrono::seconds(1));
  EXPECT_EQ(res.session->numTransactionsAvailable(), 0);
  auto& pool = connCache_.getPool(serverAddress_.getAddressStr(),
                                  serverAddress_.getAddressStr(),
                                  serverAddress_.getPort(),
                                  true);
  EXPECT_TRUE(pool.full());

  auto respSource = co_await res.session->sendRequest(
      HTTPFixedSource::makeFixedRequest("/"), std::move(res.reservation));
  co_await HTTPSourceReader{std::move(respSource)}.read();

  pool.drain();
}

CO_TEST_P_X(HTTPClientConnectionCacheTests, ProxyConnect) {
  HTTPClientConnectionCache::ProxyParams proxyParams;
  proxyParams.server = serverAddress_.getAddressStr();
  proxyParams.port = serverAddress_.getPort();
  proxyParams.useConnect = true;
  proxyParams.connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  HTTPClientConnectionCache connCache{evb_, std::move(proxyParams)};
  auto endpointConnParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(TransportType::TLS));
  // ConnCache conn params are a template, so need to include TLS context even
  // when used for plaintext connections
  connCache.setConnParams(endpointConnParams);
  // Ask for an HTTP URL to avoid attempting a TLS handshaked on the CONNECT
  // stream, but the endpoint expects port 443.
  auto res = co_await connCache.getSessionWithReservation(
      "http://example.com:443/", std::chrono::seconds(1));
  EXPECT_EQ(res.session->getPeerAddress().getPort(), 443);
  res.session->initiateDrain();
}

CO_TEST_P_X(HTTPClientConnectionCacheTests, ProxyGet) {
  HTTPClientConnectionCache::ProxyParams proxyParams;
  proxyParams.server = serverAddress_.getAddressStr();
  proxyParams.port = serverAddress_.getPort();
  proxyParams.useConnect = false;
  proxyParams.connParams = boost::get<HTTPCoroConnector::ConnectionParams>(
      getConnParams(GetParam()));
  HTTPClientConnectionCache connCache{evb_, std::move(proxyParams)};
  auto res = co_await connCache.getSessionWithReservation(
      "foo", 12345, false, std::chrono::seconds(1));
  EXPECT_EQ(res.session->getPeerAddress().getPort(), serverAddress_.getPort());
  res.session->initiateDrain();
}

INSTANTIATE_TEST_SUITE_P(HTTPClientTests,
                         HTTPClientTests,
                         Values(TransportType::TCP,
                                TransportType::TLS,
                                TransportType::TLS_FIZZ,
                                TransportType::QUIC),
                         transportTypeToTestName);

INSTANTIATE_TEST_SUITE_P(HTTPClientTLSTests,
                         HTTPClientTLSTests,
                         Values(TransportType::TLS,
                                TransportType::TLS_FIZZ,
                                TransportType::QUIC),
                         transportTypeToTestName);

INSTANTIATE_TEST_SUITE_P(HTTPClientTLSOnlyTests,
                         HTTPClientTLSOnlyTests,
                         Values(TransportType::TLS),
                         transportTypeToTestName);

INSTANTIATE_TEST_SUITE_P(HTTPCoroSessionPoolTests,
                         HTTPCoroSessionPoolTests,
                         Values(TransportType::TCP,
                                TransportType::TLS,
                                TransportType::TLS_FIZZ,
                                TransportType::QUIC),
                         transportTypeToTestName);

// TODO: TransportType::QUIC - doesn't currently report handshake/resume
INSTANTIATE_TEST_SUITE_P(HTTPCoroSessionPoolTLSTests,
                         HTTPCoroSessionPoolTLSTests,
                         Values(TransportType::TLS, TransportType::TLS_FIZZ),
                         transportTypeToTestName);

INSTANTIATE_TEST_SUITE_P(HTTPCoroSessionPoolSSLTests,
                         HTTPCoroSessionPoolSSLTests,
                         Values(TransportType::TLS),
                         transportTypeToTestName);

// No QUIC, for now
INSTANTIATE_TEST_SUITE_P(HTTPClientConnectionCacheTests,
                         HTTPClientConnectionCacheTests,
                         Values(TransportType::TCP,
                                TransportType::TLS,
                                TransportType::TLS_FIZZ),
                         transportTypeToTestName);

// TLS only
INSTANTIATE_TEST_SUITE_P(HTTPClientConnectionCacheTLSTests,
                         HTTPClientConnectionCacheTLSTests,
                         Values(TransportType::TLS, TransportType::TLS_FIZZ),
                         transportTypeToTestName);

} // namespace proxygen::coro::test
