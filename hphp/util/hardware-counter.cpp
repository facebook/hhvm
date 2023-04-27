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

#include "hphp/util/hardware-counter.h"

#ifndef NO_HARDWARE_COUNTERS

#include <folly/ScopeGuard.h>

#include "hphp/util/alloc.h"
#include "hphp/util/logger.h"
#include "hphp/util/service-data.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>
#include <linux/perf_event.h>

#include <folly/String.h>
#include <folly/Memory.h>
#include <folly/portability/SysMman.h>
#include <folly/portability/Unistd.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

THREAD_LOCAL_NO_CHECK(HardwareCounter, HardwareCounter::s_counter);

static bool s_recordSubprocessTimes = false;
static bool s_excludeKernel = false;
static bool s_profileHWEnable;
static bool s_fastReads = false;
static int s_exportInterval = -1;
static std::string s_profileHWEvents;

static inline bool useCounters() {
#ifdef VALGRIND
  return false;
#else
  return s_profileHWEnable;
#endif
}

/*
 * Turning this on helps with the resolution of multiplexed counters
 * (provided cap_user_time is true in the
 * perf_event_mmap_page). However, experiments show that periodically,
 * time_offset and the result of rdtsc "jump" (this is probably when
 * the thread migrates from one cpu to another); when they do, they
 * jump by appropriate amounts so that enabled and runtime progress
 * monotonically (and by sensible values) - but they don't seem to
 * jump atomically, so there can be one sample where only one has
 * jumped. This can cause a temporary blip in enabled and or runtime.
 *
 * I'm adding this so we can choose to *not* use rdtsc, and avoid the
 * blips.
 *
 * It turns out that doing so does degrade the accuracy when there's a
 * lot of multiplexing going on, and a bit more experimentation shows
 * that the blip is only really a problem if we record it in the
 * baseline during a reset (since that then affects every read until
 * the next reset), so for now, turn it on but don't use it for
 * reset_values.
 */
static constexpr auto use_cap_time = true;

#if defined(__x86_64__)
#define barrier()       __asm__ volatile("" ::: "memory")
#elif defined(__aarch64__)
#define barrier()       asm volatile("dmb ish" : : : "memory")
#define isb()           asm volatile("isb" : : : "memory")
#else
#define barrier()
#endif

static uint64_t rdtsc() {
#if defined(__x86_64__)
  uint64_t msr;
  asm volatile ( "rdtsc\n\t"    // Returns the time in EDX:EAX.
                 "shl $32, %%rdx\n\t"  // Shift the upper bits left.
                 "or %%rdx, %0"        // 'Or' in the lower bits.
                 : "=a" (msr)
                 :
                 : "rdx");
  return msr;
#endif
  always_assert(false);
}

static uint64_t rdpmc(uint32_t counter) {
#if defined(__x86_64__)
  uint32_t low, high;

  __asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));
  return low | ((uint64_t)high << 32);
#elif defined(__aarch64__)
  uint64_t ret;
  if (counter == PERF_COUNT_HW_CPU_CYCLES)
    asm volatile("mrs %0, pmccntr_el0" : "=r" (ret));
  else {
    asm volatile("msr pmselr_el0, %0" : : "r" ((uint64_t)(counter-1)));
    asm volatile("mrs %0, pmxevcntr_el0" : "=r" (ret));
  }

  isb();
  return ret;
#endif
  always_assert(false);
}

static ServiceData::ExportedTimeSeries*
createTimeSeries(const std::string& name) {
  assertx(!name.empty());

  if (s_exportInterval == -1) {
    // We're initializing counters for the main thread in a server process,
    // which won't be running requests and shouldn't have any time series. Or
    // someone manually disabled time series exporting in the config. Either
    // way, bail out early.
    return nullptr;
  }

  static const std::vector<ServiceData::StatsType> exportTypes{
    ServiceData::StatsType::AVG,
    ServiceData::StatsType::SUM,
  };

  return ServiceData::createTimeSeries(
    "perf." + name,
    exportTypes,
    {std::chrono::seconds(s_exportInterval)}
  );
}

