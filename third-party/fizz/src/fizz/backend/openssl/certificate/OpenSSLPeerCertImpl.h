/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/signature/Signature.h>
#include <fizz/fizz-config.h>
#include <fizz/protocol/Certificate.h>
#include <folly/io/async/ssl/OpenSSLTransportCertificate.h>

namespace fizz {
class PeerCert;
enum class CertificateVerifyContext;

namespace openssl {

#if FIZZ_CERTIFICATE_USE_OPENSSL_CERT
#define FIZZ_MAYBE_OVERRIDE override
#else
#define FIZZ_MAYBE_OVERRIDE
#endif

/**
 * fizz::PeerCert, depending on compilation modes, may or may not declare
 * `getX509()` as part of the public interface. This API change was done
 * in order to prevent a hard dependency on OpenSSL when using fizz vocabulary
 * types (such as fizz::PeerCert).
 *
 * OpenSSLPeerCertBase defines an intermediate interface that forces the
 * presence of `getX509()`. It is a no-op when fizz::PeerCert already has
 * this.
 */
class OpenSSLPeerCertBase : public fizz::PeerCert {
 public:
  virtual ~OpenSSLPeerCertBase() override = default;
  virtual folly::ssl::X509UniquePtr getX509() const FIZZ_MAYBE_OVERRIDE = 0;
};

template <KeyType T>
class OpenSSLPeerCertImpl : public OpenSSLPeerCertBase {
 public:
  static Status create(
      std::unique_ptr<OpenSSLPeerCertImpl>& ret,
      Error& err,
      folly::ssl::X509UniquePtr cert);

  ~OpenSSLPeerCertImpl() override = default;

  [[nodiscard]] std::string getIdentity() const override;

  Status verify(
      Error& err,
      SignatureScheme scheme,
      CertificateVerifyContext context,
      folly::ByteRange toBeSigned,
      folly::ByteRange signature) const override;

  [[nodiscard]] folly::ssl::X509UniquePtr getX509() const override;

  std::optional<std::string> getDER() const override;

 protected:
  OpenSSLPeerCertImpl(
      OpenSSLSignature<T> signature,
      folly::ssl::X509UniquePtr cert);
  OpenSSLSignature<T> signature_;
  folly::ssl::X509UniquePtr cert_;
};

} // namespace openssl
} // namespace fizz

#undef FIZZ_MAYBE_OVERRIDE

#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl-inl.h>
