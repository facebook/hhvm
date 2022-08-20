/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GTest.h>

#include <fizz/client/PskCache.h>

namespace fizz {
namespace client {
namespace test {

CachedPsk getTestPsk(
    std::string pskName,
    std::chrono::system_clock::time_point issueTime) {
  CachedPsk psk;
  psk.psk = std::move(pskName);
  psk.secret = "resumptionsecret";
  psk.type = PskType::Resumption;
  psk.version = ProtocolVersion::tls_1_3;
  psk.cipher = CipherSuite::TLS_AES_128_GCM_SHA256;
  psk.group = NamedGroup::x25519;
  psk.maxEarlyDataSize = 0;
  psk.ticketAgeAdd = 0x11111111;
  psk.ticketIssueTime = issueTime;
  psk.ticketExpirationTime = issueTime + std::chrono::seconds(10);
  psk.ticketHandshakeTime = issueTime - std::chrono::seconds(10);
  psk.alpn = "h2";
  return psk;
}

void pskEq(const CachedPsk& psk1, const CachedPsk& psk2) {
  EXPECT_EQ(psk1.psk, psk2.psk);
  EXPECT_EQ(psk1.secret, psk2.secret);
  EXPECT_EQ(psk1.type, psk2.type);
  EXPECT_EQ(psk1.version, psk2.version);
  EXPECT_EQ(psk1.cipher, psk2.cipher);
  EXPECT_EQ(psk1.group, psk2.group);
  EXPECT_EQ(psk1.maxEarlyDataSize, psk2.maxEarlyDataSize);
  EXPECT_EQ(psk1.ticketAgeAdd, psk2.ticketAgeAdd);
  EXPECT_EQ(psk1.ticketIssueTime, psk2.ticketIssueTime);
  EXPECT_EQ(psk1.ticketExpirationTime, psk2.ticketExpirationTime);
  EXPECT_EQ(psk1.ticketHandshakeTime, psk2.ticketHandshakeTime);
  EXPECT_EQ(psk1.alpn, psk2.alpn);
}
} // namespace test
} // namespace client
} // namespace fizz
