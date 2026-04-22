/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/crypto/exchange/HybridKeyExchange.h>

namespace fizz {

HybridKeyExchange::HybridKeyExchange(
    std::unique_ptr<KeyExchange> first,
    std::unique_ptr<KeyExchange> second)
    : firstKex_(std::move(first)), secondKex_(std::move(second)) {}

Status HybridKeyExchange::create(
    std::unique_ptr<HybridKeyExchange>& ret,
    Error& err,
    std::unique_ptr<KeyExchange> first,
    std::unique_ptr<KeyExchange> second) {
  if (first == nullptr || second == nullptr) {
    return err.error("Passing null KeyExchange Object!");
  }
  ret = std::unique_ptr<HybridKeyExchange>(
      new HybridKeyExchange(std::move(first), std::move(second)));
  return Status::Success;
}

Status HybridKeyExchange::generateKeyPair(Error& err) {
  FIZZ_RETURN_ON_ERROR(firstKex_->generateKeyPair(err));
  FIZZ_RETURN_ON_ERROR(secondKex_->generateKeyPair(err));
  if (firstKex_->getExpectedKeyShareSize() == 0 ||
      secondKex_->getExpectedKeyShareSize() == 0) {
    return err.error("expected key share size is 0!");
  }
  return Status::Success;
}

/**
 * Simply concatenate the public key. According to RFC, if either of two key
 * exchanges is not fixed-length, we need to inject length fields or other data
 * to ensure the composition of the two values injective. But this is out of
 * this draft RFC's scope. We need to implement this in future to avoid boundary
 * confusion attacks.
 */
std::unique_ptr<folly::IOBuf> HybridKeyExchange::getKeyShare() const {
  // We don't need to copy the IOBuf even though we use first's keyshare to
  // concatenate second's keyshare because we used unique_ptr.
  auto keyShare = firstKex_->getKeyShare();
  auto secondKeyShare = secondKex_->getKeyShare();
  keyShare->appendToChain(std::move(secondKeyShare));
  return keyShare;
}

std::unique_ptr<folly::IOBuf> HybridKeyExchange::generateSharedSecret(
    folly::ByteRange keyShare) const {
  if (keyShare.size() !=
      firstKex_->getExpectedKeyShareSize() +
          secondKex_->getExpectedKeyShareSize()) {
    throw std::runtime_error("Invalid external public key combination ");
  }
  // keyShare.size() should be >= 2 now
  auto firstKeyShare = folly::ByteRange(
      keyShare.begin(),
      keyShare.begin() + firstKex_->getExpectedKeyShareSize());
  auto sharedSecret = firstKex_->generateSharedSecret(std::move(firstKeyShare));
  auto secondKeyShare = folly::ByteRange(
      keyShare.end() - secondKex_->getExpectedKeyShareSize(), keyShare.end());
  sharedSecret->appendToChain(
      secondKex_->generateSharedSecret(std::move(secondKeyShare)));
  return sharedSecret;
}

/**
 * Deep copy of first and second.
 */
std::unique_ptr<KeyExchange> HybridKeyExchange::clone() const {
  std::unique_ptr<HybridKeyExchange> kexCopy;
  Error err;
  FIZZ_THROW_ON_ERROR(
      HybridKeyExchange::create(
          kexCopy, err, firstKex_->clone(), secondKex_->clone()),
      err);
  return kexCopy;
}

std::size_t HybridKeyExchange::getExpectedKeyShareSize() const {
  return firstKex_->getExpectedKeyShareSize() +
      secondKex_->getExpectedKeyShareSize();
}
} // namespace fizz
