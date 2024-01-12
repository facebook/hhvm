/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertUtils.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/OpenSSLPeerCertImpl.h>
#include <fizz/protocol/OpenSSLSelfCertImpl.h>
#include <proxygen/lib/http/session/SecondaryAuthManagerBase.h>

namespace proxygen {

class SecondaryAuthManager : public SecondaryAuthManagerBase {
 public:
  explicit SecondaryAuthManager(std::unique_ptr<fizz::SelfCert> cert);

  SecondaryAuthManager() = default;

  ~SecondaryAuthManager() override;

  std::pair<uint16_t, std::unique_ptr<folly::IOBuf>> createAuthRequest(
      std::unique_ptr<folly::IOBuf> certRequestContext,
      std::vector<fizz::Extension> extensions) override;

  std::pair<uint16_t, std::unique_ptr<folly::IOBuf>> getAuthenticator(
      const fizz::AsyncFizzBase& transport,
      TransportDirection dir,
      uint16_t requestId,
      std::unique_ptr<folly::IOBuf> authRequest) override;

  bool validateAuthenticator(
      const fizz::AsyncFizzBase& transport,
      TransportDirection dir,
      uint16_t certId,
      std::unique_ptr<folly::IOBuf> authenticator) override;

  /**
   * Retrieve a Cert-ID given the corresponding Request-ID.
   */
  folly::Optional<uint16_t> getCertId(uint16_t requestId);

  /**
   * Retrieve the peer certificate chain given the corresponding Cert-ID.
   */
  folly::Optional<std::vector<fizz::CertificateEntry>> getPeerCert(
      uint16_t certId);

 private:
  uint16_t requestIdCounter_{0};
  uint16_t certIdCounter_{0};

  /**
   * Verify if the certificate_request_context of the authenticator contains
   * a Request-ID of a previous CERTIFICATE_REQUEST.
   * @param authenticator The received exported authenticator.
   * @return The authenticator request if verification passes.
   */
  folly::Optional<std::unique_ptr<folly::IOBuf>> verifyContext(
      std::unique_ptr<folly::IOBuf> authenticator);

  // Locally cached authenticator requests, used for authenticator validation
  // and the CERTIFICATE_NEEDED frame.
  std::map<uint16_t, std::unique_ptr<folly::IOBuf>> outstandingRequests_;

  // Secondary certificate possessed by the local endpoint.
  std::unique_ptr<fizz::SelfCert> cert_;

  // Caching the Request-ID:Cert-ID mapping which guides the use of
  // USE_CERTIFICATE frame.
  std::map<uint16_t, uint16_t> requestCertMap_;

  // Locally cached certificates which authenticates the secondary identity of
  // the peer.
  std::map<uint16_t, std::vector<fizz::CertificateEntry>> receivedCerts_;
};

} // namespace proxygen
