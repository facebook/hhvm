/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/ClientProtocol.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/client/test/Mocks.h>
#include <fizz/client/test/Utilities.h>
#include <fizz/compression/test/Mocks.h>
#include <fizz/crypto/hpke/Utils.h>
#include <fizz/crypto/hpke/test/Mocks.h>
#include <fizz/protocol/clock/test/Mocks.h>
#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/test/TestUtil.h>
#include <fizz/protocol/test/Matchers.h>
#include <fizz/protocol/test/ProtocolTest.h>
#include <fizz/protocol/test/TestMessages.h>
#include <fizz/record/test/Mocks.h>

using namespace fizz::test;

namespace fizz {
namespace client {
namespace test {

class ClientProtocolTest : public ProtocolTest<ClientTypes, Actions> {
 public:
  void SetUp() override {
    context_ = std::make_shared<FizzClientContext>();
    context_->setSupportedVersions({ProtocolVersion::tls_1_3});
    context_->setSupportedCiphers(
        {CipherSuite::TLS_AES_128_GCM_SHA256,
         CipherSuite::TLS_AES_256_GCM_SHA384});
    context_->setSupportedSigSchemes(
        {SignatureScheme::ecdsa_secp256r1_sha256,
         SignatureScheme::rsa_pss_sha256});
    auto mockFactory = std::make_unique<MockFactory>();
    mockFactory->setDefaults();
    factory_ = mockFactory.get();
    context_->setFactory(std::move(mockFactory));
    context_->setSupportedAlpns({"h2"});
    verifier_ = std::make_shared<MockCertificateVerifier>();
    pskCache_ = std::make_shared<MockPskCache>();
    context_->setPskCache(pskCache_);
    context_->setSendEarlyData(true);
    mockLeaf_ = std::make_shared<MockPeerCert>();
    mockClientCert_ = std::make_shared<MockSelfCert>();
    mockClock_ = std::make_shared<MockClock>();
    context_->setClock(mockClock_);
    ON_CALL(*mockClock_, getCurrentTime())
        .WillByDefault(Return(
            std::chrono::system_clock::time_point(std::chrono::minutes(5))));
  }

 protected:
  void setMockRecord() {
    mockRead_ = new MockPlaintextReadRecordLayer();
    mockWrite_ = new MockPlaintextWriteRecordLayer();
    mockWrite_->setDefaults();
    state_.readRecordLayer().reset(mockRead_);
    state_.writeRecordLayer().reset(mockWrite_);
  }

  void setMockHandshakeEncryptedRecord() {
    mockHandshakeRead_ =
        new MockEncryptedReadRecordLayer(EncryptionLevel::Handshake);
    mockHandshakeWrite_ =
        new MockEncryptedWriteRecordLayer(EncryptionLevel::Handshake);
    mockHandshakeWrite_->setDefaults();
    state_.readRecordLayer().reset(mockHandshakeRead_);
    state_.writeRecordLayer().reset(mockHandshakeWrite_);
  }

  void setMockEarlyRecord() {
    mockEarlyWrite_ =
        new MockEncryptedWriteRecordLayer(EncryptionLevel::EarlyData);
    mockEarlyWrite_->setDefaults();
    state_.earlyWriteRecordLayer().reset(mockEarlyWrite_);
  }

  void setMockContextAndScheduler() {
    auto handshakeContext = std::make_unique<MockHandshakeContext>();
    mockHandshakeContext_ = handshakeContext.get();
    mockHandshakeContext_->setDefaults();
    state_.handshakeContext() = std::move(handshakeContext);
    auto keyScheduler = std::make_unique<MockKeyScheduler>();
    mockKeyScheduler_ = keyScheduler.get();
    mockKeyScheduler_->setDefaults();
    state_.keyScheduler() = std::move(keyScheduler);
  }

  static ClientHello getDefaultClientHello() {
    return TestMessages::clientHello();
  }

  CachedPsk getCachedPsk() {
    CachedPsk psk = getTestPsk(
        "PSK", std::chrono::system_clock::time_point(std::chrono::minutes(5)));
    psk.serverCert = mockLeaf_;
    psk.clientCert = mockClientCert_;
    return psk;
  }

  EarlyDataParams getEarlyDataParams() {
    EarlyDataParams params;
    params.version = TestProtocolVersion;
    params.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
    params.serverCert = mockLeaf_;
    params.clientCert = mockClientCert_;
    params.alpn = "h2";
    params.earlyExporterSecret = folly::IOBuf::copyBuffer("earlyexporter");
    return params;
  }

  void setupExpectingServerHello() {
    setMockRecord();
    state_.context() = context_;
    state_.encodedClientHello() = folly::IOBuf::copyBuffer("chlo");
    auto mockKex = std::make_unique<MockKeyExchange>();
    mockKex_ = mockKex.get();
    mockKex_->setDefaults();
    std::map<NamedGroup, std::unique_ptr<KeyExchange>> kexs;
    kexs.emplace(NamedGroup::x25519, std::move(mockKex));
    state_.keyExchangers() = std::move(kexs);
    Random random;
    random.fill(0x44);
    state_.clientRandom() = std::move(random);
    state_.legacySessionId() = folly::IOBuf::create(0);
    state_.sni() = "www.hostname.com";
    state_.verifier() = verifier_;
    state_.earlyDataType() = EarlyDataType::NotAttempted;
    state_.state() = StateEnum::ExpectingServerHello;
    state_.requestedExtensions() = std::vector<ExtensionType>(
        {ExtensionType::supported_versions,
         ExtensionType::key_share,
         ExtensionType::server_name,
         ExtensionType::application_layer_protocol_negotiation,
         ExtensionType::pre_shared_key,
         ExtensionType::early_data});
  }

  void setupExpectingServerHelloAfterHrr() {
    setupExpectingServerHello();
    state_.keyExchangeType() = KeyExchangeType::HelloRetryRequest;
    auto handshakeContext = std::make_unique<MockHandshakeContext>();
    mockHandshakeContext_ = handshakeContext.get();
    mockHandshakeContext_->setDefaults();
    state_.handshakeContext() = std::move(handshakeContext);
    state_.version() = TestProtocolVersion;
    state_.cipher() = CipherSuite::TLS_AES_128_GCM_SHA256;
    state_.group() = NamedGroup::x25519;
    state_.requestedExtensions() = std::vector<ExtensionType>(
        {ExtensionType::supported_versions,
         ExtensionType::key_share,
         ExtensionType::server_name,
         ExtensionType::application_layer_protocol_negotiation,
         ExtensionType::pre_shared_key,
         ExtensionType::early_data});
  }

  void setupExpectingEncryptedExtensions() {
    setMockRecord();
    setMockContextAndScheduler();
    state_.context() = context_;
    state_.earlyDataType() = EarlyDataType::NotAttempted;
    state_.version() = TestProtocolVersion;
    state_.cipher() = CipherSuite::TLS_AES_128_GCM_SHA256;
    state_.state() = StateEnum::ExpectingEncryptedExtensions;
    state_.requestedExtensions() = std::vector<ExtensionType>(
        {ExtensionType::supported_versions,
         ExtensionType::key_share,
         ExtensionType::server_name,
         ExtensionType::application_layer_protocol_negotiation,
         ExtensionType::pre_shared_key,
         ExtensionType::early_data});
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void setupExpectingEncryptedExtensionsEarlySent() {
    setupExpectingEncryptedExtensions();
    setMockEarlyRecord();
    state_.attemptedPsk() = getCachedPsk();
    state_.pskType() = PskType::Resumption;
    state_.handshakeTime() =
        std::chrono::system_clock::now() - std::chrono::hours(1);
    state_.earlyDataType() = EarlyDataType::Attempted;
    state_.earlyDataParams() = getEarlyDataParams();
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void setupExpectingCertificate() {
    setMockRecord();
    setMockContextAndScheduler();
    state_.context() = context_;
    state_.state() = StateEnum::ExpectingCertificate;
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void setupExpectingCertificateRequest() {
    setMockRecord();
    setMockContextAndScheduler();
    context_->setClientCertificate(mockClientCert_);
    state_.context() = context_;
    state_.state() = StateEnum::ExpectingCertificate;
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void setupExpectingCertificateVerify() {
    setMockRecord();
    setMockContextAndScheduler();
    state_.context() = context_;
    state_.verifier() = verifier_;
    mockIntermediate_ = std::make_shared<MockPeerCert>();
    std::vector<std::shared_ptr<const PeerCert>> certs;
    certs.push_back(mockLeaf_);
    certs.push_back(mockIntermediate_);
    state_.unverifiedCertChain() = std::move(certs);
    state_.state() = StateEnum::ExpectingCertificateVerify;
    state_.clientAuthRequested() = ClientAuthType::NotRequested;
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void setupExpectingFinished() {
    setMockHandshakeEncryptedRecord();
    setMockContextAndScheduler();
    state_.context() = context_;
    state_.version() = TestProtocolVersion;
    state_.cipher() = CipherSuite::TLS_AES_128_GCM_SHA256;
    state_.group() = NamedGroup::x25519;
    state_.clientHandshakeSecret() = folly::IOBuf::copyBuffer("cht");
    state_.serverHandshakeSecret() = folly::IOBuf::copyBuffer("sht");
    state_.state() = StateEnum::ExpectingFinished;
    state_.clientAuthRequested() = ClientAuthType::NotRequested;
    state_.earlyDataType() = EarlyDataType::NotAttempted;
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void setupAcceptingData() {
    setMockRecord();
    setMockContextAndScheduler();
    state_.version() = ProtocolVersion::tls_1_3;
    state_.cipher() = CipherSuite::TLS_AES_128_GCM_SHA256;
    state_.group() = NamedGroup::x25519;
    state_.context() = context_;
    state_.state() = StateEnum::Established;
    state_.resumptionSecret() = folly::IOBuf::copyBuffer("resumptionsecret");
    state_.sni() = "www.hostname.com";
    state_.serverCert() = mockLeaf_;
    state_.handshakeTime() =
        std::chrono::system_clock::time_point(std::chrono::minutes(4));
  }

  void doFinishedFlow(ClientAuthType authType);

  std::shared_ptr<FizzClientContext> context_;
  MockPlaintextReadRecordLayer* mockRead_;
  MockPlaintextWriteRecordLayer* mockWrite_;
  MockEncryptedWriteRecordLayer* mockEarlyWrite_;
  MockEncryptedWriteRecordLayer* mockHandshakeWrite_;
  MockEncryptedReadRecordLayer* mockHandshakeRead_;
  MockKeyExchange* mockKex_;
  MockKeyScheduler* mockKeyScheduler_;
  MockHandshakeContext* mockHandshakeContext_;
  std::shared_ptr<MockPeerCert> mockLeaf_;
  std::shared_ptr<MockPeerCert> mockIntermediate_;
  std::shared_ptr<MockSelfCert> mockClientCert_;
  std::shared_ptr<MockCertificateVerifier> verifier_;
  std::shared_ptr<MockPskCache> pskCache_;
  std::shared_ptr<MockClock> mockClock_;
};

TEST_F(ClientProtocolTest, TestInvalidTransitionNoAlert) {
  auto actions = ClientStateMachine().processAppWrite(state_, AppWrite());
  expectError<FizzException>(actions, folly::none, "invalid event");
}

TEST_F(ClientProtocolTest, TestInvalidWriteNewSessionTicket) {
  auto actions = ClientStateMachine().processWriteNewSessionTicket(
      state_, WriteNewSessionTicket());
  expectError<FizzException>(actions, folly::none, "invalid event");
}

TEST_F(ClientProtocolTest, TestInvalidTransitionAlert) {
  setMockRecord();
  EXPECT_CALL(*mockWrite_, _write(_, _));
  auto actions = ClientStateMachine().processAppWrite(state_, AppWrite());
  expectError<FizzException>(
      actions, AlertDescription::unexpected_message, "invalid event");
}

TEST_F(ClientProtocolTest, TestInvalidTransitionError) {
  state_.state() = StateEnum::Error;
  auto actions = ClientStateMachine().processAppWrite(state_, AppWrite());
  expectActions<ReportError>(actions);
}

TEST_F(ClientProtocolTest, TestAlertEncryptionLevel) {
  setMockRecord();
  EXPECT_CALL(*mockWrite_, _write(_, _));
  auto encryptionLevel = state_.writeRecordLayer()->getEncryptionLevel();
  auto actions = ClientStateMachine().processAppWrite(state_, AppWrite());
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents.size(), 1);
  EXPECT_EQ(write.contents[0].encryptionLevel, encryptionLevel);
  EXPECT_EQ(write.contents[0].contentType, ContentType::alert);
}

TEST_F(ClientProtocolTest, TestConnectFlow) {
  EXPECT_CALL(*factory_, makePlaintextReadRecordLayer())
      .WillOnce(Invoke([this]() {
        auto ret = std::make_unique<MockPlaintextReadRecordLayer>();
        mockRead_ = ret.get();
        return ret;
      }));
  EXPECT_CALL(*factory_, makePlaintextWriteRecordLayer())
      .WillOnce(Invoke([this]() {
        auto ret = std::make_unique<MockPlaintextWriteRecordLayer>();
        mockWrite_ = ret.get();
        EXPECT_CALL(*ret, _writeInitialClientHello(_))
            .WillOnce(Invoke([](Buf& encoded) {
              EXPECT_TRUE(folly::IOBufEqualTo()(
                  encoded, encodeHandshake(getDefaultClientHello())));
              TLSContent record;
              record.contentType = ContentType::handshake;
              record.encryptionLevel = EncryptionLevel::Plaintext;
              record.data = folly::IOBuf::copyBuffer("writtenchlo");
              return record;
            }));
        return ret;
      }));
  EXPECT_CALL(*factory_, makeRandom()).WillOnce(Invoke([]() {
    Random random;
    random.fill(0x44);
    return random;
  }));
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::x25519, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));

  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  connect.verifier = verifier_;
  auto actions = detail::processEvent(state_, std::move(connect));

  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenchlo")));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.encodedClientHello(), encodeHandshake(getDefaultClientHello())));
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::x25519).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  Random random;
  random.fill(0x44);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::NotAttempted);
  EXPECT_EQ(state_.earlyWriteRecordLayer().get(), nullptr);
  EXPECT_FALSE(state_.earlyDataParams().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskFlow) {
  auto psk = getCachedPsk();
  EXPECT_CALL(*factory_, makePlaintextReadRecordLayer())
      .WillOnce(Invoke([this]() {
        auto ret = std::make_unique<MockPlaintextReadRecordLayer>();
        mockRead_ = ret.get();
        return ret;
      }));
  EXPECT_CALL(*factory_, makePlaintextWriteRecordLayer())
      .WillOnce(Invoke([this]() {
        auto ret = std::make_unique<MockPlaintextWriteRecordLayer>();
        mockWrite_ = ret.get();
        EXPECT_CALL(*ret, _writeInitialClientHello(_))
            .WillOnce(Invoke([](Buf&) {
              TLSContent record;
              record.contentType = ContentType::handshake;
              record.encryptionLevel = EncryptionLevel::Plaintext;
              record.data = folly::IOBuf::copyBuffer("writtenchlo");
              return record;
            }));
        return ret;
      }));
  EXPECT_CALL(*factory_, makeRandom()).WillOnce(Invoke([]() {
    Random random;
    random.fill(0x44);
    return random;
  }));
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::x25519, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  Sequence contextSeq;
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveEarlySecret(RangeMatches("resumptionsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(EarlySecrets::ResumptionPskBinder, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'b', 'k'}),
            EarlySecrets::ResumptionPskBinder);
      }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("bk")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("binder"); }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);

  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  connect.cachedPsk = psk;
  connect.verifier = verifier_;
  auto actions = detail::processEvent(state_, std::move(connect));

  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenchlo")));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::x25519).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  Random random;
  random.fill(0x44);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_TRUE(state_.legacySessionId().value()->empty());
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::NotAttempted);
  EXPECT_EQ(state_.earlyWriteRecordLayer().get(), nullptr);
  EXPECT_FALSE(state_.earlyDataParams().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskEarlyFlow) {
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 9000;
  EXPECT_CALL(*factory_, makePlaintextReadRecordLayer())
      .WillOnce(Invoke([this]() {
        auto ret = std::make_unique<MockPlaintextReadRecordLayer>();
        mockRead_ = ret.get();
        return ret;
      }));
  EXPECT_CALL(*factory_, makePlaintextWriteRecordLayer())
      .WillOnce(Invoke([this]() {
        auto ret = std::make_unique<MockPlaintextWriteRecordLayer>();
        mockWrite_ = ret.get();
        EXPECT_CALL(*ret, _writeInitialClientHello(_))
            .WillOnce(Invoke([](Buf&) {
              TLSContent record;
              record.contentType = ContentType::handshake;
              record.encryptionLevel = EncryptionLevel::Plaintext;
              record.data = folly::IOBuf::copyBuffer("writtenchlo");
              return record;
            }));
        return ret;
      }));
  EXPECT_CALL(*factory_, makeRandom()).WillOnce(Invoke([]() {
    Random random;
    random.fill(0x44);
    return random;
  }));
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::x25519, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  Sequence contextSeq;
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveEarlySecret(RangeMatches("resumptionsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(EarlySecrets::ResumptionPskBinder, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'b', 'k'}),
            EarlySecrets::ResumptionPskBinder);
      }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("bk")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("binder"); }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .WillRepeatedly(
          InvokeWithoutArgs([]() { return folly::IOBuf::copyBuffer("chlo"); }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(EarlySecrets::EarlyExporter, RangeMatches("chlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'e', 'e'}), EarlySecrets::EarlyExporter);
      }));

  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(EarlySecrets::ClientEarlyTraffic, RangeMatches("chlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'e', 't'}),
            EarlySecrets::ClientEarlyTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cet"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientearlykey"),
            folly::IOBuf::copyBuffer("clientearlyiv")};
      }));
  MockAead* earlyAead;
  MockEncryptedWriteRecordLayer* earlyRecordLayer;
  expectAeadCreation({{"clientearlykey", &earlyAead}});
  expectEncryptedWriteRecordLayerCreation(
      &earlyRecordLayer, &earlyAead, folly::StringPiece("cet"));

  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  connect.cachedPsk = psk;
  connect.verifier = verifier_;
  auto actions = detail::processEvent(state_, std::move(connect));

