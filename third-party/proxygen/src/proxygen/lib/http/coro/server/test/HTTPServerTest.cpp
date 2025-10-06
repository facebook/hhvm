/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/coro/HTTPCoroSession.h>
#include <proxygen/lib/http/coro/HTTPFixedSource.h>
#include <proxygen/lib/http/coro/HTTPSourceReader.h>
#include <proxygen/lib/http/coro/client/HTTPClient.h>
#include <proxygen/lib/http/coro/filters/ServerFilterFactory.h>
#include <proxygen/lib/http/coro/filters/StatsFilterUtil.h>
#include <proxygen/lib/http/coro/filters/test/FakeServerStats.h>
#include <proxygen/lib/http/coro/server/HTTPServer.h>
#include <proxygen/lib/http/coro/server/ScopedHTTPServer.h>
#include <proxygen/lib/http/coro/test/HTTPTestSources.h>

#include <chrono>
#include <folly/coro/GtestHelpers.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/coro/test/TestUtils.h>
#include <quic/api/test/Mocks.h>
#include <quic/logging/test/Mocks.h>
#include <quic/state/test/MockQuicStats.h>
#include <quic/state/test/Mocks.h>

using namespace proxygen::coro;

namespace {

struct StatsFactory : public ServerFilterFactory {
  std::pair<HTTPSourceFilter*, HTTPSourceFilter*> makeFilters() override {
    return StatsFilterUtil::makeFilters(&stats_);
  }
  void onServerStart(folly::EventBase*) noexcept override {
  }
  void onServerStop() noexcept override {
  }
  proxygen::FakeHTTPServerStats stats_;
};

} // namespace

namespace proxygen::coro::test {

using namespace folly;
using namespace testing;

enum class TransportType { QUIC, TLS };
std::string transportTypeToString(testing::TestParamInfo<TransportType> type) {
  switch (type.param) {
    case TransportType::QUIC:
      return "quic";
    case TransportType::TLS:
      return "tls";
    default:
      return "undefined";
  }
}

class TestHandler : public HTTPHandler {
 protected:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override {
    HTTPSourceReader reader(std::move(requestSource));
    co_await reader.read();
    co_return HTTPFixedSource::makeFixedResponse(200, "OK");
  }
};

class MockServerObserver : public HTTPServer::Observer {
 public:
  MOCK_METHOD((void), onThreadStart, (folly::EventBase*));
  MOCK_METHOD((void), onThreadStop, (folly::EventBase*));
};

class HTTPServerTests : public TestWithParam<TransportType> {
 protected:
  void startServer(MockServerObserver* mockObserver = nullptr,
                   bool expectRequest = false) {
    serverConfig_.socketConfig.bindAddress.setFromIpPort(listenAddress_,
                                                         listenPort_);
    serverConfig_.socketConfig.sslContextConfigs.emplace_back(getTLSConfig());

    if (GetParam() == TransportType::QUIC) {
      serverConfig_.quicConfig = HTTPServer::QuicConfig();

      // create mock qlogger and adjust sampling rate based on whether we expect
      // a request
      auto mockQLogger =
          std::make_shared<quic::test::MockQLogger>(quic::VantagePoint::Server);
      serverConfig_.quicConfig->qloggerSampling.updateRate(int(expectRequest));
      EXPECT_CALL(*mockQLogger,
                  addPacket(Matcher<const quic::RegularQuicPacket&>(_), _))
          .Times(expectRequest ? AtLeast(1) : Exactly(0));
      serverConfig_.quicConfig->qlogger = std::move(mockQLogger);

      if (expectRequest) {
        // add observer to verify attach callback & onPacketReceived
        EXPECT_CALL(quicSocketObserver_, attached(_)).Times(1);
        // quicSocketObserver_
        EXPECT_CALL(quicSocketObserver_, packetsReceived(_, _))
            .Times(AtLeast(1));
        serverConfig_.quicConfig->observers.push_back(&quicSocketObserver_);

        // expect invocations on QuicStats CongestionControllerFactory mocks
        auto testFactory = std::make_unique<quic::MockQuicStatsFactory>();
        auto ccFactory =
            std::make_shared<quic::test::MockCongestionControllerFactory>();
        EXPECT_CALL(*ccFactory, makeCongestionController(_, _))
            .Times(AtLeast(1))
            .WillRepeatedly(Invoke([](quic::QuicConnectionStateBase& conn,
                                      quic::CongestionControlType ccType) {
              return quic::DefaultCongestionControllerFactory()
                  .makeCongestionController(conn, ccType);
            }));
        ON_CALL(*testFactory, make()).WillByDefault(testing::Invoke([]() {
          auto mockQuicStats = std::make_unique<quic::MockQuicStats>();
          EXPECT_CALL(*mockQuicStats, onPacketReceived()).Times(AtLeast(1));
          EXPECT_CALL(*mockQuicStats, onNewConnection()).Times(AtLeast(1));
          EXPECT_CALL(*mockQuicStats, onNewQuicStream()).Times(AtLeast(1));
          return mockQuicStats;
        }));
        serverConfig_.quicConfig->statsFactory = std::move(testFactory);
        serverConfig_.quicConfig->ccFactory = std::move(ccFactory);
      }
    }

    CHECK(handler_);
    server_ = ScopedHTTPServer::start(
        std::move(serverConfig_), handler_, mockObserver);
  }

