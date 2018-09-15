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

#include "hphp/util/perf-event.h"

#if defined(__linux__) && defined(__x86_64__) && defined(FACEBOOK)

#include "hphp/util/assertions.h"
#include "hphp/util/logger.h"
#include "hphp/util/safe-cast.h"

#include <folly/FileUtil.h>
#include <folly/Optional.h>
#include <folly/String.h>

#include <mutex>
#include <string>

#include <asm/unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

// These two files must be included in this relative order, because the latter
// transitively includes a local copy of the former unless it detects that the
// system version has already been included.
#include <linux/perf_event.h>
#include <perfmon/pfmlib_perf_event.h>

namespace HPHP {

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Process initialization bit and lock.
 */
bool s_did_init = false;
std::mutex s_init_lock;

/*
 * Page size.
 */
size_t s_pagesz = 0;

/*
 * Microarch-dependent event names for perf's cpu/mem-{loads,stores}/ events,
 * in a form understood by libpfm4.
 *
 * We could just encode the `config' for perf_event_attr ourselves, but libpfm4
 * does other things for us, like set the exclusion bits, and the encoding is
 * not well-specified in the first place.  Instead, it just means we had to
 * match some bits to names ahead of time.
 *
 * These may be altered when the module is initialized.
 */
// On Haswell and later, this is called "LOAD_LATENCY".
const char* s_mem_loads = "MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD";
// On Haswell and later, "MEM_UOPS_RETIRED:ALL_STORES" is used instead.
const char* s_mem_stores = "MEM_TRANS_RETIRED:PRECISE_STORE";

///////////////////////////////////////////////////////////////////////////////

/*
 * Metadata for a fully set up perf_event.
 */
struct perf_event_handle {
  perf_event_handle() {}
  perf_event_handle(int fd, struct perf_event_mmap_page* meta)
    : fd(fd)
    , meta(meta)
  {}

  // File descriptor of the opened perf_event.
  int fd{-1};

  // Metadata header page, followed by the ring buffer for samples.
  struct perf_event_mmap_page* meta{nullptr};

