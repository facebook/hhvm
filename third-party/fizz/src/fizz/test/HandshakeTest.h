/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/test/Mocks.h>
#include <fizz/compression/ZlibCertificateCompressor.h>
#include <fizz/compression/ZlibCertificateDecompressor.h>
#include <fizz/crypto/Utils.h>
#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/crypto/test/TestUtil.h>
#include <fizz/extensions/tokenbinding/TokenBindingClientExtension.h>
#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/extensions/tokenbinding/TokenBindingServerExtension.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <fizz/protocol/test/Matchers.h>
#include <fizz/protocol/test/Utilities.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/CookieTypes.h>
#include <fizz/server/TicketTypes.h>
#include <fizz/server/test/Mocks.h>
#include <fizz/test/LocalTransport.h>

namespace fizz {
namespace test {

using namespace folly;
using namespace folly::test;
using namespace fizz::client;
using namespace fizz::extensions;
using namespace fizz::server;

struct ExpectedParameters {
  ProtocolVersion version{ProtocolVersion::tls_1_3};
  CipherSuite cipher{CipherSuite::TLS_AES_128_GCM_SHA256};
  folly::Optional<SignatureScheme> scheme{
      SignatureScheme::ecdsa_secp256r1_sha256};
  folly::Optional<NamedGroup> group{NamedGroup::x25519};
  PskType pskType{PskType::NotAttempted};
  folly::Optional<PskKeyExchangeMode> pskMode;
  folly::Optional<KeyExchangeType> clientKexType{KeyExchangeType::OneRtt};
  folly::Optional<KeyExchangeType> serverKexType{KeyExchangeType::OneRtt};
  folly::Optional<EarlyDataType> earlyDataType{EarlyDataType::NotAttempted};
  folly::Optional<std::string> alpn;
  std::shared_ptr<const Cert> clientCert;
  folly::Optional<CertificateCompressionAlgorithm> serverCertCompAlgo;
};

class HandshakeTest : public Test {
 public:
  void SetUp() override {
    CryptoUtils::init();

    clientContext_ = std::make_shared<FizzClientContext>();
    serverContext_ = std::make_shared<FizzServerContext>();

    auto pskCache = std::make_shared<BasicPskCache>();
    clientContext_->setPskCache(std::move(pskCache));

    auto certManager = std::make_shared<CertManager>();
    std::vector<std::shared_ptr<CertificateCompressor>> compressors = {
        std::make_shared<ZlibCertificateCompressor>(9)};
    std::vector<ssl::X509UniquePtr> rsaCerts;
    rsaCerts.emplace_back(getCert(kRSACertificate));
    certManager->addCert(
        std::make_shared<OpenSSLSelfCertImpl<KeyType::RSA>>(
            getPrivateKey(kRSAKey), std::move(rsaCerts), compressors),
        true);
    std::vector<ssl::X509UniquePtr> p256Certs;
    std::vector<ssl::X509UniquePtr> p384Certs;
    std::vector<ssl::X509UniquePtr> p521Certs;
    p256Certs.emplace_back(getCert(kP256Certificate));
    p384Certs.emplace_back(getCert(kP384Certificate));
    p521Certs.emplace_back(getCert(kP521Certificate));
    certManager->addCert(std::make_shared<OpenSSLSelfCertImpl<KeyType::P256>>(
        getPrivateKey(kP256Key), std::move(p256Certs), compressors));
    certManager->addCert(std::make_shared<OpenSSLSelfCertImpl<KeyType::P384>>(
        getPrivateKey(kP384Key), std::move(p384Certs), compressors));
    certManager->addCert(std::make_shared<OpenSSLSelfCertImpl<KeyType::P521>>(
        getPrivateKey(kP521Key), std::move(p521Certs), compressors));
    serverContext_->setCertManager(certManager);
    serverContext_->setEarlyDataSettings(
        true,
        {std::chrono::seconds(-60), std::chrono::seconds(60)},
        std::make_shared<AllowAllReplayReplayCache>());

    auto caCert = getCert(kClientAuthCACert);
    auto clientCert = getCert(kClientAuthClientCert);
    auto clientKey = getPrivateKey(kClientAuthClientKey);
    folly::ssl::X509StoreUniquePtr store(X509_STORE_new());
    ASSERT_EQ(X509_STORE_add_cert(store.get(), caCert.get()), 1);
    auto verifier = std::make_shared<const DefaultCertificateVerifier>(
        VerificationContext::Server, std::move(store));
    serverContext_->setClientCertVerifier(verifier);
    std::vector<folly::ssl::X509UniquePtr> certVec;
    certVec.emplace_back(std::move(clientCert));
    auto clientSelfCert = std::make_shared<OpenSSLSelfCertImpl<KeyType::RSA>>(
        std::move(clientKey), std::move(certVec));
    clientContext_->setClientCertificate(std::move(clientSelfCert));

    auto ticketCipher = std::make_shared<AES128TicketCipher>(
        serverContext_->getFactoryPtr(), std::move(certManager));
    auto ticketSeed = RandomGenerator<32>().generateRandom();
    ticketCipher->setTicketSecrets({{range(ticketSeed)}});
    server::TicketPolicy policy;
    policy.setTicketValidity(std::chrono::seconds(60));
    ticketCipher->setPolicy(std::move(policy));
    serverContext_->setTicketCipher(std::move(ticketCipher));

    auto tokenCipher = std::make_unique<Aead128GCMTokenCipher>(
        std::vector<std::string>({"Fizz Cookie Cipher v1"}));
    cookieCipher_ =
        std::make_shared<AES128CookieCipher>(std::move(tokenCipher));
    auto cookieSeed = RandomGenerator<32>().generateRandom();
    cookieCipher_->setCookieSecrets({{range(cookieSeed)}});
    cookieCipher_->setContext(serverContext_.get());
    serverContext_->setCookieCipher(cookieCipher_);

    ON_CALL(clientRead_, isBufferMovable_()).WillByDefault(Return(true));
    ON_CALL(serverRead_, isBufferMovable_()).WillByDefault(Return(true));

    resetTransports();
  }

