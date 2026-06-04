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
namespace openssl {

/**
 * Certificate verifier that verifies a certificate against a trusted
 * certificate store
 *
 * This does not perform any identity or hostname verification.
 */
class OpenSSLCertificateVerifier : public CertificateVerifier {
 protected:
  explicit OpenSSLCertificateVerifier(
      VerificationContext context,
      CertificateAuthorities&& authorities)
      : context_(std::move(context)),
        x509Store_(nullptr),
        authorities_(std::move(authorities)) {}

  explicit OpenSSLCertificateVerifier(
      VerificationContext context,
      folly::ssl::X509StoreUniquePtr&& store,
      CertificateAuthorities&& authorities)
      : context_(std::move(context)),
        x509Store_(std::move(store)),
        authorities_(std::move(authorities)) {}

  static Status
  createAuthorities(CertificateAuthorities& ret, Error& err, X509_STORE* store);

 public:
  using X509VerifyCallback = int (*)(int, X509_STORE_CTX*);

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  Status verify(
      std::shared_ptr<const Cert>& ret,
      Error& err,
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs)
      const override;

  /**
   * Return a std::unique_ptr to X509_STORE_CTX which is used for verifying
   * the certificate chain. Additional verification result can be extracted
   * from the returned store context.
   */
  Status verifyWithX509StoreCtx(
      folly::ssl::X509StoreCtxUniquePtr& ret,
      Error& err,
      const std::vector<std::shared_ptr<const fizz::PeerCert>>& certs) const;

  void setCustomVerifyCallback(X509VerifyCallback cb) {
    customVerifyCallback_ = cb;
  }

  Status setX509Store(Error& err, folly::ssl::X509StoreUniquePtr&& store) {
    x509Store_ = std::move(store);
    X509_STORE* storePtr = nullptr;
    if (x509Store_) {
      storePtr = x509Store_.get();
    } else {
      FIZZ_RETURN_ON_ERROR(getDefaultX509Store(storePtr, err));
    }
    return createAuthorities(authorities_, err, storePtr);
  }

  Status getCertificateRequestExtensions(
      std::vector<Extension>& ret,
      Error& err) const override;

  static Status getDefaultX509Store(X509_STORE*& ret, Error& err);

  static Status create(
      std::unique_ptr<OpenSSLCertificateVerifier>& ret,
      Error& err,
      VerificationContext context,
      folly::ssl::X509StoreUniquePtr&& store = nullptr);

  static Status createFromCAFile(
      std::unique_ptr<OpenSSLCertificateVerifier>& ret,
      Error& err,
      VerificationContext context,
      const std::string& caFile);

  static Status createFromCAFiles(
      std::unique_ptr<OpenSSLCertificateVerifier>& ret,
      Error& err,
      VerificationContext context,
      const std::vector<std::string>& caFiles);

 private:
  VerificationContext context_;
  folly::ssl::X509StoreUniquePtr x509Store_;
  CertificateAuthorities authorities_;
  X509VerifyCallback customVerifyCallback_{nullptr};
};

} // namespace openssl
} // namespace fizz
