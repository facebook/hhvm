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
  Status setKey(Error& /* err */, TrafficKey key) override {
    _setKey(key);
    return Status::Success;
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _encrypt,
      (std::unique_ptr<folly::IOBuf> & plaintext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum,
       Aead::AeadOptions options),
      (const));
  Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& /* err */,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override {
    ret = _encrypt(plaintext, associatedData, seqNum, options);
    return Status::Success;
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _encryptNonce,
      (std::unique_ptr<folly::IOBuf> & plaintext,
       const folly::IOBuf* associatedData,
       folly::ByteRange nonce,
       Aead::AeadOptions options),
      (const));
  Status encrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& /* err */,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      AeadOptions options) const override {
    ret = _encryptNonce(plaintext, associatedData, nonce, options);
    return Status::Success;
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _inplaceEncrypt,
      (std::unique_ptr<folly::IOBuf> & plaintext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum),
      (const));
  Status inplaceEncrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& /* err */,
      std::unique_ptr<folly::IOBuf>&& plaintext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum) const override {
    ret = _inplaceEncrypt(plaintext, associatedData, seqNum);
    return Status::Success;
  }

  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _decrypt,
      (std::unique_ptr<folly::IOBuf> & ciphertext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum,
       Aead::AeadOptions options),
      (const));
  Status decrypt(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override {
    FIZZ_THROW_TO_ERROR(
        ret, _decrypt(ciphertext, associatedData, seqNum, options));
  }

  MOCK_METHOD(
      folly::Optional<std::unique_ptr<folly::IOBuf>>,
      _tryDecrypt,
      (std::unique_ptr<folly::IOBuf> & ciphertext,
       const folly::IOBuf* associatedData,
       uint64_t seqNum,
       Aead::AeadOptions options),
      (const));
  Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& /* err */,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      uint64_t seqNum,
      Aead::AeadOptions options) const override {
    ret = _tryDecrypt(ciphertext, associatedData, seqNum, options);
    return Status::Success;
  }

  MOCK_METHOD(
      folly::Optional<std::unique_ptr<folly::IOBuf>>,
      _tryDecryptNonce,
      (std::unique_ptr<folly::IOBuf> & ciphertext,
       const folly::IOBuf* associatedData,
       folly::ByteRange nonce,
       Aead::AeadOptions options),
      (const));
  Status tryDecrypt(
      folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
      Error& /* err */,
      std::unique_ptr<folly::IOBuf>&& ciphertext,
      const folly::IOBuf* associatedData,
      folly::ByteRange nonce,
      Aead::AeadOptions options) const override {
    ret = _tryDecryptNonce(ciphertext, associatedData, nonce, options);
    return Status::Success;
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
