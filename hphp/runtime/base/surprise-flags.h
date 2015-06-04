/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_RUNTIME_SURPRISE_FLAGS_H_
#define incl_HPHP_RUNTIME_SURPRISE_FLAGS_H_

#include "hphp/runtime/base/rds-header.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Surprise flags are stored in the upper 16-bits of the stackLimitAndSurprise
 * in the rds header, so they must be above 2^48.
 */
enum SurpriseFlag : size_t {
  MemExceededFlag      = 1ul << 48,
  TimedOutFlag         = 1ul << 49,
  SignaledFlag         = 1ul << 50,
  EventHookFlag        = 1ul << 51,
  PendingExceptionFlag = 1ul << 52,
  InterceptFlag        = 1ul << 53,
  XenonSignalFlag      = 1ul << 54,
  AsyncEventHookFlag   = 1ul << 55,

  /* Set by the debugger to break out of loops in translated code. */
  DebuggerSignalFlag   = 1ul << 56,

  /* Set by the debugger hook handler to force function entry/exit events. */
  DebuggerHookFlag     = 1ul << 57,

  CPUTimedOutFlag      = 1ul << 58,
  IntervalTimerFlag    = 1ul << 59,

  /*
   * Flags that shouldn't be cleared by fetchAndClearSurpriseFlags, because
   * fetchAndClearSurpriseFlags is only supposed to touch flags related to
   * PHP-visible signals/exceptions and resource limits.
   */
  ResourceFlags = MemExceededFlag | TimedOutFlag | CPUTimedOutFlag,
  StickyFlags =
    AsyncEventHookFlag |
    DebuggerHookFlag |
    EventHookFlag |
    InterceptFlag |
    XenonSignalFlag |
    IntervalTimerFlag |
    ResourceFlags,
};

/*
 * Mask of the bits in stackLimitAndSurprise that actually store surprise
 * flags, and the bits that store the stack base pointer.
 */
auto constexpr kSurpriseFlagMask      = 0xffff000000000000ull;
auto constexpr kSurpriseFlagStackMask = 0x0000ffffffffffffull;

//////////////////////////////////////////////////////////////////////

inline std::atomic<size_t>& stackLimitAndSurprise() {
  return rds::header()->stackLimitAndSurprise;
}

inline bool checkSurpriseFlags() {
  auto const val = stackLimitAndSurprise().load(std::memory_order_acquire);
  return val & kSurpriseFlagMask;
}

inline void setSurpriseFlag(SurpriseFlag flag) {
  assertx(flag >= 1ul << 48);
  stackLimitAndSurprise().fetch_or(flag);
}

inline bool getSurpriseFlag(SurpriseFlag flag) {
  assertx(flag >= 1ul << 48);
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

#endif
