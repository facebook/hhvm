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
 *     exactly the same size. For example, EmptyArray.
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
  static ThreadLocal<MemoryManager> &TheMemoryManager();

  MemoryManager();

  /**
   * Without calling this, everything should work as if there is no memory
   * manager.
   */
  void enable() { m_enabled = true;}

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
   * Application can call this function optionally as an optimization not
   * to put back object pointers to free list before sweeping them all.
   */
  void disableDealloc();

  /**
   * Mark current allocator's position as ending point of a generation and
   * sweep all memory that has allocated since the previous check point.
   */
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
  const MemoryUsageStats &getStats() { return m_stats;}

  /**
   * Called during session starts to reset all usage stats.
   */
  void resetStats();

private:
  static DECLARE_THREAD_LOCAL(MemoryManager, s_singleton);

  bool m_enabled;
  bool m_checkpoint;

  std::vector<SmartAllocatorImpl*> m_smartAllocators;
  LinearAllocator m_linearAllocator;
  std::set<UnsafePointer*> m_unsafePointers;

  MemoryUsageStats m_stats;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_MEMORY_MANAGER_H__
