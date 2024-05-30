/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/openssl/crypto/OpenSSLKeyUtils.h>
#include <fizz/backend/openssl/crypto/exchange/OpenSSLKeyExchange.h>

namespace fizz {
namespace openssl {

namespace detail {
folly::ssl::EvpPkeyUniquePtr OpenSSLECKeyDecoder::decode(
    folly::ByteRange range,
    const int curveNid) {
  return decodeECPublicKey(range, curveNid);
}

std::unique_ptr<folly::IOBuf> OpenSSLECKeyEncoder::encode(
    const folly::ssl::EvpPkeyUniquePtr& key) {
  return encodeECPublicKey(key);
}
} // namespace detail

void OpenSSLECKeyExchange::generateKeyPair() {
  key_ = detail::generateECKeyPair(nid_);
}

std::unique_ptr<folly::IOBuf> OpenSSLECKeyExchange::getKeyShare() const {
  if (!key_) {
    throw std::runtime_error("Key not initialized");
  }
  return detail::OpenSSLECKeyEncoder::encode(key_);
}

std::unique_ptr<folly::IOBuf> OpenSSLECKeyExchange::generateSharedSecret(
    folly::ByteRange keyShare) const {
  auto peerKey = detail::OpenSSLECKeyDecoder::decode(keyShare, nid_);
  return generateSharedSecret(peerKey);
}

std::unique_ptr<folly::IOBuf> OpenSSLECKeyExchange::generateSharedSecret(
    const folly::ssl::EvpPkeyUniquePtr& peerKey) const {
  if (!key_) {
    throw std::runtime_error("Key not generated");
  }
  return detail::generateEvpSharedSecret(key_, peerKey);
}

void OpenSSLECKeyExchange::setPrivateKey(
    folly::ssl::EvpPkeyUniquePtr privateKey) {
  detail::validateECKey(privateKey, nid_);
  key_ = std::move(privateKey);
}

const folly::ssl::EvpPkeyUniquePtr& OpenSSLECKeyExchange::getPrivateKey()
    const {
  return key_;
}

std::unique_ptr<KeyExchange> OpenSSLECKeyExchange::clone() const {
  if (!key_) {
    throw std::runtime_error("Key not initialized");
  }

  // Increment the reference for the current key
  EVP_PKEY_up_ref(key_.get());
  // Create key copy
  folly::ssl::EvpPkeyUniquePtr keyCopy(key_.get());

  // Construct a new copy key exchange to return
  auto copyKex = std::make_unique<OpenSSLECKeyExchange>(nid_, keyShareLength_);
  copyKex->setPrivateKey(std::move(keyCopy));

  return copyKex;
}

std::size_t OpenSSLECKeyExchange::getExpectedKeyShareSize() const {
  return keyShareLength_;
}

} // namespace openssl
} // namespace fizz