  void stopServer() {
    server_.reset();
  }

  wangle::SSLContextConfig getTLSConfig() {
    auto tlsConfig = HTTPServer::HTTPServer::getDefaultTLSConfig();
    tlsConfig.isDefault = true;
    tlsConfig.clientVerification =
        folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
    tlsConfig.setNextProtocols({"h2", "http/1.1"});
    try {
      const std::string kTestDir = getContainingDirectory(XLOG_FILENAME).str();
      tlsConfig.setCertificate(kTestDir + "certs/test_cert1.pem",
                               kTestDir + "certs/test_key1.pem",
                               "");
    } catch (const std::exception& ex) {
      XLOG(ERR) << "Invalid certificate file or key file: %s" << ex.what();
    }
    return tlsConfig;
  }

  void initClient() {
    HTTPClient::setDefaultCAPaths({});
    HTTPClient::setDefaultFizzCertVerifier(
        std::make_shared<InsecureVerifierDangerousDoNotUseInProduction>());
  }

  std::string listenAddress_{"127.0.0.1"};
  uint16_t listenPort_{0};
  std::shared_ptr<HTTPHandler> handler_{std::make_shared<TestHandler>()};

  std::unique_ptr<ScopedHTTPServer> server_;
  HTTPServer::Config serverConfig_;
  quic::MockObserver quicSocketObserver_{
      quic::ManagedObserver::EventSetBuilder()
          .enable(quic::ManagedObserver::Events::packetsReceivedEvents)
          .build()};
};

TEST_P(HTTPServerTests, TestBasic) {
  // `expectRequest=true` will use the default sampling of one. We then send a
  // request to validate `.addPacket()` is invoked on mocked qlogger
  startServer(nullptr, /*expectRequest=*/true);
  initClient();

  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = folly::coro::blockingWait(
      HTTPClient::get(&evb, url, std::chrono::milliseconds(500), useQuic),
      &evb);
  EXPECT_NE(response.headers.get(), nullptr);
  EXPECT_EQ(response.headers->getStatusCode(), 200);
  stopServer();
}

TEST_P(HTTPServerTests, TestExistingSocket) {
  AsyncServerSocket::UniquePtr serverSocket(new folly::AsyncServerSocket());
  serverSocket->bind(0);
  serverConfig_.preboundSocket = serverSocket->getNetworkSocket().toFd();

  auto useQuic = GetParam() == TransportType::QUIC;
  // still call startServer for TransportType::QUIC, preboundSocket
  // should have no effect
  if (useQuic) {
    startServer(nullptr, /*expectRequest=*/true);
  } else {
    serverConfig_.socketConfig.sslContextConfigs.emplace_back(getTLSConfig());
    server_ =
        ScopedHTTPServer::start(std::move(serverConfig_), handler_, nullptr);
  }

  initClient();

  auto address = server_->address();
  CHECK(address.has_value());
  auto url = fmt::format("https://{}/test", address->describe());
  EventBase evb;
  auto response = folly::coro::blockingWait(
      HTTPClient::get(
          &evb, url, std::chrono::milliseconds(500), useQuic /* useQuic */),
      &evb);
  EXPECT_NE(response.headers.get(), nullptr);
  EXPECT_EQ(response.headers->getStatusCode(), 200);
  stopServer();
}

TEST_P(HTTPServerTests, TestStopMultipleTimes) {
  MockServerObserver mockObserver;
  serverConfig_.numIOThreads = 4;
  EXPECT_CALL(mockObserver, onThreadStart(_))
      .Times(folly::to<int>(serverConfig_.numIOThreads));
  EXPECT_CALL(mockObserver, onThreadStop(_))
      .Times(folly::to<int>(serverConfig_.numIOThreads));

  startServer(&mockObserver);
  server_->getServer().drain();
  server_->getServer().forceStop();
  // Draining or stopping an already stopped server should be safe
  stopServer();
}

TEST_P(HTTPServerTests, TestZeroThreadsMeansNumCPUs) {
  MockServerObserver mockObserver;
  serverConfig_.numIOThreads = 0;
  auto cpuCount = std::thread::hardware_concurrency();
  EXPECT_CALL(mockObserver, onThreadStart(_)).Times(folly::to<int>(cpuCount));
  EXPECT_CALL(mockObserver, onThreadStop(_)).Times(folly::to<int>(cpuCount));

  startServer(&mockObserver);
  EXPECT_EQ(server_->getServer().getConfig().numIOThreads, cpuCount);
  stopServer();
}

TEST_P(HTTPServerTests, TestSampling) {
  // `expectRequest=false` will set the sampling rate to zero. We then send a
  // request to validate `.addPacket()` is never invoked on mocked qlogger.
  startServer(nullptr, /*expectRequest=*/false);
  initClient();

  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = folly::coro::blockingWait(
      HTTPClient::get(&evb, url, std::chrono::milliseconds(500), useQuic),
      &evb);
  EXPECT_NE(response.headers.get(), nullptr);
  EXPECT_EQ(response.headers->getStatusCode(), 200);
  stopServer();
}

TEST_P(HTTPServerTests, TestFilterPass) {
  serverConfig_.newConnectionFilter =
      [](const folly::SocketAddress* /* address */,
         const folly::AsyncTransportCertificate* /*peerCert*/,
         const std::string& /* nextProtocolName */,
         SecureTransportType /* secureTransportType */,
         const wangle::TransportInfo& /* tinfo */) { return true; };
  startServer(nullptr, /*expectRequest=*/true);
  initClient();

  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = folly::coro::blockingWait(
      HTTPClient::get(&evb, url, std::chrono::milliseconds(500), useQuic),
      &evb);
  EXPECT_NE(response.headers.get(), nullptr);
  EXPECT_EQ(response.headers->getStatusCode(), 200);
  stopServer();
}

TEST_P(HTTPServerTests, TestFilterFail) {
  serverConfig_.newConnectionFilter =
      [](const folly::SocketAddress* /* address */,
         const folly::AsyncTransportCertificate* /*peerCert*/,
         const std::string& /* nextProtocolName */,
         SecureTransportType /* secureTransportType */,
         const wangle::TransportInfo& /* tinfo */) { return false; };
  startServer(nullptr, /*expectRequest=*/false);
  initClient();

  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = folly::coro::blockingWait(
      co_awaitTry(
          HTTPClient::get(&evb, url, std::chrono::milliseconds(500), useQuic)),
      &evb);
  EXPECT_TRUE(response.hasException());
  stopServer();
}

TEST_P(HTTPServerTests, TestFilterFailException) {
  serverConfig_.newConnectionFilter =
      [](const folly::SocketAddress* /* address */,
         const folly::AsyncTransportCertificate* /*peerCert*/,
         const std::string& /* nextProtocolName */,
         SecureTransportType /* secureTransportType */,
         const wangle::TransportInfo& /* tinfo */) {
        throw std::runtime_error("filter failed this connection");
        return true;
      };
  startServer(nullptr, /*expectRequest=*/false);
  initClient();

  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = folly::coro::blockingWait(
      co_awaitTry(
          HTTPClient::get(&evb, url, std::chrono::milliseconds(500), useQuic)),
      &evb);
  EXPECT_TRUE(response.hasException());
  stopServer();
}

INSTANTIATE_TEST_SUITE_P(HTTPServerStartStop,
                         HTTPServerTests,
                         testing::Values(TransportType::QUIC,
                                         TransportType::TLS),
                         transportTypeToString);

class HTTPStatsFilterTests : public HTTPServerTests {
  void SetUp() override {
    // makes this test easier with only one server evb
    serverConfig_.numIOThreads = 1;

    // store ref to server evb
    EXPECT_CALL(mockObserver_, onThreadStart(_))
        .Times(1)
        .WillOnce([this](folly::EventBase* evb) { this->serverEvb_ = evb; });

    statsFilterFactory_ = std::make_shared<StatsFactory>();
    serverConfig_.filterFactories.push_back(statsFilterFactory_);
  }

