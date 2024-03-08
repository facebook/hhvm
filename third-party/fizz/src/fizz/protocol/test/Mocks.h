/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/crypto/exchange/test/Mocks.h>
#include <fizz/crypto/test/Mocks.h>
#include <fizz/protocol/AsyncFizzBase.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/CertificateVerifier.h>
#include <fizz/protocol/HandshakeContext.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/OpenSSLFactory.h>
#include <fizz/protocol/Types.h>
#include <fizz/protocol/ech/Decrypter.h>
#include <fizz/record/test/Mocks.h>

#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace test {

/* using override */
using namespace testing;

class MockKeyScheduler : public KeyScheduler {
 public:
  MockKeyScheduler() : KeyScheduler(std::make_unique<MockKeyDerivation>()) {}

  MOCK_METHOD(void, deriveEarlySecret, (folly::ByteRange psk));
  MOCK_METHOD(void, deriveHandshakeSecret, ());
  MOCK_METHOD(void, deriveHandshakeSecret, (folly::ByteRange ecdhe));
  MOCK_METHOD(void, deriveMasterSecret, ());
  MOCK_METHOD(void, deriveAppTrafficSecrets, (folly::ByteRange transcript));
  MOCK_METHOD(void, clearMasterSecret, ());
  MOCK_METHOD(uint32_t, clientKeyUpdate, ());
  MOCK_METHOD(uint32_t, serverKeyUpdate, ());
  MOCK_METHOD(
      DerivedSecret,
      getSecret,
      (EarlySecrets s, folly::ByteRange transcript),
      (const));
  MOCK_METHOD(
      DerivedSecret,
      getSecret,
      (HandshakeSecrets s, folly::ByteRange transcript),
      (const));
  MOCK_METHOD(
      DerivedSecret,
      getSecret,
      (MasterSecrets s, folly::ByteRange transcript),
      (const));
  MOCK_METHOD(DerivedSecret, getSecret, (AppTrafficSecrets s), (const));
  MOCK_METHOD(
      TrafficKey,
      getTrafficKey,
      (folly::ByteRange trafficSecret, size_t keyLength, size_t ivLength),
      (const));
  MOCK_METHOD(
      TrafficKey,
      getTrafficKeyWithLabel,
      (folly::ByteRange trafficSecret,
       folly::StringPiece keyLabel,
       folly::StringPiece ivLabel,
       size_t keyLength,
       size_t ivLength),
      (const));
  MOCK_METHOD(
      Buf,
      getResumptionSecret,
      (folly::ByteRange, folly::ByteRange),
      (const));

  void setDefaults() {
    ON_CALL(*this, getTrafficKey(_, _, _))
        .WillByDefault(InvokeWithoutArgs([]() {
          return TrafficKey{
              folly::IOBuf::copyBuffer("key"), folly::IOBuf::copyBuffer("iv")};
        }));
    ON_CALL(*this, getTrafficKeyWithLabel(_, _, _, _, _))
        .WillByDefault(InvokeWithoutArgs([]() {
          return TrafficKey{
              folly::IOBuf::copyBuffer("key_with_label"),
              folly::IOBuf::copyBuffer("iv_with_label")};
        }));
    ON_CALL(*this, getResumptionSecret(_, _))
        .WillByDefault(InvokeWithoutArgs(
            []() { return folly::IOBuf::copyBuffer("resumesecret"); }));
    ON_CALL(*this, getSecret(An<EarlySecrets>(), _))
        .WillByDefault(Invoke([](EarlySecrets type, folly::ByteRange) {
          return DerivedSecret(std::vector<uint8_t>(), type);
        }));
    ON_CALL(*this, getSecret(An<HandshakeSecrets>(), _))
        .WillByDefault(Invoke([](HandshakeSecrets type, folly::ByteRange) {
          return DerivedSecret(std::vector<uint8_t>(), type);
        }));
    ON_CALL(*this, getSecret(EarlySecrets::ECHAcceptConfirmation, _))
        .WillByDefault(InvokeWithoutArgs([]() {
          return DerivedSecret(
              std::vector<uint8_t>({'e', 'c', 'h', 'a', 'c', 'c', 'p', 't'}),
              EarlySecrets::ECHAcceptConfirmation);
        }));
    ON_CALL(*this, getSecret(EarlySecrets::HRRECHAcceptConfirmation, _))
        .WillByDefault(InvokeWithoutArgs([]() {
          return DerivedSecret(
              std::vector<uint8_t>({'e', 'c', 'h', 'a', 'c', 'c', 'p', 't'}),
              EarlySecrets::HRRECHAcceptConfirmation);
        }));
    ON_CALL(*this, getSecret(An<MasterSecrets>(), _))
        .WillByDefault(Invoke([](MasterSecrets type, folly::ByteRange) {
          return DerivedSecret(std::vector<uint8_t>(), type);
        }));
    ON_CALL(*this, getSecret(_))
        .WillByDefault(Invoke([](AppTrafficSecrets type) {
          // The app traffic secret should be 32 bytes
          return DerivedSecret(
              std::vector<uint8_t>({'a', 'p', 'p', 't', 'r', 'a', 'f', 'f',
                                    'i', 'c', 'a', 'p', 'p', 't', 'r', 'a',
                                    'f', 'f', 'i', 'c', 'a', 'p', 'p', 't',
                                    'r', 'a', 'f', 'f', 'i', 'c', '3', '2'}),
              type);
        }));
  }
};

