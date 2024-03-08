/*
 *  Copyright (c) 2023-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/Aead.h>
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

/**
 * An object satisfies AeadImpl if the following requirements hold:
 *     * void AeadImpl::init(folly::ByteRange iv, const folly::IOBuf*
 * associatedData, size_t plaintextLength)
 *       - initializes an encryption context with `iv` and
 * associated data. Associated data can be null.
 *
 *     * void AeadImpl::encrypt(folly::IOBuf& ciphertext, folly::IOBuf&
 * plaintext)
 *       - encrypts `plaintextLength` bytes of `plaintext`. The implementation
 * must write the ciphertext to `ciphertext`. `cipherText` is guaranteed to be
 * writable for `plaintextLength` bytes.
 *
 *     * void AeadImpl::final(int tagLen, void* tagOut)
 *       - finalizes the encryption, and writes the resulting AEAD tag into
 * `tagOut` which is guaranteed to be at least `taglen` bytes.
 */
template <class AeadImpl>
std::unique_ptr<folly::IOBuf> encryptHelper(
    AeadImpl&& impl,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    size_t tagLen,
    size_t headroom,
    Aead::AeadOptions options) {
  auto inputLength = plaintext->computeChainDataLength();
  impl.init(iv, associatedData, inputLength);

  const auto& bufOption = options.bufferOpt;
  const auto& allocOption = options.allocOpt;
  // Setup input and output buffers.
  std::unique_ptr<folly::IOBuf> output;
  folly::IOBuf* input;

  // Whether to allow modifying buffer contents directly
  bool allowInplaceEdit = !plaintext->isShared() ||
      bufOption != Aead::BufferOption::RespectSharedPolicy;

  // Whether to allow growing tailRoom
  bool allowGrowth =
      (bufOption ==
           Aead::BufferOption::AllowFullModification || // When explicitly
                                                        // requested
       !plaintext->isShared() || // When plaintext is unique
       !allowInplaceEdit); // When not in-place (new buffer)

  // Whether to allow memory allocations (throws if needs more memory)
  bool allowAlloc = allocOption == Aead::AllocationOption::Allow;

  if (!allowInplaceEdit && !allowAlloc) {
    throw std::runtime_error(
        "Cannot encrypt (must be in-place or allow allocation)");
  }

  if (allowInplaceEdit) {
    output = std::move(plaintext);
    input = output.get();
  } else {
    // create enough to also fit the tag and headroom
    size_t totalSize{0};
    if (!folly::checked_add<size_t>(
            &totalSize, headroom, inputLength, tagLen)) {
      throw std::overflow_error("Output buffer size");
    }
    output = folly::IOBuf::create(totalSize);
    output->advance(headroom);
    output->append(inputLength);
    input = plaintext.get();
  }

  impl.encrypt(*input, *output);

  // output is always something we can modify
  auto tailRoom = output->prev()->tailroom();
  if (tailRoom < tagLen || !allowGrowth) {
    if (!allowAlloc) {
      throw std::runtime_error("Cannot encrypt (insufficient space for tag)");
    }
    std::unique_ptr<folly::IOBuf> tag = folly::IOBuf::create(tagLen);
    tag->append(tagLen);
    impl.final(tagLen, tag->writableData());
    output->prependChain(std::move(tag));
  } else {
    auto lastBuf = output->prev();
    lastBuf->append(tagLen);
    // we can copy into output directly
    impl.final(tagLen, lastBuf->writableTail() - tagLen);
  }
  return output;
}

/**
 * AeadImpl has the following requirements:
 *     * void AeadImpl::init(folly::ByteRange iv, const folly::IOBuf*
 * associatedData, size_t ciphertextLength)
 *       - initializes a decryption context with `iv` and associated data.
 * Associated data can be null.
 *     * bool AeadImpl::decryptAndFinal(folly::IOBuf& ciphertext, folly::IOBuf&
 * plaintext, folly::MutableByteRange tagOut)
 *       - decrypts `ciphertextLength` bytes of `ciphertext`. The implementation
 * must write the plaintext to `plaintext`. `plaintext` is guaranteed to be
 * writable for `ciphertextLength` bytes. Return whether the decryption was
 * successful.
 */
template <class AeadImpl>
folly::Optional<std::unique_ptr<folly::IOBuf>> decryptHelper(
    AeadImpl&& impl,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::MutableByteRange tagOut,
    bool inPlace) {
  auto inputLength = ciphertext->computeChainDataLength();
  impl.init(iv, associatedData, inputLength);

  folly::IOBuf* input;
  std::unique_ptr<folly::IOBuf> output;
  // If not in-place, allocate buffers. Otherwise in and out are same.
  if (!inPlace) {
    output = folly::IOBuf::create(inputLength);
    output->append(inputLength);
    input = ciphertext.get();
  } else {
    output = std::move(ciphertext);
    input = output.get();
  }

  bool decrypted = impl.decryptAndFinal(*input, *output, tagOut);

  if (!decrypted) {
    return folly::none;
  }
  return output;
}

} // namespace fizz
