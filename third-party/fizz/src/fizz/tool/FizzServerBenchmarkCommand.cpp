/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/experimental/batcher/Batcher.h>
#include <fizz/experimental/server/BatchSignatureAsyncSelfCert.h>
#include <fizz/extensions/delegatedcred/DelegatedCredentialCertManager.h>

#include <fizz/protocol/OpenSSLFactory.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/SlidingBloomReplayCache.h>
#include <fizz/server/TicketTypes.h>
#include <fizz/tool/FizzCommandCommon.h>
#include <fizz/util/KeyLogWriter.h>
#include <fizz/util/Parse.h>

#include <folly/Format.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncServerSocket.h>

#include <string>
#include <vector>

using namespace fizz::server;
using namespace folly;

namespace fizz {
namespace tool {
namespace {

/**
 * Server Side Benchmark Tool:
 * We simpilfy the settings to focus on the performance evaluation.
 *
 *  - Enforce verification of server certificate
 *  - Enforce TLS_AES_128_GCM_SHA256 for cipher
 *  - No early data in ClientHello
 *  - No Application Layer Protocol Negotiation (ALPN)
 *  - No certificate compression
 *  - No client side certificate verification
 *  - Enforce loop of the server
 *  - Disable HTTP
 */

void printUsage() {
  // clang-format off
  std::cerr
    << "Usage: server_benchmark args\n"
    << "\n"
    << "Supported arguments:\n"
    << " -accept port             (set port to accept connections on. Default: 8443)\n"
    << " -threads num             (set # of threads used for handling TLS handshakes)\n"
    << " -cert cert               (PEM format server certificate. Default: none, generates a self-signed cert)\n"
    << " -key key                 (PEM format private key for server certificate. Default: none)\n"
    << " -pass password           (private key password. Default: none)\n"
    << " -backlog num             (maximum number of queued connections; a small backlog can lead to potential\n"
    << "                           connection drop or long latency. Default: 100)\n"
    << " -batch                   (use the batch signature scheme ecdsa_secp256r1_sha256_batch)\n";
  // clang-format on
}

class ServerTask : public AsyncFizzServer::HandshakeCallback {
 public:
  explicit ServerTask(
      EventBase* evb,
      std::shared_ptr<FizzServerContext> serverContext)
      : evb_(evb), serverContext_(serverContext) {}

  void start(int fd) {
    auto sock = new AsyncSocket(evb_, folly::NetworkSocket::fromFd(fd));
    fizzServer_ = AsyncFizzServer::UniquePtr(
        new AsyncFizzServer(AsyncSocket::UniquePtr(sock), serverContext_));
    fizzServer_->accept(this);
  }

  void endTask() {
    delete this;
  }

  void fizzHandshakeSuccess(AsyncFizzServer*) noexcept override {
    endTask();
  }

  void fizzHandshakeError(AsyncFizzServer*, exception_wrapper ex) noexcept
      override {
    VLOG(1) << "Error: " << ex.what();
    endTask();
  }

  virtual void fizzHandshakeAttemptFallback(
      AttemptVersionFallback) noexcept override {
    endTask();
  }

 private:
  EventBase* evb_;
  std::shared_ptr<FizzServerContext> serverContext_;
  AsyncFizzServer::UniquePtr fizzServer_;
};

class FizzServerAcceptor : AsyncServerSocket::AcceptCallback {
 public:
  explicit FizzServerAcceptor(
      uint16_t port,
      size_t backlog,
      EventBase* evb,
      std::shared_ptr<FizzServerContext> serverContext,
      std::shared_ptr<IOThreadPoolExecutor> threadExe)
      : evb_(evb), serverContext_(serverContext), threadExe_(threadExe) {
    socket_ = AsyncServerSocket::UniquePtr(new AsyncServerSocket(evb_));
    socket_->bind(port);
    socket_->listen(backlog);
    socket_->addAcceptCallback(this, evb_);
    socket_->startAccepting();
    LOG(INFO) << "Started listening on " << socket_->getAddress();
  }

  void connectionAccepted(
      folly::NetworkSocket fdNetworkSocket,
      const SocketAddress& clientAddr,
      AcceptInfo /* info */) noexcept override {
    int fd = fdNetworkSocket.toFd();
    LOG(INFO) << "Connection accepted from " << clientAddr;

    via(threadExe_->weakRef()).thenValue([=](auto&&) {
      auto evb = folly::EventBaseManager::get()->getEventBase();
      auto task = new ServerTask(evb, serverContext_);
      task->start(fd);
    });
  }

  void acceptError(folly::exception_wrapper ex) noexcept override {
    LOG(ERROR) << "Failed to accept connection: " << ex;
  }

