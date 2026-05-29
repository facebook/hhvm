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

#include <cstdio>
#include <cstdlib>

namespace fizz {

/**
 * Which context we are verifying in (eg client context means we are a client
 * verifying server certificates).
 */
enum class VerificationContext { Client, Server };

class CertificateVerifier {
 public:
  virtual ~CertificateVerifier() = default;

  /**
   * Verifies the certificates in certs. The peer has been already proven
   * possession of the first certificate in certs. Returns error status on
   * error or if verification fails.
   * The returned cert will be used for replacing peer cert in fizz State
   * if it is a valid fizz::Cert. Implementation
   * can return a null shared_ptr if replacing the peer cert is not desired.
   */
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  virtual Status verify(
      std::shared_ptr<const Cert>& ret,
      Error& err,
      const std::vector<std::shared_ptr<const PeerCert>>& certs) const = 0;

  /**
   * Returns a vector of extensions to send in a certificate request.
   */
  virtual Status getCertificateRequestExtensions(
      std::vector<Extension>& ret,
      Error& err) const = 0;
};

/**
 * A CertificateVerifier that terminates the process if verify() is called.
 *
 * This forces callers to configure a real CertificateVerifier rather than
 * silently skipping verification. Used as the DefaultCertificateVerifier
 * on platforms without OpenSSL.
 */
class TerminatingCertificateVerifier : public CertificateVerifier {
 public:
  explicit TerminatingCertificateVerifier(VerificationContext) {}

  Status verify(
      std::shared_ptr<const Cert>& /* ret */,
      Error& /* err */,
      const std::vector<std::shared_ptr<const PeerCert>>&) const override {
    fprintf(
        stderr,
        "DefaultCertificateVerifier is not supported on this platform. "
        "Set CertificateVerifier explicitly.\n");
    std::abort();
  }

  Status getCertificateRequestExtensions(
      std::vector<Extension>& /* ret */,
      Error& /* err */) const override {
    return Status::Success;
  }
};

/**
 * A CertificateVerifier that accepts all certificates without verification.
 *
 * WARNING: This is insecure and should only be used in testing or when
 * certificate verification is handled by another layer.
 */
class InsecureCertificateVerifier : public CertificateVerifier {
 public:
  explicit InsecureCertificateVerifier(VerificationContext) {}

  Status verify(
      std::shared_ptr<const Cert>& ret,
      Error& /* err */,
      const std::vector<std::shared_ptr<const PeerCert>>& certs)
      const override {
    ret = nullptr;
    if (!certs.empty()) {
      ret = certs.front();
    }
    return Status::Success;
  }

  Status getCertificateRequestExtensions(
      std::vector<Extension>& /* ret */,
      Error& /* err */) const override {
    return Status::Success;
  }
};

} // namespace fizz
