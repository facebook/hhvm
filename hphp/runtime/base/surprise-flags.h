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
#include <cstddef>

#include "hphp/util/assertions.h"

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
  /*
   * Remark: bit 53 is the InterceptFlag and is relied upon by the legacy,
   * surprise flag based, implementation of fb_intercept2.  Once the legacy
   * implementation is removed bit 53 can be reused for other purposes.
   */
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

  ResourceFlags =
    MemExceededFlag |
    TimedOutFlag |
    PendingGCFlag |
    PendingPerfEventFlag,

  /*
   * Flags that should only be checked at MemoryManager safe points.
   */
  SafepointFlags =
    PendingGCFlag |
    PendingPerfEventFlag,

  NonSafepointFlags = ~SafepointFlags & kSurpriseFlagMask,

  /*
   * Profiling and monitoring flags that should be masked during debugger
   * stepping. This ensures that Xenon sampling doesn't interfere with
   * debugger functionality.
   */
  ProfilingMonitoringFlags =
    XenonSignalFlag |
    HeapSamplingFlag |
    IntervalTimerFlag,

  /*
   * Flags that should be processed during debugger stepping. Excludes
   * profiling/monitoring to avoid stepping into Xenon serialization.
   */
  DebuggerSteppingFlags = kSurpriseFlagMask & ~ProfilingMonitoringFlags,
};

//////////////////////////////////////////////////////////////////////

/*
 * Code within this scope must never handle any of the specified flags, and is
 * furthermore not even allowed to check for them using, e.g. `hasSurprise()`,
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

//////////////////////////////////////////////////////////////////////

/*
 * Combination of surprise flags and the limit (lowest address) of the
 * evaluation stack.  May be written to by other threads.
 *
 * At various points, the runtime will check whether this word contains a
 * higher number than what it believes the evaluation stack needs to be
 * (remember the eval stack grows down), which combines a stack overflow
 * check and a check for unusual conditions.  If this check triggers, the
 * runtime will do more detailed checks to see if it's actually dealing with
 * a stack overflow, or a surprise condition.
 *
 * All the surprise flag bits are in the upper 16 bits of this value, which
 * must be zero if it is actually a pointer to the lowest address of the
 * evaluation stack (the normal, "unsurprised" situation)---if one of the
 * surprise flags is set, the pointer will be higher than any legal eval
 * stack pointer and we'll go to a slow path to handle possible unusual
 * conditions (e.g. OOM).  (This is making use of the x64 property that
 * "canonical form" addresses have all their upper bits the same as bit 47,
 * and that this is zero for linux userland pointers.)
 */
struct StackLimitAndSurpriseFlags {

  /*
   * RAII helper that atomically clears a set of surprise flags on
   * construction and restores exactly the ones that were originally set
   * on destruction.  Used, e.g., to temporarily mask ResourceFlags while
   * handling an exception in program-functions.cpp.
   */
  struct FlagSaver {
    FlagSaver(StackLimitAndSurpriseFlags& owner, SurpriseFlag flags)
      : m_owner(owner)
      , m_saved(owner.m_value.fetch_and(~flags) & flags) {}

    ~FlagSaver() {
      if (m_saved) m_owner.m_value.fetch_or(m_saved);
    }

    FlagSaver(const FlagSaver&) = delete;
    FlagSaver& operator=(const FlagSaver&) = delete;

  private:
    StackLimitAndSurpriseFlags& m_owner;
    size_t m_saved;
  };

  /*
   * Store the initial stack limit.  The stack limit occupies the lower 48 bits,
   * so the value must not have any of the upper surprise-flag bits set.
   */
  void initialize(size_t stackLimit) {
    assertx(!(stackLimit & kSurpriseFlagMask));
    m_value.store(stackLimit, std::memory_order_release);
  }

  bool hasSurprise() const {
    auto constexpr all = static_cast<SurpriseFlag>(kSurpriseFlagMask);
    NoHandleSurpriseScope::AssertNone(all);
    return m_value.load(std::memory_order_acquire) & kSurpriseFlagMask;
  }

  void setFlag(SurpriseFlag flag) {
    assertx(flag >= 1ull << 48);
    m_value.fetch_or(flag);
  }

  void clearFlag(SurpriseFlag flag) {
    assertx(flag >= 1ull << 48);
    m_value.fetch_and(~flag);
  }

  bool getFlag(SurpriseFlag flag) const {
    assertx(flag >= 1ull << 48);
    NoHandleSurpriseScope::AssertNone(flag);
    return m_value.load() & flag;
  }

  size_t fetchFlags() const {
    return m_value.load(std::memory_order_acquire) & kSurpriseFlagMask;
  }

  void clearFlags() {
    m_value.fetch_and(kSurpriseFlagStackMask, std::memory_order_acq_rel);
  }

  /*
   * Offset of the raw atomic word from the start of the struct.  Used by
   * the JIT to compute kSurpriseFlagsOff.  m_value must be the first (and
   * only) data member so that this is always 0.
   */
  static constexpr ptrdiff_t valueOffset() {
    return offsetof(StackLimitAndSurpriseFlags, m_value);
  }

private:
  std::atomic<size_t> m_value;
};

///////////////////////////////////////////////////////////////////////////////
}
