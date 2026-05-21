/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include <fizz/crypto/Hasher.h>
#include <fizz/crypto/KeyDerivation.h>

namespace fizz {

class MockHasher : public Hasher {
 public:
  MOCK_METHOD(void, _hash_update, (folly::ByteRange));
  MOCK_METHOD(void, _hash_final, (folly::MutableByteRange));
  MOCK_METHOD(std::unique_ptr<Hasher>, _clone, (), (const));
  MOCK_METHOD(size_t, getHashLen, (), (const));
  MOCK_METHOD(size_t, getBlockSize, (), (const));

  Status hash_update(Error& err, folly::ByteRange data) override {
    FIZZ_THROW_TO_ERROR(_hash_update(data));
  }
  Status hash_final(Error& err, folly::MutableByteRange out) override {
    FIZZ_THROW_TO_ERROR(_hash_final(out));
  }
  Status clone(std::unique_ptr<Hasher>& ret, Error& err) const override {
    FIZZ_THROW_TO_ERROR(ret, _clone());
  }

  void setDefaults() {
    ON_CALL(*this, getHashLen()).WillByDefault(::testing::Return(32));
    ON_CALL(*this, getBlockSize()).WillByDefault(::testing::Return(64));
    ON_CALL(*this, _clone()).WillByDefault(::testing::InvokeWithoutArgs([] {
      auto h = std::make_unique<::testing::NiceMock<MockHasher>>();
      h->setDefaults();
      return h;
    }));
  }
};

class MockKeyDerivation : public KeyDerivation {
 public:
  MOCK_METHOD(size_t, hashLength, (), (const));
  MOCK_METHOD(folly::ByteRange, blankHash, (), (const));
  MOCK_METHOD(
      Buf,
      _expandLabel,
      (folly::ByteRange secret,
       folly::StringPiece label,
       Buf& hashValue,
       uint16_t length));
  MOCK_METHOD(
      Buf,
      _hkdfExpand,
      (folly::ByteRange secret, Buf& info, uint16_t length));
  Status expandLabel(
      Buf& ret,
      Error& err,
      folly::ByteRange secret,
      folly::StringPiece label,
      Buf hashValue,
      uint16_t length) override {
    FIZZ_THROW_TO_ERROR(ret, _expandLabel(secret, label, hashValue, length));
  }
  Status hkdfExpand(
      Buf& ret,
      Error& err,
      folly::ByteRange secret,
      Buf info,
      uint16_t length) override {
    FIZZ_THROW_TO_ERROR(ret, _hkdfExpand(secret, info, length));
  }
  MOCK_METHOD(
      std::vector<uint8_t>,
      _deriveSecret,
      (folly::ByteRange secret,
       folly::StringPiece label,
       folly::ByteRange messageHash,
       uint16_t length));
  Status deriveSecret(
      std::vector<uint8_t>& ret,
      Error& err,
      folly::ByteRange secret,
      folly::StringPiece label,
      folly::ByteRange messageHash,
      uint16_t length) override {
    FIZZ_THROW_TO_ERROR(ret, _deriveSecret(secret, label, messageHash, length));
  }
  MOCK_METHOD(
      std::vector<uint8_t>,
      _hkdfExtract,
      (folly::ByteRange salt, folly::ByteRange ikm));
  Status hkdfExtract(
      std::vector<uint8_t>& ret,
      Error& err,
      folly::ByteRange salt,
      folly::ByteRange ikm) override {
    FIZZ_THROW_TO_ERROR(ret, _hkdfExtract(salt, ikm));
  }
  MOCK_METHOD(
      void,
      hash,
      (const folly::IOBuf& in, folly::MutableByteRange out));
  MOCK_METHOD(
      void,
      hmac,
      (folly::ByteRange key,
       const folly::IOBuf& in,
       folly::MutableByteRange out));
  MOCK_METHOD(std::unique_ptr<KeyDerivation>, clone, (), (const));
};

} // namespace fizz
