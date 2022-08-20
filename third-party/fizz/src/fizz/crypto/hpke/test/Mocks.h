/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/exchange/X25519.h>
#include <fizz/crypto/hpke/Context.h>
#include <folly/portability/GMock.h>

namespace fizz {
namespace hpke {
namespace test {

class MockAeadCipher : public Aead {
 public:
  explicit MockAeadCipher(std::unique_ptr<Aead> actualCipher)
      : actualCipher_(std::move(actualCipher)) {}

  size_t keyLength() const override {
    return actualCipher_->keyLength();
  }

  size_t ivLength() const override {
    return actualCipher_->ivLength();
  }

  MOCK_METHOD(folly::Optional<TrafficKey>, getKey, (), (const));
  MOCK_METHOD(void, _setKey, (TrafficKey * key));
  void setKey(TrafficKey key) override {
    _setKey(&key);
    actualCipher_->setKey(std::move(key));
  }

  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      AeadOptions options) const override {
    return actualCipher_->encrypt(
        std::move(plaintext), associatedData, seqNum, options);
  }

  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const override {
    return actualCipher_->encrypt(
        std::move(plaintext), associatedData, nonce, options);
  }

  std::unique_ptr<folly::IOBuf> inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const override {
    return actualCipher_->inplaceEncrypt(
        std::move(plaintext), associatedData, seqNum);
  }

  void setEncryptedBufferHeadroom(size_t headroom) override {
    return actualCipher_->setEncryptedBufferHeadroom(headroom);
  }

  std::unique_ptr<folly::IOBuf> decrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      AeadOptions options) const override {
    return actualCipher_->decrypt(
        std::move(ciphertext), associatedData, seqNum, options);
  }

  std::unique_ptr<folly::IOBuf> decrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const override {
    return actualCipher_->decrypt(
        std::move(ciphertext), associatedData, nonce, options);
  }

  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      AeadOptions options) const override {
    return actualCipher_->tryDecrypt(
        std::move(ciphertext), associatedData, seqNum, options);
  }

  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const override {
    return actualCipher_->tryDecrypt(
        std::move(ciphertext), associatedData, nonce, options);
  }

  size_t getCipherOverhead() const override {
    return actualCipher_->getCipherOverhead();
  }

 private:
  std::unique_ptr<Aead> actualCipher_;
};

class MockHpkeContext : public HpkeContext {
 public:
  MOCK_METHOD2(
      _seal,
      std::unique_ptr<folly::IOBuf>(
          const folly::IOBuf* aad,
          std::unique_ptr<folly::IOBuf>& pt));
  std::unique_ptr<folly::IOBuf> seal(
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> pt) override {
    return _seal(aad, pt);
  }

  MOCK_METHOD2(
      _open,
      std::unique_ptr<folly::IOBuf>(
          const folly::IOBuf* aad,
          std::unique_ptr<folly::IOBuf>& ct));
  std::unique_ptr<folly::IOBuf> open(
      const folly::IOBuf* aad,
      std::unique_ptr<folly::IOBuf> pt) override {
    return _open(aad, pt);
  }

  MOCK_CONST_METHOD2(
      _exportSecret,
      std::unique_ptr<folly::IOBuf>(
          std::unique_ptr<folly::IOBuf>& exporterContext,
          size_t desiredLength));
  std::unique_ptr<folly::IOBuf> exportSecret(
      std::unique_ptr<folly::IOBuf> exporterContext,
      size_t desiredLength) const override {
    return _exportSecret(exporterContext, desiredLength);
  }

  MOCK_METHOD0(getExporterSecret, std::unique_ptr<folly::IOBuf>());
};

} // namespace test
} // namespace hpke
} // namespace fizz
