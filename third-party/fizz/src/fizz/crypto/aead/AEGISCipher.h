/*
 *  Copyright (c) Meta Platforms, Inc. and affiliates.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fizz/fizz-config.h>

#if FIZZ_HAS_AEGIS

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/aead/IOBufUtil.h>
#include <folly/Conv.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/lang/Bits.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {
class AEGISCipher : public Aead {
 public:
  using EncryptFn = int (*const)(
      unsigned char* c,
      unsigned long long* clen_p,
      const unsigned char* m,
      unsigned long long mlen,
      const unsigned char* ad,
      unsigned long long adlen,
      const unsigned char* nsec,
      const unsigned char* npub,
      const unsigned char* k);
  using DecryptFn = int (*const)(
      unsigned char* m,
      unsigned long long* mlen_p,
      unsigned char* nsec,
      const unsigned char* c,
      unsigned long long clen,
      const unsigned char* ad,
      unsigned long long adlen,
      const unsigned char* npub,
      const unsigned char* k);

  static constexpr size_t kMaxIVLength = 32;

  static std::unique_ptr<Aead> make128L();
  static std::unique_ptr<Aead> make256();

  void setKey(TrafficKey trafficKey) override;
  folly::Optional<TrafficKey> getKey() const override;

  size_t keyLength() const override {
    return keyLength_;
  }

  size_t ivLength() const override {
    return ivLength_;
  }

  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override;

  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override;

  // TODO: (T136805571) We will add implementation for inplace encryption later
  std::unique_ptr<folly::IOBuf> inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const override;

  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override;

  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override;

  size_t getCipherOverhead() const override;

  void setEncryptedBufferHeadroom(size_t headroom) override {
    headroom_ = headroom;
  }

 private:
  AEGISCipher(
      EncryptFn encrypt,
      DecryptFn decrypt,
      size_t keyLength,
      size_t ivLength,
      size_t tagLength);

  TrafficKey trafficKey_;
  folly::ByteRange trafficIvKey_;
  folly::ByteRange trafficKeyKey_;
  size_t headroom_{0};

  // set by the ctor
  EncryptFn encrypt_;
  DecryptFn decrypt_;
  size_t keyLength_;
  size_t ivLength_;
  size_t tagLength_;
};
} // namespace fizz
#endif
