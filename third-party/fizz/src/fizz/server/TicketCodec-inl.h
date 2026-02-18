/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

namespace fizz {
namespace server {

// out-of-line definition of constexpr static data member is redundant in C++17
// and is deprecated
#if __cplusplus < 201703L
template <CertificateStorage Storage>
constexpr folly::StringPiece TicketCodec<Storage>::Label;
#endif // __cplusplus < 201703L

template <CertificateStorage Storage>
Buf TicketCodec<Storage>::encode(ResumptionState resState) {
  Buf selfIdentity = folly::IOBuf::create(0);
  if (resState.serverCert) {
    selfIdentity = folly::IOBuf::copyBuffer(resState.serverCert->getIdentity());
  }

  uint64_t ticketIssueTime = std::chrono::duration_cast<std::chrono::seconds>(
                                 resState.ticketIssueTime.time_since_epoch())
                                 .count();

  auto buf = folly::IOBuf::create(60);
  folly::io::Appender appender(buf.get(), 60);

  Error err;
  FIZZ_THROW_ON_ERROR(
      fizz::detail::write(err, resState.version, appender), err);
  FIZZ_THROW_ON_ERROR(fizz::detail::write(err, resState.cipher, appender), err);
  FIZZ_THROW_ON_ERROR(
      fizz::detail::writeBuf<uint16_t>(
          err, resState.resumptionSecret, appender),
      err);
  FIZZ_THROW_ON_ERROR(
      fizz::detail::writeBuf<uint16_t>(err, selfIdentity, appender), err);
  appendClientCertificate(Storage, resState.clientCert, appender);
  FIZZ_THROW_ON_ERROR(
      fizz::detail::write(err, resState.ticketAgeAdd, appender), err);
  FIZZ_THROW_ON_ERROR(fizz::detail::write(err, ticketIssueTime, appender), err);
  if (resState.alpn) {
    auto alpnBuf = folly::IOBuf::copyBuffer(*resState.alpn);
    FIZZ_THROW_ON_ERROR(
        fizz::detail::writeBuf<uint8_t>(err, alpnBuf, appender), err);
  } else {
    FIZZ_THROW_ON_ERROR(
        fizz::detail::writeBuf<uint8_t>(err, nullptr, appender), err);
  }
  FIZZ_THROW_ON_ERROR(
      fizz::detail::writeBuf<uint16_t>(err, resState.appToken, appender), err);
  uint64_t handshakeTime = std::chrono::duration_cast<std::chrono::seconds>(
                               resState.handshakeTime.time_since_epoch())
                               .count();
  FIZZ_THROW_ON_ERROR(fizz::detail::write(err, handshakeTime, appender), err);
  return buf;
}

template <CertificateStorage Storage>
ResumptionState TicketCodec<Storage>::decode(
    Buf encoded,
    const Factory& factory,
    const CertManager& certManager) {
  folly::io::Cursor cursor(encoded.get());

  ResumptionState resState;
  Error err;
  size_t len;
  FIZZ_THROW_ON_ERROR(
      fizz::detail::read(len, err, resState.version, cursor), err);
  FIZZ_THROW_ON_ERROR(
      fizz::detail::read(len, err, resState.cipher, cursor), err);
  fizz::detail::readBuf<uint16_t>(resState.resumptionSecret, cursor);
  Buf selfIdentity;
  fizz::detail::readBuf<uint16_t>(selfIdentity, cursor);

  resState.clientCert = readClientCertificate(cursor, factory);

  FIZZ_THROW_ON_ERROR(
      fizz::detail::read(len, err, resState.ticketAgeAdd, cursor), err);
  uint64_t seconds;
  FIZZ_THROW_ON_ERROR(fizz::detail::read(len, err, seconds, cursor), err);
  Buf alpnBuf;
  fizz::detail::readBuf<uint8_t>(alpnBuf, cursor);
  if (!alpnBuf->empty()) {
    resState.alpn = alpnBuf->to<std::string>();
  }

  resState.ticketIssueTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::seconds(seconds));
  // If unset, set handshake timestamp to ticket timestamp
  resState.handshakeTime = resState.ticketIssueTime;

  resState.serverCert = certManager.getCert(selfIdentity->to<std::string>());

  if (cursor.isAtEnd()) {
    return resState;
  }
  fizz::detail::readBuf<uint16_t>(resState.appToken, cursor);

  if (cursor.isAtEnd()) {
    return resState;
  }
  FIZZ_THROW_ON_ERROR(fizz::detail::read(len, err, seconds, cursor), err);
  resState.handshakeTime = std::chrono::time_point<std::chrono::system_clock>(
      std::chrono::seconds(seconds));

  return resState;
}
} // namespace server
} // namespace fizz