struct HardwareCounterImpl {
  HardwareCounterImpl(int type, unsigned long config, const char* desc)
    : m_desc(desc ? desc : "")
    , m_timeSeries(createTimeSeries(m_desc))
    , m_timeSeriesNonPsp(createTimeSeries(m_desc + "-nonpsp")) {
    pe.type = type;
    pe.size = sizeof (struct perf_event_attr);
    pe.config = config;
    pe.inherit = s_recordSubprocessTimes;
    pe.disabled = 1;
    pe.pinned = 0;
    pe.exclude_kernel = s_excludeKernel;
    pe.exclude_hv = 1;
    pe.read_format =
      PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;
  }

  ~HardwareCounterImpl() {
    close();
  }

  void updateServiceData(StructuredLogEntry* entry, bool includingPsp) {
    auto const value = read();
    auto timeSeries = includingPsp ? m_timeSeries : m_timeSeriesNonPsp;

    if (value != 0) {
      if (entry) entry->setInt(m_desc, value);
      if (timeSeries) timeSeries->addValue(value);
    }
  }

  void init_if_not() {
    /*
     * perf_event_open(struct perf_event_attr *hw_event_uptr, pid_t pid,
     *                 int cpu, int group_fd, unsigned long flags)
     */
    if (inited) return;
    inited = true;
    m_fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (m_fd < 0) {
      Logger::FWarning("HardwareCounter: perf_event_open failed with: {}",
                       folly::errnoStr(errno));
      m_err = -1;
      return;
    }

    fcntl(m_fd, F_SETFD, O_CLOEXEC);

    if (ioctl(m_fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
      Logger::FWarning("perf_event failed to enable: {}",
                       folly::errnoStr(errno));
      close();
      m_err = -1;
      return;
    }

    if (!s_fastReads) return;

    auto const base = mmap(nullptr, s_pageSize, PROT_READ | PROT_WRITE,
                           MAP_SHARED, m_fd, 0);
    if (base == MAP_FAILED) {
      Logger::FWarning("HardwareCounter: failed to mmap perf_event: {}",
                      folly::errnoStr(errno));
    } else {
      m_meta = static_cast<perf_event_mmap_page*>(base);
      if (!m_meta->cap_user_rdpmc ||
          (use_cap_time && !m_meta->cap_user_time)) {
        munmap(m_meta, s_pageSize);
        m_meta = nullptr;
      }
      ioctl(m_fd, PERF_EVENT_IOC_RESET, 0);
    }

    reset();
  }

  int64_t read() {
    uint64_t values[3];
    if (auto const width = readRaw(values)) {
      values[0] -= reset_values[0];
      values[1] -= reset_values[1];
      values[2] -= reset_values[2];
      if (width < 64) {
        auto const mask = (1uLL << width) - 1;
        values[0] &= mask;
        if (values[0] > (mask >> 1)) return extra;
      } else if (values[0] > std::numeric_limits<int64_t>::max()) {
        return extra;
      }
      if (values[1] == values[2]) {
        return values[0] + extra;
      }
      if (!values[2]) {
        return extra;
      }
      int64_t value = (double)values[0] * values[1] / values[2];
      return value + extra;
    }
    return 0;
  }

  void incCount(int64_t amount) {
    extra += amount;
  }

  /*
   * read current value, enabled time, and running time for the
   * counter.
   *
   * returns the width of the counter in bits, or zero on failure.
   */
  uint32_t readRaw(uint64_t* values, bool forReset = false) {
    if (m_err || !useCounters()) return 0;
    init_if_not();

    // try to read the values in user space
    if (m_meta) {
      uint32_t seq, time_mult, time_shift, idx, width;
      uint64_t cyc, time_offset;
      uint64_t count, enabled, running;

      do {
        seq = m_meta->lock;
        barrier();
        enabled = m_meta->time_enabled;
        running = m_meta->time_running;

        if (use_cap_time && !forReset) {
          assertx(m_meta->cap_user_time);

          cyc = rdtsc();
          time_offset = m_meta->time_offset;
          time_mult   = m_meta->time_mult;
          time_shift  = m_meta->time_shift;
        }

        idx = m_meta->index;
        count = m_meta->offset;
        width = m_meta->pmc_width;

        assertx(m_meta->cap_user_rdpmc);
        if (idx) {
          count += rdpmc(idx - 1);
        }

        barrier();
      } while (m_meta->lock != seq);

      [&] {
        if (!ever_active) {
          if (!idx && !count) {
            // enabled and running don't get meaningful values until
            // the first time the counter is enabled. This only really
            // matters if this call is being used to initialize the
            // reset_values, because we'll get garbage values for the
            // baseline.
            enabled = running = 0;
            return;
          }
          ever_active = true;
        }
        if (use_cap_time && !forReset) {
          auto const quot = (cyc >> time_shift);
          auto const rem = cyc & (((uint64_t)1 << time_shift) - 1);
          auto const delta = time_offset + quot * time_mult +
            ((rem * time_mult) >> time_shift);

          enabled += delta;
          if (idx) running += delta;
        }
      }();

      values[0] = count;
      values[1] = enabled;
      values[2] = running;
      return width;
    }

    if (m_fd <= 0) return 0;
    /*
     * read the count + scaling values
     *
     * It is not necessary to stop an event to read its value
     */
    auto ret = ::read(m_fd, values, sizeof(*values) * 3);
    return ret == sizeof(*values) * 3 ? 64 : 0;
  }

  void reset() {
    if (m_err || !useCounters()) return;
    init_if_not();
    extra = 0;
    if (m_fd > 0) {
      if (!m_meta && ioctl(m_fd, PERF_EVENT_IOC_RESET, 0) < 0) {
        Logger::FWarning("perf_event failed to reset with: {}",
                         folly::errnoStr(errno));
        m_err = -1;
        return;
      }
      if (!readRaw(reset_values, true)) {
        Logger::FWarning("perf_event failed to reset with: {}",
                         folly::errnoStr(errno));
        m_err = -1;
        return;
      }
    }
  }

  void pause() {
    if (m_fd > 0) {
      if (ioctl(m_fd, PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP) < 0) {
        Logger::FWarning("perf_event failed to disable: {}",
                         folly::errnoStr(errno));
      }
    }
  }

  void resume() {
    if (m_fd > 0) {
      if (ioctl(m_fd, PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP) < 0) {
        Logger::FWarning("perf_event failed to resume: {}",
                         folly::errnoStr(errno));
      }
    }
  }

public:
  std::string m_desc;
  int m_err{0};
private:
  int m_fd{-1};
  bool inited{false};
  bool ever_active{false};
  ServiceData::ExportedTimeSeries* m_timeSeries;
  ServiceData::ExportedTimeSeries* m_timeSeriesNonPsp;
  struct perf_event_attr pe{};
  uint64_t reset_values[3];
  uint64_t extra{0};
  perf_event_mmap_page* m_meta{};

  void close() {
    if (m_fd > 0) {
      ::close(m_fd);
      m_fd = -1;
      if (m_meta) {
        munmap(m_meta, s_pageSize);
        m_meta = nullptr;
      }
    }
  }
};

HardwareCounter::HardwareCounter()
  : m_countersSet(false) {
  m_instructionCounter = std::make_unique<HardwareCounterImpl>(
    PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"
  );
  if (s_profileHWEvents.empty()) {
    m_loadCounter = std::make_unique<HardwareCounterImpl>(
      PERF_TYPE_HW_CACHE,
      PERF_COUNT_HW_CACHE_L1D | ((PERF_COUNT_HW_CACHE_OP_READ) << 8),
      "loads"
    );
    m_storeCounter = std::make_unique<HardwareCounterImpl>(
      PERF_TYPE_HW_CACHE,
      PERF_COUNT_HW_CACHE_L1D | ((PERF_COUNT_HW_CACHE_OP_WRITE) << 8),
      "stores"
    );
  } else {
    m_countersSet = true;
    setPerfEvents(s_profileHWEvents);
  }
}

HardwareCounter::~HardwareCounter() {
}

void HardwareCounter::RecordSubprocessTimes() {
  s_recordSubprocessTimes = true;
}

void HardwareCounter::ExcludeKernel() {
  s_excludeKernel = true;
}

void HardwareCounter::Init(bool enable, const std::string& events,
                           bool subProc,
                           bool excludeKernel,
                           bool fastReads,
                           int exportInterval) {
  s_profileHWEnable = enable;
  s_profileHWEvents = events;
  s_recordSubprocessTimes = subProc;
  s_excludeKernel = excludeKernel;
  s_fastReads = fastReads,
  s_exportInterval = exportInterval;
}

void HardwareCounter::Reset() {
  s_counter->reset();
}

void HardwareCounter::Pause() {
  s_counter->pause();
}

void HardwareCounter::Resume() {
  s_counter->resume();
}

void HardwareCounter::reset() {
  m_instructionCounter->reset();
  if (!m_countersSet) {
    m_storeCounter->reset();
    m_loadCounter->reset();
  }
  for (unsigned i = 0; i < m_counters.size(); i++) {
    m_counters[i]->reset();
  }
}

void HardwareCounter::pause() {
  m_instructionCounter->pause();
  if (!m_countersSet) {
    m_storeCounter->pause();
    m_loadCounter->pause();
  }
  for (unsigned i = 0; i < m_counters.size(); i++) {
    m_counters[i]->pause();
  }
}

void HardwareCounter::resume() {
  m_instructionCounter->resume();
  if (!m_countersSet) {
    m_storeCounter->resume();
    m_loadCounter->resume();
  }
  for (unsigned i = 0; i < m_counters.size(); i++) {
    m_counters[i]->resume();
  }
}

int64_t HardwareCounter::GetInstructionCount() {
  return s_counter->getInstructionCount();
}

int64_t HardwareCounter::getInstructionCount() {
  return m_instructionCounter->read();
}

int64_t HardwareCounter::GetLoadCount() {
  return s_counter->getLoadCount();
}

int64_t HardwareCounter::getLoadCount() {
  return m_loadCounter ? m_loadCounter->read() : 0;
}

int64_t HardwareCounter::GetStoreCount() {
  return s_counter->getStoreCount();
}

int64_t HardwareCounter::getStoreCount() {
  return m_storeCounter ? m_storeCounter->read() : 0;
}

void HardwareCounter::IncInstructionCount(int64_t amount) {
  s_counter->m_instructionCounter->incCount(amount);
}

void HardwareCounter::IncLoadCount(int64_t amount) {
  if (!s_counter->m_countersSet) {
    s_counter->m_loadCounter->incCount(amount);
  }
}

void HardwareCounter::IncStoreCount(int64_t amount) {
  if (!s_counter->m_countersSet) {
    s_counter->m_storeCounter->incCount(amount);
  }
}

struct PerfTable perfTable[] = {
  /* PERF_TYPE_HARDWARE events */
#define PC(n)    PERF_TYPE_HARDWARE, PERF_COUNT_HW_ ## n
  { "cpu-cycles",              PC(CPU_CYCLES)              },
  { "cycles",                  PC(CPU_CYCLES)              },
  { "instructions",            PC(INSTRUCTIONS)            },
  { "cache-references",        PC(CACHE_REFERENCES)        },
  { "cache-misses",            PC(CACHE_MISSES)            },
  { "branch-instructions",     PC(BRANCH_INSTRUCTIONS)     },
  { "branches",                PC(BRANCH_INSTRUCTIONS)     },
  { "branch-misses",           PC(BRANCH_MISSES)           },
  { "bus-cycles",              PC(BUS_CYCLES)              },
  { "stalled-cycles-frontend", PC(STALLED_CYCLES_FRONTEND) },
  { "stalled-cycles-backend",  PC(STALLED_CYCLES_BACKEND)  },

