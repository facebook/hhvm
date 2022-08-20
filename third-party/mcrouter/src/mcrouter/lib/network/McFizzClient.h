/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/AsyncFizzClient.h>
#include <folly/io/SocketOptionMap.h>

namespace facebook {
namespace memcache {

class McFizzClient : public fizz::client::AsyncFizzClient {
 public:
  McFizzClient(
      folly::EventBase* eventBase,
      std::shared_ptr<const fizz::client::FizzClientContext> fizzContext,
      std::shared_ptr<const fizz::CertificateVerifier> verifier)
      : fizz::client::AsyncFizzClient(eventBase, std::move(fizzContext)),
        verifier_(std::move(verifier)) {}

  using fizz::client::AsyncFizzClient::connect;

  void connect(
      folly::AsyncSocket::ConnectCallback* callback,
      const folly::SocketAddress& address,
      int timeout,
      const folly::SocketOptionMap& options) {
    auto timeoutChrono = std::chrono::milliseconds(timeout);
    connect(
        address,
        callback,
        verifier_,
        folly::none,
        sessionKey_,
        timeoutChrono,
        timeoutChrono,
        options);
  }

  void setSessionKey(std::string key) {
    sessionKey_ = std::move(key);
  }

 private:
  std::shared_ptr<const fizz::CertificateVerifier> verifier_;
  folly::Optional<std::string> sessionKey_;
};
} // namespace memcache
} // namespace facebook
