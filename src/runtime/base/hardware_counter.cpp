/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#define _GNU_SOURCE 1
#include <runtime/base/hardware_counter.h>
#include <util/logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>
#include <linux/perf_event.h>
#include <runtime/base/string_data.h>
#include <runtime/base/zend/zend_url.h>
#include <runtime/base/runtime_option.h>
#include <runtime/vm/translator/translator-x64.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_THREAD_LOCAL_NO_CHECK(HardwareCounter,
    HardwareCounter::s_counter);

static inline bool useCounters() {
#ifdef VALGRIND
  return false;
#else
  return RuntimeOption::EvalProfileHWEnable;
#endif
}

class HardwareCounterImpl {
public:
  HardwareCounterImpl(int type, unsigned long config, StringData* desc = NULL)
    : m_desc(desc), m_err(0), m_fd(-1) {
    memset (&pe, 0, sizeof (struct perf_event_attr));
    pe.type = type;
    pe.size = sizeof (struct perf_event_attr);
    pe.config = config;
    pe.disabled = 1;
    pe.pinned = 0;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.read_format =
      PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING;

    if (!useCounters()) return;
    /*
     * perf_event_open(struct perf_event_attr *hw_event_uptr, pid_t pid,
     *                 int cpu, int group_fd, unsigned long flags)
     */
    m_fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (m_fd < 0) {
      Logger::Verbose("perf_event_open failed with: %s",
                      Util::safe_strerror(errno).c_str());
      m_err = -1;
      return;
    }
    if (ioctl(m_fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
      Logger::Warning("perf_event failed to enable: %s",
                      Util::safe_strerror(errno).c_str());
      close();
      m_err = -1;
      return;
    }
    reset();
    }

  ~HardwareCounterImpl() {
    close();
  }

  int64 read() {
    if (!useCounters()) return 0;

    int64 count = 0;

    if (m_fd > 0) {
      int64 values[3];
      int ret;

      /*
       * read the count + scaling values
       *
       * It is not necessary to stop an event to read its value
       */
      ret = ::read(m_fd, values, sizeof(values));
      if (ret == sizeof(values)) {
        /*
         * scale count
         *
         * values[0] = raw count
         * values[1] = TIME_ENABLED
         * values[2] = TIME_RUNNING
         */
        if (values[2]) {
          count = (int64) ((double) values[0] * values[1] / values[2]);
        }
      }
    }
    return count;
  }

  void reset() {
    if (m_fd > 0 && ioctl (m_fd, PERF_EVENT_IOC_RESET, 0) < 0) {
      Logger::Warning("perf_event failed to reset with: %s",
          Util::safe_strerror(errno).c_str());
      m_err = -1;
    }
  }

public:
  StringData* m_desc;
  int m_err;
private:
  int m_fd;
  struct perf_event_attr pe;

  void close() {
    if (m_fd > 0) {
      ::close(m_fd);
      m_fd = -1;
    }
  }
};

class InstructionCounter : public HardwareCounterImpl {
public:
  InstructionCounter() :
    HardwareCounterImpl(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS) {}
};

class LoadCounter : public HardwareCounterImpl {
public:
  LoadCounter() :
    HardwareCounterImpl(PERF_TYPE_HW_CACHE,
        (PERF_COUNT_HW_CACHE_L1D | ((PERF_COUNT_HW_CACHE_OP_READ) << 8))) {}
};

class StoreCounter : public HardwareCounterImpl {
public:
  StoreCounter() :
    HardwareCounterImpl(PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_L1D | ((PERF_COUNT_HW_CACHE_OP_WRITE) << 8)) {}
};

HardwareCounter::HardwareCounter()
  : m_countersSet(false), m_pseudoEvents(false) {
  m_instructionCounter = new InstructionCounter();
  if (RuntimeOption::EvalProfileHWEvents == "") {
    m_loadCounter = new LoadCounter();
    m_storeCounter = new StoreCounter();
  } else {
    m_countersSet = true;
    setPerfEvents(RuntimeOption::EvalProfileHWEvents);
  }
}

HardwareCounter::~HardwareCounter() {
  delete m_instructionCounter;
  if (!m_countersSet) {
    delete m_loadCounter;
    delete m_storeCounter;
  }
  for (unsigned i = 0; i < m_counters.size(); i++) {
    delete m_counters[i];
  }
  m_counters.clear();
}

void HardwareCounter::Reset(void) {
  s_counter->reset();
}

void HardwareCounter::reset(void) {
  m_instructionCounter->reset();
  if (!m_countersSet) {
    m_storeCounter->reset();
    m_loadCounter->reset();
  }
  for (unsigned i = 0; i < m_counters.size(); i++) {
    m_counters[i]->reset();
  }
}

int64 HardwareCounter::GetInstructionCount() {
  return s_counter->getInstructionCount();
}

int64 HardwareCounter::getInstructionCount() {
  return m_instructionCounter->read();
}

int64 HardwareCounter::GetLoadCount() {
  return s_counter->getLoadCount();
}

int64 HardwareCounter::getLoadCount() {
  return m_loadCounter->read();
}

int64 HardwareCounter::GetStoreCount() {
  return s_counter->getStoreCount();
}

int64 HardwareCounter::getStoreCount() {
  return m_storeCounter->read();
}

struct PerfTable perfTable[] = {
  /* PERF_TYPE_HARDWARE events */
#define PC(n)    PERF_TYPE_HARDWARE, PERF_COUNT_HW_ ## n
  { "cpu-cycles",          PC(CPU_CYCLES)          },
  { "cycles",              PC(CPU_CYCLES)          },
  { "instructions",        PC(INSTRUCTIONS)        },
  { "cache-references",    PC(CACHE_REFERENCES)    },
  { "cache-misses",        PC(CACHE_MISSES)        },
  { "branch-instructions", PC(BRANCH_INSTRUCTIONS) },
  { "branches",            PC(BRANCH_INSTRUCTIONS) },
  { "branch-misses",       PC(BRANCH_MISSES)       },
  { "bus-cycles",          PC(BUS_CYCLES)          },

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

static int findEvent(char *event, struct PerfTable *t,
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
  unsigned long x;
  asm volatile ("cpuid" : "=a"(x): "a"(1) : "ebx", "ecx", "edx");
  return CPUID_STEPPING(x) == 6 && CPUID_MODEL(x) == 0xd
         && CPUID_FAMILY(x) == 6 && CPUID_TYPE(x) == 0;
}

static void checkLLCHack(char* event, uint32_t& type, uint64_t& config) {
  if (!strncmp(event, "LLC-load", 8) && isIntelE5_2670()) {
    type = PERF_TYPE_RAW;
    if (!strncmp(&event[4], "loads", 5)) {
      config = 0x534f2e;
    } else if (!strncmp(&event[4], "load-misses", 11)) {
      config = 0x53412e;
    }
  }
}

bool HardwareCounter::addPerfEvent(char *event) {
  uint32_t type = 0;
  uint64_t config = 0;
  int i, match_len;
  bool found = false;
  char *ev = event;
  HardwareCounterImpl* hwc;

  while ((i = findEvent(ev, perfTable,
                        sizeof(perfTable)/sizeof(struct PerfTable),
                        &match_len))
       != -1) {
    found = true;
    type = perfTable[i].type;
    config |= perfTable[i].config;
    ev = &ev[match_len];
  }

  checkLLCHack(event, type, config);

  if (!found) {
    Logger::Warning("failed to find perf event: %s", event);
    return false;
  }
  hwc = new HardwareCounterImpl(
            type, config, StringData::GetStaticString(event));
  if (hwc->m_err) {
    Logger::Warning("failed to set perf event: %s", event);
    delete hwc;
    return false;
  }
  m_counters.push_back(hwc);
  if (!m_countersSet) {
    // delete load and store counters. This is because
    // perf does not seem to handle more than three counters
    // very well.
    delete m_loadCounter;
    delete m_storeCounter;
    m_countersSet = true;
  }
  return true;
}

bool HardwareCounter::eventExists(char *event) {
  // hopefully m_counters set is small, so a linear scan does not hurt
  for(unsigned i = 0; i < m_counters.size(); i++) {
    if (!strcmp(event, m_counters[i]->m_desc->data())) {
      return true;
    }
  }
  return false;
}

bool HardwareCounter::setPerfEvents(CStrRef events) {
  StringData* sd = events.get();
  StringData sevents(sd->data(), sd->size(), CopyString);
  char *strtok_buf = NULL;
  char *s = strtok_r(const_cast<char *>(sevents.data()), ",", &strtok_buf);
  m_pseudoEvents = false;
  while (s) {
    int len = strlen(s);
    char* event = url_decode(s, len);
    bool isPseudoEvent = TranslatorX64::isPseudoEvent(event);
    m_pseudoEvents = m_pseudoEvents || isPseudoEvent;
    if (!eventExists(event)) {
      if (!addPerfEvent(event) && !isPseudoEvent) {
        return false;
      }
    }
    s = strtok_r(NULL, ",", &strtok_buf);
  }
  return true;
}

bool HardwareCounter::SetPerfEvents(CStrRef events) {
  return s_counter->setPerfEvents(events);
}

void HardwareCounter::clearPerfEvents() {
  for (unsigned i = 0; i < m_counters.size(); i++) {
    delete m_counters[i];
  }
  m_counters.clear();
}

void HardwareCounter::ClearPerfEvents() {
  s_counter->clearPerfEvents();
}

void HardwareCounter::getPerfEvents(Array& ret) {
  ret.set("instructions", getInstructionCount());
  if (!m_countersSet) {
    ret.set("loads", getLoadCount());
    ret.set("stores", getStoreCount());
  }
  for (unsigned i = 0; i < m_counters.size(); i++) {
    ret.set(m_counters[i]->m_desc->data(), m_counters[i]->read());
  }
  if (m_pseudoEvents) {
    TranslatorX64::Get()->getPerfCounters(ret);
  }
}

void HardwareCounter::GetPerfEvents(Array& ret) {
  s_counter->getPerfEvents(ret);
}

///////////////////////////////////////////////////////////////////////////////
}
