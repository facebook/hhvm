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

#if FIZZ_CERTIFICATE_USE_OPENSSL_CERT
#define FIZZ_MAYBE_OVERRIDE override
#else
#define FIZZ_MAYBE_OVERRIDE
#endif

namespace fizz {
class PeerCert;
enum class CertificateVerifyContext;

namespace openssl {

template <KeyType T>
class OpenSSLPeerCertImpl : public fizz::PeerCert {
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

  [[nodiscard]] folly::ssl::X509UniquePtr getX509() const FIZZ_MAYBE_OVERRIDE;

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
