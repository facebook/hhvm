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

#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"

#include <signal.h>
#ifdef __x86_64__
#include <ucontext.h>
#endif

#include <folly/sorted_vector_types.h>
#include <algorithm>

namespace HPHP {

/*
 * Utility to trace reference count changes for certain heap objects. This can
 * be useful when debugging heap corruptions related to bugs in reference
 * counting implementation, e.g., missed IncRef.
 *
 * To use this, we need to have some idea on what HeapObject may be corrupted,
 * and allocate the HeapObject using HeapObjectSanitizer instead of
 * MemoryManager. Use the `watch` method to print out value changes of the
 * 4-byte word (usually the refcount).
 */
struct HeapObjectSanitizer {
  static constexpr size_t kPageSize = 4096ull;
  static constexpr size_t kPageSizeMask = kPageSize - 1;

  static inline constexpr size_t ru(size_t size) {
    return (size + kPageSizeMask) & ~kPageSizeMask;
  }

  // Allocate a piece of memory, for which the range beyond offset won't be
  // watched. Usually, we want to put the refcount word and the rest of the
  // HeapObject on different pages, to avoid unnecessarily watching all changes
  // to the object.
  void* alloc(size_t size, uint32_t offset);

  void free(void* ptr);

  void watch(void* ptr) {
    addresses.insert(ptr);
    // Subsequent writes to the page containing ptr will cause SIGSEGV (and is
    // handled gracefully without actually crashing).
    set_page_protection(ptr, PROT_READ);
  }

  void unwatch(void* ptr) {
    auto iter = addresses.find(ptr);
    always_assert(iter != addresses.end());
    addresses.erase(iter);
    set_page_protection(ptr, PROT_READ | PROT_WRITE);
  }

  void reset() {
    for (auto p : pages) {
      auto pageStart = p.first & ~kPageSizeMask;
      auto r = munmap(reinterpret_cast<void*>(pageStart), p.second);
      if (r == -1) {
        fprintf(stderr, "munmap() failure for %p, errno = %d\n",
                reinterpret_cast<void*>(p.first), errno);
      }
    }
    addresses.clear();
    pages.clear();
  }

  static void install_signal_handler();
  static void access_handler(int signo, siginfo_t* info, void* extra);

private:
  // Parse the instruction stream to find the next instruction.
  static uint8_t* find_next_inst(uint8_t* ip);

  // Whether the address is in the managed pages..
  auto find_page_containing(void* ptr) {
    if (pages.empty()) return pages.end();
    auto const p = reinterpret_cast<uintptr_t>(ptr);
    auto iter = pages.upper_bound(p);
    if (iter != pages.begin()) --iter;
    if (iter->first > p) {
      abort();
    }
    if (iter->first + iter->second > p) return iter;
    return pages.end();
  }

  // Return whether page protection was set successfully.
  static bool set_page_protection(void* ptr, int prot);

  // Return the value previously at `addr`. It must be different from `byte`.
  static uint8_t patch(uint8_t* addr, uint8_t byte);

private:
  folly::sorted_vector_set<void*> addresses;
  folly::sorted_vector_map<uintptr_t, size_t> pages;

  uint8_t* trapAddr{nullptr};           // the address we just put a trap in.
  uint8_t origByte{0};                  // in the instruction stream

  void* dataPage{nullptr};              // page that was allowed PROT_WRITE

  static struct sigaction oldHandler;
};

extern __thread HeapObjectSanitizer* tl_heap_sanitizer;

}