 protected:
  class SourceFactoryTestHandler : public TestHandler {
   public:
    SourceFactoryTestHandler() = default;

    folly::coro::Task<HTTPSourceHolder> handleRequest(
        folly::EventBase* evb,
        HTTPSessionContextPtr ctx,
        HTTPSourceHolder requestSource) override {
      auto result = co_await TestHandler::handleRequest(
          evb, ctx, std::move(requestSource));
      co_return sourceFactory ? sourceFactory() : std::move(result);
    }

    std::function<HTTPSource*(void)> sourceFactory{nullptr};
  };

  // used to get a ref to the server evb since server stats is thread local
  MockServerObserver mockObserver_;
  folly::EventBase* serverEvb_{nullptr};
  std::shared_ptr<StatsFactory> statsFilterFactory_{nullptr};
};

TEST_P(HTTPStatsFilterTests, TestSimplePostStatsFilter) {
  startServer(&mockObserver_, /*expectRequest=*/true);
  initClient();

  // send post request with random body
  constexpr uint8_t bodyLen = 200;
  std::string body = makeBuf(bodyLen)->to<std::string>();
  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = folly::coro::blockingWait(
      HTTPClient::post(
          &evb, url, std::move(body), std::chrono::milliseconds(500), useQuic),
      &evb);
  CHECK(response.headers.get());
  auto statusCode = response.headers->getStatusCode();
  EXPECT_EQ(statusCode, 200);

  // stats is thread local and needs to run in server evb
  CHECK_NOTNULL(serverEvb_)
      ->runImmediatelyOrRunInEventBaseThreadAndWait(
          [this, bodyLen, statusCode]() {
            auto& fakeStats = statsFilterFactory_->stats_;
            EXPECT_EQ(fakeStats.reqs, 1);
            EXPECT_EQ(fakeStats.errors, 0);
            EXPECT_EQ(fakeStats.reqBodyBytes, bodyLen);
            EXPECT_EQ(fakeStats.responseCodes[statusCode / 100], 1);
            // test handler returns OK -> 2 bytes
            EXPECT_EQ(fakeStats.resBodyBytes, 2);
          });

  stopServer();
}

TEST_P(HTTPStatsFilterTests, TestServerErrorHandler) {
  auto handler = std::make_shared<SourceFactoryTestHandler>();
  handler->sourceFactory = []() {
    return new ErrorSource(
        /*body=*/makeBuf(200)->to<std::string>(),
        /*client=*/false,
        /*bytesTillTheError=*/0);
  };
  handler_ = std::move(handler);

  startServer(&mockObserver_, /*expectRequest=*/true);
  initClient();

  // send post request with random body
  constexpr uint16_t bodyLen = 300;
  std::string body = makeBuf(bodyLen)->to<std::string>();
  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;

  EventBase evb;
  auto response = folly::coro::blockingWait(
      folly::coro::co_awaitTry(HTTPClient::post(
          &evb, url, std::move(body), std::chrono::milliseconds(500), useQuic)),
      &evb);
  EXPECT_TRUE(response.hasException());

  // stats is thread local and needs to run in server evb
  CHECK_NOTNULL(serverEvb_)
      ->runImmediatelyOrRunInEventBaseThreadAndWait([this, bodyLen]() {
        auto& fakeStats = statsFilterFactory_->stats_;
        EXPECT_EQ(fakeStats.reqs, 1);
        EXPECT_EQ(fakeStats.errors, 1);
        EXPECT_EQ(fakeStats.reqBodyBytes, bodyLen);
        // status code 200 at index 200/100 = 2
        EXPECT_EQ(fakeStats.responseCodes[2], 1);
        // in HTTPErrorCode2ProxygenError, internal error gets mapped to
        // kErrorStreamAbort.
        constexpr uint8_t expectedError = ProxygenError::kErrorStreamAbort;
        EXPECT_EQ(fakeStats.errorTypes[expectedError], 1);
        // error source bytesTillTheError = 0
        EXPECT_EQ(fakeStats.resBodyBytes, 0);
      });

  stopServer();
}

INSTANTIATE_TEST_SUITE_P(HTTPStatsFilter,
                         HTTPStatsFilterTests,
                         testing::Values(TransportType::QUIC,
                                         TransportType::TLS),
                         transportTypeToString);

class MultiAcceptorHttpServerTest : public HTTPServerTests {
 protected:
  void startServer(std::vector<folly::SocketAddress>&& addresses,
                   MockServerObserver* mockObserver = nullptr) {
    CHECK(handler_);
    HTTPServer::SocketAcceptorConfigFactoryFn socketAcceptorConfigFactoryFn =
        [this, addresses](folly::EventBase& evb,
                          [[maybe_unused]] const HTTPServer::Config& config) {
          auto tlsConfig = this->getTLSConfig();
          std::vector<HTTPServer::SocketAcceptorConfig> configs;
          auto accConfig = std::make_shared<proxygen::AcceptorConfiguration>();
          accConfig->sslContextConfigs.push_back(tlsConfig);
          for (const auto& address : addresses) {
            folly::AsyncServerSocket::UniquePtr serverSocket(
                new folly::AsyncServerSocket(&evb));
            serverSocket->bind(address);
            serverSocket->listen(1024);
            configs.emplace_back(std::move(serverSocket), accConfig);
          }
          return configs;
        };

    server_ = ScopedHTTPServer::start(std::move(serverConfig_),
                                      handler_,
                                      mockObserver,
                                      socketAcceptorConfigFactoryFn);
  }
};

TEST_F(MultiAcceptorHttpServerTest, TestMultiplePorts) {
  initClient();
  std::vector<std::string> ips = {"127.0.0.1", "::1"};
  std::vector<folly::SocketAddress> addresses;
  addresses.reserve(ips.size());
  for (const auto& ip : ips) {
    addresses.emplace_back(ip, 0);
  }
  startServer(std::move(addresses));

  auto boundAddresses = server_->addresses();
  EXPECT_EQ(boundAddresses.size(), 2);
  EXPECT_NE(boundAddresses[0].getPort(), boundAddresses[1].getPort());

  EventBase evb;
  for (const auto& address : boundAddresses) {
    auto url = fmt::format("https://{}/test", address.describe());
    auto response = folly::coro::blockingWait(
        HTTPClient::get(&evb, url, std::chrono::milliseconds(500), false),
        &evb);
    CHECK(response.headers.get());
    EXPECT_EQ(response.headers->getStatusCode(), 200);
  }

  // Hit a bad port and confirm that we get an error.
  auto badAddress = folly::SocketAddress(listenAddress_, 9999);
  auto url = fmt::format("https://{}/test", badAddress.describe());
  auto result = folly::coro::blockingWait(
      folly::coro::co_awaitTry(
          HTTPClient::get(&evb, url, std::chrono::milliseconds(500), false)),
      &evb);
  EXPECT_TRUE(result.hasException());

  stopServer();
}

TEST_F(MultiAcceptorHttpServerTest, TestShutdown) {
  initClient();
  std::vector<std::string> ips = {"127.0.0.1", "::1"};
  std::vector<folly::SocketAddress> addresses;
  addresses.reserve(ips.size());
  for (const auto& ip : ips) {
    addresses.emplace_back(ip, 0);
  }
  startServer(std::move(addresses));

  auto boundAddresses = server_->addresses();
  EXPECT_EQ(boundAddresses.size(), 2);

  stopServer();

  EventBase evb;
  for (const auto& address : boundAddresses) {
    auto url = fmt::format("https://{}/test", address.describe());
    auto result = folly::coro::blockingWait(
        folly::coro::co_awaitTry(
            HTTPClient::get(&evb, url, std::chrono::milliseconds(500), false)),
        &evb);
    EXPECT_TRUE(result.hasException());
  }
}
} // namespace proxygen::coro::test