  expectActions<
      MutateState,
      WriteToSocket,
      ReportEarlyHandshakeSuccess,
      SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenchlo")));
  auto earlySuccess = expectAction<ReportEarlyHandshakeSuccess>(actions);
  EXPECT_EQ(earlySuccess.maxEarlyDataSize, 9000);
  expectSecret(
      actions, EarlySecrets::ClientEarlyTraffic, folly::StringPiece("cet"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::x25519).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  Random random;
  random.fill(0x44);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_TRUE(state_.legacySessionId().value()->empty());
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Attempted);
  EXPECT_EQ(state_.earlyWriteRecordLayer().get(), earlyRecordLayer);
  EXPECT_EQ(state_.earlyDataParams()->version, TestProtocolVersion);
  EXPECT_EQ(
      state_.earlyDataParams()->cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.earlyDataParams()->serverCert, mockLeaf_);
  EXPECT_EQ(state_.earlyDataParams()->clientCert, mockClientCert_);
  EXPECT_EQ(state_.earlyDataParams()->alpn, "h2");
  EXPECT_TRUE(folly::IOBufEqualTo()(
      state_.earlyDataParams()->earlyExporterSecret,
      folly::IOBuf::copyBuffer("ee")));
}

TEST_F(ClientProtocolTest, TestConnectNoHostNoPsk) {
  Connect connect;
  connect.context = context_;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskStaleHandshake) {
  Connect connect;
  connect.context = context_;
  connect.cachedPsk = getCachedPsk();
  std::string sni = "www.hostname.com";
  connect.sni = sni;
  context_->setMaxPskHandshakeLife(std::chrono::seconds(1));
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskFutureHandshake) {
  Connect connect;
  connect.context = context_;
  connect.cachedPsk = getTestPsk(
      "PSK", std::chrono::system_clock::time_point(std::chrono::hours(86400)));
  connect.cachedPsk->serverCert = mockLeaf_;
  connect.cachedPsk->clientCert = mockClientCert_;
  std::string sni = "www.hostname.com";
  connect.sni = sni;
  context_->setMaxPskHandshakeLife(std::chrono::seconds(1));
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskBadVersion) {
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.version = ProtocolVersion::tls_1_2;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskBadCipher) {
  context_->setSupportedCiphers({CipherSuite::TLS_AES_128_GCM_SHA256});
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.cipher = CipherSuite::TLS_AES_256_GCM_SHA384;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
}

TEST_F(ClientProtocolTest, TestConnectSeparatePskIdentity) {
  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  auto psk = getCachedPsk();
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
}

TEST_F(ClientProtocolTest, TestConnectPskIdentityWithoutSni) {
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
  EXPECT_FALSE(state_.sni().has_value());
}

TEST_F(ClientProtocolTest, TestConnectNoSni) {
  Connect connect;
  connect.context = context_;
  connect.verifier = verifier_;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  auto chlo = getDefaultClientHello();
  TestMessages::removeExtension(chlo, ExtensionType::server_name);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.encodedClientHello(), encodeHandshake(std::move(chlo))));
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_FALSE(state_.sni().has_value());
}

TEST_F(ClientProtocolTest, TestConnectNoAlpn) {
  context_->setSupportedAlpns({});
  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  auto chlo = getDefaultClientHello();
  TestMessages::removeExtension(
      chlo, ExtensionType::application_layer_protocol_negotiation);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.encodedClientHello(), encodeHandshake(std::move(chlo))));
}

TEST_F(ClientProtocolTest, TestConnectExtension) {
  Connect connect;
  connect.context = context_;
  connect.verifier = verifier_;
  connect.sni = "www.hostname.com";
  auto extensions = std::make_shared<MockClientExtensions>();
  connect.extensions = extensions;
  EXPECT_CALL(*extensions, getClientHelloExtensions())
      .WillOnce(InvokeWithoutArgs([]() {
        Extension ext;
        ext.extension_type = ExtensionType::token_binding;
        ext.extension_data = folly::IOBuf::copyBuffer("some extension");
        std::vector<Extension> exts;
        exts.push_back(std::move(ext));
        return exts;
      }));
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  auto chlo = getDefaultClientHello();
  Extension ext;
  ext.extension_type = ExtensionType::token_binding;
  ext.extension_data = folly::IOBuf::copyBuffer("some extension");
  chlo.extensions.push_back(std::move(ext));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.encodedClientHello(), encodeHandshake(std::move(chlo))));
}

TEST_F(ClientProtocolTest, TestConnectMultipleShares) {
  MockKeyExchange* mockKex1;
  MockKeyExchange* mockKex2;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::x25519, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex1]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("x25519share");
        }));
        mockKex1 = ret.get();
        return ret;
      }));
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex2]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("p256share");
        }));
        mockKex2 = ret.get();
        return ret;
      }));

  context_->setDefaultShares({NamedGroup::x25519, NamedGroup::secp256r1});
  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.keyExchangers()->size(), 2);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::x25519).get(), mockKex1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex2);
}

TEST_F(ClientProtocolTest, TestConnectCachedGroup) {
  context_->setDefaultShares({NamedGroup::x25519});
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("p256share");
        }));
        mockKex = ret.get();
        return ret;
      }));

  auto psk = getCachedPsk();
  psk.group = NamedGroup::secp256r1;

  Connect connect;
  connect.context = context_;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex);
}

TEST_F(ClientProtocolTest, TestConnectNoShares) {
  context_->setDefaultShares({});
  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.keyExchangers()->size(), 0);
}

TEST_F(ClientProtocolTest, TestConnectPskEarly) {
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 1000;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<
      MutateState,
      WriteToSocket,
      ReportEarlyHandshakeSuccess,
      SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Attempted);
  EXPECT_TRUE(state_.earlyDataParams().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskEarlyNoAlpn) {
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 1000;
  psk.alpn = folly::none;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<
      MutateState,
      WriteToSocket,
      ReportEarlyHandshakeSuccess,
      SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Attempted);
  EXPECT_TRUE(state_.earlyDataParams().has_value());
  EXPECT_EQ(state_.earlyDataParams()->alpn, folly::none);
}

TEST_F(ClientProtocolTest, TestConnectPskEarlyDisabled) {
  context_->setSendEarlyData(false);
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 1000;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::NotAttempted);
  EXPECT_FALSE(state_.earlyDataParams().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskEarlyAlpnMismatch) {
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 1000;
  psk.alpn = "gopher";
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::NotAttempted);
  EXPECT_FALSE(state_.earlyDataParams().has_value());
}

TEST_F(ClientProtocolTest, TestConnectPskEarlyOmitEarlyRecord) {
  context_->setOmitEarlyRecordLayer(true);
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 1000;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<
      MutateState,
      WriteToSocket,
      ReportEarlyHandshakeSuccess,
      SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Attempted);
  EXPECT_TRUE(state_.earlyDataParams().has_value());
  EXPECT_EQ(state_.earlyWriteRecordLayer().get(), nullptr);
}

TEST_F(ClientProtocolTest, TestConnectPskExternalNoCerts) {
  context_->setOmitEarlyRecordLayer(true);
  Connect connect;
  connect.context = context_;
  CachedPsk psk;
  psk.psk = "External";
  psk.secret = "externalsecret";
  psk.type = PskType::External;
  psk.version = ProtocolVersion::tls_1_3;
  psk.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
}

TEST_F(ClientProtocolTest, TestConnectCompat) {
  context_->setCompatibilityMode(true);
  Connect connect;
  connect.context = context_;
  connect.sni = "www.hostname.com";
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_FALSE(state_.legacySessionId().value()->empty());
}

TEST_F(ClientProtocolTest, TestConnectECH) {
  auto echConfig = ech::test::getECHConfig();
  Connect connect;
  connect.context = context_;
  connect.echConfigs = std::vector<ech::ECHConfig>();
  connect.echConfigs->push_back(echConfig);
  connect.sni = "www.hostname.com";
  const auto& actualChlo = getDefaultClientHello();

  // Two randoms should be generated, 1 for the client hello inner and 1 for the
  // client hello outer.
  EXPECT_CALL(*factory_, makeRandom()).Times(2);

  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);

  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_TRUE(state_.encodedClientHello().has_value());

  auto encodedClientHello = state_.encodedClientHello().value()->clone();

  // We expect this to be false because the encoded client hello should be
  // the encrypted client hello, which contains the actualChlo.
  EXPECT_FALSE(
      folly::IOBufEqualTo()(encodedClientHello, encodeHandshake(actualChlo)));

  // Get rid of handshake header (type + version).
  encodedClientHello->trimStart(4);
  ClientHello chloOuter = decode<ClientHello>(std::move(encodedClientHello));

  // Check we used fake server name.
  auto sniExt = getExtension<ServerNameList>(chloOuter.extensions);
  EXPECT_TRUE(sniExt.hasValue());
  EXPECT_TRUE(folly::IOBufEqualTo()(
      sniExt.value().server_name_list[0].hostname,
      folly::IOBuf::copyBuffer("public.dummy.com")));

  // Check that the state is using the fake server name
  EXPECT_EQ(*state_.sni(), "public.dummy.com");

  // Check that the real name is saved.
  EXPECT_EQ(state_.echState()->sni, "www.hostname.com");

  // Check the legacy session id is the same in both the client hello inner
  // and the client hello outer
  EXPECT_TRUE(folly::IOBufEqualTo()(
      actualChlo.legacy_session_id, chloOuter.legacy_session_id));

  // Check there exists an ECH extension.
  auto echExtension =
      getExtension<ech::OuterECHClientHello>(chloOuter.extensions);
  EXPECT_TRUE(echExtension.hasValue());

  // Check that early data and PSK are NOT present.
  auto pskExtension = getExtension<ClientPresharedKey>(chloOuter.extensions);
  EXPECT_FALSE(pskExtension.hasValue());
  auto earlyExtension = getExtension<ClientEarlyData>(chloOuter.extensions);
  EXPECT_FALSE(earlyExtension.hasValue());
}

TEST_F(ClientProtocolTest, TestConnectECHWithPSK) {
  Connect connect;
  connect.context = context_;
  connect.echConfigs = std::vector<ech::ECHConfig>();
  connect.echConfigs->push_back(ech::test::getECHConfig());

  CachedPsk psk;
  psk.psk = "External";
  psk.secret = "externalsecret";
  psk.type = PskType::External;
  psk.version = ProtocolVersion::tls_1_3;
  psk.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
  connect.cachedPsk = psk;

  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);

  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
  EXPECT_TRUE(state_.encodedClientHello().has_value());

  // Check that ECH state is valid
  EXPECT_TRUE(state_.echState().has_value());
  EXPECT_NE(state_.echState()->encodedECH, nullptr);

  // Grab inner hello for comparison
  auto encodedInnerHello = std::move(state_.echState()->encodedECH);
  encodedInnerHello->trimStart(4);
  ClientHello decodedInnerChlo =
      decode<ClientHello>(std::move(encodedInnerHello));
  auto innerPskExt =
      getExtension<ClientPresharedKey>(decodedInnerChlo.extensions);
  EXPECT_TRUE(innerPskExt.hasValue());

  // Check client hello outer has a GREASE PSK extension
  auto encodedOuterHello = std::move(state_.encodedClientHello().value());
  encodedOuterHello->trimStart(4);
  ClientHello decodedOuterChlo =
      decode<ClientHello>(std::move(encodedOuterHello));
  auto greasePskExt =
      getExtension<ClientPresharedKey>(decodedOuterChlo.extensions);
  EXPECT_TRUE(greasePskExt.hasValue());
  EXPECT_EQ(greasePskExt->identities.size(), 1);
  EXPECT_EQ(greasePskExt->binders.size(), 1);
  auto idRange = greasePskExt->identities.front().psk_identity->coalesce();
  EXPECT_EQ(idRange.size(), psk.psk.size());
  for (size_t i = 0; i < idRange.size(); i++) {
    EXPECT_EQ(idRange.data()[i], 0x44);
  }
  EXPECT_EQ(greasePskExt->identities.front().obfuscated_ticket_age, 0x44444444);

  const std::string binderValue = "verifydata";
  const size_t binderSize =
      innerPskExt->binders.front().binder->computeChainDataLength();
  auto binderRange = greasePskExt->binders.front().binder->coalesce();
  EXPECT_EQ(binderRange.size(), binderSize);
  for (size_t i = 0; i < binderRange.size(); i++) {
    EXPECT_EQ(binderRange.data()[i], 0x44);
  }
}

TEST_F(ClientProtocolTest, TestConnectCompatEarly) {
  context_->setCompatibilityMode(true);
  Connect connect;
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.maxEarlyDataSize = 1000;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<
      MutateState,
      WriteToSocket,
      ReportEarlyHandshakeSuccess,
      SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Attempted);
  EXPECT_TRUE(state_.earlyDataParams().has_value());
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_FALSE(state_.legacySessionId().value()->empty());
}

TEST_F(ClientProtocolTest, TestServerHelloFlow) {
  setupExpectingServerHello();
  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }));
  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(*mockKex_, generateSharedSecret(RangeMatches("servershare")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveHandshakeSecret(RangeMatches("sharedsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectActions<MutateState, SecretAvailable>(actions);

  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockHandshakeContext_);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.group(), NamedGroup::x25519);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::OneRtt);
  EXPECT_EQ(state_.pskType(), PskType::NotAttempted);
  EXPECT_EQ(state_.serverCert(), nullptr);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
}

TEST_F(ClientProtocolTest, TestServerHelloECHFlow) {
  setupExpectingServerHello();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.echState().emplace();
  state_.echState()->encodedECH = folly::IOBuf::copyBuffer("chlo");
  state_.echState()->sni = std::move(*state_.sni());
  state_.echState()->random.fill(0xEC);
  state_.sni() = "fakehostname.com";

  auto shlo = TestMessages::serverHello();
  memset(shlo.random.data() + 24, 0xAA, 8);
  auto dummyshlo = TestMessages::serverHello();
  memset(dummyshlo.random.data() + 24, 0, 8);

  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  auto mockEchHandshakeContext = new MockHandshakeContext();
  Sequence contextSeq;
  Sequence factorySeq;
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(factorySeq)
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }))
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext);
      }));
  EXPECT_CALL(*mockEchHandshakeContext, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches(encodedEncryptedClientHello)))
      .InSequence(contextSeq);
  auto mockEchAcceptScheduler = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(factorySeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<KeyScheduler>(mockEchAcceptScheduler);
      }));
  Random r;
  r.fill(0xEC);
  EXPECT_CALL(
      *mockEchAcceptScheduler,
      deriveEarlySecret(RangeMatches(std::string(r.begin(), r.end()))))
      .InSequence(contextSeq);
  auto mockEchAcceptContext = new MockHandshakeContext();
  EXPECT_CALL(*mockEchHandshakeContext, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchAcceptContext);
      }));
  EXPECT_CALL(*mockEchAcceptContext, appendToTranscript(_))
      .InSequence(contextSeq)
      .WillOnce(Invoke([&](const std::unique_ptr<folly::IOBuf>& buf) mutable {
        EXPECT_TRUE(
            folly::IOBufEqualTo()(buf, encodeHandshake(std::move(dummyshlo))));
      }));
  EXPECT_CALL(*mockEchAcceptContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("acceptctx"); }));
  EXPECT_CALL(
      *mockEchAcceptScheduler,
      getSecret(EarlySecrets::ECHAcceptConfirmation, RangeMatches("acceptctx")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>(
                {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}),
            EarlySecrets::ECHAcceptConfirmation);
      }));
  EXPECT_CALL(
      *mockEchHandshakeContext, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(*mockKex_, generateSharedSecret(RangeMatches("servershare")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveHandshakeSecret(RangeMatches("sharedsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto actions = detail::processEvent(state_, std::move(shlo));
  expectActions<MutateState, SecretAvailable>(actions);

  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockEchHandshakeContext);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.group(), NamedGroup::x25519);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::OneRtt);
  EXPECT_EQ(state_.pskType(), PskType::NotAttempted);
  EXPECT_EQ(state_.serverCert(), nullptr);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
  EXPECT_EQ(state_.sni(), "www.hostname.com");
  EXPECT_EQ(state_.echState()->sni, "www.hostname.com");
  EXPECT_EQ(state_.echState()->status, ECHStatus::Accepted);
}

TEST_F(ClientProtocolTest, TestServerHelloECHRejectedFlow) {
  setupExpectingServerHello();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.echState().emplace();
  state_.echState()->encodedECH = folly::IOBuf::copyBuffer("chlo");
  state_.echState()->sni = std::move(*state_.sni());
  state_.echState()->random.fill(0xEC);
  state_.sni() = "fakehostname.com";

  auto shlo = TestMessages::serverHello();
  auto dummyshlo = TestMessages::serverHello();
  memset(dummyshlo.random.data() + 24, 0, 8);

  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  auto mockEchHandshakeContext = new MockHandshakeContext();
  auto mockEchKeyScheduler = new MockKeyScheduler();
  Sequence factorySeq;
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(factorySeq)
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(factorySeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<KeyScheduler>(mockEchKeyScheduler);
      }));
  Sequence contextSeq;
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }))
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext);
      }));
  EXPECT_CALL(*mockEchHandshakeContext, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches(encodedEncryptedClientHello)))
      .InSequence(contextSeq);
  Random r;
  r.fill(0xEC);
  EXPECT_CALL(
      *mockEchKeyScheduler,
      deriveEarlySecret(RangeMatches(std::string(r.begin(), r.end()))))
      .InSequence(contextSeq);
  auto mockEchAcceptContext = new MockHandshakeContext();
  EXPECT_CALL(*mockEchHandshakeContext, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchAcceptContext);
      }));
  EXPECT_CALL(*mockEchAcceptContext, appendToTranscript(_))
      .InSequence(contextSeq)
      .WillOnce(Invoke([&](const std::unique_ptr<folly::IOBuf>& buf) mutable {
        EXPECT_TRUE(
            folly::IOBufEqualTo()(buf, encodeHandshake(std::move(dummyshlo))));
      }));
  EXPECT_CALL(*mockEchAcceptContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("acceptctx"); }));
  EXPECT_CALL(
      *mockEchKeyScheduler,
      getSecret(EarlySecrets::ECHAcceptConfirmation, RangeMatches("acceptctx")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>(
                {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}),
            EarlySecrets::ECHAcceptConfirmation);
      }));
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(*mockKex_, generateSharedSecret(RangeMatches("servershare")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveHandshakeSecret(RangeMatches("sharedsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto actions = detail::processEvent(state_, std::move(shlo));
  expectActions<MutateState, SecretAvailable>(actions);

  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockHandshakeContext_);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.group(), NamedGroup::x25519);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::OneRtt);
  EXPECT_EQ(state_.pskType(), PskType::NotAttempted);
  EXPECT_EQ(state_.serverCert(), nullptr);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
  EXPECT_EQ(state_.sni(), "fakehostname.com");
  EXPECT_EQ(state_.echState()->sni, "www.hostname.com");
  EXPECT_EQ(state_.echState()->status, ECHStatus::Rejected);
}