class MockHandshakeContext : public HandshakeContext {
 public:
  MOCK_METHOD(void, appendToTranscript, (const Buf& transcript));
  MOCK_METHOD(Buf, getHandshakeContext, (), (const));
  MOCK_METHOD(Buf, getFinishedData, (folly::ByteRange baseKey), (const));
  MOCK_METHOD(folly::ByteRange, getBlankContext, (), (const));
  MOCK_METHOD(std::unique_ptr<HandshakeContext>, clone, (), (const));

  void setDefaults() {
    ON_CALL(*this, getHandshakeContext()).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("context");
    }));

    ON_CALL(*this, getFinishedData(_)).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("verifydata");
    }));

    ON_CALL(*this, clone()).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<MockHandshakeContext>();
      ret->setDefaults();
      return ret;
    }));
  }
};

class MockCert : public Cert {
 public:
  MOCK_METHOD(std::string, getIdentity, (), (const));
  MOCK_METHOD(folly::ssl::X509UniquePtr, getX509, (), (const));
};

class MockSelfCert : public SelfCert {
 public:
  MOCK_METHOD(std::string, getIdentity, (), (const));
  MOCK_METHOD(std::vector<std::string>, getAltIdentities, (), (const));
  MOCK_METHOD(std::vector<SignatureScheme>, getSigSchemes, (), (const));

  MOCK_METHOD(CertificateMsg, _getCertMessage, (Buf&), (const));
  CertificateMsg getCertMessage(Buf buf) const override {
    return _getCertMessage(buf);
  }
  MOCK_METHOD(
      CompressedCertificate,
      getCompressedCert,
      (CertificateCompressionAlgorithm),
      (const));

  MOCK_METHOD(
      Buf,
      sign,
      (SignatureScheme scheme,
       CertificateVerifyContext context,
       folly::ByteRange toBeSigned),
      (const));
  MOCK_METHOD(folly::ssl::X509UniquePtr, getX509, (), (const));
};

class MockPeerCert : public PeerCert {
 public:
  MOCK_METHOD(std::string, getIdentity, (), (const));
  MOCK_METHOD(
      void,
      verify,
      (SignatureScheme scheme,
       CertificateVerifyContext context,
       folly::ByteRange toBeSigned,
       folly::ByteRange signature),
      (const));
  MOCK_METHOD(folly::ssl::X509UniquePtr, getX509, (), (const));
};

class MockCertificateVerifier : public CertificateVerifier {
 public:
  MOCK_METHOD(
      std::shared_ptr<const Cert>,
      verify,
      (const std::vector<std::shared_ptr<const PeerCert>>&),
      (const));

  MOCK_METHOD(
      std::vector<Extension>,
      getCertificateRequestExtensions,
      (),
      (const));
};

