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

} // namespace fizz
