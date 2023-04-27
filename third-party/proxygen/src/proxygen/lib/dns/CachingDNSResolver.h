/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <vector>

#include <folly/container/EvictingCacheMap.h>
#include <proxygen/lib/utils/Time.h>

#include "proxygen/lib/dns/DNSResolver.h"

namespace proxygen {

class CachingDNSResolver : public DNSResolver {
 public:
  using UniquePtr =
      std::unique_ptr<CachingDNSResolver, CachingDNSResolver::Destructor>;

  template <typename... Args>
  static UniquePtr newResolver(Args&&... args) {
    return UniquePtr(new CachingDNSResolver(std::forward<Args>(args)...));
  }

  explicit CachingDNSResolver(DNSResolver::UniquePtr resolver,
                              size_t cacheMaxSize = 4096,
                              size_t cacheClearSize = 256,
                              size_t staleCacheSizeMultiplier = 4,
                              size_t staleCacheTTLMin = 24 * 60 * 60,
                              size_t staleCacheTTLScale = 3,
                              std::unique_ptr<TimeUtil> timeUtil =
                                  std::unique_ptr<TimeUtil>(new TimeUtil()))
      : resolver_(std::move(resolver)),
        cache_(cacheMaxSize, cacheClearSize),
        staleCache_(cacheMaxSize * staleCacheSizeMultiplier, cacheClearSize),
        staleCacheTTLMin_(staleCacheTTLMin),
        staleCacheTTLScale_(staleCacheTTLScale),
        timeUtil_(std::move(timeUtil)) {
  }

  ~CachingDNSResolver() override {
    // destroy resolver_ before others becasue it will fail all pending
    // requests and trigger callbacks to this class
    cache_.clear();
    staleCache_.clear();
    resolver_.reset();
  }

  void resolveHostname(
      DNSResolver::ResolutionCallback* cb,
      const std::string& name,
      std::chrono::milliseconds timeout = std::chrono::milliseconds(100),
      sa_family_t family = AF_INET,
      TraceEventContext teContext = TraceEventContext()) override;

  // passthroughs
  void resolveAddress(DNSResolver::ResolutionCallback* cb,
                      const folly::SocketAddress& address,
                      std::chrono::milliseconds timeout =
                          std::chrono::milliseconds(100)) override {
    resolver_->resolveAddress(cb, address, timeout);
  }

  void resolveMailExchange(ResolutionCallback* cb,
                           const std::string& domain,
                           std::chrono::milliseconds timeout) override {
    resolver_->resolveMailExchange(cb, domain, timeout);
  }

  void setStatsCollector(DNSResolver::StatsCollector* statsCollector) override {
    statsCollector_ = statsCollector;
    resolver_->setStatsCollector(statsCollector);
  }

  DNSResolver::StatsCollector* getStatsCollector() const override {
    return statsCollector_;
  }

  std::list<folly::SocketAddress> getResolverAddresses(
      const std::string& hostname) const override {
    return resolver_->getResolverAddresses(hostname);
  }

  struct CacheEntry {
    // The answers are stored in a set to aid in removing duplicates when we
    // merge in answers from the old set. We explicitly shuffle the results
    // when we return them from the cache.
    std::set<Answer> answers_;
    TimePoint baseTime_;
  };

  using DNSCache = folly::EvictingCacheMap<std::string, CacheEntry>;

  // no locking, must run in eventbase thread
  void dumpDNSCache(std::vector<std::pair<std::string, CacheEntry>>& results,
                    DNSCache& cache);

  bool searchDNSCache(std::string name, CacheEntry& out, DNSCache& cache);

  DNSCache& getDNSCache() {
    return cache_;
  }

  DNSCache& getStaleDNSCache() {
    return staleCache_;
  }

  void flushDNSCache();

 protected:
  void insertCache(
      std::string name,
      const std::vector<Answer>& answers,
      DNSCache& cache,
      const std::function<std::chrono::seconds(std::chrono::seconds)>& pf);

  void addToCache(std::string name, const std::vector<Answer>& answers);

  void addToStaleCache(std::string name, const std::vector<Answer>& answers);

  // DNSCache is a LRUCacheMap, so do not use const& here
  void searchCache(std::string name,
                   sa_family_t family,
                   std::vector<Answer>& answers,
                   DNSCache& cache);

  void lookupCache(std::string name,
                   sa_family_t family,
                   std::vector<Answer>& answers) {
    searchCache(name, family, answers, cache_);
  }

  void lookupStaleCache(std::string name,
                        sa_family_t family,
                        std::vector<Answer>& answers) {
    searchCache(name, family, answers, staleCache_);
  }

  class Query
      : public DNSResolver::ResolutionCallback
      , public DNSResolver::QueryBase {
   public:
    explicit Query(ResolutionCallback* cb,
                   const std::string name,
                   sa_family_t family,
                   CachingDNSResolver* parent)
        : cb_(cb), name_(name), family_(family), parent_(parent) {
    }

    void resolutionSuccess(std::vector<Answer> answers) noexcept override {
      if (!answers.empty()) {
        parent_->addToCache(name_, answers);
        parent_->addToStaleCache(name_, answers);
      }

      if (cb_) {
        cb_->eraseQuery(this);
        cb_->resolutionSuccess(answers);
      }
      delete this;
    }

    void resolutionError(const folly::exception_wrapper& ew) noexcept override {
      std::vector<Answer> answers;
      parent_->lookupStaleCache(name_, family_, answers);

      if (!answers.empty()) {
        auto statsCollector = parent_->getStatsCollector();
        if (statsCollector) {
          statsCollector->recordStaleCacheHit();
        }

        if (cb_) {
          cb_->eraseQuery(this);
          cb_->resolutionSuccess(answers);
        }
      } else {
        if (cb_) {
          cb_->eraseQuery(this);
          cb_->resolutionError(ew);
        }
      }
      delete this;
    }

    void cancelResolutionImpl() override {
      cb_ = nullptr;
    }

   private:
    // current request
    ResolutionCallback* cb_;
    std::string name_;
    sa_family_t family_;
    CachingDNSResolver* parent_;
  };

  DNSResolver::UniquePtr resolver_;
  DNSCache cache_, staleCache_;
  size_t staleCacheTTLMin_, staleCacheTTLScale_;
  DNSResolver::StatsCollector* statsCollector_{nullptr};
  std::unique_ptr<TimeUtil> timeUtil_;
};

} // namespace proxygen
