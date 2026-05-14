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
Status OpenSSLECKeyDecoder::decode(
    folly::ssl::EvpPkeyUniquePtr& ret,
    Error& err,
    folly::ByteRange range,
    const int curveNid) {
  FIZZ_RETURN_ON_ERROR(decodeECPublicKey(ret, err, range, curveNid));
  return Status::Success;
}

Status OpenSSLECKeyEncoder::encode(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& key) {
  FIZZ_RETURN_ON_ERROR(encodeECPublicKey(ret, err, key));
  return Status::Success;
}
} // namespace detail

Status OpenSSLECKeyExchange::generateKeyPair(Error& err) {
  FIZZ_RETURN_ON_ERROR(detail::generateECKeyPair(key_, err, nid_));
  return Status::Success;
}

Status OpenSSLECKeyExchange::getKeyShare(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err) const {
  if (!key_) {
    return err.error("Key not initialized");
  }
  FIZZ_RETURN_ON_ERROR(detail::OpenSSLECKeyEncoder::encode(ret, err, key_));
  return Status::Success;
}

Status OpenSSLECKeyExchange::generateSharedSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange keyShare) const {
  folly::ssl::EvpPkeyUniquePtr peerKey;
  FIZZ_RETURN_ON_ERROR(
      detail::OpenSSLECKeyDecoder::decode(peerKey, err, keyShare, nid_));
  FIZZ_RETURN_ON_ERROR(generateSharedSecret(ret, err, peerKey));
  return Status::Success;
}

Status OpenSSLECKeyExchange::generateSharedSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    const folly::ssl::EvpPkeyUniquePtr& peerKey) const {
  if (!key_) {
    return err.error("Key not generated");
  }
  FIZZ_RETURN_ON_ERROR(
      detail::generateEvpSharedSecret(ret, err, key_, peerKey));
  return Status::Success;
}

Status OpenSSLECKeyExchange::setPrivateKey(
    Error& err,
    folly::ssl::EvpPkeyUniquePtr privateKey) {
  FIZZ_RETURN_ON_ERROR(detail::validateECKey(err, privateKey, nid_));
  key_ = std::move(privateKey);
  return Status::Success;
}

const folly::ssl::EvpPkeyUniquePtr& OpenSSLECKeyExchange::getPrivateKey()
    const {
  return key_;
}

Status OpenSSLECKeyExchange::clone(
    std::unique_ptr<KeyExchange>& ret,
    Error& err) const {
  if (!key_) {
    return err.error("Key not initialized");
  }

  // Increment the reference for the current key
  EVP_PKEY_up_ref(key_.get());
  // Create key copy
  folly::ssl::EvpPkeyUniquePtr keyCopy(key_.get());

  // Construct a new copy key exchange to return
  auto copyKex = std::make_unique<OpenSSLECKeyExchange>(nid_, keyShareLength_);
  FIZZ_RETURN_ON_ERROR(copyKex->setPrivateKey(err, std::move(keyCopy)));

  ret = std::move(copyKex);
  return Status::Success;
}

std::size_t OpenSSLECKeyExchange::getExpectedKeyShareSize() const {
  return keyShareLength_;
}

} // namespace openssl
} // namespace fizz
