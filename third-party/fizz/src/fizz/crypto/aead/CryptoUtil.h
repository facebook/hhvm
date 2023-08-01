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

/**
 * EVPDecImp has the following requirements:
 *     * bool EVPDecImp::decryptUpdate - returns if each evp is decrypted
 * successfully
 *     * bool EVPDecImp::setExpectedTag - returns if the tag is set successfully
 *     * bool EVPDecImp::decryptFinal - returns if decryption is finalized
 * successfully
 */
template <size_t BS, class EVPDecImpl>
bool decFuncBlocks(
    EVPDecImpl&& impl,
    const folly::IOBuf& ciphertext,
    folly::IOBuf& output,
    folly::MutableByteRange tagOut) {
  size_t totalWritten = 0;
  size_t totalInput = 0;
  int outLen = 0;
  auto outputCursor = transformBufferBlocks<BS>(
      ciphertext,
      output,
      [&](uint8_t* plain, const uint8_t* cipher, size_t len) {
        if (len > std::numeric_limits<int>::max()) {
          throw std::runtime_error("Decryption error: too much cipher text");
        }
        if (!impl.decryptUpdate(plain, cipher, len, &outLen)) {
          throw std::runtime_error("Decryption error");
        }
        totalWritten += outLen;
        totalInput += len;
        return static_cast<size_t>(outLen);
      });

  if (!impl.setExpectedTag(tagOut.size(), tagOut.begin())) {
    throw std::runtime_error("Decryption error");
  }

  // We might end up needing to write more in the final encrypt stage
  auto numBuffered = totalInput - totalWritten;
  auto numLeftInOutput = outputCursor.length();
  if (numBuffered <= numLeftInOutput) {
    return impl.decryptFinal(outputCursor.writableData(), &outLen);
  } else {
    // we need to copy nicely - this should be at most one block
    std::array<uint8_t, BS> block = {};
    auto res = impl.decryptFinal(block.data(), &outLen);
    if (!res) {
      return false;
    }
    outputCursor.push(block.data(), outLen);
    return true;
  }
}

} // namespace fizz
