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

#ifndef incl_HPHP_SLAB_MANAGER_H_
#define incl_HPHP_SLAB_MANAGER_H_

#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"

#include <atomic>
#include <vector>
#include <utility>

namespace HPHP {

constexpr unsigned kLgSlabSize = 21;
constexpr size_t kSlabSize = 1ull << kLgSlabSize;
constexpr size_t kSlabAlign = kSlabSize;

// To mitigate the ABA problem (i.e., a slab is allocated and returned to the
// list without another thread noticing), we tag the pointers on the lower 16
// bits.  This should be sufficient for our purpose of slab management, so we
// don't consider also using other bits for now.
struct TaggedSlabPtr {
  static constexpr uintptr_t TagMask = (1ul << 16) - 1;
  static_assert(kSlabAlign > TagMask, "");
  TaggedSlabPtr() noexcept : rep(0) {}
  /* implicit */ TaggedSlabPtr(std::nullptr_t) noexcept : rep(0) {}
  TaggedSlabPtr(void* p, uint16_t tag = 0) noexcept
    : rep(reinterpret_cast<uintptr_t>(p) | tag) {
    assertx(ptr() == p);
  }
  void* ptr() const {
    return reinterpret_cast<void*>(rep & ~TagMask);
  }
  uint16_t tag() const {
    return static_cast<uint16_t>(rep);
  }
  explicit operator bool() const {
    return !!rep;
  }
 private:
  uintptr_t rep;
};

using AtomicTaggedSlabPtr = std::atomic<TaggedSlabPtr>;

/*
 * Instrusive singly linked list of slabs using TaggedSlabPtr at the beginning
 * of each slab.
 */
struct TaggedSlabList {
  bool empty() const {
    return !m_head.load(std::memory_order_relaxed);
  }
  TaggedSlabPtr head() {
    return m_head.load(std::memory_order_relaxed);
  }
  /*
   * Add a slab to the list.  If `local`, assume the list is only accessed in a
   * single thread.
   */
  template<bool local = false> void push_front(void* p, uint16_t tag) {
    ++tag;
    TaggedSlabPtr tagged{p, tag};
    auto ptr = reinterpret_cast<AtomicTaggedSlabPtr*>(p);
    if (local) {
      auto currHead = m_head.load(std::memory_order_relaxed);
      ptr->store(currHead, std::memory_order_relaxed);
      m_head.store(tagged, std::memory_order_relaxed);
      return;
    }
    while (true) {
      auto currHead = m_head.load(std::memory_order_acquire);
      ptr->store(currHead, std::memory_order_release);
      if (m_head.compare_exchange_weak(currHead, tagged,
                                       std::memory_order_release)) {
        return;
      }
    }
  }

  // Divide a preallocated piece of memory into slabs and add to the list.
  NEVER_INLINE void addRange(void* ptr, std::size_t size);

 protected:
  AtomicTaggedSlabPtr m_head;
};

struct SlabManager : TaggedSlabList {
  // Create one SlabManager for each NUMA node, and add some slabs there.
  // Currently they are backed by huge pages, see EvalNum1GPagesForSlabs and
  // EvalNum2MPagesForSlabs.
  static void init();

  static SlabManager* get(int node = -1) {
    if (node < 0) node = 0;
    if (node >= s_slabManagers.size()) return nullptr;
    return s_slabManagers[node];
  }

  TaggedSlabPtr tryAlloc() {
    while (auto currHead = m_head.load(std::memory_order_acquire)) {
      auto const ptr =reinterpret_cast<AtomicTaggedSlabPtr*>(currHead.ptr());
      auto next = ptr->load(std::memory_order_acquire);
      if (m_head.compare_exchange_weak(currHead, next,
                                       std::memory_order_release)) {
        return currHead;
      }
    }
    return nullptr;
  }

  // Push everything in a local TaggedSlabList starting with `newHead` and
  // ending with `localTail` to this global list.  The linking on the local list
  // should be performed before this call. This is intended for returning
  // multiple local slabs to the global list in one batch at the end of each
  // request.
  void merge(TaggedSlabPtr newHead, void* localTail) {
    assertx(newHead);
    // No need to bump the tag here, as it is already bumped when forming the
    // local list.
    auto last = reinterpret_cast<AtomicTaggedSlabPtr*>(localTail);
    while (true) {
      auto currHead = m_head.load(std::memory_order_acquire);
      last->store(currHead, std::memory_order_release);
      if (m_head.compare_exchange_weak(currHead, newHead,
                                       std::memory_order_release)) {
        return;
      }
    }
  }

  static std::vector<SlabManager*> s_slabManagers; // one for each NUMA node
};

}

#endif
