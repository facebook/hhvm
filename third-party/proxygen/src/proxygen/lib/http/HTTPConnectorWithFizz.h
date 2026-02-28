/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/AsyncFizzClient.h>
#include <folly/io/SocketOptionMap.h>
#include <proxygen/lib/http/HTTPConnector.h>

/**
 * Extension of the HTTPConnector that uses Fizz to
 * support TLS 1.3 connections.
 **/

namespace proxygen {

class HTTPConnectorWithFizz : public HTTPConnector {
 public:
  using HTTPConnector::HTTPConnector;

  void connectFizz(
      folly::EventBase* eventBase,
      const folly::SocketAddress& connectAddr,
      std::shared_ptr<const fizz::client::FizzClientContext> context,
      std::shared_ptr<const fizz::CertificateVerifier> verifier,
      std::chrono::milliseconds totalTimeout = std::chrono::milliseconds(0),
      std::chrono::milliseconds tcpConnectTimeout =
          std::chrono::milliseconds(0),
      const folly::SocketOptionMap& socketOptions = folly::emptySocketOptionMap,
      const folly::SocketAddress& bindAddr = folly::AsyncSocket::anyAddress(),
      folly::Optional<std::string> sni = folly::none,
      folly::Optional<std::string> pskIdentity = folly::none);

 protected:
  void connectSuccess() noexcept override;
};

} // namespace proxygen
