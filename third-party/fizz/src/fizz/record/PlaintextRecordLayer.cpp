/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/PlaintextRecordLayer.h>

#include <folly/String.h>

namespace fizz {

using ContentTypeType = typename std::underlying_type<ContentType>::type;
using ProtocolVersionType =
    typename std::underlying_type<ProtocolVersion>::type;

static constexpr uint16_t kMaxPlaintextRecordSize = 0x4000; // 16k
static constexpr size_t kPlaintextHeaderSize =
    sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

Status PlaintextReadRecordLayer::read(
    ReadResult<TLSMessage>& ret,
    Error& err,
    folly::IOBufQueue& buf,
    Aead::AeadOptions) {
  while (true) {
    folly::io::Cursor cursor(buf.front());

    if (buf.empty() || !cursor.canAdvance(kPlaintextHeaderSize)) {
      ret = ReadResult<TLSMessage>::noneWithSizeHint(
          kPlaintextHeaderSize - buf.chainLength());
      return Status::Success;
    }

    TLSMessage msg;
    msg.type = static_cast<ContentType>(cursor.readBE<ContentTypeType>());

    if (skipEncryptedRecords_) {
      if (msg.type == ContentType::application_data) {
        cursor.skip(sizeof(ProtocolVersion));
        auto length = cursor.readBE<uint16_t>();
        if (buf.chainLength() < (cursor - buf.front()) + length) {
          auto missing = ((cursor - buf.front()) + length) - buf.chainLength();
          ret = ReadResult<TLSMessage>::noneWithSizeHint(missing);
          return Status::Success;
        }
        buf.trimStart(static_cast<size_t>(kPlaintextHeaderSize) + length);
        continue;
      } else if (msg.type != ContentType::change_cipher_spec) {
        skipEncryptedRecords_ = false;
      }
    }

    switch (msg.type) {
      case ContentType::handshake:
      case ContentType::alert:
        break;
      case ContentType::change_cipher_spec:
        break;
      default:
        return err.error(
            folly::to<std::string>(
                "received plaintext content type ",
                static_cast<ContentTypeType>(msg.type),
                ", header: ",
                folly::hexlify(buf.splitAtMost(10)->coalesce())));
    }

    receivedRecordVersion_ =
        static_cast<ProtocolVersion>(cursor.readBE<ProtocolVersionType>());

    auto length = cursor.readBE<uint16_t>();
    if (length > kMaxPlaintextRecordSize) {
      return err.error("received too long plaintext record");
    }
    if (length == 0) {
      return err.error("received empty plaintext record");
    }
    if (buf.chainLength() < (cursor - buf.front()) + length) {
      auto missing = ((cursor - buf.front()) + length) - buf.chainLength();
      ret = ReadResult<TLSMessage>::noneWithSizeHint(missing);
      return Status::Success;
    }

    cursor.clone(msg.fragment, length);

    buf.trimStart(cursor - buf.front());

    if (msg.type == ContentType::change_cipher_spec) {
      msg.fragment->coalesce();
      if (msg.fragment->length() == 1 && *msg.fragment->data() == 0x01) {
        continue;
      } else {
        return err.error("received ccs", AlertDescription::illegal_parameter);
      }
    }

    ret = ReadResult<TLSMessage>::from(std::move(msg));
    return Status::Success;
  }
}

EncryptionLevel PlaintextReadRecordLayer::getEncryptionLevel() const {
  return EncryptionLevel::Plaintext;
}

Status PlaintextWriteRecordLayer::write(
    TLSContent& ret,
    Error& err,
    TLSMessage&& msg,
    Aead::AeadOptions /*options*/) const {
  return write(ret, err, std::move(msg), ProtocolVersion::tls_1_2);
}

Status PlaintextWriteRecordLayer::writeInitialClientHello(
    TLSContent& ret,
    Error& err,
    Buf encodedClientHello) const {
  return write(
      ret,
      err,
      TLSMessage{ContentType::handshake, std::move(encodedClientHello)},
      ProtocolVersion::tls_1_0);
}

Status PlaintextWriteRecordLayer::write(
    TLSContent& ret,
    Error& err,
    TLSMessage msg,
    ProtocolVersion recordVersion) const {
  if (msg.type == ContentType::application_data) {
    return err.error("refusing to send plaintext application data");
  }

  auto fragment = std::move(msg.fragment);
  folly::io::Cursor cursor(fragment.get());
  std::unique_ptr<folly::IOBuf> data;
  while (!cursor.isAtEnd()) {
    Buf thisFragment;
    auto len = cursor.cloneAtMost(thisFragment, kMaxPlaintextRecordSize);

    auto header = folly::IOBuf::create(kPlaintextHeaderSize);
    folly::io::Appender appender(header.get(), kPlaintextHeaderSize);
    appender.writeBE(static_cast<ContentTypeType>(msg.type));
    appender.writeBE(static_cast<ProtocolVersionType>(recordVersion));
    appender.writeBE<uint16_t>(len);

    if (!data) {
      data = std::move(header);
    } else {
      data->prependChain(std::move(header));
    }
    data->prependChain(std::move(thisFragment));
  }
  TLSContent content;
  content.data = std::move(data);
  content.contentType = msg.type;
  content.encryptionLevel = EncryptionLevel::Plaintext;
  ret = std::move(content);
  return Status::Success;
}

EncryptionLevel PlaintextWriteRecordLayer::getEncryptionLevel() const {
  return EncryptionLevel::Plaintext;
}
} // namespace fizz
