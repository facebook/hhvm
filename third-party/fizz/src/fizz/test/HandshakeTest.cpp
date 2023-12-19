/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/test/HandshakeTest.h>

namespace fizz {
namespace test {

class SigSchemeTest : public HandshakeTest,
                      public ::testing::WithParamInterface<SignatureScheme> {};

TEST_F(HandshakeTest, BasicHandshake) {
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, BasicHandshakeSynchronous) {
  evb_.runInLoop([this]() { startHandshake(); });

  // Ensure that the handshake can complete synchronously (in the current loop
  // iteration).
  expectSuccess();
  evb_.loopOnce();
  verifyParameters();
}

TEST_F(HandshakeTest, BasicHandshakeInplaceDecrypt) {
  // Trickle + in-place means each buffer will hold 1 encrypted byte,
  // which will be where the decrypted byte also goes. Because of
  // trim(), there may be empty bufs in the chain, so countChainElements()
  // >= computeChainDataLength().
  client_->setDecryptInplace(true);
  server_->setDecryptInplace(true);
  clientTransport_->setTrickle(true);
  serverTransport_->setTrickle(true);
  expectSuccess();
  doHandshake();
  verifyParameters();
  EXPECT_CALL(clientRead_, readBufferAvailable_(BufMatches("serverdata")))
      .WillOnce(Invoke([](auto& buf) {
        EXPECT_GE(buf->countChainElements(), buf->computeChainDataLength());
        EXPECT_TRUE(buf->isShared());
      }));
  EXPECT_CALL(serverRead_, readBufferAvailable_(BufMatches("clientdata")))
      .WillOnce(Invoke([](auto& buf) {
        EXPECT_GE(buf->countChainElements(), buf->computeChainDataLength());
        EXPECT_TRUE(buf->isShared());
      }));
  clientWrite("clientdata");
  serverWrite("serverdata");
}

TEST_F(HandshakeTest, BasicHandshakeInplaceEncryptDisabled) {
  client_->setEncryptInplace(false);
  server_->setEncryptInplace(false);
  expectSuccess();
  doHandshake();
  verifyParameters();
  expectClientRead("serverdata");
  expectServerRead("clientdata");
  auto clientSendBuf = IOBuf::copyBuffer("clientdata");
  auto serverSendBuf = IOBuf::copyBuffer("serverdata");
  client_->writeChain(nullptr, clientSendBuf->clone());
  server_->writeChain(nullptr, serverSendBuf->clone());
  EXPECT_TRUE(IOBufEqualTo()(clientSendBuf, IOBuf::copyBuffer("clientdata")));
  EXPECT_TRUE(IOBufEqualTo()(serverSendBuf, IOBuf::copyBuffer("serverdata")));
}

TEST_F(HandshakeTest, BasicHandshakeInplaceEncryptEnabled) {
  client_->setEncryptInplace(true);
  server_->setEncryptInplace(true);
  expectSuccess();
  doHandshake();
  verifyParameters();
  expectClientRead("serverdata");
  expectServerRead("clientdata");
  auto clientSendBuf = IOBuf::copyBuffer("clientdata");
  auto serverSendBuf = IOBuf::copyBuffer("serverdata");
  client_->writeChain(nullptr, clientSendBuf->clone());
  server_->writeChain(nullptr, serverSendBuf->clone());
  EXPECT_FALSE(IOBufEqualTo()(clientSendBuf, IOBuf::copyBuffer("clientdata")));
  EXPECT_FALSE(IOBufEqualTo()(serverSendBuf, IOBuf::copyBuffer("serverdata")));
}

TEST_F(HandshakeTest, BasicHandshakeSharedDecrypt) {
  // Same as before, but now we ask it to respect shared policy.
  // Because of setTrickle trimming the incoming packet, we expect
  // that it will see a shared buffer and decrypt in a separate (unique)
  // buffer.
  client_->setDecryptInplace(false);
  server_->setDecryptInplace(false);
  clientTransport_->setTrickle(true);
  serverTransport_->setTrickle(true);
  expectSuccess();
  doHandshake();
  verifyParameters();
  EXPECT_CALL(clientRead_, readBufferAvailable_(BufMatches("serverdata")))
      .WillOnce(Invoke([](auto& buf) {
        EXPECT_FALSE(buf->isChained());
        EXPECT_FALSE(buf->isShared());
      }));
  EXPECT_CALL(serverRead_, readBufferAvailable_(BufMatches("clientdata")))
      .WillOnce(Invoke([](auto& buf) {
        EXPECT_FALSE(buf->isChained());
        EXPECT_FALSE(buf->isShared());
      }));
  clientWrite("clientdata");
  serverWrite("serverdata");
}

TEST_F(HandshakeTest, BasicHandshakeTrickle) {
  clientTransport_->setTrickle(true);
  serverTransport_->setTrickle(true);
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, P256) {
  clientContext_->setSupportedGroups(
      {NamedGroup::x25519, NamedGroup::secp256r1});
  clientContext_->setDefaultShares({NamedGroup::x25519, NamedGroup::secp256r1});
  serverContext_->setSupportedGroups({NamedGroup::secp256r1});
  expected_.group = NamedGroup::secp256r1;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, P384) {
  clientContext_->setSupportedGroups(
      {NamedGroup::x25519, NamedGroup::secp384r1});
  clientContext_->setDefaultShares({NamedGroup::x25519, NamedGroup::secp384r1});
  serverContext_->setSupportedGroups({NamedGroup::secp384r1});
  expected_.group = NamedGroup::secp384r1;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, P521) {
  clientContext_->setSupportedGroups(
      {NamedGroup::x25519, NamedGroup::secp521r1});
  clientContext_->setDefaultShares({NamedGroup::x25519, NamedGroup::secp521r1});
  serverContext_->setSupportedGroups({NamedGroup::secp521r1});
  expected_.group = NamedGroup::secp521r1;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, GroupServerPref) {
  clientContext_->setSupportedGroups(
      {NamedGroup::secp256r1, NamedGroup::x25519});
  clientContext_->setDefaultShares({NamedGroup::secp256r1, NamedGroup::x25519});
  serverContext_->setSupportedGroups(
      {NamedGroup::x25519, NamedGroup::secp256r1});
  expected_.group = NamedGroup::x25519;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, GroupMismatch) {
  clientContext_->setSupportedGroups({NamedGroup::secp256r1});
  clientContext_->setDefaultShares({NamedGroup::secp256r1});
  serverContext_->setSupportedGroups({NamedGroup::x25519});

  expectError("alert: handshake_failure", "no group match");
  doHandshake();
}

TEST_F(HandshakeTest, SchemeServerPref) {
  clientContext_->setSupportedSigSchemes(
      {SignatureScheme::ecdsa_secp256r1_sha256,
       SignatureScheme::rsa_pss_sha256});
  serverContext_->setSupportedSigSchemes(
      {SignatureScheme::rsa_pss_sha256,
       SignatureScheme::ecdsa_secp256r1_sha256});
  expected_.scheme = SignatureScheme::rsa_pss_sha256;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, SchemeMismatch) {
  clientContext_->setSupportedSigSchemes(
      {SignatureScheme::ecdsa_secp256r1_sha256});
  serverContext_->setSupportedSigSchemes({SignatureScheme::rsa_pss_sha256});

  // Server will fail to negotiate a cert match
  expectError("alert: handshake_failure", "could not find suitable cert");
  doHandshake();
}

TEST_F(HandshakeTest, HRR) {
  clientContext_->setDefaultShares({});
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, PskDheKe) {
  setupResume();

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, HrrPskDheKe) {
  clientContext_->setDefaultShares({});
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;
  setupResumeWithHRR();
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, HrrPskDheKeWithCache) {
  clientContext_->setDefaultShares({});
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;
  setupResume();

  // OneRtt as the first round should have cached the group
  expected_.clientKexType = expected_.serverKexType = KeyExchangeType::OneRtt;
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, HrrIncompatiblePsk) {
  expectSuccess();
  doHandshake();
  verifyParameters();
  resetTransports();

  serverContext_->setSupportedGroups({NamedGroup::secp256r1});
  serverContext_->setSupportedCiphers({{CipherSuite::TLS_AES_256_GCM_SHA384}});
  expected_.group = NamedGroup::secp256r1;
  expected_.cipher = CipherSuite::TLS_AES_256_GCM_SHA384;
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, PskKe) {
  serverContext_->setSupportedPskModes({PskKeyExchangeMode::psk_ke});
  setupResume();

  expected_.group = none;
  expected_.pskMode = PskKeyExchangeMode::psk_ke;
  expected_.clientKexType = expected_.serverKexType = KeyExchangeType::None;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

// This test is only run with 1.1.0 as it requires chacha to run (chacha and
// aes-gcm-128 are the only ciphers with a compatible hash algorithm).
#if FOLLY_OPENSSL_HAS_CHACHA
TEST_F(HandshakeTest, ResumeChangeCipher) {
  setupResume();
  clientContext_->setSupportedCiphers(
      {CipherSuite::TLS_AES_128_GCM_SHA256,
       CipherSuite::TLS_CHACHA20_POLY1305_SHA256});
  serverContext_->setSupportedCiphers(
      {{CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
       {CipherSuite::TLS_AES_128_GCM_SHA256}});

  expected_.cipher = CipherSuite::TLS_CHACHA20_POLY1305_SHA256;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}
#endif // FOLLY_OPENSSL_HAS_CHACHA

TEST_F(HandshakeTest, TestEkmSame) {
  expectSuccess();
  doHandshake();
  auto clientEkm =
      client_->getExportedKeyingMaterial("EXPORTER-Some-Label", nullptr, 32);
  auto serverEkm =
      server_->getExportedKeyingMaterial("EXPORTER-Some-Label", nullptr, 32);
  EXPECT_TRUE(IOBufEqualTo()(clientEkm, serverEkm));
  EXPECT_THROW(
      client_->getEarlyEkm("EXPORTER-Some-Label", nullptr, 32), std::exception);
  EXPECT_THROW(
      server_->getEarlyEkm("EXPORTER-Some-Label", nullptr, 32), std::exception);
}

TEST_F(HandshakeTest, TestEarlyEkmSame) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  expectSuccess();
  doHandshake();
  auto clientEkm = client_->getEarlyEkm("EXPORTER-Some-Label", nullptr, 32);
  auto serverEkm = server_->getEarlyEkm("EXPORTER-Some-Label", nullptr, 32);
  EXPECT_TRUE(IOBufEqualTo()(clientEkm, serverEkm));
}

TEST_F(HandshakeTest, TestExtensions) {
  auto context = std::make_shared<TokenBindingContext>();
  auto clientTokBind = std::make_shared<TokenBindingClientExtension>(context);
  auto serverTokBind = std::make_shared<TokenBindingServerExtension>(context);
  clientExtensions_ = clientTokBind;
  serverExtensions_ = serverTokBind;
  resetTransports();
  doHandshake();
  EXPECT_TRUE(clientTokBind->getNegotiatedKeyParam().has_value());
  EXPECT_TRUE(clientTokBind->getVersion().has_value());
  EXPECT_TRUE(serverTokBind->getNegotiatedKeyParam().has_value());
  EXPECT_EQ(
      *clientTokBind->getNegotiatedKeyParam(),
      TokenBindingKeyParameters::ecdsap256);
  EXPECT_EQ(
      *clientTokBind->getVersion(),
      TokenBindingProtocolVersion::token_binding_1_0);
  EXPECT_EQ(
      *serverTokBind->getNegotiatedKeyParam(),
      TokenBindingKeyParameters::ecdsap256);
}

TEST_F(HandshakeTest, BasicCertRequest) {
  expectSuccess();
  serverContext_->setClientAuthMode(ClientAuthMode::Required);
  expected_.clientCert = std::make_shared<PeerCertImpl<KeyType::RSA>>(
      getCert(kClientAuthClientCert));
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_P(SigSchemeTest, Schemes) {
  SignatureScheme scheme = GetParam();
  clientContext_->setSupportedSigSchemes({scheme});
  serverContext_->setSupportedSigSchemes({scheme});
  expected_.scheme = scheme;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, CertRequestPskPreservesIdentity) {
  serverContext_->setClientAuthMode(ClientAuthMode::Required);
  expected_.clientCert = std::make_shared<PeerCertImpl<KeyType::RSA>>(
      getCert(kClientAuthClientCert));
  setupResume();

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, CertRequestNoCert) {
  serverContext_->setClientAuthMode(ClientAuthMode::Required);
  clientContext_->setClientCertificate(nullptr);
  expectServerError(
      "alert: certificate_required", "certificate requested but none received");
  doHandshake();
}

TEST_F(HandshakeTest, CertRequestPermitNoCert) {
  serverContext_->setClientAuthMode(ClientAuthMode::Optional);
  clientContext_->setClientCertificate(nullptr);
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, CertRequestBadCert) {
  serverContext_->setClientAuthMode(ClientAuthMode::Required);
  auto badCert = createCert("foo", false, nullptr);
  std::vector<folly::ssl::X509UniquePtr> certVec;
  certVec.emplace_back(std::move(badCert.cert));
  clientContext_->setClientCertificate(
      std::make_shared<SelfCertImpl<KeyType::P256>>(
          std::move(badCert.key), std::move(certVec)));
  expectServerError("alert: bad_certificate", "client certificate failure");
  doHandshake();
}

TEST_F(HandshakeTest, BasicCertCompression) {
  expectSuccess();
  auto decompressor = std::make_shared<ZlibCertificateDecompressor>();
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  clientContext_->setCertDecompressionManager(decompressionMgr);
  serverContext_->setSupportedCompressionAlgorithms(
      {CertificateCompressionAlgorithm::zlib});
  expected_.serverCertCompAlgo = CertificateCompressionAlgorithm::zlib;
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, EarlyDataAccepted) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Accepted;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expectReplaySafety();
  expectServerSuccess();
  expectServerRead("early");
  doServerHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, EarlyDataRejected) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  serverContext_->setEarlyDataSettings(false, {}, nullptr);
  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Rejected;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expectEarlyDataRejectError();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
}

TEST_F(HandshakeTest, EarlyDataRejectedHrr) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  serverContext_->setSupportedGroups({NamedGroup::secp256r1});
  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Rejected;
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;
  expected_.group = NamedGroup::secp256r1;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expectEarlyDataRejectError();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
}

TEST_F(HandshakeTest, EarlyDataRejectedResend) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  serverContext_->setEarlyDataSettings(false, {}, nullptr);
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);
  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Rejected;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expectReplaySafety();
  expectServerRead("early");
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, EarlyDataRejectedDontResend) {
  clientContext_->setSendEarlyData(true);
  clientContext_->setSupportedAlpns({"h2"});
  serverContext_->setSupportedAlpns({"h2"});
  expected_.alpn = "h2";
  setupResume();

  serverContext_->setSupportedAlpns({});
  client_->setEarlyDataRejectionPolicy(
      EarlyDataRejectionPolicy::AutomaticResend);

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expected_.earlyDataType = EarlyDataType::Rejected;
  expected_.alpn = none;

  expectEarlyDataRejectError();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();

  expected_.pskType = PskType::NotAttempted;
  expected_.pskMode = none;
  expected_.scheme = SignatureScheme::ecdsa_secp256r1_sha256;
  expected_.earlyDataType = EarlyDataType::NotAttempted;

  resetTransports();
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, EarlyDataTrickleSendAccepted) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  clientTransport_->setTrickle(true, [this]() { clientWrite("e"); });
  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Accepted;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();

  expectReplaySafety();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
}