class MockFactory : public OpenSSLFactory {
 public:
  MOCK_METHOD(
      std::unique_ptr<PlaintextReadRecordLayer>,
      makePlaintextReadRecordLayer,
      (),
      (const));
  MOCK_METHOD(
      std::unique_ptr<PlaintextWriteRecordLayer>,
      makePlaintextWriteRecordLayer,
      (),
      (const));
  MOCK_METHOD(
      std::unique_ptr<EncryptedReadRecordLayer>,
      makeEncryptedReadRecordLayer,
      (EncryptionLevel encryptionLevel),
      (const));
  MOCK_METHOD(
      std::unique_ptr<EncryptedWriteRecordLayer>,
      makeEncryptedWriteRecordLayer,
      (EncryptionLevel encryptionLevel),
      (const));
  MOCK_METHOD(
      std::unique_ptr<KeyScheduler>,
      makeKeyScheduler,
      (CipherSuite cipher),
      (const));
  MOCK_METHOD(
      std::unique_ptr<HandshakeContext>,
      makeHandshakeContext,
      (CipherSuite cipher),
      (const));
  MOCK_METHOD(
      std::unique_ptr<KeyExchange>,
      makeKeyExchange,
      (NamedGroup group, Factory::KeyExchangeMode mode),
      (const));
  MOCK_METHOD(std::unique_ptr<Aead>, makeAead, (CipherSuite cipher), (const));
  MOCK_METHOD(Random, makeRandom, (), (const));
  MOCK_METHOD(uint32_t, makeTicketAgeAdd, (), (const));

  MOCK_METHOD(
      std::shared_ptr<PeerCert>,
      _makePeerCert,
      (CertificateEntry & entry, bool leaf),
      (const));
  std::shared_ptr<PeerCert> makePeerCert(CertificateEntry entry, bool leaf)
      const override {
    return _makePeerCert(entry, leaf);
  }

  MOCK_CONST_METHOD1(
      makeRandomBytes,
      std::unique_ptr<folly::IOBuf>(size_t count));

  void setDefaults() {
    ON_CALL(*this, makePlaintextReadRecordLayer())
        .WillByDefault(InvokeWithoutArgs([]() {
          return std::make_unique<NiceMock<MockPlaintextReadRecordLayer>>();
        }));

    ON_CALL(*this, makePlaintextWriteRecordLayer())
        .WillByDefault(InvokeWithoutArgs([]() {
          auto ret =
              std::make_unique<NiceMock<MockPlaintextWriteRecordLayer>>();
          ret->setDefaults();
          return ret;
        }));
    ON_CALL(*this, makeEncryptedReadRecordLayer(_))
        .WillByDefault(Invoke([](EncryptionLevel encryptionLevel) {
          return std::make_unique<NiceMock<MockEncryptedReadRecordLayer>>(
              encryptionLevel);
        }));

    ON_CALL(*this, makeEncryptedWriteRecordLayer(_))
        .WillByDefault(Invoke([](EncryptionLevel encryptionLevel) {
          auto ret = std::make_unique<NiceMock<MockEncryptedWriteRecordLayer>>(
              encryptionLevel);
          ret->setDefaults();
          return ret;
        }));

    ON_CALL(*this, makeKeyScheduler(_)).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<NiceMock<MockKeyScheduler>>();
      ret->setDefaults();
      return ret;
    }));
    ON_CALL(*this, makeHandshakeContext(_))
        .WillByDefault(InvokeWithoutArgs([]() {
          auto ret = std::make_unique<NiceMock<MockHandshakeContext>>();
          ret->setDefaults();
          return ret;
        }));
    ON_CALL(*this, makeKeyExchange(_, _)).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<NiceMock<MockKeyExchange>>();
      ret->setDefaults();
      return ret;
    }));
    ON_CALL(*this, makeAead(_)).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<NiceMock<MockAead>>();
      ret->setDefaults();
      return ret;
    }));
    ON_CALL(*this, makeRandom()).WillByDefault(InvokeWithoutArgs([]() {
      Random random;
      random.fill(0x44);
      return random;
    }));
    ON_CALL(*this, makeRandomBytes(_)).WillByDefault(Invoke([](size_t count) {
      auto random = folly::IOBuf::create(count);
      memset(random->writableData(), 0x44, count);
      random->append(count);
      return random;
    }));
    ON_CALL(*this, makeTicketAgeAdd()).WillByDefault(InvokeWithoutArgs([]() {
      return 0x44444444;
    }));
    ON_CALL(*this, _makePeerCert(_, _)).WillByDefault(InvokeWithoutArgs([]() {
      return std::make_unique<NiceMock<MockPeerCert>>();
    }));
  }
};

class MockAsyncKexFactory : public OpenSSLFactory {
 public:
  MOCK_METHOD(
      std::unique_ptr<KeyExchange>,
      makeKeyExchange,
      (NamedGroup group, Factory::KeyExchangeMode mode),
      (const));
};

