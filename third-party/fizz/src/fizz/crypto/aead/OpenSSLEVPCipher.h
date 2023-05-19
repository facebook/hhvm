/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/AESGCM128.h>
#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/aead/IOBufUtil.h>
#include <folly/Conv.h>
#include <folly/Memory.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/lang/Bits.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <glog/logging.h>
#include <openssl/evp.h>

namespace fizz {

/**
 * Aead implementation using an OpenSSL EvpCipher.
 *
 * The template struct requires the following parameters:
 *   - Cipher: function returning EVP_CIPHER*
 *   - kKeyLength: length of key required
 *   - kIvLength: length of iv required
 *   - kTagLength: authentication tag length
 *   - kOperatesInBlocks: if the cipher outputs data in chunks vs. streaming 1:1
 *         with the input
 *   - kRequiresPresetTagLen: if the cipher requires setting the tag length
 *         explicitly
 */
class OpenSSLEVPCipher : public Aead {
 public:
  ~OpenSSLEVPCipher() override = default;
  OpenSSLEVPCipher() = default;

  static constexpr size_t kMaxIVLength = 20;
  static constexpr size_t kMaxTagLength = 20;

  template <class EVPImpl>
  static std::unique_ptr<Aead> makeCipher();

  OpenSSLEVPCipher(OpenSSLEVPCipher&& other) = default;
  OpenSSLEVPCipher& operator=(OpenSSLEVPCipher&& other) = default;

  void setKey(TrafficKey trafficKey) override;
  folly::Optional<TrafficKey> getKey() const override;

  size_t keyLength() const override {
    return keyLength_;
  }

  size_t ivLength() const override {
    return ivLength_;
  }

  // If plaintext is not shared, encrypt in place and append a tag,
  // either in the tail room if available, or by appending a new buf
  // If plaintext is shared, alloc a new output and encrypt to output.
  // The returned buffer will have head room == headroom_
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

  // The same as encrypt(), except always do an inplace encrypt if possible,
  // even if the IOBuf is shared. If there is not enough room for the tag,
  // this will throw an exception.
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
  OpenSSLEVPCipher(
      size_t keyLength,
      size_t ivLength,
      size_t tagLength,
      const EVP_CIPHER* cipher,
      bool operatesInBlocks,
      bool requiresPresetTagLen);

  TrafficKey trafficKey_;
  folly::ByteRange trafficIvKey_;
  size_t headroom_{5};

  // set by the ctor
  size_t keyLength_;
  size_t ivLength_;
  size_t tagLength_;
  const EVP_CIPHER* cipher_;
  bool operatesInBlocks_;
  bool requiresPresetTagLen_;

  folly::ssl::EvpCipherCtxUniquePtr encryptCtx_;
  folly::ssl::EvpCipherCtxUniquePtr decryptCtx_;
};
} // namespace fizz
#include <fizz/crypto/aead/OpenSSLEVPCipher-inl.h>
