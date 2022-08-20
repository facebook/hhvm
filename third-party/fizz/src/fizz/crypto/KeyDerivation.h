/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

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

  virtual void hash(const folly::IOBuf& in, folly::MutableByteRange out) = 0;

  virtual void hmac(
      folly::ByteRange key,
      const folly::IOBuf& in,
      folly::MutableByteRange out) = 0;

  virtual std::unique_ptr<KeyDerivation> clone() const = 0;
};

class KeyDerivationImpl : public KeyDerivation {
 public:
  ~KeyDerivationImpl() override = default;

  template <typename Hash>
  static KeyDerivationImpl create(const std::string& labelPrefix) {
    return KeyDerivationImpl(
        labelPrefix,
        Hash::HashLen,
        &Hash::hash,
        &Hash::hmac,
        HkdfImpl::create<Hash>(),
        Hash::BlankHash);
  }

  template <typename Hash>
  static std::unique_ptr<KeyDerivationImpl> make(
      const std::string& labelPrefix) {
    return std::unique_ptr<KeyDerivationImpl>(new KeyDerivationImpl(
        labelPrefix,
        Hash::HashLen,
        &Hash::hash,
        &Hash::hmac,
        HkdfImpl::create<Hash>(),
        Hash::BlankHash));
  }

  size_t hashLength() const override {
    return hashLength_;
  }

  void hash(const folly::IOBuf& in, folly::MutableByteRange out) override {
    hashFunc_(in, out);
  }

  void hmac(
      folly::ByteRange key,
      const folly::IOBuf& in,
      folly::MutableByteRange out) override {
    hmacFunc_(key, in, out);
  }

  folly::ByteRange blankHash() const override {
    return blankHash_;
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
    return std::unique_ptr<KeyDerivation>(new KeyDerivationImpl(
        labelPrefix_, hashLength_, hashFunc_, hmacFunc_, hkdf_, blankHash_));
  }

 private:
  using HashFunc = void (*)(const folly::IOBuf&, folly::MutableByteRange);
  using HmacFunc =
      void (*)(folly::ByteRange, const folly::IOBuf&, folly::MutableByteRange);

  KeyDerivationImpl(
      const std::string& labelPrefix,
      size_t hashLength,
      HashFunc hashFunc,
      HmacFunc hmacFunc,
      HkdfImpl hkdf,
      folly::ByteRange blankHash);

  std::string labelPrefix_;
  size_t hashLength_;
  HashFunc hashFunc_;
  HmacFunc hmacFunc_;
  HkdfImpl hkdf_;
  folly::ByteRange blankHash_;
};
} // namespace fizz