  void resetTransports() {
    clientTransport_ = new LocalTransport();
    auto client = LocalTransport::UniquePtr(clientTransport_);
    serverTransport_ = new LocalTransport();
    auto server = LocalTransport::UniquePtr(serverTransport_);
    client->attachEventBase(&evb_);
    server->attachEventBase(&evb_);
    client->setPeer(server.get());
    server->setPeer(client.get());

    client_.reset(new AsyncFizzClient(
        std::move(client), clientContext_, clientExtensions_));
    server_.reset(new AsyncFizzServer(
        std::move(server), serverContext_, serverExtensions_));
  }

  void resetTransportsAndStartCookieHandshake() {
    clientTransport_ = new LocalTransport();
    auto client = LocalTransport::UniquePtr(clientTransport_);
    serverTransport_ = new LocalTransport();
    auto server = LocalTransport::UniquePtr(serverTransport_);
    client->attachEventBase(&evb_);
    server->attachEventBase(&evb_);
    client->setPeer(server.get());
    server->setPeer(client.get());

    client_.reset(new AsyncFizzClient(
        std::move(client), clientContext_, clientExtensions_));

    folly::test::MockReadCallback serverRawRead;
    ON_CALL(serverRawRead, isBufferMovable_()).WillByDefault(Return(true));

    EXPECT_CALL(serverRawRead, readBufferAvailable_(_))
        .WillOnce(Invoke([&](std::unique_ptr<IOBuf>& readBuf) {
          server->setReadCB(nullptr);
          auto tokenOrRetry = cookieCipher_->getTokenOrRetry(
              std::move(readBuf), IOBuf::copyBuffer("test"));
          auto retry =
              std::move(boost::get<StatelessHelloRetryRequest>(tokenOrRetry));
          server->writeChain(nullptr, std::move(retry.data));
        }));

    server->setReadCB(&serverRawRead);
    doClientHandshake();
    EXPECT_EQ(server->getReadCallback(), nullptr);

    server_.reset(new AsyncFizzServer(
        std::move(server), serverContext_, serverExtensions_));
  }

  void resetTransportsAndDoCookieHandshake() {
    resetTransportsAndStartCookieHandshake();
    doServerHandshake();
  }

  void startHandshake() {
    client_->connect(
        &clientCallback_,
        nullptr,
        folly::none,
        std::string("Fizz"),
        folly::Optional<std::vector<ech::ECHConfig>>(folly::none));
    server_->accept(&serverCallback_);
  }

  void doHandshake() {
    startHandshake();
    evb_.loop();
  }

  void doClientHandshake() {
    client_->connect(
        &clientCallback_,
        nullptr,
        folly::none,
        std::string("Fizz"),
        folly::Optional<std::vector<ech::ECHConfig>>(folly::none));
    evb_.loop();
  }

