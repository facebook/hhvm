/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/hhbbc/misc.h"

#include "hphp/hhbbc/parallel.h"

#include "hphp/runtime/base/string-data.h"

#include "hphp/util/extern-worker.h"
#include "hphp/util/hash.h"

//////////////////////////////////////////////////////////////////////

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

std::vector<std::vector<SString>>
consistently_bucketize(const std::vector<SString>& items, size_t bucketSize) {
  // Calculate the number of buckets we need, assuming each bucket
  // will contain "bucketSize" number of elements (rounding up).
  assertx(bucketSize > 0);
  auto const numBuckets = (items.size() + bucketSize - 1) / bucketSize;
  return consistently_bucketize_by_num_buckets(items, numBuckets);
}

std::vector<std::vector<SString>>
consistently_bucketize_by_num_buckets(const std::vector<SString>& items,
                                      size_t numBuckets) {
  using namespace folly::gen;
  if (numBuckets == items.size()) {
    return from(items)
      | map([] (SString s) { return singleton_vec(s); })
      | as<std::vector>();
  }
  if (numBuckets == 1) return singleton_vec(items);

  // Consistently hash the strings into their buckets indices.
  auto const indices = parallel::map(
    items,
    [&] (SString s) {
      auto const idx = consistent_hash(s->hashStatic(), numBuckets);
      assertx(idx < numBuckets);
      return idx;
    }
  );

  // Then partition the strings into the buckets using the indices.
  std::vector<std::vector<SString>> buckets;
  buckets.resize(numBuckets);
  assertx(indices.size() == items.size());
  for (size_t i = 0, size = indices.size(); i < size; ++i) {
    buckets[indices[i]].emplace_back(items[i]);
  }

  // Sort each bucket to keep things consistent.
  parallel::for_each(
    buckets,
    [] (std::vector<SString>& bucket) {
      std::sort(begin(bucket), end(bucket), string_data_lt_type{});
    }
  );

  // Finally, remove any empty buckets.
  buckets.erase(
    std::remove_if(
      begin(buckets),
      end(buckets),
      [] (const std::vector<SString>& b) { return b.empty(); }
    ),
    end(buckets)
  );
  return buckets;
}

//////////////////////////////////////////////////////////////////////

namespace {

// Would be nice to determine this better.
static constexpr size_t kMaxMemory = 8UL*1024*1024*1024;

}

//////////////////////////////////////////////////////////////////////

extern_worker::Client::ExecMetadata make_exec_metadata(const std::string& job,
                                                       const std::string& key) {
  extern_worker::Client::ExecMetadata meta;
  meta.max_memory = kMaxMemory;

  meta.job_key = folly::sformat("{}{}{}", job, key.empty() ? "" : " ", key);
  meta.affinity_keys.emplace_back(meta.job_key);
  if (!key.empty()) meta.affinity_keys.emplace_back(job);
  return meta;
}

extern_worker::Client::ExecMetadata make_exec_metadata(const std::string& job,
                                                       int round,
                                                       const std::string& key) {
  extern_worker::Client::ExecMetadata meta;
  meta.max_memory = kMaxMemory;

  auto const jobAndRound = folly::sformat("{} round {}", job, round);
  meta.job_key =
    folly::sformat("{}{}{}", jobAndRound, key.empty() ? "" : " ", key);
  meta.affinity_keys.emplace_back(meta.job_key);
  if (!key.empty()) meta.affinity_keys.emplace_back(jobAndRound);
  meta.affinity_keys.emplace_back(job);
  return meta;
}

//////////////////////////////////////////////////////////////////////

}
