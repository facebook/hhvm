/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/backend/liboqs/OQSKeyExchange.h>

#if FIZZ_HAVE_OQS

namespace fizz::liboqs {
std::unique_ptr<OQSKeyExchange> OQSKeyExchange::createOQSKeyExchange(
    KeyExchangeRole role,
    const std::string& algorithm) {
  if (role == KeyExchangeRole::Server) {
    return std::make_unique<OQSServerKeyExchange>(algorithm);
  } else {
    return std::make_unique<OQSClientKeyExchange>(algorithm);
  }
}

OQSKeyExchange::OQSKeyExchange(const std::string& algorithm) {
  // Generate kem here to avoid dealing with nullptr error as the object will
  // fail to construct if that's the case.
  kem_ = std::unique_ptr<OQS_KEM, kemDeleter_>(OQS_KEM_new(algorithm.c_str()));
  if (kem_ == nullptr) {
    throw std::runtime_error("OQSKeyExchange(): kem is null!");
  }
}

OQSClientKeyExchange::OQSClientKeyExchange(const std::string& algorithm)
    : OQSKeyExchange(algorithm) {
  publicKey_ = std::unique_ptr<folly::IOBuf, keyDeleter_>(
      new folly::IOBuf(folly::IOBuf::CREATE, kem_->length_public_key));
  secretKey_ = std::unique_ptr<folly::IOBuf, keyDeleter_>(
      new folly::IOBuf(folly::IOBuf::CREATE, kem_->length_secret_key));
}

OQSServerKeyExchange::OQSServerKeyExchange(const std::string& algorithm)
    : OQSKeyExchange(algorithm) {
  cipherText_ = std::unique_ptr<folly::IOBuf, keyDeleter_>(
      new folly::IOBuf(folly::IOBuf::CREATE, kem_->length_ciphertext));
}

inline bool OQSClientKeyExchange::isInitiated() const {
  return !secretKey_->empty() && !publicKey_->empty();
}

inline bool OQSServerKeyExchange::isInitiated() const {
  return !cipherText_->empty();
}

Status OQSClientKeyExchange::generateKeyPair(Error& err) {
  // We allow regeneration of key pairs as in clone() we deep-copied the buffer
  // instead of just sharing the buf.
  checkChained();
  if (kem_->keypair(publicKey_->writableData(), secretKey_->writableData())) {
    return err.error(
        "OQSClientKeyExchange::generateKeyPair(): keypair generation error!");
  }
  if (!isInitiated()) {
    publicKey_->append(kem_->length_public_key);
    secretKey_->append(kem_->length_secret_key);
  }
  return Status::Success;
}

std::unique_ptr<folly::IOBuf> OQSClientKeyExchange::getKeyShare() const {
  if (!isInitiated()) {
    throw std::runtime_error(
        "OQSClientKeyExchange::getKeyShare(): keypair not generated!");
  }
  checkChained();
  return folly::IOBuf::copyBuffer(publicKey_->data(), publicKey_->length());
}

std::unique_ptr<folly::IOBuf> OQSServerKeyExchange::getKeyShare() const {
  if (!isInitiated()) {
    throw std::runtime_error(
        "OQSServerKeyExchange::getKeyShare(): cipher text not generated!");
  }
  checkChained();
  return folly::IOBuf::copyBuffer(cipherText_->data(), cipherText_->length());
}

Status OQSClientKeyExchange::generateSharedSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange keyShare) const {
  if (!isInitiated()) {
    return err.error(
        "OQSClientKeyExchange::generateSharedSecret(): keys not generated!");
  }
  if (keyShare.size() != getExpectedKeyShareSize()) {
    return err.error(
        "OQSClientKeyExchange::generateSharedSecret(): Invalid external cipher text!");
  }
  checkChained();
  // Note here we can't use the safe deleter due to constraints on return type.
  auto sharedKeyLength = kem_->length_shared_secret;
  auto sharedSecret = folly::IOBuf::create(sharedKeyLength);
  sharedSecret->append(sharedKeyLength);
  if (kem_->decaps(
          sharedSecret->writableData(), keyShare.data(), secretKey_->data())) {
    return err.error(
        "OQSClientKeyExchange::generateSharedSecret(): cannot generate the shared secret!");
  }
  ret = std::move(sharedSecret);
  return Status::Success;
}

Status OQSServerKeyExchange::generateSharedSecret(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange keyShare) const {
  if (keyShare.size() != getExpectedKeyShareSize()) {
    return err.error(
        "OQSServerKeyExchange::generateSharedSecret(): Invalid external public key!");
  }
  checkChained();
  auto sharedKeyLength = kem_->length_shared_secret;
  auto sharedSecret = folly::IOBuf::create(sharedKeyLength);
  sharedSecret->append(sharedKeyLength);
  // We allow regeneration of the cipher text.
  if (kem_->encaps(
          cipherText_->writableData(),
          sharedSecret->writableData(),
          keyShare.data())) {
    return err.error(
        "OQSServerKeyExchange::generateSharedSecret(): cannot generate the shared secret!");
  } else {
    if (!isInitiated()) {
      cipherText_->append(kem_->length_ciphertext);
    }
  }
  ret = std::move(sharedSecret);
  return Status::Success;
}

Status OQSClientKeyExchange::clone(
    std::unique_ptr<KeyExchange>& ret,
    Error& err) const {
  if (!isInitiated()) {
    return err.error("OQSClientKeyExchange::clone(): keys not generated!");
  }
  checkChained();
  auto copy = std::make_unique<OQSClientKeyExchange>(kem_->method_name);
  // Deep copy keys. We assume the key length never changed.
  publicKey_->cloneInto(*(copy->publicKey_));
  copy->publicKey_->unshare();
  secretKey_->cloneInto(*(copy->secretKey_));
  copy->secretKey_->unshare();

  ret = std::move(copy);
  return Status::Success;
}

Status OQSServerKeyExchange::clone(
    std::unique_ptr<KeyExchange>& ret,
    Error& err) const {
  // We have to break the requirements that the keys are generated as the server
  // side is not required to generate the key. We check cipher text instead.
  if (!isInitiated()) {
    return err.error("OQSServerKeyExchange::clone(): cipher not generated!");
  }
  checkChained();
  auto copy = std::make_unique<OQSServerKeyExchange>(kem_->method_name);
  // Deep copy keys. We assume the key length never changed.
  cipherText_->cloneInto(*(copy->cipherText_));
  copy->cipherText_->unshare();

  ret = std::move(copy);
  return Status::Success;
}

inline std::size_t OQSClientKeyExchange::getExpectedKeyShareSize() const {
  return kem_->length_ciphertext;
}

inline std::size_t OQSServerKeyExchange::getExpectedKeyShareSize() const {
  return kem_->length_public_key;
}

inline void OQSClientKeyExchange::checkChained() const {
  if (publicKey_->isChained() || secretKey_->isChained()) {
    throw std::runtime_error(
        "OQSClientKeyExchange: public key or secret key is chained!");
  }
}

inline void OQSServerKeyExchange::checkChained() const {
  if (cipherText_->isChained()) {
    throw std::runtime_error("OQSServerKeyExchange: cipher text is chained!");
  }
}
} // namespace fizz::liboqs

#endif
