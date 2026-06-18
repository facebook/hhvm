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

#include "hphp/util/perf-cputime-ns.h"

#ifndef HHVM_FACEBOOK

/*
 * On Linux we can read CLOCK_THREAD_CPUTIME_ID entirely in userspace via the
 * perf_events interface.  This requires a CPU whose cycle counter we know how
 * to read (see cpuCycles() in cycles.h) and a kernel new enough to advertise
 * userspace timekeeping (cap_user_time); we report failure otherwise so the
 * caller falls back to clock_gettime().
 */
#if defined(__linux__) && (defined(__x86_64__) || defined(__aarch64__))
#define HHVM_USE_PERF_THREAD_CPUTIME 1
#endif

#ifdef HHVM_USE_PERF_THREAD_CPUTIME

#include <errno.h>
#include <fcntl.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>

#include <folly/String.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>
#include <folly/system/AtFork.h>

#include "hphp/util/alloc-defs.h"
#include "hphp/util/cycles.h"
#include "hphp/util/logger.h"
#include "hphp/util/safe-cast.h"

namespace HPHP {
namespace {
///////////////////////////////////////////////////////////////////////////////
// Userspace CLOCK_THREAD_CPUTIME_ID via perf_events.
//
// We open a PERF_COUNT_SW_TASK_CLOCK software event scoped to the calling
// thread.  Its `time_enabled` field, exposed on the mmap'd perf_event_mmap_page,
// is the thread's accumulated on-CPU time in nanoseconds.  When the kernel
// advertises cap_user_time, the page also exports the constants needed to
// project that value forward to "now" using the raw cycle counter -- exactly as
// the kernel does internally -- so we can compute the current thread CPU time
// without a syscall.  This mirrors the read loop in hardware-counter.cpp.

// Compiler/CPU barrier bracketing the seqlock read, matching the one in
// hardware-counter.cpp.  x86_64 is strongly ordered so a compiler barrier
// suffices; aarch64 needs an explicit memory barrier.
#if defined(__x86_64__)
#define perf_barrier() __asm__ volatile("" ::: "memory")
#elif defined(__aarch64__)
#define perf_barrier() asm volatile("dmb ish" ::: "memory")
#endif

// Per-thread perf state.  m_fd < 0 means "not yet initialized"; m_meta == null
// after init means the fast path is unavailable and we should use the syscall.
struct PerfThreadClock {
  ~PerfThreadClock() {
    if (m_meta) {
      munmap(m_meta, s_pageSize);
      folly::AtFork::unregisterHandler(this);
    }
    if (m_fd >= 0) ::close(m_fd);
  }

  // Lazily open the event and map its page.  Returns the page on success, or
  // nullptr if the fast path is unavailable (and won't become available).
  perf_event_mmap_page* init() {
    if (m_fd >= 0) return m_meta;

    perf_event_attr pe{};
    pe.type = PERF_TYPE_SOFTWARE;
    pe.config = PERF_COUNT_SW_TASK_CLOCK;
    pe.size = sizeof(pe);
    // Exclude nothing: CLOCK_THREAD_CPUTIME_ID counts both user and kernel
    // time spent on behalf of the thread.

    // pid == 0 -> calling thread, cpu == -1 -> follow it across CPUs.
    auto const ret = syscall(__NR_perf_event_open, &pe, 0, -1, -1, PERF_FLAG_FD_CLOEXEC);
    if (ret < 0) {
      Logger::Verbose("perf thread clock: perf_event_open failed with: %s",
                      folly::errnoStr(errno).c_str());
      // Leave m_fd >= 0 so we don't retry the syscall on every read.
      m_fd = kDisabled;
      return nullptr;
    }
    m_fd = safe_cast<int>(ret);

    auto const base = mmap(nullptr, s_pageSize, PROT_READ, MAP_SHARED, m_fd, 0);
    if (base == MAP_FAILED) {
      Logger::Verbose("perf thread clock: failed to mmap perf_event: %s",
                      folly::errnoStr(errno).c_str());
      return nullptr;
    }
    auto const meta = static_cast<perf_event_mmap_page*>(base);

    // Reset resources in fork()'d children.
    folly::AtFork::registerHandler(
      this,
      // prepare
      {},
      // parent
      {},
      // child
      [this](){
        m_fd = -1;
        m_meta = nullptr;
      }
    );

    // Without cap_user_time the kernel won't give us the constants to
    // extrapolate from the cycle counter; the page is useless to us.
    if (!meta->cap_user_time) {
      Logger::Verbose("perf thread clock: cap_user_time unavailable");
      munmap(meta, s_pageSize);
      return nullptr;
    }

    m_meta = meta;
    return m_meta;
  }

  static constexpr int kDisabled = -2;
  int m_fd{-1};
  perf_event_mmap_page* m_meta{nullptr};
};

thread_local PerfThreadClock t_threadClock;

///////////////////////////////////////////////////////////////////////////////
}
}

// Read the current thread's CPU time in nanoseconds via perf.  Returns 0 and
// writes *ns on success, or non-zero if the fast path is unavailable.
int fb_perf_get_thread_cputime_ns(uint64_t* ns) {
  auto const meta = HPHP::t_threadClock.init();
  if (!meta) return -1;

  uint32_t seq;
  uint64_t enabled, cyc, time_offset;
  uint32_t time_mult;
  uint16_t time_shift;
  uint64_t time_cycles = 0, time_mask = ~0ull;
  bool short_time;

  // Seqlock retry loop: the kernel bumps `lock` around updates from the
  // context-switch path, so reread until we observe a stable snapshot.
  do {
    seq = meta->lock;
    perf_barrier();

    // For a task-bound software event there is no PMU multiplexing, so
    // time_enabled == time_running and both accrue only while this thread is
    // on-CPU -- i.e. exactly CLOCK_THREAD_CPUTIME_ID.  We extrapolate
    // time_enabled because the kernel's documented formula only projects the
    // cycle delta onto running when a hardware counter index is live, which a
    // software event never has.
    enabled = meta->time_enabled;

    cyc = HPHP::cpuCycles();
    time_offset = meta->time_offset;
    time_mult = meta->time_mult;
    time_shift = meta->time_shift;

    // cap_user_time_short means the hardware cycle counter is narrower than
    // 64 bits and must be folded against the kernel's last-seen value.
    short_time = meta->cap_user_time_short;
    if (short_time) {
      time_cycles = meta->time_cycles;
      time_mask = meta->time_mask;
    }

    perf_barrier();
  } while (meta->lock != seq);

  if (short_time) {
    cyc = time_cycles + ((cyc - time_cycles) & time_mask);
  }

  // Project `enabled` forward by the elapsed cycles, per the formula the
  // kernel documents in <linux/perf_event.h>.  The calling thread is by
  // definition currently scheduled, so the full delta has accrued as CPU time.
  auto const quot = cyc >> time_shift;
  auto const rem = cyc & ((uint64_t{1} << time_shift) - 1);
  auto const delta = time_offset + quot * time_mult +
    ((rem * time_mult) >> time_shift);

  *ns = enabled + delta;
  return 0;
}

#undef perf_barrier

#else // !HHVM_USE_PERF_THREAD_CPUTIME

// No userspace fast path on this platform; always fall back to clock_gettime().
int fb_perf_get_thread_cputime_ns(uint64_t* /*ns*/) {
  return -1;
}

#endif

#endif // !HHVM_FACEBOOK
