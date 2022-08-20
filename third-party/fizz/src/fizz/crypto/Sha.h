/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/io/IOBuf.h>
#include <folly/ssl/OpenSSLHash.h>

namespace fizz {

/**
 * Hash implementation using OpenSSL.
 *
 * The template struct requires the following parameters:
 *   - HashLen: length of the hash digest
 *   - HashEngine: function returning EVP_MD* to use
 *   - BlankHash: ByteRange containing the digest of a hash of empty input
 */
template <typename T>
class Sha {
 public:
  void hash_init();

  void hash_update(folly::ByteRange data);

  void hash_update(const folly::IOBuf& data);

  void hash_final(folly::MutableByteRange out);

  /**
   * Puts HMAC(key, in) into out. Out must be at least of size HashLen.
   */
  static void hmac(
      folly::ByteRange key,
      const folly::IOBuf& in,
      folly::MutableByteRange out);

  /**
   * Puts Hash(in) into out. Out must be at least of size HashLen.
   */
  static void hash(const folly::IOBuf& in, folly::MutableByteRange out);

 private:
  folly::ssl::OpenSSLHash::Digest digest_;
};
} // namespace fizz
#include <fizz/crypto/Sha-inl.h>
