/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace fizz {
namespace openssl {

namespace detail {
extern folly::Optional<std::string> getIdentityFromX509(X509* x);
}

template <KeyType T>
OpenSSLPeerCertImpl<T>::OpenSSLPeerCertImpl(
    folly::ssl::EvpPkeyUniquePtr pkey,
    folly::ssl::X509UniquePtr cert)
    : cert_(std::move(cert)) {
  signature_.setKey(std::move(pkey));
}

template <KeyType T>
/* static */ Status OpenSSLPeerCertImpl<T>::create(
    std::unique_ptr<OpenSSLPeerCertImpl>& ret,
    Error& err,
    folly::ssl::X509UniquePtr cert) {
  folly::ssl::EvpPkeyUniquePtr pkey(X509_get_pubkey(cert.get()));
  if (!pkey) {
    return err.error("could not get key from cert");
  }
  ret = std::unique_ptr<OpenSSLPeerCertImpl>(
      new OpenSSLPeerCertImpl(std::move(pkey), std::move(cert)));
  return Status::Success;
}

template <KeyType T>
std::string OpenSSLPeerCertImpl<T>::getIdentity() const {
  return detail::getIdentityFromX509(cert_.get()).value_or("");
}

template <KeyType T>
inline Status OpenSSLPeerCertImpl<T>::verify(
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  FIZZ_RETURN_ON_ERROR(
      CertUtils::verify<T>(
          err,
          signature_,
          scheme,
          context,
          std::move(toBeSigned),
          std::move(signature)));
  return Status::Success;
}

template <KeyType T>
folly::ssl::X509UniquePtr OpenSSLPeerCertImpl<T>::getX509() const {
  X509_up_ref(cert_.get());
  return folly::ssl::X509UniquePtr(cert_.get());
}
} // namespace openssl
} // namespace fizz
