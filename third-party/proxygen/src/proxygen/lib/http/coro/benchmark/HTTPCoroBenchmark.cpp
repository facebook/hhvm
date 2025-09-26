/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/client/HTTPClient.h"
#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include "proxygen/lib/http/coro/server/ScopedHTTPServer.h"
#include <folly/logging/xlog.h>

#include "folly/init/Init.h"
#include <folly/Benchmark.h>
#include <folly/Range.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Generator.h>
#include <proxygen/httpserver/ScopedHTTPServer.h>
#include <proxygen/httpserver/samples/hq/HQServer.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>

using namespace proxygen;
using namespace proxygen::coro;
using namespace quic::samples;

DEFINE_string(cert,
              "",
              "server certificate to use, defaults to client test cert");
DEFINE_string(key, "", "server key to use, defaults to client test key");

namespace {
const std::string kTestDir =
    getContainingDirectory(XLOG_FILENAME).str() + "../client/test/";

std::string getServerCertPath() {
  if (FLAGS_cert.empty()) {
    return kTestDir + "certs/test_cert1.pem";
  } else {
    return FLAGS_cert;
  }
}

std::string getServerKeyPath() {
  if (FLAGS_key.empty()) {
    return kTestDir + "certs/test_key1.pem";
  } else {
    return FLAGS_key;
  }
}

// This is an insecure certificate verifier and is not meant to be
// used in production. Using it in production would mean that this will
// leave everyone insecure.
class InsecureVerifierDangerousDoNotUseInProduction
    : public fizz::CertificateVerifier {
 public:
  ~InsecureVerifierDangerousDoNotUseInProduction() override = default;

  [[nodiscard]] std::shared_ptr<const folly::AsyncTransportCertificate> verify(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override {
    return certs.front();
  }

  [[nodiscard]] std::vector<fizz::Extension> getCertificateRequestExtensions()
      const override {
    return std::vector<fizz::Extension>();
  }
};

enum class TransportType { TCP, TLS, TLS_FIZZ, QUIC };

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

// Handler that returns a fixed-size 200 response
class SizeHandler
    : public HTTPHandler
    , HTTPTransactionHandler {
  std::unique_ptr<folly::IOBuf> respBody_;
  HTTPTransaction* txn_{nullptr};

 public:
  explicit SizeHandler(size_t size) : respBody_(makeBuf(size)) {
  }

  explicit SizeHandler(std::unique_ptr<folly::IOBuf> respBody)
      : respBody_(std::move(respBody)) {
  }

  SizeHandler(const SizeHandler& other) : respBody_(other.respBody_->clone()) {
  }

  SizeHandler& operator=(const SizeHandler& other) {
    if (this != &other) {
      respBody_ = other.respBody_->clone();
    }
    return *this;
  }

  // Coro handler
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* /*evb*/,
      HTTPSessionContextPtr /*ctx*/,
      HTTPSourceHolder requestSource) override {
    auto headerEvent = co_await requestSource.readHeaderEvent();
    XCHECK(headerEvent.eom);
    co_return HTTPFixedSource::makeFixedResponse(200, respBody_->clone());
  }

  // ScopedHTTPServer handler
  void operator()(const HTTPMessage&,
                  std::unique_ptr<folly::IOBuf>,
                  ResponseBuilder& resp) {
    resp.status(200, "OK");
    resp.body(respBody_->clone());
  }

  static HTTPTransactionHandler* makeHandler(
      HTTPMessage*, std::unique_ptr<folly::IOBuf> resp) {
    return new SizeHandler(std::move(resp));
  }

  // HTTPTransactionHandler
  void setTransaction(HTTPTransaction* txn) noexcept override {
    txn_ = txn;
  }
  void detachTransaction() noexcept override {
    txn_ = nullptr;
    delete this;
  }
  void onHeadersComplete(std::unique_ptr<HTTPMessage>) noexcept override {
  }
  void onBody(std::unique_ptr<folly::IOBuf>) noexcept override {
  }
  void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept override {
  }
  void onEOM() noexcept override {
    if (txn_) {
      txn_->sendHeaders(getResponse(200));
      txn_->sendBody(respBody_->clone());
      txn_->sendEOM();
    }
  }
  void onUpgrade(UpgradeProtocol) noexcept override {
  }
  void onError(const HTTPException& error) noexcept override {
    if (error.getDirection() == HTTPException::Direction::INGRESS && txn_) {
      txn_->sendAbort();
      txn_ = nullptr;
    }
  }
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }
};

