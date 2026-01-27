/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/conformance/stresstest/client/ClientFactory.h>
#include <thrift/conformance/stresstest/client/FizzStopTLSConnector.h>
#include "common/services/cpp/security/FizzThriftFactory.h"

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/MultiClientExtensions.h>
#include <folly/FileUtil.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/experimental/io/AsyncIoUringSocket.h>
#include <folly/experimental/io/AsyncIoUringSocketFactory.h>
#include <folly/experimental/io/IoUringBackend.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <quic/client/QuicClientAsyncTransport.h>
#include <quic/common/events/FollyQuicEventBase.h>
#include <quic/common/events/HighResQuicTimer.h>
#include <quic/common/udpsocket/FollyQuicAsyncUDPSocket.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>
#include <thrift/conformance/stresstest/client/ClientConfig.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersClientExtension.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

namespace apache::thrift::stress {

namespace {

std::function<std::shared_ptr<folly::SSLContext>()> customSslContextFn;
std::function<std::shared_ptr<fizz::client::FizzClientContext>()>
    customFizzClientContextFn;
std::function<std::shared_ptr<fizz::CertificateVerifier>()>
    customFizzVerifierFn;

folly::AsyncTransport::UniquePtr createEPollSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg);
folly::AsyncTransport::UniquePtr createTLSSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg);
folly::AsyncTransport::UniquePtr createFizzSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg);
#if FOLLY_HAS_LIBURING
folly::AsyncTransport::UniquePtr createIOUring(
    folly::EventBase* evb, const ClientConnectionConfig& cfg);
folly::AsyncTransport::UniquePtr createIOUringTLS(
    folly::EventBase* evb, const ClientConnectionConfig& cfg);
folly::AsyncTransport::UniquePtr createIOUringFizz(
    folly::EventBase* evb, const ClientConnectionConfig& cfg);
#endif
class ConnectCallback : public folly::AsyncSocket::ConnectCallback,
                        public folly::DelayedDestruction {
 public:
  void connectSuccess() noexcept override { destroy(); }
  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    LOG(FATAL) << "Socket connection failed: " << ex.what();
  }
};

std::shared_ptr<folly::SSLContext> getSslContext(
    const ClientConnectionConfig& cfg) {
  static auto sslContext = [&]() {
    if (customSslContextFn) {
      return customSslContextFn();
    }
    auto ctx = std::make_shared<folly::SSLContext>();
    ctx->loadCertificate(cfg.certPath.c_str());
    ctx->loadPrivateKey(cfg.keyPath.c_str());
    ctx->loadTrustedCertificates(cfg.trustedCertsPath.c_str());
    ctx->setVerificationOption(folly::SSLContext::SSLVerifyPeerEnum::VERIFY);
    return ctx;
  }();
  return sslContext;
}

std::shared_ptr<fizz::client::FizzClientContext> getFizzContext(
    const ClientConnectionConfig& cfg) {
  static auto fizzContext = [&]() {
    if (customFizzClientContextFn) {
      return customFizzClientContextFn();
    }
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    ctx->setSupportedAlpns({"rs"});
    if (!cfg.certPath.empty() && !cfg.keyPath.empty()) {
      std::string cert, key;
      folly::readFile(cfg.certPath.c_str(), cert);
      folly::readFile(cfg.keyPath.c_str(), key);
      auto selfCert = fizz::openssl::CertUtils::makeSelfCert(cert, key);
      auto certMgr = std::make_shared<fizz::client::CertManager>();
      certMgr->addCert(std::move(selfCert));
      ctx->setClientCertManager(std::move(certMgr));
    }
    return ctx;
  }();
  return fizzContext;
}

std::shared_ptr<fizz::CertificateVerifier> getFizzVerifier(
    const ClientConnectionConfig& cfg) {
  if (customFizzVerifierFn) {
    return customFizzVerifierFn();
  }
  if (!cfg.trustedCertsPath.empty()) {
    return fizz::DefaultCertificateVerifier::createFromCAFile(
        fizz::VerificationContext::Client, cfg.trustedCertsPath);
  } else {
    return std::make_shared<fizz::DefaultCertificateVerifier>(
        fizz::VerificationContext::Client);
  }
}

