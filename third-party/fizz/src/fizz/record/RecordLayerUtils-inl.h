/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <fizz/crypto/aead/IOBufUtil.h>
#include <fizz/record/Types.h>

namespace fizz {

// Maximum size for encrypted records (16k + 256)
constexpr uint16_t kMaxEncryptedRecordSize = 0x4000 + 256;

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
    const folly::IOBuf* header,
    const folly::IOBuf* aad,
    uint64_t seqNum,
    Aead::AeadOptions options) {
  // Encrypt the data using the provided AAD for integrity protection
  auto cipherText = aead->encrypt(std::move(plaintext), aad, seqNum, options);

  // Construct the final record with the header for on-wire framing
  // Header is always required in StopTLSV2
  DCHECK(header != nullptr);
  DCHECK_GT(header->length(), 0);

  std::unique_ptr<folly::IOBuf> record;
  if (!cipherText->isShared() && cipherText->headroom() >= header->length()) {
    // Prepend the header to the ciphertext
    cipherText->prepend(header->length());
    memcpy(cipherText->writableData(), header->data(), header->length());
    record = std::move(cipherText);
  } else {
    // Create a new buffer for the header
    record = folly::IOBuf::copyBuffer(header->data(), header->length());
    record->prependChain(std::move(cipherText));
  }

  return record;
}

inline RecordLayerUtils::ParsedEncryptedRecord
RecordLayerUtils::parseEncryptedRecord(folly::IOBufQueue& buf) {
  using ContentTypeType = typename std::underlying_type<ContentType>::type;

  auto frontBuf = buf.front();
  folly::io::Cursor cursor(frontBuf);

  // Precondition: Caller must ensure buffer has at least kEncryptedHeaderSize
  // bytes
  DCHECK(!buf.empty() && cursor.canAdvance(kEncryptedHeaderSize))
      << "parseEncryptedRecord called with insufficient buffer data";

  // Create additional data buffer from the header
  std::array<uint8_t, kEncryptedHeaderSize> ad{};
  folly::io::Cursor adCursor(cursor);
  adCursor.pull(ad.data(), ad.size());
  auto adBuf = folly::IOBuf::copyBuffer(ad.data(), ad.size());

  auto contentType = static_cast<ContentType>(cursor.readBE<ContentTypeType>());
  cursor.skip(sizeof(ProtocolVersion));
  auto length = cursor.readBE<uint16_t>();

  if (length == 0) {
    throw std::runtime_error("received 0 length encrypted record");
  }
  if (length > kMaxEncryptedRecordSize) {
    throw std::runtime_error("received too long encrypted record");
  }

  auto consumedBytes = cursor - frontBuf;

  // Precondition: Caller must ensure buffer has enough data for full record
  DCHECK_GE(buf.chainLength(), consumedBytes + length)
      << "parseEncryptedRecord called with incomplete record data";

  if (contentType == ContentType::alert && length == 2) {
    auto alert = decode<Alert>(cursor);
    throw std::runtime_error(folly::to<std::string>(
        "received plaintext alert in encrypted record: ",
        toString(alert.description)));
  }

  // Extract the ciphertext
  std::unique_ptr<folly::IOBuf> ciphertext;
  if (buf.chainLength() == consumedBytes + length) {
    ciphertext = buf.move();
  } else {
    ciphertext = buf.split(consumedBytes + length);
  }
  trimStart(*ciphertext, consumedBytes);

  ParsedEncryptedRecord result;
  result.contentType = contentType;
  result.ciphertext = std::move(ciphertext);
  result.header = std::move(adBuf);
  result.continueReading = false;

  if (contentType == ContentType::change_cipher_spec) {
    result.ciphertext->coalesce();
    if (result.ciphertext->length() == 1 &&
        *result.ciphertext->data() == 0x01) {
      result.continueReading = true;
    } else {
      throw FizzException("received ccs", AlertDescription::illegal_parameter);
    }
  }

  return result;
}

inline std::unique_ptr<folly::IOBuf> prepareBufferWithPadding(
    folly::IOBufQueue& queue,
    ContentType contentType,
    const BufAndPaddingPolicy& paddingPolicy,
    uint16_t maxRecord,
    Aead* aead) {
  // Caller contract: queue must not be empty
  DCHECK(!queue.empty());

  // Get the buffer and padding size from the policy
  auto bufAndPadding =
      paddingPolicy.getBufAndPaddingToEncrypt(queue, maxRecord);
  auto dataBuf = std::move(bufAndPadding.first);
  auto paddingSize = bufAndPadding.second;

  // Assert our invariant that dataBuf should never be null
  DCHECK(dataBuf) << "getBufAndPaddingToEncrypt returned nullptr";

  // check if we have enough room to add padding and the encrypted footer.
  if (!dataBuf->isShared() &&
      dataBuf->prev()->tailroom() >= sizeof(ContentType) + paddingSize) {
    folly::io::Appender appender(dataBuf.get(), 0);
    appender.writeBE(static_cast<uint8_t>(contentType));
    memset(appender.writableData(), 0, paddingSize);
    appender.append(paddingSize);
  } else {
    auto encryptedFooter = folly::IOBuf::create(
        sizeof(ContentType) + paddingSize + aead->getCipherOverhead());
    folly::io::Appender appender(encryptedFooter.get(), 0);
    appender.writeBE(static_cast<uint8_t>(contentType));
    memset(appender.writableData(), 0, paddingSize);
    appender.append(paddingSize);
    dataBuf->prependChain(std::move(encryptedFooter));
  }

  return dataBuf;
}

} // namespace fizz