TEST_F(ClientProtocolTest, TestServerHelloAfterHrrFlow) {
  setupExpectingServerHelloAfterHrr();
  mockKeyScheduler_ = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  Sequence contextSeq;
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(*mockKex_, generateSharedSecret(RangeMatches("servershare")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveHandshakeSecret(RangeMatches("sharedsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectActions<MutateState, SecretAvailable>(actions);
  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockHandshakeContext_);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.group(), NamedGroup::x25519);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.pskType(), PskType::NotAttempted);
  EXPECT_EQ(state_.serverCert(), nullptr);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
}

TEST_F(ClientProtocolTest, TestServerHelloPskFlow) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }));
  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(*mockKex_, generateSharedSecret(RangeMatches("servershare")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveEarlySecret(RangeMatches("resumptionsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveHandshakeSecret(RangeMatches("sharedsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectActions<MutateState, SecretAvailable>(actions);
  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockHandshakeContext_);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.group(), NamedGroup::x25519);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::OneRtt);
  EXPECT_EQ(state_.pskType(), PskType::Resumption);
  EXPECT_EQ(state_.serverCert(), mockLeaf_);
  EXPECT_EQ(state_.clientCert(), mockClientCert_);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(
          std::chrono::minutes(5) - std::chrono::seconds(10)));
}

TEST_F(ClientProtocolTest, TestServerHelloPskNoDhFlow) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  mockKeyScheduler_ = new MockKeyScheduler();
  mockHandshakeContext_ = new MockHandshakeContext();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext_);
      }));
  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveEarlySecret(RangeMatches("resumptionsecret")));
  EXPECT_CALL(*mockKeyScheduler_, deriveHandshakeSecret());
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto shlo = TestMessages::serverHelloPsk();
  TestMessages::removeExtension(shlo, ExtensionType::key_share);
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectActions<MutateState, SecretAvailable>(actions);

  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockHandshakeContext_);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::None);
  EXPECT_EQ(state_.pskType(), PskType::Resumption);
  EXPECT_EQ(state_.serverCert(), mockLeaf_);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(
          std::chrono::minutes(5) - std::chrono::seconds(10)));
}

TEST_F(ClientProtocolTest, TestServerHelloPskAfterHrrFlow) {
  setupExpectingServerHelloAfterHrr();
  state_.attemptedPsk() = getCachedPsk();
  mockKeyScheduler_ = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  Sequence contextSeq;
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("shloencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo_shlo"); }));
  EXPECT_CALL(*mockKex_, generateSharedSecret(RangeMatches("servershare")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveHandshakeSecret(RangeMatches("sharedsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ServerHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'h', 't'}),
            HandshakeSecrets::ServerHandshakeTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          HandshakeSecrets::ClientHandshakeTraffic, RangeMatches("chlo_shlo")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'h', 't'}),
            HandshakeSecrets::ClientHandshakeTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cht"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sht"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cht"));

  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectActions<MutateState, SecretAvailable>(actions);
  expectSecret(
      actions,
      HandshakeSecrets::ServerHandshakeTraffic,
      folly::StringPiece("sht"));
  expectSecret(
      actions,
      HandshakeSecrets::ClientHandshakeTraffic,
      folly::StringPiece("cht"));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(state_.handshakeContext().get(), mockHandshakeContext_);
  EXPECT_EQ(state_.keyScheduler().get(), mockKeyScheduler_);
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(state_.group(), NamedGroup::x25519);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.clientHandshakeSecret(), folly::IOBuf::copyBuffer("cht")));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.serverHandshakeSecret(), folly::IOBuf::copyBuffer("sht")));
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.pskType(), PskType::Resumption);
  EXPECT_EQ(state_.serverCert(), mockLeaf_);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(
          std::chrono::minutes(5) - std::chrono::seconds(10)));
}

TEST_F(ClientProtocolTest, TestServerHelloECHRejectedAfterHRRAccepted) {
  setupExpectingServerHelloAfterHrr();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.echState().emplace();
  state_.echState()->encodedECH = folly::IOBuf::copyBuffer("chlo");
  state_.echState()->sni = state_.sni();
  state_.echState()->status = ECHStatus::Accepted;
  auto echHandshakeContext = std::make_unique<MockHandshakeContext>();
  echHandshakeContext->setDefaults();
  state_.echState()->handshakeContext = std::move(echHandshakeContext);

  auto shlo = TestMessages::serverHello();
  memset(shlo.random.data() + 24, 0xFF, 8);
  auto dummyshlo = TestMessages::serverHello();
  memset(dummyshlo.random.data() + 24, 0, 8);

  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "ech acceptance mismatch between hrr and shlo");
}

TEST_F(ClientProtocolTest, TestServerHelloECHAcceptedAfterHRRRejected) {
  setupExpectingServerHelloAfterHrr();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.echState().emplace();
  state_.echState()->encodedECH = folly::IOBuf::copyBuffer("chlo");
  state_.echState()->sni = std::move(*state_.sni());
  state_.echState()->status = ECHStatus::Rejected;
  state_.echState()->random.fill(0xEC);
  state_.sni() = "fakehostname.com";
  auto echHandshakeContext = std::make_unique<MockHandshakeContext>();
  auto echHandshakeContextPtr = echHandshakeContext.get();
  echHandshakeContext->setDefaults();
  state_.echState()->handshakeContext = std::move(echHandshakeContext);

  auto shlo = TestMessages::serverHello();
  memset(shlo.random.data() + 24, 0xAA, 8);
  auto dummyshlo = TestMessages::serverHello();
  memset(dummyshlo.random.data() + 24, 0, 8);

  mockKeyScheduler_ = new MockKeyScheduler();
  auto mockEchKeyScheduler = new MockKeyScheduler();
  Sequence factorySeq;
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(factorySeq)
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(factorySeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<KeyScheduler>(mockEchKeyScheduler);
      }));
  Sequence contextSeq;
  Random r;
  r.fill(0xEC);
  EXPECT_CALL(
      *mockEchKeyScheduler,
      deriveEarlySecret(RangeMatches(std::string(r.begin(), r.end()))))
      .InSequence(contextSeq);
  auto mockEchAcceptContext = new MockHandshakeContext();
  EXPECT_CALL(*echHandshakeContextPtr, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchAcceptContext);
      }));
  EXPECT_CALL(*mockEchAcceptContext, appendToTranscript(_))
      .InSequence(contextSeq)
      .WillOnce(Invoke([&](const std::unique_ptr<folly::IOBuf>& buf) mutable {
        EXPECT_TRUE(
            folly::IOBufEqualTo()(buf, encodeHandshake(std::move(dummyshlo))));
      }));
  EXPECT_CALL(*mockEchAcceptContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("acceptctx"); }));
  EXPECT_CALL(
      *mockEchKeyScheduler,
      getSecret(EarlySecrets::ECHAcceptConfirmation, RangeMatches("acceptctx")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>(
                {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}),
            EarlySecrets::ECHAcceptConfirmation);
      }));

  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "ech acceptance mismatch between hrr and shlo");
}

TEST_F(ClientProtocolTest, TestServerHello) {
  setupExpectingServerHello();
  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectActions<MutateState, SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
}

TEST_F(ClientProtocolTest, TestServerHelloPsk) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectActions<MutateState, SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.pskType(), PskType::Resumption);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(
          std::chrono::minutes(5) - std::chrono::seconds(10)));
}

TEST_F(ClientProtocolTest, TestServerHelloPskRejected) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectActions<MutateState, SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.pskType(), PskType::Rejected);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
}

TEST_F(ClientProtocolTest, TestServerHelloExtraData) {
  setupExpectingServerHello();
  EXPECT_CALL(*mockRead_, hasUnparsedHandshakeData())
      .WillRepeatedly(Return(true));
  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectError<FizzException>(
      actions, AlertDescription::unexpected_message, "data after server hello");
}

TEST_F(ClientProtocolTest, TestServerHelloBadVersion) {
  setupExpectingServerHello();
  auto shlo = TestMessages::serverHello();
  TestMessages::removeExtension(shlo, ExtensionType::supported_versions);
  ServerSupportedVersions supportedVersions;
  supportedVersions.selected_version = ProtocolVersion::tls_1_1;
  shlo.extensions.push_back(encodeExtension(std::move(supportedVersions)));
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions,
      AlertDescription::protocol_version,
      "unsupported server version");
}

TEST_F(ClientProtocolTest, TestServerHelloBadCipher) {
  setupExpectingServerHello();
  auto shlo = TestMessages::serverHello();
  shlo.cipher_suite = static_cast<CipherSuite>(0x03ff);
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "unsupported cipher");
}

TEST_F(ClientProtocolTest, TestServerHelloBadGroup) {
  context_->setSupportedGroups({NamedGroup::x25519});
  setupExpectingServerHello();
  auto shlo = TestMessages::serverHello();
  TestMessages::removeExtension(shlo, ExtensionType::key_share);
  ServerKeyShare serverKeyShare;
  serverKeyShare.server_share.group = NamedGroup::secp256r1;
  serverKeyShare.server_share.key_exchange =
      folly::IOBuf::copyBuffer("servershare");
  shlo.extensions.push_back(encodeExtension(std::move(serverKeyShare)));
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "unsupported group");
}

TEST_F(ClientProtocolTest, TestServerHelloNoKeyShare) {
  setupExpectingServerHello();
  auto shlo = TestMessages::serverHello();
  TestMessages::removeExtension(shlo, ExtensionType::key_share);
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "did not send share");
}

TEST_F(ClientProtocolTest, TestServerHelloHrrBadVersion) {
  setupExpectingServerHelloAfterHrr();
  state_.version() = ProtocolVersion::tls_1_2;
  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "version does not match");
}

TEST_F(ClientProtocolTest, TestServerHelloHrrBadCipher) {
  setupExpectingServerHelloAfterHrr();
  state_.cipher() = CipherSuite::TLS_AES_256_GCM_SHA384;
  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "cipher does not match");
}

TEST_F(ClientProtocolTest, TestServerHelloHrrBadGroup) {
  setupExpectingServerHelloAfterHrr();
  auto mockKex = std::make_unique<MockKeyExchange>();
  mockKex->setDefaults();
  std::map<NamedGroup, std::unique_ptr<KeyExchange>> kexs;
  kexs.emplace(NamedGroup::secp256r1, std::move(mockKex));
  state_.keyExchangers() = std::move(kexs);

  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "group");
}

TEST_F(ClientProtocolTest, TestServerHelloPskAcceptedNotSent) {
  setupExpectingServerHello();
  state_.requestedExtensions() = std::vector<ExtensionType>(
      {ExtensionType::supported_versions,
       ExtensionType::key_share,
       ExtensionType::server_name,
       ExtensionType::application_layer_protocol_negotiation});
  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "unexpected extension in shlo: pre_shared_key");
}

TEST_F(ClientProtocolTest, TestServerHelloAcceptedGreasePsk) {
  setupExpectingServerHello();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.echState().emplace();
  state_.echState()->encodedECH = folly::IOBuf::copyBuffer("chlo");
  state_.echState()->sni = state_.sni();

  auto psk = getCachedPsk();
  ClientPresharedKey greasePsk;
  PskIdentity greaseIdentity;
  greaseIdentity.psk_identity = factory_->makeRandomBytes(psk.psk.size());
  greaseIdentity.obfuscated_ticket_age = 0x44444444;
  greasePsk.identities.push_back(std::move(greaseIdentity));
  PskBinder greaseBinder;
  greaseBinder.binder = factory_->makeRandomBytes(10);
  greasePsk.binders.push_back(std::move(greaseBinder));
  state_.echState()->greasePsk = std::move(greasePsk);
  state_.attemptedPsk() = std::move(psk);

  auto shlo = TestMessages::serverHelloPsk();
  memset(shlo.random.data() + 24, 0xFF, 8);
  auto dummyshlo = TestMessages::serverHelloPsk();
  memset(dummyshlo.random.data() + 24, 0, 8);

  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "ech rejected but server accepted psk");
}

TEST_F(ClientProtocolTest, TestServerHelloOtherPskAccepted) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  auto shlo = TestMessages::serverHello();
  ServerPresharedKey pskExt;
  pskExt.selected_identity = 1;
  shlo.extensions.push_back(encodeExtension(std::move(pskExt)));
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "non-0 psk");
}

TEST_F(ClientProtocolTest, TestServerHelloPskDifferentHash) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  state_.attemptedPsk()->cipher = CipherSuite::TLS_AES_256_GCM_SHA384;
  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectError<FizzException>(
      actions,
      AlertDescription::handshake_failure,
      "incompatible cipher in psk");
}

TEST_F(ClientProtocolTest, TestServerHelloPskDifferentCompatibleCipher) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  state_.attemptedPsk()->cipher = CipherSuite::TLS_CHACHA20_POLY1305_SHA256;
  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectActions<MutateState, SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingEncryptedExtensions);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(
          std::chrono::minutes(5) - std::chrono::seconds(10)));
}

TEST_F(ClientProtocolTest, TestServerHelloPskDheNotSupported) {
  context_->setSupportedPskModes({PskKeyExchangeMode::psk_ke});
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  auto actions = detail::processEvent(state_, TestMessages::serverHelloPsk());
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "unsupported psk mode");
}

TEST_F(ClientProtocolTest, TestServerHelloExtensions) {
  setupExpectingEncryptedExtensions();
  auto ext = std::make_shared<MockClientExtensions>();
  state_.extensions() = ext;
  EXPECT_CALL(*ext, onEncryptedExtensions(_));
  auto actions = detail::processEvent(state_, TestMessages::encryptedExt());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
}

TEST_F(ClientProtocolTest, TestServerHelloPskKeNotSupported) {
  context_->setSupportedPskModes({PskKeyExchangeMode::psk_dhe_ke});
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  auto shlo = TestMessages::serverHelloPsk();
  TestMessages::removeExtension(shlo, ExtensionType::key_share);
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "unsupported psk mode");
}

TEST_F(ClientProtocolTest, TestServerHelloBadSessionId) {
  setupExpectingServerHello();
  auto shlo = TestMessages::serverHello();
  shlo.legacy_session_id_echo = folly::IOBuf::copyBuffer("hi!!");
  auto actions = detail::processEvent(state_, std::move(shlo));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "session id");
}

TEST_F(ClientProtocolTest, TestConnectPskKeNoShares) {
  Connect connect;
  context_->setSendKeyShare(SendKeyShare::WhenNecessary);
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.group = folly::none;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);

  auto& encodedHello = *state_.encodedClientHello();

  // Get rid of handshake header (type + version)
  encodedHello->trimStart(4);
  auto decodedHello = decode<ClientHello>(std::move(encodedHello));
  auto keyShare = getExtension<ClientKeyShare>(decodedHello.extensions);
  EXPECT_TRUE(keyShare->client_shares.empty());
  EXPECT_TRUE(state_.keyExchangers()->empty());
}

