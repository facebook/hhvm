/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/openssl/OpenSSLKeyUtils.h>

namespace fizz {
namespace detail {

template <class T>
class OpenSSLECKeyDecoder {
 public:
  static folly::ssl::EvpPkeyUniquePtr decode(folly::ByteRange range) {
    return decodeECPublicKey(range, T::curveNid);
  }
};

class OpenSSLECKeyEncoder {
 public:
  static std::unique_ptr<folly::IOBuf> encode(
      const folly::ssl::EvpPkeyUniquePtr& key) {
    return encodeECPublicKey(key);
  }
};
} // namespace detail

template <class T>
void OpenSSLECKeyExchange<T>::generateKeyPair() {
  key_ = detail::generateECKeyPair(T::curveNid);
}

template <class T>
std::unique_ptr<folly::IOBuf> OpenSSLECKeyExchange<T>::getKeyShare() const {
  if (!key_) {
    throw std::runtime_error("Key not initialized");
  }
  return detail::OpenSSLECKeyEncoder::encode(key_);
}

template <class T>
std::unique_ptr<folly::IOBuf> OpenSSLECKeyExchange<T>::generateSharedSecret(
    folly::ByteRange keyShare) const {
  auto peerKey = detail::OpenSSLECKeyDecoder<T>::decode(keyShare);
  return generateSharedSecret(peerKey);
}

template <class T>
std::unique_ptr<folly::IOBuf> OpenSSLECKeyExchange<T>::generateSharedSecret(
    const folly::ssl::EvpPkeyUniquePtr& peerKey) const {
  if (!key_) {
    throw std::runtime_error("Key not generated");
  }
  return detail::generateEvpSharedSecret(key_, peerKey);
}

template <class T>
void OpenSSLECKeyExchange<T>::setPrivateKey(
    folly::ssl::EvpPkeyUniquePtr privateKey) {
  detail::validateECKey(privateKey, T::curveNid);
  key_ = std::move(privateKey);
}

template <class T>
const folly::ssl::EvpPkeyUniquePtr& OpenSSLECKeyExchange<T>::getPrivateKey()
    const {
  return key_;
}

template <class T>
std::unique_ptr<KeyExchange> OpenSSLECKeyExchange<T>::clone() const {
  if (!key_) {
    throw std::runtime_error("Key not initialized");
  }

  // Increment the reference for the current key
  EVP_PKEY_up_ref(key_.get());
  // Create key copy
  folly::ssl::EvpPkeyUniquePtr keyCopy(key_.get());

  // Construct a new copy key exchange to return
  auto copyKex = std::make_unique<OpenSSLECKeyExchange<T>>();
  copyKex->setPrivateKey(std::move(keyCopy));

  return copyKex;
}

template <class T>
std::size_t OpenSSLECKeyExchange<T>::getExpectedKeyShareSize() const {
  return T::keyShareLength;
}

} // namespace fizz
