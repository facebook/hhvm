/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Crypto.h>
#include <fizz/crypto/Hkdf.h>
#include <fizz/record/Types.h>

namespace fizz {

/**
 * Interface for common TLS 1.3 key derivation functions.
 */
class KeyDerivation {
 public:
  virtual ~KeyDerivation() = default;

  virtual size_t hashLength() const = 0;

  /**
   * Returns the hash of a blank input (ie Hash("")).
   */
  virtual folly::ByteRange blankHash() const = 0;

  virtual Buf expandLabel(
      folly::ByteRange secret,
      folly::StringPiece label,
      Buf hashValue,
      uint16_t length) = 0;

  virtual std::vector<uint8_t> deriveSecret(
      folly::ByteRange secret,
      folly::StringPiece label,
      folly::ByteRange messageHash,
      uint16_t length) = 0;

  /**
   * Performs HDKF expansion.
   */
  virtual Buf
  hkdfExpand(folly::ByteRange secret, Buf info, uint16_t length) = 0;

  virtual std::vector<uint8_t> hkdfExtract(
      folly::ByteRange salt,
      folly::ByteRange ikm) = 0;

  virtual std::unique_ptr<KeyDerivation> clone() const = 0;
};

class KeyDerivationImpl : public KeyDerivation {
 public:
  explicit KeyDerivationImpl(const HasherFactoryWithMetadata* hf) : hkdf_(hf) {}

  size_t hashLength() const override {
    return hkdf_.hashLength();
  }

  folly::ByteRange blankHash() const override {
    return hkdf_.hasher()->blankHash();
  }

  Buf expandLabel(
      folly::ByteRange secret,
      folly::StringPiece label,
      Buf hashValue,
      uint16_t length) override;

  std::vector<uint8_t> deriveSecret(
      folly::ByteRange secret,
      folly::StringPiece label,
      folly::ByteRange messageHash,
      uint16_t length) override;

  virtual Buf hkdfExpand(folly::ByteRange secret, Buf info, uint16_t length)
      override;

  std::vector<uint8_t> hkdfExtract(folly::ByteRange salt, folly::ByteRange ikm)
      override {
    return hkdf_.extract(salt, ikm);
  }

  std::unique_ptr<KeyDerivation> clone() const override {
    return std::unique_ptr<KeyDerivation>(
        new KeyDerivationImpl(hkdf_.hasher()));
  }

 private:
  Hkdf hkdf_;
};
} // namespace fizz
