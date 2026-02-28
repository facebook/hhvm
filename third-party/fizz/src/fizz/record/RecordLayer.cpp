/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/RecordLayer.h>
#include <fizz/util/Logging.h>

namespace fizz {

using HandshakeTypeType = typename std::underlying_type<HandshakeType>::type;

static constexpr size_t kHandshakeHeaderSize =
    sizeof(HandshakeType) + detail::bits24::size;

#define TRY FIZZ_RETURN_ON_ERROR

Status ReadRecordLayer::readEvent(
    ReadResult<Param>& ret,
    Error& err,
    folly::IOBufQueue& socketBuf,
    Aead::AeadOptions options) {
  if (!unparsedHandshakeData_.empty()) {
    folly::Optional<Param> param;
    TRY(decodeHandshakeMessage(param, err, unparsedHandshakeData_));
    if (param) {
      FIZZ_VLOG(8) << "Received handshake message "
                   << toString(EventVisitor()(*param));
      ret = ReadResult<Param>::from(std::move(param).value());
      return Status::Success;
    }
  }

  while (true) {
    // Read one record. We read one record at a time since records could cause
    // a change in the record layer.
    ReadResult<TLSMessage> messageResult;
    TRY(read(messageResult, err, socketBuf, options));
    if (!messageResult) {
      ret = ReadResult<Param>::noneWithSizeHint(messageResult.sizeHint);
      return Status::Success;
    }

    auto& message = messageResult.message;

    if (!unparsedHandshakeData_.empty() &&
        message->type != ContentType::handshake) {
      return err.error("spliced handshake data");
    }

    switch (message->type) {
      case ContentType::alert: {
        Alert alert;
        TRY(decode<Alert>(alert, err, std::move(message->fragment)));
        if (alert.description == AlertDescription::close_notify) {
          ret = ReadResult<Param>::from(Param(CloseNotify(socketBuf.move())));
        } else {
          ret = ReadResult<Param>::from(Param(std::move(alert)));
        }
        return Status::Success;
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
        folly::Optional<Param> param;
        TRY(decodeHandshakeMessage(param, err, unparsedHandshakeData_));
        if (param) {
          FIZZ_VLOG(8) << "Received handshake message "
                       << toString(EventVisitor()(*param));
          ret = ReadResult<Param>::from(std::move(param).value());
          return Status::Success;
        } else {
          // If we read handshake data but didn't have enough to get a full
          // message we immediately try to read another record.
          // TODO: add limits on number of records we buffer
          continue;
        }
      }
      case ContentType::application_data:
        ret = ReadResult<Param>::from(
            Param(AppData(std::move(message->fragment))));
        return Status::Success;
      default:
        return err.error("unknown content type");
    }
  }
}

template <typename T>
static Status
parse(folly::Optional<Param>& ret, Error& err, Buf handshakeMsg, Buf original) {
  T msg;
  TRY(decode<T>(msg, err, std::move(handshakeMsg)));
  msg.originalEncoding = std::move(original);
  ret = std::move(msg);
  return Status::Success;
}

template <>
Status parse<ServerHello>(
    folly::Optional<Param>& ret,
    Error& err,
    Buf handshakeMsg,
    Buf original) {
  ServerHello shlo;
  TRY(decode<ServerHello>(shlo, err, std::move(handshakeMsg)));
  if (shlo.random == HelloRetryRequest::HrrRandom) {
    HelloRetryRequest hrr;
    hrr.legacy_version = shlo.legacy_version;
    hrr.legacy_session_id_echo = std::move(shlo.legacy_session_id_echo);
    hrr.cipher_suite = shlo.cipher_suite;
    hrr.legacy_compression_method = shlo.legacy_compression_method;
    hrr.extensions = std::move(shlo.extensions);

    hrr.originalEncoding = std::move(original);
    ret = std::move(hrr);
  } else {
    shlo.originalEncoding = std::move(original);
    ret = std::move(shlo);
  }
  return Status::Success;
}

Status ReadRecordLayer::decodeHandshakeMessage(
    folly::Optional<Param>& ret,
    Error& err,
    folly::IOBufQueue& buf) {
  folly::io::Cursor cursor(buf.front());

  if (!cursor.canAdvance(kHandshakeHeaderSize)) {
    ret = folly::none;
    return Status::Success;
  }

  auto handshakeType =
      static_cast<HandshakeType>(cursor.readBE<HandshakeTypeType>());
  auto length = detail::readBits24(cursor);

  if (length > kMaxHandshakeSize) {
    return err.error("handshake record too big");
  }
  if (buf.chainLength() < (cursor - buf.front()) + length) {
    ret = folly::none;
    return Status::Success;
  }

  Buf handshakeMsg;
  cursor.clone(handshakeMsg, length);
  auto original = buf.split(kHandshakeHeaderSize + length);

  switch (handshakeType) {
    case HandshakeType::client_hello:
      return parse<ClientHello>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::server_hello:
      return parse<ServerHello>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::end_of_early_data:
      return parse<EndOfEarlyData>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::new_session_ticket:
      return parse<NewSessionTicket>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::encrypted_extensions:
      return parse<EncryptedExtensions>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::certificate:
      return parse<CertificateMsg>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::compressed_certificate:
      return parse<CompressedCertificate>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::certificate_request:
      return parse<CertificateRequest>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::certificate_verify:
      return parse<CertificateVerify>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::finished:
      return parse<Finished>(
          ret, err, std::move(handshakeMsg), std::move(original));
    case HandshakeType::key_update:
      return parse<KeyUpdate>(
          ret, err, std::move(handshakeMsg), std::move(original));
    default:
      return err.error("unknown handshake type");
  }
}
bool ReadRecordLayer::hasUnparsedHandshakeData() const {
  return !unparsedHandshakeData_.empty();
}
} // namespace fizz
