/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ATOMIC_COUNTABLE_H_
#define incl_HPHP_ATOMIC_COUNTABLE_H_

#include <atomic>
#include <cstdint>

namespace HPHP {

/**
 * If an object may be shared by multiple threads but we want to reclaim it
 * when all threads are finished using it, we need to allocate it with the C++
 * new operator (instead of MemoryManager) and we need to use AtomicSharedPtr.
 */
struct AtomicCountable {
  using RefCount = int32_t;
  AtomicCountable() : m_count(0) {}
  RefCount getCount() const { return m_count; }
  void incAtomicCount() const { ++m_count; }
  RefCount decAtomicCount() const { return --m_count; }
 protected:
  mutable std::atomic<RefCount> m_count;
};

}
#endif
