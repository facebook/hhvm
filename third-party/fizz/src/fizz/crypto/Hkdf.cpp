/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hkdf.h>

namespace fizz {

std::vector<uint8_t> HkdfImpl::extract(
    folly::ByteRange salt,
    folly::ByteRange ikm) const {
  auto zeros = std::vector<uint8_t>(hashLength_, 0);
  // Extraction step HMAC-HASH(salt, IKM)
  std::vector<uint8_t> extractedKey(hashLength_);
  salt = salt.empty() ? folly::range(zeros) : salt;
  hmacFunc_(
      salt, folly::IOBuf::wrapBufferAsValue(ikm), folly::range(extractedKey));
  return extractedKey;
}

std::unique_ptr<folly::IOBuf> HkdfImpl::expand(
    folly::ByteRange extractedKey,
    const folly::IOBuf& info,
    size_t outputBytes) const {
  CHECK_EQ(extractedKey.size(), hashLength_);
  if (UNLIKELY(outputBytes > 255 * hashLength_)) {
    throw std::runtime_error("Output too long");
  }
  // HDKF expansion step.
  size_t numRounds = (outputBytes + hashLength_ - 1) / hashLength_;
  auto expanded = folly::IOBuf::create(numRounds * hashLength_);

  auto in = folly::IOBuf::create(0);
  for (size_t round = 1; round <= numRounds; ++round) {
    in->prependChain(info.clone());
    // We're guaranteed that the round num will fit in
    // one byte because of the check at the beginning of
    // the method.
    auto roundNum = folly::IOBuf::create(1);
    roundNum->append(1);
    roundNum->writableData()[0] = round;
    in->prependChain(std::move(roundNum));

    size_t outputStartIdx = (round - 1) * hashLength_;
    hmacFunc_(
        folly::range(extractedKey),
        *in,
        {expanded->writableData() + outputStartIdx, hashLength_});
    expanded->append(hashLength_);

    in = expanded->clone();
    in->trimStart(outputStartIdx);
  }
  expanded->trimEnd(numRounds * hashLength_ - outputBytes);
  return expanded;
}

std::unique_ptr<folly::IOBuf> HkdfImpl::hkdf(
    folly::ByteRange ikm,
    folly::ByteRange salt,
    const folly::IOBuf& info,
    size_t outputBytes) const {
  return expand(folly::range(extract(salt, ikm)), info, outputBytes);
}
} // namespace fizz