  /* PERF_TYPE_HW_CACHE hw_cache_id */
#define PCC(n)   PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_ ## n
  { "L1-dcache-",          PCC(L1D)                },
  { "L1-icache-",          PCC(L1I)                },
  { "LLC-",                PCC(LL)                 },
  { "dTLB-",               PCC(DTLB)               },
  { "iTLB-",               PCC(ITLB)               },
  { "branch-",             PCC(BPU)                },

  /* PERF_TYPE_HW_CACHE hw_cache_op, hw_cache_result */
#define PCCO(n, m)  PERF_TYPE_HW_CACHE, \
                    ((PERF_COUNT_HW_CACHE_OP_ ## n) << 8 | \
                    (PERF_COUNT_HW_CACHE_RESULT_ ## m) << 16)
  { "loads",               PCCO(READ, ACCESS)      },
  { "load-misses",         PCCO(READ, MISS)        },
  { "stores",              PCCO(WRITE, ACCESS)     },
  { "store-misses",        PCCO(WRITE, MISS)       },
  { "prefetches",          PCCO(PREFETCH, ACCESS)  },
  { "prefetch-misses",     PCCO(PREFETCH, MISS)    }
};

static int findEvent(const char *event, struct PerfTable *t,
                     int len, int *match_len) {
  int i;

  for (i = 0; i < len; i++) {
    if (!strncmp(event, t[i].name, strlen(t[i].name))) {
      *match_len = strlen(t[i].name);
      return i;
    }
  }
  return -1;
}

#define CPUID_STEPPING(x)  ((x) & 0xf)
#define CPUID_MODEL(x)     (((x) & 0xf0) >> 4)
#define CPUID_FAMILY(x)    (((x) & 0xf00) >> 8)
#define CPUID_TYPE(x)      (((x) & 0x3000) >> 12)

// hack to get LLC counters on perflab frc machines
static bool isIntelE5_2670() {
#ifdef __x86_64__
  unsigned long x;
  asm volatile ("cpuid" : "=a"(x): "a"(1) : "ebx", "ecx", "edx");
  return CPUID_STEPPING(x) == 6 && CPUID_MODEL(x) == 0xd
         && CPUID_FAMILY(x) == 6 && CPUID_TYPE(x) == 0;
#else
  return false;
#endif
}

static void checkLLCHack(const char* event, uint32_t& type, uint64_t& config) {
  if (!strncmp(event, "LLC-load", 8) && isIntelE5_2670()) {
    type = PERF_TYPE_RAW;
    if (!strncmp(&event[4], "loads", 5)) {
      config = 0x534f2e;
    } else if (!strncmp(&event[4], "load-misses", 11)) {
      config = 0x53412e;
    }
  }
}

bool HardwareCounter::addPerfEvent(const char* event) {
  uint32_t type = 0;
  uint64_t config = 0;
  int i, match_len;
  bool found = false;
  const char* ev = event;

  while ((i = findEvent(ev, perfTable,
                        sizeof(perfTable)/sizeof(struct PerfTable),
                        &match_len))
       != -1) {
    if (!found) {
      found = true;
      type = perfTable[i].type;
    } else if (type != perfTable[i].type) {
      Logger::FWarning("failed to find perf event: {}", event);
      return false;
    }
    config |= perfTable[i].config;
    ev = &ev[match_len];
  }

  checkLLCHack(event, type, config);

  // Check if we have a raw spec.
  if (!found && event[0] == 'r' && event[1] != 0) {
    config = strtoull(event + 1, const_cast<char**>(&ev), 16);
    if (*ev == 0) {
      found = true;
      type = PERF_TYPE_RAW;
    }
  }

  if (!found || *ev) {
    Logger::FWarning("failed to find perf event: {}", event);
    return false;
  }
  auto hwc = std::make_unique<HardwareCounterImpl>(type, config, event);
  if (hwc->m_err) {
    Logger::FWarning("failed to set perf event: {}", event);
    return false;
  }
  m_counters.emplace_back(std::move(hwc));
  if (!m_countersSet) {
    // reset load and store counters. This is because
    // perf does not seem to handle more than three counters
    // very well.
    m_loadCounter.reset();
    m_storeCounter.reset();
    m_countersSet = true;
  }
  return true;
}

bool HardwareCounter::eventExists(const char *event) {
  // hopefully m_counters set is small, so a linear scan does not hurt
  for(unsigned i = 0; i < m_counters.size(); i++) {
    if (!strcmp(event, m_counters[i]->m_desc.c_str())) {
      return true;
    }
  }
  return false;
}

bool HardwareCounter::setPerfEvents(folly::StringPiece sevents) {
  // Make a copy of the string for use with strtok.
  auto const sevents_buf = static_cast<char*>(malloc(sevents.size() + 1));
  SCOPE_EXIT { free(sevents_buf); };
  memcpy(sevents_buf, sevents.data(), sevents.size());
  sevents_buf[sevents.size()] = '\0';

  char* strtok_buf = nullptr;
  char* s = strtok_r(sevents_buf, ",", &strtok_buf);
  while (s) {
    if (!eventExists(s) && !addPerfEvent(s)) {
      return false;
    }
    s = strtok_r(nullptr, ",", &strtok_buf);
  }
  return true;
}

bool HardwareCounter::SetPerfEvents(folly::StringPiece events) {
  return s_counter->setPerfEvents(events);
}

void HardwareCounter::clearPerfEvents() {
  m_counters.clear();
}

void HardwareCounter::ClearPerfEvents() {
  s_counter->clearPerfEvents();
}

void HardwareCounter::updateServiceData(StructuredLogEntry* entry,
                                        bool includingPsp) {
  forEachCounter([entry,includingPsp](HardwareCounterImpl& counter) {
    counter.updateServiceData(entry, includingPsp);
  });
}

void HardwareCounter::UpdateServiceData(const timespec& cpu_begin,
                                        const timespec& wall_begin,
                                        StructuredLogEntry* entry,
                                        bool includingPsp) {
  // The begin timespec should be what was recorded at the beginning of the
  // request, so we subtract that out from the current measurement. The
  // perf-based counters owned by this file are reset to 0 at the same time as
  // the begin timespec is recorded, so there's no subtraction needed for
  // those.
  struct timespec cpu_now;
  gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_now);

  s_counter->updateServiceData(entry, includingPsp);

  static auto cpuTimeSeries = createTimeSeries("cpu-time-us");
  static auto cpuTimeNonPspSeries = createTimeSeries("cpu-time-us-nonpsp");
  auto cpu_series = includingPsp ? cpuTimeSeries : cpuTimeNonPspSeries;
  auto const cpuTimeUs = gettime_diff_us(cpu_begin, cpu_now);
  if (cpuTimeUs > 0) {
    if (entry) entry->setInt("cpu-time-us", cpuTimeUs);
    if (cpu_series) cpu_series->addValue(cpuTimeUs);
  }

  struct timespec wall_now;
  Timer::GetMonotonicTime(wall_now);
  static auto wallTimeSeries = createTimeSeries("wall-time-us");
  static auto wallTimeNonPspSeries = createTimeSeries("wall-time-us-nonpsp");
  auto wall_series = includingPsp ? wallTimeSeries : wallTimeNonPspSeries;
  auto const wallTimeUs = gettime_diff_us(wall_begin, wall_now);
  if (wallTimeUs > 0) {
    if (entry) entry->setInt("wall-time-us", wallTimeUs);
    if (wall_series) wall_series->addValue(wallTimeUs);
  }

  if (entry) entry->setInt("includingPsp", includingPsp);
}

void HardwareCounter::getPerfEvents(PerfEventCallback f, void* data) {
  forEachCounter([f, data](HardwareCounterImpl& counter) {
    f(counter.m_desc, counter.read(), data);
  });
}

template<typename F>
void HardwareCounter::forEachCounter(F func) {
  func(*m_instructionCounter);
  if (!m_countersSet) {
    func(*m_loadCounter);
    func(*m_storeCounter);
  }
  for (auto& counter : m_counters) func(*counter);
}

void HardwareCounter::GetPerfEvents(PerfEventCallback f, void* data) {
  s_counter->getPerfEvents(f, data);
}

///////////////////////////////////////////////////////////////////////////////
}


#else // NO_HARDWARE_COUNTERS

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

HardwareCounter HardwareCounter::s_counter;

///////////////////////////////////////////////////////////////////////////////
}

#endif // NO_HARDWARE_COUNTERS
