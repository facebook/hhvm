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

#include <folly/io/Cursor.h>

namespace fizz {

Status Hkdf::extract(
    std::vector<uint8_t>& ret,
    Error& err,
    folly::ByteRange salt,
    folly::ByteRange ikm) const {
  auto hlen = hashLength();
  auto zeros = std::vector<uint8_t>(hlen, 0);
  // Extraction step HMAC-HASH(salt, IKM)
  std::vector<uint8_t> extractedKey(hlen);
  salt = salt.empty() ? folly::range(zeros) : salt;
  FIZZ_RETURN_ON_ERROR(hmac(
      err,
      makeHasher_,
      salt,
      folly::IOBuf::wrapBufferAsValue(ikm),
      folly::range(extractedKey)));
  ret = std::move(extractedKey);
  return Status::Success;
}

Status Hkdf::expand(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange extractedKey,
    const folly::IOBuf& info,
    size_t outputBytes) const {
  auto hlen = hashLength();
  FIZZ_CHECK_EQ(extractedKey.size(), hlen);
  if (UNLIKELY(outputBytes > 255 * hlen)) {
    return err.error("Output too long");
  }
  if (outputBytes == 0) {
    ret = folly::IOBuf::create(0);
    return Status::Success;
  }
  // HDKF expansion step.
  size_t numRounds = (outputBytes + hlen - 1) / hlen;
  auto expanded = folly::IOBuf::create(numRounds * hlen);

  // Every round except the first hashes T(round-1) || info || round, whose
  // size is constant across rounds. Lay out that maximal input once and reuse
  // it: [ T(round-1) : hlen ][ info : infoLength ][ round : 1 ]. The round num
  // is guaranteed to fit in one byte because of the check above.
  size_t infoLength = info.computeChainDataLength();
  std::vector<uint8_t> in(hlen + infoLength + 1);
  folly::io::Cursor(&info).pull(in.data() + hlen, infoLength);
  uint8_t* roundNum = in.data() + hlen + infoLength;

  // First round only hashes info || 1 (T(0) is the empty string), so it starts
  // partway into the buffer, skipping the unused T(round-1) slot.
  *roundNum = 1;
  auto hmacIn =
      folly::IOBuf::wrapBufferAsValue(in.data() + hlen, infoLength + 1);
  FIZZ_RETURN_ON_ERROR(hmac(
      err,
      makeHasher_,
      folly::range(extractedKey),
      hmacIn,
      {expanded->writableData(), hlen}));
  expanded->append(hlen);

  // Remaining rounds hash the full T(round-1) || info || round buffer.
  hmacIn = folly::IOBuf::wrapBufferAsValue(in.data(), in.size());
  for (size_t round = 2; round <= numRounds; ++round) {
    memcpy(in.data(), expanded->data() + (round - 2) * hlen, hlen);
    *roundNum = round;

    size_t outputStartIdx = (round - 1) * hlen;
    FIZZ_RETURN_ON_ERROR(hmac(
        err,
        makeHasher_,
        folly::range(extractedKey),
        hmacIn,
        {expanded->writableData() + outputStartIdx, hlen}));
    expanded->append(hlen);
  }
  expanded->trimEnd(numRounds * hlen - outputBytes);
  ret = std::move(expanded);
  return Status::Success;
}

Status Hkdf::hkdf(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    folly::ByteRange ikm,
    folly::ByteRange salt,
    const folly::IOBuf& info,
    size_t outputBytes) const {
  std::vector<uint8_t> extracted;
  FIZZ_RETURN_ON_ERROR(extract(extracted, err, salt, ikm));
  FIZZ_RETURN_ON_ERROR(
      expand(ret, err, folly::range(extracted), info, outputBytes));
  return Status::Success;
}
} // namespace fizz
