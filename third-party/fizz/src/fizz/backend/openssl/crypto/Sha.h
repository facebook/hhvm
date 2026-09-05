/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/Hasher.h>
#include <folly/CPortability.h>
#include <folly/Range.h>

#include <openssl/sha.h>

namespace fizz {
namespace openssl {

namespace detail {

/**
 * Maps a fizz hash tag type (fizz::Sha256, fizz::Sha384, fizz::Sha512) to the
 * corresponding OpenSSL low-level hash context and its init/update/final
 * functions.
 *
 * The low-level API stores the whole hash state in a plain, trivially-copyable
 * struct, which lets `Sha` embed it inline and avoid the per-hash EVP_MD_CTX
 * heap allocation that the EVP interface incurs.
 *
 * These low-level SHA functions (SHA256_Init/Update/Final and friends) are
 * marked deprecated in OpenSSL 3.0 (OSSL_DEPRECATEDIN_3_0). The inline hash
 * state and the elimination of per-hash heap allocation described above
 * justify using the deprecated APIs; the FOLLY_PUSH_WARNING block below
 * suppresses the resulting deprecation warnings.
 */
template <class T>
struct ShaTraits;

FOLLY_PUSH_WARNING
FOLLY_GNU_DISABLE_WARNING("-Wdeprecated-declarations")
FOLLY_MSVC_DISABLE_WARNING(4996)

template <>
struct ShaTraits<fizz::Sha256> {
  using Ctx = SHA256_CTX;
  static inline constexpr auto init = SHA256_Init;
  static inline constexpr auto update = SHA256_Update;
  static inline constexpr auto finalize = SHA256_Final;
};

template <>
struct ShaTraits<fizz::Sha384> {
  using Ctx = SHA512_CTX;
  static inline constexpr auto init = SHA384_Init;
  static inline constexpr auto update = SHA384_Update;
  static inline constexpr auto finalize = SHA384_Final;
};

template <>
struct ShaTraits<fizz::Sha512> {
  using Ctx = SHA512_CTX;
  static inline constexpr auto init = SHA512_Init;
  static inline constexpr auto update = SHA512_Update;
  static inline constexpr auto finalize = SHA512_Final;
};

FOLLY_POP_WARNING

} // namespace detail

/**
 * Hash implementation using OpenSSL's low-level SHA API.
 *
 * The hash state lives inline in the object, so constructing and destroying a
 * hasher performs no heap allocation. `T` is a fizz hash tag type such as
 * fizz::Sha256, and each instantiation is a distinct hasher for that algorithm.
 */
template <class T>
class Sha : public fizz::Hasher {
 public:
  Sha();

  using fizz::Hasher::hash_update;
  Status hash_update(Error& err, folly::ByteRange data) override;
  Status hash_final(Error& err, folly::MutableByteRange out) override;
  Status clone(std::unique_ptr<fizz::Hasher>& ret, Error& err) const override;

  size_t getHashLen() const override;
  size_t getBlockSize() const override;

 private:
  typename detail::ShaTraits<T>::Ctx ctx_;
};

} // namespace openssl
} // namespace fizz
