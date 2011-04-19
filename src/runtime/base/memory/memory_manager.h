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

#ifndef __HPHP_MEMORY_MANAGER_H__
#define __HPHP_MEMORY_MANAGER_H__

#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/memory/linear_allocator.h>
#include <runtime/base/memory/unsafe_pointer.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * MemoryManager categorizes memory usage into 4 categories and maintain some
 * of them with different strategy:
 *
 *  1. Fixed size objects: de/allocated by SmartAllocators, these objects have
 *     exactly the same size.
 *  2. Interally malloc-ed and variable sized memory held by fixed size
 *     objects, for example, StringData's m_data. These memory can be backed up
 *     and restored by LinearAllocator.
 *  3. Unsafe pointers held by fixed size objects, for example, ObjectData*
 *     held by Object. These pointers point to some external memory that's out
 *     of the control of MemoryManager, and therefore they are only interfaced
 *     through UnsafePointer class to make sure they are protected when these
 *     pointers are backed up to LinearAllocator.
 *  4. Freelance memory, malloced by extensions or STL classes, that are
 *     completely out of MemoryManager's control.
 */
class MemoryManager {
public:
  static ThreadLocalNoCheck<MemoryManager> &TheMemoryManager();

  MemoryManager();

  /**
   * Without calling this, everything should work as if there is no memory
   * manager.
   */
  void enable() { m_enabled = true;}
  void disable() { m_enabled = false;}

  /**
   * Register a smart allocator. Done by SmartAlloctorImpl's constructor.
   */
  void add(SmartAllocatorImpl *allocator);

  /**
   * Register an unsafe pointer. Done by UnsafePointer's constructor.
   */
  void add(UnsafePointer *p);

  /**
   * Register an unsafe pointer. Done by UnsafePointer's destructor.
   */
  void remove(UnsafePointer *p);

  /**
   * Whether a checkpoint has been taken.
   */
  bool beforeCheckpoint() const {
    return m_enabled && !m_checkpoint;
  }
  bool afterCheckpoint() const {
    return m_enabled && m_checkpoint;
  }

  /**
   * Mark current allocator's position as starting point of a new generation.
   */
  void checkpoint();

  /**
   * Mark current allocator's position as ending point of a generation and
   * sweep all memory that has allocated since the previous check point.
   */
  void sweepAll();
  void rollback();

  /**
   * For any objects that need to do extra work during thread shutdown time.
   */
  void cleanup();

  /**
   * Protect the unsafe pointers.
   */
  void protectUnsafePointers();

  /**
   * Write stats to ServerStats.
   */
  void logStats();

  /**
   * Display any leaked or double-freed memory.
   */
  void checkMemory(bool detailed);

  /**
   * Find out how much memory we have used so far.
   */
  MemoryUsageStats &getStats(bool refresh=false) {
    if (refresh) {
      refreshStats();
    }
    return m_stats;
  }

  /**
   * Called during session starts to reset all usage stats.
   */
  void resetStats();

  /**
   * Refresh stats to reflect directly malloc()ed memory, and determine whether
   * the request memory limit has been exceeded.
   */
  void refreshStats() {
#ifdef USE_JEMALLOC
    // Incrementally incorporate the difference between the previous and current
    // deltas into the memory usage statistic.  For reference, the total
    // malloced memory usage could be calculated as such, if delta0 were
    // recorded in resetStats():
    //
    //   int64 musage = delta - delta0;
    //
    // Note however, that SmartAllocator subtracts from m_stats.usage when it
    // calls malloc(), so that later smart allocations that adjust m_stats.usage
    // don't double-count memory.  Thus musage in the example code may well
    // substantially exceed m_stats.usage.
    if (s_stats_enabled) {
      int64 delta = int64(*m_allocated) - int64(*m_deallocated);
      m_stats.usage += delta - m_delta;
      m_delta = delta;
    }
#endif
    if (m_stats.usage > m_stats.peakUsage) {
      // NOTE: the peak memory usage monotonically increases, so there cannot
      // be a second OOM exception in one request.
      if (m_stats.maxBytes > 0 && m_stats.peakUsage <= m_stats.maxBytes &&
          m_stats.usage > m_stats.maxBytes) {
        refreshStatsHelperExceeded();
      }
      // Check whether the process's active memory limit has been exceeded, and
      // if so, stop the server.
      //
      // Only check whether the total memory limit was exceeded if this request
      // is at a new high water mark.  This check could be performed regardless
      // of this request's current memory usage (because other request threads
      // could be to blame for the increased memory usage), but doing so would
      // measurably increase computation for little benefit.
#ifdef USE_JEMALLOC
      // (*m_cactive) consistency is achieved via atomic operations.  The fact
      // that we do not use an atomic operation here means that we could get a
      // stale read, but in practice that poses no problems for how we are
      // using the value.
      if (*m_cactive > m_cactiveLimit) {
        refreshStatsHelperStop();
      }
#endif
      m_stats.peakUsage = m_stats.usage;
    }
  }

  class MaskAlloc {
    MemoryManager *m_mm;
  public:
    MaskAlloc(MemoryManager *mm) : m_mm(mm) {
      // capture all mallocs prior to construction
      m_mm->refreshStats();
    }
    ~MaskAlloc() {
#ifdef USE_JEMALLOC
      // exclude mallocs and frees since construction
      if (s_stats_enabled) {
        m_mm->m_delta = int64(*m_mm->m_allocated) - int64(*m_mm->m_deallocated);
      }
#endif
    }
  };

private:
  void refreshStatsHelperExceeded();
#ifdef USE_JEMALLOC
  void refreshStatsHelperStop();
#endif

  static DECLARE_THREAD_LOCAL_NO_CHECK(MemoryManager, s_singleton);

  bool m_enabled;
  bool m_checkpoint;

  std::vector<SmartAllocatorImpl*> m_smartAllocators;
  LinearAllocator m_linearAllocator;
  std::set<UnsafePointer*> m_unsafePointers;

  MemoryUsageStats m_stats;
#ifdef USE_JEMALLOC
  uint64* m_allocated;
  uint64* m_deallocated;
  int64  m_delta;
  size_t* m_cactive;
  size_t m_cactiveLimit;
  bool m_stopped; // Set to true if m_cactive exceeded limit.

public:
  static bool s_stats_enabled;
#endif
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MEMORY_MANAGER_H__
