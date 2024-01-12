// Copyright 2004-present Facebook. All Rights Reserved.

#include "fizz/client/PskSerializationUtils.h"

#include <fizz/record/Types.h>
#include <folly/ssl/OpenSSLCertUtils.h>

using namespace folly;

namespace fizz {

namespace {

void tryWriteCert(const fizz::Cert* cert, io::Appender& appender) {
  if (auto opensslCert = dynamic_cast<const fizz::OpenSSLCert*>(cert)) {
    auto x509 = opensslCert->getX509();
    fizz::detail::writeBuf<uint32_t>(
        x509 ? folly::ssl::OpenSSLCertUtils::derEncode(*x509) : nullptr,
        appender);
  } else {
    fizz::detail::writeBuf<uint32_t>(nullptr, appender);
  }
}

} // namespace

namespace client {

std::string serializePsk(const fizz::client::CachedPsk& psk) {
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
  tryWriteCert(psk.serverCert.get(), appender);
  tryWriteCert(psk.clientCert.get(), appender);
  fizz::detail::write(psk.maxEarlyDataSize, appender);
  fizz::detail::write(ticketHandshakeTime, appender);

  return serialized->moveToFbString().toStdString();
}

fizz::client::CachedPsk deserializePsk(
    const std::string& str,
    const fizz::Factory& factory) {
  auto buf = IOBuf::wrapBuffer(str.data(), str.length());
  io::Cursor cursor(buf.get());
  fizz::client::CachedPsk psk;
  psk.type = fizz::PskType::Resumption;

  std::unique_ptr<IOBuf> pskData;
  fizz::detail::readBuf<uint16_t>(pskData, cursor);
  psk.psk = pskData->moveToFbString().toStdString();

  std::unique_ptr<IOBuf> secretData;
  fizz::detail::readBuf<uint16_t>(secretData, cursor);
  psk.secret = secretData->moveToFbString().toStdString();

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
    psk.alpn = alpnData->moveToFbString().toStdString();
  }

  fizz::detail::read(psk.ticketAgeAdd, cursor);

  uint64_t ticketIssueTime;
  fizz::detail::read(ticketIssueTime, cursor);
  psk.ticketIssueTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::milliseconds(ticketIssueTime));

  uint64_t ticketExpirationTime;
  fizz::detail::read(ticketExpirationTime, cursor);
  psk.ticketExpirationTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::seconds(ticketExpirationTime));

  CertificateEntry entry;
  fizz::detail::readBuf<uint32_t>(entry.cert_data, cursor);
  if (!entry.cert_data->empty()) {
    psk.serverCert = factory.makePeerCert(std::move(entry), true);
  }

  CertificateEntry clientEntry;
  fizz::detail::readBuf<uint32_t>(clientEntry.cert_data, cursor);
  if (!clientEntry.cert_data->empty()) {
    psk.clientCert = factory.makePeerCert(std::move(clientEntry), true);
  }

  fizz::detail::read(psk.maxEarlyDataSize, cursor);

  if (!cursor.isAtEnd()) {
    uint64_t ticketHandshakeTime;
    fizz::detail::read(ticketHandshakeTime, cursor);
    psk.ticketHandshakeTime =
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(ticketHandshakeTime));
  } else {
    // Just assign it now();
    psk.ticketHandshakeTime = std::chrono::system_clock::now();
  }

  return psk;
}

} // namespace client
} // namespace fizz
