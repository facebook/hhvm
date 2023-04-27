/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/AsyncFizzBase.h>
#include <proxygen/lib/http/codec/TransportDirection.h>

namespace proxygen {

class SecondaryAuthManagerBase {
 public:
  virtual ~SecondaryAuthManagerBase() = default;

  /**
   * Generate an authenticator request given a certificate_request_context and
   * a set of extensions.
   * @return (request ID, encoded authenticator request)
   */
  virtual std::pair<uint16_t, std::unique_ptr<folly::IOBuf>> createAuthRequest(
      std::unique_ptr<folly::IOBuf> certRequestContext,
      std::vector<fizz::Extension> extensions) = 0;

  /**
   * Generate an authenticator request given the Request-ID and authenticator
   * request..
   * @return (cert ID, encoded authenticator)
   */
  virtual std::pair<uint16_t, std::unique_ptr<folly::IOBuf>> getAuthenticator(
      const fizz::AsyncFizzBase& transport,
      TransportDirection dir,
      uint16_t requestId,
      std::unique_ptr<folly::IOBuf> authRequest) = 0;

  /**
   * Validate an authenticator and cache the received certificate along with the
   * Cert-ID if it is valid.
   */
  virtual bool validateAuthenticator(
      const fizz::AsyncFizzBase& transport,
      TransportDirection dir,
      uint16_t certId,
      std::unique_ptr<folly::IOBuf> authenticator) = 0;
};

} // namespace proxygen