/* Slightly different thatn folly::ScopedEventBaseThread - this class calls
 * loop() instead of loopForever().
 */
class ScopedEventBaseThread {
 public:
  explicit ScopedEventBaseThread()
      : thread_([this] {
          started_.post();
          scheduled_.wait();
          evb_.loop();
        }) {
    started_.wait();
  }

  void runInThread(std::function<void(folly::EventBase*)> func) {
    evb_.runInEventBaseThread([this, func = std::move(func)] { func(&evb_); });
    scheduled_.post();
  }

  ~ScopedEventBaseThread() {
    thread_.join();
  }

  folly::Baton<> started_;
  folly::Baton<> scheduled_;
  folly::EventBase evb_;
  std::thread thread_;
};

class BenchmarkFixture {
 public:
  explicit BenchmarkFixture(TransportType type,
                            size_t respSize = 0,
                            size_t concurrentReqsPerSess = 1,
                            size_t numClientThreads = 1,
                            bool coro = true)
      : transportType_(type),
        respSize_(respSize),
        concurrentReqsPerSess_(concurrentReqsPerSess),
        numClientThreads_(numClientThreads),
        coro_(coro) {
    BENCHMARK_SUSPEND {
      init();
    }
  }

  ~BenchmarkFixture() {
    BENCHMARK_SUSPEND {
      tearDown();
    }
  }

  void init() {
    // Disable cert verification since the server is self-signed
    HTTPClient::setDefaultCAPaths({});
    HTTPClient::setDefaultFizzCertVerifier(
        std::make_shared<InsecureVerifierDangerousDoNotUseInProduction>());
    if (coro_) {
      initCoro();
    } else {
      initLegacy();
    }
    req_ = getGetRequest();
  }

  void initCoro() {
    proxygen::coro::HTTPServer::Config serverConfig;
    auto tlsConfig = proxygen::coro::HTTPServer::getDefaultTLSConfig();
    tlsConfig.isDefault = true;
    tlsConfig.clientVerification =
        folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
    tlsConfig.setNextProtocols({"h2", "http/1.1"});
    try {
      tlsConfig.setCertificate(getServerCertPath(), getServerKeyPath(), "");
    } catch (const std::exception& ex) {
      XLOG(ERR) << "Invalid certificate file or key file: %s" << ex.what();
    }
    serverConfig.socketConfig.bindAddress.setFromLocalPort(uint16_t(0));
    if (transportType_ != TransportType::TCP) {
      serverConfig.socketConfig.sslContextConfigs.emplace_back(
          std::move(tlsConfig));
    }
    if (transportType_ == TransportType::QUIC) {
      serverConfig.quicConfig = proxygen::coro::HTTPServer::QuicConfig();
      serverConfig.quicConfig->transportSettings.maxNumPTOs = 1000;
      serverConfig.quicConfig->transportSettings.maxCwndInMss =
          quic::kLargeMaxCwndInMss;
      serverConfig.quicConfig->transportSettings.batchingMode =
          quic::QuicBatchingMode::BATCHING_MODE_GSO;
      serverConfig.quicConfig->transportSettings.maxBatchSize = 48;
      serverConfig.quicConfig->transportSettings.dataPathType =
          quic::DataPathType::ContinuousMemory;
      serverConfig.quicConfig->transportSettings
          .writeConnectionDataPacketsLimit = 48;
    }
    serverConfig.shutdownOnSignals = {};

    coroServer_ = proxygen::coro::ScopedHTTPServer::start(
        std::move(serverConfig), std::make_shared<SizeHandler>(respSize_));
    serverAddress_ = *coroServer_->address();
  }

