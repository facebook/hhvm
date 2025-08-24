/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>

#include "proxygen/lib/dns/DNSModule.h"
#include "proxygen/lib/dns/DNSResolver.h"
#include "proxygen/lib/dns/Rfc6724.h"
#include "proxygen/lib/http/coro/client/CoroDNSResolver.h"
#include <folly/coro/Baton.h>
#include <folly/io/async/EventBaseLocal.h>

using folly::coro::co_error;
using folly::coro::co_nothrow;

namespace {
using namespace proxygen;

using StatsCollector = DNSResolver::StatsCollector;
static folly::EventBaseLocal<DNSResolver::UniquePtr> resolver_;
static folly::EventBaseLocal<std::unique_ptr<StatsCollector>> stats_;

bool hasDNSResolverInstance(folly::EventBase* evb) {
  return resolver_.get(*evb) != nullptr;
}

bool hasStatsCollector(folly::EventBase* evb) {
  return stats_.get(*evb);
}

StatsCollector* getStatsCollector(folly::EventBase* evb) {
  XCHECK(hasStatsCollector(evb));
  return stats_.get(*evb)->get();
}

DNSResolver::UniquePtr* getDNSResolverInstance(folly::EventBase* evb) {
  if (hasDNSResolverInstance(evb)) {
    return resolver_.get(*evb);
  }

  if (auto dnsModule = DNSModule::get()) {
    auto resolver = dnsModule->provideDNSResolver(evb);
    if (hasStatsCollector(evb)) {
      resolver->setStatsCollector(getStatsCollector(evb));
    }
    resolver_.try_emplace(*evb, std::move(resolver));
  }

  // returns nullptr if DNSModule::get() == nullptr (i.e. on shutdown)
  return resolver_.get(*evb);
}

class CoroResolutionCallback : public DNSResolver::ResolutionCallback {
 public:
  explicit CoroResolutionCallback(folly::coro::Baton& baton) : baton_(baton) {
  }

  void resolutionSuccess(
      std::vector<DNSResolver::Answer> ans) noexcept override {
    for (const auto& answer : ans) {
      if (answer.type == DNSResolver::Answer::AT_ADDRESS) {
        addrs.push_back(answer.address);
      }
    }
    if (addrs.empty()) {
      exception = DNSResolver::makeNoNameException();
    } else {
      // fallback to unsorted addresses if rfc6724_sort throws an exception
      folly::makeTryWith([&]() { rfc6724_sort(addrs); });
    }

    baton_.post();
  }
  void resolutionError(const folly::exception_wrapper& exp) noexcept override {
    exception = exp;
    baton_.post();
  }

  std::vector<folly::SocketAddress> addrs;
  folly::exception_wrapper exception;

 private:
  folly::coro::Baton& baton_;
};

} // namespace

namespace proxygen::coro {

void CoroDNSResolver::emplaceStatsCollector(
    folly::EventBase* evb, std::unique_ptr<StatsCollector> statsCollector) {
  stats_.try_emplace(*evb, std::move(statsCollector));
}

void CoroDNSResolver::resetDNSResolverInstance(folly::EventBase* evb,
                                               DNSResolver::UniquePtr ptr) {
  if (hasDNSResolverInstance(evb)) {
    resolver_.erase(*evb);
  }
  resolver_.try_emplace(*evb, std::move(ptr));
}

folly::coro::Task<std::vector<folly::SocketAddress>>
CoroDNSResolver::resolveHostAll(folly::EventBase* evb,
                                std::string hostname,
                                std::chrono::milliseconds timeout,
                                DNSResolver* resolver) {

  folly::coro::Baton baton;
  CoroResolutionCallback cb(baton);

  if (resolver) {
    resolver->resolveHostname(&cb, hostname, timeout, AF_UNSPEC);
  } else if (auto* res = getDNSResolverInstance(evb)) {
    res->get()->resolveHostname(&cb, hostname, timeout, AF_UNSPEC);
  } else {
    // fail dns request during shutdown; signal baton to avoid blocking
    cb.exception = folly::make_exception_wrapper<DNSResolver::Exception>(
        DNSResolver::SHUTDOWN, "shutting down");
    baton.post();
  }

  co_await baton;

  if (cb.exception) {
    co_yield co_error(cb.exception);
  }
  XCHECK(!cb.addrs.empty());
  co_return cb.addrs;
}

folly::coro::Task<CoroDNSResolver::Result> CoroDNSResolver::resolveHost(
    folly::EventBase* evb,
    std::string hostname,
    std::chrono::milliseconds timeout,
    DNSResolver* resolver) {
  auto addrs = co_await co_nothrow(CoroDNSResolver::resolveHostAll(
      evb, std::move(hostname), timeout, resolver));

  CoroDNSResolver::Result result;
  result.primary = std::move(addrs.front());
  for (auto it = addrs.begin() + 1; it != addrs.end(); ++it) {
    if (it->getFamily() != result.primary.getFamily()) {
      result.fallback = std::move(*it);
      break;
    }
  }
  co_return result;
}

} // namespace proxygen::coro