TEST_F(HandshakeTest, EarlyDataTrickleSendRejected) {
  clientContext_->setSendEarlyData(true);
  setupResume();

  clientTransport_->setTrickle(true, [this]() { clientWrite("e"); });
  serverContext_->setClientAuthMode(ClientAuthMode::Required);
  serverContext_->setTicketCipher(nullptr);

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();

  expected_.pskType = PskType::Rejected;
  expected_.pskMode = none;
  expected_.earlyDataType = EarlyDataType::Rejected;
  expected_.scheme = SignatureScheme::ecdsa_secp256r1_sha256;
  expected_.clientCert = std::make_shared<PeerCertImpl<KeyType::RSA>>(
      getCert(kClientAuthClientCert));

  expectEarlyDataRejectError();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
}

TEST_F(HandshakeTest, EarlyDataAcceptedOmitEarlyRecord) {
  clientContext_->setSendEarlyData(true);
  clientContext_->setOmitEarlyRecordLayer(true);
  serverContext_->setOmitEarlyRecordLayer(true);
  setupResume();

  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Accepted;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();

  expectReplaySafety();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
  sendAppData();
}

// Disables early data, initiates connection with a null callback, writes a
// couple strings that should be queued, successfully connects, and verifies
// whether queued messaged arrived on server
TEST_F(HandshakeTest, QueuePendingHandshake) {
  // disable early data
  clientContext_->setSendEarlyData(false);

  // connect with a null callback
  doClientHandshakeNullCallback();

  clientWrite("queued");
  clientWrite("up");

  expectServerSuccess();
  expectServerRead("queued");
  expectServerRead("up");
  doServerHandshake();

  // rely on defaults
  verifyParameters();
  // send some more, just in case
  sendAppData();
}

