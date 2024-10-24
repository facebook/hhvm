// Copyright 2004-present Facebook. All Rights Reserved.

#include "fizz/client/PskSerializationUtils.h"

#include <fizz/record/Types.h>
#include <folly/io/IOBuf.h>

using namespace folly;

namespace fizz {

namespace {

void tryWriteCert(
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
    fizz::detail::writeBuf<uint32_t>(serializedCert, appender);
  } else {
    fizz::detail::writeBuf<uint32_t>(nullptr, appender);
  }
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

std::string serializePsk(
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
  fizz::detail::writeBuf<uint16_t>(
      folly::IOBuf::wrapBuffer(StringPiece(psk.psk)), appender);
  fizz::detail::writeBuf<uint16_t>(
      folly::IOBuf::wrapBuffer(StringPiece(psk.secret)), appender);
  fizz::detail::write(psk.version, appender);
  fizz::detail::write(psk.cipher, appender);
  if (psk.group.has_value()) {
    fizz::detail::write(static_cast<uint8_t>(1), appender);
    fizz::detail::write(*psk.group, appender);
  } else {
    fizz::detail::write(static_cast<uint8_t>(0), appender);
  }
  fizz::detail::writeBuf<uint8_t>(
      psk.alpn ? folly::IOBuf::wrapBuffer(StringPiece(*psk.alpn)) : nullptr,
      appender);
  fizz::detail::write(psk.ticketAgeAdd, appender);
  fizz::detail::write(ticketIssueTime, appender);
  fizz::detail::write(ticketExpirationTime, appender);
  tryWriteCert(serializer, psk.serverCert.get(), appender);
  tryWriteCert(serializer, psk.clientCert.get(), appender);
  fizz::detail::write(psk.maxEarlyDataSize, appender);
  fizz::detail::write(ticketHandshakeTime, appender);

  return serialized->to<std::string>();
}

fizz::client::CachedPsk deserializePsk(
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

  fizz::detail::read(psk.version, cursor);
  fizz::detail::read(psk.cipher, cursor);
  uint8_t hasGroup;
  fizz::detail::read(hasGroup, cursor);
  if (hasGroup == 1) {
    fizz::NamedGroup group;
    fizz::detail::read(group, cursor);
    psk.group = group;
  }

  std::unique_ptr<IOBuf> alpnData;
  fizz::detail::readBuf<uint8_t>(alpnData, cursor);
  if (!alpnData->empty()) {
    psk.alpn = alpnData->to<std::string>();
  }

  fizz::detail::read(psk.ticketAgeAdd, cursor);

  uint64_t ticketIssueTime;
  fizz::detail::read(ticketIssueTime, cursor);
  psk.ticketIssueTime =
      clampTimePoint<std::chrono::milliseconds>(ticketIssueTime);

  uint64_t ticketExpirationTime;
  fizz::detail::read(ticketExpirationTime, cursor);
  psk.ticketExpirationTime =
      clampTimePoint<std::chrono::seconds>(ticketExpirationTime);

  psk.serverCert = tryReadCert(serializer, cursor);
  psk.clientCert = tryReadCert(serializer, cursor);

  fizz::detail::read(psk.maxEarlyDataSize, cursor);

  if (!cursor.isAtEnd()) {
    uint64_t ticketHandshakeTime;
    fizz::detail::read(ticketHandshakeTime, cursor);
    psk.ticketHandshakeTime =
        clampTimePoint<std::chrono::milliseconds>(ticketHandshakeTime);
  } else {
    // Just assign it now();
    psk.ticketHandshakeTime = std::chrono::system_clock::now();
  }

  return psk;
}

} // namespace client
} // namespace fizz
