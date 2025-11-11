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

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <folly/CPortability.h>
#include "folly/memory/detail/MallocImpl.h"
#include "hphp/util/ptr.h"

#if FOLLY_SANITIZE
// ASan is less precise than valgrind so we'll need a superset of those tweaks
#  define VALGRIND
// TODO: (t2869817) ASan doesn't play well with jemalloc
#  ifdef USE_JEMALLOC
#    undef USE_JEMALLOC
#  endif
#endif

#ifdef USE_JEMALLOC
#include <jemalloc/jemalloc.h>
#if JEMALLOC_VERSION_MAJOR < 5 || (JEMALLOC_VERSION_MAJOR == 5 && JEMALLOC_VERSION_MINOR < 3)
#  error "jemalloc 5.3 is required"
#endif
#endif

namespace HPHP {
constexpr bool use_jemalloc =
#ifdef USE_JEMALLOC
  true
#else
  false
#endif
  ;

// Whether to use custom jemalloc arenas that allocate
// from well-defined address ranges.
// This is incompatible with OSS PIE builds.
constexpr bool use_position_dependent_jemalloc_arenas =
#if USE_JEMALLOC && !defined(HHVM_PIE)
  true
#else
  false
#endif
;

// When we have control over the virtual address space for the heap, all
// static/uncounted strings/arrays have addresses lower than kUncountedMaxAddr,
// and all counted HeapObjects have higher addresses.
constexpr bool addr_encodes_persistency =
#if USE_JEMALLOC && defined(__x86_64__) && defined(__linux__) && !defined(HHVM_PIE)
  true
#else
  false
#endif
  ;

// ASAN modifies the generated code in ways that cause abnormally high C++
// stack usage.
constexpr size_t kStackSizeMinimum =
#if FOLLY_SANITIZE
  16 << 20;
#else
  8 << 20;
#endif

extern const size_t s_pageSize;

// Rounding a value/pointer down to align.
template<size_t align, typename T>
constexpr T rd(T const n) {
  static_assert(std::is_integral<T>::value || std::is_pointer<T>::value, "");
  static_assert(sizeof(T) <= sizeof(uintptr_t), "");
  static_assert((align & (align - 1)) == 0, "");
  return (T)(((uintptr_t)n) & ~(align - 1));
}

// Rounding a value/pointer up to align.
template<size_t align, typename T>
constexpr T ru(T const n) {
  return rd<align, T>(T(((uintptr_t)n) + align - 1));
}

}
