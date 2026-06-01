/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/SocketAddress.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Task.h>

#include <proxygen/lib/dns/DNSResolver.h>

#include <string>

namespace proxygen::coro {

/**
 * Stack-allocated callback for bridging callback-based DNSResolver to
 * coroutine suspension. Stores raw Answer objects so callers can extract
 * addresses or populate caches as needed.
 */
class BatonResolutionCallback : public DNSResolver::ResolutionCallback {
 public:
  explicit BatonResolutionCallback(folly::coro::Baton& baton);

  void resolutionSuccess(
      std::vector<DNSResolver::Answer> ans) noexcept override;

  void resolutionError(const folly::exception_wrapper& exp) noexcept override;

  /**
   * Extract SocketAddress entries from answers, sorted via RFC 6724
   * (falling back to unsorted on sort failure). May return empty.
   */
  [[nodiscard]] std::vector<folly::SocketAddress> extractAddresses() const;

  std::vector<DNSResolver::Answer> answers;
  folly::exception_wrapper exception;

 private:
  folly::coro::Baton& baton_;
};

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
