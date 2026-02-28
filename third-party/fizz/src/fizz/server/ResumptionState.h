/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/Types.h>
#include <fizz/record/Types.h>
#include <chrono>

namespace fizz {
namespace server {

struct ResumptionState {
  ProtocolVersion version;
  CipherSuite cipher;
  Buf resumptionSecret;
  std::shared_ptr<const Cert> serverCert;
  std::shared_ptr<const Cert> clientCert;

  folly::Optional<std::string> alpn;
  uint32_t ticketAgeAdd;
  std::chrono::system_clock::time_point ticketIssueTime;
  Buf appToken;
  std::chrono::system_clock::time_point handshakeTime;
};
} // namespace server
} // namespace fizz
