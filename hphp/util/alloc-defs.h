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
#ifndef incl_HPHP_COMPILATION_ALLOC_FLAGS_H
#define incl_HPHP_COMPILATION_ALLOC_FLAGS_H

#include <cstddef>

#include <folly/CPortability.h>
#include "hphp/util/low-ptr-def.h"

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
#  if (JEMALLOC_VERSION_MAJOR >= 5) && defined(USE_LOWPTR) && \
  defined(__linux__) && !defined(USE_JEMALLOC_EXTENT_HOOKS)
#    define USE_JEMALLOC_EXTENT_HOOKS 1
#  endif
#  if USE_JEMALLOC_EXTENT_HOOKS
#    if (JEMALLOC_VERSION_MAJOR > 5) || (JEMALLOC_VERSION_MINOR >= 1)
#       define JEMALLOC_METADATA_1G_PAGES 1
#    endif
#  endif
#  if (JEMALLOC_VERSION_MAJOR > 4)
#    define JEMALLOC_NEW_ARENA_CMD "arenas.create"
#  else
#    define JEMALLOC_NEW_ARENA_CMD "arenas.extend"
#  endif
#endif

namespace HPHP {
constexpr bool use_jemalloc =
#ifdef USE_JEMALLOC
  true
#else
  false
#endif
  ;

// When we have control over the virtual address space for the heap, all
// static/uncounted strings/arrays have addresses lower than kUncountedMaxAddr,
// and all counted HeapObjects have higher addresses.
constexpr bool addr_encodes_persistency =
#if USE_JEMALLOC_EXTENT_HOOKS && defined(__x86_64__) && defined(__linux__)
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

}

#endif
