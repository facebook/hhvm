/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/crypto/signature/Signature.h>
#include <fizz/crypto/test/TestUtil.h>

#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
namespace test {

struct RSATest;

struct P256Test {
  static constexpr openssl::KeyType Type = openssl::KeyType::P256;
  static constexpr SignatureScheme Scheme =
      SignatureScheme::ecdsa_secp256r1_sha256;

  using Invalid = RSATest;
};

struct P384Test {
  static constexpr openssl::KeyType Type = openssl::KeyType::P384;
  static constexpr SignatureScheme Scheme =
      SignatureScheme::ecdsa_secp384r1_sha384;

  using Invalid = RSATest;
};

struct P521Test {
  static constexpr openssl::KeyType Type = openssl::KeyType::P521;
  static constexpr SignatureScheme Scheme =
      SignatureScheme::ecdsa_secp521r1_sha512;

  using Invalid = RSATest;
};

struct RSATest {
  static constexpr openssl::KeyType Type = openssl::KeyType::RSA;
  static constexpr SignatureScheme Scheme = SignatureScheme::rsa_pss_sha256;

  using Invalid = P256Test;
};

struct Ed25519Test {
  static constexpr openssl::KeyType Type = openssl::KeyType::ED25519;
  static constexpr SignatureScheme Scheme = SignatureScheme::ed25519;

  using Invalid = P256Test;
};

template <typename T>
static folly::ssl::X509UniquePtr getCert();

template <>
folly::ssl::X509UniquePtr getCert<P256Test>() {
  return fizz::test::getCert(kP256Certificate);
}

template <>
folly::ssl::X509UniquePtr getCert<P384Test>() {
  return fizz::test::getCert(kP384Certificate);
}

template <>
folly::ssl::X509UniquePtr getCert<P521Test>() {
  return fizz::test::getCert(kP521Certificate);
}

template <>
folly::ssl::X509UniquePtr getCert<RSATest>() {
  return fizz::test::getCert(kRSACertificate);
}

template <>
folly::ssl::X509UniquePtr getCert<Ed25519Test>() {
  return fizz::test::getCert(kEd25519Certificate);
}

template <typename T>
static folly::ssl::EvpPkeyUniquePtr getKey();

template <>
folly::ssl::EvpPkeyUniquePtr getKey<P256Test>() {
  return getPrivateKey(kP256Key);
}

template <>
folly::ssl::EvpPkeyUniquePtr getKey<P384Test>() {
  return getPrivateKey(kP384Key);
}

template <>
folly::ssl::EvpPkeyUniquePtr getKey<P521Test>() {
  return getPrivateKey(kP521Key);
}

template <>
folly::ssl::EvpPkeyUniquePtr getKey<RSATest>() {
  return getPrivateKey(kRSAKey);
}

template <>
folly::ssl::EvpPkeyUniquePtr getKey<Ed25519Test>() {
  return getPrivateKey(kEd25519Key);
}

} // namespace test
} // namespace fizz
