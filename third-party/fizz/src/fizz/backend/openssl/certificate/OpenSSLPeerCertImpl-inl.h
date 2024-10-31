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
OpenSSLPeerCertImpl<T>::OpenSSLPeerCertImpl(folly::ssl::X509UniquePtr cert) {
  folly::ssl::EvpPkeyUniquePtr key(X509_get_pubkey(cert.get()));
  if (!key) {
    throw std::runtime_error("could not get key from cert");
  }
  signature_.setKey(std::move(key));
  cert_ = std::move(cert);
}

template <KeyType T>
std::string OpenSSLPeerCertImpl<T>::getIdentity() const {
  return detail::getIdentityFromX509(cert_.get()).value_or("");
}

template <KeyType T>
inline void OpenSSLPeerCertImpl<T>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  CertUtils::verify<T>(
      signature_, scheme, context, std::move(toBeSigned), std::move(signature));
}

template <KeyType T>
folly::ssl::X509UniquePtr OpenSSLPeerCertImpl<T>::getX509() const {
  X509_up_ref(cert_.get());
  return folly::ssl::X509UniquePtr(cert_.get());
}
} // namespace openssl
} // namespace fizz
