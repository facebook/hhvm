/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <thread>

#include "proxygen/lib/dns/DNSResolver.h"

namespace proxygen {

/**
 * SyncDNSResolver provides a synchronous interface around the async
 * DNSResolver. Internally it runs a separate thread running event loop
 * and does all the DNS resolution there.
 *
 * Use this if you want timeout functionality, etc or if you are doing a lot
 * of lookups and want to avoid hitting the bug in `getaddrinfo` described
 * here -
 *
 *   https://sourceware.org/bugzilla/show_bug.cgi?id=15946
 */
class SyncDNSResolver {
 public:
  SyncDNSResolver();
  explicit SyncDNSResolver(DNSResolver::UniquePtr resolver);
  ~SyncDNSResolver();

  std::vector<folly::SocketAddress> resolveHostname(
      const std::string& hostname,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(100),
      sa_family_t family = AF_UNSPEC,
      bool rfc6724sort = true);

  // Helpful while testing
  folly::EventBase* getEventBase() {
    return &evb_;
  }

 private:
  std::thread thread_;
  folly::EventBase evb_;
  DNSResolver::UniquePtr resolver_;
};

} // namespace proxygen
