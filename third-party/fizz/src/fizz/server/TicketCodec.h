/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <fizz/server/FizzServerContext.h>
#include <fizz/server/ResumptionState.h>

namespace fizz {
namespace server {
enum class CertificateStorage : uint8_t {
  None = 0,
  X509 = 1,
  IdentityOnly = 2
};
}

std::string toString(server::CertificateStorage storage);

namespace server {

void appendClientCertificate(
    CertificateStorage storage,
    const std::shared_ptr<const Cert>& cert,
    folly::io::Appender& appender);

std::shared_ptr<const Cert> readClientCertificate(
    folly::io::Cursor& cursor,
    const Factory& factory);

template <CertificateStorage Storage>
struct TicketCodec {
  /**
   * This label can be used to derive the encryption key and should be changed
   * whenever the encoding changes so previous tickets are invalidated.
   */
  static constexpr folly::StringPiece Label{"Fizz Ticket Codec v2"};

  static Buf encode(ResumptionState state);

  static ResumptionState
  decode(Buf encoded, const Factory& factory, const CertManager& certManager);
};
} // namespace server
} // namespace fizz

#include <fizz/server/TicketCodec-inl.h>
