/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <proxygen/lib/dns/DNSResolver.h>
#include <proxygen/lib/stats/BaseStats.h>

namespace proxygen {

/**
 * A DNSResolver::StatsCollector instance that uses thread-local stats for
 * aggregation and reporting.
 *
 * DNS Stats description:
 * ----------------------
 *
 * 1) DNS Cache stats:
 * - cache_hit: Lookup successful in the DNS cache
 *
 * - cache_miss: Entry not present in the cache
 *
 * - cache_partial_miss: Entry present for the required family, but is expired
 *
 * - stale_cache_hit: Lookup performed successfully from the stale cache
 *
 * 2) DNS lookups:
 * - reqs: Total outbound DNS lookups. These are request which werent handled by
 *   the DNS cache. Typeicaly reqs = cache_miss + cache_partial_miss
 *
 * - success: Answers received successfuly from the DNS servers.
 *
 * - nodata: DNS server replied, but with no answer. This can happen if the
 *   client provided wrong hostname, or if the DNS servers were misconfigured.
 *   We use the status codes ARES_ENODATA (DNS server returned answer with
 *   no data) and ARES_ENOTFOUND (Domain name not found), to identify
 *   these responses.
 *
 * - error: Any other errors during DNS lookup.
 *
 * 3) Individual DNS queries:
 * - rcode.(0-15): The lookups could be split into individual DNS queries for
 *   A and AAAA records. These stats describe the return codes of the
 *   individual queries.
 *
 */
class AsyncDNSStatsCollector : public DNSResolver::StatsCollector {
 public:
  explicit AsyncDNSStatsCollector(const std::string& prefix);
  ~AsyncDNSStatsCollector() override {
  }

  // DNSResolver::StatsCollector
  void recordSuccess(const std::vector<DNSResolver::Answer>& answers,
                     std::chrono::milliseconds latency) noexcept override;
  void recordError(const folly::exception_wrapper& ew,
                   std::chrono::milliseconds latency) noexcept override;
  void recordQueryResult(uint8_t rcode) noexcept override;

  void recordCacheHit() noexcept override {
    cacheHits_.add(1);
  }

  void recordCacheMiss() noexcept override {
    cacheMisses_.add(1);
  }

  void recordCachePartialMiss() noexcept override {
    cachePartialMisses_.add(1);
  }

  void recordStaleCacheHit() noexcept override {
    staleCacheHits_.add(1);
  }

 private:
  BaseStats::TLTimeseries reqs_;
  BaseStats::TLTimeseries success_;
  BaseStats::TLTimeseries error_;
  BaseStats::TLTimeseries timeouts_;
  BaseStats::TLTimeseries nodata_;
  BaseStats::TLTimeseries answersA_;
  BaseStats::TLTimeseries answersAAAA_;
  BaseStats::TLTimeseries answersCNAME_;
  BaseStats::TLHistogram latency_;
  BaseStats::TLHistogram ttl_;
  std::vector<std::unique_ptr<BaseStats::TLTimeseries>> status_;
  std::vector<std::unique_ptr<BaseStats::TLTimeseries>> rcodes_;

  BaseStats::BaseStats::TLTimeseries cacheHits_;
  BaseStats::TLTimeseries cacheMisses_;
  BaseStats::TLTimeseries cachePartialMisses_;
  BaseStats::TLTimeseries staleCacheHits_;

  void recordStatus(DNSResolver::ResolutionStatus status);
};

} // namespace proxygen
