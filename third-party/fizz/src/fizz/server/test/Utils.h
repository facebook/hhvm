/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/protocol/DefaultCertificateVerifier.h>
#include <fizz/protocol/OpenSSLFactory.h>
#include <fizz/protocol/test/Utilities.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/TicketTypes.h>
#include <folly/io/async/AsyncServerSocket.h>

namespace fizz {
namespace server {
namespace test {

class FizzTestServer : public folly::AsyncServerSocket::AcceptCallback {
 public:
  class CallbackFactory {
   public:
    virtual ~CallbackFactory() = default;
    virtual AsyncFizzServer::HandshakeCallback* getCallback(
        std::shared_ptr<AsyncFizzServer> server) = 0;
  };

  FizzTestServer(
      folly::EventBase& evb,
      CallbackFactory* factory,
      int port = 0,
      std::string ip = "")
      : factory_(factory), evb_(evb) {
    auto certData =
        fizz::test::createCert("fizz-test-selfsign", false, nullptr);
    std::vector<folly::ssl::X509UniquePtr> certChain;
    certChain.push_back(std::move(certData.cert));
    auto fizzCert = std::make_unique<SelfCertImpl<KeyType::P256>>(
        std::move(certData.key), std::move(certChain));
    auto certManager = std::make_unique<CertManager>();
    certManager->addCert(std::move(fizzCert), true);
    ctx_ = std::make_shared<FizzServerContext>();
    ctx_->setCertManager(std::move(certManager));

    socket_ = folly::AsyncServerSocket::UniquePtr(
        new folly::AsyncServerSocket(&evb_));
    if (ip.empty()) {
      socket_->bind(port);
    } else {
      socket_->bind(
          folly::SocketAddress(ip, port, false /* allowNameLookup */));
    }
    socket_->listen(100);
    socket_->addAcceptCallback(this, &evb_);
    socket_->startAccepting();
  }

  void setFizzContext(std::shared_ptr<FizzServerContext> ctx) {
    ctx_ = ctx;
  }

  void acceptError(folly::exception_wrapper ex) noexcept override {
    LOG(ERROR) << "Accept error: " << ex;
  }

  void connectionAccepted(
      folly::NetworkSocket fdNetworkSocket,
      const folly::SocketAddress& /* clientAddr */,
      AcceptInfo /* info */) noexcept override {
    auto sock = new folly::AsyncSocket(&evb_, fdNetworkSocket);
    std::shared_ptr<AsyncFizzServer> transport = AsyncFizzServer::UniquePtr(
        new AsyncFizzServer(folly::AsyncSocket::UniquePtr(sock), ctx_));
    auto callback = factory_->getCallback(transport);
    if (startHandshakeOnAccept_) {
      transport->accept(callback);
    }
  }

  void setStartHandshakeOnAccept(bool enable) {
    startHandshakeOnAccept_ = enable;
  }

  void setResumption(bool enable) {
    if (enable) {
      auto ticketCipher = std::make_shared<
          Aead128GCMTicketCipher<TicketCodec<CertificateStorage::X509>>>(
          std::make_shared<OpenSSLFactory>(), std::make_shared<CertManager>());
      auto ticketSeed = RandomGenerator<32>().generateRandom();
      ticketCipher->setTicketSecrets({{folly::range(ticketSeed)}});
      ctx_->setTicketCipher(ticketCipher);
    } else {
      ctx_->setTicketCipher(nullptr);
    }
  }

  void setCertificate(std::unique_ptr<SelfCert> cert) {
    auto certManager = std::make_unique<CertManager>();
    certManager->addCert(std::move(cert), true);
    ctx_->setCertManager(std::move(certManager));
  }

  void enableClientAuthWithChain(
      std::string path,
      ClientAuthMode mode = ClientAuthMode::Optional) {
    ctx_->setClientAuthMode(mode);
    std::string certData;
    CHECK(folly::readFile(path.c_str(), certData));
    auto certRange = folly::ByteRange(folly::StringPiece(certData));

    auto clientAuthCerts =
        folly::ssl::OpenSSLCertUtils::readCertsFromBuffer(certRange);
    ERR_clear_error();
    folly::ssl::X509StoreUniquePtr store(X509_STORE_new());
    for (auto& caCert : clientAuthCerts) {
      if (X509_STORE_add_cert(store.get(), caCert.get()) != 1) {
        auto err = ERR_get_error();
        CHECK(
            ERR_GET_LIB(err) == ERR_LIB_X509 &&
            ERR_GET_REASON(err) == X509_R_CERT_ALREADY_IN_HASH_TABLE)
            << "Could not insert CA certificate into store: "
            << std::string(ERR_error_string(err, nullptr));
      }
    }

    auto verifier = std::make_shared<DefaultCertificateVerifier>(
        VerificationContext::Server, std::move(store));
    ctx_->setClientCertVerifier(std::move(verifier));
  }

  void disableClientAuth() {
    ctx_->setClientAuthMode(ClientAuthMode::None);
    ctx_->setClientCertVerifier(nullptr);
  }

  void setAcceptEarlyData(bool enable) {
    if (enable) {
      ctx_->setEarlyDataSettings(
          true,
          {std::chrono::seconds(-10), std::chrono::seconds(10)},
          std::make_shared<AllowAllReplayReplayCache>());
    } else {
      ctx_->setEarlyDataSettings(false, ClockSkewTolerance(), nullptr);
    }
  }

  void stopAccepting() {
    socket_.reset();
  }

  folly::SocketAddress getAddress() {
    folly::SocketAddress addr;
    socket_->getAddress(&addr);
    return addr;
  }

  std::shared_ptr<FizzServerContext> getFizzContext() {
    return ctx_;
  }

 private:
  folly::AsyncServerSocket::UniquePtr socket_;
  std::shared_ptr<FizzServerContext> ctx_;
  CallbackFactory* factory_;
  folly::EventBase& evb_;
  bool startHandshakeOnAccept_{true};
};

} // namespace test
} // namespace server
} // namespace fizz
