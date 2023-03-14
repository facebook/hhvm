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

#include <fizz/client/AsyncFizzClient.h>
#include <folly/FileUtil.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>

namespace apache {
namespace thrift {
namespace stress {

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
      auto selfCert = fizz::CertUtils::makeSelfCert(cert, key);
      ctx->setClientCertificate(std::move(selfCert));
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

folly::AsyncTransport::UniquePtr createSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  switch (cfg.security) {
    case ClientSecurity::None:
      return createEPollSocket(evb, cfg);
    case ClientSecurity::TLS:
      return createTLSSocket(evb, cfg);
    case ClientSecurity::FIZZ:
      return createFizzSocket(evb, cfg);
  }
}

folly::AsyncTransport::UniquePtr createEPollSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto sock = folly::AsyncSocket::newSocket(evb);
  sock->connect(new ConnectCallback(), cfg.serverHost);
  return sock;
}

folly::AsyncTransport::UniquePtr createTLSSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto sock = folly::AsyncSSLSocket::newSocket(getSslContext(cfg), evb);
  sock->connect(new ConnectCallback(), cfg.serverHost);
  return sock;
}

folly::AsyncTransport::UniquePtr createFizzSocket(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto fizzClient = fizz::client::AsyncFizzClient::UniquePtr(
      new fizz::client::AsyncFizzClient(evb, getFizzContext(cfg)));
  fizzClient->connect(
      cfg.serverHost, new ConnectCallback(), getFizzVerifier(cfg), {}, {});
  return fizzClient;
}

} // namespace

/* static */ std::unique_ptr<StressTestAsyncClient>
ClientFactory::createRocketClient(
    folly::EventBase* evb, const ClientConnectionConfig& cfg) {
  auto chan = RocketClientChannel::newChannel(createSocket(evb, cfg));
  return std::make_unique<StressTestAsyncClient>(std::move(chan));
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<Client<StressTest>>,
    createClient,
    folly::EventBase* evb,
    const ClientConnectionConfig& cfg) {
  return ClientFactory::createRocketClient(evb, cfg);
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

} // namespace stress
} // namespace thrift
} // namespace apache