// Disables early data, initiates connection with a null callback, writes a
// couple strings that should be queued, fails to handshake, and verifies queued
// writes receive error callbacks
TEST_F(HandshakeTest, ClearPendingHandshakeQueueOnError) {
  // disable early data
  clientContext_->setSendEarlyData(false);

  // set up a failure during handshake
  clientContext_->setSupportedGroups({NamedGroup::secp256r1});
  clientContext_->setDefaultShares({NamedGroup::secp256r1});
  serverContext_->setSupportedGroups({NamedGroup::x25519});

  // connect with a null callback
  doClientHandshakeNullCallback();

  // enqueue one write with callback and one without
  clientWriteWithCallback("queued");
  clientWrite("up");

  // server handshake callback should get a failure
  EXPECT_CALL(serverCallback_, _fizzHandshakeError(_))
      .WillOnce(Invoke([](folly::exception_wrapper ex) {
        EXPECT_THAT(ex.what().toStdString(), HasSubstr("no group match"));
      }));
  ON_CALL(serverCallback_, _fizzHandshakeSuccess()).WillByDefault(Invoke([]() {
    FAIL() << "Server should not have succeeded its handshake.";
  }));

  // error callback will occur once handshake fails
  EXPECT_CALL(clientWriteCallback_, writeErr_(_, _))
      .WillOnce(Invoke([](size_t bytesWritten, folly::exception_wrapper ex) {
        EXPECT_THAT(bytesWritten, Eq(0));
        EXPECT_THAT(
            ex.what().toStdString(), HasSubstr("alert: handshake_failure"));
      }));

  doServerHandshake();
}

