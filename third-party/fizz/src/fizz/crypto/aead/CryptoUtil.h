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
template <class EVPDecImpl>
Status decFuncBlocks(
    bool& ret,
    Error& err,
    EVPDecImpl&& impl,
    const folly::IOBuf& ciphertext,
    folly::IOBuf& output,
    folly::MutableByteRange tagOut,
    size_t blockSize) {
  size_t totalWritten = 0;
  size_t totalInput = 0;
  int outLen = 0;
  folly::io::RWPrivateCursor outputCursor(&output);
  FIZZ_RETURN_ON_ERROR(transformBufferBlocks(
      outputCursor,
      err,
      ciphertext,
      output,
      [&impl, &outLen, &totalWritten, &totalInput](
          size_t& written,
          Error& innerErr,
          uint8_t* plain,
          const uint8_t* cipher,
          size_t len) -> Status {
        if (len > std::numeric_limits<int>::max()) {
          return innerErr.error("Decryption error: too much cipher text");
        }
        if (!impl.decryptUpdate(plain, cipher, len, &outLen)) {
          return innerErr.error("Decryption error");
        }
        totalWritten += outLen;
        totalInput += len;
        written = static_cast<size_t>(outLen);
        return Status::Success;
      },
      blockSize));

  if (!impl.setExpectedTag(tagOut.size(), tagOut.begin())) {
    return err.error("Decryption error");
  }

  // We might end up needing to write more in the final encrypt stage
  auto numBuffered = totalInput - totalWritten;
  auto numLeftInOutput = outputCursor.length();
  if (numBuffered <= numLeftInOutput) {
    ret = impl.decryptFinal(outputCursor.writableData(), &outLen);
    return Status::Success;
  } else {
    // we need to copy nicely - this should be at most one block
    std::array<uint8_t, kTransformBufferBlocksMaxBlocksize> block = {};
    auto res = impl.decryptFinal(block.data(), &outLen);
    if (!res) {
      ret = false;
      return Status::Success;
    }
    outputCursor.push(block.data(), outLen);
    ret = true;
    return Status::Success;
  }
}

/**
 * EVPEncImp has the following requirements:
 *     * bool EVPDecImp::encryptUpdate - returns if each evp is encrypted
 * successfully
 *     * bool EVPDecImp::encryptFinal - returns if encryption is finalized
 * successfully
 */
template <class EVPEncImpl>
Status encFuncBlocks(
    Error& err,
    EVPEncImpl&& impl,
    const folly::IOBuf& plaintext,
    folly::IOBuf& output,
    size_t blockSize) {
  size_t totalWritten = 0;
  size_t totalInput = 0;
  int outLen = 0;
  folly::io::RWPrivateCursor outputCursor(&output);
  FIZZ_RETURN_ON_ERROR(transformBufferBlocks(
      outputCursor,
      err,
      plaintext,
      output,
      [&impl, &outLen, &totalWritten, &totalInput](
          size_t& written,
          Error& innerErr,
          uint8_t* cipher,
          const uint8_t* plain,
          size_t len) -> Status {
        if (len > std::numeric_limits<int>::max()) {
          return innerErr.error("Encryption error: too much plain text");
        }
        if (len == 0) {
          written = 0;
          return Status::Success;
        }
        if (!impl.encryptUpdate(
                cipher, &outLen, plain, static_cast<int>(len)) ||
            outLen < 0) {
          return innerErr.error("Encryption error");
        }
        totalWritten += outLen;
        totalInput += len;
        written = static_cast<size_t>(outLen);
        return Status::Success;
      },
      blockSize));

  // We might end up needing to write more in the final encrypt stage
  auto numBuffered = totalInput - totalWritten;
  auto numLeftInOutput = outputCursor.length();
  if (numBuffered <= numLeftInOutput) {
    if (!impl.encryptFinal(outputCursor.writableData(), &outLen)) {
      return err.error("Encryption error");
    }
  } else {
    // we need to copy nicely - this should be at most one block
    std::array<uint8_t, kTransformBufferBlocksMaxBlocksize> block = {};
    if (!impl.encryptFinal(block.data(), &outLen)) {
      return err.error("Encryption error");
    }
    outputCursor.push(block.data(), outLen);
  }
  return Status::Success;
}

