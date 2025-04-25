// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <fizz/record/Types.h>

namespace fizz {

inline folly::Optional<ContentType> RecordLayerUtils::parseAndRemoveContentType(
    std::unique_ptr<folly::IOBuf>& decryptedBuf) {
  // Find the content type byte and remove padding
  auto currentBuf = decryptedBuf.get();
  bool nonZeroFound = false;
  ContentType contentType;

  do {
    currentBuf = currentBuf->prev();
    size_t i = currentBuf->length();
    while (i > 0 && !nonZeroFound) {
      nonZeroFound = (currentBuf->data()[i - 1] != 0);
      i--;
    }
    if (nonZeroFound) {
      contentType = static_cast<ContentType>(currentBuf->data()[i]);
    }
    currentBuf->trimEnd(currentBuf->length() - i);
  } while (!nonZeroFound && currentBuf != decryptedBuf.get());

  if (!nonZeroFound) {
    return folly::none;
  }

  return contentType;
}

inline std::unique_ptr<folly::IOBuf> RecordLayerUtils::writeEncryptedRecord(
    std::unique_ptr<folly::IOBuf> plaintext,
    Aead* aead,
    folly::IOBuf* header,
    uint64_t seqNum,
    bool useAdditionalData,
    Aead::AeadOptions options) {
  // Encrypt the data
  auto cipherText = aead->encrypt(
      std::move(plaintext),
      useAdditionalData ? header : nullptr,
      seqNum,
      options);

  // Construct the final record
  std::unique_ptr<folly::IOBuf> record;
  if (!cipherText->isShared() &&
      cipherText->headroom() >= kEncryptedHeaderSize) {
    // Prepend the header to the ciphertext
    cipherText->prepend(kEncryptedHeaderSize);
    memcpy(cipherText->writableData(), header->data(), header->length());
    record = std::move(cipherText);
  } else {
    // Create a new buffer for the header
    record = folly::IOBuf::copyBuffer(header->data(), header->length());
    record->prependChain(std::move(cipherText));
  }

  return record;
}

} // namespace fizz