  // Buffer for samples that wrap around.
  char* buf{nullptr};
  size_t buf_sz{0};
};

/*
 * Per-thread perf_event metadata.
 */
thread_local struct {
  perf_event_handle loads;
  perf_event_handle stores;
  perf_event_signal_fn_t signal;
} tl_perf_event = {};

/*
 * Ensure that this module is properly initialized.
 *
 * Returns true if the module has been initialized successfully (by anyone),
 * else false.
 */
bool perf_event_init() {
  if (s_did_init) return true;

  std::lock_guard<std::mutex> l(s_init_lock);
  if (s_did_init) return true;

  s_pagesz = sysconf(_SC_PAGESIZE);

  std::string event_str;
  if (folly::readFile("/sys/devices/cpu/events/mem-stores", event_str)) {
    // If the read fails, we'll stick with the {Sandy,Ivy}Bridge event name.
    // Otherwise, check for the Haswell encoding string.
    //
    // @see: linux/arch/x86/events/intel/core.c.
    if (event_str == "event=0xd0,umask=0x82") {
      s_mem_stores = "MEM_UOPS_RETIRED:ALL_STORES";
    }
    // `event_str' should be "event=0xcd,umask=0x2" on *Bridge, but we don't
    // care since we're using that event name as our default.
  }

  // libpfm4 needs to be initialized exactly once per process lifetime.
  auto const pfmr = pfm_initialize();
  if (pfmr != PFM_SUCCESS) {
    Logger::Warning("perf_event: pfm_initialize failed: %s",
                    pfm_strerror(pfmr));
    return false;
  }
  s_did_init = true;
  return true;
}

/*
 * Size of the mmap'd perf_event output ring buffer.
 *
 * Must be exactly 2^n pages for some `n' (or 1 + 2^n, if we include the
 * perf_event header page).
 */
size_t buffer_sz() { return s_pagesz * (1 << 5); }  // ring buffer only
size_t mmap_sz() { return s_pagesz + buffer_sz(); } // with header

///////////////////////////////////////////////////////////////////////////////

/*
 * Register that a perf event was generated.
 */
void signal_event(int sig, siginfo_t* info, void* /*context*/) {
  if (sig != SIGIO || info == nullptr) return;

  // Older versions of Linux have SIGIO here; newer versions have POLLIN.
  if (info->si_code != SIGIO && info->si_code != POLLIN) return;
  // We only care about read signals.
  if ((info->si_band & POLLERR) || (info->si_band & POLLNVAL)) return;
  if (!(info->si_band & POLLIN)) return;

  if (tl_perf_event.signal == nullptr) return;

  auto const type = [&]() -> folly::Optional<PerfEvent> {
    if (info->si_fd == tl_perf_event.loads.fd)  return PerfEvent::Load;
    if (info->si_fd == tl_perf_event.stores.fd) return PerfEvent::Store;
    return folly::none;
  }();
  if (!type) return;

  tl_perf_event.signal(*type);
}

/*
 * Install `signal_event' to notify the user of new perf_event samples.
 *
 * Returns true if the handler was successfully installed, else false.  If a
 * handler for SIGIO was already installed, this will fail.  Otherwise, if we
 * install `signal_event' successfully, SIGIO will be unconditionally unblocked
 * for the calling thread.
 */
bool install_sigio_handler() {
  struct sigaction old_action;

  if (sigaction(SIGIO, nullptr, &old_action) < 0) {
    Logger::Warning("perf_event: could not install SIGIO handler: %s",
                    folly::errnoStr(errno).c_str());
    return false;
  }

  // Fail if a competing SIGIO handler is found.
  if (old_action.sa_handler != SIG_DFL &&
      old_action.sa_handler != SIG_IGN &&
      old_action.sa_sigaction != signal_event) {
    Logger::Warning("perf_event: could not install SIGIO handler: "
                    "found existing handler");
    return false;
  }

  // Install our signal handler for SIGIO.
  struct sigaction action = {};
  action.sa_sigaction = signal_event;
  action.sa_flags = SA_SIGINFO;

  if (sigaction(SIGIO, &action, nullptr) < 0) {
    Logger::Warning("perf_event: could not install SIGIO handler: %s",
                    folly::errnoStr(errno).c_str());
    return false;
  }

  // Ensure that SIGIO is unblocked.
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, SIGIO);
  if (pthread_sigmask(SIG_UNBLOCK, &sigs, nullptr) < 0) {
    Logger::Warning("perf_event: could not unblock SIGIO: %s",
                    folly::errnoStr(errno).c_str());
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Pause or resume an event.
 */
void pause_event(const perf_event_handle& pe) {
  ioctl(pe.fd, PERF_EVENT_IOC_DISABLE, 0);
}
void resume_event(const perf_event_handle& pe) {
  ioctl(pe.fd, PERF_EVENT_IOC_ENABLE, 0);
}

/*
 * Logically delete all events that are currently buffered for `pe'.
 */
void clear_events(const perf_event_handle& pe) {
  auto const data_head = pe.meta->data_head;
  __sync_synchronize(); // smp_mb()
  pe.meta->data_tail = data_head;
}

/*
 * Disable and close a perf event.
 */
void close_event(const perf_event_handle& pe) {
  clear_events(pe);
  free(pe.buf);
  ioctl(pe.fd, PERF_EVENT_IOC_DISABLE, 0);
  munmap(pe.meta, mmap_sz());
  close(pe.fd);
}

/*
 * Open a file descriptor for perf events with `event_name', mmap it, and set
 * things up so that the calling thread receives SIGIO signals from it.
 *
 * Returns the perf_event_handle on success, else folly::none.
 */
folly::Optional<perf_event_handle> enable_event(const char* event_name,
                                                uint64_t sample_freq) {
  struct perf_event_attr attr = {};
  pfm_perf_encode_arg_t arg = {};
  arg.attr = &attr;
  arg.size = sizeof(arg);

  // Populate the `type', `config', and `exclude_*' members on `attr'.
  auto const pfmr = pfm_get_os_event_encoding(event_name, PFM_PLM3,
                                              PFM_OS_PERF_EVENT, &arg);
  if (pfmr != PFM_SUCCESS) {
    Logger::Warning("perf_event: failed to get encoding for %s: %s",
                    event_name, pfm_strerror(pfmr));
    return folly::none;
  }

  // Finish setting up `attr' and open the event.
  attr.size = sizeof(attr);
  attr.disabled = 1;
  attr.sample_freq = sample_freq;
  attr.freq = 1;
  attr.watermark = 0;
  attr.wakeup_events = 1;
  attr.precise_ip = 2;  // request zero skid

  attr.sample_type = PERF_SAMPLE_IP
                   | PERF_SAMPLE_TID
                   | PERF_SAMPLE_ADDR
                   | PERF_SAMPLE_CALLCHAIN
                   | PERF_SAMPLE_DATA_SRC
                   ;

  auto const ret = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
  if (ret < 0) {
    // Some machines might not have PEBS support (needed for precise_ip > 0),
    // but then PERF_SAMPLE_ADDR will always return zeros instead of the target
    // memory address.  Just fail silently in this case.
    Logger::Verbose("perf_event: perf_event_open failed with: %s",
                    folly::errnoStr(errno).c_str());
    return folly::none;
  }
  auto const fd = safe_cast<int>(ret);

  // Recent versions of Linux have a CLOEXEC flag for perf_event_open(), but
  // use fcntl() for portability.  Note that since we do this after we open the
  // event, this could in theory race with an exec() from another thread---but
  // that shouldn't be happening anyway.
  fcntl(fd, F_SETFD, O_CLOEXEC);

  // Make sure that any SIGIO sent from `fd' is handled by the calling thread.
  f_owner_ex owner;
  owner.type = F_OWNER_TID;
  owner.pid = syscall(__NR_gettid);

  // Set up `fd' to send SIGIO with sigaction info.
  if (fcntl(fd, F_SETFL, O_ASYNC) < 0 ||
      fcntl(fd, F_SETSIG, SIGIO) < 0 ||
      fcntl(fd, F_SETOWN_EX, &owner) < 0) {
    Logger::Warning("perf_event: failed to set up asynchronous I/O: %s",
                    folly::errnoStr(errno).c_str());
    close(fd);
    return folly::none;
  }

  // Map the ring buffer for our samples.
  auto const base = mmap(nullptr, mmap_sz(), PROT_READ | PROT_WRITE,
                         MAP_SHARED, fd, 0);
  if (base == MAP_FAILED) {
    Logger::Warning("perf_event: failed to mmap perf_event: %s",
                    folly::errnoStr(errno).c_str());
    close(fd);
    return folly::none;
  }
  auto const meta = reinterpret_cast<struct perf_event_mmap_page*>(base);

  auto const pe = perf_event_handle { fd, meta };

  // Reset the event.  This seems to be present in most examples, but it's
  // unclear if it's necessary or just good hygeine.  (It's possible that it's
  // necessary on successive opens.)
  if (ioctl(fd, PERF_EVENT_IOC_RESET, 0) < 0) {
    Logger::Warning("perf_event: failed to reset perf_event: %s",
                    folly::errnoStr(errno).c_str());
    close_event(pe);
    return folly::none;
  }

  // Enable the event.  The man page and other examples of usage all suggest
  // that the right thing to do is to start with the event disabled and then
  // enable it manually afterwards, so we do the same here even though it seems
  // strange and circuitous.
  if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
    Logger::Warning("perf_event: failed to enable perf_event: %s",
                    folly::errnoStr(errno).c_str());
    close_event(pe);
    return folly::none;
  }

  return pe;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Ensure that `pe.buf' can hold at least `cap' bytes.
 */
void ensure_buffer_capacity(perf_event_handle& pe, size_t cap) {
  if (pe.buf_sz >= cap) return;
  free(pe.buf);
  pe.buf = reinterpret_cast<char*>(malloc(cap * 2));
}

/*
 * Iterate through all the pending sampled events in `pe' and pass each one to
 * `consume'.
 */
void consume_events(PerfEvent kind, perf_event_handle& pe,
                    perf_event_consume_fn_t consume) {
  auto const data_tail = pe.meta->data_tail;
  auto const data_head = pe.meta->data_head;

  asm volatile("" : : : "memory"); // smp_rmb()
  if (data_head == data_tail) return;

  auto const base = reinterpret_cast<char*>(pe.meta) + s_pagesz;

  auto const begin = base + data_tail % buffer_sz();
  auto const end = base + data_head % buffer_sz();

  auto cur = begin;

  while (cur != end) {
    auto header = reinterpret_cast<struct perf_event_header*>(cur);

    if (cur + header->size > base + buffer_sz()) {
      // The current entry wraps around the ring buffer.  Copy it into a stack
      // buffer, and update `cur' to wrap around appropriately.
      auto const prefix_len = base + buffer_sz() - cur;

      ensure_buffer_capacity(pe, header->size);

      memcpy(pe.buf, cur, prefix_len);
      memcpy(pe.buf + prefix_len, base, header->size - prefix_len);
      header = reinterpret_cast<struct perf_event_header*>(pe.buf);

      cur = base + header->size - prefix_len;
    } else if (cur + header->size == base + buffer_sz()) {
      // Perfect wraparound.
      cur = base;
    } else {
      cur += header->size;
    }

    if (header->type == PERF_RECORD_SAMPLE) {
      auto const sample = reinterpret_cast<perf_event_sample*>(header + 1);

      assertx(header->size == sizeof(struct perf_event_header) +
                              sizeof(perf_event_sample) +
                              sample->nr * sizeof(*sample->ips) +
                              sizeof(perf_event_sample_tail));
      assertx((char*)(sample->tail() + 1) == (char*)header + header->size);
      consume(kind, sample);
    }
  }

  __sync_synchronize(); // smp_mb()
  pe.meta->data_tail = data_head;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

perf_event_data_src_info
perf_event_data_src(PerfEvent kind, uint64_t data_src) {
  auto info = perf_event_data_src_info{};

  DEBUG_ONLY auto const mem_op = data_src;
  switch (kind) {
    case PerfEvent::Load:
      assertx(mem_op & PERF_MEM_OP_LOAD);
      break;
    case PerfEvent::Store:
      assertx(mem_op & PERF_MEM_OP_STORE);
      break;
  }

  auto const mem_lvl = data_src >> PERF_MEM_LVL_SHIFT;

  if (mem_lvl & PERF_MEM_LVL_NA) {
    info.mem_lvl = "(unknown)";
    info.mem_hit = 0;
  } else {
    info.mem_hit = (mem_lvl & PERF_MEM_LVL_HIT)  ?  1 :
                   (mem_lvl & PERF_MEM_LVL_MISS) ? -1 : 0;

#define MEM_LVLS \
  X(L1) \
  X(LFB) \
  X(L2) \
  X(L3) \
  X(LOC_RAM) \
  X(REM_RAM1) \
  X(REM_RAM2) \
  X(REM_CCE1) \
  X(REM_CCE2) \
  X(IO) \
  X(UNC)

    auto const mem_lvl_only = mem_lvl & (0x0
#define X(lvl)  | PERF_MEM_LVL_##lvl
      MEM_LVLS
#undef X
    );

    info.mem_lvl = [&]() -> const char* {
      switch (mem_lvl_only) {
        case 0x0: return "(none)";
#define X(lvl)  \
        case PERF_MEM_LVL_##lvl: return #lvl;
        MEM_LVLS
#undef X
        default: return "(mixed)";
      }
    }();
  }

#undef MEM_LVLS

  auto const mem_snoop = data_src >> PERF_MEM_SNOOP_SHIFT;
  if (mem_snoop & PERF_MEM_SNOOP_NA) {
    info.snoop = 0;
    info.snoop_hit = 0;
    info.snoop_hitm = 0;
  } else {
    info.snoop_hit = (mem_snoop & PERF_MEM_SNOOP_HIT)  ?  1 :
                     (mem_snoop & PERF_MEM_SNOOP_MISS) ? -1 : 0;
    info.snoop      = (mem_snoop & PERF_MEM_SNOOP_NONE) ? -1 : 1;
    info.snoop_hitm = (mem_snoop & PERF_MEM_SNOOP_HITM) ? 1 : -1;
  }

  auto const mem_lock = data_src >> PERF_MEM_LOCK_SHIFT;
  info.locked = (mem_lock & PERF_MEM_LOCK_NA) ? 0 :
                (mem_lock & PERF_MEM_LOCK_LOCKED) ? 1 : -1;

  auto const mem_tlb = data_src >> PERF_MEM_TLB_SHIFT;

  if (mem_tlb & PERF_MEM_TLB_NA) {
    info.tlb = "(unknown)";
    info.tlb_hit = 0;
  } else {
    info.tlb_hit = (mem_tlb & PERF_MEM_TLB_HIT)  ?  1 :
                   (mem_tlb & PERF_MEM_TLB_MISS) ? -1 : 0;

#define TLBS \
  X(L1) \
  X(L2) \
  X(WK) \
  X(OS)

    auto const tlb_only = mem_tlb & (0x0
#define X(tlb)  | PERF_MEM_TLB_##tlb
      TLBS
#undef X
    );

    info.tlb = [&]() -> const char* {
      switch (tlb_only) {
        case 0x0: return "(none)";
#define X(tlb)  \
        case PERF_MEM_TLB_##tlb: return #tlb;
        TLBS
#undef X
        case (PERF_MEM_TLB_L1 | PERF_MEM_TLB_L2): return "L1-L2";
        default: return "(mixed)";
      }
    }();
  }

  return info;
}

///////////////////////////////////////////////////////////////////////////////

bool perf_event_enable(uint64_t sample_freq, perf_event_signal_fn_t signal_fn) {
  if (!perf_event_init()) return false;

  // If `tl_perf_event' has already been initialized, we're done.
  if (tl_perf_event.signal != nullptr) return true;

  if (!install_sigio_handler()) return false;

  auto const ld_pe = enable_event(s_mem_loads, sample_freq);
  if (!ld_pe) return false;

  auto const st_pe = enable_event(s_mem_stores, sample_freq);
  if (!st_pe) {
    close_event(*ld_pe);
    return false;
  }

  // Set `tl_perf_event'---and in particular, `signal'---only after everything
  // is enabled.  This will cause us to ignore signals until we're ready to
  // process the events.
  tl_perf_event.loads = *ld_pe;
  tl_perf_event.stores = *st_pe;
  asm volatile("" : : : "memory");
  tl_perf_event.signal = signal_fn;

  return true;
}

void perf_event_pause() {
  if (tl_perf_event.signal == nullptr) return;
  pause_event(tl_perf_event.loads);
  pause_event(tl_perf_event.stores);
}

void perf_event_resume() {
  if (tl_perf_event.signal == nullptr) return;
  resume_event(tl_perf_event.loads);
  resume_event(tl_perf_event.stores);
}

void perf_event_disable() {
  if (tl_perf_event.signal == nullptr) return;

  close_event(tl_perf_event.loads);
  close_event(tl_perf_event.stores);
  tl_perf_event = {};
}

void perf_event_consume(perf_event_consume_fn_t consume) {
  if (tl_perf_event.signal == nullptr) return;

  consume_events(PerfEvent::Load,  tl_perf_event.loads,  consume);
  consume_events(PerfEvent::Store, tl_perf_event.stores, consume);
}

///////////////////////////////////////////////////////////////////////////////

}

#else // defined(__linux__) && defined(__x86_64__)

namespace HPHP {

perf_event_data_src_info
perf_event_data_src(PerfEvent kind, uint64_t data_src) {
  return perf_event_data_src_info{};
}

bool perf_event_enable(uint64_t, perf_event_signal_fn_t) { return false; }
void perf_event_disable() {}
void perf_event_pause() {}
void perf_event_resume() {}
void perf_event_consume(perf_event_consume_fn_t) {}

}

#endif
