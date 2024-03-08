/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertificateVerifier.h>

namespace fizz {

/**
 * Which context we are verifying in (ie client context means we are a client
 * verifying server certificates).
 */
enum class VerificationContext { Client, Server };

/**
 * Certificate verifier that verifies a certificate against a trusted
 * certificate store
 *
 * This does not perform any identity or hostname verification.
 */
class JavaCryptoCertificateVerifier : public CertificateVerifier {
 public:
  explicit JavaCryptoCertificateVerifier(VerificationContext context)
      : context_(context), x509Store_(nullptr) {
    createAuthorities();
  }
  explicit JavaCryptoCertificateVerifier(
      VerificationContext context,
      folly::ssl::X509StoreUniquePtr&& store)
      : context_(context), x509Store_(std::move(store)) {
    createAuthorities();
  }

  std::shared_ptr<const Cert> verify(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override;

  void setX509Store(folly::ssl::X509StoreUniquePtr&& store) {
    x509Store_ = std::move(store);
    createAuthorities();
  }

  std::vector<Extension> getCertificateRequestExtensions() const override;

  static X509_STORE* getDefaultX509Store();

  static std::unique_ptr<JavaCryptoCertificateVerifier> createFromCAFile(
      VerificationContext context,
      const std::string& caFile);

 private:
  void createAuthorities();
  CertificateAuthorities authorities_;
  VerificationContext context_;
  folly::ssl::X509StoreUniquePtr x509Store_;
};
} // namespace fizz
