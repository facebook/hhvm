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

IMPLEMENT_THREAD_LOCAL_NO_CHECK(HardwareCounter,
    HardwareCounter::s_counter);

static bool s_recordSubprocessTimes = false;
static bool s_excludeKernel = false;
static bool s_profileHWEnable;
static int s_exportInterval = -1;
static std::string s_profileHWEvents;

static inline bool useCounters() {
#ifdef VALGRIND
  return false;
#else
  return s_profileHWEnable;
#endif
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
    , m_err(0)
    , m_timeSeries(createTimeSeries(m_desc))
    , m_timeSeriesNonPsp(createTimeSeries(m_desc + "-nonpsp"))
    , m_fd(-1)
    , inited(false) {
    memset (&pe, 0, sizeof (struct perf_event_attr));
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
      Logger::Warning("perf_event_open failed with: %s",
                      folly::errnoStr(errno).c_str());
      m_err = -1;
      return;
    }

    fcntl(m_fd, F_SETFD, O_CLOEXEC);

    if (ioctl(m_fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
      Logger::Warning("perf_event failed to enable: %s",
                      folly::errnoStr(errno).c_str());
      close();
      m_err = -1;
      return;
    }
    reset();
  }

  int64_t read() {
    uint64_t values[3];
    if (readRaw(values)) {
      if (!values[2]) return 0;
      int64_t value = (double)values[0] * values[1] / values[2];
      return value + extra;
    }
    return 0;
  }

  void incCount(int64_t amount) {
    extra += amount;
  }

  bool readRaw(uint64_t* values) {
    if (m_err || !useCounters()) return false;
    init_if_not();

    if (m_fd > 0) {
      /*
       * read the count + scaling values
       *
       * It is not necessary to stop an event to read its value
       */
      auto ret = ::read(m_fd, values, sizeof(*values) * 3);
      if (ret == sizeof(*values) * 3) {
        values[0] -= reset_values[0];
        values[1] -= reset_values[1];
        if (values[2] > reset_values[2]) {
          values[2] -= reset_values[2];
        } else {
          values[2] = 0;
        }
        return true;
      }
    }
    return false;
  }

  void reset() {
    if (m_err || !useCounters()) return;
    init_if_not();
    extra = 0;
    if (m_fd > 0) {
      if (ioctl (m_fd, PERF_EVENT_IOC_RESET, 0) < 0) {
        Logger::Warning("perf_event failed to reset with: %s",
                        folly::errnoStr(errno).c_str());
        m_err = -1;
        return;
      }
      auto ret = ::read(m_fd, reset_values, sizeof(reset_values));
      if (ret != sizeof(reset_values)) {
        Logger::Warning("perf_event failed to reset with: %s",
                        folly::errnoStr(errno).c_str());
        m_err = -1;
        return;
      }
    }
  }

public:
  std::string m_desc;
  int m_err;
private:
  ServiceData::ExportedTimeSeries* m_timeSeries;
  ServiceData::ExportedTimeSeries* m_timeSeriesNonPsp;
  int m_fd;
  struct perf_event_attr pe;
  bool inited;
  uint64_t reset_values[3];
  uint64_t extra{0};

  void close() {
    if (m_fd > 0) {
      ::close(m_fd);
      m_fd = -1;
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
                           bool subProc, bool excludeKernel,
                           int exportInterval) {
  s_profileHWEnable = enable;
  s_profileHWEvents = events;
  s_recordSubprocessTimes = subProc;
  s_excludeKernel = excludeKernel;
  s_exportInterval = exportInterval;
}

void HardwareCounter::Reset() {
  s_counter->reset();
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
  return m_loadCounter->read();
}

int64_t HardwareCounter::GetStoreCount() {
  return s_counter->getStoreCount();
}

int64_t HardwareCounter::getStoreCount() {
  return m_storeCounter->read();
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
      Logger::Warning("failed to find perf event: %s", event);
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
    Logger::Warning("failed to find perf event: %s", event);
    return false;
  }
  auto hwc = std::make_unique<HardwareCounterImpl>(type, config, event);
  if (hwc->m_err) {
    Logger::Warning("failed to set perf event: %s", event);
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
