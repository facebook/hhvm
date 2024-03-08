/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Certificate.h>
#include <fizz/record/Extensions.h>

namespace fizz {

class FizzVerificationException : public FizzException {
 public:
  FizzVerificationException(
      const std::string& msg,
      folly::Optional<AlertDescription> alert)
      : FizzException(msg, alert) {}
};

class CertificateVerifier {
 public:
  virtual ~CertificateVerifier() = default;

  /**
   * Verifies the certificates in certs. The peer has been already proven
   * possession of the first certificate in certs. Throws on error or if
   * verification fails.
   * The returned cert will be used for replacing peer cert in fizz State
   * if it is a valid fizz::Cert. Implementation
   * can return a null shared_ptr if replacing the peer cert is not desired.
   */
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  virtual std::shared_ptr<const Cert> verify(
      const std::vector<std::shared_ptr<const PeerCert>>& certs) const = 0;

  /**
   * Returns a vector of extensions to send in a certificate request.
   */
  virtual std::vector<Extension> getCertificateRequestExtensions() const = 0;
};
} // namespace fizz