  // don't register a callback for connect/handshake
  void doClientHandshakeNullCallback() {
    // register read callback here because we don't call expectClientSuccess
    client_->setReadCB(&clientRead_);
    client_->connect(
        nullptr,
        nullptr,
        folly::none,
        std::string("Fizz"),
        folly::Optional<std::vector<ech::ECHConfig>>(folly::none));
    evb_.loop();
  }

  void doServerHandshake() {
    server_->accept(&serverCallback_);
    evb_.loop();
  }

  void expectClientSuccess() {
    EXPECT_CALL(clientCallback_, _fizzHandshakeSuccess())
        .WillOnce(Invoke([this]() {
          client_->setReadCB(&clientRead_);
          if (!client_->isReplaySafe()) {
            client_->setReplaySafetyCallback(&replayCallback_);
          }
        }));
    ON_CALL(clientCallback_, _fizzHandshakeError(_))
        .WillByDefault(Invoke([](folly::exception_wrapper ex) {
          FAIL() << "Client Error: " << ex.what().toStdString();
        }));
    ON_CALL(clientRead_, readErr_(_))
        .WillByDefault(Invoke([](const AsyncSocketException& ex) {
          FAIL() << "Client Read Error: " << ex.what();
        }));
  }

  void expectServerSuccess() {
    EXPECT_CALL(serverCallback_, _fizzHandshakeSuccess())
        .WillOnce(Invoke([this]() { server_->setReadCB(&serverRead_); }));
    ON_CALL(serverCallback_, _fizzHandshakeError(_))
        .WillByDefault(Invoke([](folly::exception_wrapper ex) {
          FAIL() << "Server Error: " << ex.what().toStdString();
        }));
    ON_CALL(serverRead_, readErr_(_))
        .WillByDefault(Invoke([](const AsyncSocketException& ex) {
          FAIL() << "Server Read Error: " << ex.what();
        }));
  }

  void expectSuccess() {
    expectClientSuccess();
    expectServerSuccess();
  }

  void expectError(const std::string& clientStr, const std::string& serverStr) {
    EXPECT_CALL(clientCallback_, _fizzHandshakeError(_))
        .WillOnce(Invoke([clientStr](folly::exception_wrapper ex) {
          EXPECT_THAT(ex.what().toStdString(), HasSubstr(clientStr));
        }));
    EXPECT_CALL(serverCallback_, _fizzHandshakeError(_))
        .WillOnce(Invoke([serverStr](folly::exception_wrapper ex) {
          EXPECT_THAT(ex.what().toStdString(), HasSubstr(serverStr));
        }));
  }

  void expectServerError(
      const std::string& clientError,
      const std::string& serverError) {
    EXPECT_CALL(clientCallback_, _fizzHandshakeSuccess());
    client_->setReadCB(&readCallback_);
    EXPECT_CALL(readCallback_, readErr_(_))
        .WillOnce(Invoke([clientError](const AsyncSocketException& ex) {
          EXPECT_THAT(std::string(ex.what()), HasSubstr(clientError));
        }));
    EXPECT_CALL(serverCallback_, _fizzHandshakeError(_))
        .WillOnce(Invoke([serverError](folly::exception_wrapper ex) {
          EXPECT_THAT(ex.what().toStdString(), HasSubstr(serverError));
        }));
  }

  void clientWrite(StringPiece write) {
    client_->writeChain(nullptr, IOBuf::copyBuffer(write));
  }

  void clientWriteWithCallback(StringPiece write) {
    client_->writeChain(&clientWriteCallback_, IOBuf::copyBuffer(write));
  }

  void serverWrite(StringPiece write) {
    server_->writeChain(nullptr, IOBuf::copyBuffer(write));
  }

  void expectClientRead(StringPiece read) {
    EXPECT_CALL(clientRead_, readBufferAvailable_(BufMatches(read)));
  }

  void expectServerRead(StringPiece read) {
    EXPECT_CALL(serverRead_, readBufferAvailable_(BufMatches(read)));
  }

  void expectEarlyDataRejectError() {
    EXPECT_CALL(clientRead_, readErr_(_))
        .WillOnce(Invoke([](const AsyncSocketException& ex) {
          EXPECT_EQ(ex.getType(), AsyncSocketException::EARLY_DATA_REJECTED);
        }));
  }

  void expectReplaySafety() {
    EXPECT_CALL(replayCallback_, onReplaySafe_());
  }

  void sendAppData() {
    expectClientRead("serverdata");
    expectServerRead("clientdata");
    clientWrite("clientdata");
    serverWrite("serverdata");
  }

