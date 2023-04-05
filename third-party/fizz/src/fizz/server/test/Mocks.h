/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/crypto/exchange/test/Mocks.h>
#include <fizz/protocol/test/Mocks.h>
#include <fizz/record/test/Mocks.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/AsyncSelfCert.h>
#include <fizz/server/CookieCipher.h>
#include <fizz/server/ReplayCache.h>
#include <fizz/server/ServerExtensions.h>
#include <fizz/server/ServerProtocol.h>
#include <fizz/server/TokenCipher.h>

namespace fizz {
namespace server {
namespace test {

/* using override */
using namespace testing;
/* using override */
using namespace fizz::test;

class MockServerStateMachine : public ServerStateMachine {
 public:
  MOCK_METHOD(
      folly::Optional<AsyncActions>,
      _processAccept,
      (const State&,
       folly::Executor*,
       std::shared_ptr<const FizzServerContext> context,
       const std::shared_ptr<ServerExtensions>& extensions));
  AsyncActions processAccept(
      const State& state,
      folly::Executor* executor,
      std::shared_ptr<const FizzServerContext> context,
      const std::shared_ptr<ServerExtensions>& extensions) override {
    return *_processAccept(state, executor, std::move(context), extensions);
  }

  MOCK_METHOD(
      folly::Optional<AsyncActions>,
      _processSocketData,
      (const State&, folly::IOBufQueue&, Aead::AeadOptions));
  AsyncActions processSocketData(
      const State& state,
      folly::IOBufQueue& queue,
      Aead::AeadOptions options) override {
    return *_processSocketData(state, queue, options);
  }

  MOCK_METHOD(
      folly::Optional<AsyncActions>,
      _processWriteNewSessionTicket,
      (const State&, WriteNewSessionTicket&));
  AsyncActions processWriteNewSessionTicket(
      const State& state,
      WriteNewSessionTicket write) override {
    return *_processWriteNewSessionTicket(state, write);
  }

  MOCK_METHOD(
      folly::Optional<AsyncActions>,
      _processAppWrite,
      (const State&, AppWrite&));
  AsyncActions processAppWrite(const State& state, AppWrite appWrite) override {
    return *_processAppWrite(state, appWrite);
  }

  MOCK_METHOD(
      folly::Optional<AsyncActions>,
      _processEarlyAppWrite,
      (const State&, EarlyAppWrite&));
  AsyncActions processEarlyAppWrite(const State& state, EarlyAppWrite appWrite)
      override {
    return *_processEarlyAppWrite(state, appWrite);
  }

  MOCK_METHOD(folly::Optional<Actions>, _processAppClose, (const State&));
  Actions processAppClose(const State& state) override {
    return *_processAppClose(state);
  }

  MOCK_METHOD(
      folly::Optional<Actions>,
      _processAppCloseImmediate,
      (const State&));
  Actions processAppCloseImmediate(const State& state) override {
    return *_processAppCloseImmediate(state);
  }
};

class MockTicketCipher : public TicketCipher {
 public:
  MOCK_METHOD(
      (folly::SemiFuture<
          folly::Optional<std::pair<Buf, std::chrono::seconds>>>),
      _encrypt,
      (ResumptionState&),
      (const));
  folly::SemiFuture<folly::Optional<std::pair<Buf, std::chrono::seconds>>>
  encrypt(ResumptionState resState) const override {
    return _encrypt(resState);
  }

  MOCK_METHOD(
      (folly::SemiFuture<std::pair<PskType, folly::Optional<ResumptionState>>>),
      _decrypt,
      (std::unique_ptr<folly::IOBuf> & encryptedTicket),
      (const));
  folly::SemiFuture<std::pair<PskType, folly::Optional<ResumptionState>>>
  decrypt(std::unique_ptr<folly::IOBuf> encryptedTicket) const override {
    return _decrypt(encryptedTicket);
  }

  void setDefaults(
      std::chrono::system_clock::time_point ticketIssued =
          std::chrono::system_clock::now()) {
    ON_CALL(*this, _decrypt(_))
        .WillByDefault(InvokeWithoutArgs([ticketIssued]() {
          ResumptionState res;
          res.version = ProtocolVersion::tls_1_3;
          res.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
          res.resumptionSecret = folly::IOBuf::copyBuffer("resumesecret");
          res.alpn = "h2";
          res.ticketAgeAdd = 0;
          res.ticketIssueTime = ticketIssued;
          res.handshakeTime = ticketIssued;
          return std::make_pair(PskType::Resumption, std::move(res));
        }));
    ON_CALL(*this, _encrypt(_)).WillByDefault(InvokeWithoutArgs([]() {
      return std::make_pair(
          folly::IOBuf::copyBuffer("ticket"), std::chrono::seconds(100));
    }));
  }
};

class MockTokenCipher : public TokenCipher {
 public:
  MOCK_METHOD(
      bool,
      setSecrets,
      (const std::vector<folly::ByteRange>&),
      (override));