  void initLegacy() {
    if (transportType_ == TransportType::QUIC) {
      HQServerParams params;
      params.serverThreads = 1;
      params.transportSettings.maxNumPTOs = 1000;
      params.transportSettings.maxCwndInMss = quic::kLargeMaxCwndInMss;
      params.transportSettings.batchingMode =
          quic::QuicBatchingMode::BATCHING_MODE_GSO;
      params.transportSettings.dataPathType =
          quic::DataPathType::ContinuousMemory;
      params.transportSettings.maxBatchSize = 48;
      params.transportSettings.writeConnectionDataPacketsLimit = 48;
      resp_ = makeBuf(respSize_);
      legacyQuicServer_ = ScopedHQServer::start(
          params,
          [this](HTTPMessage* msg) {
            return SizeHandler::makeHandler(msg, resp_->clone());
          },
          getServerCertPath(),
          getServerKeyPath(),
          fizz::server::ClientAuthMode::None,
          kDefaultSupportedAlpns);
      serverAddress_ = legacyQuicServer_->getAddress();
    } else {
      std::unique_ptr<wangle::SSLContextConfig> sslCfg;
      if (transportType_ != TransportType::TCP) {
        sslCfg = std::make_unique<wangle::SSLContextConfig>();
        sslCfg->isDefault = true;
        sslCfg->clientVerification =
            folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
        sslCfg->setNextProtocols({"h2", "http/1.1"});
        sslCfg->setCertificate(getServerCertPath(), getServerKeyPath(), "");
      }
      legacyServer_ = proxygen::ScopedHTTPServer::start(SizeHandler(respSize_),
                                                        /*port=*/0,
                                                        /*numThreads=*/1,
                                                        std::move(sslCfg));
      serverAddress_ = legacyServer_->getAddresses()[0].address;
    }
  }

  void tearDown() {
    coroServer_.reset();
    legacyServer_.reset();
  }

  void run(size_t iters) {
    std::list<ScopedEventBaseThread> clientThreads;
    BENCHMARK_SUSPEND {
      clientThreads.resize(numClientThreads_);
    }
    iters /= numClientThreads_;
    for (auto& thr : clientThreads) {
      thr.runInThread([this, iters](folly::EventBase* evb) {
        co_withExecutor(evb, runImpl(evb, iters)).start();
      });
    }
  }

 private:
  TransportType transportType_;
  size_t respSize_{0};
  size_t concurrentReqsPerSess_{100};
  size_t connsPerThread_{15};
  size_t numClientThreads_{1};
  bool coro_{true};
  std::unique_ptr<proxygen::coro::ScopedHTTPServer> coroServer_;
  std::unique_ptr<proxygen::ScopedHTTPServer> legacyServer_;
  std::unique_ptr<ScopedHQServer> legacyQuicServer_;
  folly::SocketAddress serverAddress_;
  HTTPMessage req_;
  std::unique_ptr<folly::IOBuf> resp_;

  // Make one HTTP request from a pool
  folly::coro::Task<void> get(HTTPCoroSessionPool& pool) {
    auto res = co_await co_awaitTry(pool.getSessionWithReservation());
    if (res.hasException()) {
      XLOG(ERR) << res.exception().what();
      res.throwUnlessValue();
    }
    HTTPSourceReader reader;
    reader
        .onHeaders([](std::unique_ptr<HTTPMessage> resp, bool, bool) {
          XLOG_IF(ERR, resp->getStatusCode() != 200)
              << "Error response, status=" << resp->getStatusCode();
          return HTTPSourceReader::Continue;
        })
        .onError([](HTTPSourceReader::ErrorContext, const HTTPError& err) {
          XLOG(ERR) << err.msg;
        });
    auto maybe = co_await co_awaitTry(HTTPClient::request(
        res->session,
        std::move(res->reservation),
        HTTPFixedSource::makeFixedSource(std::make_unique<HTTPMessage>(req_)),
        std::move(reader)));
    if (maybe.hasException()) {
      XLOG(ERR) << maybe.exception().what();
      res.throwUnlessValue();
    }
  }

  std::unique_ptr<HTTPCoroSessionPool> makePool(folly::EventBase* evb) {
    HTTPCoroConnector::SessionParams sessParams;
    sessParams.connFlowControl = 1 << 21;
    sessParams.maxConcurrentOutgoingStreams = concurrentReqsPerSess_;
    std::unique_ptr<HTTPCoroSessionPool> pool;
    auto poolParams = HTTPCoroSessionPool::defaultPoolParams();
    poolParams.maxConnections = connsPerThread_;
    if (transportType_ == TransportType::QUIC) {
      auto qConnParams = HTTPClient::getQuicConnParams();
      qConnParams.transportSettings.maxNumPTOs = 1000;
      auto qConnParamsPtr =
          std::make_shared<const HTTPCoroConnector::QuicConnectionParams>(
              std::move(qConnParams));
      pool =
          std::make_unique<HTTPCoroSessionPool>(evb,
                                                serverAddress_.getAddressStr(),
                                                serverAddress_.getPort(),
                                                poolParams,
                                                std::move(qConnParamsPtr),
                                                sessParams);
    } else {
      pool = std::make_unique<HTTPCoroSessionPool>(
          evb,
          serverAddress_.getAddressStr(),
          serverAddress_.getPort(),
          poolParams,
          HTTPClient::getConnParams(transportImpl(transportType_)),
          sessParams);
    }
    return pool;
  }

