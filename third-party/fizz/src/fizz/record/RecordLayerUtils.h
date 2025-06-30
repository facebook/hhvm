// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

#pragma once

#include <fizz/crypto/aead/Aead.h>
#include <fizz/record/BufAndPaddingPolicy.h>
#include <fizz/record/Types.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBufQueue.h>

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

  /**
   * Structure to hold parsed encrypted record data before decryption.
   * This is used to share parsing logic between different record layer
   * implementations.
   */
  struct ParsedEncryptedRecord {
    ContentType contentType{
        ContentType::application_data}; // Default initialization
    std::unique_ptr<folly::IOBuf> ciphertext;
    std::unique_ptr<folly::IOBuf> header;
    bool continueReading{false}; // Set to true for change_cipher_spec records
  };

  /**
   * PRECONDITIONS:
   * - Buffer must contain at least kEncryptedHeaderSize bytes
   * - Buffer must contain the complete record (header + payload)
   * Caller is responsible for validating these conditions.
   *
   * @return The parsed record data
   * @throws std::runtime_error if the record contains invalid protocol data
   */
  static ParsedEncryptedRecord parseEncryptedRecord(folly::IOBufQueue& buf);
};

/**
 * Prepares a buffer with appropriate padding for encryption.
 * This function handles extracting data from the queue and applying padding
 * according to the provided policy.
 *
 * REQUIRES: `queue` must not be empty. Caller is responsible for enforcing.
 *
 * @param queue The input buffer queue containing data to encrypt
 * @param contentType The TLS content type to append
 * @param paddingPolicy The padding policy to use
 * @param maxRecord Maximum record size
 * @param aead The AEAD implementation to use
 * @return A buffer prepared for encryption
 */
std::unique_ptr<folly::IOBuf> prepareBufferWithPadding(
    folly::IOBufQueue& queue,
    ContentType contentType,
    const BufAndPaddingPolicy& paddingPolicy,
    uint16_t maxRecord,
    Aead* aead);

} // namespace fizz

#include <fizz/record/RecordLayerUtils-inl.h>
