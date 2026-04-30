/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/AsyncKeyExchange.h>
#include <fizz/crypto/exchange/KeyExchange.h>
#include <folly/portability/GMock.h>

namespace fizz {

/* using override */
using namespace testing;

class MockKeyExchange : public KeyExchange {
 public:
  MOCK_METHOD(void, _generateKeyPair, ());
  Status generateKeyPair(Error& err) override {
    FIZZ_THROW_TO_ERROR(_generateKeyPair());
  }
  MOCK_METHOD(std::unique_ptr<folly::IOBuf>, getKeyShare, (), (const));
  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _generateSharedSecret,
      (folly::ByteRange keyShare),
      (const));
  Status generateSharedSecret(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      folly::ByteRange keyShare) const override {
    FIZZ_THROW_TO_ERROR(ret, _generateSharedSecret(keyShare));
  }
  MOCK_METHOD(std::unique_ptr<KeyExchange>, _clone, (), (const));
  Status clone(std::unique_ptr<KeyExchange>& ret, Error& /*err*/)
      const override {
    ret = _clone();
    return Status::Success;
  }
  MOCK_METHOD(std::size_t, getExpectedKeyShareSize, (), (const));
  int keyGenerated = 0;

  void setDefaults() {
    ON_CALL(*this, getKeyShare()).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("keyshare");
    }));
    ON_CALL(*this, _generateSharedSecret(_))
        .WillByDefault(InvokeWithoutArgs(
            []() { return folly::IOBuf::copyBuffer("sharedsecret"); }));
    // Excluding \n
    ON_CALL(*this, getExpectedKeyShareSize())
        .WillByDefault(Return(sizeof("keyshare") - 1));
  }

  void setForHybridKeyExchange() {
    ON_CALL(*this, _generateKeyPair())
        .WillByDefault(InvokeWithoutArgs([this]() { keyGenerated = 1; }));
    ON_CALL(*this, getKeyShare()).WillByDefault(InvokeWithoutArgs([this]() {
      if (!keyGenerated) {
        throw std::runtime_error("Key not generated");
      }
      return folly::IOBuf::copyBuffer("keyshare");
    }));
    ON_CALL(*this, _generateSharedSecret(_))
        .WillByDefault(InvokeWithoutArgs([this]() {
          if (!keyGenerated) {
            throw std::runtime_error("Key not generated");
          }
          return folly::IOBuf::copyBuffer("sharedsecret");
        }));
    // Excluding \n
    ON_CALL(*this, getExpectedKeyShareSize())
        .WillByDefault(Return(sizeof("keyshare") - 1));
    ON_CALL(*this, _clone()).WillByDefault(InvokeWithoutArgs([this]() {
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

class MockAsyncKeyExchange : public AsyncKeyExchange {
 public:
  MOCK_METHOD(void, _generateKeyPair, ());
  Status generateKeyPair(Error& err) override {
    FIZZ_THROW_TO_ERROR(_generateKeyPair());
  }
  MOCK_METHOD(std::unique_ptr<folly::IOBuf>, getKeyShare, (), (const));
  MOCK_METHOD(
      std::unique_ptr<folly::IOBuf>,
      _generateSharedSecret,
      (folly::ByteRange keyShare),
      (const));
  Status generateSharedSecret(
      std::unique_ptr<folly::IOBuf>& ret,
      Error& err,
      folly::ByteRange keyShare) const override {
    FIZZ_THROW_TO_ERROR(ret, _generateSharedSecret(keyShare));
  }
  MOCK_METHOD(std::unique_ptr<KeyExchange>, _clone, (), (const));
  Status clone(std::unique_ptr<KeyExchange>& ret, Error& /*err*/)
      const override {
    ret = _clone();
    return Status::Success;
  }
  MOCK_METHOD(std::size_t, getExpectedKeyShareSize, (), (const));
  MOCK_METHOD(
      folly::SemiFuture<DoKexResult>,
      doAsyncKexFuture,
      (std::unique_ptr<folly::IOBuf> peerKeyShare));
};
} // namespace fizz
