/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>

#include <folly/Optional.h>
#include <folly/Range.h>
#include <folly/io/IOBuf.h>

namespace fizz {

/**
 * X25519 key exchange implementation using libsodium.
 */
class X25519KeyExchange : public KeyExchange {
 public:
  ~X25519KeyExchange() override = default;
  void generateKeyPair() override;
  std::unique_ptr<folly::IOBuf> getKeyShare() const override;
  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override;
  std::unique_ptr<KeyExchange> clone() const override;
  std::size_t getExpectedKeyShareSize() const override;

  // Should only be used for testing.
  void setKeyPair(
      std::unique_ptr<folly::IOBuf> privKey,
      std::unique_ptr<folly::IOBuf> pubKey);

  void setPrivateKey(std::unique_ptr<folly::IOBuf> gotPrivKey);

 private:
  constexpr static size_t kCurve25519PubBytes = 32;
  constexpr static size_t kCurve25519PrivBytes = 32;
  using PrivKey = std::array<uint8_t, kCurve25519PubBytes>;
  using PubKey = std::array<uint8_t, kCurve25519PrivBytes>;

  folly::Optional<PrivKey> privKey_;
  folly::Optional<PubKey> pubKey_;
};
} // namespace fizz
