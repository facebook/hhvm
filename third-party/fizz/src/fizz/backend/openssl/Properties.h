// Copyright 2004-present Facebook. All Rights Reserved.
//
// This file associates OpenSSL specific information to the backend-agnostic
// cryptographic primitive tag types (e.g. fizz::Sha256).

#pragma once

#include <fizz/crypto/Crypto.h>
#include <fizz/util/Status.h>
#include <folly/portability/OpenSSL.h>
#include <openssl/evp.h>

namespace fizz {
namespace openssl {

template <class T>
struct Properties;

template <>
struct Properties<fizz::AESGCM128> {
  static Status Cipher(const EVP_CIPHER*& ret, Error& /*err*/) {
    ret = EVP_aes_128_gcm();
    return Status::Success;
  }

  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

template <>
struct Properties<fizz::AESGCM256> {
  static Status Cipher(const EVP_CIPHER*& ret, Error& /*err*/) {
    ret = EVP_aes_256_gcm();
    return Status::Success;
  }
  static const bool kOperatesInBlocks{false};
  static const bool kRequiresPresetTagLen{false};
};

template <>
struct Properties<fizz::AESOCB128> {
  static Status Cipher(
      [[maybe_unused]] const EVP_CIPHER*& ret,
      [[maybe_unused]] Error& err) {
#if !defined(OPENSSL_NO_OCB)
    ret = EVP_aes_128_ocb();
    return Status::Success;
#else
    return err.error("aes-ocb support requires OpenSSL 1.1.0 with ocb enabled");
#endif
  }

  static const bool kOperatesInBlocks{true};
  static const bool kRequiresPresetTagLen{true};
};

template <>
struct Properties<fizz::ChaCha20Poly1305> {
  static Status Cipher(
      [[maybe_unused]] const EVP_CIPHER*& ret,
      [[maybe_unused]] [[maybe_unused]] Error& err) {
#if FOLLY_OPENSSL_HAS_CHACHA
    ret = EVP_chacha20_poly1305();
    return Status::Success;
#else
    return err.error("chacha20-poly1305 support requires OpenSSL 1.1.0");
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
