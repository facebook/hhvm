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

#include "hphp/runtime/base/request-fanout-limit.h"
#include "hphp/util/logger.h"

namespace HPHP {

RequestFanoutLimit::RequestFanoutLimit(int limit, int size)
  : m_limit(limit), m_map(size) {  
  assertx(limit > 0);
  assertx(size > 0);
}

void RequestFanoutLimit::increment(const RequestId& id) {
  if (id.unallocated()) {
    return;
  }

  auto it = m_map.find(id.id());
  if (it == m_map.end()) {
    // Inserting 2 here (instead of 1) to include parent request as part of tracking. 
    // This, and together with all increment for children request will happen before 
    // enqueue() and inside the parent request's lifetime, ensures the count will 
    // always > 0 as long as any offspring children (i.e. xbox) request is running, 
    // which avoids operating on abandoned counter erased by other racing children 
    // (i.e. xbox) threads. 
    auto insertResult = m_map.insert(id.id(), std::make_shared<std::atomic<int64_t>>(2));
  } else {
    auto counter = it->second.get();
    assertx(counter);

    /**
      * This operation is safe from operating on abandoned 
      * counter erased by other racing xbox threads, because:
      * - The counter is always initialized to 1 when root request onSessionInit(), AND
      * - All increment for children request will happen before enqueue() 
      *   and inside the parent request's lifetime, ensures the count will always > 0 
      *   as long as any offspring xbox request is running
      */
    auto countBeforeIncrement = counter->fetch_add(1, std::memory_order_acq_rel);
    if (countBeforeIncrement >= m_limit) {
      Logger::Verbose("Request fanout limit exceeded.");
    }
  }
}

void RequestFanoutLimit::decrement(const RequestId& id) {
  if (id.unallocated()) {
    return; 
  }

  auto it = m_map.find(id.id());
  if (it == m_map.end()) {
    // This check is safe and necessary to handle the cases of cli-server also
    // calling RI.onSessionExit() during SCOPE_EXIT. 
    return;
  }

  auto counter = it->second.get();
  assertx(counter);

  auto countBeforeDecrement = counter->fetch_sub(1, std::memory_order_acq_rel);
  
  // Clear the entry if current thread does 1 to 0 decrement
  if (countBeforeDecrement == 1) {
    m_map.erase(id.id());
    Logger::Verbose("All children requests of %ld completed", id.id());
  }
  
}

int RequestFanoutLimit::getCurrentCount(const RequestId& id) const {
  auto it = m_map.find(id.id());
  if (it == m_map.end()) {
    return -1;
  }

  auto counter = it->second.get();
  if (counter == nullptr) {
    return -1;
  }

  return counter->load(std::memory_order_acquire);
}

} // namespace HPHP