TEST_F(HandshakeTest, Compat) {
  clientContext_->setCompatibilityMode(true);
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCompatHRR) {
  clientContext_->setCompatibilityMode(true);
  clientContext_->setDefaultShares({});
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCompatEarly) {
  clientContext_->setCompatibilityMode(true);
  clientContext_->setSendEarlyData(true);
  setupResume();

  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Accepted;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();

  expectReplaySafety();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCompatEarlyRejected) {
  clientContext_->setCompatibilityMode(true);
  clientContext_->setSendEarlyData(true);
  setupResume();

  serverContext_->setEarlyDataSettings(false, {}, nullptr);
  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Rejected;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expectEarlyDataRejectError();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
}

TEST_F(HandshakeTest, TestCompatEarlyRejectedHRR) {
  clientContext_->setCompatibilityMode(true);
  clientContext_->setSendEarlyData(true);
  setupResume();

  serverContext_->setSupportedGroups({NamedGroup::secp256r1});
  expected_.pskType = PskType::Resumption;
  expected_.earlyDataType = EarlyDataType::Rejected;
  expected_.clientKexType = expected_.serverKexType =
      KeyExchangeType::HelloRetryRequest;
  expected_.group = NamedGroup::secp256r1;

  expectClientSuccess();
  doClientHandshake();
  verifyEarlyParameters();
  clientWrite("early");

  expectEarlyDataRejectError();
  expectServerSuccess();
  doServerHandshake();
  verifyParameters();
}

