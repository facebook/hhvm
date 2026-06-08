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

#pragma once

#include <atomic>
#include <chrono>
#include <thread>

#include "hphp/util/cycles.h"

namespace HPHP {

// 3-phase backoff (spin → yield → sleep). The strategy and default iteration
// counts are well tested in another major managed runtime.
struct SpinYield {
  static constexpr uint32_t kDefaultSpinLimit = 4096;
  static constexpr uint32_t kDefaultYieldLimit = 64;
  static constexpr uint32_t kDefaultSleepNs = 1000;

  explicit SpinYield(uint32_t spin_limit = kDefaultSpinLimit,
                     uint32_t yield_limit = kDefaultYieldLimit,
                     uint32_t sleep_ns = kDefaultSleepNs)
      : spin_limit_(spin_limit),
        yield_limit_(yield_limit),
        sleep_ns_(sleep_ns) {}

  void wait() {
    if (spins_ < spin_limit_) {
      ++spins_;
      cpuRelax();
    } else if (yields_ < yield_limit_) {
      ++yields_;
      std::this_thread::yield();
    } else {
      std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_ns_));
    }
  }

 private:
  uint32_t spins_{0};
  uint32_t yields_{0};
  uint32_t spin_limit_;
  uint32_t yield_limit_;
  uint32_t sleep_ns_;
};

/*
 * Base node for ConcurrentSList. T must inherit from this.
 */
struct ConcurrentSListNode {
  ConcurrentSListNode* next{nullptr};
};

/*
 * Lock-free intrusive singly-linked list.
 *
 * T must inherit from ConcurrentSListNode. Nodes are not owned by the list;
 * the caller is responsible for their lifetime and deallocation.
 *
 * Concurrency model:
 *  - insert_head() is lock-free and may be called from multiple threads.
 *  - begin() / iterator traversal and removal require external
 *    synchronization (single consumer).
 *  - Concurrent insert_head() during iteration is safe — newly prepended
 *    nodes won't be visited by an in-progress walk, but the iterator
 *    correctly handles head removal when new nodes have been prepended.
 */
template <typename T>
class ConcurrentSList {
 public:
  class iterator {
   public:
    iterator() = default;
    explicit iterator(ConcurrentSListNode* node, ConcurrentSList* list)
        : curr_(node), list_(list) {}

    // Returns a reference to the current element.
    T& data() { return *static_cast<T*>(curr_); }

    // Advance to the next node. If remove is true, the current node is
    // unlinked from the list before advancing.
    void advance(bool remove = false);

    bool valid() const { return curr_ != nullptr; }

   private:
    friend class ConcurrentSList;
    ConcurrentSListNode* prev_{nullptr};
    ConcurrentSListNode* curr_{nullptr};
    ConcurrentSList* list_{nullptr};
  };

  ConcurrentSList() = default;
  ~ConcurrentSList() = default;

  ConcurrentSList(const ConcurrentSList&) = delete;
  ConcurrentSList& operator=(const ConcurrentSList&) = delete;

  // Atomically prepend item to the head of the list. Lock-free.
  void insert_head(T& item);

  // Return an iterator starting at the current head.
  iterator begin();

 private:
  std::atomic<ConcurrentSListNode*> head_{nullptr};
};

// --- Implementation ---

template <typename T>
void ConcurrentSList<T>::insert_head(T& item) {
  ConcurrentSListNode* node = &item;
  ConcurrentSListNode* expected = head_.load(std::memory_order_relaxed);
  node->next = expected;
  SpinYield spinner;
  // _weak can spuriously fail on LL/SC architectures (ARM); the outer
  // retry loop already handles that, so _strong's inner retry is redundant.
  while (!head_.compare_exchange_weak(
      expected, node,
      std::memory_order_acq_rel,
      std::memory_order_relaxed)) {
    spinner.wait();
    node->next = expected;
  }
}

template <typename T>
void ConcurrentSList<T>::iterator::advance(bool remove) {
  ConcurrentSListNode* next = curr_->next;

  if (!remove) {
    prev_ = curr_;
    curr_ = next;
    return;
  }

  if (prev_) {
    prev_->next = next;
    curr_ = next;
    return;
  }

  // Removing head: CAS may fail if a concurrent insert_head prepended nodes.
  ConcurrentSListNode* expected = curr_;
  if (list_->head_.compare_exchange_strong(
          expected, next,
          std::memory_order_relaxed,
          std::memory_order_relaxed)) {
    curr_ = next;
    return;
  }

  // CAS failed — new nodes were prepended. Walk from new head to find prev.
  ConcurrentSListNode* p = expected;
  while (p->next != curr_) {
    p = p->next;
  }
  p->next = next;
  prev_ = p;
  curr_ = next;
}

template <typename T>
typename ConcurrentSList<T>::iterator ConcurrentSList<T>::begin() {
  return iterator(head_.load(std::memory_order_relaxed), this);
}

} // namespace HPHP