TEST_F(ClientProtocolTest, TestConnectPskKeAlwaysShares) {
  Connect connect;
  context_->setSendKeyShare(SendKeyShare::Always);
  connect.context = context_;
  auto psk = getCachedPsk();
  psk.group = folly::none;
  connect.cachedPsk = psk;
  auto actions = detail::processEvent(state_, std::move(connect));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);

  auto& encodedHello = *state_.encodedClientHello();

  // Get rid of handshake header (type + version)
  encodedHello->trimStart(4);
  auto decodedHello = decode<ClientHello>(std::move(encodedHello));
  auto keyShare = getExtension<ClientKeyShare>(decodedHello.extensions);
  EXPECT_TRUE(!keyShare->client_shares.empty());
  EXPECT_TRUE(!state_.keyExchangers()->empty());
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestFlow) {
  setupExpectingServerHello();
  state_.clientRandom()->fill(0x66);
  auto mockHandshakeContext1 = new MockHandshakeContext();
  auto mockHandshakeContext2 = new MockHandshakeContext();
  Sequence contextSeq;
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext1);
      }));
  EXPECT_CALL(*mockHandshakeContext1, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext2);
      }));
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  auto chlo = getDefaultClientHello();
  chlo.random.fill(0x66);
  ClientKeyShare keyShare;
  KeyShareEntry entry;
  entry.group = NamedGroup::secp256r1;
  entry.key_exchange = folly::IOBuf::copyBuffer("keyshare");
  keyShare.client_shares.push_back(std::move(entry));
  auto it = chlo.extensions.erase(
      findExtension(chlo.extensions, ExtensionType::key_share));
  chlo.extensions.insert(it, encodeExtension(std::move(keyShare)));
  auto encodedExpectedChlo = encodeHandshake(std::move(chlo));
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(msg.fragment, encodedExpectedChlo));
        content.data = folly::IOBuf::copyBuffer("writtenchlo");
        return content;
      }));

  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenchlo")));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(*state_.encodedClientHello(), encodedExpectedChlo));
  EXPECT_EQ(
      folly::StringPiece((*state_.encodedClientHello())->coalesce()),
      folly::StringPiece(encodedExpectedChlo->coalesce()));
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  Random random;
  random.fill(0x66);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.earlyDataType(), EarlyDataType::NotAttempted);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestPskFlow) {
  auto psk = getCachedPsk();
  setupExpectingServerHello();
  state_.attemptedPsk() = psk;
  state_.clientRandom()->fill(0x66);
  auto mockHandshakeContext1 = new MockHandshakeContext();
  auto mockHandshakeContext2 = new MockHandshakeContext();
  auto mockHandshakeContext3 = new MockHandshakeContext();
  mockKeyScheduler_ = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveEarlySecret(RangeMatches("resumptionsecret")));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(EarlySecrets::ResumptionPskBinder, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'b', 'k'}),
            EarlySecrets::ResumptionPskBinder);
      }));
  Sequence contextSeq;
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext1);
      }));
  EXPECT_CALL(*mockHandshakeContext1, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("chlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext2);
      }));
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext2, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext3);
      }));
  EXPECT_CALL(*mockHandshakeContext3, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext3, getFinishedData(RangeMatches("bk")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("binder"); }));
  EXPECT_CALL(*mockHandshakeContext3, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        content.data = folly::IOBuf::copyBuffer("writtenchlo");
        return content;
      }));

  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents[0].contentType, ContentType::handshake);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Plaintext);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  Random random;
  random.fill(0x66);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
  EXPECT_EQ(state_.earlyDataType(), EarlyDataType::NotAttempted);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestECHFlow) {
  setupExpectingServerHello();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  state_.echState().emplace();
  state_.echState()->encodedECH = std::move(*state_.encodedClientHello());
  state_.echState()->random.fill(0x66);
  state_.echState()->sni = std::move(*state_.sni());
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.clientRandom()->fill(0xEE);
  state_.sni() = "fakehostname.com";

  auto mockHpkeContext = new hpke::test::MockHpkeContext();
  state_.echState()->hpkeSetup.enc = folly::IOBuf::copyBuffer("enc");
  state_.echState()->hpkeSetup.context.reset(mockHpkeContext);
  state_.echState()->supportedConfig.config.version = ech::ECHVersion::Draft15;
  state_.echState()->supportedConfig.config.ech_config_content =
      folly::IOBuf::copyBuffer("echconfig");
  state_.echState()->supportedConfig.cipherSuite = {
      hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  state_.echState()->supportedConfig.maxLen = 42;
  state_.echState()->supportedConfig.configId = 0xFB;

  auto mockHandshakeContext1 = new MockHandshakeContext();
  auto mockHandshakeContext2 = new MockHandshakeContext();
  auto mockEchHandshakeContext1 = new MockHandshakeContext();
  auto mockEchHandshakeContext2 = new MockHandshakeContext();
  Sequence contextSeq;
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext1);
      }));
  EXPECT_CALL(*mockHandshakeContext1, appendToTranscript(BufMatches("esni")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("outerchlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext2);
      }));
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext1);
      }));
  EXPECT_CALL(*mockEchHandshakeContext1, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("innerchlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext2);
      }));
  EXPECT_CALL(*mockEchHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  mockKeyScheduler_ = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  Random r;
  r.fill(0x66);
  EXPECT_CALL(
      *mockKeyScheduler_,
      deriveEarlySecret(RangeMatches(std::string(r.begin(), r.end()))))
      .InSequence(contextSeq);
  auto mockEchAcceptContext = new MockHandshakeContext();
  EXPECT_CALL(*mockEchHandshakeContext2, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchAcceptContext);
      }));
  EXPECT_CALL(*mockEchAcceptContext, appendToTranscript(_))
      .InSequence(contextSeq)
      .WillOnce(Invoke([&](const std::unique_ptr<folly::IOBuf>& buf) mutable {
        auto hrr = TestMessages::helloRetryRequest();
        ech::ECHHelloRetryRequest conf;
        conf.confirmation.fill(0x00);
        hrr.extensions.push_back(encodeExtension(conf));
        EXPECT_TRUE(
            folly::IOBufEqualTo()(buf, encodeHandshake(std::move(hrr))));
      }));
  EXPECT_CALL(*mockEchAcceptContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("acceptctx"); }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          EarlySecrets::HRRECHAcceptConfirmation, RangeMatches("acceptctx")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>(
                {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}),
            EarlySecrets::HRRECHAcceptConfirmation);
      }));
  // Construct expected hellos for hpke sealing setup.
  auto chlo = getDefaultClientHello();
  chlo.random.fill(0x66);
  ClientKeyShare keyShare;
  KeyShareEntry entry;
  entry.group = NamedGroup::secp256r1;
  entry.key_exchange = folly::IOBuf::copyBuffer("keyshare");
  keyShare.client_shares.push_back(std::move(entry));
  auto it = chlo.extensions.erase(
      findExtension(chlo.extensions, ExtensionType::key_share));
  chlo.extensions.insert(it, encodeExtension(std::move(keyShare)));

  // Outer has the outer random + sni set.
  auto chloOuter = chlo.clone();
  chloOuter.random.fill(0xEE);
  ServerNameList sni;
  sni.server_name_list.emplace_back();
  sni.server_name_list.front().hostname =
      folly::IOBuf::copyBuffer("fakehostname.com");
  it = chloOuter.extensions.erase(
      findExtension(chloOuter.extensions, ExtensionType::server_name));
  chloOuter.extensions.insert(it, encodeExtension(std::move(sni)));

  // Add the extension to the inner one
  chlo.extensions.push_back(encodeExtension(ech::InnerECHClientHello()));

  // Save this one (the real one), then blank the legacy session id for AAD
  // construction
  auto encodedClientHelloInner = encodeHandshake(chlo.clone());

  chlo.legacy_session_id = folly::IOBuf::copyBuffer("");
  auto encodedClientHelloInnerAad = encode(chlo);

  // Add padding
  auto paddingSize = ech::calculateECHPadding(
      chlo,
      encodedClientHelloInnerAad->computeChainDataLength(),
      42); // maxLen

  if (paddingSize > 0) {
    auto paddingBuf = folly::IOBuf::create(paddingSize);
    memset(paddingBuf->writableData(), 0, paddingSize);
    paddingBuf->append(paddingSize);
    encodedClientHelloInnerAad->prependChain(std::move(paddingBuf));
  }

  // Set up ECH extension for AAD and ECH
  ech::OuterECHClientHello echExtension;
  echExtension.cipher_suite = state_.echState()->supportedConfig.cipherSuite;
  echExtension.config_id = state_.echState()->supportedConfig.configId;
  echExtension.enc = folly::IOBuf::create(0);

  // Make dummy payload.
  size_t payloadSize = encodedClientHelloInnerAad->computeChainDataLength() +
      hpke::makeCipher(echExtension.cipher_suite.aead_id)->getCipherOverhead();
  echExtension.payload = folly::IOBuf::create(payloadSize);
  memset(echExtension.payload->writableData(), 0, payloadSize);
  echExtension.payload->append(payloadSize);

  // Make AAD outer chlo
  auto chloOuterForAad = chloOuter.clone();
  chloOuterForAad.extensions.push_back(encodeExtension(echExtension));
  auto clientHelloOuterAad = encode(chloOuterForAad);

  EXPECT_CALL(*mockHpkeContext, _seal(_, _))
      .InSequence(contextSeq)
      .WillOnce(Invoke(
          [&](const folly::IOBuf* aad, std::unique_ptr<folly::IOBuf>& pt) {
            EXPECT_TRUE(folly::IOBufEqualTo()(aad, clientHelloOuterAad.get()));
            EXPECT_TRUE(folly::IOBufEqualTo()(pt, encodedClientHelloInnerAad));
            auto sealed = folly::IOBuf::create(payloadSize);
            memset(sealed->writableData(), 0xEC, payloadSize);
            sealed->append(payloadSize);
            return sealed;
          }));
  EXPECT_CALL(
      *mockEchHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  // Now outer chlo should have a mocked ECH extension.
  memset(echExtension.payload->writableData(), 0xEC, payloadSize);
  chloOuter.extensions.push_back(encodeExtension(std::move(echExtension)));
  auto encodedExpectedChlo = encodeHandshake(std::move(chloOuter));
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(msg.fragment, encodedExpectedChlo));
        content.data = folly::IOBuf::copyBuffer("writtenchlo");
        return content;
      }));

  // Generate HRR with accepted ECH
  auto hrr = TestMessages::helloRetryRequest();
  ech::ECHHelloRetryRequest conf;
  conf.confirmation.fill(0xAA);
  hrr.extensions.push_back(encodeExtension(conf));

  auto actions = detail::processEvent(state_, std::move(hrr));
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenchlo")));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(*state_.encodedClientHello(), encodedExpectedChlo));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      state_.echState()->encodedECH, encodedClientHelloInner));
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  EXPECT_EQ(state_.echState()->sni, "www.hostname.com");
  EXPECT_EQ(state_.echState()->status, ECHStatus::Accepted);
  Random random;
  random.fill(0x66);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_EQ(state_.echState()->random, random);
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.earlyDataType(), EarlyDataType::NotAttempted);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestECHRejectedFlow) {
  setupExpectingServerHello();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  state_.echState().emplace();
  state_.echState()->encodedECH = std::move(*state_.encodedClientHello());
  state_.echState()->random.fill(0x66);
  state_.echState()->sni = std::move(*state_.sni());
  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.clientRandom()->fill(0xEE);
  state_.sni() = "fakehostname.com";

  auto mockHpkeContext = new hpke::test::MockHpkeContext();
  state_.echState()->hpkeSetup.enc = folly::IOBuf::copyBuffer("enc");
  state_.echState()->hpkeSetup.context.reset(mockHpkeContext);
  state_.echState()->supportedConfig.config.version = ech::ECHVersion::Draft15;
  state_.echState()->supportedConfig.config.ech_config_content =
      folly::IOBuf::copyBuffer("echconfig");
  state_.echState()->supportedConfig.cipherSuite = {
      hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  state_.echState()->supportedConfig.configId = 0xFB;
  state_.echState()->supportedConfig.maxLen = 42;

  auto mockHandshakeContext1 = new MockHandshakeContext();
  auto mockHandshakeContext2 = new MockHandshakeContext();
  auto mockEchHandshakeContext1 = new MockHandshakeContext();
  auto mockEchHandshakeContext2 = new MockHandshakeContext();
  Sequence contextSeq;
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext1);
      }));
  EXPECT_CALL(*mockHandshakeContext1, appendToTranscript(BufMatches("esni")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("outerchlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext2);
      }));
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext1);
      }));
  EXPECT_CALL(*mockEchHandshakeContext1, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("innerchlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext2);
      }));
  EXPECT_CALL(*mockEchHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  mockKeyScheduler_ = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  Random r;
  r.fill(0x66);
  EXPECT_CALL(
      *mockKeyScheduler_,
      deriveEarlySecret(RangeMatches(std::string(r.begin(), r.end()))))
      .InSequence(contextSeq);
  auto mockEchAcceptContext = new MockHandshakeContext();
  EXPECT_CALL(*mockEchHandshakeContext2, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchAcceptContext);
      }));
  EXPECT_CALL(*mockEchAcceptContext, appendToTranscript(_))
      .InSequence(contextSeq)
      .WillOnce(Invoke([&](const std::unique_ptr<folly::IOBuf>& buf) mutable {
        auto hrr = TestMessages::helloRetryRequest();
        ech::ECHHelloRetryRequest conf;
        conf.confirmation.fill(0x00);
        hrr.extensions.push_back(encodeExtension(conf));
        EXPECT_TRUE(
            folly::IOBufEqualTo()(buf, encodeHandshake(std::move(hrr))));
      }));
  EXPECT_CALL(*mockEchAcceptContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("acceptctx"); }));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(
          EarlySecrets::HRRECHAcceptConfirmation, RangeMatches("acceptctx")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>(
                {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}),
            EarlySecrets::HRRECHAcceptConfirmation);
      }));
  // Construct expected hellos for hpke sealing setup.
  auto chlo = getDefaultClientHello();
  chlo.random.fill(0x66);
  ClientKeyShare keyShare;
  KeyShareEntry entry;
  entry.group = NamedGroup::secp256r1;
  entry.key_exchange = folly::IOBuf::copyBuffer("keyshare");
  keyShare.client_shares.push_back(std::move(entry));
  auto it = chlo.extensions.erase(
      findExtension(chlo.extensions, ExtensionType::key_share));
  chlo.extensions.insert(it, encodeExtension(std::move(keyShare)));

  // Outer has the outer random + sni set.
  auto chloOuter = chlo.clone();
  chloOuter.random.fill(0xEE);
  ServerNameList sni;
  sni.server_name_list.emplace_back();
  sni.server_name_list.front().hostname =
      folly::IOBuf::copyBuffer("fakehostname.com");
  it = chloOuter.extensions.erase(
      findExtension(chloOuter.extensions, ExtensionType::server_name));
  chloOuter.extensions.insert(it, encodeExtension(std::move(sni)));

  // Add the extension to the inner one
  chlo.extensions.push_back(encodeExtension(ech::InnerECHClientHello()));

  // Save this one (the real one), then blank the legacy session id for AAD
  // construction
  auto encodedClientHelloInner = encodeHandshake(chlo.clone());
  chlo.legacy_session_id = folly::IOBuf::copyBuffer("");
  auto encodedClientHelloInnerAad = encode(chlo);

  // Add padding
  auto paddingSize = ech::calculateECHPadding(
      chlo,
      encodedClientHelloInnerAad->computeChainDataLength(),
      42); // maxLen

  if (paddingSize > 0) {
    auto paddingBuf = folly::IOBuf::create(paddingSize);
    memset(paddingBuf->writableData(), 0, paddingSize);
    paddingBuf->append(paddingSize);
    encodedClientHelloInnerAad->prependChain(std::move(paddingBuf));
  }

  // Set up ECH extension for AAD and ECH
  ech::OuterECHClientHello echExtension;
  echExtension.cipher_suite = state_.echState()->supportedConfig.cipherSuite;
  echExtension.config_id = state_.echState()->supportedConfig.configId;
  echExtension.enc = folly::IOBuf::create(0);

  // Make dummy payload.
  size_t payloadSize = encodedClientHelloInnerAad->computeChainDataLength() +
      hpke::makeCipher(echExtension.cipher_suite.aead_id)->getCipherOverhead();
  echExtension.payload = folly::IOBuf::create(payloadSize);
  memset(echExtension.payload->writableData(), 0, payloadSize);
  echExtension.payload->append(payloadSize);

  // Make AAD outer chlo
  auto chloOuterForAad = chloOuter.clone();
  chloOuterForAad.extensions.push_back(encodeExtension(echExtension));
  auto clientHelloOuterAad = encode(chloOuterForAad);

  EXPECT_CALL(*mockHpkeContext, _seal(_, _))
      .InSequence(contextSeq)
      .WillOnce(Invoke(
          [&](const folly::IOBuf* aad, std::unique_ptr<folly::IOBuf>& pt) {
            EXPECT_TRUE(folly::IOBufEqualTo()(aad, clientHelloOuterAad.get()));
            EXPECT_TRUE(folly::IOBufEqualTo()(pt, encodedClientHelloInnerAad));
            auto sealed = folly::IOBuf::create(payloadSize);
            memset(sealed->writableData(), 0xEC, payloadSize);
            sealed->append(payloadSize);
            return sealed;
          }));
  EXPECT_CALL(
      *mockEchHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  // Now outer chlo should have a mocked ECH extension.
  memset(echExtension.payload->writableData(), 0xEC, payloadSize);
  chloOuter.extensions.push_back(encodeExtension(std::move(echExtension)));
  auto encodedExpectedChlo = encodeHandshake(std::move(chloOuter));
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(msg.fragment, encodedExpectedChlo));
        content.data = folly::IOBuf::copyBuffer("writtenchlo");
        return content;
      }));

  // Generate HRR with rejected ECH
  auto hrr = TestMessages::helloRetryRequest();
  ech::ECHHelloRetryRequest conf;
  conf.confirmation.fill(0xFF);
  hrr.extensions.push_back(encodeExtension(conf));

  auto actions = detail::processEvent(state_, std::move(hrr));
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenchlo")));
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_TRUE(
      folly::IOBufEqualTo()(*state_.encodedClientHello(), encodedExpectedChlo));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      state_.echState()->encodedECH, encodedClientHelloInner));
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "fakehostname.com");
  EXPECT_EQ(state_.echState()->sni, "www.hostname.com");
  EXPECT_EQ(state_.echState()->status, ECHStatus::Rejected);
  Random random;
  random.fill(0xEE);
  EXPECT_EQ(*state_.clientRandom(), random);
  random.fill(0x66);
  EXPECT_EQ(state_.echState()->random, random);
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.earlyDataType(), EarlyDataType::NotAttempted);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestECHPSKFlow) {
  setupExpectingServerHello();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  state_.echState().emplace();
  state_.echState()->encodedECH = std::move(*state_.encodedClientHello());
  state_.echState()->random.fill(0x66);
  state_.echState()->sni = std::move(*state_.sni());

  auto encodedEncryptedClientHello = "esni";
  state_.encodedClientHello() =
      folly::IOBuf::copyBuffer(encodedEncryptedClientHello);
  state_.clientRandom()->fill(0xEE);
  state_.sni() = "fakehostname.com";
  auto psk = getCachedPsk();
  state_.attemptedPsk() = psk;

  ClientPresharedKey greasePsk;
  PskIdentity greaseIdentity;
  greaseIdentity.psk_identity = factory_->makeRandomBytes(psk.psk.size());
  greaseIdentity.obfuscated_ticket_age = 0x44444444;
  greasePsk.identities.push_back(std::move(greaseIdentity));
  PskBinder greaseBinder;
  greaseBinder.binder = factory_->makeRandomBytes(10);
  greasePsk.binders.push_back(std::move(greaseBinder));
  state_.echState()->greasePsk = std::move(greasePsk);

  auto mockHpkeContext = new hpke::test::MockHpkeContext();
  state_.echState()->hpkeSetup.enc = folly::IOBuf::copyBuffer("enc");
  state_.echState()->hpkeSetup.context.reset(mockHpkeContext);
  state_.echState()->supportedConfig.config.version = ech::ECHVersion::Draft15;
  state_.echState()->supportedConfig.config.ech_config_content =
      folly::IOBuf::copyBuffer("echconfig");
  state_.echState()->supportedConfig.cipherSuite = {
      hpke::KDFId::Sha256, hpke::AeadId::TLS_AES_128_GCM_SHA256};
  state_.echState()->supportedConfig.configId = 0xFB;

  auto mockHandshakeContext1 = new MockHandshakeContext();
  auto mockHandshakeContext2 = new MockHandshakeContext();
  auto mockEchHandshakeContext1 = new MockHandshakeContext();
  auto mockEchHandshakeContext2 = new MockHandshakeContext();
  auto mockResumptionContext = new MockHandshakeContext();
  Sequence contextSeq;
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext1);
      }));
  EXPECT_CALL(*mockHandshakeContext1, appendToTranscript(BufMatches("esni")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("outerchlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockHandshakeContext2);
      }));
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext1);
      }));
  EXPECT_CALL(*mockEchHandshakeContext1, appendToTranscript(BufMatches("chlo")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext1, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("innerchlo1"); }));
  EXPECT_CALL(
      *factory_, makeHandshakeContext(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchHandshakeContext2);
      }));
  EXPECT_CALL(*mockEchHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  mockKeyScheduler_ = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          [=]() { return std::unique_ptr<KeyScheduler>(mockKeyScheduler_); }));
  EXPECT_CALL(*mockEchHandshakeContext2, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockResumptionContext);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveEarlySecret(RangeMatches("resumptionsecret")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(EarlySecrets::ResumptionPskBinder, _))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'b', 'k'}),
            EarlySecrets::ResumptionPskBinder);
      }));
  EXPECT_CALL(*mockResumptionContext, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockResumptionContext, getFinishedData(RangeMatches("bk")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("binder"); }));
  EXPECT_CALL(*mockResumptionContext, appendToTranscript(_))
      .InSequence(contextSeq);
  auto mockAcceptKeyScheduler = new MockKeyScheduler();
  EXPECT_CALL(*factory_, makeKeyScheduler(CipherSuite::TLS_AES_128_GCM_SHA256))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<KeyScheduler>(mockAcceptKeyScheduler);
      }));
  Random r;
  r.fill(0x66);
  EXPECT_CALL(
      *mockAcceptKeyScheduler,
      deriveEarlySecret(RangeMatches(std::string(r.begin(), r.end()))))
      .InSequence(contextSeq);
  auto mockEchAcceptContext = new MockHandshakeContext();
  EXPECT_CALL(*mockEchHandshakeContext2, clone())
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([=]() {
        return std::unique_ptr<HandshakeContext>(mockEchAcceptContext);
      }));
  EXPECT_CALL(*mockEchAcceptContext, appendToTranscript(_))
      .InSequence(contextSeq)
      .WillOnce(Invoke([&](const std::unique_ptr<folly::IOBuf>& buf) mutable {
        auto hrr = TestMessages::helloRetryRequest();
        ech::ECHHelloRetryRequest conf;
        conf.confirmation.fill(0x00);
        hrr.extensions.push_back(encodeExtension(conf));
        EXPECT_TRUE(
            folly::IOBufEqualTo()(buf, encodeHandshake(std::move(hrr))));
      }));
  EXPECT_CALL(*mockEchAcceptContext, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("acceptctx"); }));
  EXPECT_CALL(
      *mockAcceptKeyScheduler,
      getSecret(
          EarlySecrets::HRRECHAcceptConfirmation, RangeMatches("acceptctx")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>(
                {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}),
            EarlySecrets::HRRECHAcceptConfirmation);
      }));
  EXPECT_CALL(*mockHpkeContext, _seal(_, _))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("echsealed"); }));
  EXPECT_CALL(
      *mockEchHandshakeContext2, appendToTranscript(BufMatches("hrrencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockEchHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext2, appendToTranscript(_))
      .InSequence(contextSeq);
  MockKeyExchange* mockKex;
  EXPECT_CALL(
      *factory_,
      makeKeyExchange(NamedGroup::secp256r1, Factory::KeyExchangeMode::Client))
      .WillOnce(InvokeWithoutArgs([&mockKex]() {
        auto ret = std::make_unique<MockKeyExchange>();
        EXPECT_CALL(*ret, generateKeyPair());
        EXPECT_CALL(*ret, getKeyShare()).WillOnce(InvokeWithoutArgs([]() {
          return folly::IOBuf::copyBuffer("keyshare");
        }));
        mockKex = ret.get();
        return ret;
      }));
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        content.data = folly::IOBuf::copyBuffer("writtenchlo");
        return content;
      }));

  // Generate HRR with accepted ECH
  auto hrr = TestMessages::helloRetryRequest();
  ech::ECHHelloRetryRequest conf;
  conf.confirmation.fill(0xAA);
  hrr.extensions.push_back(encodeExtension(conf));

  auto actions = detail::processEvent(state_, std::move(hrr));
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents[0].contentType, ContentType::handshake);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Plaintext);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.readRecordLayer().get(), mockRead_);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.writeRecordLayer().get(), mockWrite_);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Plaintext);
  EXPECT_EQ(state_.keyExchangers()->size(), 1);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::secp256r1).get(), mockKex);
  EXPECT_EQ(state_.verifier(), verifier_);
  EXPECT_EQ(*state_.sni(), "www.hostname.com");
  EXPECT_EQ(state_.echState()->sni, "www.hostname.com");
  EXPECT_EQ(state_.echState()->status, ECHStatus::Accepted);
  Random random;
  random.fill(0x66);
  EXPECT_EQ(*state_.clientRandom(), random);
  EXPECT_EQ(state_.echState()->random, random);
  EXPECT_FALSE(state_.sentCCS());
  EXPECT_EQ(state_.version(), TestProtocolVersion);
  EXPECT_EQ(state_.cipher(), CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_EQ(state_.keyExchangeType(), KeyExchangeType::HelloRetryRequest);
  EXPECT_EQ(state_.attemptedPsk()->psk, psk.psk);
  EXPECT_EQ(state_.earlyDataType(), EarlyDataType::NotAttempted);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequest) {
  setupExpectingServerHello();
  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestPskDifferentHash) {
  setupExpectingServerHello();
  state_.attemptedPsk() = getCachedPsk();
  state_.attemptedPsk()->cipher = CipherSuite::TLS_AES_256_GCM_SHA384;
  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.attemptedPsk().has_value());
}

