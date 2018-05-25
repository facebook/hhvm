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

#ifndef incl_HPHP_UTIL_MANAGED_ARENA_H_
#define incl_HPHP_UTIL_MANAGED_ARENA_H_

#include "hphp/util/alloc.h"

#if USE_JEMALLOC_EXTENT_HOOKS

#include "hphp/util/bump-mapper.h"
#include "hphp/util/extent-hooks.h"
#include <string>

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
  // Constructor forwards all arguments.  The only correct way to create a
  // ManagedArena is `CreateAt()` on statically allocated memory.
  template<typename... Args>
  explicit ManagedArena(Args&&... args)
    : ExtentAllocator(std::forward<Args>(args)...) {
    init();
  }
  // Create the arena and set up hooks.
  void init();

  ManagedArena(const ManagedArena&) = delete;
  ManagedArena& operator=(const ManagedArena&) = delete;
  // Don't run the destructor, as we are not good at managing arena lifetime.
  ~ManagedArena() = delete;

 public:
  inline unsigned id() const {
    return m_arenaId;
  }

  // For stats reporting
  size_t unusedSize();
  std::string reportStats();

  template<typename... Args>
  static ManagedArena* CreateAt(void* addr, Args&&... args) {
    return new (addr) ManagedArena(std::forward<Args>(args)...);
  }

 protected:
  unsigned m_arenaId{0};
};

// Eventually we'd just call it LowArena, when we kill the current low_arena.
using LowArena = alloc::ManagedArena<alloc::BumpExtentAllocator>;
using HighArena = alloc::ManagedArena<alloc::BumpExtentAllocator>;
// Not using std::aligned_storage<> because zero initialization can be useful.
extern uint8_t g_lowArena[sizeof(LowArena)];
extern uint8_t g_highArena[sizeof(HighArena)];

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

}}

#endif // USE_JEMALLOC_EXTENT_HOOKS
#endif
