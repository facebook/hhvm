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
#include "hphp/util/configs/server.h"
#include "hphp/util/logger.h"
#include "hphp/util/struct-log.h"

namespace HPHP {

static std::atomic<int64_t> s_maxConcurrentFanoutCount = 0;

int64_t RequestFanoutLimit::RequestFanoutCount::increment() {
  /**
  * This operation is safe from operating on abandoned 
  * counter erased by other racing xbox threads, because:
  * - The counter is always initialized to 2 during insertion to account for root-req, AND
  * - All increment for children request will happen before enqueue() 
  *   and inside the parent request's lifetime, ensures the count will always > 0 
  *   as long as any offspring xbox request is running
  */
  auto countBeforeIncrement = currentCount.fetch_add(1, std::memory_order_acq_rel);
  auto newCount = countBeforeIncrement + 1;

  /**
   * Check the new count is below the configured fanout limit
   */
  if (newCount > Cfg::Server::RequestFanoutLimit) {
    // TODO add enforcement here
  }

  /**
   * Update the max concurrent count for the given root req
   */
  auto prevMax = maxCount.load(std::memory_order_acquire);
  while (newCount > prevMax 
    && !maxCount.compare_exchange_weak(
      prevMax, 
      newCount, 
      std::memory_order_acq_rel, 
      std::memory_order_acquire)
    ) {
      // prevMax updated by compare_exchange_weak
  }

  return countBeforeIncrement;
}

int64_t RequestFanoutLimit::RequestFanoutCount::decrement() {
  return currentCount.fetch_sub(1, std::memory_order_acq_rel);
}

RequestFanoutLimit::RequestFanoutLimit(int limit, int size)
  : m_limit(limit), m_map(size) {  
  assertx(limit > 0);
  assertx(size > 0);
  if (Cfg::Server::EnableRequestFanoutLogging) {
    m_fanoutLogger = ServiceData::createCounter("request_fanout.max_concurrent_count");
  }
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
    auto insertResult = m_map.insert(
      id.id(), 
      std::make_shared<RequestFanoutLimit::RequestFanoutCount>(2, 2)
    );
  } else {
    auto counter = it->second.get();
    assertx(counter);
    counter->increment();
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

  auto countBeforeDecrement = counter->decrement();
  
  if (countBeforeDecrement == 1) {
    // Update the global server max counter if current root-req has spawn 
    // more children req than any other previous root-req
    auto prevGlobalMax = s_maxConcurrentFanoutCount.load(std::memory_order_acquire);
    auto currReqMax = counter->maxCount.load(std::memory_order_acquire);
    while (
      currReqMax > prevGlobalMax 
      && !s_maxConcurrentFanoutCount.compare_exchange_weak(
        prevGlobalMax, 
        currReqMax, 
        std::memory_order_acq_rel, 
        std::memory_order_acquire
      )
    ) {
      // global server max updated by compare_exchange_weak
    }

    // Log the up-to-date server maxConcurrentFanoutCount to ODS
    if (m_fanoutLogger) {
      m_fanoutLogger->setValue(s_maxConcurrentFanoutCount.load(std::memory_order_acquire));
    }
    // Log maxConcurrentCount of current root request to Scuba
    if (Cfg::Server::EnableRequestFanoutLogging && StructuredLog::enabled()) {
      StructuredLogEntry entry;
      entry.setInt("request_max_concurrent_fanout_count", currReqMax);
      // TODO: Log root request script filename
      StructuredLog::log("hhvm_request_fanout", entry);
    }

    // Erase the entry
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

  return counter->currentCount.load(std::memory_order_acquire);
}

int RequestFanoutLimit::getMaxCount(const RequestId& id) const {
  auto it = m_map.find(id.id());
  if (it == m_map.end()) {
    return -1;
  }

  auto counter = it->second.get();
  if (counter == nullptr) {
    return -1;
  }

  return counter->maxCount.load(std::memory_order_acquire);
}

} // namespace HPHP
