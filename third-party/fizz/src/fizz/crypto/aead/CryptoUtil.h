/*
 *  Copyright (c) 2023-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

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

/**
 * EVPEncImp has the following requirements:
 *     * bool EVPDecImp::encryptUpdate - returns if each evp is encrypted
 * successfully
 *     * bool EVPDecImp::encryptFinal - returns if encryption is finalized
 * successfully
 */
template <size_t BS, class EVPEncImpl>
void encFuncBlocks(
    EVPEncImpl&& impl,
    const folly::IOBuf& plaintext,
    folly::IOBuf& output) {
  size_t totalWritten = 0;
  size_t totalInput = 0;
  int outLen = 0;
  auto outputCursor = transformBufferBlocks<BS>(
      plaintext,
      output,
      [&](uint8_t* cipher, const uint8_t* plain, size_t len) {
        if (len > std::numeric_limits<int>::max()) {
          throw std::runtime_error("Encryption error: too much plain text");
        }
        if (len == 0) {
          return static_cast<size_t>(0);
        }
        if (!impl.encryptUpdate(
                cipher, &outLen, plain, static_cast<int>(len)) ||
            outLen < 0) {
          throw std::runtime_error("Encryption error");
        }
        totalWritten += outLen;
        totalInput += len;
        return static_cast<size_t>(outLen);
      });

  // We might end up needing to write more in the final encrypt stage
  auto numBuffered = totalInput - totalWritten;
  auto numLeftInOutput = outputCursor.length();
  if (numBuffered <= numLeftInOutput) {
    if (!impl.encryptFinal(outputCursor.writableData(), &outLen)) {
      throw std::runtime_error("Encryption error");
    }
  } else {
    // we need to copy nicely - this should be at most one block
    std::array<uint8_t, BS> block = {};
    if (!impl.encryptFinal(block.data(), &outLen)) {
      throw std::runtime_error("Encryption error");
    }
    outputCursor.push(block.data(), outLen);
  }
}

} // namespace fizz