class MockAsyncFizzBase : public AsyncFizzBase {
 public:
  MockAsyncFizzBase()
      : AsyncFizzBase(
            folly::AsyncTransport::UniquePtr(
                new folly::test::MockAsyncTransport()),
            AsyncFizzBase::TransportOptions()) {}
  MOCK_METHOD(bool, good, (), (const));
  MOCK_METHOD(bool, readable, (), (const));
  MOCK_METHOD(bool, connecting, (), (const));
  MOCK_METHOD(bool, error, (), (const));
  MOCK_METHOD(folly::ssl::X509UniquePtr, getPeerCert, (), (const));
  MOCK_METHOD(const X509*, getSelfCert, (), (const));
  MOCK_METHOD(bool, isReplaySafe, (), (const));
  MOCK_METHOD(
      void,
      setReplaySafetyCallback,
      (folly::AsyncTransport::ReplaySafetyCallback * callback));
  MOCK_METHOD(const Cert*, getSelfCertificate, (), (const));
  MOCK_METHOD(const Cert*, getPeerCertificate, (), (const));
  MOCK_METHOD(std::string, getApplicationProtocol_, (), (const));

  MOCK_METHOD(void, setReadCB, (ReadCallback*));
  MOCK_METHOD(void, setEndOfTLSCallback, (EndOfTLSCallback*));

  std::string getApplicationProtocol() const noexcept override {
    return getApplicationProtocol_();
  }

  MOCK_METHOD(folly::Optional<CipherSuite>, getCipher, (), (const));
  MOCK_METHOD(folly::Optional<NamedGroup>, getGroup, (), (const));
  MOCK_METHOD(
      std::vector<SignatureScheme>,
      getSupportedSigSchemes,
      (),
      (const));
  MOCK_METHOD(
      Buf,
      _getExportedKeyingMaterial,
      (folly::StringPiece, Buf&, uint16_t),
      (const));

  Buf getExportedKeyingMaterial(
      folly::StringPiece label,
      Buf context,
      uint16_t length) const override {
    return _getExportedKeyingMaterial(label, context, length);
  }

  MOCK_METHOD(folly::Optional<Random>, getClientRandom, (), (const));
  MOCK_METHOD(void, tlsShutdown, ());
  MOCK_METHOD(void, shutdownWrite, (), (override));
  MOCK_METHOD(void, shutdownWriteNow, (), (override));
  MOCK_METHOD(void, initiateKeyUpdate, (KeyUpdateRequest), (override));

  MOCK_METHOD(
      void,
      writeAppDataInternal,
      (folly::AsyncTransport::WriteCallback*,
       std::shared_ptr<folly::IOBuf>,
       folly::WriteFlags));

  void writeAppData(
      folly::AsyncTransport::WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    writeAppDataInternal(
        callback, std::shared_ptr<folly::IOBuf>(buf.release()), flags);
  }

  MOCK_METHOD(void, transportError, (const folly::AsyncSocketException&));

  MOCK_METHOD(void, transportDataAvailable, ());
  MOCK_METHOD(void, pauseEvents, ());
  MOCK_METHOD(void, resumeEvents, ());
};

class MockECHDecrypter : public ech::Decrypter {
 public:
  MOCK_METHOD(
      folly::Optional<ech::DecrypterResult>,
      decryptClientHello,
      (const ClientHello& chlo));

  MOCK_METHOD(
      ClientHello,
      _decryptClientHelloHRR_Stateful,
      (const ClientHello& chlo, std::unique_ptr<hpke::HpkeContext>& context));

  ClientHello decryptClientHelloHRR(
      const ClientHello& chlo,
      std::unique_ptr<hpke::HpkeContext>& context) override {
    return _decryptClientHelloHRR_Stateful(chlo, context);
  }

  MOCK_METHOD(
      ClientHello,
      _decryptClientHelloHRR_Stateless,
      (const ClientHello& chlo,
       const std::unique_ptr<folly::IOBuf>& encapsulatedKey));

  ClientHello decryptClientHelloHRR(
      const ClientHello& chlo,
      const std::unique_ptr<folly::IOBuf>& encapsulatedKey) override {
    return _decryptClientHelloHRR_Stateless(chlo, encapsulatedKey);
  }

  MOCK_METHOD(
      std::vector<ech::ECHConfig>,
      getRetryConfigs,
      (),
      (const, override));
};
} // namespace test
} // namespace fizz
