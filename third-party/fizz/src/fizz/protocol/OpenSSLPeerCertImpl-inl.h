/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/CertUtils.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace fizz {

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

template <>
inline void OpenSSLPeerCertImpl<KeyType::P256>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256:
      return signature_.verify<SignatureScheme::ecdsa_secp256r1_sha256>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void OpenSSLPeerCertImpl<KeyType::P384>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp384r1_sha384:
      return signature_.verify<SignatureScheme::ecdsa_secp384r1_sha384>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void OpenSSLPeerCertImpl<KeyType::P521>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp521r1_sha512:
      return signature_.verify<SignatureScheme::ecdsa_secp521r1_sha512>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void OpenSSLPeerCertImpl<KeyType::ED25519>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ed25519:
      return signature_.verify<SignatureScheme::ed25519>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void OpenSSLPeerCertImpl<KeyType::RSA>::verify(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::rsa_pss_sha256:
      return signature_.verify<SignatureScheme::rsa_pss_sha256>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <KeyType T>
folly::ssl::X509UniquePtr OpenSSLPeerCertImpl<T>::getX509() const {
  X509_up_ref(cert_.get());
  return folly::ssl::X509UniquePtr(cert_.get());
}
} // namespace fizz
