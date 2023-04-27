/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>
#include <folly/ssl/OpenSSLPtrTypes.h>

namespace fizz {

/**
 * Eliptic curve key exchange implementation using OpenSSL.
 *
 * The template struct requires the following parameters:
 *   - curveNid: OpenSSL NID for the named curve
 */
template <class T>
class OpenSSLECKeyExchange : public KeyExchange {
 public:
  ~OpenSSLECKeyExchange() override = default;

  void generateKeyPair() override;

  std::unique_ptr<folly::IOBuf> getKeyShare() const override;

  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      folly::ByteRange keyShare) const override;

  std::unique_ptr<KeyExchange> clone() const override;

  std::unique_ptr<folly::IOBuf> generateSharedSecret(
      const folly::ssl::EvpPkeyUniquePtr& peerKey) const;

  void setPrivateKey(folly::ssl::EvpPkeyUniquePtr privateKey);

  const folly::ssl::EvpPkeyUniquePtr& getPrivateKey() const;

  std::size_t getExpectedKeyShareSize() const override;

 private:
  folly::ssl::EvpPkeyUniquePtr key_;
};
} // namespace fizz

#include <fizz/crypto/exchange/OpenSSLKeyExchange-inl.h>
