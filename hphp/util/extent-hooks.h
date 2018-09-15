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

#ifndef incl_HPHP_UTIL_EXTENT_HOOK_H_
#define incl_HPHP_UTIL_EXTENT_HOOK_H_

#include "hphp/util/alloc.h"

#if USE_JEMALLOC_EXTENT_HOOKS

#include "hphp/util/bump-mapper.h"

/*
 * Recent versions of jemalloc (specifically, jemalloc 5.x here) allow us to
 * write hooks that specifiy how an arena interacts with the OS.  We use the
 * mechanism to customize the arena for various purposes, for example,
 * limiting the address ranges, managing huge pages, logging, etc.
 *
 * A limitation in the current implementation is that we only do things in the
 * alloc hook, and ignore everything else.  As a result, we are unable to
 * reclaim memory given to the arena, even when the arena is ready to give it
 * back to the system.  Therefore, this should only be used in arenas where the
 * total size of allocated memory doesn't decrease significantly where the
 * server is running.
 */

namespace HPHP { namespace alloc {

/**
 * Extent hooks that do bump mapping for ManagedArena.
 */
struct BumpExtentAllocator : private BumpAllocState {
  // Both highAddr and maxCap should be 2M-aligned.
  BumpExtentAllocator(uintptr_t highAddr, size_t maxCap,
                      LockPolicy p, BumpMapper* mapper);

  using BumpAllocState::mappedSize;
  using BumpAllocState::allocatedSize;
  using BumpAllocState::maxCapacity;

  static void* extent_alloc(extent_hooks_t* extent_hooks, void* addr,
                            size_t size, size_t alignment, bool* zero,
                            bool* commit, unsigned arena_ind);

  // The hook passed to the underlying arena upon creation.
  static extent_hooks_t s_hooks;

  BumpMapper* const m_mapper;
};

}}

#endif // USE_JEMALLOC_EXTENT_HOOKS
#endif // incl_HPHP_UTIL_EXTENT_HOOK_H_
