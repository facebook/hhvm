/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
  int fd{-1};
  struct perf_event_mmap_page* meta{nullptr};
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
void signal_event(int sig, siginfo_t* info, void* context) {
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
  attr.sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_ADDR;
  attr.freq = 1;
  attr.watermark = 0;
  attr.wakeup_events = 1;
  attr.precise_ip = 2;  // request zero skid

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
  // use fcntl() for portability.
  fcntl(fd, F_SETFD, O_CLOEXEC);

  // Make sure that any SIGIO sent from `fd' is handled by the calling thread.
  f_owner_ex owner;
  owner.type = F_OWNER_TID;
  owner.pid = syscall(__NR_gettid);
  fcntl(fd, F_SETOWN_EX, &owner);

  // Install our signal handler for SIGIO.
  struct sigaction action = {};
  action.sa_sigaction = signal_event;
  action.sa_flags = SA_SIGINFO;

  if (sigaction(SIGIO, &action, nullptr) < 0) {
    Logger::Warning("perf_event: could not install SIGIO handler: %s",
                    folly::errnoStr(errno).c_str());
    close(fd);
    return folly::none;
  }

  // Set up `fd' to send SIGIO with sigaction info.
  fcntl(fd, F_SETSIG, SIGIO);
  fcntl(fd, F_SETFL, O_ASYNC);

  // Enable the event.  The man page and other examples of usage all suggest
  // that the right thing to do is to start with the event disabled and then
  // enable it manually afterwards, so we do the same here even though it seems
  // strange and circuitous.
  if (ioctl(fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
    Logger::Warning("perf_event: failed to enable perf_event: %s",
                    folly::errnoStr(errno).c_str());
    close(fd);
    return folly::none;
  }

  auto const base = mmap(nullptr, mmap_sz(), PROT_READ | PROT_WRITE,
                         MAP_SHARED, fd, 0);
  if (base == MAP_FAILED) {
    Logger::Warning("perf_event: failed to mmap perf_event: %s",
                    folly::errnoStr(errno).c_str());
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
    close(fd);
    return folly::none;
  }
  auto const meta = reinterpret_cast<struct perf_event_mmap_page*>(base);

  return perf_event_handle { fd, meta };
}

/*
 * Disable and close a perf event.
 */
void close_event(const perf_event_handle& pe) {
  ioctl(pe.fd, PERF_EVENT_IOC_DISABLE, 0);
  munmap(pe.meta, mmap_sz());
  close(pe.fd);
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Iterate through all the pending sampled events in `pe' and pass each one to
 * `consume'.
 */
void consume_events(PerfEvent kind, perf_event_handle& pe,
                    perf_event_consume_fn_t consume) {
  // Temporary buffer for holding structs that were wrapped in the ring buffer.
  // 256 bytes should be large enough to hold any perf records.
  char tmp[256];

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
      // Make sure the record isn't bigger than expected.
      assertx(header->size <= 256);

      // The current entry wraps around the ring buffer.  Copy it into a stack
      // buffer, and update `cur' to wrap around appropriately.
      auto const prefix_len = base + buffer_sz() - cur;

      memcpy(tmp, cur, prefix_len);
      memcpy(tmp + prefix_len, base, header->size - prefix_len);
      header = reinterpret_cast<struct perf_event_header*>(tmp);

      cur = base + header->size - prefix_len;
    } else if (cur + header->size == base + buffer_sz()) {
      // Perfect wraparound.
      cur = base;
    } else {
      cur += header->size;
    }

    if (header->type == PERF_RECORD_SAMPLE) {
      assertx(header->size == sizeof(struct perf_event_header) +
                              sizeof(perf_event_sample));
      auto const sample = reinterpret_cast<perf_event_sample*>(header + 1);
      consume(kind, sample);
    }
  }

  __sync_synchronize(); // smp_mb()
  pe.meta->data_tail = data_head;
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

bool perf_event_enable(uint64_t sample_freq, perf_event_signal_fn_t signal_fn) {
  if (!perf_event_init()) return false;

  // If `tl_perf_event' has already been initialized, we're done.
  if (tl_perf_event.signal != nullptr) return true;

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

void perf_event_disable() {
  if (tl_perf_event.signal == nullptr) return;

  close_event(tl_perf_event.loads);
  close_event(tl_perf_event.stores);
  tl_perf_event = {};
}

void perf_event_consume(perf_event_consume_fn_t consume) {
  consume_events(PerfEvent::Load,  tl_perf_event.loads,  consume);
  consume_events(PerfEvent::Store, tl_perf_event.stores, consume);
}

///////////////////////////////////////////////////////////////////////////////

}

#else // defined(__linux__) && defined(__x86_64__)

namespace HPHP {

bool perf_event_enable(uint64_t, perf_event_signal_fn_t) { return false; }
void perf_event_disable() {}
void perf_event_consume(perf_event_consume_fn_t) {}

}

#endif
