/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>

#include <fizz/crypto/exchange/KeyExchange.h>

namespace fizz {

/* using override */
using namespace testing;

class MockKeyExchange : public KeyExchange {
 public:
  MOCK_METHOD(void, generateKeyPair, ());
  MOCK_METHOD(std::unique_ptr<folly::IOBuf>, getKeyShare, (), (const));
  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      generateSharedSecret,
      (folly::ByteRange keyShare),
      (const));
  MOCK_METHOD(std::unique_ptr<KeyExchange>, clone, (), (const));
  MOCK_METHOD(std::size_t, getExpectedKeyShareSize, (), (const));
  int keyGenerated = 0;

  void setDefaults() {
    ON_CALL(*this, getKeyShare()).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("keyshare");
    }));
    ON_CALL(*this, generateSharedSecret(_))
        .WillByDefault(InvokeWithoutArgs(
            []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
    // Excluding \n
    ON_CALL(*this, getExpectedKeyShareSize())
        .WillByDefault(Return(sizeof("keyshare") - 1));
  }

  void setForHybridKeyExchange() {
    ON_CALL(*this, generateKeyPair()).WillByDefault(InvokeWithoutArgs([this]() {
      keyGenerated = 1;
    }));
    ON_CALL(*this, getKeyShare()).WillByDefault(InvokeWithoutArgs([this]() {
      if (!keyGenerated) {
        throw std::runtime_error("Key not generated");
      }
      return folly::IOBuf::copyBuffer("keyshare");
    }));
    ON_CALL(*this, generateSharedSecret(_))
        .WillByDefault(InvokeWithoutArgs([this]() {
          if (!keyGenerated) {
            throw std::runtime_error("Key not generated");
          }
          return folly::IOBuf::copyBuffer("sharedsecret");
        }));
    // Excluding \n
    ON_CALL(*this, getExpectedKeyShareSize())
        .WillByDefault(Return(sizeof("keyshare") - 1));
    ON_CALL(*this, clone()).WillByDefault(InvokeWithoutArgs([this]() {
      auto copy = std::make_unique<MockKeyExchange>();
      copy->setDefaults();
      copy->keyGenerated = keyGenerated;
      return copy;
    }));
  }

  void setReturnZeroKeyLength() {
    ON_CALL(*this, getExpectedKeyShareSize()).WillByDefault(Return(0));
  }
};

} // namespace fizz
