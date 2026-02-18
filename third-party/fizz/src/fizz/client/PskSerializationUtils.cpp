// Copyright 2004-present Facebook. All Rights Reserved.

#include "fizz/client/PskSerializationUtils.h"

#include <fizz/record/Types.h>
#include <folly/io/IOBuf.h>

using namespace folly;

namespace fizz {

namespace {

Status tryWriteCert(
    Error& err,
    const CertificateSerialization& serializer,
    const fizz::Cert* cert,
    io::Appender& appender) {
  std::unique_ptr<folly::IOBuf> serializedCert;
  if (cert) {
    try {
      serializedCert = serializer.serialize(*cert);
    } catch (...) {
    }
  }

  if (serializedCert) {
    FIZZ_RETURN_ON_ERROR(
        fizz::detail::writeBuf<uint32_t>(err, serializedCert, appender));
  } else {
    FIZZ_RETURN_ON_ERROR(
        fizz::detail::writeBuf<uint32_t>(err, nullptr, appender));
  }
  return Status::Success;
}

std::shared_ptr<const fizz::Cert> tryReadCert(
    const CertificateSerialization& serializer,
    folly::io::Cursor& cursor) {
  std::unique_ptr<folly::IOBuf> serializedCert;
  fizz::detail::readBuf<uint32_t>(serializedCert, cursor);
  if (!serializedCert || serializedCert->empty()) {
    return nullptr;
  }
  try {
    return serializer.deserialize(serializedCert->coalesce());
  } catch (...) {
    return nullptr;
  }
}

} // namespace

namespace client {

/**
 * Returns a system_clock::time_point that corresponds to a `value` expressed
 * as a duration of `Duration`, possibly clamping it such that the `value`
 * does not overflow the system_clock's time_point representation.
 */
template <class Duration>
std::chrono::system_clock::time_point clampTimePoint(uint64_t value) {
  static_assert(std::ratio_less_equal_v<
                std::chrono::system_clock::period,
                typename Duration::period>);
  constexpr uint64_t kMaxValue = std::chrono::duration_cast<Duration>(
                                     std::chrono::system_clock::duration::max())
                                     .count();
  return std::chrono::system_clock::time_point(
      Duration(std::min(value, kMaxValue)));
}

Status serializePsk(
    std::string& ret,
    Error& err,
    const CertificateSerialization& serializer,
    const fizz::client::CachedPsk& psk) {
  uint64_t ticketIssueTime =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          psk.ticketIssueTime.time_since_epoch())
          .count();
  uint64_t ticketExpirationTime =
      std::chrono::duration_cast<std::chrono::seconds>(
          psk.ticketExpirationTime.time_since_epoch())
          .count();
  uint64_t ticketHandshakeTime =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          psk.ticketHandshakeTime.time_since_epoch())
          .count();

  auto serialized = IOBuf::create(0);
  io::Appender appender(serialized.get(), 512);
  FIZZ_RETURN_ON_ERROR(
      fizz::detail::writeBuf<uint16_t>(
          err, folly::IOBuf::wrapBuffer(StringPiece(psk.psk)), appender));
  FIZZ_RETURN_ON_ERROR(
      fizz::detail::writeBuf<uint16_t>(
          err, folly::IOBuf::wrapBuffer(StringPiece(psk.secret)), appender));
  FIZZ_RETURN_ON_ERROR(fizz::detail::write(err, psk.version, appender));
  FIZZ_RETURN_ON_ERROR(fizz::detail::write(err, psk.cipher, appender));
  if (psk.group.has_value()) {
    FIZZ_RETURN_ON_ERROR(
        fizz::detail::write(err, static_cast<uint8_t>(1), appender));
    FIZZ_RETURN_ON_ERROR(fizz::detail::write(err, *psk.group, appender));
  } else {
    FIZZ_RETURN_ON_ERROR(
        fizz::detail::write(err, static_cast<uint8_t>(0), appender));
  }
  FIZZ_RETURN_ON_ERROR(
      fizz::detail::writeBuf<uint8_t>(
          err,
          psk.alpn ? folly::IOBuf::wrapBuffer(StringPiece(*psk.alpn)) : nullptr,
          appender));
  FIZZ_RETURN_ON_ERROR(fizz::detail::write(err, psk.ticketAgeAdd, appender));
  FIZZ_RETURN_ON_ERROR(fizz::detail::write(err, ticketIssueTime, appender));
  FIZZ_RETURN_ON_ERROR(
      fizz::detail::write(err, ticketExpirationTime, appender));
  FIZZ_RETURN_ON_ERROR(
      tryWriteCert(err, serializer, psk.serverCert.get(), appender));
  FIZZ_RETURN_ON_ERROR(
      tryWriteCert(err, serializer, psk.clientCert.get(), appender));
  FIZZ_RETURN_ON_ERROR(
      fizz::detail::write(err, psk.maxEarlyDataSize, appender));
  FIZZ_RETURN_ON_ERROR(fizz::detail::write(err, ticketHandshakeTime, appender));

