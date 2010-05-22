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

#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/memory/leak_detectable.h>
#include <runtime/base/memory/sweepable.h>
#include <runtime/base/runtime_option.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL(MemoryManager, MemoryManager::s_singleton);

ThreadLocal<MemoryManager> &MemoryManager::TheMemoryManager() {
  return s_singleton;
}

MemoryManager::MemoryManager() : m_enabled(false), m_checkpoint(false) {
  if (RuntimeOption::EnableMemoryManager) {
    m_enabled = true;
  }
  resetStats();
}

void MemoryManager::resetStats() {
  m_stats.usage = 0;
  m_stats.alloc = 0;
  m_stats.peakUsage = 0;
  m_stats.peakAlloc = 0;
}

void MemoryManager::add(SmartAllocatorImpl *allocator) {
  ASSERT(allocator);
  m_smartAllocators.push_back(allocator);
  allocator->registerStats(&m_stats);

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

void MemoryManager::rollback() {
  m_linearAllocator.beginRestore();
  Sweepable::SweepAll();
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->rollbackObjects(m_linearAllocator);
  }
  m_linearAllocator.endRestore();
  protectUnsafePointers();
}

void MemoryManager::disableDealloc() {
  for (unsigned int i = 0; i < m_smartAllocators.size(); i++) {
    m_smartAllocators[i]->disableDealloc();
  }
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
