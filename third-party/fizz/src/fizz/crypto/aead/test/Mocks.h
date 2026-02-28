/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <folly/portability/GMock.h>

namespace fizz {
namespace test {

/* using override */
using namespace testing;

class MockAead : public Aead {
 public:
  MOCK_METHOD(size_t, keyLength, (), (const));
  MOCK_METHOD(size_t, ivLength, (), (const));
  MOCK_METHOD(size_t, getCipherOverhead, (), (const));
  MOCK_METHOD(void, setEncryptedBufferHeadroom, (size_t));

  MOCK_METHOD(void, _setKey, (TrafficKey & key));
  void setKey(TrafficKey key) override {
    return _setKey(key);
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _encrypt,
      (std::unique_ptr<folly::IOBuf> & plaintext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum,
       Aead::AeadOptions options),
      (const));
  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override {
    return _encrypt(plaintext, associatedData, seqNum, options);
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _encryptNonce,
      (std::unique_ptr<folly::IOBuf> & plaintext,
       const folly::IOBuf* associatedData,
       folly::ByteRange nonce,
       Aead::AeadOptions options),
      (const));
  std::unique_ptr<folly::IOBuf> encrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const override {
    return _encryptNonce(plaintext, associatedData, nonce, options);
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _inplaceEncrypt,
      (std::unique_ptr<folly::IOBuf> & plaintext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum),
      (const));
  std::unique_ptr<folly::IOBuf> inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const override {
    return _inplaceEncrypt(plaintext, associatedData, seqNum);
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _decrypt,
      (std::unique_ptr<folly::IOBuf> & ciphertext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum,
       Aead::AeadOptions options),
      (const));
  std::unique_ptr<folly::IOBuf> decrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override {
    return _decrypt(ciphertext, associatedData, seqNum, options);
  }

  MOCK_METHOD(
      folly::Optional<std::unique_ptr<folly::IOBuf>>,
      _tryDecrypt,
      (std::unique_ptr<folly::IOBuf> & ciphertext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum,
       Aead::AeadOptions options),
      (const));
  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override {
    return _tryDecrypt(ciphertext, associatedData, seqNum, options);
  }

  MOCK_METHOD(
      folly::Optional<std::unique_ptr<folly::IOBuf>>,
      _tryDecryptNonce,
      (std::unique_ptr<folly::IOBuf> & ciphertext,
       const folly::IOBuf* associatedData,
       folly::ByteRange nonce,
       Aead::AeadOptions options),
      (const));
  folly::Optional<std::unique_ptr<folly::IOBuf>> tryDecrypt(
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override {
    return _tryDecryptNonce(ciphertext, associatedData, nonce, options);
  }

  MOCK_METHOD(folly::Optional<TrafficKey>, getKey, (), (const));

  void setDefaults() {
    ON_CALL(*this, _encrypt(_, _, _, _)).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("ciphertext");
    }));
    ON_CALL(*this, _encryptNonce(_, _, _, _))
        .WillByDefault(InvokeWithoutArgs(
            []() { return folly::IOBuf::copyBuffer("ciphertext"); }));
    ON_CALL(*this, _decrypt(_, _, _, _)).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("plaintext");
    }));
    ON_CALL(*this, _tryDecrypt(_, _, _, _))
        .WillByDefault(InvokeWithoutArgs(
            []() { return folly::IOBuf::copyBuffer("plaintext"); }));
  }
};
} // namespace test
} // namespace fizz
