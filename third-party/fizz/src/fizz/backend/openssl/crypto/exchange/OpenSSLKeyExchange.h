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
namespace openssl {

namespace detail {
class OpenSSLECKeyDecoder {
 public:
  static folly::ssl::EvpPkeyUniquePtr decode(
      folly::ByteRange range,
      const int curveNid);
};

class OpenSSLECKeyEncoder {
 public:
  static std::unique_ptr<folly::IOBuf> encode(
      const folly::ssl::EvpPkeyUniquePtr& key);
};
} // namespace detail

/**
 * Eliptic curve key exchange implementation using OpenSSL.
 *
 * The template struct requires the following parameters:
 *   - curveNid: OpenSSL NID for the named curve
 */
class OpenSSLECKeyExchange : public KeyExchange {
 public:
  OpenSSLECKeyExchange(int nid, int keyShareLength)
      : nid_(nid), keyShareLength_(keyShareLength) {}

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
  int nid_;
  int keyShareLength_;
};
} // namespace openssl
} // namespace fizz
