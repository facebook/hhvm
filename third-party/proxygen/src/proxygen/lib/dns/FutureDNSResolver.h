/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <folly/SocketAddress.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>

#include "proxygen/lib/dns/DNSResolver.h"

namespace proxygen {

/**
 * A wrapper around a proxygen::DNSResolver that provides a Future API around
 * the resolveAddress() and resolveHostname() methods.
 */
class FutureDNSResolver {
 public:
  static constexpr std::chrono::milliseconds kDefaultTimeout() {
#if ASAN_ENABLED || TSAN_ENABLED
    return std::chrono::milliseconds(1000);
#else
    return std::chrono::milliseconds(100);
#endif
  }

  FutureDNSResolver(folly::EventBase* evb, DNSResolver::UniquePtr resolver);
  virtual ~FutureDNSResolver();

  virtual folly::Future<std::vector<DNSResolver::Answer>> resolveAddress(
      const folly::SocketAddress& address,
      std::chrono::milliseconds timeout = kDefaultTimeout());

  virtual folly::Future<std::vector<DNSResolver::Answer>> resolveHostname(
      const std::string& name,
      std::chrono::milliseconds timeout = kDefaultTimeout(),
      sa_family_t family = AF_INET,
      proxygen::TraceEventContext teCtx = proxygen::TraceEventContext());

  virtual DNSResolver::StatsCollector* getStatsCollector() const;
  virtual void setStatsCollector(DNSResolver::StatsCollector* collector);

  folly::EventBase* getEventBase() const {
    return evb_;
  }

 private:
  folly::EventBase* evb_;
  std::shared_ptr<DNSResolver> resolver_;
};

} // namespace proxygen