  static bool certsMatch(
      const std::shared_ptr<const Cert>& a,
      const std::shared_ptr<const Cert>& b) {
    if (!a || !b) {
      return a == b;
    } else {
      return a->getIdentity() == b->getIdentity();
    }
  }

  void verifyEarlyParameters() {
    EXPECT_EQ(
        client_->getState().earlyDataParams()->version, expected_.version);
    EXPECT_EQ(client_->getState().earlyDataParams()->cipher, expected_.cipher);
    EXPECT_EQ(client_->getState().earlyDataParams()->alpn, expected_.alpn);
    EXPECT_TRUE(certsMatch(
        client_->getState().earlyDataParams()->clientCert,
        expected_.clientCert));
  }

  void verifyParameters() {
    EXPECT_EQ(*client_->getState().version(), expected_.version);
    EXPECT_EQ(*client_->getState().cipher(), expected_.cipher);
    EXPECT_EQ(client_->getState().sigScheme(), expected_.scheme);
    EXPECT_EQ(client_->getState().group(), expected_.group);
    EXPECT_EQ(*server_->getState().pskType(), expected_.pskType);
    EXPECT_EQ(client_->getState().pskMode(), expected_.pskMode);
    EXPECT_EQ(client_->getState().keyExchangeType(), expected_.clientKexType);
    EXPECT_EQ(client_->getState().earlyDataType(), expected_.earlyDataType);
    EXPECT_EQ(client_->getState().alpn(), expected_.alpn);
    EXPECT_TRUE(
        certsMatch(client_->getState().clientCert(), expected_.clientCert));

    EXPECT_EQ(*server_->getState().version(), expected_.version);
    EXPECT_EQ(*server_->getState().cipher(), expected_.cipher);
    EXPECT_EQ(server_->getState().sigScheme(), expected_.scheme);
    EXPECT_EQ(server_->getState().group(), expected_.group);
    EXPECT_EQ(*server_->getState().pskType(), expected_.pskType);
    EXPECT_EQ(server_->getState().pskMode(), expected_.pskMode);
    EXPECT_EQ(server_->getState().keyExchangeType(), expected_.serverKexType);
    EXPECT_EQ(server_->getState().earlyDataType(), expected_.earlyDataType);
    EXPECT_EQ(server_->getState().alpn(), expected_.alpn);
    EXPECT_TRUE(
        certsMatch(server_->getState().clientCert(), expected_.clientCert));
    EXPECT_EQ(
        client_->getState().serverCertCompAlgo(), expected_.serverCertCompAlgo);
    EXPECT_EQ(
        server_->getState().serverCertCompAlgo(), expected_.serverCertCompAlgo);
  }

  void setupResume() {
    expectSuccess();
    doHandshake();
    verifyParameters();
    resetTransports();
    expected_.scheme = none;
    expected_.pskType = PskType::Resumption;
    expected_.pskMode = PskKeyExchangeMode::psk_dhe_ke;
  }

  void setupResumeWithHRR() {
    serverContext_->setSupportedGroups({NamedGroup::secp256r1});
    expected_.group = NamedGroup::secp256r1;
    expectSuccess();
    doHandshake();
    verifyParameters();
    // Explicitly set a different supported group to trigger another
    // negotiation, even if group is cached
    serverContext_->setSupportedGroups({NamedGroup::x25519});
    expected_.group = NamedGroup::x25519;
    resetTransports();
    expected_.scheme = none;
    expected_.pskType = PskType::Resumption;
    expected_.pskMode = PskKeyExchangeMode::psk_dhe_ke;
  }

 protected:
  EventBase evb_;
  std::shared_ptr<FizzClientContext> clientContext_;
  std::shared_ptr<FizzServerContext> serverContext_;
  AsyncFizzClient::UniquePtr client_;
  AsyncFizzServer::UniquePtr server_;

  std::shared_ptr<AES128CookieCipher> cookieCipher_;

  fizz::client::test::MockHandshakeCallback clientCallback_;
  fizz::server::test::MockHandshakeCallback serverCallback_;

  folly::test::MockReadCallback readCallback_;
  folly::test::MockWriteCallback clientWriteCallback_;

  std::shared_ptr<fizz::ClientExtensions> clientExtensions_;
  std::shared_ptr<fizz::ServerExtensions> serverExtensions_;

  LocalTransport* clientTransport_;
  LocalTransport* serverTransport_;

  MockReadCallback clientRead_;
  MockReadCallback serverRead_;

  MockReplaySafetyCallback replayCallback_;

  ExpectedParameters expected_;
};
} // namespace test
} // namespace fizz
