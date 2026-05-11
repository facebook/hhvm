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

Status OQSKeyExchange::createOQSKeyExchange(
    std::unique_ptr<OQSKeyExchange>& ret,
    Error& err,
    KeyExchangeRole role,
    const std::string& algorithm) {
  if (role == KeyExchangeRole::Server) {
    std::unique_ptr<OQSServerKeyExchange> server;
    FIZZ_RETURN_ON_ERROR(OQSServerKeyExchange::create(server, err, algorithm));
    ret = std::move(server);
  } else {
    std::unique_ptr<OQSClientKeyExchange> client;
    FIZZ_RETURN_ON_ERROR(OQSClientKeyExchange::create(client, err, algorithm));
    ret = std::move(client);
  }
  return Status::Success;
}

Status OQSClientKeyExchange::create(
    std::unique_ptr<OQSClientKeyExchange>& ret,
    Error& err,
    const std::string& algorithm) {
  std::unique_ptr<OQS_KEM, kemDeleter_> kem(OQS_KEM_new(algorithm.c_str()));
  if (kem == nullptr) {
    return err.error("OQSKeyExchange(): kem is null!");
  }
  auto publicKey = std::unique_ptr<folly::IOBuf, keyDeleter_>(
      new folly::IOBuf(folly::IOBuf::CREATE, kem->length_public_key));
  auto secretKey = std::unique_ptr<folly::IOBuf, keyDeleter_>(
      new folly::IOBuf(folly::IOBuf::CREATE, kem->length_secret_key));
  ret.reset(new OQSClientKeyExchange(
      std::move(kem), std::move(publicKey), std::move(secretKey)));
  return Status::Success;
}

Status OQSServerKeyExchange::create(
    std::unique_ptr<OQSServerKeyExchange>& ret,
    Error& err,
    const std::string& algorithm) {
  std::unique_ptr<OQS_KEM, kemDeleter_> kem(OQS_KEM_new(algorithm.c_str()));
  if (kem == nullptr) {
    return err.error("OQSKeyExchange(): kem is null!");
  }
  auto cipherText = std::unique_ptr<folly::IOBuf, keyDeleter_>(
      new folly::IOBuf(folly::IOBuf::CREATE, kem->length_ciphertext));
  ret.reset(new OQSServerKeyExchange(std::move(kem), std::move(cipherText)));
  return Status::Success;
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
  FIZZ_RETURN_ON_ERROR(checkChained(err));
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

Status OQSClientKeyExchange::getKeyShare(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err) const {
  if (!isInitiated()) {
    return err.error(
        "OQSClientKeyExchange::getKeyShare(): keypair not generated!");
  }
  FIZZ_RETURN_ON_ERROR(checkChained(err));
  ret = folly::IOBuf::copyBuffer(publicKey_->data(), publicKey_->length());
  return Status::Success;
}

Status OQSServerKeyExchange::getKeyShare(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err) const {
  if (!isInitiated()) {
    return err.error(
        "OQSServerKeyExchange::getKeyShare(): cipher text not generated!");
  }
  FIZZ_RETURN_ON_ERROR(checkChained(err));
  ret = folly::IOBuf::copyBuffer(cipherText_->data(), cipherText_->length());
  return Status::Success;
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
  FIZZ_RETURN_ON_ERROR(checkChained(err));
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
  FIZZ_RETURN_ON_ERROR(checkChained(err));
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
  FIZZ_RETURN_ON_ERROR(checkChained(err));
  std::unique_ptr<OQSClientKeyExchange> copy;
  FIZZ_RETURN_ON_ERROR(
      OQSClientKeyExchange::create(copy, err, kem_->method_name));
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
  FIZZ_RETURN_ON_ERROR(checkChained(err));
  std::unique_ptr<OQSServerKeyExchange> copy;
  FIZZ_RETURN_ON_ERROR(
      OQSServerKeyExchange::create(copy, err, kem_->method_name));
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

inline Status OQSClientKeyExchange::checkChained(Error& err) const {
  if (publicKey_->isChained() || secretKey_->isChained()) {
    return err.error(
        "OQSClientKeyExchange: public key or secret key is chained!");
  }
  return Status::Success;
}

inline Status OQSServerKeyExchange::checkChained(Error& err) const {
  if (cipherText_->isChained()) {
    return err.error("OQSServerKeyExchange: cipher text is chained!");
  }
  return Status::Success;
}
} // namespace fizz::liboqs

#endif