 private:
  EventBase* evb_;
  std::shared_ptr<FizzServerContext> serverContext_;
  std::shared_ptr<IOThreadPoolExecutor> threadExe_;
  AsyncServerSocket::UniquePtr socket_;
};

} // namespace

int fizzServerBenchmarkCommand(const std::vector<std::string>& args) {
  // configurable parameters and their default values
  uint16_t port = 8443;
  std::string certPath;
  std::string keyPath;
  std::string keyPass;
  int threadNum = 1;
  size_t backlog = 100;
  std::vector<std::vector<CipherSuite>> ciphers{
      {CipherSuite::TLS_AES_128_GCM_SHA256}};
  std::vector<ProtocolVersion> versions{
      ProtocolVersion::tls_1_3, ProtocolVersion::tls_1_3_28};
  bool enableBatch = false;
  size_t batchNumMsgThreshold = 0;
  std::shared_ptr<SynchronizedBatcher<Sha256>> batcher;

  // Argument Handler Map
  // clang-format off
  FizzArgHandlerMap handlers = {
    {"-accept", {true, [&port](const std::string& arg) {
        port = portFromString(arg, true);
    }}},
    {"-cert", {true, [&certPath](const std::string& arg) {
      certPath = arg;
    }}},
    {"-key", {true, [&keyPath](const std::string& arg) {
      keyPath = arg;
    }}},
    {"-pass", {true, [&keyPass](const std::string& arg) {
      keyPass = arg;
    }}},
    {"-threads", {true, [&threadNum](const std::string& arg) {
      threadNum = std::stoi(arg);
    }}},
    {"-backlog", {true, [&backlog](const std::string& arg) {
      backlog = std::stoi(arg);
    }}},
    {"-batch", {true, [&enableBatch, &batchNumMsgThreshold](const std::string& arg) {
      enableBatch = true;
      batchNumMsgThreshold = std::stoi(arg);
    }}}
  };
  // clang-format on

  // parse arguments
  try {
    if (parseArguments(args, handlers, printUsage)) {
      // Parsing failed, return
      return 1;
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what();
    return 1;
  }
  if (certPath.empty() || keyPath.empty()) {
    LOG(ERROR)
        << "-cert and -key are both required for the server benchmark tool";
    return 1;
  }

  // set up the IO Thread Pool and get the EventBase
  EventBase evb; // main thread event base, used for accepting new connections
  auto threadExe = std::make_shared<IOThreadPoolExecutor>(
      threadNum,
      std::make_shared<NamedThreadFactory>("ServerBenchmarkPool"),
      folly::EventBaseManager::get(),
      IOThreadPoolExecutor::Options().setWaitForAll(true));

  // prepare FizzServerContext
  auto serverContext = std::make_shared<FizzServerContext>();
  serverContext->setSupportedCiphers(std::move(ciphers));
  auto ticketCipher = std::make_shared<
      Aead128GCMTicketCipher<TicketCodec<CertificateStorage::X509>>>(
      std::make_shared<OpenSSLFactory>(), std::make_shared<CertManager>());
  auto ticketSeed = RandomGenerator<32>().generateRandom();
  ticketCipher->setTicketSecrets({{range(ticketSeed)}});
  serverContext->setTicketCipher(ticketCipher);
  serverContext->setSupportedVersions(std::move(versions));

  // load Server's certificate and private key
  std::unique_ptr<CertManager> certManager =
      std::make_unique<fizz::extensions::DelegatedCredentialCertManager>();
  std::vector<std::shared_ptr<CertificateCompressor>> compressors;
  {
    std::string certData;
    std::string keyData;
    if (!readFile(certPath.c_str(), certData)) {
      LOG(ERROR) << "Failed to read certificate";
      return 1;
    } else if (!readFile(keyPath.c_str(), keyData)) {
      LOG(ERROR) << "Failed to read private key";
      return 1;
    }
    std::unique_ptr<SelfCert> cert;
    if (!keyPass.empty()) {
      cert = CertUtils::makeSelfCert(certData, keyData, keyPass, compressors);
    } else {
      cert = CertUtils::makeSelfCert(certData, keyData, compressors);
    }
    std::shared_ptr<SelfCert> sharedCert = std::move(cert);
    if (enableBatch) {
      batcher = std::make_shared<SynchronizedBatcher<Sha256>>(
          batchNumMsgThreshold, sharedCert, CertificateVerifyContext::Server);
      auto batchCert =
          std::make_shared<BatchSignatureAsyncSelfCert<Sha256>>(batcher);
      serverContext->setSupportedSigSchemes(batchCert->getSigSchemes());
      certManager->addCert(batchCert, true);
    } else {
      serverContext->setSupportedSigSchemes(sharedCert->getSigSchemes());
      certManager->addCert(sharedCert, true);
    }
  }
  serverContext->setCertManager(std::move(certManager));

  // start to listen to new connections
  FizzServerAcceptor acceptor(port, backlog, &evb, serverContext, threadExe);
  evb.loop();
  return 0;
}

} // namespace tool
} // namespace fizz