folly::AsyncTransport::UniquePtr createSocketWithEPoll(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  switch (cfg.security) {
    case ClientSecurity::None:
      return createEPollSocket(evb, cfg);
    case ClientSecurity::TLS:
      return createTLSSocket(evb, cfg);
    case ClientSecurity::FIZZ: {
      if (cfg.stopTLSv1) {
        folly::ScopedEventBaseThread connThread;
        FizzStopTLSConnector connector;
        auto fut =
            connector.connect(cfg.serverHost, connThread.getEventBase(), evb);
        try {
          // Wait for StopTLS negotiation to complete with timeout
          auto result = std::move(fut).get(std::chrono::seconds(10));
          // The result is nullptr; now create the actual socket on the correct
          // thread
          auto [fd, zeroCopyBufId] = connector.getSocketParams();

          // Create socket directly - we're already on the correct EventBase
          // thread during client initialization
          return folly::AsyncSocket::UniquePtr(
              new folly::AsyncSocket(evb, fd, zeroCopyBufId));
        } catch (const std::exception& e) {
          LOG(ERROR) << "StopTLSv1 negotiation failed: " << e.what()
                     << ". Server may not support stopTLSv1. "
                     << "Falling back to regular FIZZ connection.";
          // Fall back to regular FIZZ connection if stopTLSv1 fails
          return createFizzSocket(evb, cfg);
        }
      } else {
        return createFizzSocket(evb, cfg);
      }
    }
  }
}

#if FOLLY_HAS_LIBURING
folly::AsyncTransport::UniquePtr createSocketWithIOUring(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  switch (cfg.security) {
    case ClientSecurity::None:
      return createIOUring(evb, cfg);
    case ClientSecurity::TLS:
      return createIOUringTLS(evb, cfg);
    case ClientSecurity::FIZZ:
      return createIOUringFizz(evb, cfg);
  }
}
#endif
folly::AsyncTransport::UniquePtr createQuicSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  static quic::TransportSettings ts{
      .advertisedInitialConnectionFlowControlWindow = 60 * 1024 * 1024,
      .advertisedInitialBidiLocalStreamFlowControlWindow = 60 * 1024 * 1024,
      .numGROBuffers_ = quic::kMaxNumGROBuffers,
      .connectUDP = true,
      .pacingEnabled = true,
      .pacingTickInterval = std::chrono::microseconds(200),
      .writeConnectionDataPacketsLimit = 50,
      .batchingMode = quic::QuicBatchingMode::BATCHING_MODE_GSO,
      .maxBatchSize = 50,
      .initCwndInMss = 100,
      .maxCwndInMss = quic::kLargeMaxCwndInMss,
      .maxRecvBatchSize = 64,
      .shouldUseRecvmmsgForBatchRecv = true,
  };
  auto qEvb = std::make_shared<quic::FollyQuicEventBase>(evb);
  auto sock = std::make_unique<quic::FollyQuicAsyncUDPSocket>(qEvb);
  constexpr size_t kBufSize = 4 * 1024 * 1024;
  sock->setRcvBuf(kBufSize);
  sock->setSndBuf(kBufSize);
  auto quicClient = std::make_shared<quic::QuicClientTransport>(
      qEvb,
      std::move(sock),
      quic::FizzClientQuicHandshakeContext::Builder()
          .setFizzClientContext(getFizzContext(cfg))
          .setCertificateVerifier(getFizzVerifier(cfg))
          .build());
  quicClient->setPacingTimer(
      std::make_shared<quic::HighResQuicTimer>(
          evb, std::chrono::microseconds(200)));
  quicClient->setTransportSettings(ts);
  quicClient->addNewPeerAddress(cfg.serverHost);
  auto quicAsyncTransport = new quic::QuicClientAsyncTransport(quicClient);
  return folly::AsyncTransport::UniquePtr(quicAsyncTransport);
}

folly::AsyncTransport::UniquePtr createSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  if (cfg.useQuic) {
    return createQuicSocket(evb, cfg);
  } else if (cfg.ioUring) {
#if FOLLY_HAS_LIBURING
    return createSocketWithIOUring(evb, cfg);
#else
    LOG(FATAL) << "IoUring not supported";
#endif
  } else {
    return createSocketWithEPoll(evb, cfg);
  }
}

folly::AsyncTransport::UniquePtr createEPollSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto sock = folly::AsyncSocket::newSocket(evb);
  sock->connect(cfg.connectCb, cfg.serverHost);
  return sock;
}

folly::AsyncTransport::UniquePtr createTLSSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto sock = folly::AsyncSSLSocket::newSocket(getSslContext(cfg), evb);
  sock->connect(cfg.connectCb, cfg.serverHost);
  return sock;
}

