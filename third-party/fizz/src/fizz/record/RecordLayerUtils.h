// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <fizz/record/Types.h>
#include <folly/io/Cursor.h>

namespace fizz {

/**
 * Common utilities for TLS record layer operations.
 * This extracts shared functionality to avoid code duplication.
 */
class RecordLayerUtils {
 public:
  /**
   * Extract and remove the content type from decrypted data.
   * Returns an Optional ContentType and modifies the input buffer
   * to remove padding and the content type byte.
   */
  static folly::Optional<ContentType> parseAndRemoveContentType(
      std::unique_ptr<folly::IOBuf>& decryptedBuf);
};

} // namespace fizz

#include <fizz/record/RecordLayerUtils-inl.h>