TEST_F(ClientProtocolTest, TestDoubleHelloRetryRequest) {
  setupExpectingServerHello();
  state_.keyExchangeType() = KeyExchangeType::HelloRetryRequest;
  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectError<FizzException>(
      actions, AlertDescription::unexpected_message, "two HRRs");
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestBadVersion) {
  setupExpectingServerHello();
  auto hrr = TestMessages::helloRetryRequest();
  TestMessages::removeExtension(hrr, ExtensionType::supported_versions);
  ServerSupportedVersions supportedVersions;
  supportedVersions.selected_version = ProtocolVersion::tls_1_1;
  hrr.extensions.push_back(encodeExtension(std::move(supportedVersions)));
  auto actions = detail::processEvent(state_, std::move(hrr));
  expectError<FizzException>(
      actions,
      AlertDescription::protocol_version,
      "unsupported server version");
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestBadCipher) {
  setupExpectingServerHello();
  auto hrr = TestMessages::helloRetryRequest();
  hrr.cipher_suite = static_cast<CipherSuite>(0x03ff);
  auto actions = detail::processEvent(state_, std::move(hrr));
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "unsupported cipher");
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestBadGroup) {
  setupExpectingServerHello();
  auto hrr = TestMessages::helloRetryRequest();
  TestMessages::removeExtension(hrr, ExtensionType::key_share);
  HelloRetryRequestKeyShare keyShare;
  keyShare.selected_group = static_cast<NamedGroup>(0x8923);
  hrr.extensions.push_back(encodeExtension(std::move(keyShare)));
  auto actions = detail::processEvent(state_, std::move(hrr));
  expectError<FizzException>(
      actions, AlertDescription::handshake_failure, "unsupported group");
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestGroupAlreadySent) {
  setupExpectingServerHello();
  auto hrr = TestMessages::helloRetryRequest();
  TestMessages::removeExtension(hrr, ExtensionType::key_share);
  HelloRetryRequestKeyShare keyShare;
  keyShare.selected_group = NamedGroup::x25519;
  hrr.extensions.push_back(encodeExtension(std::move(keyShare)));

  auto actions = detail::processEvent(state_, std::move(hrr));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "already-sent group");
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestNoKeyShare) {
  setupExpectingServerHello();
  auto kex = state_.keyExchangers()->at(NamedGroup::x25519).get();
  auto mockKex = std::make_unique<MockKeyExchange>();
  mockKex->setDefaults();
  state_.keyExchangers()->emplace(NamedGroup::secp256r1, std::move(mockKex));
  auto hrr = TestMessages::helloRetryRequest();
  TestMessages::removeExtension(hrr, ExtensionType::key_share);
  auto actions = detail::processEvent(state_, std::move(hrr));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_FALSE(state_.group().has_value());
  EXPECT_EQ(state_.keyExchangers()->size(), 2);
  EXPECT_EQ(state_.keyExchangers()->at(NamedGroup::x25519).get(), kex);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestCookie) {
  setupExpectingServerHello();
  auto hrr = TestMessages::helloRetryRequest();
  Cookie cookie;
  cookie.cookie = folly::IOBuf::copyBuffer("cookie!!");
  hrr.extensions.push_back(encodeExtension(std::move(cookie)));
  auto actions = detail::processEvent(state_, std::move(hrr));
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  auto range = (*state_.encodedClientHello())->coalesce();
  EXPECT_THAT(std::string(range.begin(), range.end()), HasSubstr("cookie!!"));
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestAttemptedEarly) {
  setupExpectingServerHello();
  state_.earlyDataType() = EarlyDataType::Attempted;
  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_EQ(state_.earlyDataType(), EarlyDataType::Rejected);
  EXPECT_EQ(state_.earlyWriteRecordLayer(), nullptr);
}

TEST_F(ClientProtocolTest, TestHelloRetryRequestCompat) {
  context_->setCompatibilityMode(true);
  setupExpectingServerHello();
  auto actions =
      detail::processEvent(state_, TestMessages::helloRetryRequest());
  expectActions<MutateState, WriteToSocket>(actions);
  processStateMutations(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents.size(), 2);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Plaintext);
  EXPECT_EQ(write.contents[0].contentType, ContentType::change_cipher_spec);
  EXPECT_EQ(write.contents[1].encryptionLevel, EncryptionLevel::Plaintext);
  EXPECT_EQ(write.contents[1].contentType, ContentType::handshake);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingServerHello);
  EXPECT_TRUE(state_.sentCCS());
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsFlow) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("eeencoding")));

  auto actions = detail::processEvent(state_, TestMessages::encryptedExt());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(*state_.alpn(), "h2");
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificate);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsPsk) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  state_.serverCert() = mockLeaf_;
  state_.pskType() = PskType::Resumption;
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("eeencoding")));

  auto actions = detail::processEvent(state_, TestMessages::encryptedExt());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(*state_.alpn(), "h2");
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
  EXPECT_EQ(state_.serverCert(), mockLeaf_);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsPskExternalNoCert) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  state_.pskType() = PskType::External;
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("eeencoding")));

  auto actions = detail::processEvent(state_, TestMessages::encryptedExt());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(*state_.alpn(), "h2");
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
  EXPECT_EQ(state_.pskType(), PskType::External);
  EXPECT_EQ(state_.serverCert(), nullptr);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsAlpn) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  auto actions = detail::processEvent(state_, TestMessages::encryptedExt());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(*state_.alpn(), "h2");
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificate);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEmptyAlpn) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  auto ee = TestMessages::encryptedExt();
  TestMessages::removeExtension(
      ee, ExtensionType::application_layer_protocol_negotiation);
  ee.extensions.push_back(encodeExtension(ProtocolNameList()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "alpn list");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsAlpnMismatch) {
  context_->setSupportedAlpns({"h3", "h1"});
  setupExpectingEncryptedExtensions();
  auto actions = detail::processEvent(state_, TestMessages::encryptedExt());
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "alpn mismatch");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsNoAlpn) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  auto ee = TestMessages::encryptedExt();
  TestMessages::removeExtension(
      ee, ExtensionType::application_layer_protocol_negotiation);
  auto actions = detail::processEvent(state_, std::move(ee));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_FALSE(state_.alpn().has_value());
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificate);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsRequireAlpn) {
  context_->setSupportedAlpns({"h2"});
  context_->setRequireAlpn(true);
  setupExpectingEncryptedExtensions();
  auto ee = TestMessages::encryptedExt();
  TestMessages::removeExtension(
      ee, ExtensionType::application_layer_protocol_negotiation);
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions, AlertDescription::no_application_protocol, "alpn is required");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsDisallowedExtension) {
  setupExpectingEncryptedExtensions();
  auto ee = TestMessages::encryptedExt();
  ee.extensions.push_back(encodeExtension(ClientPresharedKey()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "unexpected extension in ee: pre_shared_key");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsUnrequestedExtension) {
  setupExpectingEncryptedExtensions();
  state_.requestedExtensions() = std::vector<ExtensionType>(
      {ExtensionType::key_share,
       ExtensionType::application_layer_protocol_negotiation,
       ExtensionType::pre_shared_key});
  auto ee = TestMessages::encryptedExt();
  ee.extensions.push_back(encodeExtension(ServerNameList()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "unexpected extension in ee: server_name");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEarlyAccepted) {
  setupExpectingEncryptedExtensionsEarlySent();
  auto ee = TestMessages::encryptedExt();
  ee.extensions.push_back(encodeExtension(ServerEarlyData()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Accepted);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEarlyRejected) {
  setupExpectingEncryptedExtensionsEarlySent();
  auto ee = TestMessages::encryptedExt();
  auto actions = detail::processEvent(state_, std::move(ee));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Rejected);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEarlyAlreadyRejected) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.earlyDataType() = EarlyDataType::Rejected;
  auto ee = TestMessages::encryptedExt();
  auto actions = detail::processEvent(state_, std::move(ee));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
  EXPECT_EQ(*state_.earlyDataType(), EarlyDataType::Rejected);
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEarlyAcceptedHrr) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.earlyDataType() = EarlyDataType::Rejected;
  auto ee = TestMessages::encryptedExt();
  ee.extensions.push_back(encodeExtension(ServerEarlyData()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "unexpected accepted early data");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEarlyCipherMismatch) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.cipher() = CipherSuite::TLS_CHACHA20_POLY1305_SHA256;
  auto ee = TestMessages::encryptedExt();
  ee.extensions.push_back(encodeExtension(ServerEarlyData()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "different cipher");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsEarlyAlpnMismatch) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.earlyDataParams()->alpn = "h3";
  state_.attemptedPsk()->alpn = "h3";
  auto ee = TestMessages::encryptedExt();
  ee.extensions.push_back(encodeExtension(ServerEarlyData()));
  auto actions = detail::processEvent(state_, std::move(ee));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "different alpn");
}

TEST_F(ClientProtocolTest, TestEncryptedExtensionsECHRetryConfigs) {
  context_->setSupportedAlpns({"h2"});
  setupExpectingEncryptedExtensions();
  state_.requestedExtensions()->push_back(
      ExtensionType::encrypted_client_hello);
  state_.echState().emplace();
  state_.echState()->status = ECHStatus::Rejected;
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("eeencoding")));

  auto ee = TestMessages::encryptedExt();
  ech::ECHEncryptedExtensions serverECH;
  ech::ECHConfig cfg;
  cfg.version = ech::ECHVersion::Draft15;
  cfg.ech_config_content = folly::IOBuf::copyBuffer("retryconfig");
  serverECH.retry_configs.push_back(std::move(cfg));
  ee.extensions.push_back(encodeExtension(std::move(serverECH)));

  auto actions = detail::processEvent(state_, std::move(ee));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(*state_.alpn(), "h2");
  EXPECT_TRUE(folly::IOBufEqualTo()(
      state_.echState()->retryConfigs.value()[0].ech_config_content,
      folly::IOBuf::copyBuffer("retryconfig")));
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificate);
}

TEST_F(ClientProtocolTest, TestCertificateFlow) {
  setupExpectingCertificate();
  EXPECT_CALL(
      *mockHandshakeContext_, appendToTranscript(BufMatches("certencoding")));
  mockLeaf_ = std::make_shared<MockPeerCert>();
  mockIntermediate_ = std::make_shared<MockPeerCert>();
  EXPECT_CALL(*factory_, _makePeerCert(CertEntryBufMatches("cert1"), true))
      .WillOnce(Return(mockLeaf_));
  EXPECT_CALL(*factory_, _makePeerCert(CertEntryBufMatches("cert2"), false))
      .WillOnce(Return(mockIntermediate_));

  auto certificate = TestMessages::certificate();
  CertificateEntry entry1;
  entry1.cert_data = folly::IOBuf::copyBuffer("cert1");
  certificate.certificate_list.push_back(std::move(entry1));
  CertificateEntry entry2;
  entry2.cert_data = folly::IOBuf::copyBuffer("cert2");
  certificate.certificate_list.push_back(std::move(entry2));
  auto actions = detail::processEvent(state_, std::move(certificate));

  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.unverifiedCertChain()->size(), 2);
  EXPECT_EQ(state_.unverifiedCertChain()->at(0), mockLeaf_);
  EXPECT_EQ(state_.unverifiedCertChain()->at(1), mockIntermediate_);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificateVerify);
}

