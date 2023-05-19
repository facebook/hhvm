// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <fizz/crypto/aead/IOBufUtil.h>
#include <folly/Conv.h>
#include <folly/Memory.h>
#include <folly/Range.h>

namespace fizz {

template <size_t kMaxIVLength>
std::array<uint8_t, kMaxIVLength>
createIV(uint64_t seqNum, size_t ivLength, folly::ByteRange trafficIvKey) {
  std::array<uint8_t, kMaxIVLength> iv;
  uint64_t bigEndianSeqNum = folly::Endian::big(seqNum);
  const size_t prefixLength = ivLength - sizeof(uint64_t);
  memset(iv.data(), 0, prefixLength);
  memcpy(iv.data() + prefixLength, &bigEndianSeqNum, sizeof(uint64_t));
  folly::MutableByteRange mutableIv{iv.data(), ivLength};
  XOR(trafficIvKey, mutableIv);
  return iv;
}

} // namespace fizz