/**
 * An object satisfies AeadImpl if the following requirements hold:
 *     * Status AeadImpl::init(Error& err, folly::ByteRange iv,
 * const folly::IOBuf* associatedData, size_t plaintextLength)
 *       - initializes an encryption context with `iv` and
 * associated data. Associated data can be null.
 *
 *     * Status AeadImpl::encrypt(Error& err, folly::IOBuf& ciphertext,
 * folly::IOBuf& plaintext)
 *       - encrypts `plaintextLength` bytes of `plaintext`. The implementation
 * must write the ciphertext to `ciphertext`. `cipherText` is guaranteed to be
 * writable for `plaintextLength` bytes.
 *
 *     * Status AeadImpl::final(Error& err, int tagLen, void* tagOut)
 *       - finalizes the encryption, and writes the resulting AEAD tag into
 * `tagOut` which is guaranteed to be at least `taglen` bytes.
 */
template <class AeadImpl>
Status encryptHelper(
    std::unique_ptr<folly::IOBuf>& ret,
    Error& err,
    AeadImpl&& impl,
    std::unique_ptr<folly::IOBuf>&& plaintext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    size_t tagLen,
    size_t headroom,
    Aead::AeadOptions options) {
  auto inputLength = plaintext->computeChainDataLength();
  FIZZ_RETURN_ON_ERROR(impl.init(err, iv, associatedData, inputLength));

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
    return err.error("Cannot encrypt (must be in-place or allow allocation)");
  }

  if (allowInplaceEdit) {
    output = std::move(plaintext);
    input = output.get();
  } else {
    // create enough to also fit the tag and headroom
    size_t totalSize{0};
    if (!folly::checked_add<size_t>(
            &totalSize, headroom, inputLength, tagLen)) {
      return err.error(
          "Output buffer size", folly::none, Error::Category::StdOverFlow);
    }
    output = folly::IOBuf::create(totalSize);
    output->advance(headroom);
    output->append(inputLength);
    input = plaintext.get();
  }

  FIZZ_RETURN_ON_ERROR(impl.encrypt(err, *input, *output));

  // output is always something we can modify
  auto tailRoom = output->prev()->tailroom();
  if (tailRoom < tagLen || !allowGrowth) {
    if (!allowAlloc) {
      return err.error("Cannot encrypt (insufficient space for tag)");
    }
    std::unique_ptr<folly::IOBuf> tag = folly::IOBuf::create(tagLen);
    tag->append(tagLen);
    FIZZ_RETURN_ON_ERROR(impl.final(err, tagLen, tag->writableData()));
    output->prependChain(std::move(tag));
  } else {
    auto lastBuf = output->prev();
    lastBuf->append(tagLen);
    // we can copy into output directly
    FIZZ_RETURN_ON_ERROR(
        impl.final(err, tagLen, lastBuf->writableTail() - tagLen));
  }
  ret = std::move(output);
  return Status::Success;
}

/**
 * AeadImpl has the following requirements:
 *     * Status AeadImpl::init(Error& err, folly::ByteRange iv,
 * const folly::IOBuf* associatedData, size_t ciphertextLength)
 *       - initializes a decryption context with `iv` and associated data.
 * Associated data can be null.
 *     * Status AeadImpl::decryptAndFinal(bool& ret, Error& err,
 * folly::IOBuf& ciphertext, folly::IOBuf& plaintext,
 * folly::MutableByteRange tagOut)
 *       - decrypts `ciphertextLength` bytes of `ciphertext`. The implementation
 * must write the plaintext to `plaintext`. `plaintext` is guaranteed to be
 * writable for `ciphertextLength` bytes. Sets ret to whether the decryption was
 * successful.
 */
template <class AeadImpl>
Status decryptHelper(
    folly::Optional<std::unique_ptr<folly::IOBuf>>& ret,
    Error& err,
    AeadImpl&& impl,
    std::unique_ptr<folly::IOBuf>&& ciphertext,
    const folly::IOBuf* associatedData,
    folly::ByteRange iv,
    folly::MutableByteRange tagOut,
    bool inPlace) {
  auto inputLength = ciphertext->computeChainDataLength();
  FIZZ_RETURN_ON_ERROR(impl.init(err, iv, associatedData, inputLength));

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

  bool decrypted;
  FIZZ_RETURN_ON_ERROR(
      impl.decryptAndFinal(decrypted, err, *input, *output, tagOut));

  if (!decrypted) {
    ret = folly::none;
  } else {
    ret = std::move(output);
  }
  return Status::Success;
}

} // namespace fizz
