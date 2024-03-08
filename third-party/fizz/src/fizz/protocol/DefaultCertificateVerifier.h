/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertificateVerifier.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

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
class DefaultCertificateVerifier : public CertificateVerifier {
 public:
  using X509VerifyCallback = int (*)(int, X509_STORE_CTX*);

  explicit DefaultCertificateVerifier(VerificationContext context)
      : context_(context), x509Store_(nullptr) {
    createAuthorities();
  }

  explicit DefaultCertificateVerifier(
      VerificationContext context,
      folly::ssl::X509StoreUniquePtr&& store)
      : context_(context), x509Store_(std::move(store)) {
    createAuthorities();
  }

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  std::shared_ptr<const Cert> verify(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override;

  /**
   * Return a std::unique_ptr to X509_STORE_CTX which is used for verifying
   * the certificate chain. Additional verification result can be extracted
   * from the returned store context.
   */
  folly::ssl::X509StoreCtxUniquePtr verifyWithX509StoreCtx(
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs) const;

  void setCustomVerifyCallback(X509VerifyCallback cb) {
    customVerifyCallback_ = cb;
  }

  void setX509Store(folly::ssl::X509StoreUniquePtr&& store) {
    x509Store_ = std::move(store);
    createAuthorities();
  }

  std::vector<Extension> getCertificateRequestExtensions() const override;

  static X509_STORE* getDefaultX509Store();

  static std::unique_ptr<DefaultCertificateVerifier> createFromCAFile(
      VerificationContext context,
      const std::string& caFile);

  static std::unique_ptr<DefaultCertificateVerifier> createFromCAFiles(
      VerificationContext context,
      const std::vector<std::string>& caFile);

 private:
  void createAuthorities();

  CertificateAuthorities authorities_;
  VerificationContext context_;
  folly::ssl::X509StoreUniquePtr x509Store_;
  X509VerifyCallback customVerifyCallback_{nullptr};
};
} // namespace fizz
