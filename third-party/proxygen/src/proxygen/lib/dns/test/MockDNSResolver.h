/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <stdexcept>

#include "proxygen/lib/dns/DNSResolver.h"

namespace proxygen {

class MockDNSResolver : public proxygen::DNSResolver {
 public:
  class MockQueryBase : public DNSResolver::QueryBase {
   public:
    MOCK_METHOD(void, cancelResolutionImpl, ());
  };

  MOCK_METHOD(void,
              resolveAddress,
              (proxygen::DNSResolver::ResolutionCallback*,
               const folly::SocketAddress&,
               std::chrono::milliseconds));

  MOCK_METHOD(void,
              resolveHostname,
              (DNSResolver::ResolutionCallback*,
               const std::string&,
               std::chrono::milliseconds,
               sa_family_t,
               TraceEventContext));

  MOCK_METHOD(void,
              resolveMailExchange,
              (DNSResolver::ResolutionCallback*,
               const std::string&,
               std::chrono::milliseconds));

  MOCK_METHOD(void, setStatsCollector, (DNSResolver::StatsCollector*));
  MOCK_METHOD(DNSResolver::StatsCollector*, getStatsCollector, (), (const));
};

} // namespace proxygen
