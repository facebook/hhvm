/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Random.h>

#include "proxygen/lib/dns/CachingDNSResolver.h"

using folly::SocketAddress;
using std::function;
using std::max;
using std::chrono::duration;
using std::chrono::seconds;

namespace proxygen {

void CachingDNSResolver::resolveHostname(DNSResolver::ResolutionCallback* cb,
                                         const std::string& name,
                                         std::chrono::milliseconds timeout,
                                         sa_family_t family,
                                         TraceEventContext teContext) {
  std::vector<Answer> results;
  TimePoint now = timeUtil_->now();
  DNSCache::iterator c_iter;

  if ((c_iter = cache_.find(name)) != cache_.end()) {
    CacheEntry& entry = c_iter->second;
    std::set<Answer>& res = entry.answers_;
    bool needQuery = false;       // true if no or only expired answer exists
    bool hasCachedAnswer = false; // with respect to TTR
    bool isPartialMiss = false;
    for (auto& answer : res) {
      if (answer.type != Answer::AnswerType::AT_ADDRESS) {
        continue;
      }

      if (family == AF_UNSPEC || answer.address.getFamily() == family) {
        if (entry.baseTime_ + answer.ttl < now) {
          // expired answer exists and satisfies the family type
          needQuery = true;
          if (answer.ttl.count() > 0) {
            isPartialMiss = true;
          }
        } else {
          Answer ans(answer);
          ans.ttl -= secondsBetween(now, entry.baseTime_);
          results.push_back(std::move(ans));
        }
      }

      hasCachedAnswer |= entry.baseTime_ + answer.ttl >= now;
    }

    if (!hasCachedAnswer) {
      cache_.erase(name);
    }

    needQuery |= results.empty();

    if (!needQuery) {
      if (statsCollector_) {
        statsCollector_->recordCacheHit();
      }

      // The results come from an ordered set, we need to shuffle the list in
      // order to prevent uneven load on the first result:
      if (results.size() > 0) {
        std::shuffle(
            std::begin(results), std::end(results), folly::ThreadLocalPRNG{});
      }
      cb->resolutionSuccess(std::move(results));
      return;
    } else {
      if (isPartialMiss && statsCollector_) {
        statsCollector_->recordCachePartialMiss();
      }
    }
  } else {
    if (statsCollector_) {
      statsCollector_->recordCacheMiss();
    }
  }

  Query* q = new Query(cb, name, family, this);
  cb->insertQuery(q);
  resolver_->resolveHostname(q, name, timeout, family, std::move(teContext));
}

// add the entry into both cache_ and staleCache_ with diff TTL
void CachingDNSResolver::insertCache(
    std::string name,
    const std::vector<Answer>& answers,
    DNSCache& cache,
    const std::function<seconds(seconds)>& pf) {
  CacheEntry newEntry;
  newEntry.baseTime_ = timeUtil_->now();

  // get original answers, prune expired ones and update TTL
  auto iter = cache.find(name);
  if (iter != cache.end()) {
    CacheEntry& entry = iter->second;
    std::set<Answer>& res = entry.answers_;
    for (auto& a : res) {
      if (entry.baseTime_ + a.ttl >= newEntry.baseTime_) {
        Answer ans(a);
        ans.ttl -= secondsBetween(newEntry.baseTime_, entry.baseTime_);
        newEntry.answers_.insert(ans);
      }
    }
  }

  // merge exsting and new answers
  for (auto& i : answers) {
    Answer ans(i);
    ans.ttl = pf(ans.ttl);
    auto rc = newEntry.answers_.insert(ans);
    // update its TTL if the answer already exists
    if (!rc.second && rc.first->ttl < ans.ttl) {
      newEntry.answers_.erase(rc.first);
      newEntry.answers_.insert(ans);
    }
  }

  cache.set(name, std::move(newEntry));
}

void CachingDNSResolver::addToCache(std::string name,
                                    const std::vector<Answer>& answers) {
  insertCache(name, answers, cache_, [&](const seconds ttl) { return ttl; });
}

void CachingDNSResolver::addToStaleCache(std::string name,
                                         const std::vector<Answer>& answers) {
  insertCache(name, answers, staleCache_, [&](const seconds ttl) {
    return seconds(
        max(static_cast<uint64_t>(staleCacheTTLMin_),
            static_cast<uint64_t>(ttl.count() * staleCacheTTLScale_)));
  });
}

void CachingDNSResolver::searchCache(std::string name,
                                     sa_family_t family,
                                     std::vector<Answer>& answers,
                                     DNSCache& cache) {
  TimePoint now = timeUtil_->now();
  auto iter = cache.find(name);
  if (iter != cache.end()) {
    CacheEntry& entry = iter->second;
    std::set<Answer>& res = entry.answers_;
    for (auto& answer : res) {
      if (answer.type != Answer::AnswerType::AT_ADDRESS) {
        continue;
      }

      if (family == AF_UNSPEC || answer.address.getFamily() == family) {
        if (entry.baseTime_ + answer.ttl > now) {
          Answer ans(
              secondsBetween(now, entry.baseTime_), answer.name, answer.type);
          ans.address = answer.address;
          answers.push_back(ans);
        }
      }
    }
  }
}

void CachingDNSResolver::dumpDNSCache(
    std::vector<std::pair<std::string, CacheEntry>>& results, DNSCache& cache) {
  for (const auto& e : cache) {
    results.push_back(e); // copy
  }
}

bool CachingDNSResolver::searchDNSCache(std::string name,
                                        CacheEntry& out,
                                        DNSCache& cache) {
  auto iter = cache.find(name);
  if (iter != cache.end()) {
    out = iter->second;
    return true;
  }

  return false;
}

void CachingDNSResolver::flushDNSCache() {
  cache_.clear();
}
} // namespace proxygen
