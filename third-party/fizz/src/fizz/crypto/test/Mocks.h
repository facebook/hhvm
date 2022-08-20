/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include <fizz/crypto/KeyDerivation.h>

namespace fizz {

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
  Buf expandLabel(
      folly::ByteRange secret,
      folly::StringPiece label,
      Buf hashValue,
      uint16_t length) override {
    return _expandLabel(secret, label, hashValue, length);
  }
  Buf hkdfExpand(folly::ByteRange secret, Buf info, uint16_t length) override {
    return _hkdfExpand(secret, info, length);
  }
  MOCK_METHOD(
      std::vector<uint8_t>,
      deriveSecret,
      (folly::ByteRange secret,
       folly::StringPiece label,
       folly::ByteRange messageHash,
       uint16_t length));
  MOCK_METHOD(
      std::vector<uint8_t>,
      hkdfExtract,
      (folly::ByteRange salt, folly::ByteRange ikm));
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
