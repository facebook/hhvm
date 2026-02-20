/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <folly/system/HardwareConcurrency.h>
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
#include <folly/testing/TestUtil.h>
#include <proxygen/lib/http/coro/test/TestUtils.h>
#include <quic/api/test/Mocks.h>
#include <quic/logging/test/Mocks.h>
#include <quic/state/test/MockQuicStats.h>
#include <quic/state/test/Mocks.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>

using namespace proxygen::coro;
using folly::coro::blockingWait;

namespace {

std::string_view getTestDir() {
  static const std::string kTestDir =
      getContainingDirectory(XLOG_FILENAME).str();
  return kTestDir;
}

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

class MockFizzLoggingCallback : public wangle::FizzLoggingCallback {
 public:
  MOCK_METHOD(void,
              logFizzHandshakeSuccess,
              (const fizz::server::AsyncFizzServer&,
               const wangle::TransportInfo&),
              (noexcept, override));
  MOCK_METHOD(void,
              logFallbackHandshakeSuccess,
              (const folly::AsyncSSLSocket&, const wangle::TransportInfo&),
              (noexcept, override));
  MOCK_METHOD(void,
              logFizzHandshakeFallback,
              (const fizz::server::AsyncFizzServer&,
               const wangle::TransportInfo&),
              (noexcept, override));
  MOCK_METHOD(void,
              logFizzHandshakeError,
              (const fizz::server::AsyncFizzServer&,
               const folly::exception_wrapper&),
              (noexcept, override));
  MOCK_METHOD(void,
              logFallbackHandshakeError,
              (const folly::AsyncSSLSocket&,
               const folly::AsyncSocketException&,
               const fizz::server::HandshakeLogging*),
              (noexcept, override));
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
      const std::string kTestDir{getTestDir()};
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
  auto response = blockingWait(
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
  auto response = blockingWait(
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
  auto cpuCount = folly::available_concurrency();
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
  auto response = blockingWait(
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
  auto response = blockingWait(
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
  auto response =
      blockingWait(co_awaitTry(HTTPClient::get(
                       &evb, url, std::chrono::milliseconds(500), useQuic)),
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
  auto response =
      blockingWait(co_awaitTry(HTTPClient::get(
                       &evb, url, std::chrono::milliseconds(500), useQuic)),
                   &evb);
  EXPECT_TRUE(response.hasException());
  stopServer();
}

TEST_P(HTTPServerTests, TestFizzLoggingCallbackInvoked) {
  auto mockFizzLoggingCallback = std::make_shared<MockFizzLoggingCallback>();
  if (GetParam() == TransportType::TLS) {
    EXPECT_CALL(*mockFizzLoggingCallback, logFizzHandshakeSuccess(_, _))
        .Times(1);
  }

  serverConfig_.fizzLoggingCallback = std::move(mockFizzLoggingCallback);
  startServer(nullptr, /*expectRequest=*/true);
  initClient();

  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = blockingWait(
      HTTPClient::get(&evb, url, std::chrono::milliseconds(500), useQuic),
      &evb);
  EXPECT_NE(response.headers.get(), nullptr);
  EXPECT_EQ(response.headers->getStatusCode(), 200);

  stopServer();
}

/**
 * Unit test extrapolated from proxygen/httpserver/tests/HTTPServerTest.cpp
 */
TEST_P(HTTPServerTests, TestUpdateTLSCredentials) {
  // Set up a temporary file with credentials that we will update
  folly::test::TemporaryFile credFile;
  auto copyCreds = [path = credFile.path()](const std::string& certFile,
                                            const std::string& keyFile) {
    std::string certData, keyData;
    folly::readFile(certFile.c_str(), certData);
    folly::writeFile(certData, path.c_str(), O_WRONLY | O_CREAT | O_TRUNC);
    folly::writeFile(std::string("\n"), path.c_str(), O_WRONLY | O_APPEND);
    folly::readFile(keyFile.c_str(), keyData);
    folly::writeFile(keyData, path.c_str(), O_WRONLY | O_APPEND);
  };

  auto getCertDigest = [&](const X509* x) -> std::string {
    unsigned int n;
    unsigned char md[EVP_MAX_MD_SIZE];
    const EVP_MD* dig = EVP_sha256();

    if (!X509_digest(x, dig, md, &n)) {
      throw std::runtime_error("Cannot calculate digest");
    }
    return std::string((const char*)md, n);
  };

  // init tmp file w/ cert1.pem & cert1.key
  const std::string testDir{getTestDir()};
  const std::string credFilePath{credFile.path().string()};

  copyCreds(testDir + "certs/test_cert1.pem", testDir + "certs/test_key1.pem");
  // set tlsConfig certificate to tmp file
  auto tlsConfig = getTLSConfig();
  tlsConfig.setCertificate(credFilePath, credFilePath, "");

  // start server with custom tlsConfig constructed above
  serverConfig_.socketConfig.bindAddress.setFromIpPort(listenAddress_,
                                                       listenPort_);
  serverConfig_.socketConfig.sslContextConfigs.emplace_back(
      std::move(tlsConfig));
  server_ =
      ScopedHTTPServer::start(std::move(serverConfig_), handler_, nullptr);

  struct TlsConnectCb : public folly::AsyncSocket::ConnectCallback {
    TlsConnectCb(folly::AsyncSSLSocket& sock) : sock(sock) {
    }
    void connectSuccess() noexcept override {
      if (auto cert = sock.getPeerCertificate()) {
        peerCert = folly::OpenSSLTransportCertificate::tryExtractX509(cert);
      }
      baton.post();
    }
    void connectErr(
        const folly::AsyncSocketException& socketEx) noexcept override {
      ex = folly::make_exception_wrapper<folly::AsyncSocketException>(socketEx);
      baton.post();
    }

    folly::AsyncSocket& sock;
    folly::ssl::X509UniquePtr peerCert{nullptr};
    folly::exception_wrapper ex;
    folly::coro::Baton baton;
  };

  folly::EventBase evb;

  // Connect and store digest of server cert
  auto doTlsConnection = [&]() -> auto {
    return folly::coro::co_invoke([&]() -> folly::coro::Task<std::string> {
      folly::AsyncSSLSocket::UniquePtr sock(
          new folly::AsyncSSLSocket(std::make_shared<SSLContext>(), &evb));
      TlsConnectCb cb{*sock};
      sock->connect(&cb, server_->address().value(), 100);
      co_await cb.baton;
      CHECK(!cb.ex && cb.peerCert);
      co_return getCertDigest(cb.peerCert.get());
    });
  };

  /**
   * 1. do tls connection and get first certificate
   * 2. update temp credFile & invoke HTTPServer::updateTlsCredentials
   * 3. do new tls connection and get second certificate
   * 4. verify first cert != second cert
   */
  auto cert1 = blockingWait(doTlsConnection(), &evb);

  // update credFile to a different cert/key
  copyCreds(testDir + "certs/test_cert2.pem", testDir + "certs/test_key2.pem");
  auto& httpServer = server_->getServer();
  httpServer.evb()->runImmediatelyOrRunInEventBaseThreadAndWait(
      [&]() { httpServer.updateTlsCredentials(); });

  // second tls connection post updating server cert file
  auto cert2 = blockingWait(doTlsConnection(), &evb);

  // verify certificates yielded from first and second tls conn attempts are
  // different
  EXPECT_EQ(cert1.length(), SHA256_DIGEST_LENGTH);
  EXPECT_EQ(cert2.length(), SHA256_DIGEST_LENGTH);
  EXPECT_NE(cert1, cert2);
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
  constexpr size_t bodyLen = 100'000;
  auto body = makeBuf(bodyLen)->to<std::string>();
  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;
  EventBase evb;
  auto response = blockingWait(
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
  auto body = makeBuf(bodyLen)->to<std::string>();
  auto url = fmt::format("https://{}/test", server_->address()->describe());
  auto useQuic = GetParam() == TransportType::QUIC;

  EventBase evb;
  auto response = blockingWait(
      co_awaitTry(HTTPClient::post(
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
    auto response = blockingWait(
        HTTPClient::get(&evb, url, std::chrono::milliseconds(500), false),
        &evb);
    CHECK(response.headers.get());
    EXPECT_EQ(response.headers->getStatusCode(), 200);
  }

  // Hit a bad port and confirm that we get an error.
  auto badAddress = folly::SocketAddress(listenAddress_, 9999);
  auto url = fmt::format("https://{}/test", badAddress.describe());
  auto result =
      blockingWait(co_awaitTry(HTTPClient::get(
                       &evb, url, std::chrono::milliseconds(500), false)),
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
    auto result =
        blockingWait(co_awaitTry(HTTPClient::get(
                         &evb, url, std::chrono::milliseconds(500), false)),
                     &evb);
    EXPECT_TRUE(result.hasException());
  }
}

TEST(StatsFilterFactory, LatencyOnDestruction) {
  FakeHTTPServerStats stats;
  auto filters = StatsFilterUtil::makeFilters(&stats);
  delete filters.first;
  delete filters.second;
  EXPECT_EQ(stats.latencies.at(0).count(), 0);
}

} // namespace proxygen::coro::test
