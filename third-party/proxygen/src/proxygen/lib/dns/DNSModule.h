/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Memory.h>

#include "proxygen/lib/dns/CAresResolver.h"
#include "proxygen/lib/dns/CachingDNSResolver.h"

namespace proxygen {

class DNSModule {

 public:
  static std::shared_ptr<DNSModule> get();

  DNSModule();
  virtual ~DNSModule() = default;

  /**
   * @Provides proxygen::DNSResolver::UniquePtr
   */
  virtual DNSResolver::UniquePtr provideDNSResolver(
      folly::EventBase* eventBase) {
    auto cares = CAresResolver::newResolver();

    cares->attachEventBase(eventBase);
    cares->setPort(dnsPort_);
    cares->setServers(dnsServers_);
    cares->init();

    auto resolver =
        CachingDNSResolver::newResolver(DNSResolver::UniquePtr(cares.release()),
                                        cacheMaxSize_,
                                        cacheClearSize_);

    return DNSResolver::UniquePtr(resolver.release());
  }

  /**
   * Configure the provided DNS resolver to make requests to the given port.
   */
  void setDNSPort(uint16_t port) {
    dnsPort_ = port;
  }

  /**
   * Configure the provided DNS resolver to make requests to the given set of
   * servers. Any port specified in these addresses is ignored in favor of the
   * port set via setDNSPort().
   */
  void setDNSServers(const std::list<folly::SocketAddress>& servers) {
    dnsServers_.clear();
    dnsServers_.insert(dnsServers_.begin(), servers.begin(), servers.end());
  }

  /**
   * Configure the parameters of the CachingDNSResolver cache
   * Note: CachingDNSResolver is one instance per Acceptor
   */
  void setCacheParameters(size_t maxSize, size_t clearSize) {
    cacheMaxSize_ = maxSize;
    cacheClearSize_ = clearSize;
  }

  /**
   * Configure the TTL of entries in CachingDNSResolver staleCache_
   * Note: TTL = max(staleCacheTTLMin, staleCacheTTLScale * TTL)
   */
  void setStaleCacheTTL(size_t min, size_t factor) {
    staleCacheTTLMin_ = min;
    staleCacheTTLScale_ = factor;
  }

  void setStaleCacheSizeMultiplier(size_t multiplier) {
    staleCacheSizeMultiplier_ = multiplier;
  }

 protected:
  uint16_t dnsPort_{53};
  std::list<folly::SocketAddress> dnsServers_;
  size_t cacheMaxSize_{4096};
  size_t cacheClearSize_{256};
  size_t staleCacheSizeMultiplier_{4};
  size_t staleCacheTTLMin_{24 * 60 * 60}; // by default 24 hours;
  size_t staleCacheTTLScale_{3};          // by default 3 times of TTL;
};

} // namespace proxygen