  // Single thread main
  //
  // Create a pool, then make iters requests,
  // connsPerThread_ * concurrentReqsPerSess_ at a time
  folly::coro::Task<void> runImpl(folly::EventBase* evb,
                                  size_t iters) noexcept {
    auto pool = makePool(evb);
    auto generator = [this](HTTPCoroSessionPool& pool, size_t iters)
        -> folly::coro::Generator<folly::coro::Task<void>&&> {
      for (size_t i = 0; i < iters; ++i) {
        co_yield get(pool);
      }
    };
    co_await folly::coro::collectAllWindowed(
        generator(*pool, iters), concurrentReqsPerSess_ * connsPerThread_);
  }
};
} // namespace

// Benchmarks

// Size 0, Concurrency 1
BENCHMARK(legacy_plaintext_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 0, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(plaintext_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 0, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_fizz_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 0, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 0, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_openssl_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 0, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(openssl_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 0, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 0, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_1_size_0, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 0, 1, 2);
  fixture.run(iters);
}

// Size 0, concurrency 20
BENCHMARK(legacy_fizz_threads_2_concurrency_20_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 0, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_20_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 0, 20, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_openssl_threads_2_concurrency_20_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 0, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(openssl_threads_2_concurrency_20_size_0, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 0, 20, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_20_size_0, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 0, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_20_size_0, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 0, 20, 2);
  fixture.run(iters);
}

// Size 4096, concurrency 1
BENCHMARK(legacy_plaintext_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 4096, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(plaintext_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 4096, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_fizz_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 4096, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 4096, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_openssl_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 4096, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(openssl_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 4096, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 4096, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_1_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 4096, 1, 2);
  fixture.run(iters);
}

// Size 4096, concurrency 20
BENCHMARK(legacy_fizz_threads_2_concurrency_20_size_4096, iters) {
  BenchmarkFixture fixture(
      TransportType::TLS_FIZZ, 4096, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_20_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 4096, 20, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_openssl_threads_2_concurrency_20_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 4096, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(openssl_threads_2_concurrency_20_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 4096, 20, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_20_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 4096, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_20_size_4096, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 4096, 20, 2);
  fixture.run(iters);
}

// Size 65535, concurrency 1
BENCHMARK(legacy_plaintext_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 65535, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(plaintext_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 65535, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_fizz_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(
      TransportType::TLS_FIZZ, 65535, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 65535, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_openssl_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 65535, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(openssl_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 65535, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 65535, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_1_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 65535, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_plaintext_threads_2_concurrency_1_size_1M, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 1 << 20, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(plaintext_threads_2_concurrency_1_size_1M, iters) {
  BenchmarkFixture fixture(TransportType::TCP, 1 << 20, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_fizz_threads_2_concurrency_1_size_1M, iters) {
  BenchmarkFixture fixture(
      TransportType::TLS_FIZZ, 1 << 20, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_1_size_1M, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 1 << 20, 1, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_1_size_1M, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 1 << 20, 1, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_1_size_1M, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 1 << 20, 1, 2);
  fixture.run(iters);
}

// Size 65535, concurrency 20
BENCHMARK(legacy_fizz_threads_2_concurrency_20_size_65535, iters) {
  BenchmarkFixture fixture(
      TransportType::TLS_FIZZ, 65535, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(fizz_threads_2_concurrency_20_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TLS_FIZZ, 65535, 20, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_openssl_threads_2_concurrency_20_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 65535, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(openssl_threads_2_concurrency_20_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::TLS, 65535, 20, 2);
  fixture.run(iters);
}

BENCHMARK(legacy_quic_threads_2_concurrency_20_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 65535, 20, 2, /*coro=*/false);
  fixture.run(iters);
}

BENCHMARK(quic_threads_2_concurrency_20_size_65535, iters) {
  BenchmarkFixture fixture(TransportType::QUIC, 65535, 20, 2);
  fixture.run(iters);
}

int main(int argc, char** argv) {
  auto init = folly::Init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
