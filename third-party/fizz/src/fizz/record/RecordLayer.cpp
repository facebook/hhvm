/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/RecordLayer.h>

namespace fizz {

using HandshakeTypeType = typename std::underlying_type<HandshakeType>::type;

static constexpr size_t kHandshakeHeaderSize =
    sizeof(HandshakeType) + detail::bits24::size;

ReadRecordLayer::ReadResult<Param> ReadRecordLayer::readEvent(
    folly::IOBufQueue& socketBuf,
    Aead::AeadOptions options) {
  if (!unparsedHandshakeData_.empty()) {
    auto param = decodeHandshakeMessage(unparsedHandshakeData_);
    if (param) {
      VLOG(8) << "Received handshake message "
              << toString(EventVisitor()(*param));
      return ReadResult<Param>::from(std::move(param).value());
    }
  }

  while (true) {
    // Read one record. We read one record at a time since records could cause
    // a change in the record layer.
    auto messageResult = read(socketBuf, options);
    if (!messageResult) {
      return ReadResult<Param>::noneWithSizeHint(messageResult.sizeHint);
    }

    auto& message = messageResult.message;

    if (!unparsedHandshakeData_.empty() &&
        message->type != ContentType::handshake) {
      throw std::runtime_error("spliced handshake data");
    }

    switch (message->type) {
      case ContentType::alert: {
        auto alert = decode<Alert>(std::move(message->fragment));
        if (alert.description == AlertDescription::close_notify) {
          return ReadResult<Param>::from(Param(CloseNotify(socketBuf.move())));
        } else {
          return ReadResult<Param>::from(Param(std::move(alert)));
        }
      }
      case ContentType::handshake: {
        std::unique_ptr<folly::IOBuf> handshakeMessage =
            unparsedHandshakeData_.move();
        // It is possible that a peer might send us records in a manner such
        // that there is a 16KB record and only 1 byte of handshake message in
        // each record. Since we normally just trim the IOBuf, we would end up
        // holding 16K of data. To prevent this we allocate a contiguous
        // buffer to copy over these bytes. We supply kExtraAlloc bytes in
        // order to avoid needing to re-allocate a lot of times if we receive
        // a lot of small messages. There might be more optimal reallocation
        // policies, but this should be fine.
        message->fragment->coalesce();
        constexpr size_t kExtraAlloc = 1024;
        if (!handshakeMessage) {
          handshakeMessage =
              folly::IOBuf::create(message->fragment->length() + kExtraAlloc);
        } else if (handshakeMessage->tailroom() < message->fragment->length()) {
          // There might be remaining bytes from the previous handshake that are
          // left over in the unparsedHandshakeData_ buffer.
          handshakeMessage->unshare();
          handshakeMessage->reserve(
              0, message->fragment->length() + kExtraAlloc);
        }
        memcpy(
            handshakeMessage->writableTail(),
            message->fragment->data(),
            message->fragment->length());
        handshakeMessage->append(message->fragment->length());
        unparsedHandshakeData_.append(std::move(handshakeMessage));
        auto param = decodeHandshakeMessage(unparsedHandshakeData_);
        if (param) {
          VLOG(8) << "Received handshake message "
                  << toString(EventVisitor()(*param));
          return ReadResult<Param>::from(std::move(param).value());
        } else {
          // If we read handshake data but didn't have enough to get a full
          // message we immediately try to read another record.
          // TODO: add limits on number of records we buffer
          continue;
        }
      }
      case ContentType::application_data:
        return ReadResult<Param>::from(
            Param(AppData(std::move(message->fragment))));
      default:
        throw std::runtime_error("unknown content type");
    }
  }
}

template <typename T>
static Param parse(Buf handshakeMsg, Buf original) {
  auto msg = decode<T>(std::move(handshakeMsg));
  msg.originalEncoding = std::move(original);
  return std::move(msg);
}

template <>
Param parse<ServerHello>(Buf handshakeMsg, Buf original) {
  auto shlo = decode<ServerHello>(std::move(handshakeMsg));
  if (shlo.random == HelloRetryRequest::HrrRandom) {
    HelloRetryRequest hrr;
    hrr.legacy_version = shlo.legacy_version;
    hrr.legacy_session_id_echo = std::move(shlo.legacy_session_id_echo);
    hrr.cipher_suite = shlo.cipher_suite;
    hrr.legacy_compression_method = shlo.legacy_compression_method;
    hrr.extensions = std::move(shlo.extensions);

    hrr.originalEncoding = std::move(original);
    return std::move(hrr);
  } else {
    shlo.originalEncoding = std::move(original);
    return std::move(shlo);
  }
}

folly::Optional<Param> ReadRecordLayer::decodeHandshakeMessage(
    folly::IOBufQueue& buf) {
  folly::io::Cursor cursor(buf.front());

  if (!cursor.canAdvance(kHandshakeHeaderSize)) {
    return folly::none;
  }

  auto handshakeType =
      static_cast<HandshakeType>(cursor.readBE<HandshakeTypeType>());
  auto length = detail::readBits24(cursor);

  if (length > kMaxHandshakeSize) {
    throw std::runtime_error("handshake record too big");
  }
  if (buf.chainLength() < (cursor - buf.front()) + length) {
    return folly::none;
  }

  Buf handshakeMsg;
  cursor.clone(handshakeMsg, length);
  auto original = buf.split(kHandshakeHeaderSize + length);

  switch (handshakeType) {
    case HandshakeType::client_hello:
      return parse<ClientHello>(std::move(handshakeMsg), std::move(original));
    case HandshakeType::server_hello:
      return parse<ServerHello>(std::move(handshakeMsg), std::move(original));
    case HandshakeType::end_of_early_data:
      return parse<EndOfEarlyData>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::new_session_ticket:
      return parse<NewSessionTicket>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::encrypted_extensions:
      return parse<EncryptedExtensions>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::certificate:
      return parse<CertificateMsg>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::compressed_certificate:
      return parse<CompressedCertificate>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::certificate_request:
      return parse<CertificateRequest>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::certificate_verify:
      return parse<CertificateVerify>(
          std::move(handshakeMsg), std::move(original));
    case HandshakeType::finished:
      return parse<Finished>(std::move(handshakeMsg), std::move(original));
    case HandshakeType::key_update:
      return parse<KeyUpdate>(std::move(handshakeMsg), std::move(original));
    default:
      throw std::runtime_error("unknown handshake type");
  }
}

bool ReadRecordLayer::hasUnparsedHandshakeData() const {
  return !unparsedHandshakeData_.empty();
}
} // namespace fizz
