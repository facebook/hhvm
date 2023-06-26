/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/exchange/X25519.h>

#include <fizz/crypto/Utils.h>

#include <folly/Conv.h>
#include <sodium.h>

using namespace folly;

namespace fizz {

void X25519KeyExchange::setKeyPair(
    std::unique_ptr<folly::IOBuf> gotPrivKey,
    std::unique_ptr<folly::IOBuf> gotPubKey) {
  std::array<uint8_t, kCurve25519PrivBytes> privKey;
  auto privKeyRange = gotPrivKey->coalesce();
  std::copy(
      privKeyRange.begin(),
      privKeyRange.begin() + kCurve25519PrivBytes,
      privKey.begin());

  std::array<uint8_t, kCurve25519PubBytes> pubKey;
  auto pubKeyRange = gotPubKey->coalesce();
  std::copy(
      pubKeyRange.begin(),
      pubKeyRange.begin() + kCurve25519PubBytes,
      pubKey.begin());

  privKey_ = privKey;
  pubKey_ = pubKey;
}

void X25519KeyExchange::setPrivateKey(
    std::unique_ptr<folly::IOBuf> gotPrivKey) {
  if (!gotPrivKey ||
      gotPrivKey->computeChainDataLength() != kCurve25519PrivBytes) {
    throw std::runtime_error("Invalid Private Key.");
  }

  std::array<uint8_t, kCurve25519PrivBytes> privKey;
  auto privKeyRange = gotPrivKey->coalesce();
  std::copy(privKeyRange.begin(), privKeyRange.end(), privKey.begin());

  std::array<uint8_t, kCurve25519PubBytes> pubKey;
  crypto_scalarmult_curve25519_base(pubKey.data(), privKeyRange.data());

  privKey_ = privKey;
  pubKey_ = pubKey;
}

void X25519KeyExchange::generateKeyPair() {
  auto privKey = PrivKey();
  auto pubKey = PubKey();
  static_assert(
      X25519KeyExchange::PrivKey().size() == crypto_scalarmult_SCALARBYTES,
      "Incorrect size of the private key");
  static_assert(
      X25519KeyExchange::PubKey().size() == crypto_scalarmult_BYTES,
      "Incorrect size of the public key");
  auto err = crypto_box_curve25519xsalsa20poly1305_keypair(
      pubKey.data(), privKey.data());
  if (err != 0) {
    throw std::runtime_error(to<std::string>("Could not generate keys ", err));
  }
  privKey_ = std::move(privKey);
  pubKey_ = std::move(pubKey);
}

std::unique_ptr<IOBuf> X25519KeyExchange::getKeyShare() const {
  if (!privKey_ || !pubKey_) {
    throw std::runtime_error("Key not generated");
  }
  return IOBuf::copyBuffer(pubKey_->data(), pubKey_->size());
}

std::unique_ptr<folly::IOBuf> X25519KeyExchange::generateSharedSecret(
    folly::ByteRange keyShare) const {
  if (!privKey_ || !pubKey_) {
    throw std::runtime_error("Key not generated");
  }
  if (keyShare.size() != crypto_scalarmult_BYTES) {
    throw std::runtime_error("Invalid external public key");
  }
  auto key = IOBuf::create(crypto_scalarmult_BYTES);
  key->append(crypto_scalarmult_BYTES);
  int err =
      crypto_scalarmult(key->writableData(), privKey_->data(), keyShare.data());
  if (err != 0) {
    throw std::runtime_error("Invalid point");
  }
  return key;
}

std::unique_ptr<KeyExchange> X25519KeyExchange::clone() const {
  if (!privKey_ || !pubKey_) {
    throw std::runtime_error("Key not generated");
  }

  auto kexCopy = std::make_unique<X25519KeyExchange>();
  kexCopy->privKey_ = privKey_;
  kexCopy->pubKey_ = pubKey_;
  return kexCopy;
}

std::size_t X25519KeyExchange::getExpectedKeyShareSize() const {
  return kCurve25519PubBytes;
}

} // namespace fizz
