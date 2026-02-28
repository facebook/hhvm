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

#pragma once

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <thread>

#include <wangle/acceptor/AcceptorHandshakeManager.h>

template <class T>
struct UseSharedPtrPolicy;

template <class T>
struct UseOwnedRawPtrPolicy;

template <
    template <class> class UniquePtrTranslationPolicy = UseSharedPtrPolicy>
class MockHandshakeHelperCallback
    : public wangle::AcceptorHandshakeHelper::Callback,
      public UniquePtrTranslationPolicy<
          MockHandshakeHelperCallback<UniquePtrTranslationPolicy>> {
 public:
  MOCK_METHOD3(
      connectionError_,
      void(
          folly::AsyncTransport*,
          folly::exception_wrapper,
          folly::Optional<wangle::SSLErrorEnum>));
  void connectionError(
      folly::AsyncTransport* transport,
      folly::exception_wrapper ex,
      folly::Optional<wangle::SSLErrorEnum> sslErr) noexcept override {
    connectionError_(transport, ex, sslErr);
  }

  void connectionReady(
      folly::AsyncTransport::UniquePtr transport,
      std::string nextProtocol,
      SecureTransportType secureTransportType,
      folly::Optional<wangle::SSLErrorEnum> sslErr) noexcept override {
    MockHandshakeHelperCallback::dispatchConnectionReady(
        std::move(transport),
        std::move(nextProtocol),
        secureTransportType,
        std::move(sslErr));
  }
};

template <
    template <class> class UniquePtrTranslationPolicy = UseSharedPtrPolicy>
class MockHandshakeHelper
    : public wangle::AcceptorHandshakeHelper,
      public UniquePtrTranslationPolicy<
          MockHandshakeHelper<UniquePtrTranslationPolicy>> {
 public:
  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      AcceptorHandshakeHelper::Callback* callback) noexcept override {
    MockHandshakeHelper::dispatchStart(std::move(sock), callback);
  }

  MOCK_METHOD1(dropConnection, void(wangle::SSLErrorEnum reason));
};

template <template <class> class P>
struct UseSharedPtrPolicy<MockHandshakeHelper<P>> {
 public:
  MOCK_METHOD2(
      startInternal,
      void(
          std::shared_ptr<folly::AsyncSSLSocket>,
          wangle::AcceptorHandshakeHelper::Callback*));

  void dispatchStart(
      folly::AsyncSSLSocket::UniquePtr sock,
      wangle::AcceptorHandshakeHelper::Callback* callback) noexcept {
    startInternal(
        std::shared_ptr<folly::AsyncSSLSocket>(std::move(sock)), callback);
  }
};

template <template <class> class P>
struct UseOwnedRawPtrPolicy<MockHandshakeHelper<P>> {
 public:
  MOCK_METHOD2(
      startInternal,
      void(folly::AsyncSSLSocket*, wangle::AcceptorHandshakeHelper::Callback*));

  void dispatchStart(
      folly::AsyncSSLSocket::UniquePtr sock,
      wangle::AcceptorHandshakeHelper::Callback* callback) noexcept {
    startInternal(sock.release(), callback);
  }
};

template <template <class> class P>
struct UseSharedPtrPolicy<MockHandshakeHelperCallback<P>> {
  MOCK_METHOD4(
      connectionReadyInternal,
      void(
          std::shared_ptr<folly::AsyncTransport>,
          std::string,
          SecureTransportType,
          folly::Optional<wangle::SSLErrorEnum>));

  void dispatchConnectionReady(
      folly::AsyncTransport::UniquePtr transport,
      std::string nextProtocol,
      SecureTransportType secureTransportType,
      folly::Optional<wangle::SSLErrorEnum> sslErr) {
    connectionReadyInternal(
        std::move(transport),
        std::move(nextProtocol),
        secureTransportType,
        std::move(sslErr));
  }
};

template <template <class> class P>
struct UseOwnedRawPtrPolicy<MockHandshakeHelperCallback<P>> {
  MOCK_METHOD4(
      connectionReadyInternalRaw,
      void(
          folly::AsyncTransport*,
          std::string,
          SecureTransportType,
          folly::Optional<wangle::SSLErrorEnum>));

  void dispatchConnectionReady(
      folly::AsyncTransport::UniquePtr transport,
      std::string nextProtocol,
      SecureTransportType secureTransportType,
      folly::Optional<wangle::SSLErrorEnum> sslErr) {
    connectionReadyInternalRaw(
        transport.release(),
        std::move(nextProtocol),
        secureTransportType,
        std::move(sslErr));
  }
};
