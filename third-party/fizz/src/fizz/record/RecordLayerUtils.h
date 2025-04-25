// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/record/Types.h>
#include <folly/io/Cursor.h>

namespace fizz {

/**
 * Common utilities for TLS record layer operations.
 * This extracts shared functionality to avoid code duplication.
 */
class RecordLayerUtils {
 public:
  // Constants needed for record layer operations
  static constexpr size_t kEncryptedHeaderSize =
      sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

  /**
   * Extract and remove the content type from decrypted data.
   * Returns an Optional ContentType and modifies the input buffer
   * to remove padding and the content type byte.
   */
  static folly::Optional<ContentType> parseAndRemoveContentType(
      std::unique_ptr<folly::IOBuf>& decryptedBuf);

  /**
   * Write an encrypted record with the given parameters.
   * This function handles encrypting the data and assembling the final TLS
   * record.
   */
  static std::unique_ptr<folly::IOBuf> writeEncryptedRecord(
      std::unique_ptr<folly::IOBuf> plaintext,
      Aead* aead,
      folly::IOBuf* header,
      uint64_t seqNum,
      bool useAdditionalData,
      Aead::AeadOptions options);
};

} // namespace fizz

#include <fizz/record/RecordLayerUtils-inl.h>
