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

  Status labeledExtract(
      std::vector<uint8_t>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf> salt,
      folly::ByteRange label,
      std::unique_ptr<folly::IOBuf> ikm,
      std::unique_ptr<folly::IOBuf> suiteId);
  Status extract(
      std::vector<uint8_t>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf> salt,
      std::unique_ptr<folly::IOBuf> ikm);
  Status labeledExpand(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      folly::ByteRange prk,
      folly::ByteRange label,
      std::unique_ptr<folly::IOBuf> info,
      size_t L,
      std::unique_ptr<folly::IOBuf> suiteId);
  Status expand(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      folly::ByteRange prk,
      std::unique_ptr<folly::IOBuf> label,
      size_t L);

  // TODO: Deprecate. Use the Status-returning labeledExtract overload instead.
  std::vector<uint8_t> labeledExtract(
      std::unique_ptr<folly::IOBuf> salt,
      folly::ByteRange label,
      std::unique_ptr<folly::IOBuf> ikm,
      std::unique_ptr<folly::IOBuf> suiteId) {
    std::vector<uint8_t> ret;
    Error err;
    FIZZ_THROW_ON_ERROR(
        labeledExtract(
            ret,
            err,
            std::move(salt),
            label,
            std::move(ikm),
            std::move(suiteId)),
        err);
    return ret;
  }

  // TODO: Deprecate. Use the Status-returning extract overload instead.
  std::vector<uint8_t> extract(
      std::unique_ptr<folly::IOBuf> salt,
      std::unique_ptr<folly::IOBuf> ikm) {
    std::vector<uint8_t> ret;
    Error err;
    FIZZ_THROW_ON_ERROR(
        extract(ret, err, std::move(salt), std::move(ikm)), err);
    return ret;
  }

  size_t hashLength();

 private:
  folly::ByteRange prefix_;
  fizz::Hkdf hkdf_;
};

} // namespace hpke
} // namespace fizz