  MOCK_METHOD(folly::Optional<Buf>, _encrypt, (Buf, folly::IOBuf*), (const));
  folly::Optional<Buf> encrypt(
      Buf plaintext,
      folly::IOBuf* associatedData = nullptr) const override {
    return _encrypt(std::move(plaintext), associatedData);
  }

  MOCK_METHOD(folly::Optional<Buf>, _decrypt, (Buf, folly::IOBuf*), (const));
  folly::Optional<Buf> decrypt(
      Buf ciphertext,
      folly::IOBuf* associatedData = nullptr) const override {
    return _decrypt(std::move(ciphertext), associatedData);
  }
};

class MockCookieCipher : public CookieCipher {
 public:
  MOCK_METHOD(folly::Optional<CookieState>, _decrypt, (Buf&), (const));
  folly::Optional<CookieState> decrypt(Buf cookie) const override {
    return _decrypt(cookie);
  }
};

template <typename SM>
class MockHandshakeCallbackT : public AsyncFizzServerT<SM>::HandshakeCallback {
 public:
  MOCK_METHOD(void, _fizzHandshakeSuccess, ());
  void fizzHandshakeSuccess(AsyncFizzServerT<SM>*) noexcept override {
    _fizzHandshakeSuccess();
  }

  MOCK_METHOD(void, _fizzHandshakeError, (folly::exception_wrapper));
  void fizzHandshakeError(
      AsyncFizzServerT<SM>*,
      folly::exception_wrapper ew) noexcept override {
    _fizzHandshakeError(std::move(ew));
  }

  MOCK_METHOD(
      void,
      _fizzHandshakeAttemptFallback,
      (std::unique_ptr<folly::IOBuf>&));
  void fizzHandshakeAttemptFallback(
      std::unique_ptr<folly::IOBuf> clientHello) override {
    return _fizzHandshakeAttemptFallback(clientHello);
  }
};

using MockHandshakeCallback = MockHandshakeCallbackT<ServerStateMachine>;

template <typename SM>
class MockAsyncFizzServerT : public AsyncFizzServerT<SM> {
 public:
  MockAsyncFizzServerT(
      folly::AsyncTransportWrapper::UniquePtr socket,
      const std::shared_ptr<FizzServerContext>& fizzContext)
      : AsyncFizzServerT<SM>(std::move(socket), fizzContext) {}

  using UniquePtr = std::
      unique_ptr<MockAsyncFizzServerT, folly::DelayedDestruction::Destructor>;

  MOCK_METHOD(
      Buf,
      getExportedKeyingMaterial,
      (folly::StringPiece, Buf, uint16_t),
      (const));
};

using MockAsyncFizzServer = MockAsyncFizzServerT<ServerStateMachine>;

class MockCertManager : public CertManager {
 public:
  MOCK_METHOD(
      CertMatch,
      getCert,
      (const folly::Optional<std::string>& sni,
       const std::vector<SignatureScheme>& supportedSigSchemes,
       const std::vector<SignatureScheme>& peerSigSchemes,
       const std::vector<Extension>& peerExtensions),
      (const));
  MOCK_METHOD(
      std::shared_ptr<SelfCert>,
      getCert,
      (const std::string& identity),
      (const));
};

class MockServerExtensions : public ServerExtensions {
 public:
  MOCK_METHOD(std::vector<Extension>, getExtensions, (const ClientHello& chlo));
};

class MockReplayCache : public ReplayCache {
 public:
  MOCK_METHOD(
      folly::SemiFuture<ReplayCacheResult>,
      check,
      (std::unique_ptr<folly::IOBuf>));
};

class MockAppTokenValidator : public AppTokenValidator {
 public:
  MOCK_METHOD(bool, validate, (const ResumptionState&), (const));
};

class MockAsyncSelfCert : public AsyncSelfCert {
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
  MOCK_METHOD(
      folly::SemiFuture<folly::Optional<Buf>>,
      signFuture,
      (SignatureScheme scheme,
       CertificateVerifyContext context,
       std::unique_ptr<folly::IOBuf> toBeSigned),
      (const));
};
} // namespace test
} // namespace server
} // namespace fizz