TEST_F(HandshakeTest, TestCookie) {
  expected_.clientKexType = KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  resetTransportsAndDoCookieHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCookieGroupNegotiate) {
  clientContext_->setDefaultShares({});
  expected_.clientKexType = KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  resetTransportsAndDoCookieHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCookieResume) {
  setupResume();

  expected_.clientKexType = KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  resetTransportsAndDoCookieHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCookieIncompatiblePsk) {
  expectSuccess();
  doHandshake();
  verifyParameters();

  serverContext_->setSupportedCiphers({{CipherSuite::TLS_AES_256_GCM_SHA384}});
  expected_.cipher = CipherSuite::TLS_AES_256_GCM_SHA384;
  expected_.clientKexType = KeyExchangeType::HelloRetryRequest;

  expectSuccess();
  resetTransportsAndDoCookieHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestCookiePskKe) {
  serverContext_->setSupportedPskModes({PskKeyExchangeMode::psk_ke});
  setupResume();

  expected_.group = none;
  expected_.pskMode = PskKeyExchangeMode::psk_ke;
  expected_.clientKexType = KeyExchangeType::None;
  expected_.serverKexType = KeyExchangeType::None;

  expectSuccess();
  resetTransportsAndDoCookieHandshake();
  verifyParameters();
  sendAppData();
}