folly::AsyncTransport::UniquePtr createFizzSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto fizzContext = getFizzContext(cfg);
  std::shared_ptr<apache::thrift::ThriftParametersClientExtension>
      thriftExtension{nullptr};
  if (cfg.stopTLSv2) {
    auto thriftParametersContext =
        std::make_shared<apache::thrift::ThriftParametersContext>();
    thriftParametersContext->setUseStopTLSV2(true);
    thriftExtension =
        std::make_shared<apache::thrift::ThriftParametersClientExtension>(
            thriftParametersContext);
    fizzContext->setFactory(
        std::make_shared<facebook::services::FizzThriftFactory>());
  }

  std::vector<std::shared_ptr<fizz::ClientExtensions>> extensions;
  if (thriftExtension != nullptr) {
    extensions.push_back(thriftExtension);
  }

  auto fizzClient = fizz::client::AsyncFizzClient::UniquePtr(
      new fizz::client::AsyncFizzClient(
          evb,
          fizzContext,
          extensions.empty()
              ? nullptr
              : std::make_shared<fizz::client::MultiClientExtensions>(
                    std::move(extensions))));

  fizzClient->connect(
      cfg.serverHost, cfg.connectCb, getFizzVerifier(cfg), {}, {});
  return fizzClient;
}

#if FOLLY_HAS_LIBURING
folly::AsyncTransport::UniquePtr createIOUring(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto ring = new folly::AsyncIoUringSocket(evb);
  if (cfg.ioUringZcrx) {
    folly::AsyncIoUringSocketFactory::bindSocketForZcRx(
        *ring, cfg.serverHost.getIPAddress(), cfg.serverHost.getPort());
  }
  ring->connect(cfg.connectCb, cfg.serverHost);
  return folly::AsyncTransport::UniquePtr(ring);
}

folly::AsyncTransport::UniquePtr createIOUringTLS(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto sock = folly::AsyncSSLSocket::newSocket(getSslContext(cfg), evb);
  auto ring = new folly::AsyncIoUringSocket(std::move(sock));
  if (cfg.ioUringZcrx) {
    folly::AsyncIoUringSocketFactory::bindSocketForZcRx(
        *ring, cfg.serverHost.getIPAddress(), cfg.serverHost.getPort());
  }
  ring->connect(cfg.connectCb, cfg.serverHost);
  return folly::AsyncTransport::UniquePtr(ring);
}

folly::AsyncTransport::UniquePtr createIOUringFizz(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto sock = new folly::AsyncIoUringSocket(evb);
  if (cfg.ioUringZcrx) {
    folly::AsyncIoUringSocketFactory::bindSocketForZcRx(
        *sock, cfg.serverHost.getIPAddress(), cfg.serverHost.getPort());
  }
  auto fizzClient = fizz::client::AsyncFizzClient::UniquePtr(
      new fizz::client::AsyncFizzClient(
          folly::AsyncTransport::UniquePtr(sock), getFizzContext(cfg)));
  fizzClient->connect(
      cfg.serverHost, cfg.connectCb, getFizzVerifier(cfg), {}, {});
  return fizzClient;
}
#endif
} // namespace

/* static */ std::unique_ptr<StressTestAsyncClient>
ClientFactory::createRocketClient(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto chan = RocketClientChannel::newChannel(createSocket(evb, cfg));

  chan->setProtocolId(cfg.thriftProtocol);

  if (cfg.compressionConfigOpt) {
    chan->setDesiredCompressionConfig(*cfg.compressionConfigOpt);
  }

  return std::make_unique<StressTestAsyncClient>(std::move(chan));
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::vector<std::unique_ptr<StressTestClient>>,
    createClients,
    folly::EventBase* evb,
    const ClientConfig& cfg,
    ClientRpcStats& stats) {
  std::vector<std::unique_ptr<StressTestClient>> clients;

  // Create Thrift clients
  for (size_t connectionIdx = 0; connectionIdx < cfg.numConnectionsPerThread;
       connectionIdx++) {
    std::shared_ptr<StressTestAsyncClient> connection =
        ClientFactory::createRocketClient(evb, cfg.connConfig);
    for (size_t i = 0; i < cfg.numClientsPerConnection; i++) {
      clients.emplace_back(
          std::make_unique<ThriftStressTestClient>(
              connection, stats, cfg.enableChecksum));
    }
  }
  return clients;
}

/* static */ void ClientFactory::useCustomSslContext(
    std::function<std::shared_ptr<folly::SSLContext>()> fn) {
  customSslContextFn = std::move(fn);
}

/* static */ void ClientFactory::useCustomFizzClientContext(
    std::function<std::shared_ptr<fizz::client::FizzClientContext>()> fn) {
  customFizzClientContextFn = std::move(fn);
}

/* static */ void ClientFactory::useCustomFizzVerifier(
    std::function<std::shared_ptr<fizz::CertificateVerifier>()> fn) {
  customFizzVerifierFn = std::move(fn);
}

} // namespace apache::thrift::stress
