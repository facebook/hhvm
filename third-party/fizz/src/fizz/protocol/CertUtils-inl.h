/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

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
} // namespace fizz
