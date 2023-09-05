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

#include "hphp/util/alloc-defs.h"
#include "hphp/util/bump-mapper.h"
#include "hphp/util/extent-hooks.h"
#include <limits>
#include <string>

#if USE_JEMALLOC_EXTENT_HOOKS

namespace HPHP { namespace alloc {

////////////////////////////////////////////////////////////////////////////////

/**
 * ManagedArena is a wrapper around jemalloc arena with customized extent hooks.
 * The extent hook is a set of callbacks jemalloc uses to interact with the OS
 * when managing memory mappings.
 *
 * For various purposes, we want to control the properties of the underlying
 * memory in a particular arena, such as address range, physical placement on
 * NUMA nodes, or huge pages.  The extent alloc hook comes in handy for the
 * purpose, and is wrapped in the ExtentAllocator policy class.
 */
template <typename ExtentAllocator>
struct ManagedArena : public ExtentAllocator {
 private:
  // Constructor forwards all arguments. Use `CreateAt()` or `AttachTo()` to
  // create instances.
  template<typename... Args>
  explicit ManagedArena(Args&&... args)
    : ExtentAllocator(std::forward<Args>(args)...) {}

  void create();
  void updateHook();

  ManagedArena(const ManagedArena&) = delete;
  ManagedArena& operator=(const ManagedArena&) = delete;
  // Don't run the destructor, as we are not good at managing arena lifetime.
  ~ManagedArena() = delete;

 public:
  unsigned id() const {
    return m_arenaId;
  }

  void bindCurrentThread() {
    mallctlWrite("thread.arena", id());
  }

  // For stats reporting
  size_t unusedSize();
  std::string reportStats() const;

  template<typename... Args>
  static ManagedArena* CreateAt(void* addr, Args&&... args) {
    auto arena = new (addr) ManagedArena(std::forward<Args>(args)...);
    arena->create();
    arena->updateHook();
    return arena;
  }

  template<typename... Args>
  static ManagedArena* AttachTo(void* addr, unsigned id, Args&&... args) {
    auto arena = new (addr) ManagedArena(std::forward<Args>(args)...);
    arena->m_arenaId = id;
    arena->updateHook();
    return arena;
  }

 protected:
  static constexpr auto kInvalidArena = std::numeric_limits<unsigned>::max();
  unsigned m_arenaId{kInvalidArena};
};

using RangeArena = alloc::ManagedArena<alloc::MultiRangeExtentAllocator>;
using LowArena = RangeArena;
using HighArena = RangeArena;
static_assert(alignof(RangeArena) <= 64, "");
using RangeArenaStorage = std::aligned_storage<sizeof(RangeArena), 64>::type;
extern RangeArenaStorage g_lowerArena;
extern RangeArenaStorage g_lowArena;
extern RangeArenaStorage g_lowColdArena;
extern RangeArenaStorage g_highArena;
extern RangeArenaStorage g_coldArena;

#ifndef MAX_MANAGED_ARENA_COUNT
#define MAX_MANAGED_ARENA_COUNT 16
#endif
static_assert(MAX_MANAGED_ARENA_COUNT >= 1, "");
// All ManagedArena's represented as an array of pair<id, pointer>.  Each
// pointer can be casted to the underlying ExtentAllocator/Arena. We use this
// to access the state of ExtentAllocators in extent hooks.  An id of zero
// indicates an empty entry.  If the arena doesn't have a custom extent hook,
// the arena won't be registered here.
using ArenaArray = std::array<std::pair<unsigned, void*>,
                              MAX_MANAGED_ARENA_COUNT>;
extern ArenaArray g_arenas;
template<typename T> inline T* GetByArenaId(unsigned id) {
  for (auto i : g_arenas) {
    if (i.first == id) {
      return static_cast<T*>(i.second);
    }
  }
  return nullptr;
}

inline LowArena* lowArena() {
  auto p = reinterpret_cast<LowArena*>(&g_lowArena);
  if (p->id()) return p;
  return nullptr;
}

inline HighArena* highArena() {
  auto p = reinterpret_cast<HighArena*>(&g_highArena);
  if (p->id()) return p;
  return nullptr;
}

using PreMappedArena = alloc::ManagedArena<alloc::RangeFallbackExtentAllocator>;

extern PreMappedArena* g_arena0;
extern std::vector<PreMappedArena*> g_local_arenas; // keyed by numa node id

}

using DefaultArena = alloc::ManagedArena<alloc::DefaultExtentAllocator>;

/*
 * Make sure we have at least `count` extra arenas, with the same number of
 * extra arenas for each NUMA node.  Returns whether we have enough arenas to
 * meet the required count.  This function tries to create the extra arenas at
 * the first time it is called with nonzero count.  Subsequent calls won't
 * change the number of extra arenas.
 */
bool setup_extra_arenas(unsigned count);
/*
 * Get the next extra arena on the specified NUMA node.
 */
DefaultArena* next_extra_arena(int node);

}

#endif // USE_JEMALLOC_EXTENT_HOOKS
