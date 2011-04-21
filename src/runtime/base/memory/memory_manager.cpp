/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/memory/sweepable.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/http_server.h>
#include <util/alloc.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_JEMALLOC
bool MemoryManager::s_stats_enabled = false;

static size_t threadAllocatedpMib[2];
static size_t threadDeallocatedpMib[2];
static size_t statsCactiveMib[2];
static pthread_once_t mibOnce = PTHREAD_ONCE_INIT;
static void mibInit() {
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
  MemoryManager::s_stats_enabled = true;
}

static inline void thread_stats(uint64*& allocated, uint64*& deallocated,
                                size_t*& cactive) {
  pthread_once(&mibOnce, mibInit);
  if (!MemoryManager::s_stats_enabled) return;

  size_t len = sizeof(allocated);
  if (mallctlbymib(threadAllocatedpMib,
                   sizeof(threadAllocatedpMib) / sizeof(size_t),
                   &allocated, &len, NULL, 0)) {
    assert(false);
  }

  len = sizeof(deallocated);
  if (mallctlbymib(threadDeallocatedpMib,
                   sizeof(threadDeallocatedpMib) / sizeof(size_t),
                   &deallocated, &len, NULL, 0)) {
    assert(false);
  }

  len = sizeof(cactive);
  if (mallctlbymib(statsCactiveMib,
                   sizeof(statsCactiveMib) / sizeof(size_t),
                   &cactive, &len, NULL, 0)) {
    assert(false);
  }
}
#endif

IMPLEMENT_THREAD_LOCAL_NO_CHECK(MemoryManager, MemoryManager::s_singleton);

ThreadLocalNoCheck<MemoryManager> &MemoryManager::TheMemoryManager() {
  return s_singleton;
}

MemoryManager::MemoryManager() : m_enabled(false), m_checkpoint(false) {
  if (RuntimeOption::EnableMemoryManager) {
    m_enabled = true;
  }
#ifdef USE_JEMALLOC
  thread_stats(m_allocated, m_deallocated, m_cactive);
  m_cactiveLimit = RuntimeOption::ServerMemoryMaxActive;
#endif
  resetStats();
  m_stats.maxBytes = 0;
}

void MemoryManager::resetStats() {
  m_stats.usage = 0;
  m_stats.alloc = 0;
  m_stats.peakUsage = 0;
  m_stats.peakAlloc = 0;
#ifdef USE_JEMALLOC
  if (s_stats_enabled) {
    m_delta = int64(*m_allocated) - int64(*m_deallocated);
  }
#endif
}

void MemoryManager::refreshStatsHelperExceeded() {
  RequestInjectionData &data =
    ThreadInfo::s_threadInfo.getNoCheck()->m_reqInjectionData;
  Lock lock(data.surpriseMutex);
  data.memExceeded = true;
  data.surprised = true;
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
  ASSERT(&allocator->getStats() == &m_stats);

  /**
   * If a SmartAllocator is registered later than checkpoint, there must be no
   * historical objects to restore. But this allocator still needs to be
   * registered to rollback new objects after checkpoints.
   */
  if (m_checkpoint) {
    allocator->disableRestore();
  }
}

void MemoryManager::add(UnsafePointer *p) {
  ASSERT(p);
  ASSERT(!m_checkpoint);
  m_unsafePointers.insert(p);
}

void MemoryManager::remove(UnsafePointer *p) {
  ASSERT(p);
  ASSERT(!m_checkpoint);
  m_unsafePointers.erase(p);
}

void MemoryManager::protectUnsafePointers() {
  for (std::set<UnsafePointer*>::iterator iter = m_unsafePointers.begin();
       iter != m_unsafePointers.end(); ++iter) {
    UnsafePointer *p = *iter;
    ASSERT(p);
    p->protect();
  }
}

void MemoryManager::checkpoint() {
  ASSERT(!m_checkpoint);
  m_checkpoint = true;

  protectUnsafePointers();
  int size = 0;
  int count = 0;
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    count += m_smartAllocators[i]->calculateObjects(m_linearAllocator, size);
  }

  m_linearAllocator.beginBackup(size, count);
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->backupObjects(m_linearAllocator);
  }
  m_linearAllocator.endBackup();
}

void MemoryManager::sweepAll() {
  Sweepable::SweepAll();
}

void MemoryManager::rollback() {
  m_linearAllocator.beginRestore();
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->rollbackObjects(m_linearAllocator);
  }
  m_linearAllocator.endRestore();
  protectUnsafePointers();
}

void MemoryManager::cleanup() {
}

void MemoryManager::logStats() {
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->logStats();
  }
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
  m_linearAllocator.checkMemory(detailed);
  printf("Unsafe pointers: %d\n", (int)m_unsafePointers.size());
}

///////////////////////////////////////////////////////////////////////////////
}
