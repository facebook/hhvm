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

#include "hphp/hhbbc/options.h"
#include "hphp/hhbbc/parallel.h"

#include "hphp/runtime/base/string-data.h"

#include "hphp/util/hash.h"

//////////////////////////////////////////////////////////////////////

namespace HPHP::HHBBC {

//////////////////////////////////////////////////////////////////////

std::vector<std::vector<SString>>
consistently_bucketize(const std::vector<SString>& items, size_t bucketSize) {
  using namespace folly::gen;

  // Calculate the number of buckets we need, assuming each bucket
  // will contain "bucketSize" number of elements (rounding up).
  assertx(bucketSize > 0);
  auto const numBuckets = (items.size() + bucketSize - 1) / bucketSize;
  return consistently_bucketize_by_num_buckets(items, numBuckets);
}

std::vector<std::vector<SString>>
consistently_bucketize_by_num_buckets(const std::vector<SString>& items, size_t numBuckets) {
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
      std::sort(begin(bucket), end(bucket), string_data_lti{});
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

}