  ret = serialized->to<std::string>();
  return Status::Success;
}

Status deserializePsk(
    fizz::client::CachedPsk& ret,
    Error& err,
    const CertificateSerialization& serializer,
    folly::ByteRange serializedPsk) {
  auto buf = IOBuf::wrapBuffer(serializedPsk.data(), serializedPsk.size());
  io::Cursor cursor(buf.get());
  fizz::client::CachedPsk psk;
  psk.type = fizz::PskType::Resumption;

  std::unique_ptr<IOBuf> pskData;
  fizz::detail::readBuf<uint16_t>(pskData, cursor);
  psk.psk = pskData->to<std::string>();

  std::unique_ptr<IOBuf> secretData;
  fizz::detail::readBuf<uint16_t>(secretData, cursor);
  psk.secret = secretData->to<std::string>();

  size_t len;
  FIZZ_RETURN_ON_ERROR(fizz::detail::read(len, err, psk.version, cursor));
  FIZZ_RETURN_ON_ERROR(fizz::detail::read(len, err, psk.cipher, cursor));
  uint8_t hasGroup;
  FIZZ_RETURN_ON_ERROR(fizz::detail::read(len, err, hasGroup, cursor));
  if (hasGroup == 1) {
    fizz::NamedGroup group;
    FIZZ_RETURN_ON_ERROR(fizz::detail::read(len, err, group, cursor));
    psk.group = group;
  }

  std::unique_ptr<IOBuf> alpnData;
  fizz::detail::readBuf<uint8_t>(alpnData, cursor);
  if (!alpnData->empty()) {
    psk.alpn = alpnData->to<std::string>();
  }

  FIZZ_RETURN_ON_ERROR(fizz::detail::read(len, err, psk.ticketAgeAdd, cursor));

  uint64_t ticketIssueTime;
  FIZZ_RETURN_ON_ERROR(fizz::detail::read(len, err, ticketIssueTime, cursor));
  psk.ticketIssueTime =
      clampTimePoint<std::chrono::milliseconds>(ticketIssueTime);

  uint64_t ticketExpirationTime;
  FIZZ_RETURN_ON_ERROR(
      fizz::detail::read(len, err, ticketExpirationTime, cursor));
  psk.ticketExpirationTime =
      clampTimePoint<std::chrono::seconds>(ticketExpirationTime);

  psk.serverCert = tryReadCert(serializer, cursor);
  psk.clientCert = tryReadCert(serializer, cursor);

  FIZZ_RETURN_ON_ERROR(
      fizz::detail::read(len, err, psk.maxEarlyDataSize, cursor));

  if (!cursor.isAtEnd()) {
    uint64_t ticketHandshakeTime;
    FIZZ_RETURN_ON_ERROR(
        fizz::detail::read(len, err, ticketHandshakeTime, cursor));
    psk.ticketHandshakeTime =
        clampTimePoint<std::chrono::milliseconds>(ticketHandshakeTime);
  } else {
    // Just assign it now();
    psk.ticketHandshakeTime = std::chrono::system_clock::now();
  }

  ret = std::move(psk);
  return Status::Success;
}

} // namespace client
} // namespace fizz