TEST_F(ClientProtocolTest, TestCertificate) {
  setupExpectingCertificate();
  auto certificate = TestMessages::certificate();
  CertificateEntry entry;
  entry.cert_data = folly::IOBuf::copyBuffer("cert");
  certificate.certificate_list.push_back(std::move(entry));
  auto actions = detail::processEvent(state_, std::move(certificate));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.unverifiedCertChain()->size(), 1);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificateVerify);
}

TEST_F(ClientProtocolTest, TestCertificateWithRequestContext) {
  setupExpectingCertificate();
  auto certificate = TestMessages::certificate();
  certificate.certificate_request_context =
      folly::IOBuf::copyBuffer("something");
  CertificateEntry entry;
  entry.cert_data = folly::IOBuf::copyBuffer("cert");
  certificate.certificate_list.push_back(std::move(entry));
  auto actions = detail::processEvent(state_, std::move(certificate));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "context must be empty");
}

TEST_F(ClientProtocolTest, TestCertificateEmpty) {
  setupExpectingCertificate();
  auto certificate = TestMessages::certificate();
  auto actions = detail::processEvent(state_, std::move(certificate));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "no cert");
}

TEST_F(ClientProtocolTest, TestCertificateExtensions) {
  setupExpectingCertificate();
  // Set up cert with an extension.
  auto certificate = TestMessages::certificate();
  CertificateEntry entry;
  entry.cert_data = folly::IOBuf::copyBuffer("cert");
  Extension certExt;
  certExt.extension_type = static_cast<fizz::ExtensionType>(0xbeef);
  certExt.extension_data = folly::IOBuf::create(0);
  entry.extensions.push_back(std::move(certExt));
  certificate.certificate_list.push_back(std::move(entry));
  auto ext = std::make_shared<MockClientExtensions>();
  EXPECT_CALL(*ext, getClientHelloExtensions())
      .WillOnce(InvokeWithoutArgs([]() {
        Extension ext;
        ext.extension_type = static_cast<fizz::ExtensionType>(0xbeef);
        ext.extension_data = folly::IOBuf::create(0);
        std::vector<Extension> exts;
        exts.push_back(std::move(ext));
        return exts;
      }));
  state_.extensions() = ext;
  auto actions = detail::processEvent(state_, std::move(certificate));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.unverifiedCertChain()->size(), 1);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificateVerify);
}

TEST_F(ClientProtocolTest, TestCertificateUnrequestedExtensions) {
  setupExpectingCertificate();
  // Set up cert with an extension.
  auto certificate = TestMessages::certificate();
  CertificateEntry entry;
  entry.cert_data = folly::IOBuf::copyBuffer("cert");
  Extension certExt;
  certExt.extension_type = static_cast<fizz::ExtensionType>(0xbeef);
  certExt.extension_data = folly::IOBuf::create(0);
  entry.extensions.push_back(std::move(certExt));
  certificate.certificate_list.push_back(std::move(entry));
  auto ext = std::make_shared<MockClientExtensions>();
  EXPECT_CALL(*ext, getClientHelloExtensions())
      .WillOnce(InvokeWithoutArgs([]() {
        Extension ext;
        // Different type here
        ext.extension_type = static_cast<fizz::ExtensionType>(0xface);
        ext.extension_data = folly::IOBuf::create(0);
        std::vector<Extension> exts;
        exts.push_back(std::move(ext));
        return exts;
      }));
  state_.extensions() = ext;
  auto actions = detail::processEvent(state_, std::move(certificate));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "unrequested certificate extension");
}

TEST_F(ClientProtocolTest, TestCertificateNoExtensionsSent) {
  setupExpectingCertificate();
  // Set up cert with an extension.
  auto certificate = TestMessages::certificate();
  CertificateEntry entry;
  entry.cert_data = folly::IOBuf::copyBuffer("cert");
  Extension certExt;
  certExt.extension_type = static_cast<fizz::ExtensionType>(0xbeef);
  certExt.extension_data = folly::IOBuf::create(0);
  entry.extensions.push_back(std::move(certExt));
  certificate.certificate_list.push_back(std::move(entry));
  auto actions = detail::processEvent(state_, std::move(certificate));
  expectError<FizzException>(
      actions,
      AlertDescription::illegal_parameter,
      "certificate extensions must be empty");
}

TEST_F(ClientProtocolTest, TestCompressedCertificateFlow) {
  setupExpectingCertificate();
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches("compcertencoding")));
  mockLeaf_ = std::make_shared<MockPeerCert>();
  mockIntermediate_ = std::make_shared<MockPeerCert>();
  EXPECT_CALL(*factory_, _makePeerCert(CertEntryBufMatches("cert1"), true))
      .WillOnce(Return(mockLeaf_));
  EXPECT_CALL(*factory_, _makePeerCert(CertEntryBufMatches("cert2"), false))
      .WillOnce(Return(mockIntermediate_));

  auto decompressor = std::make_shared<MockCertificateDecompressor>();
  decompressor->setDefaults();
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  context_->setCertDecompressionManager(std::move(decompressionMgr));
  EXPECT_CALL(*decompressor, decompress(_))
      .WillOnce(Invoke([](const CompressedCertificate& cc) {
        EXPECT_TRUE(folly::IOBufEqualTo()(
            cc.compressed_certificate_message,
            folly::IOBuf::copyBuffer("compressedcerts")));
        EXPECT_EQ(cc.algorithm, CertificateCompressionAlgorithm::zlib);
        EXPECT_EQ(cc.uncompressed_length, 0x111111);
        auto certificate = TestMessages::certificate();
        CertificateEntry entry1;
        entry1.cert_data = folly::IOBuf::copyBuffer("cert1");
        certificate.certificate_list.push_back(std::move(entry1));
        CertificateEntry entry2;
        entry2.cert_data = folly::IOBuf::copyBuffer("cert2");
        certificate.certificate_list.push_back(std::move(entry2));
        return certificate;
      }));

  auto compressedCert = TestMessages::compressedCertificate();
  auto actions = detail::processEvent(state_, std::move(compressedCert));

  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.unverifiedCertChain()->size(), 2);
  EXPECT_EQ(state_.unverifiedCertChain()->at(0), mockLeaf_);
  EXPECT_EQ(state_.unverifiedCertChain()->at(1), mockIntermediate_);
  EXPECT_EQ(state_.serverCertCompAlgo(), CertificateCompressionAlgorithm::zlib);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificateVerify);
}

TEST_F(ClientProtocolTest, TestCompressedCertificate) {
  setupExpectingCertificate();
  auto decompressor = std::make_shared<MockCertificateDecompressor>();
  decompressor->setDefaults();
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  context_->setCertDecompressionManager(std::move(decompressionMgr));
  EXPECT_CALL(*decompressor, decompress(_))
      .WillOnce(Invoke([](const CompressedCertificate& cc) {
        auto certificate = TestMessages::certificate();
        CertificateEntry entry;
        entry.cert_data = folly::IOBuf::copyBuffer("cert");
        certificate.certificate_list.push_back(std::move(entry));
        return certificate;
      }));

  auto compressedCert = TestMessages::compressedCertificate();
  auto actions = detail::processEvent(state_, std::move(compressedCert));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.unverifiedCertChain()->size(), 1);
  EXPECT_EQ(state_.serverCertCompAlgo(), CertificateCompressionAlgorithm::zlib);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCertificateVerify);
}

TEST_F(ClientProtocolTest, TestCompressedCertificateUnknownAlgo) {
  setupExpectingCertificate();
  auto decompressor = std::make_shared<MockCertificateDecompressor>();
  decompressor->setDefaults();
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  context_->setCertDecompressionManager(std::move(decompressionMgr));

  auto compressedCert = TestMessages::compressedCertificate();
  compressedCert.algorithm = static_cast<CertificateCompressionAlgorithm>(0xff);
  auto actions = detail::processEvent(state_, std::move(compressedCert));
  expectError<FizzException>(
      actions, AlertDescription::bad_certificate, "unsupported algorithm");
}

TEST_F(ClientProtocolTest, TestCompressedCertificateDecompressionFailed) {
  setupExpectingCertificate();
  auto decompressor = std::make_shared<MockCertificateDecompressor>();
  decompressor->setDefaults();
  EXPECT_CALL(*decompressor, decompress(_))
      .WillOnce(Invoke([](const CompressedCertificate& cc) -> CertificateMsg {
        throw std::runtime_error("foo");
      }));
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  context_->setCertDecompressionManager(std::move(decompressionMgr));
  auto compressedCert = TestMessages::compressedCertificate();
  auto actions = detail::processEvent(state_, std::move(compressedCert));
  expectError<FizzException>(
      actions, AlertDescription::bad_certificate, "decompression failed: foo");
}

TEST_F(ClientProtocolTest, TestCompressedCertificateWithRequestContext) {
  setupExpectingCertificate();
  auto decompressor = std::make_shared<MockCertificateDecompressor>();
  decompressor->setDefaults();
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  context_->setCertDecompressionManager(std::move(decompressionMgr));
  EXPECT_CALL(*decompressor, decompress(_))
      .WillOnce(Invoke([](const CompressedCertificate& cc) {
        auto certificate = TestMessages::certificate();
        certificate.certificate_request_context =
            folly::IOBuf::copyBuffer("something");
        CertificateEntry entry;
        entry.cert_data = folly::IOBuf::copyBuffer("cert");
        certificate.certificate_list.push_back(std::move(entry));
        return certificate;
      }));

  auto compressedCert = TestMessages::compressedCertificate();
  auto actions = detail::processEvent(state_, std::move(compressedCert));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "context must be empty");
}

TEST_F(ClientProtocolTest, TestCompressedCertificateEmpty) {
  setupExpectingCertificate();
  auto decompressor = std::make_shared<MockCertificateDecompressor>();
  decompressor->setDefaults();
  EXPECT_CALL(*decompressor, decompress(_))
      .WillOnce(Invoke([](const CompressedCertificate& cc) {
        return TestMessages::certificate();
      }));
  auto decompressionMgr = std::make_shared<CertDecompressionManager>();
  decompressionMgr->setDecompressors(
      {std::static_pointer_cast<CertificateDecompressor>(decompressor)});
  context_->setCertDecompressionManager(std::move(decompressionMgr));
  auto compressedCert = TestMessages::compressedCertificate();
  auto actions = detail::processEvent(state_, std::move(compressedCert));
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "no cert");
}

TEST_F(ClientProtocolTest, TestUnexpectedCompressedCertificate) {
  setupExpectingCertificate();
  auto compressedCert = TestMessages::compressedCertificate();
  auto actions = detail::processEvent(state_, std::move(compressedCert));
  expectError<FizzException>(
      actions, AlertDescription::unexpected_message, "received unexpectedly");
}

TEST_F(ClientProtocolTest, TestCertificateVerifyFlow) {
  setupExpectingCertificateVerify();
  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("certcontext"); }));
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches("certverifyencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(
      *mockLeaf_,
      verify(
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          RangeMatches("certcontext"),
          RangeMatches("signature")));
  EXPECT_CALL(*verifier_, verify(_))
      .WillOnce(Invoke(
          [this](const std::vector<std::shared_ptr<const PeerCert>>& certs) {
            EXPECT_EQ(certs.size(), 2);
            EXPECT_EQ(certs[0], mockLeaf_);
            EXPECT_EQ(certs[1], mockIntermediate_);
            return certs.front();
          }));

  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.sigScheme(), SignatureScheme::ecdsa_secp256r1_sha256);
  EXPECT_EQ(state_.serverCert(), mockLeaf_);
  EXPECT_FALSE(state_.unverifiedCertChain().has_value());
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
}

TEST_F(ClientProtocolTest, TestCertificateVerify) {
  setupExpectingCertificateVerify();
  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
}

TEST_F(ClientProtocolTest, TestCertificateVerifyNoVerifier) {
  setupExpectingCertificateVerify();
  state_.verifier() = nullptr;
  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingFinished);
}

TEST_F(ClientProtocolTest, TestCertificateVerifyUnsupportedAlgorithm) {
  context_->setSupportedSigSchemes({SignatureScheme::rsa_pss_sha256});
  setupExpectingCertificateVerify();
  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectError<FizzException>(
      actions, AlertDescription::illegal_parameter, "unsupported sig scheme");
}

TEST_F(ClientProtocolTest, TestCertificateVerifyFailure) {
  setupExpectingCertificateVerify();
  EXPECT_CALL(
      *mockLeaf_,
      verify(
          SignatureScheme::ecdsa_secp256r1_sha256,
          CertificateVerifyContext::Server,
          RangeMatches("context"),
          RangeMatches("signature")))
      .WillOnce(Throw(
          FizzException("verify failed", AlertDescription::bad_record_mac)));
  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectError<FizzException>(
      actions, AlertDescription::bad_record_mac, "verify failed");
}

TEST_F(ClientProtocolTest, TestCertificateVerifyVerifierFailure) {
  setupExpectingCertificateVerify();
  EXPECT_CALL(*verifier_, verify(_))
      .WillOnce(Throw(FizzVerificationException(
          "verify failed", AlertDescription::bad_record_mac)));
  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectError<FizzVerificationException>(
      actions, AlertDescription::bad_record_mac, "verify failed");
}

TEST_F(ClientProtocolTest, TestCertificateVerifyVerifierFailureOtherException) {
  setupExpectingCertificateVerify();
  EXPECT_CALL(*verifier_, verify(_))
      .WillOnce(Throw(std::runtime_error("no good")));
  auto actions =
      detail::processEvent(state_, TestMessages::certificateVerify());
  expectError<FizzException>(
      actions, AlertDescription::bad_certificate, "verifier failure: no good");
}

TEST_F(ClientProtocolTest, TestCertificateRequestNoCert) {
  setupExpectingCertificate();
  auto certificateRequest = TestMessages::certificateRequest();
  auto actions = detail::processEvent(state_, std::move(certificateRequest));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.clientAuthRequested(), ClientAuthType::RequestedNoMatch);
  EXPECT_EQ(state_.selectedClientCert(), nullptr);
  EXPECT_EQ(state_.clientAuthSigScheme(), folly::none);
}

TEST_F(ClientProtocolTest, TestCertificateRequestDuplicated) {
  setupExpectingCertificate();
  auto certificateRequest = TestMessages::certificateRequest();
  auto actions = detail::processEvent(state_, std::move(certificateRequest));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  certificateRequest = TestMessages::certificateRequest();
  actions = detail::processEvent(state_, std::move(certificateRequest));
  expectError<FizzException>(
      actions,
      AlertDescription::unexpected_message,
      "duplicate certificate request message");
}

TEST_F(ClientProtocolTest, TestCertificateRequestAlgosMismatch) {
  setupExpectingCertificateRequest();
  auto certificateRequest = TestMessages::certificateRequest();

  SignatureAlgorithms sigAlgs;
  sigAlgs.supported_signature_algorithms = {
      SignatureScheme::ecdsa_secp256r1_sha256};
  certificateRequest.extensions.clear();
  certificateRequest.extensions.emplace_back(
      encodeExtension(std::move(sigAlgs)));

  EXPECT_CALL(*mockClientCert_, getSigSchemes())
      .WillOnce(Return(
          std::vector<SignatureScheme>(1, SignatureScheme::rsa_pss_sha256)));

  auto actions = detail::processEvent(state_, std::move(certificateRequest));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.clientAuthRequested(), ClientAuthType::RequestedNoMatch);
  EXPECT_EQ(state_.selectedClientCert(), nullptr);
  EXPECT_EQ(state_.clientAuthSigScheme(), folly::none);
}

TEST_F(ClientProtocolTest, TestCertificateRequestContextAlgosUnsupported) {
  setupExpectingCertificateRequest();
  context_->setSupportedSigSchemes({SignatureScheme::rsa_pss_sha512});
  auto certificateRequest = TestMessages::certificateRequest();

  SignatureAlgorithms sigAlgs;
  sigAlgs.supported_signature_algorithms = {
      SignatureScheme::ecdsa_secp256r1_sha256, SignatureScheme::rsa_pss_sha256};
  certificateRequest.extensions.clear();
  certificateRequest.extensions.emplace_back(
      encodeExtension(std::move(sigAlgs)));

  EXPECT_CALL(*mockClientCert_, getSigSchemes())
      .WillOnce(Return(
          std::vector<SignatureScheme>(1, SignatureScheme::rsa_pss_sha256)));

  auto actions = detail::processEvent(state_, std::move(certificateRequest));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.clientAuthRequested(), ClientAuthType::RequestedNoMatch);
  EXPECT_EQ(state_.selectedClientCert(), nullptr);
  EXPECT_EQ(state_.clientAuthSigScheme(), folly::none);
}

TEST_F(ClientProtocolTest, TestCertificateRequestPrefersContextOrder) {
  setupExpectingCertificateRequest();
  context_->setSupportedSigSchemes(
      {SignatureScheme::rsa_pss_sha512,
       SignatureScheme::ecdsa_secp521r1_sha512,
       SignatureScheme::ed25519});
  auto certificateRequest = TestMessages::certificateRequest();
  SignatureAlgorithms requestAlgos;
  requestAlgos.supported_signature_algorithms = {
      SignatureScheme::ecdsa_secp521r1_sha512,
      SignatureScheme::ed25519,
      SignatureScheme::rsa_pss_sha512};
  certificateRequest.extensions.clear();
  certificateRequest.extensions.emplace_back(
      encodeExtension(std::move(requestAlgos)));

  EXPECT_CALL(*mockClientCert_, getSigSchemes())
      .WillOnce(Return(std::vector<SignatureScheme>(
          {SignatureScheme::ed25519,
           SignatureScheme::ecdsa_secp521r1_sha512,
           SignatureScheme::rsa_pss_sha512})));

  auto actions = detail::processEvent(state_, std::move(certificateRequest));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.clientAuthRequested(), ClientAuthType::Sent);
  EXPECT_EQ(state_.selectedClientCert(), mockClientCert_);
  EXPECT_EQ(state_.clientAuthSigScheme(), SignatureScheme::rsa_pss_sha512);
}

