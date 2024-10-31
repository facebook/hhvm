/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace openssl {

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

template <>
inline void CertUtils::verify<KeyType::P256>(
    const OpenSSLSignature<KeyType::P256>& certSignature,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp256r1_sha256:
      return certSignature.verify<SignatureScheme::ecdsa_secp256r1_sha256>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void CertUtils::verify<KeyType::P384>(
    const OpenSSLSignature<KeyType::P384>& certSignature,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp384r1_sha384:
      return certSignature.verify<SignatureScheme::ecdsa_secp384r1_sha384>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void CertUtils::verify<KeyType::P521>(
    const OpenSSLSignature<KeyType::P521>& certSignature,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ecdsa_secp521r1_sha512:
      return certSignature.verify<SignatureScheme::ecdsa_secp521r1_sha512>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void CertUtils::verify<KeyType::ED25519>(
    const OpenSSLSignature<KeyType::ED25519>& certSignature,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::ed25519:
      return certSignature.verify<SignatureScheme::ed25519>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

template <>
inline void CertUtils::verify<KeyType::RSA>(
    const OpenSSLSignature<KeyType::RSA>& certSignature,
    SignatureScheme scheme,
    CertificateVerifyContext context,
    folly::ByteRange toBeSigned,
    folly::ByteRange signature) {
  auto signData = CertUtils::prepareSignData(context, toBeSigned);
  switch (scheme) {
    case SignatureScheme::rsa_pss_sha256:
      return certSignature.verify<SignatureScheme::rsa_pss_sha256>(
          signData->coalesce(), signature);
    default:
      throw std::runtime_error("Unsupported signature scheme");
  }
}

} // namespace openssl
} // namespace fizz
