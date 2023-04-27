/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/crypto/aead/IOBufUtil.h>
#include <fizz/record/EncryptedRecordLayer.h>

namespace fizz {

using ContentTypeType = typename std::underlying_type<ContentType>::type;
using ProtocolVersionType =
    typename std::underlying_type<ProtocolVersion>::type;

static constexpr uint16_t kMaxEncryptedRecordSize = 0x4000 + 256; // 16k + 256
static constexpr size_t kEncryptedHeaderSize =
    sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

EncryptedReadRecordLayer::ReadResult<Buf>
EncryptedReadRecordLayer::getDecryptedBuf(
    folly::IOBufQueue& buf,
    Aead::AeadOptions options) {
  while (true) {
    // Cache the front buffer, calling front may invoke and update
    // of the tail cache.
    auto frontBuf = buf.front();
    folly::io::Cursor cursor(frontBuf);

    if (buf.empty() || !cursor.canAdvance(kEncryptedHeaderSize)) {
      return ReadResult<Buf>::noneWithSizeHint(
          kEncryptedHeaderSize - buf.chainLength());
    }

    std::array<uint8_t, kEncryptedHeaderSize> ad;
    folly::io::Cursor adCursor(cursor);
    adCursor.pull(ad.data(), ad.size());
    folly::IOBuf adBuf{folly::IOBuf::wrapBufferAsValue(folly::range(ad))};

    auto contentType =
        static_cast<ContentType>(cursor.readBE<ContentTypeType>());
    cursor.skip(sizeof(ProtocolVersion));

    auto length = cursor.readBE<uint16_t>();
    if (length == 0) {
      throw std::runtime_error("received 0 length encrypted record");
    }
    if (length > kMaxEncryptedRecordSize) {
      throw std::runtime_error("received too long encrypted record");
    }
    auto consumedBytes = cursor - frontBuf;
    if (buf.chainLength() < consumedBytes + length) {
      auto remaining = (consumedBytes + length) - buf.chainLength();
      return ReadResult<Buf>::noneWithSizeHint(remaining);
    }

    if (contentType == ContentType::alert && length == 2) {
      auto alert = decode<Alert>(cursor);
      throw std::runtime_error(folly::to<std::string>(
          "received plaintext alert in encrypted record: ",
          toString(alert.description)));
    }

    // If we already know that the length of the buffer is the
    // same as the number of bytes we need, move the entire buffer.
    std::unique_ptr<folly::IOBuf> encrypted;
    if (buf.chainLength() == consumedBytes + length) {
      encrypted = buf.move();
    } else {
      encrypted = buf.split(consumedBytes + length);
    }
    trimStart(*encrypted, consumedBytes);

    if (contentType == ContentType::change_cipher_spec) {
      encrypted->coalesce();
      if (encrypted->length() == 1 && *encrypted->data() == 0x01) {
        continue;
      } else {
        throw FizzException(
            "received ccs", AlertDescription::illegal_parameter);
      }
    }

    if (seqNum_ == std::numeric_limits<uint64_t>::max()) {
      throw std::runtime_error("max read seq num");
    }
    if (skipFailedDecryption_) {
      auto decryptAttempt = aead_->tryDecrypt(
          std::move(encrypted),
          useAdditionalData_ ? &adBuf : nullptr,
          seqNum_,
          options);
      if (decryptAttempt) {
        seqNum_++;
        skipFailedDecryption_ = false;
        return ReadResult<Buf>::from(std::move(decryptAttempt).value());
      } else {
        continue;
      }
    } else {
      return ReadResult<Buf>::from(aead_->decrypt(
          std::move(encrypted),
          useAdditionalData_ ? &adBuf : nullptr,
          seqNum_++,
          options));
    }
  }
}

EncryptedReadRecordLayer::ReadResult<TLSMessage> EncryptedReadRecordLayer::read(
    folly::IOBufQueue& buf,
    Aead::AeadOptions options) {
  auto decryptedBuf = getDecryptedBuf(buf, std::move(options));
  if (!decryptedBuf) {
    return ReadResult<TLSMessage>::noneWithSizeHint(decryptedBuf.sizeHint);
  }

  TLSMessage msg{};
  // Iterate over the buffers while trying to find
  // the first non-zero octet. This is much faster than
  // first iterating and then trimming.
  auto currentBuf = decryptedBuf->get();
  bool nonZeroFound = false;
  do {
    currentBuf = currentBuf->prev();
    size_t i = currentBuf->length();
    while (i > 0 && !nonZeroFound) {
      nonZeroFound = (currentBuf->data()[i - 1] != 0);
      i--;
    }
    if (nonZeroFound) {
      msg.type = static_cast<ContentType>(currentBuf->data()[i]);
    }
    currentBuf->trimEnd(currentBuf->length() - i);
  } while (!nonZeroFound && currentBuf != decryptedBuf->get());
  if (!nonZeroFound) {
    throw std::runtime_error("No content type found");
  }
  msg.fragment = std::move(*decryptedBuf);

  switch (msg.type) {
    case ContentType::handshake:
    case ContentType::alert:
    case ContentType::application_data:
      break;
    default:
      throw std::runtime_error(folly::to<std::string>(
          "received encrypted content type ",
          static_cast<ContentTypeType>(msg.type)));
  }

  if (!msg.fragment || msg.fragment->empty()) {
    if (msg.type == ContentType::application_data) {
      msg.fragment = folly::IOBuf::create(0);
    } else {
      throw std::runtime_error("received empty fragment");
    }
  }

  return ReadResult<TLSMessage>::from(std::move(msg));
}

EncryptionLevel EncryptedReadRecordLayer::getEncryptionLevel() const {
  return encryptionLevel_;
}

TLSContent EncryptedWriteRecordLayer::write(
    TLSMessage&& msg,
    Aead::AeadOptions options) const {
  folly::IOBufQueue queue;
  queue.append(std::move(msg.fragment));
  std::unique_ptr<folly::IOBuf> outBuf;
  std::array<uint8_t, kEncryptedHeaderSize> headerBuf;
  auto header = folly::IOBuf::wrapBufferAsValue(folly::range(headerBuf));
  aead_->setEncryptedBufferHeadroom(kEncryptedHeaderSize);
  while (!queue.empty()) {
    Buf dataBuf;
    uint16_t paddingSize;
    std::tie(dataBuf, paddingSize) =
        bufAndPaddingPolicy_->getBufAndPaddingToEncrypt(queue, maxRecord_);

    // check if we have enough room to add padding and the encrypted footer.
    if (!dataBuf->isShared() &&
        dataBuf->prev()->tailroom() >= sizeof(ContentType) + paddingSize) {
      // extend it and add padding and footer
      folly::io::Appender appender(dataBuf.get(), 0);
      appender.writeBE(static_cast<ContentTypeType>(msg.type));
      memset(appender.writableData(), 0, paddingSize);
      appender.append(paddingSize);
    } else {
      // not enough or shared - let's add enough for the tag as well
      auto encryptedFooter = folly::IOBuf::create(
          sizeof(ContentType) + paddingSize + aead_->getCipherOverhead());
      folly::io::Appender appender(encryptedFooter.get(), 0);
      appender.writeBE(static_cast<ContentTypeType>(msg.type));
      memset(appender.writableData(), 0, paddingSize);
      appender.append(paddingSize);
      dataBuf->prependChain(std::move(encryptedFooter));
    }

    if (seqNum_ == std::numeric_limits<uint64_t>::max()) {
      throw std::runtime_error("max write seq num");
    }

    // we will either be able to memcpy directly into the ciphertext or
    // need to create a new buf to insert before the ciphertext but we need
    // it for additional data
    header.clear();
    folly::io::Appender appender(&header, 0);
    appender.writeBE(
        static_cast<ContentTypeType>(ContentType::application_data));
    appender.writeBE(
        static_cast<ProtocolVersionType>(ProtocolVersion::tls_1_2));
    auto ciphertextLength =
        dataBuf->computeChainDataLength() + aead_->getCipherOverhead();
    appender.writeBE<uint16_t>(ciphertextLength);

    auto cipherText = aead_->encrypt(
        std::move(dataBuf),
        useAdditionalData_ ? &header : nullptr,
        seqNum_++,
        options);

    std::unique_ptr<folly::IOBuf> record;
    if (!cipherText->isShared() &&
        cipherText->headroom() >= kEncryptedHeaderSize) {
      // prepend and then write it in
      cipherText->prepend(kEncryptedHeaderSize);
      memcpy(cipherText->writableData(), header.data(), header.length());
      record = std::move(cipherText);
    } else {
      record = folly::IOBuf::copyBuffer(header.data(), header.length());
      record->prependChain(std::move(cipherText));
    }

    if (!outBuf) {
      outBuf = std::move(record);
    } else {
      outBuf->prependChain(std::move(record));
    }
  }

  if (!outBuf) {
    outBuf = folly::IOBuf::create(0);
  }

  TLSContent content;
  content.data = std::move(outBuf);
  content.contentType = msg.type;
  content.encryptionLevel = encryptionLevel_;
  return content;
}

EncryptionLevel EncryptedWriteRecordLayer::getEncryptionLevel() const {
  return encryptionLevel_;
}
} // namespace fizz