TEST_F(ClientProtocolTest, TestCertificateRequestMatch) {
  setupExpectingCertificateRequest();
  auto certificateRequest = TestMessages::certificateRequest();

  EXPECT_CALL(*mockClientCert_, getSigSchemes())
      .WillOnce(Return(
          std::vector<SignatureScheme>(1, SignatureScheme::rsa_pss_sha256)));

  auto actions = detail::processEvent(state_, std::move(certificateRequest));
  expectActions<MutateState>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.clientAuthRequested(), ClientAuthType::Sent);
  EXPECT_EQ(state_.selectedClientCert(), mockClientCert_);
  EXPECT_EQ(state_.clientAuthSigScheme(), SignatureScheme::rsa_pss_sha256);
}

TEST_F(ClientProtocolTest, TestFinishedFlow) {
  setupExpectingFinished();
  doFinishedFlow(ClientAuthType::NotRequested);
}

TEST_F(ClientProtocolTest, TestFinishedCertNoMatchFlow) {
  setupExpectingFinished();
  state_.clientAuthRequested() = ClientAuthType::RequestedNoMatch;
  doFinishedFlow(ClientAuthType::RequestedNoMatch);
  EXPECT_EQ(state_.clientCert(), nullptr);
}

TEST_F(ClientProtocolTest, TestFinishedCertSentFlow) {
  setupExpectingFinished();
  state_.clientAuthRequested() = ClientAuthType::Sent;
  state_.clientAuthSigScheme() = SignatureScheme::ecdsa_secp256r1_sha256;
  state_.selectedClientCert() = mockClientCert_;
  doFinishedFlow(ClientAuthType::Sent);
  EXPECT_EQ(state_.clientCert(), mockClientCert_);
}

TEST_F(ClientProtocolTest, TestFinishedEarlyFlow) {
  setupExpectingFinished();
  state_.earlyDataType() = EarlyDataType::Accepted;
  setMockEarlyRecord();

  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("sht")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("verifydata"); }));
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches("finishedencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("sfincontext"); }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("sfin_eoed"); }));
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("cht")))
      .InSequence(contextSeq)
      .WillRepeatedly(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("verifydata"); }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("fincontext"); }));
  EXPECT_CALL(*mockHandshakeWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockHandshakeWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, encodeHandshake(TestMessages::finished())));
        content.data = folly::IOBuf::copyBuffer("finwrite");
        return content;
      }));
  EXPECT_CALL(*mockEarlyWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockEarlyWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, encodeHandshake(EndOfEarlyData())));
        content.data = folly::IOBuf::copyBuffer("eoed");
        return content;
      }));
  EXPECT_CALL(*mockKeyScheduler_, deriveMasterSecret());
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(MasterSecrets::ExporterMaster, RangeMatches("sfincontext")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(MasterSecrets::ResumptionMaster, RangeMatches("fincontext")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'r', 'e', 's'}),
            MasterSecrets::ResumptionMaster);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveAppTrafficSecrets(RangeMatches("sfincontext")));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ServerAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'a', 't'}),
            AppTrafficSecrets::ServerAppTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ClientAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'a', 't'}),
            AppTrafficSecrets::ClientAppTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sat"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cat"));
  EXPECT_CALL(*mockKeyScheduler_, clearMasterSecret());

  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectActions<
      MutateState,
      ReportHandshakeSuccess,
      WriteToSocket,
      SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents.size(), 2);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::EarlyData);
  EXPECT_EQ(write.contents[0].contentType, ContentType::handshake);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("eoed")));

  EXPECT_EQ(write.contents[1].encryptionLevel, EncryptionLevel::Handshake);
  EXPECT_EQ(write.contents[1].contentType, ContentType::handshake);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[1].data, folly::IOBuf::copyBuffer("finwrite")));
  auto reportSuccess = expectAction<ReportHandshakeSuccess>(actions);
  EXPECT_EQ(reportSuccess.earlyDataAccepted, true);

  expectSecret(
      actions, AppTrafficSecrets::ServerAppTraffic, folly::StringPiece("sat"));
  expectSecret(
      actions, AppTrafficSecrets::ClientAppTraffic, folly::StringPiece("cat"));
  processStateMutations(actions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.earlyWriteRecordLayer().get(), nullptr);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.resumptionSecret(), folly::IOBuf::copyBuffer("res")));
}

TEST_F(ClientProtocolTest, TestFinishedEarlyFlowOmitEarlyRecord) {
  setupExpectingFinished();
  state_.earlyDataType() = EarlyDataType::Accepted;
  context_->setOmitEarlyRecordLayer(true);

  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("sht")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("verifydata"); }));
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches("finishedencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("sfin"); }));
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("cht")))
      .InSequence(contextSeq)
      .WillRepeatedly(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("verifydata"); }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("fincontext"); }));
  EXPECT_CALL(*mockHandshakeWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockHandshakeWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, encodeHandshake(TestMessages::finished())));
        content.data = folly::IOBuf::copyBuffer("finwrite");
        return content;
      }));
  EXPECT_CALL(*mockKeyScheduler_, deriveMasterSecret());
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(MasterSecrets::ExporterMaster, RangeMatches("sfin")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(MasterSecrets::ResumptionMaster, RangeMatches("fincontext")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'r', 'e', 's'}),
            MasterSecrets::ResumptionMaster);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveAppTrafficSecrets(RangeMatches("sfin")));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ServerAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'a', 't'}),
            AppTrafficSecrets::ServerAppTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ClientAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'a', 't'}),
            AppTrafficSecrets::ClientAppTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sat"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cat"));
  EXPECT_CALL(*mockKeyScheduler_, clearMasterSecret());

  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectActions<
      MutateState,
      ReportHandshakeSuccess,
      WriteToSocket,
      SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents.size(), 1);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Handshake);
  EXPECT_EQ(write.contents[0].contentType, ContentType::handshake);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("finwrite")));
  auto reportSuccess = expectAction<ReportHandshakeSuccess>(actions);
  EXPECT_EQ(reportSuccess.earlyDataAccepted, true);

  expectSecret(
      actions, AppTrafficSecrets::ServerAppTraffic, folly::StringPiece("sat"));
  expectSecret(
      actions, AppTrafficSecrets::ClientAppTraffic, folly::StringPiece("cat"));
  processStateMutations(actions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.earlyWriteRecordLayer().get(), nullptr);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.resumptionSecret(), folly::IOBuf::copyBuffer("res")));
}

void ClientProtocolTest::doFinishedFlow(ClientAuthType authType) {
  Sequence contextSeq;
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("sht")))
      .InSequence(contextSeq)
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("verifydata"); }));
  EXPECT_CALL(
      *mockHandshakeContext_,
      appendToTranscript(BufMatches("finishedencoding")))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("sfincontext"); }));
  if (authType != ClientAuthType::NotRequested) {
    if (authType == ClientAuthType::Sent) {
      EXPECT_CALL(*mockClientCert_, _getCertMessage(_))
          .InSequence(contextSeq)
          .WillOnce(
              InvokeWithoutArgs([]() { return TestMessages::certificate(); }));
    }
    EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
        .InSequence(contextSeq)
        .WillOnce(Invoke([authType](const Buf& enc) {
          if (authType == ClientAuthType::Sent) {
            EXPECT_TRUE(folly::IOBufEqualTo()(
                enc, encodeHandshake(TestMessages::certificate())));
          } else {
            EXPECT_TRUE(
                folly::IOBufEqualTo()(enc, encodeHandshake(CertificateMsg())));
          }
        }));
    if (authType == ClientAuthType::Sent) {
      EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
          .InSequence(contextSeq)
          .WillRepeatedly(Invoke(
              []() { return folly::IOBuf::copyBuffer("csentcontext"); }));
      EXPECT_CALL(
          *mockClientCert_,
          sign(
              SignatureScheme::ecdsa_secp256r1_sha256,
              CertificateVerifyContext::Client,
              RangeMatches("csentcontext")))
          .InSequence(contextSeq)
          .WillOnce(InvokeWithoutArgs(
              []() { return folly::IOBuf::copyBuffer("signature"); }));
      EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
          .InSequence(contextSeq)
          .WillOnce(Invoke([](const Buf& enc) {
            EXPECT_TRUE(folly::IOBufEqualTo()(
                enc, encodeHandshake(TestMessages::certificateVerify())));
          }));
    }
  }
  EXPECT_CALL(*mockHandshakeContext_, getFinishedData(RangeMatches("cht")))
      .InSequence(contextSeq)
      .WillRepeatedly(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("verifydata"); }));
  EXPECT_CALL(*mockHandshakeContext_, appendToTranscript(_))
      .InSequence(contextSeq);
  EXPECT_CALL(*mockHandshakeContext_, getHandshakeContext())
      .InSequence(contextSeq)
      .WillRepeatedly(
          Invoke([]() { return folly::IOBuf::copyBuffer("fincontext"); }));
  EXPECT_CALL(*mockHandshakeWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockHandshakeWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);

        switch (authType) {
          case ClientAuthType::NotRequested:
          case ClientAuthType::Stored:
            EXPECT_TRUE(folly::IOBufEqualTo()(
                msg.fragment, encodeHandshake(TestMessages::finished())));
            break;
          case ClientAuthType::RequestedNoMatch: {
            auto expectedMessages = encodeHandshake(CertificateMsg());
            expectedMessages->prependChain(
                encodeHandshake(TestMessages::finished()));
            EXPECT_TRUE(folly::IOBufEqualTo()(msg.fragment, expectedMessages));
            break;
          }
          case ClientAuthType::Sent: {
            auto expectedMessages =
                encodeHandshake(TestMessages::certificate());
            expectedMessages->prependChain(
                encodeHandshake(TestMessages::certificateVerify()));
            expectedMessages->prependChain(
                encodeHandshake(TestMessages::finished()));
            EXPECT_TRUE(folly::IOBufEqualTo()(msg.fragment, expectedMessages));
            break;
          }
        }
        content.data = folly::IOBuf::copyBuffer("finwrite");
        return content;
      }));
  EXPECT_CALL(*mockKeyScheduler_, deriveMasterSecret());
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(MasterSecrets::ExporterMaster, RangeMatches("sfincontext")));
  EXPECT_CALL(
      *mockKeyScheduler_,
      getSecret(MasterSecrets::ResumptionMaster, RangeMatches("fincontext")))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'r', 'e', 's'}),
            MasterSecrets::ResumptionMaster);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, deriveAppTrafficSecrets(RangeMatches("sfincontext")));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ServerAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'a', 't'}),
            AppTrafficSecrets::ServerAppTraffic);
      }));
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ClientAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'a', 't'}),
            AppTrafficSecrets::ClientAppTraffic);
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));
  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;
  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sat"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cat"));
  EXPECT_CALL(*mockKeyScheduler_, clearMasterSecret());

  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectActions<
      MutateState,
      ReportHandshakeSuccess,
      WriteToSocket,
      SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Handshake);
  EXPECT_EQ(write.contents[0].contentType, ContentType::handshake);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("finwrite")));
  auto reportSuccess = expectAction<ReportHandshakeSuccess>(actions);
  EXPECT_EQ(reportSuccess.earlyDataAccepted, false);
  expectSecret(
      actions, AppTrafficSecrets::ServerAppTraffic, folly::StringPiece("sat"));
  expectSecret(
      actions, AppTrafficSecrets::ClientAppTraffic, folly::StringPiece("cat"));
  processStateMutations(actions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      *state_.resumptionSecret(), folly::IOBuf::copyBuffer("res")));
  EXPECT_FALSE(state_.sentCCS());
}

TEST_F(ClientProtocolTest, TestFinished) {
  setupExpectingFinished();
  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectActions<
      MutateState,
      ReportHandshakeSuccess,
      WriteToSocket,
      SecretAvailable>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
}

TEST_F(ClientProtocolTest, TestFinishedExtraData) {
  setupExpectingFinished();
  EXPECT_CALL(*mockHandshakeRead_, hasUnparsedHandshakeData())
      .WillRepeatedly(Return(true));
  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectError<FizzException>(
      actions, AlertDescription::unexpected_message, "data after finished");
}

TEST_F(ClientProtocolTest, TestFinishedMismatch) {
  setupExpectingFinished();
  auto finished = TestMessages::finished();
  finished.verify_data = folly::IOBuf::copyBuffer("ver1fydata");
  auto actions = detail::processEvent(state_, std::move(finished));
  expectError<FizzException>(
      actions, AlertDescription::bad_record_mac, "finished verify failure");
}

TEST_F(ClientProtocolTest, TestFinishedRejectedECH) {
  setupExpectingFinished();
  state_.echState().emplace();
  state_.echState()->status = ECHStatus::Rejected;
  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectErrorAtEnd<FizzException>(
      actions, AlertDescription::ech_required, "ech not accepted");
}

TEST_F(ClientProtocolTest, TestFinishedRejectedEarly) {
  setupExpectingFinished();
  state_.earlyDataType() = EarlyDataType::Rejected;
  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectActions<
      MutateState,
      ReportHandshakeSuccess,
      WriteToSocket,
      SecretAvailable>(actions);
  auto reportSuccess = expectAction<ReportHandshakeSuccess>(actions);
  EXPECT_EQ(reportSuccess.earlyDataAccepted, false);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
}

TEST_F(ClientProtocolTest, TestFinishedCompat) {
  context_->setCompatibilityMode(true);
  setupExpectingFinished();
  auto actions = detail::processEvent(state_, TestMessages::finished());
  expectActions<
      MutateState,
      ReportHandshakeSuccess,
      WriteToSocket,
      SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents.size(), 2);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Plaintext);
  EXPECT_EQ(write.contents[0].contentType, ContentType::change_cipher_spec);
  EXPECT_EQ(write.contents[1].encryptionLevel, EncryptionLevel::Handshake);
  EXPECT_EQ(write.contents[1].contentType, ContentType::handshake);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
  EXPECT_TRUE(state_.sentCCS());
}

TEST_F(ClientProtocolTest, TestNewSessionTicket) {
  setupAcceptingData();

  EXPECT_CALL(
      *mockKeyScheduler_,
      getResumptionSecret(RangeMatches("resumptionsecret"), RangeMatches("")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("derivedsecret"); }));
  state_.clientCert() = mockClientCert_;

  auto actions = detail::processEvent(state_, TestMessages::newSessionTicket());
  auto newCachedPsk = expectSingleAction<NewCachedPsk>(std::move(actions));
  auto psk = newCachedPsk.psk;
  EXPECT_EQ(psk.psk, "ticket");
  EXPECT_EQ(psk.secret, "derivedsecret");
  EXPECT_EQ(psk.type, PskType::Resumption);
  EXPECT_EQ(psk.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(psk.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(psk.group, NamedGroup::x25519);
  EXPECT_EQ(psk.serverCert, mockLeaf_);
  EXPECT_EQ(psk.clientCert, mockClientCert_);
  EXPECT_EQ(psk.maxEarlyDataSize, 0);
  EXPECT_EQ(psk.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      psk.ticketIssueTime,
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
  EXPECT_EQ(
      psk.ticketHandshakeTime,
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
}

TEST_F(ClientProtocolTest, TestNewSessionTicketNonce) {
  setupAcceptingData();

  EXPECT_CALL(
      *mockKeyScheduler_,
      getResumptionSecret(
          RangeMatches("resumptionsecret"), RangeMatches("nonce")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("derivedsecret"); }));

  auto nst = TestMessages::newSessionTicket();
  nst.ticket_nonce = folly::IOBuf::copyBuffer("nonce");
  auto actions = detail::processEvent(state_, std::move(nst));
  expectSingleAction<NewCachedPsk>(std::move(actions));
}

TEST_F(ClientProtocolTest, TestNewSessionTicketEarlyData) {
  setupAcceptingData();

  EXPECT_CALL(
      *mockKeyScheduler_,
      getResumptionSecret(RangeMatches("resumptionsecret"), RangeMatches("")))
      .WillOnce(InvokeWithoutArgs(
          []() { return folly::IOBuf::copyBuffer("derivedsecret"); }));

  auto nst = TestMessages::newSessionTicket();
  TicketEarlyData early;
  early.max_early_data_size = 2000;
  nst.extensions.push_back(encodeExtension(std::move(early)));
  auto actions = detail::processEvent(state_, std::move(nst));
  auto newCachedPsk = expectSingleAction<NewCachedPsk>(std::move(actions));
  auto psk = newCachedPsk.psk;
  EXPECT_EQ(psk.psk, "ticket");
  EXPECT_EQ(psk.secret, "derivedsecret");
  EXPECT_EQ(psk.type, PskType::Resumption);
  EXPECT_EQ(psk.version, ProtocolVersion::tls_1_3);
  EXPECT_EQ(psk.cipher, CipherSuite::TLS_AES_128_GCM_SHA256);
  EXPECT_EQ(psk.group, NamedGroup::x25519);
  EXPECT_EQ(psk.serverCert, mockLeaf_);
  EXPECT_EQ(psk.maxEarlyDataSize, 2000);
  EXPECT_EQ(psk.ticketAgeAdd, 0x44444444);
  EXPECT_EQ(
      psk.ticketIssueTime,
      std::chrono::system_clock::time_point(std::chrono::minutes(5)));
  EXPECT_EQ(
      psk.ticketHandshakeTime,
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
}

TEST_F(ClientProtocolTest, TestAppData) {
  setupAcceptingData();

  auto actions = detail::processEvent(state_, TestMessages::appData());

  expectSingleAction<DeliverAppData>(std::move(actions));
}

TEST_F(ClientProtocolTest, TestAppWrite) {
  setupAcceptingData();
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::application_data);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, folly::IOBuf::copyBuffer("appdata")));
        content.data = folly::IOBuf::copyBuffer("writtenappdata");
        return content;
      }));

  auto actions = detail::processEvent(state_, TestMessages::appWrite());
  auto write = expectSingleAction<WriteToSocket>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenappdata")));
  EXPECT_EQ(
      write.contents[0].encryptionLevel,
      state_.writeRecordLayer()->getEncryptionLevel());
  EXPECT_EQ(write.contents[0].contentType, ContentType::application_data);
}

