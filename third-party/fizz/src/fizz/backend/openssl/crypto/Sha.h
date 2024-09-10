/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Hasher.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/ssl/OpenSSLHash.h>

namespace fizz {
namespace openssl {

/**
 * Hash implementation using OpenSSL.
 *
 * The template struct requires the following parameters:
 *   - HashLen: length of the hash digest
 *   - HashEngine: function returning EVP_MD* to use
 *   - BlankHash: ByteRange containing the digest of a hash of empty input
 */
template <typename T>
class Sha : public fizz::Hasher {
 public:
  Sha() {
    digest_.hash_init(T::HashEngine());
  }

  using fizz::Hasher::hash_update;
  void hash_update(folly::ByteRange data) override;

  void hash_final(folly::MutableByteRange out) override;

  std::unique_ptr<fizz::Hasher> clone() const override;

  size_t getHashLen() const override;

  size_t getBlockSize() const override;

  static void hmac(
      folly::ByteRange key,
      const folly::IOBuf& in,
      folly::MutableByteRange out);

  static void hash(const folly::IOBuf& in, folly::MutableByteRange out);

 private:
  folly::ssl::OpenSSLHash::Digest digest_;
};
} // namespace openssl
} // namespace fizz
#include <fizz/backend/openssl/crypto/Sha-inl.h>