TEST_F(HandshakeTest, TestBadCookie) {
  expectError("decrypt_error", "could not decrypt cookie");
  resetTransportsAndStartCookieHandshake();

  auto cookieSeed = RandomGenerator<32>().generateRandom();
  cookieCipher_->setCookieSecrets({{range(cookieSeed)}});

  doServerHandshake();
}

class KeepTransportAliveCallback : public AsyncFizzBase::EndOfTLSCallback {
 public:
  bool isCalled() {
    return called;
  }

 private:
  void endOfTLS(
      AsyncFizzBase* /* unused */,
      std::unique_ptr<folly::IOBuf> /* unused */) override {
    called = true;
  }

  bool called{false};
};

TEST_F(HandshakeTest, TestTransportAliveAfterTLSShutdownByServer) {
  KeepTransportAliveCallback clientKCB;
  KeepTransportAliveCallback serverKCB;
  client_->setEndOfTLSCallback(&clientKCB);
  server_->setEndOfTLSCallback(&serverKCB);
  expectSuccess();
  doHandshake();

  server_->tlsShutdown();
  EXPECT_TRUE(clientKCB.isCalled());
  EXPECT_TRUE(serverKCB.isCalled());

  folly::test::MockReadCallback readCallback;
  ON_CALL(readCallback, isBufferMovable_()).WillByDefault(Return(true));
  serverTransport_->setReadCB(&readCallback);

  EXPECT_CALL(readCallback, readBufferAvailable_(BufMatches("foo")));
  clientTransport_->writeChain(nullptr, IOBuf::copyBuffer("foo"));
}

class MockAsyncFizzClient : public AsyncFizzClient {
 public:
  MockAsyncFizzClient(
      folly::AsyncTransportWrapper::UniquePtr socket,
      std::shared_ptr<const FizzClientContext> fizzContext,
      const std::shared_ptr<ClientExtensions>& extensions)
      : AsyncFizzClient(std::move(socket), std::move(fizzContext), extensions) {
  }
  MOCK_METHOD(bool, connecting, (), (const));
};

TEST_F(HandshakeTest, TestFailureOnInvalidCloseNotify) {
  auto client = LocalTransport::UniquePtr(new LocalTransport());
  auto server = LocalTransport::UniquePtr(new LocalTransport());
  client->attachEventBase(&evb_);
  server->attachEventBase(&evb_);
  client->setPeer(server.get());
  server->setPeer(client.get());

  auto fizzClient = new MockAsyncFizzClient(
      std::move(client), clientContext_, clientExtensions_);
  server_.reset(new AsyncFizzServer(
      std::move(server), serverContext_, serverExtensions_));
  client_.reset(fizzClient);

  KeepTransportAliveCallback clientKCB;
  KeepTransportAliveCallback serverKCB;
  fizzClient->setEndOfTLSCallback(&clientKCB);
  server_->setEndOfTLSCallback(&serverKCB);

  expectSuccess();
  doHandshake();

  ON_CALL(*fizzClient, connecting()).WillByDefault(Return(true));
  // error callback will occur once client sees incorrect close notify
  EXPECT_CALL(clientRead_, readErr_(_))
      .WillOnce(Invoke([](folly::exception_wrapper ex) {
        EXPECT_THAT(
            ex.what().toStdString(), HasSubstr("tls connection torn down"));
      }));
  server_->tlsShutdown();

  // EndOfTLS returns early with an error so this is never called
  EXPECT_FALSE(clientKCB.isCalled());
  // This can't actually happen in reality as there's no way the server should
  // be reading !connecting as true while the client sees it as false
  EXPECT_TRUE(serverKCB.isCalled());
}