TEST_F(ClientProtocolTest, TestKeyUpdateNotRequested) {
  setupAcceptingData();
  EXPECT_CALL(*mockKeyScheduler_, serverKeyUpdate());
  EXPECT_CALL(*mockRead_, hasUnparsedHandshakeData()).WillOnce(Return(false));

  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ServerAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'a', 't'}),
            AppTrafficSecrets::ServerAppTraffic);
      }));

  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));

  MockAead* raead;
  MockEncryptedReadRecordLayer* rrl;

  expectAeadCreation({{"serverkey", &raead}});
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sat"));

  auto actions = detail::processEvent(state_, TestMessages::keyUpdate(false));
  expectActions<MutateState, SecretAvailable>(actions);
  EXPECT_EQ(getNumActions<WriteToSocket>(actions, false), 0);

  expectSecret(
      actions, AppTrafficSecrets::ServerAppTraffic, folly::StringPiece("sat"));
  processStateMutations(actions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
}

TEST_F(ClientProtocolTest, TestKeyUpdateExtraData) {
  setupAcceptingData();
  EXPECT_CALL(*mockRead_, hasUnparsedHandshakeData())
      .WillRepeatedly(Return(true));
  auto actions = detail::processEvent(state_, TestMessages::keyUpdate(false));
  expectError<FizzException>(
      actions, AlertDescription::unexpected_message, "data after key_update");
}

TEST_F(ClientProtocolTest, TestKeyUpdateRequestFlow) {
  setupAcceptingData();
  EXPECT_CALL(*mockKeyScheduler_, serverKeyUpdate());
  EXPECT_CALL(*mockRead_, hasUnparsedHandshakeData()).WillOnce(Return(false));

  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ServerAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'s', 'a', 't'}),
            AppTrafficSecrets::ServerAppTraffic);
      }));

  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::handshake);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, encodeHandshake(TestMessages::keyUpdate(false))));
        content.data = folly::IOBuf::copyBuffer("keyupdated");
        return content;
      }));

  EXPECT_CALL(*mockKeyScheduler_, clientKeyUpdate());
  EXPECT_CALL(
      *mockKeyScheduler_, getSecret(AppTrafficSecrets::ClientAppTraffic))
      .WillOnce(InvokeWithoutArgs([]() {
        return DerivedSecret(
            std::vector<uint8_t>({'c', 'a', 't'}),
            AppTrafficSecrets::ClientAppTraffic);
      }));

  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("sat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("serverkey"),
            folly::IOBuf::copyBuffer("serveriv")};
      }));
  EXPECT_CALL(*mockKeyScheduler_, getTrafficKey(RangeMatches("cat"), _, _))
      .WillOnce(InvokeWithoutArgs([]() {
        return TrafficKey{
            folly::IOBuf::copyBuffer("clientkey"),
            folly::IOBuf::copyBuffer("clientiv")};
      }));

  MockAead* raead;
  MockAead* waead;
  MockEncryptedReadRecordLayer* rrl;
  MockEncryptedWriteRecordLayer* wrl;

  expectAeadCreation(&waead, &raead);
  expectEncryptedReadRecordLayerCreation(
      &rrl, &raead, folly::StringPiece("sat"));
  expectEncryptedWriteRecordLayerCreation(
      &wrl, &waead, folly::StringPiece("cat"));

  auto actions = detail::processEvent(state_, TestMessages::keyUpdate(true));
  expectActions<MutateState, WriteToSocket, SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("keyupdated")));
  EXPECT_EQ(
      write.contents[0].encryptionLevel,
      state_.writeRecordLayer()->getEncryptionLevel());
  EXPECT_EQ(write.contents[0].contentType, ContentType::handshake);
  expectSecret(
      actions, AppTrafficSecrets::ServerAppTraffic, folly::StringPiece("sat"));
  expectSecret(
      actions, AppTrafficSecrets::ClientAppTraffic, folly::StringPiece("cat"));
  processStateMutations(actions);
  EXPECT_EQ(state_.readRecordLayer().get(), rrl);
  EXPECT_EQ(
      state_.readRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.writeRecordLayer().get(), wrl);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::AppTraffic);
  EXPECT_EQ(state_.state(), StateEnum::Established);
  EXPECT_EQ(
      state_.handshakeTime(),
      std::chrono::system_clock::time_point(std::chrono::minutes(4)));
}

TEST_F(ClientProtocolTest, TestInvalidEarlyWrite) {
  setupExpectingServerHello();

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  expectError<FizzException>(actions, folly::none, "invalid early write");
}

TEST_F(ClientProtocolTest, TestEarlyWriteOmitRecord) {
  setupExpectingServerHello();
  state_.earlyDataType() = EarlyDataType::Attempted;
  context_->setOmitEarlyRecordLayer(true);

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  expectError<FizzException>(actions, folly::none, "early app writes disabled");
}

TEST_F(ClientProtocolTest, TestExpectingSHEarlyWrite) {
  setupExpectingServerHello();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Attempted;
  EXPECT_CALL(*mockEarlyWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockEarlyWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::application_data);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, folly::IOBuf::copyBuffer("appdata")));
        content.data = folly::IOBuf::copyBuffer("writtenappdata");
        return content;
      }));

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());

  auto write = expectSingleAction<WriteToSocket>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenappdata")));
}

TEST_F(ClientProtocolTest, TestEarlyEncryptionLevelRecvSHLO) {
  setupExpectingServerHello();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Attempted;

  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  processStateMutations(actions);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(
      state_.earlyWriteRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::EarlyData);
}

TEST_F(ClientProtocolTest, TestEarlyEncryptionLevelRecvFinished) {
  setupExpectingServerHello();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Attempted;

  auto actions = detail::processEvent(state_, TestMessages::serverHello());
  processStateMutations(actions);
  EXPECT_EQ(
      state_.writeRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::Handshake);
  EXPECT_EQ(
      state_.earlyWriteRecordLayer()->getEncryptionLevel(),
      EncryptionLevel::EarlyData);
}

TEST_F(ClientProtocolTest, TestEarlyWriteCompat) {
  setupExpectingServerHello();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Attempted;
  context_->setCompatibilityMode(true);

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents.size(), 2);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Plaintext);
  EXPECT_EQ(write.contents[0].contentType, ContentType::change_cipher_spec);
  EXPECT_EQ(write.contents[1].encryptionLevel, EncryptionLevel::EarlyData);
  EXPECT_EQ(write.contents[1].contentType, ContentType::application_data);
  processStateMutations(actions);
  EXPECT_TRUE(state_.sentCCS());
}

TEST_F(ClientProtocolTest, TestEarlyWriteCompatCCSAlreadySent) {
  setupExpectingServerHello();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Attempted;
  state_.sentCCS() = true;
  context_->setCompatibilityMode(true);

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());

  expectActions<WriteToSocket>(actions);
  EXPECT_TRUE(state_.sentCCS());
}

TEST_F(ClientProtocolTest, TestEarlyAcceptedCompatNoEarlyData) {
  setupExpectingFinished();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Accepted;
  context_->setCompatibilityMode(true);

  auto actions = detail::processEvent(state_, TestMessages::finished());

  expectActions<
      MutateState,
      WriteToSocket,
      ReportHandshakeSuccess,
      SecretAvailable>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  ASSERT_EQ(write.contents.size(), 3);
  EXPECT_EQ(write.contents[0].contentType, ContentType::change_cipher_spec);
  EXPECT_EQ(write.contents[0].encryptionLevel, EncryptionLevel::Plaintext);

  EXPECT_EQ(write.contents[1].contentType, ContentType::handshake);
  EXPECT_EQ(write.contents[1].encryptionLevel, EncryptionLevel::EarlyData);

  EXPECT_EQ(write.contents[2].contentType, ContentType::handshake);
  EXPECT_EQ(write.contents[2].encryptionLevel, EncryptionLevel::Handshake);

  processStateMutations(actions);
  EXPECT_TRUE(state_.sentCCS());
}

TEST_F(ClientProtocolTest, TestExpectingSHEarlyWriteRejected) {
  setupExpectingServerHello();
  state_.earlyDataType() = EarlyDataType::Rejected;

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  auto failedWrite =
      expectSingleAction<ReportEarlyWriteFailed>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      failedWrite.write.data, folly::IOBuf::copyBuffer("appdata")));
}

TEST_F(ClientProtocolTest, TestExpectingEEEarlyWrite) {
  setupExpectingEncryptedExtensionsEarlySent();
  EXPECT_CALL(*mockEarlyWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockEarlyWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::application_data);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, folly::IOBuf::copyBuffer("appdata")));
        content.data = folly::IOBuf::copyBuffer("writtenappdata");
        return content;
      }));

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());

  auto write = expectSingleAction<WriteToSocket>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenappdata")));
}

TEST_F(ClientProtocolTest, TestExpectingEEEarlyWriteRejected) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.earlyDataType() = EarlyDataType::Rejected;

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  auto failedWrite =
      expectSingleAction<ReportEarlyWriteFailed>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      failedWrite.write.data, folly::IOBuf::copyBuffer("appdata")));
}

TEST_F(ClientProtocolTest, TestExpectingCertEarlyWriteRejected) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.earlyDataType() = EarlyDataType::Rejected;

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  auto failedWrite =
      expectSingleAction<ReportEarlyWriteFailed>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      failedWrite.write.data, folly::IOBuf::copyBuffer("appdata")));
}

TEST_F(ClientProtocolTest, TestExpectingCertVerifyEarlyWriteRejected) {
  setupExpectingEncryptedExtensionsEarlySent();
  state_.earlyDataType() = EarlyDataType::Rejected;

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  auto failedWrite =
      expectSingleAction<ReportEarlyWriteFailed>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      failedWrite.write.data, folly::IOBuf::copyBuffer("appdata")));
}

TEST_F(ClientProtocolTest, TestExpectingFinishedEarlyWrite) {
  setupExpectingFinished();
  setMockEarlyRecord();
  state_.earlyDataType() = EarlyDataType::Accepted;
  EXPECT_CALL(*mockEarlyWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockEarlyWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::application_data);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, folly::IOBuf::copyBuffer("appdata")));
        content.data = folly::IOBuf::copyBuffer("writtenappdata");
        return content;
      }));

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());

  auto write = expectSingleAction<WriteToSocket>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenappdata")));
}

TEST_F(ClientProtocolTest, TestExpectingFinishedEarlyWriteRejected) {
  setupExpectingFinished();
  state_.earlyDataType() = EarlyDataType::Rejected;

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  auto failedWrite =
      expectSingleAction<ReportEarlyWriteFailed>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      failedWrite.write.data, folly::IOBuf::copyBuffer("appdata")));
}

TEST_F(ClientProtocolTest, TestEstablishedEarlyWrite) {
  setupAcceptingData();
  state_.earlyDataType() = EarlyDataType::Accepted;
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        content.encryptionLevel = mockWrite_->getEncryptionLevel();
        EXPECT_EQ(msg.type, ContentType::application_data);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, folly::IOBuf::copyBuffer("appdata")));
        content.data = folly::IOBuf::copyBuffer("writtenappdata");
        return content;
      }));

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());

  auto write = expectSingleAction<WriteToSocket>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      write.contents[0].data, folly::IOBuf::copyBuffer("writtenappdata")));
}

TEST_F(ClientProtocolTest, TestEstablishedEarlyWriteRejected) {
  setupAcceptingData();
  state_.earlyDataType() = EarlyDataType::Rejected;

  auto actions = detail::processEvent(state_, TestMessages::earlyAppWrite());
  auto failedWrite =
      expectSingleAction<ReportEarlyWriteFailed>(std::move(actions));
  EXPECT_TRUE(folly::IOBufEqualTo()(
      failedWrite.write.data, folly::IOBuf::copyBuffer("appdata")));
}

TEST_F(ClientProtocolTest, TestEstablishedCloseNotifyReceived) {
  setupAcceptingData();
  auto actions = detail::processEvent(state_, CloseNotify());
  expectActions<MutateState, WriteToSocket, EndOfData>(actions);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::Closed);
  EXPECT_EQ(state_.readRecordLayer().get(), nullptr);
  EXPECT_EQ(state_.writeRecordLayer().get(), nullptr);
}

TEST_F(
    ClientProtocolTest,
    TestEstablishedCloseNotifyReceivedWithUnparsedHandshakeData) {
  setupAcceptingData();
  EXPECT_CALL(*mockRead_, hasUnparsedHandshakeData())
      .WillRepeatedly(Return(true));
  auto actions = detail::processEvent(state_, CloseNotify());
  expectError<FizzException>(actions, AlertDescription::unexpected_message);
}

TEST_F(ClientProtocolTest, TestEstablishedAppClose) {
  setupAcceptingData();
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        EXPECT_EQ(msg.type, ContentType::alert);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, encode(Alert(AlertDescription::close_notify))));
        content.data = folly::IOBuf::copyBuffer("closenotify");
        return content;
      }));
  auto actions = ClientStateMachine().processAppClose(state_);
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents[0].contentType, ContentType::alert);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::ExpectingCloseNotify);
  EXPECT_NE(state_.readRecordLayer().get(), nullptr);
  EXPECT_EQ(state_.writeRecordLayer().get(), nullptr);

  EXPECT_CALL(*mockRead_, mockReadEvent()).WillOnce(InvokeWithoutArgs([]() {
    return ReadRecordLayer::ReadResult<Param>::from(CloseNotify());
  }));

  mockRead_->useMockReadEvent(true);
  folly::IOBufQueue queue;
  actions = ClientStateMachine().processSocketData(
      state_, queue, Aead::AeadOptions());
  expectActions<MutateState, EndOfData>(actions);
  processStateMutations(actions);
  expectAction<EndOfData>(actions);
  EXPECT_EQ(state_.state(), StateEnum::Closed);
}

TEST_F(ClientProtocolTest, TestEstablishedAppCloseImmediate) {
  setupAcceptingData();
  EXPECT_CALL(*mockWrite_, _write(_, _))
      .WillOnce(Invoke([&](TLSMessage& msg, Aead::AeadOptions) {
        TLSContent content;
        content.contentType = msg.type;
        EXPECT_EQ(msg.type, ContentType::alert);
        EXPECT_TRUE(folly::IOBufEqualTo()(
            msg.fragment, encode(Alert(AlertDescription::close_notify))));
        content.data = folly::IOBuf::copyBuffer("closenotify");
        return content;
      }));
  auto actions = ClientStateMachine().processAppCloseImmediate(state_);
  expectActions<MutateState, WriteToSocket>(actions);
  auto write = expectAction<WriteToSocket>(actions);
  EXPECT_EQ(write.contents[0].contentType, ContentType::alert);
  processStateMutations(actions);
  EXPECT_EQ(state_.state(), StateEnum::Closed);
  EXPECT_EQ(state_.readRecordLayer().get(), nullptr);
  EXPECT_EQ(state_.writeRecordLayer().get(), nullptr);
}

TEST_F(ClientProtocolTest, TestDecodeErrorAlert) {
  setupAcceptingData();
  EXPECT_CALL(*mockRead_, read(_, _))
      .WillOnce(
          InvokeWithoutArgs([]() -> ReadRecordLayer::ReadResult<TLSMessage> {
            throw std::runtime_error("read record layer error");
          }));
  folly::IOBufQueue buf;
  auto actions =
      ClientStateMachine().processSocketData(state_, buf, Aead::AeadOptions());
  auto exc = expectError<FizzException>(
      actions, AlertDescription::decode_error, "read record layer error");

  ASSERT_TRUE(exc.getAlert().has_value());
  EXPECT_EQ(AlertDescription::decode_error, exc.getAlert().value());
}

TEST_F(ClientProtocolTest, TestSocketDataFizzExceptionAlert) {
  setupAcceptingData();
  EXPECT_CALL(*mockRead_, read(_, _))
      .WillOnce(
          InvokeWithoutArgs([]() -> ReadRecordLayer::ReadResult<TLSMessage> {
            throw FizzException(
                "arbitrary fizzexception with alert",
                AlertDescription::internal_error);
          }));
  folly::IOBufQueue buf;
  auto actions =
      ClientStateMachine().processSocketData(state_, buf, Aead::AeadOptions());
  auto exc = expectError<FizzException>(
      actions,
      AlertDescription::internal_error,
      "arbitrary fizzexception with alert");

  ASSERT_TRUE(exc.getAlert().has_value());
  EXPECT_EQ(AlertDescription::internal_error, exc.getAlert().value());
}

TEST_F(ClientProtocolTest, TestSocketDataFizzExceptionNoAlert) {
  setupAcceptingData();
  EXPECT_CALL(*mockRead_, read(_, _))
      .WillOnce(
          InvokeWithoutArgs([]() -> ReadRecordLayer::ReadResult<TLSMessage> {
            throw FizzException(
                "arbitrary fizzexception without alert", folly::none);
          }));
  folly::IOBufQueue buf;
  auto actions =
      ClientStateMachine().processSocketData(state_, buf, Aead::AeadOptions());
  auto exc = expectError<FizzException>(
      actions, folly::none, "arbitrary fizzexception without alert");

  EXPECT_FALSE(exc.getAlert().has_value());
}

TEST_F(ClientProtocolTest, TestWaitForDataSizeHint) {
  setupAcceptingData();
  EXPECT_CALL(*mockRead_, read(_, _))
      .WillOnce(
          InvokeWithoutArgs([]() -> ReadRecordLayer::ReadResult<TLSMessage> {
            return ReadRecordLayer::ReadResult<TLSMessage>::noneWithSizeHint(
                17);
          }));
  folly::IOBufQueue buf;
  auto actions =
      ClientStateMachine().processSocketData(state_, buf, Aead::AeadOptions());
  auto wfd = expectAction<WaitForData>(actions);
  EXPECT_EQ(wfd.recordSizeHint, 17);
}
} // namespace test
} // namespace client
} // namespace fizz
