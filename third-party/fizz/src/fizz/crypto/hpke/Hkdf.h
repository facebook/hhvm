/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Hasher.h>
#include <fizz/crypto/Hkdf.h>

namespace fizz {
namespace hpke {

class Hkdf {
 private:
  Hkdf(folly::ByteRange prefix, const HasherFactoryWithMetadata* hash)
      : prefix_(prefix), hkdf_(hash) {}

 public:
  /**
   * Construct an HKDF for HPKE-v1.
   */
  static Hkdf v1(const HasherFactoryWithMetadata* hash);

  /**
   * Construct an HKDF with a custom prefix.
   */
  static Hkdf withPrefix(
      folly::ByteRange prefix,
      const HasherFactoryWithMetadata* hash);

  std::vector<uint8_t> labeledExtract(
      std::unique_ptr<folly::IOBuf> salt,
      folly::ByteRange label,
      std::unique_ptr<folly::IOBuf> ikm,
      std::unique_ptr<folly::IOBuf> suiteId);
  std::vector<uint8_t> extract(
      std::unique_ptr<folly::IOBuf> salt,
      std::unique_ptr<folly::IOBuf> ikm);
  std::unique_ptr<folly::IOBuf> labeledExpand(
      folly::ByteRange prk,
      folly::ByteRange label,
      std::unique_ptr<folly::IOBuf> info,
      size_t L,
      std::unique_ptr<folly::IOBuf> suiteId);
  std::unique_ptr<folly::IOBuf>
  expand(folly::ByteRange prk, std::unique_ptr<folly::IOBuf> label, size_t L);
  size_t hashLength();

 private:
  folly::ByteRange prefix_;
  fizz::Hkdf hkdf_;
};

} // namespace hpke
} // namespace fizz
