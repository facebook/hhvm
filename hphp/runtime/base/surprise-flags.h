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

#include "hphp/runtime/base/rds-header.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Mask of the bits in stackLimitAndSurprise that actually store surprise
 * flags, and the bits that store the stack base pointer.
 */
auto constexpr kSurpriseFlagMask      = 0xffff000000000000ull;
auto constexpr kSurpriseFlagStackMask = 0x0000ffffffffffffull;

/*
 * Surprise flags are stored in the upper 16-bits of the stackLimitAndSurprise
 * in the rds header, so they must be above 2^48.
 */
enum SurpriseFlag : size_t {
  MemExceededFlag      = 1ull << 48,
  TimedOutFlag         = 1ull << 49,
  SignaledFlag         = 1ull << 50,
  EventHookFlag        = 1ull << 51,
  PendingExceptionFlag = 1ull << 52,
  InterceptFlag        = 1ull << 53,
  XenonSignalFlag      = 1ull << 54,
  AsyncEventHookFlag   = 1ull << 55,

  /* Set by the debugger to break out of loops in translated code. */
  DebuggerSignalFlag   = 1ull << 56,

  /* Set by the debugger hook handler to force function entry/exit events. */
  DebuggerHookFlag     = 1ull << 57,

  HeapSamplingFlag     = 1ull << 58,

  IntervalTimerFlag    = 1ull << 59,

  /* Set if a GC should be run at the next safe point. */
  PendingGCFlag        = 1ull << 60,

  /* Set when memory threshold exceeds a PHP specified limit */
  MemThresholdFlag     = 1ull << 61,

  /*
   * Set if there are perf events waiting to be consumed.
   */
  PendingPerfEventFlag = 1ull << 62,

  /* Set when executing a CLI-server request and the client has vanished. */
  CLIClientTerminated = 1ull << 63,

  /*
   * Flags that shouldn't be cleared by fetchAndClearSurpriseFlags, because
   * fetchAndClearSurpriseFlags is only supposed to touch flags related to
   * PHP-visible signals/exceptions and resource limits.
   */
  ResourceFlags =
    MemExceededFlag |
    TimedOutFlag |
    PendingGCFlag |
    PendingPerfEventFlag,

  StickyFlags =
    AsyncEventHookFlag |
    DebuggerHookFlag |
    DebuggerSignalFlag |
    EventHookFlag |
    InterceptFlag |
    XenonSignalFlag |
    HeapSamplingFlag |
    IntervalTimerFlag |
    MemThresholdFlag |
    CLIClientTerminated |
    ResourceFlags,

  /*
   * Flags that should only be checked at MemoryManager safe points.
   */
  SafepointFlags =
    PendingGCFlag |
    PendingPerfEventFlag,

  NonSafepointFlags = ~SafepointFlags & kSurpriseFlagMask,
};

//////////////////////////////////////////////////////////////////////

/*
 * Code within this scope must never handle any of the specified flags, and is
 * furthermore not even allowed to check for them using, e.g. `getSurpriseFlag',
 * regardless of whether they actually are set.
 */
struct NoHandleSurpriseScope {
#ifndef NDEBUG
  static void AssertNone(SurpriseFlag flags);
  explicit NoHandleSurpriseScope(SurpriseFlag flags);
  ~NoHandleSurpriseScope();

  private:
  SurpriseFlag m_flags;
#else
  // Compiles to nothing in release mode.
  static void AssertNone(SurpriseFlag) {}
  explicit NoHandleSurpriseScope(SurpriseFlag) {}
  ~NoHandleSurpriseScope() {}
#endif
};

inline std::atomic<size_t>& stackLimitAndSurprise() {
  return rds::header()->stackLimitAndSurprise;
}

inline bool checkSurpriseFlags() {
  auto const val = stackLimitAndSurprise().load(std::memory_order_acquire);
  auto constexpr all = static_cast<SurpriseFlag>(kSurpriseFlagMask);
  NoHandleSurpriseScope::AssertNone(all);
  return val & kSurpriseFlagMask;
}

inline void setSurpriseFlag(SurpriseFlag flag) {
  assertx(flag >= 1ull << 48);
  stackLimitAndSurprise().fetch_or(flag);
}

inline bool getSurpriseFlag(SurpriseFlag flag) {
  assertx(flag >= 1ull << 48);
  NoHandleSurpriseScope::AssertNone(flag);
  return stackLimitAndSurprise().load() & flag;
}

inline void clearSurpriseFlag(SurpriseFlag flag) {
  stackLimitAndSurprise().fetch_and(~flag);
}

inline size_t fetchAndClearSurpriseFlags() {
  return stackLimitAndSurprise().
    fetch_and(StickyFlags | kSurpriseFlagStackMask) & kSurpriseFlagMask;
}

///////////////////////////////////////////////////////////////////////////////
}

