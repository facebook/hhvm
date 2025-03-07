/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/AsyncDNSStatsCollector.h"

#include <folly/Conv.h>

using facebook::fb303::AVG;
using facebook::fb303::COUNT;
using facebook::fb303::PERCENT;
using facebook::fb303::RATE;
using facebook::fb303::SUM;

namespace {

std::vector<std::string> rcodeNames({
    "NOERROR",
    "NODATA",
    "FORMERR",
    "SERVFAIL",
    "NXDOMAIN",
    "NOTIMP",
    "REFUSED",
});

}

namespace proxygen {

AsyncDNSStatsCollector::AsyncDNSStatsCollector(const std::string& prefix)
    : reqs_(prefix + "req", RATE, SUM),
      success_(prefix + "success", RATE, SUM, PERCENT),
      error_(prefix + "error", RATE, SUM, PERCENT),
      timeouts_(prefix + "timeouts", RATE, SUM, PERCENT),
      nodata_(prefix + "nodata", RATE, SUM, PERCENT),
      answersA_(prefix + "answers.A", SUM, COUNT),
      answersAAAA_(prefix + "answers.AAAA", SUM, COUNT),
      answersCNAME_(prefix + "answers.CNAME", SUM, COUNT),
      latency_(prefix + "latency", 10, 0, 500, AVG, 50, 95, 99),
      ttl_(prefix + "ttl", 900, 0, 21600, AVG, 50, 95, 99),
      status_(static_cast<size_t>(DNSResolver::UNKNOWN) + 1),
      rcodes_(16),
      cacheHits_(prefix + "cache_hit", RATE, SUM),
      cacheMisses_(prefix + "cache_miss", RATE, SUM),
      cachePartialMisses_(prefix + "cache_partial_miss", RATE, SUM),
      staleCacheHits_(prefix + "stale_cache_hit", RATE, SUM) {
  for (uint8_t i = 0; i <= static_cast<size_t>(DNSResolver::UNKNOWN); ++i) {
    DNSResolver::ResolutionStatus status =
        static_cast<DNSResolver::ResolutionStatus>(i);
    status_[i].reset(new BaseStats::TLTimeseries(
        folly::to<std::string>(prefix, "status.", describe(status, false)),
        RATE,
        SUM));
  }
  for (uint8_t i = 0; i < 16; ++i) {
    if (i < rcodeNames.size()) {
      rcodes_[i].reset(new BaseStats::TLTimeseries(
          folly::to<std::string>(prefix, "rcode.", rcodeNames[i]), RATE, SUM));
    } else {
      rcodes_[i].reset(new BaseStats::TLTimeseries(
          folly::to<std::string>(prefix, "rcode.", i), RATE, SUM));
    }
  }
}

void AsyncDNSStatsCollector::recordSuccess(
    const std::vector<DNSResolver::Answer>& answers,
    std::chrono::milliseconds latency) noexcept {
  bool isStale = false;
  for (auto& a : answers) {
    if (a.ttl.count() <= 0) {
      isStale = true;
      break;
    }
  }
  if (!isStale) {
    reqs_.add(1);
    success_.add(1);
  } else {
    reqs_.add(0);
    success_.add(0);
  }
  error_.add(0);
  timeouts_.add(0);
  nodata_.add(0);
  latency_.add(latency.count());
  status_[static_cast<size_t>(DNSResolver::OK)]->add(1);

  size_t countA = 0, countAAAA = 0, countCNAME = 0;
  for (auto& ans : answers) {
    if (ans.type == DNSResolver::Answer::AT_ADDRESS) {
      ttl_.add(ans.ttl.count());

      switch (ans.address.getFamily()) {
        case AF_INET:
          ++countA;
          break;

        case AF_INET6:
          ++countAAAA;
          break;

        default:
          LOG(INFO) << "Ignoring unexpected address family "
                    << ans.address.getFamily();
          break;
      }
    } else if (ans.type == DNSResolver::Answer::AT_CNAME) {
      ttl_.add(ans.ttl.count());
      ++countCNAME;
    }
  }

  if (countA > 0) {
    answersA_.add(countA);
  }

  if (countAAAA > 0) {
    answersAAAA_.add(countAAAA);
  }

  if (countCNAME > 0) {
    answersCNAME_.add(countCNAME);
  }
}

void AsyncDNSStatsCollector::recordError(
    const folly::exception_wrapper& ew,
    std::chrono::milliseconds latency) noexcept {
  DNSResolver::ResolutionStatus status = DNSResolver::UNKNOWN;

  ew.with_exception([&](const DNSResolver::Exception& ex) {
    status = ex.status();
    LOG_EVERY_N(WARNING, 100) << status << " : " << ex.what();
  });

  reqs_.add(1);
  success_.add(0);
  if (status == DNSResolver::NODATA) {
    // Highly probable that non-existent hostnames were used by the client.
    nodata_.add(1);
    error_.add(0);
    timeouts_.add(0);
  } else if (status == DNSResolver::TIMEOUT) {
    // TODO(jls): Timeouts are errors, they shouldn't be ultimately excluded.
    // Restore this to its original state once timeouts in Fwdproxy have been
    // resolved.
    timeouts_.add(1);
    error_.add(0);
    nodata_.add(0);
  } else {
    error_.add(1);
    nodata_.add(0);
    timeouts_.add(0);
  }

  latency_.add(latency.count());
  status_[static_cast<size_t>(status)]->add(1);
}

void AsyncDNSStatsCollector::recordQueryResult(uint8_t rcode) noexcept {
  // RFC1035 defines the range of rcode values as [0, 16).
  if (rcode > 15) {
    LOG(DFATAL) << "Invalid rcode: " << rcode;
    return;
  }

  rcodes_[rcode]->add(1);
}

} // namespace proxygen
