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
///////////////////////////////////////////////////////////////////////////////

enum SurpriseFlag : ssize_t {
  MemExceededFlag      = 1 << 0,
  TimedOutFlag         = 1 << 1,
  SignaledFlag         = 1 << 2,
  EventHookFlag        = 1 << 3,
  PendingExceptionFlag = 1 << 4,
  InterceptFlag        = 1 << 5,
  XenonSignalFlag      = 1 << 6,
  AsyncEventHookFlag   = 1 << 7,

  /* Set by the debugger to break out of loops in translated code. */
  DebuggerSignalFlag   = 1 << 8,

  /* Set by the debugger hook handler to force function entry/exit events. */
  DebuggerHookFlag     = 1 << 9,

  CPUTimedOutFlag      = 1 << 10,
  IntervalTimerFlag    = 1 << 11,
  LastSurpriseFlag     = IntervalTimerFlag,

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

//////////////////////////////////////////////////////////////////////

inline std::atomic<ssize_t>& surpriseFlags() {
  return rds::header()->surpriseFlags;
}

inline void setSurpriseFlag(SurpriseFlag flag) {
  surpriseFlags().fetch_or(flag);
}

inline bool getSurpriseFlag(SurpriseFlag flag) {
  return surpriseFlags().load() & flag;
}

inline void clearSurpriseFlag(SurpriseFlag flag) {
  surpriseFlags().fetch_and(~flag);
}

inline ssize_t fetchAndClearSurpriseFlags() {
  return surpriseFlags().fetch_and(StickyFlags);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif
