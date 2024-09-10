// Copyright 2004-present Facebook. All Rights Reserved.
//
// This file associates OpenSSL specific information to the backend-agnostic
// cryptographic primitive tag types (e.g. fizz::Sha256).

#pragma once

#include <fizz/crypto/Crypto.h>
#include <folly/portability/OpenSSL.h>
#include <openssl/evp.h>

namespace fizz {
namespace openssl {

template <class T>
struct Properties;

template <>
struct Properties<fizz::AESGCM128> {
  static const EVP_CIPHER* Cipher() {
    return EVP_aes_128_gcm();
  }

  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

template <>
struct Properties<fizz::AESGCM256> {
  static const EVP_CIPHER* Cipher() {
    return EVP_aes_256_gcm();
  }
  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

template <>
struct Properties<fizz::AESOCB128> {
  static const EVP_CIPHER* Cipher() {
#if !defined(OPENSSL_NO_OCB)
    return EVP_aes_128_ocb();
#else
    throw std::runtime_error(
        "aes-ocb support requires OpenSSL 1.1.0 with ocb enabled");
#endif
  }

  static const bool kOperatesInBlocks{true};
  static const bool kRequiresPresetTagLen{true};
};

template <>
struct Properties<fizz::ChaCha20Poly1305> {
  static const EVP_CIPHER* Cipher() {
#if FOLLY_OPENSSL_HAS_CHACHA
    return EVP_chacha20_poly1305();
#else
    throw std::runtime_error(
        "chacha20-poly1305 support requires OpenSSL 1.1.0");
#endif // FOLLY_OPENSSL_HAS_CHACHA
  }

  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

template <>
struct Properties<fizz::Sha256> {
  static constexpr auto HashEngine = EVP_sha256;
};

template <>
struct Properties<fizz::Sha384> {
  static constexpr auto HashEngine = EVP_sha384;
};

template <>
struct Properties<fizz::Sha512> {
  static constexpr auto HashEngine = EVP_sha512;
};

} // namespace openssl
} // namespace fizz
