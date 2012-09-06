/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

// Get SIZE_MAX definition.  Do this before including any other files, to make
// sure that this is the first place that stdint.h is included.
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/memory/sweepable.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/http_server.h>
#include <util/alloc.h>
#include <util/process.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_JEMALLOC
bool MemoryManager::s_statsEnabled = false;
size_t MemoryManager::s_cactiveLimitCeiling = 0;

static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
static size_t statsCactiveMib[2];
static pthread_once_t threadStatsOnce = PTHREAD_ONCE_INIT;
static void threadStatsInit() {
  if (!mallctlnametomib) return;
  size_t miblen = sizeof(threadAllocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.allocatedp", threadAllocatedpMib, &miblen)) {
    return;
  }
  miblen = sizeof(threadDeallocatedpMib) / sizeof(size_t);
  if (mallctlnametomib("thread.deallocatedp", threadDeallocatedpMib, &miblen)) {
    return;
  }
  miblen = sizeof(statsCactiveMib) / sizeof(size_t);
  if (mallctlnametomib("stats.cactive", statsCactiveMib, &miblen)) {
    return;
  }
  MemoryManager::s_statsEnabled = true;

  // In threadStats() we wish to solve for cactiveLimit in:
  //
  //   footprint + cactiveLimit + headRoom == MemTotal
  //
  // However, headRoom comes from RuntimeOption::ServerMemoryHeadRoom, which
  // isn't initialized until after the code here runs.  Therefore, compute
  // s_cactiveLimitCeiling here in order to amortize the cost of introspecting
  // footprint and MemTotal.
  //
  //   cactiveLimit == (MemTotal - footprint) - headRoom
  //
  //   cactiveLimit == s_cactiveLimitCeiling - headRoom
  // where
  //   s_cactiveLimitCeiling == MemTotal - footprint
  size_t footprint = Process::GetCodeFootprint(Process::GetProcessId());
  size_t pageSize = size_t(sysconf(_SC_PAGESIZE));
  size_t MemTotal = size_t(sysconf(_SC_PHYS_PAGES)) * pageSize;
  if (MemTotal > footprint) {
    MemoryManager::s_cactiveLimitCeiling = MemTotal - footprint;
  }
}

static inline void threadStats(uint64*& allocated, uint64*& deallocated,
                               size_t*& cactive, size_t& cactiveLimit) {
  pthread_once(&threadStatsOnce, threadStatsInit);
  if (!MemoryManager::s_statsEnabled) return;

  size_t len = sizeof(allocated);
  if (mallctlbymib(threadAllocatedpMib,
                   sizeof(threadAllocatedpMib) / sizeof(size_t),
                   &allocated, &len, NULL, 0)) {
    not_reached();
  }

  len = sizeof(deallocated);
  if (mallctlbymib(threadDeallocatedpMib,
                   sizeof(threadDeallocatedpMib) / sizeof(size_t),
                   &deallocated, &len, NULL, 0)) {
    not_reached();
  }

  len = sizeof(cactive);
  if (mallctlbymib(statsCactiveMib,
                   sizeof(statsCactiveMib) / sizeof(size_t),
                   &cactive, &len, NULL, 0)) {
    not_reached();
  }

  size_t headRoom = RuntimeOption::ServerMemoryHeadRoom;
  // Compute cactiveLimit based on s_cactiveLimitCeiling, as computed in
  // threadStatsInit().
  if (headRoom != 0 && headRoom < MemoryManager::s_cactiveLimitCeiling) {
    cactiveLimit = MemoryManager::s_cactiveLimitCeiling - headRoom;
  } else {
    cactiveLimit = SIZE_MAX;
  }
}
#endif

IMPLEMENT_THREAD_LOCAL_NO_CHECK(MemoryManager, MemoryManager::s_singleton);

ThreadLocalNoCheck<MemoryManager> &MemoryManager::TheMemoryManager() {
  return s_singleton;
}

MemoryManager::AllocIterator::AllocIterator(const MemoryManager* mman)
  : m_mman(*mman)
  , m_it(m_mman.m_smartAllocators.begin())
{}

SmartAllocatorImpl*
MemoryManager::AllocIterator::current() const {
  return m_it == m_mman.m_smartAllocators.end() ? 0 : *m_it;
}

void MemoryManager::AllocIterator::next() {
  ++m_it;
}

MemoryManager::MemoryManager() : m_enabled(false) {
  if (RuntimeOption::EnableMemoryManager) {
    m_enabled = true;
  }
#ifdef USE_JEMALLOC
  threadStats(m_allocated, m_deallocated, m_cactive, m_cactiveLimit);
#endif
  resetStats();
  m_stats.maxBytes = INT64_MAX;
}

void MemoryManager::resetStats() {
  m_stats.usage = 0;
  m_stats.alloc = 0;
  m_stats.peakUsage = 0;
  m_stats.peakAlloc = 0;
  m_stats.totalAlloc = 0;
#ifdef USE_JEMALLOC
  if (s_statsEnabled) {
#ifdef HHVM
    m_stats.jemallocDebt = 0;
#endif
    m_prevAllocated = int64(*m_allocated);
    m_delta = m_prevAllocated - int64(*m_deallocated);
  }
#endif
}

void MemoryManager::refreshStatsHelperExceeded() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  info->m_reqInjectionData.setMemExceededFlag();
}

#ifdef USE_JEMALLOC
void MemoryManager::refreshStatsHelperStop() {
  HttpServer::Server->stop();
  // Increase the limit to the maximum possible value, so that this method
  // won't be called again.
  m_cactiveLimit = SIZE_MAX;
}
#endif

void MemoryManager::add(SmartAllocatorImpl *allocator) {
  ASSERT(allocator);
  m_smartAllocators.push_back(allocator);
}

void MemoryManager::sweepAll() {
  Sweepable::SweepAll();
#ifdef HHVM_GC
  GCRootTracker<StringData>::clear();
  GCRootTracker<ArrayData>::clear();
  GCRootTracker<ObjectData>::clear();
  GCRootTracker<Variant>::clear();
  GCRoot<StringData>::clear();
  GCRoot<ArrayData>::clear();
  GCRoot<ObjectData>::clear();
  GCRoot<Variant>::clear();
#endif
}

void MemoryManager::rollback() {
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->rollbackObjects();
  }
}

void MemoryManager::logStats() {
  LeakDetectable::LogMallocStats();
}

void MemoryManager::checkMemory(bool detailed) {
  printf("----- MemoryManager for Thread %ld -----\n", (long)pthread_self());

  refreshStats();
  printf("Current Usage: %lld bytes\t", m_stats.usage);
  printf("Current Alloc: %lld bytes\n", m_stats.alloc);
  printf("Peak Usage: %lld bytes\t", m_stats.peakUsage);
  printf("Peak Alloc: %lld bytes\n", m_stats.peakAlloc);

  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->checkMemory(detailed);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
