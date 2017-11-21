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

#include <atomic>
#include <cassert>
#include <vector>
#include <utility>

namespace HPHP {

constexpr unsigned kLgSlabSize = 21;
constexpr uint32_t kSlabSize = uint32_t{1} << kLgSlabSize;

// Simple lock-free allocator that allocates and deallocates slabs.
struct SlabManager {
  // To mitigate the ABA problem (i.e., a slab is allocated and returned to the
  // list without another thread noticing), we tag the pointers on the lower 16
  // bits.  This should be sufficient for our purpose of slab management, so we
  // don't consider also using other bits for now.
  struct TaggedPtr {
    static constexpr uintptr_t TagMask = (1ul << 16) - 1;
    TaggedPtr() noexcept : rep(0) {}
    TaggedPtr(void* p, uint16_t tag = 0) noexcept
      : rep(reinterpret_cast<uintptr_t>(p) | tag) {
      assert((reinterpret_cast<uintptr_t>(p) & TagMask) == 0);
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

  // Create one SlabManager for each NUMA node, and add some slabs there.
  // Currently they are backed by huge pages, see EvalNum1GPagesForSlabs and
  // EvalNum2MPagesForSlabs.
  static void init();

  static SlabManager* get(int node = -1) {
    if (node < 0) node = 0;
    if (node >= s_slabManagers.size()) return nullptr;
    return s_slabManagers[node];
  }

  // Add a chunk of memory to be managed, thread-safe.
  void addRange(void* ptr, std::size_t size);

  void push(void* p, uint16_t tag) {
    ++tag;
    auto ptr = reinterpret_cast<AtomicTaggedPtr*>(p);
    TaggedPtr tagged{p, tag};
    while (true) {
      auto currHead = m_head.load(std::memory_order_acquire);
      ptr->store(currHead, std::memory_order_release);
      if (m_head.compare_exchange_weak(currHead, tagged,
                                       std::memory_order_release)) {
        return;
      }
    }
  }

  // To reduce contention on m_head, push multiple slabs at once.
  void pushMulti(std::vector<std::pair<void*, uint16_t>>& slabs) {
    if (slabs.empty()) return;
    // form a local list first, without using atomics.
    auto const size = slabs.size();
    for (auto i = 0u; i < size - 1; ++i) {
      auto ptr = reinterpret_cast<TaggedPtr*>(slabs[i].first);
      uint16_t newTag = slabs[i + 1].second + 1;
      *ptr = TaggedPtr{slabs[i + 1].first, newTag};
    }
    TaggedPtr newHead{slabs[0].first, uint16_t(slabs[0].second + 1)};
    auto last = reinterpret_cast<AtomicTaggedPtr*>(slabs[size - 1].first);
    while (true) {
      auto currHead = m_head.load(std::memory_order_acquire);
      last->store(currHead, std::memory_order_release);
      if (m_head.compare_exchange_weak(currHead, newHead,
                                       std::memory_order_release)) {
        return;
      }
    }
  }

  TaggedPtr tryAlloc() {
    while (auto currHead = m_head.load(std::memory_order_acquire)) {
      auto const ptr = reinterpret_cast<AtomicTaggedPtr*>(currHead.ptr());
      auto next = ptr->load(std::memory_order_acquire);
      if (m_head.compare_exchange_weak(currHead, next,
                                       std::memory_order_release)) {
        return currHead;
      }
    }
    return {nullptr, 0};
  }

 private:
  using AtomicTaggedPtr = std::atomic<TaggedPtr>;
  AtomicTaggedPtr m_head;

  static std::vector<SlabManager*> s_slabManagers; // one for each NUMA node
};

}

#endif
