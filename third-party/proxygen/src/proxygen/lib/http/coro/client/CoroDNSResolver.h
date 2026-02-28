/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>
#include <folly/coro/Task.h>

#include <proxygen/lib/dns/DNSResolver.h>

#include <string>

namespace proxygen::coro {

/**
 * Asynchronously reoslve a hostname to an IP address.
 */
struct CoroDNSResolver {
  struct Result {
    folly::SocketAddress primary;
    folly::Optional<folly::SocketAddress> fallback;
  };

  /**
   * for both ::resolveHost & ::resolveHostAll – 
   *
   * resolves hostname using the DNSResolver obtained from the global DNSModule
   * if the resolver argument is nullptr, otherwise uses resolver
   */
  static folly::coro::Task<Result> resolveHost(
      folly::EventBase* evb,
      std::string hostname,
      std::chrono::milliseconds timeout,
      DNSResolver* resolver = nullptr);

  static folly::coro::Task<std::vector<folly::SocketAddress>> resolveHostAll(
      folly::EventBase* evb,
      std::string hostname,
      std::chrono::milliseconds timeout,
      DNSResolver* resolver = nullptr);

  static void emplaceStatsCollector(
      folly::EventBase* evb, std::unique_ptr<DNSResolver::StatsCollector>);

  static void resetDNSResolverInstance(folly::EventBase* evb,
                                       DNSResolver::UniquePtr ptr);
};
} // namespace proxygen::coro
