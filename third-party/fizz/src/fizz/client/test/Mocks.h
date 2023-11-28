/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/ClientExtensions.h>
#include <fizz/client/ECHPolicy.h>
#include <fizz/client/PskCache.h>
#include <folly/io/async/test/MockAsyncTransport.h>

namespace fizz {
namespace client {
namespace test {

class MockClientStateMachine : public ClientStateMachine {
 public:
  MOCK_METHOD(
      folly::Optional<Actions>,
      _processConnect,
      (const State&,
       std::shared_ptr<const FizzClientContext> context,
       std::shared_ptr<const CertificateVerifier>,
       folly::Optional<std::string> host,
       folly::Optional<CachedPsk> cachedPsk,
       const std::shared_ptr<ClientExtensions>& extensions,
       folly::Optional<std::vector<ech::ECHConfig>> echConfigs));
  Actions processConnect(
      const State& state,
      std::shared_ptr<const FizzClientContext> context,
      std::shared_ptr<const CertificateVerifier> verifier,
      folly::Optional<std::string> host,
      folly::Optional<CachedPsk> cachedPsk,
      const std::shared_ptr<ClientExtensions>& extensions,
      folly::Optional<std::vector<ech::ECHConfig>> echConfigs) override {
    return *_processConnect(
        state, context, verifier, host, cachedPsk, extensions, echConfigs);
  }

  MOCK_METHOD(
      folly::Optional<Actions>,
      _processSocketData,
      (const State&, folly::IOBufQueue&, Aead::AeadOptions));
  Actions processSocketData(
      const State& state,
      folly::IOBufQueue& queue,
      Aead::AeadOptions options) override {
    return *_processSocketData(state, queue, options);
  }

  MOCK_METHOD(
      folly::Optional<Actions>,
      _processAppWrite,
      (const State&, AppWrite&));
  Actions processAppWrite(const State& state, AppWrite appWrite) override {
    return *_processAppWrite(state, appWrite);
  }

  MOCK_METHOD(
      folly::Optional<Actions>,
      _processEarlyAppWrite,
      (const State&, EarlyAppWrite&));
  Actions processEarlyAppWrite(const State& state, EarlyAppWrite appWrite)
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

template <typename SM>
class MockHandshakeCallbackT : public AsyncFizzClientT<SM>::HandshakeCallback {
 public:
  MOCK_METHOD(void, _fizzHandshakeSuccess, ());
  void fizzHandshakeSuccess(AsyncFizzClientT<SM>*) noexcept override {
    _fizzHandshakeSuccess();
  }

  MOCK_METHOD(void, _fizzHandshakeError, (folly::exception_wrapper));
  void fizzHandshakeError(
      AsyncFizzClientT<SM>*,
      folly::exception_wrapper ew) noexcept override {
    _fizzHandshakeError(std::move(ew));
  }
};

using MockHandshakeCallback = MockHandshakeCallbackT<ClientStateMachine>;

class MockAsyncFizzClient : public AsyncFizzClient {
 public:
  MockAsyncFizzClient()
      : AsyncFizzClient(
            folly::AsyncTransportWrapper::UniquePtr(
                new folly::test::MockAsyncTransport()),
            std::make_shared<FizzClientContext>()) {}
  MOCK_METHOD(
      void,
      connect,
      (HandshakeCallback*,
       std::shared_ptr<const CertificateVerifier>,
       folly::Optional<std::string>,
       folly::Optional<std::string>,
       folly::Optional<std::vector<ech::ECHConfig>>,
       std::chrono::milliseconds));
  MOCK_METHOD(void, close, ());
  MOCK_METHOD(void, closeWithReset, ());
  MOCK_METHOD(void, closeNow, ());
};

class MockPskCache : public PskCache {
 public:
  MOCK_METHOD(
      folly::Optional<CachedPsk>,
      getPsk,
      (const std::string& identity));
  MOCK_METHOD(void, putPsk, (const std::string& identity, CachedPsk));
  MOCK_METHOD(void, removePsk, (const std::string& identity));
};

class MockECHPolicy : public fizz::client::ECHPolicy {
 public:
  MOCK_METHOD(
      folly::Optional<std::vector<fizz::ech::ECHConfig>>,
      getConfig,
      (const std::string& hostname),
      (const));
};

class MockClientExtensions : public ClientExtensions {
 public:
  MOCK_METHOD(std::vector<Extension>, getClientHelloExtensions, (), (const));
  MOCK_METHOD(void, onEncryptedExtensions, (const std::vector<Extension>&));
};
} // namespace test
} // namespace client
} // namespace fizz
