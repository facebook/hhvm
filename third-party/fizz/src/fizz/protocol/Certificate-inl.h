/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/ScopeGuard.h>
#include <folly/ssl/OpenSSLCertUtils.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace fizz {

namespace detail {
folly::Optional<std::string> getIdentityFromX509(X509* x);
}

template <>
inline std::vector<SignatureScheme> CertUtils::getSigSchemes<KeyType::P256>() {
  return {SignatureScheme::ecdsa_secp256r1_sha256};
}

template <>
inline std::vector<SignatureScheme> CertUtils::getSigSchemes<KeyType::P384>() {
  return {SignatureScheme::ecdsa_secp384r1_sha384};
}

template <>
inline std::vector<SignatureScheme> CertUtils::getSigSchemes<KeyType::P521>() {
  return {SignatureScheme::ecdsa_secp521r1_sha512};
}

template <>
inline std::vector<SignatureScheme> CertUtils::getSigSchemes<KeyType::RSA>() {
  return {SignatureScheme::rsa_pss_sha256};
}

template <>
inline std::vector<SignatureScheme>
CertUtils::getSigSchemes<KeyType::ED25519>() {
  return {SignatureScheme::ed25519};
}

template <KeyType T>
SelfCertImpl<T>::SelfCertImpl(std::vector<folly::ssl::X509UniquePtr> certs)
    : certs_(std::move(certs)) {}

template <KeyType T>
SelfCertImpl<T>::SelfCertImpl(
    folly::ssl::EvpPkeyUniquePtr pkey,
    std::vector<folly::ssl::X509UniquePtr> certs,
    const std::vector<std::shared_ptr<fizz::CertificateCompressor>>&
        compressors) {
  if (certs.size() == 0) {
    throw std::runtime_error("Must supply at least 1 cert");
  }
  if (X509_check_private_key(certs[0].get(), pkey.get()) != 1) {
    throw std::runtime_error("Cert does not match private key");
  }
  // TODO: more strict validation of chaining requirements.
  signature_.setKey(std::move(pkey));
  certs_ = std::move(certs);
  for (const auto& compressor : compressors) {
    compressedCerts_[compressor->getAlgorithm()] =
        compressor->compress(getCertMessage());
  }
}

template <KeyType T>
std::string SelfCertImpl<T>::getIdentity() const {
  return detail::getIdentityFromX509(certs_.front().get()).value_or("");
}

template <KeyType T>
std::vector<std::string> SelfCertImpl<T>::getAltIdentities() const {
  return folly::ssl::OpenSSLCertUtils::getSubjectAltNames(*certs_.front());
}

template <KeyType T>
CertificateMsg SelfCertImpl<T>::getCertMessage(
    Buf certificateRequestContext) const {
  return CertUtils::getCertMessage(
      certs_, std::move(certificateRequestContext));
}

template <KeyType T>
CompressedCertificate SelfCertImpl<T>::getCompressedCert(
    CertificateCompressionAlgorithm algo) const {
  return CertUtils::cloneCompressedCert(compressedCerts_.at(algo));
}

template <KeyType T>
std::vector<SignatureScheme> SelfCertImpl<T>::getSigSchemes() const {
  return CertUtils::getSigSchemes<T>();
}

template <>
inline Buf SelfCertImpl<KeyType::P256>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256:
      return signature_.sign<SignatureScheme::ecdsa_secp256r1_sha256>(
          signData->coalesce());
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline Buf SelfCertImpl<KeyType::P384>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp384r1_sha384:
      return signature_.sign<SignatureScheme::ecdsa_secp384r1_sha384>(
          signData->coalesce());
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline Buf SelfCertImpl<KeyType::P521>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp521r1_sha512:
      return signature_.sign<SignatureScheme::ecdsa_secp521r1_sha512>(
          signData->coalesce());
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline Buf SelfCertImpl<KeyType::ED25519>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ed25519:
      return signature_.sign<SignatureScheme::ed25519>(signData->coalesce());
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline Buf SelfCertImpl<KeyType::RSA>::sign(
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::rsa_pss_sha256:
      return signature_.sign<SignatureScheme::rsa_pss_sha256>(
          signData->coalesce());
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <KeyType T>
PeerCertImpl<T>::PeerCertImpl(folly::ssl::X509UniquePtr cert) {
  folly::ssl::EvpPkeyUniquePtr key(X509_get_pubkey(cert.get()));
  if (!key) {
    throw std::runtime_error("could not get key from cert");
  }
  signature_.setKey(std::move(key));
  cert_ = std::move(cert);
}

template <KeyType T>
std::string PeerCertImpl<T>::getIdentity() const {
  return detail::getIdentityFromX509(cert_.get()).value_or("");
}

template <>
inline void PeerCertImpl<KeyType::P256>::verify(
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
inline void PeerCertImpl<KeyType::P384>::verify(
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
inline void PeerCertImpl<KeyType::P521>::verify(
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
inline void PeerCertImpl<KeyType::ED25519>::verify(
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
inline void PeerCertImpl<KeyType::RSA>::verify(
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
folly::ssl::X509UniquePtr PeerCertImpl<T>::getX509() const {
  X509_up_ref(cert_.get());
  return folly::ssl::X509UniquePtr(cert_.get());
}

template <KeyType T>
folly::ssl::X509UniquePtr SelfCertImpl<T>::getX509() const {
  X509_up_ref(certs_.front().get());
  return folly::ssl::X509UniquePtr(certs_.front().get());
}
} // namespace fizz
