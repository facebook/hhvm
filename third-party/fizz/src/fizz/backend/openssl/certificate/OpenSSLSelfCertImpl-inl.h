/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <folly/ssl/OpenSSLCertUtils.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

namespace fizz {
namespace openssl {

namespace detail {
extern folly::Optional<std::string> getIdentityFromX509(X509* x);
}

template <KeyType T>
OpenSSLSelfCertImpl<T>::OpenSSLSelfCertImpl(
    std::vector<folly::ssl::X509UniquePtr> certs,
    OpenSSLSignature<T> signature,
    std::map<CertificateCompressionAlgorithm, CompressedCertificate>
        compressedCerts)
    : signature_(std::move(signature)),
      certs_(std::move(certs)),
      compressedCerts_(std::move(compressedCerts)) {}

template <KeyType T>
/* static */ Status OpenSSLSelfCertImpl<T>::create(
    std::unique_ptr<OpenSSLSelfCertImpl>& ret,
    Error& err,
    folly::ssl::EvpPkeyUniquePtr pkey,
    std::vector<folly::ssl::X509UniquePtr> certs,
    const std::vector<std::shared_ptr<fizz::CertificateCompressor>>&
        compressors) {
  if (certs.empty()) {
    return err.error("Must supply at least 1 cert");
  }
  if (X509_check_private_key(certs[0].get(), pkey.get()) != 1) {
    return err.error("Cert does not match private key");
  }
  // TODO: more strict validation of chaining requirements.
  std::map<CertificateCompressionAlgorithm, CompressedCertificate>
      compressedCerts;
  for (const auto& compressor : compressors) {
    CertificateMsg certMsg;
    FIZZ_RETURN_ON_ERROR(
        CertUtils::getCertMessage(certMsg, err, certs, nullptr));
    compressedCerts[compressor->getAlgorithm()] = compressor->compress(certMsg);
  }
  OpenSSLSignature<T> signature;
  FIZZ_RETURN_ON_ERROR(signature.setKey(err, std::move(pkey)));
  ret = std::unique_ptr<OpenSSLSelfCertImpl>(new OpenSSLSelfCertImpl(
      std::move(certs), std::move(signature), std::move(compressedCerts)));
  return Status::Success;
}

template <KeyType T>
std::string OpenSSLSelfCertImpl<T>::getIdentity() const {
  return detail::getIdentityFromX509(certs_.front().get()).value_or("");
}

template <KeyType T>
std::vector<std::string> OpenSSLSelfCertImpl<T>::getAltIdentities() const {
  return folly::ssl::OpenSSLCertUtils::getSubjectAltNames(*certs_.front());
}

template <KeyType T>
Status OpenSSLSelfCertImpl<T>::getCertMessage(
    CertificateMsg& ret,
    Error& err,
    Buf certificateRequestContext) const {
  return CertUtils::getCertMessage(
      ret, err, certs_, std::move(certificateRequestContext));
}

template <KeyType T>
CompressedCertificate OpenSSLSelfCertImpl<T>::getCompressedCert(
    CertificateCompressionAlgorithm algo) const {
  return CertUtils::cloneCompressedCert(compressedCerts_.at(algo));
}

template <KeyType T>
std::vector<SignatureScheme> OpenSSLSelfCertImpl<T>::getSigSchemes() const {
  return CertUtils::getSigSchemes<T>();
}

template <>
inline Status OpenSSLSelfCertImpl<KeyType::P256>::sign(
    Buf& ret,
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = fizz::certverify::prepareSignData(context, toBeSigned);
  if (scheme == SignatureScheme::ecdsa_secp256r1_sha256) {
    FIZZ_RETURN_ON_ERROR(
        signature_.sign<SignatureScheme::ecdsa_secp256r1_sha256>(
            ret, err, signData->coalesce()));
    return Status::Success;
  }

  return err.error("Unsupported signature scheme");
}

template <>
inline Status OpenSSLSelfCertImpl<KeyType::P384>::sign(
    Buf& ret,
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = fizz::certverify::prepareSignData(context, toBeSigned);
  if (scheme == SignatureScheme::ecdsa_secp384r1_sha384) {
    FIZZ_RETURN_ON_ERROR(
        signature_.sign<SignatureScheme::ecdsa_secp384r1_sha384>(
            ret, err, signData->coalesce()));
    return Status::Success;
  }

  return err.error("Unsupported signature scheme");
}

template <>
inline Status OpenSSLSelfCertImpl<KeyType::P521>::sign(
    Buf& ret,
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = fizz::certverify::prepareSignData(context, toBeSigned);
  if (scheme == SignatureScheme::ecdsa_secp521r1_sha512) {
    FIZZ_RETURN_ON_ERROR(
        signature_.sign<SignatureScheme::ecdsa_secp521r1_sha512>(
            ret, err, signData->coalesce()));
    return Status::Success;
  }

  return err.error("Unsupported signature scheme");
}

template <>
inline Status OpenSSLSelfCertImpl<KeyType::ED25519>::sign(
    Buf& ret,
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = fizz::certverify::prepareSignData(context, toBeSigned);
  if (scheme == SignatureScheme::ed25519) {
    FIZZ_RETURN_ON_ERROR(signature_.sign<SignatureScheme::ed25519>(
        ret, err, signData->coalesce()));
    return Status::Success;
  }

  return err.error("Unsupported signature scheme");
}

template <>
inline Status OpenSSLSelfCertImpl<KeyType::RSA>::sign(
    Buf& ret,
    Error& err,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned) const {
  auto signData = fizz::certverify::prepareSignData(context, toBeSigned);
  if (scheme == SignatureScheme::rsa_pss_sha256) {
    FIZZ_RETURN_ON_ERROR(signature_.sign<SignatureScheme::rsa_pss_sha256>(
        ret, err, signData->coalesce()));
    return Status::Success;
  }

  return err.error("Unsupported signature scheme");
}

template <KeyType T>
folly::ssl::X509UniquePtr OpenSSLSelfCertImpl<T>::getX509() const {
  X509_up_ref(certs_.front().get());
  return folly::ssl::X509UniquePtr(certs_.front().get());
}

template <KeyType T>
std::vector<folly::ssl::X509UniquePtr> OpenSSLSelfCertImpl<T>::getX509Chain()
    const {
  std::vector<folly::ssl::X509UniquePtr> ret;
  ret.reserve(certs_.size());
  for (const auto& cert : certs_) {
    X509_up_ref(cert.get());
    ret.emplace_back(folly::ssl::X509UniquePtr(cert.get()));
  }
  return ret;
}

template <KeyType T>
folly::ssl::EvpPkeyUniquePtr OpenSSLSelfCertImpl<T>::getEVPPkey() const {
  return signature_.getKey();
}

} // namespace openssl
} // namespace fizz