TEST_F(HandshakeTest, SendKeyUpdate) {
  expectSuccess();
  doHandshake();
  verifyParameters();
  sendAppData();
  // Also check the change of the secrets
  auto prevServerSec = server_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ServerAppTraffic);
  auto prevClientSec = client_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ClientAppTraffic);
  client_->initiateKeyUpdate(KeyUpdateRequest::update_not_requested);
  sendAppData();
  EXPECT_EQ(
      prevServerSec,
      server_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ServerAppTraffic));
  EXPECT_NE(
      prevClientSec,
      client_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ClientAppTraffic));

  prevServerSec = server_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ServerAppTraffic);
  prevClientSec = client_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ClientAppTraffic);
  server_->initiateKeyUpdate(KeyUpdateRequest::update_requested);
  sendAppData();
  EXPECT_NE(
      prevServerSec,
      server_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ServerAppTraffic));
  EXPECT_NE(
      prevClientSec,
      client_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ClientAppTraffic));
}

TEST_F(HandshakeTest, FuzzSendKeyUpdate) {
  expectSuccess();
  doHandshake();
  verifyParameters();
  for (int i = 0; i < 10; i++) {
    auto r = RandomGenerator<1>().generateRandom();
    if (r[0] < 100) {
      client_->initiateKeyUpdate(
          r[0] < 50 ? KeyUpdateRequest::update_requested
                    : KeyUpdateRequest::update_not_requested);
    } else if (r[0] >= 100 && r[0] < 200) {
      server_->initiateKeyUpdate(
          r[0] < 150 ? KeyUpdateRequest::update_requested
                     : KeyUpdateRequest::update_not_requested);
    } else {
      sendAppData();
    }
  }
}

TEST_F(HandshakeTest, ClientInitiateKeyUpdate) {
  expectSuccess();
  doHandshake();
  verifyParameters();

  // Client should perform a key update when bytes written exceeds threshold
  auto prevClientSec = client_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ClientAppTraffic);
  size_t rekeyThreshold = 20;
  client_->setRekeyAfterWriting(rekeyThreshold);
  auto iobuf = IOBuf::create(rekeyThreshold + 5);
  iobuf->append(rekeyThreshold + 5);
  auto writeExceedsThreshold = StringPiece(iobuf->coalesce());
  expectServerRead(writeExceedsThreshold);
  clientWrite(writeExceedsThreshold);
  EXPECT_NE(
      prevClientSec,
      client_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ClientAppTraffic));

  // If processed data does not exceed threshold, key update should not happen
  prevClientSec = client_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ClientAppTraffic);
  expectServerRead(" ");
  clientWrite(" ");
  EXPECT_EQ(
      prevClientSec,
      client_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ClientAppTraffic));
}

TEST_F(HandshakeTest, ServerInitiateKeyUpdate) {
  expectSuccess();
  doHandshake();
  verifyParameters();

  // Server should perform a key update when bytes written exceeds threshold
  auto prevServerSec = server_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ServerAppTraffic);
  size_t rekeyThreshold = 20;
  server_->setRekeyAfterWriting(rekeyThreshold);
  auto iobuf = IOBuf::create(rekeyThreshold + 5);
  iobuf->append(rekeyThreshold + 5);
  auto writeExceedsThreshold = StringPiece(iobuf->coalesce());
  expectClientRead(writeExceedsThreshold);
  serverWrite(writeExceedsThreshold);
  EXPECT_NE(
      prevServerSec,
      server_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ServerAppTraffic));

  // If processed data does not exceed threshold, key update should not happen
  prevServerSec = server_->getState().keyScheduler()->getSecret(
      AppTrafficSecrets::ServerAppTraffic);
  expectClientRead(" ");
  serverWrite(" ");
  EXPECT_EQ(
      prevServerSec,
      server_->getState().keyScheduler()->getSecret(
          AppTrafficSecrets::ServerAppTraffic));
}

INSTANTIATE_TEST_SUITE_P(
    SignatureSchemes,
    SigSchemeTest,
    ::testing::Values(
        SignatureScheme::rsa_pss_sha256,
        SignatureScheme::ecdsa_secp256r1_sha256,
        SignatureScheme::ecdsa_secp384r1_sha384,
        SignatureScheme::ecdsa_secp521r1_sha512));
} // namespace test
} // namespace fizz
