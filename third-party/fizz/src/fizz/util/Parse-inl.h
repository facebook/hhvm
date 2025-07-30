/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/Types.h>
#include <map>

namespace fizz {
template <>
inline CipherSuite parse(folly::StringPiece s) {
  static const std::map<folly::StringPiece, CipherSuite> stringToCiphers = {
      {"TLS_AES_128_GCM_SHA256", CipherSuite::TLS_AES_128_GCM_SHA256},
      {"TLS_AES_256_GCM_SHA384", CipherSuite::TLS_AES_256_GCM_SHA384},
      {"TLS_CHACHA20_POLY1305_SHA256",
       CipherSuite::TLS_CHACHA20_POLY1305_SHA256},
      {"TLS_AES_128_OCB_SHA256_EXPERIMENTAL",
       CipherSuite::TLS_AES_128_OCB_SHA256_EXPERIMENTAL},
      {"TLS_AEGIS_128L_SHA256", CipherSuite::TLS_AEGIS_128L_SHA256},
      {"TLS_AEGIS_256_SHA512", CipherSuite::TLS_AEGIS_256_SHA512}};

  auto location = stringToCiphers.find(s);
  if (location != stringToCiphers.end()) {
    return location->second;
  }

  throw std::runtime_error(folly::to<std::string>("Unknown cipher suite: ", s));
}

template <>
inline SignatureScheme parse(folly::StringPiece s) {
  static const std::map<folly::StringPiece, SignatureScheme> stringToSchemes = {
      {"ecdsa_secp256r1_sha256", SignatureScheme::ecdsa_secp256r1_sha256},
      {"ecdsa_secp384r1_sha384", SignatureScheme::ecdsa_secp384r1_sha384},
      {"ecdsa_secp521r1_sha512", SignatureScheme::ecdsa_secp521r1_sha512},
      {"rsa_pss_sha256", SignatureScheme::rsa_pss_sha256},
      {"rsa_pss_sha384", SignatureScheme::rsa_pss_sha384},
      {"rsa_pss_sha512", SignatureScheme::rsa_pss_sha512},
      {"ed25519", SignatureScheme::ed25519},
      {"ed448", SignatureScheme::ed448}};

  auto location = stringToSchemes.find(s);
  if (location != stringToSchemes.end()) {
    return location->second;
  }

  throw std::runtime_error(
      folly::to<std::string>("Unknown signature scheme: ", s));
}

template <>
inline NamedGroup parse(folly::StringPiece s) {
  static const std::map<folly::StringPiece, NamedGroup> stringToGroups = {
      {"secp256r1", NamedGroup::secp256r1},
      {"secp384r1", NamedGroup::secp384r1},
      {"secp521r1", NamedGroup::secp521r1},
      {"x25519", NamedGroup::x25519},
      {"x25519_kyber512", NamedGroup::x25519_kyber512},
      {"x25519_kyber768_draft00", NamedGroup::x25519_kyber768_draft00},
      {"x25519_kyber768_experimental",
       NamedGroup::x25519_kyber768_experimental},
      {"x25519_kyber512_experimental",
       NamedGroup::x25519_kyber512_experimental},
      {"X25519MLKEM512_FB", NamedGroup::X25519MLKEM512_FB},
      {"X25519MLKEM768", NamedGroup::X25519MLKEM768},
      {"secp256r1_kyber768_draft00", NamedGroup::secp256r1_kyber768_draft00},
      {"MLKEM512", NamedGroup::MLKEM512},
      {"MLKEM768", NamedGroup::MLKEM768},
      {"MLKEM1024", NamedGroup::MLKEM1024}};

  auto location = stringToGroups.find(s);
  if (location != stringToGroups.end()) {
    return location->second;
  }

  throw std::runtime_error(folly::to<std::string>("Unknown named group: ", s));
}

template <>
inline CertificateCompressionAlgorithm parse(folly::StringPiece s) {
  static const std::map<folly::StringPiece, CertificateCompressionAlgorithm>
      stringToAlgos = {
          {"zlib", CertificateCompressionAlgorithm::zlib},
          {"brotli", CertificateCompressionAlgorithm::brotli},
          {"zstd", CertificateCompressionAlgorithm::zstd}};

  auto location = stringToAlgos.find(s);
  if (location != stringToAlgos.end()) {
    return location->second;
  }

  throw std::runtime_error(
      folly::to<std::string>("Unknown compression algorithm: ", s));
}
} // namespace fizz
