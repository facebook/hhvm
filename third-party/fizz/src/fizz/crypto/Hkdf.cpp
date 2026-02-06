/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/Hmac.h>
#include <fizz/util/Logging.h>

namespace fizz {

std::vector<uint8_t> Hkdf::extract(folly::ByteRange salt, folly::ByteRange ikm)
    const {
  auto hlen = hashLength();
  auto zeros = std::vector<uint8_t>(hlen, 0);
  // Extraction step HMAC-HASH(salt, IKM)
  std::vector<uint8_t> extractedKey(hlen);
  salt = salt.empty() ? folly::range(zeros) : salt;
  hmac(
      makeHasher_,
      salt,
      folly::IOBuf::wrapBufferAsValue(ikm),
      folly::range(extractedKey));
  return extractedKey;
}

std::unique_ptr<folly::IOBuf> Hkdf::expand(
    folly::ByteRange extractedKey,
    const folly::IOBuf& info,
    size_t outputBytes) const {
  auto hlen = hashLength();
  FIZZ_CHECK_EQ(extractedKey.size(), hlen);
  if (UNLIKELY(outputBytes > 255 * hlen)) {
    throw std::runtime_error("Output too long");
  }
  // HDKF expansion step.
  size_t numRounds = (outputBytes + hlen - 1) / hlen;
  auto expanded = folly::IOBuf::create(numRounds * hlen);

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

    size_t outputStartIdx = (round - 1) * hlen;
    hmac(
        makeHasher_,
        folly::range(extractedKey),
        *in,
        {expanded->writableData() + outputStartIdx, hlen});
    expanded->append(hlen);

    in = expanded->clone();
    in->trimStart(outputStartIdx);
  }
  expanded->trimEnd(numRounds * hlen - outputBytes);
  return expanded;
}

std::unique_ptr<folly::IOBuf> Hkdf::hkdf(
    folly::ByteRange ikm,
    folly::ByteRange salt,
    const folly::IOBuf& info,
    size_t outputBytes) const {
  return expand(folly::range(extract(salt, ikm)), info, outputBytes);
}
} // namespace fizz
