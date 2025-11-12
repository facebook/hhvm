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

#pragma once

#include <atomic>
#include <folly/AtomicHashMap.h>
#include "hphp/runtime/base/request-id.h"
#include "hphp/util/hash-map.h"
#include "hphp/util/service-data.h"


namespace HPHP {

/**
 * RequestFanoutLimit manages per-request thread fanout limits to prevent
 * excessive thread spawning from individual requests.
 * 
 * This class is thread-safe and uses atomic operations to track the number
 * of threads spawned for each root request ID.
 * 
 * Typical usage:
 * - Call increment() when spawning a new thread for a request
 * - Call decrement() when a thread completes
 * - Call clear() to clear entire map that tracks number of threads per request
 */
struct RequestFanoutLimit {
public:
  struct RequestFanoutCount {
    std::atomic<int64_t> currentCount;
    std::atomic<int64_t> maxCount;
    RequestFanoutCount(int64_t current, int64_t max)
        : currentCount(current), maxCount(max) {}
    
    int64_t increment();
    int64_t decrement();
  };
  using RootRequestIDToThreadCnt = folly_concurrent_hash_map_simd<int64_t, std::shared_ptr<RequestFanoutCount>>;

  /**
   * Constructor
   * @param limit Maximum number of threads allowed per request ID
   * @param size Initial size of the hash map
   */
  RequestFanoutLimit(int limit, int size);

  /**
   * Destructor - cleans up the atomic hash map
   */
  ~RequestFanoutLimit() {};

  /**
   * Increment the thread count for a given request ID
   * If the given root request ID is not present in the map,
   * a new entry will be created with value 1
   * @param id The request ID to increment count for
   */
  void increment(const RequestId& id);

  /**
   * Decrement the thread count for a given request ID
   * If count reaches 0, the entry will be removed from the map
   * @param id The request ID to decrement count for
   */
  void decrement(const RequestId& id);

  /**
   * Get the current thread count for a given request ID
   * DO NOT USE outside of testing and logging as the value 
   * can be stale by the time it is read
   * @param id The root request ID to check
   * @return Current count, or -1 if entry not found
   */
  int getCurrentCount(const RequestId& id) const;

  int getMaxCount(const RequestId& id) const;

  /**
   * Get the configured limit
   * @return The maximum number of threads allowed per request
   */
  int getLimit() const { return m_limit; }

private:
  const int m_limit;
  RootRequestIDToThreadCnt m_map;
  
  ServiceData::ExportedCounter* m_fanoutLogger{nullptr};
};

} // namespace HPHP
