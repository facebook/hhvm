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

Status OpenSSLECKeyExchange::generateKeyPair(Error& /*err*/) {
  key_ = detail::generateECKeyPair(nid_);
  return Status::Success;
}

Status OpenSSLECKeyExchange::getKeyShare(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err) const {
  if (!key_) {
    return err.error("Key not initialized");
  }
  ret = detail::OpenSSLECKeyEncoder::encode(key_);
  return Status::Success;
}

Status OpenSSLECKeyExchange::generateSharedSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange keyShare) const {
  auto peerKey = detail::OpenSSLECKeyDecoder::decode(keyShare, nid_);
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

void OpenSSLECKeyExchange::setPrivateKey(
    folly::ssl::EvpPkeyUniquePtr privateKey) {
  Error err;
  FIZZ_THROW_ON_ERROR(detail::validateECKey(err, privateKey, nid_), err);
  key_ = std::move(privateKey);
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
  copyKex->setPrivateKey(std::move(keyCopy));

  ret = std::move(copyKex);
  return Status::Success;
}

std::size_t OpenSSLECKeyExchange::getExpectedKeyShareSize() const {
  return keyShareLength_;
}

} // namespace openssl
} // namespace fizz
