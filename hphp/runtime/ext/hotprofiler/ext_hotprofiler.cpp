/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/alloc.h"
#include "hphp/util/vdso.h"
#include "hphp/util/cycles.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/base/system-profiler.h"
#include "hphp/runtime/ext/xdebug/xdebug_profiler.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/base/request-event-handler.h"

#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <algorithm>
#include <map>
#include <new>
#include <utility>
#include <vector>

// Append the delimiter
#define HP_STACK_DELIM        "==>"
#define HP_STACK_DELIM_LEN    (sizeof(HP_STACK_DELIM) - 1)

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION_VERSION(hotprofiler, NO_VERSION_YET);

using std::vector;
using std::string;

const StaticString s_hotprofiler("hotprofiler");

///////////////////////////////////////////////////////////////////////////////
// helpers

/**
 * Get time delta in microseconds.
 */
static long get_us_interval(struct timeval *start, struct timeval *end) {
  return (((end->tv_sec - start->tv_sec) * 1000000)
          + (end->tv_usec - start->tv_usec));
}

/**
 * Incr time with the given microseconds.
 */
static void incr_us_interval(struct timeval *start, uint64_t incr) {
  incr += (start->tv_sec * 1000000 + start->tv_usec);
  start->tv_sec  = incr/1000000;
  start->tv_usec = incr%1000000;
  return;
}

/**
 * Truncates the given timeval to the nearest slot begin, where
 * the slot size is determined by intr
 *
 * @param  tv       Input timeval to be truncated in place
 * @param  intr     Time interval in microsecs - slot width
 * @return void
 * @author veeve
 */
static void hp_trunc_time(struct timeval *tv, uint64_t intr) {
  uint64_t time_in_micro;

  // Convert to microsecs and trunc that first
  time_in_micro = (tv->tv_sec * 1000000) + tv->tv_usec;
  time_in_micro /= intr;
  time_in_micro *= intr;

  // Update tv
  tv->tv_sec  = (time_in_micro / 1000000);
  tv->tv_usec = (time_in_micro % 1000000);
}

///////////////////////////////////////////////////////////////////////////////
// High precision timer related functions.

/**
 * This is a microbenchmark to get cpu frequency the process is running on. The
 * returned value is used to convert TSC counter values to microseconds.
 *
 * @return int64.
 * @author cjiang
 */
static int64_t get_cpu_frequency() {
  struct timeval start;
  struct timeval end;

  if (gettimeofday(&start, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64_t tsc_start = cpuCycles();
  uint64_t tsc_end;
  volatile int i;
  // Busy loop for 5 miliseconds. Don't use usleep() here since it causes the
  // CPU to halt which will generate meaningless results.
  do {
    for (i = 0; i < 1000000; i++);
    if (gettimeofday(&end, 0)) {
      perror("gettimeofday");
      return 0.0;
    }
    tsc_end = cpuCycles();
  } while (get_us_interval(&start, &end) < 5000);

  return nearbyint((tsc_end - tsc_start) * 1.0
                                   / (get_us_interval(&start, &end)));
}

#define MAX_LINELENGTH 1024

static int64_t* get_cpu_frequency_from_file(const char *file, int ncpus)
{
  std::ifstream cpuinfo(file);
  if (cpuinfo.fail()) {
    return nullptr;
  }
  char line[MAX_LINELENGTH];
  int64_t* freqs = new int64_t[ncpus];
  for (int i = 0; i < ncpus; ++i) {
    freqs[i] = 0;
  }
  int processor = -1;

  while (cpuinfo.getline(line, sizeof(line))) {
    if (sscanf(line, "processor : %d", &processor) == 1) {
      continue;
    }
    float freq;
    if (sscanf(line, "cpu MHz : %f", &freq) == 1) {
      if (processor != -1 && processor < ncpus) {
         freqs[processor] = nearbyint(freq);
         processor = -1;
      }
    }
  }
  for (int i = 0; i < ncpus; ++i) {
    if (freqs[i] == 0) {
      delete[] freqs;
      return nullptr;
    }
  }
  return freqs;
}

///////////////////////////////////////////////////////////////////////////////
// Machine information that we collect just once.

class MachineInfo {
public:
  /**
   * Bind the current process to a specified CPU. This function is to ensure
   * that the OS won't schedule the process to different processors, which
   * would make values read by rdtsc unreliable.
   *
   * @param uint32 cpu_id, the id of the logical cpu to be bound to.
   *
   * @author cjiang
   */
  static void BindToCPU(uint32_t cpu_id) {
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    CPU_SET(cpu_id, &new_mask);
    SET_AFFINITY(0, sizeof(cpu_set_t), &new_mask);
  }

public:
  // The number of logical CPUs this machine has.
  int m_cpu_num;
  // Store the cpu frequency.  Get it from /proc/cpuinfo if we can.
  int64_t* m_cpu_frequencies;

  MachineInfo() {
    m_cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    m_cpu_frequencies = get_cpu_frequency_from_file("/proc/cpuinfo", m_cpu_num);

    if (m_cpu_frequencies)
      return;

    m_cpu_frequencies = new int64_t[m_cpu_num];
    for (int i = 0; i < m_cpu_num; i++) {
      cpu_set_t prev_mask;
      GET_AFFINITY(0, sizeof(cpu_set_t), &prev_mask);
      BindToCPU(i);
      // Make sure the current process gets scheduled to the target cpu. This
      // might not be necessary though.
      usleep(0);
      m_cpu_frequencies[i] = get_cpu_frequency();
      SET_AFFINITY(0, sizeof(cpu_set_t), &prev_mask);
    }
  }

  ~MachineInfo() {
    delete[] m_cpu_frequencies;
  }
};
static MachineInfo s_machine;

static inline uint64_t
tv_to_cycles(const struct timeval& tv, int64_t MHz)
{
  return (((uint64_t)tv.tv_sec * 1000000) + tv.tv_usec) * MHz;
}

static inline uint64_t
to_usec(int64_t cycles, int64_t MHz, bool cpu_time = false)
{
#ifdef CLOCK_THREAD_CPUTIME_ID
  static int64_t vdso_usable = Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID);
#else
  static int64_t vdso_usable = -1;
#endif

  if (cpu_time && vdso_usable >= 0)
    return cycles / 1000;
  return (cycles + MHz/2) / MHz;
}

static inline uint64_t cpuTime(int64_t MHz) {
#ifdef CLOCK_THREAD_CPUTIME_ID
  int64_t rval = Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID);
  if (rval >= 0) {
    return rval;
  }
#endif
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return
    tv_to_cycles(usage.ru_utime, MHz) + tv_to_cycles(usage.ru_stime, MHz);
}

uint64_t
get_allocs()
{
#ifdef USE_JEMALLOC
  auto& mm = MM();
  return mm.getAllocated();
#endif
#ifdef USE_TCMALLOC
  if (MallocExtensionInstance) {
    size_t stat;
    MallocExtensionInstance()->GetNumericProperty(
           "generic.thread_bytes_allocated", &stat);
    return stat;
  }
#endif
  return 0;
}

uint64_t
get_frees()
{
#ifdef USE_JEMALLOC
  auto& mm = MM();
  return mm.getDeallocated();
#endif
#ifdef USE_TCMALLOC
  if (MallocExtensionInstance) {
    size_t stat;
    MallocExtensionInstance()->GetNumericProperty(
           "generic.thread_bytes_freed", &stat);
    return stat;
  }
#endif
  return 0;
}

size_t Frame::getName(char *result_buf, size_t result_len) {
  if (result_len <= 1) {
    return 0; // Insufficient result_bug. Bail!
  }

  // Add '@recurse_level' if required
  // NOTE: Dont use snprintf's return val as it is compiler dependent
  if (m_recursion) {
    snprintf(result_buf, result_len, "%s@%d", m_name, m_recursion);
  } else {
    snprintf(result_buf, result_len, "%s", m_name);
  }

  // Force null-termination at MAX
  result_buf[result_len - 1] = 0;
  return strlen(result_buf);
}

size_t Frame::getStack(int level, char *result_buf, size_t result_len) {
  // End recursion if we dont need deeper levels or
  // we dont have any deeper levels
  if (!m_parent || level <= 1) {
    return getName(result_buf, result_len);
  }

  // Take care of all ancestors first
  size_t len = m_parent->getStack(level - 1, result_buf, result_len);
  if (result_len < (len + HP_STACK_DELIM_LEN)) {
    return len; // Insufficient result_buf. Bail out!
  }

  // Add delimiter only if entry had ancestors
  if (len) {
    strncat(result_buf + len, HP_STACK_DELIM, result_len - len);
    len += HP_STACK_DELIM_LEN;
  }

  // Append the current function name
  return len + getName(result_buf + len, result_len - len);
}

const StaticString
  s_ct("ct"),
  s_wt("wt"),
  s_cpu("cpu"),
  s_mu("mu"),
  s_pmu("pmu"),
  s_alloc("alloc"),
  s_free("free"),
  s_compressed_trace("(compressed_trace)");

/**
 * Maintain profiles of a running stack.
 */
Profiler::Profiler(bool needCPUAffinity) : m_successful(true),
                                           m_stack(nullptr),
                                           m_frame_free_list(nullptr),
                                           m_has_affinity(needCPUAffinity) {
    if (!s_rand_initialized) {
      s_rand_initialized = true;
      srand(math_generate_seed());
    }

    if (m_has_affinity) {
      //
      // Bind to a random cpu so that we can use rdtsc instruction.
      //
      int cur_cpu_id = rand() % s_machine.m_cpu_num;
      GET_AFFINITY(0, sizeof(cpu_set_t), &m_prev_mask);
      MachineInfo::BindToCPU(cur_cpu_id);
      m_MHz = s_machine.m_cpu_frequencies[cur_cpu_id];
    } else {
      //
      // Take cpu0's speed as a proxy for all cpus.
      //
      m_MHz = s_machine.m_cpu_frequencies[0];
    }

    memset(m_func_hash_counters, 0, sizeof(m_func_hash_counters));
}

Profiler::~Profiler() {
    if (m_has_affinity) {
      SET_AFFINITY(0, sizeof(cpu_set_t), &m_prev_mask);
    }

    endAllFrames();
    for (Frame *p = m_frame_free_list; p;) {
      Frame *cur = p;
      p = p->m_parent;
      delete cur;
    }
}

/*
 * Called right before a function call.
 */
void Profiler::beginFrameEx(const char *symbol) {
}

/*
 * Called right after a function is finished.
 */
void Profiler::endFrameEx(const TypedValue *retval,
                          const char *_symbol) {
}

void Profiler::writeStats(Array &ret) {
}

void Profiler::endAllFrames() {
    while (m_stack) {
      endFrame(nullptr, nullptr, true);
    }
}

template<class phpret, class Name, class Counts>
void Profiler::returnVals(phpret& ret, const Name& name, const Counts& counts,
                          int flags, int64_t MHz)
{
    ArrayInit arr(5, ArrayInit::Map{});
    arr.set(s_ct,  counts.count);
    arr.set(s_wt,  to_usec(counts.wall_time, MHz));
    if (flags & TrackCPU) {
      arr.set(s_cpu, to_usec(counts.cpu, MHz, true));
    }
    if (flags & TrackMemory) {
      arr.set(s_mu,  counts.memory);
      arr.set(s_pmu, counts.peak_memory);
    } else if (flags & TrackMalloc) {
      arr.set(s_alloc, counts.memory);
      arr.set(s_free, counts.peak_memory);
    }
    ret.set(String(name), arr.toArray());
}

template<class phpret, class StatsMap>
bool Profiler::extractStats(phpret& ret, StatsMap& stats, int flags,
                            int64_t MHz)
{
    for (typename StatsMap::const_iterator iter = stats.begin();
         iter != stats.end(); ++iter) {
      returnVals(ret, iter->first, iter->second, flags, MHz);
    }
    return true;
}

bool Profiler::s_rand_initialized = false;

void Profiler::beginFrame(const char *symbol) {
  Frame *current = createFrame(symbol);

  // NOTE(cjiang): use hash code to fend off most of call-stack traversal
  int recursion_level = 0;
  if (m_func_hash_counters[current->m_hash_code] > 0) {
    // Find this symbols recurse level
    for (Frame *p = current->m_parent; p; p = p->m_parent) {
      if (strcmp(current->m_name, p->m_name) == 0) {
        recursion_level = p->m_recursion + 1;
        break;
      }
    }
  }
  current->m_recursion = recursion_level;

  m_func_hash_counters[current->m_hash_code]++;
  beginFrameEx(symbol);
}

/**
 * End top of the stack.
 */
void Profiler::endFrame(const TypedValue *retval,
                        const char *symbol,
                        bool endMain) {
  if (m_stack) {
    // special case for main() frame that's only ended by endAllFrames()
    if (!endMain && m_stack->m_parent == nullptr) {
      return;
    }
    endFrameEx(retval, symbol);
    m_func_hash_counters[m_stack->m_hash_code]--;
    releaseFrame();
  }
}

///////////////////////////////////////////////////////////////////////////////
// HierarchicalProfiler

class HierarchicalProfiler final : public Profiler {
private:
  class CountMap {
  public:
    CountMap() : count(0), wall_time(0), cpu(0), memory(0), peak_memory(0) {}

    int64_t count;
    int64_t wall_time;
    int64_t cpu;
    int64_t memory;
    int64_t peak_memory;
  };

  class HierarchicalProfilerFrame : public Frame {
  public:
    virtual ~HierarchicalProfilerFrame() {
    }

    uint64_t        m_tsc_start;   // start value for TSC counter
    int64_t         m_mu_start;    // memory usage
    int64_t         m_pmu_start;   // peak memory usage
    int64_t         m_vtsc_start;  // user/sys time start
  };

  typedef hphp_hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:
  explicit HierarchicalProfiler(int flags) : Profiler(true), m_flags(flags) {
  }

  Frame *allocateFrame() override {
    return new HierarchicalProfilerFrame();
  }

  void beginFrameEx(const char *symbol) override {
    HierarchicalProfilerFrame *frame =
      dynamic_cast<HierarchicalProfilerFrame *>(m_stack);
    frame->m_tsc_start = cpuCycles();

    if (m_flags & TrackCPU) {
      frame->m_vtsc_start = cpuTime(m_MHz);
    }

    if (m_flags & TrackMemory) {
      auto const& stats = MM().getStats();
      frame->m_mu_start  = stats.usage;
      frame->m_pmu_start = stats.peakUsage;
    } else if (m_flags & TrackMalloc) {
      frame->m_mu_start = get_allocs();
      frame->m_pmu_start = get_frees();
    }
  }

  void endFrameEx(const TypedValue *retval, const char *given_symbol) override {
    char symbol[512];
    HierarchicalProfilerFrame *frame =
      dynamic_cast<HierarchicalProfilerFrame *>(m_stack);
    frame->getStack(2, symbol, sizeof(symbol));
    CountMap &counts = m_stats[symbol];
    counts.count++;
    counts.wall_time += cpuCycles() - frame->m_tsc_start;

    if (m_flags & TrackCPU) {
      counts.cpu += cpuTime(m_MHz) - frame->m_vtsc_start;
    }

    if (m_flags & TrackMemory) {
      auto const& stats = MM().getStats();
      int64_t mu_end = stats.usage;
      int64_t pmu_end = stats.peakUsage;
      counts.memory += mu_end - frame->m_mu_start;
      counts.peak_memory += pmu_end - frame->m_pmu_start;
    } else if (m_flags & TrackMalloc) {
      counts.memory += get_allocs() - frame->m_mu_start;
      counts.peak_memory += get_frees() - frame->m_pmu_start;
    }
  }

  void writeStats(Array &ret) override {
    extractStats(ret, m_stats, m_flags, m_MHz);
  }

  bool shouldSkipBuiltins() const override {
    return m_flags & NoTrackBuiltins;
  }

private:
  uint32_t m_flags;
};

///////////////////////////////////////////////////////////////////////////////
// TraceProfiler

// Walks a log of function enter and exit events captured by
// TraceProfiler and generates statistics for each function executed.
template <class TraceIterator, class Stats>
class TraceWalker {
 public:
  struct Frame {
    TraceIterator trace; // Pointer to the log entry which pushed this frame
    int level; // Recursion level for this function
    int len; // Length of the function name
  };

  TraceWalker()
    : m_arcBuffLen(200)
    , m_arcBuff((char*)malloc(200))
    , m_badArcCount(0)
  {};

  ~TraceWalker() {
    free(m_arcBuff);
    for (auto& r : m_recursion) delete[] r.first;
  }

  void walk(TraceIterator begin, TraceIterator end, TraceIterator final,
            Stats& stats) {
    if (begin == end) return;
    m_recursion.push_back(std::make_pair(nullptr, 0));
    // Trim exit traces off the front of the log. These may be due to
    // the return from turning tracing on.
    std::map<const char*, unsigned> functionLevel;
    auto current = begin;
    while (current != end && !current->symbol) ++current;
    while (current != end) {
      if (!current->is_func_exit) {
        unsigned level = ++functionLevel[current->symbol];
        if (level >= m_recursion.size()) {
          char *level_string = new char[8];
          sprintf(level_string, "@%u", level);
          m_recursion.push_back(std::make_pair(level_string,
                                               strlen(level_string)));
        }
        Frame fr;
        fr.trace = current;
        fr.level = level - 1;
        fr.len = strlen(current->symbol);
        checkArcBuff(fr.len);
        m_stack.push_back(fr);
      } else if (m_stack.size() > 1) {
        validateStack(current, stats); // NB: may update m_stack.
        --functionLevel[m_stack.back().trace->symbol];
        popFrame(current, stats);
      }
      ++current;
    }
    // Close the dangling stack with the last entry. This
    // under-represents any functions still on the stack.
    --current;
    while (m_stack.size() > 1) {
      popFrame(current, stats);
    }
    // Close main() with the final data from when profiling was turned
    // off. This ensures main() represents the entire run, even if we
    // run out of log space.
    if (!m_stack.empty()) {
      assert(strcmp(m_stack.back().trace->symbol, "main()") == 0);
      incStats(m_stack.back().trace->symbol, final, m_stack.back(), stats);
    }
    if (m_badArcCount > 0) {
      stats["(trace has mismatched calls and returns)"].count = m_badArcCount;
    }
  }

 private:
  void checkArcBuff(int len) {
    len = 2*len + HP_STACK_DELIM_LEN + 2;
    if (len >= m_arcBuffLen) {
      m_arcBuffLen *= 2;
      m_arcBuff = (char *)realloc(m_arcBuff, m_arcBuffLen);
      if (m_arcBuff == nullptr) {
        throw std::bad_alloc();
      }
    }
  }

  void incStats(const char* arc, TraceIterator tr, const Frame& fr,
                Stats& stats) {
    auto& st = stats[arc];
    ++st.count;
    st.wall_time += tr->wall_time - fr.trace->wall_time;
    st.cpu += tr->cpu - fr.trace->cpu;
    st.memory += tr->memory - fr.trace->memory;
    st.peak_memory += tr->peak_memory - fr.trace->peak_memory;
  }

  // Look for mismatched enter and exit events, and try to correct if
  // we can. Only try to correct very simple imbalances... we could go
  // nuts here, but it's likely not worth it.
  void validateStack(TraceIterator tIt, Stats& stats) {
    auto enteredName = m_stack.back().trace->symbol;
    auto exitedName = tIt->symbol;
    if ((exitedName != nullptr) &&
        ((enteredName == nullptr) || (strcmp(enteredName, exitedName) != 0))) {
      // We have a few special names that we form on entry. We don't
      // have the information to form them again on exit, so tolerate
      // them here. See EventHook::GetFunctionNameForProfiler().
      if ((enteredName != nullptr) &&
          ((strncmp(enteredName, "run_init::", 10) == 0) ||
           (strcmp(enteredName, "_") == 0))) return;
      bool fixed = false;
      if (m_stack.size() > 1) {
        auto callerName = (m_stack.end() - 2)->trace->symbol;
        if ((callerName != nullptr) && (strcmp(callerName, exitedName) == 0)) {
          // We have an exit for Foo(), but we were in Bar(). However,
          // it appears that Foo() was the caller of Bar(). This
          // suggests we've missed the exit event for Bar() and have
          // the exit event for Foo() in hand. So remove Bar() to
          // re-balance the stack.
          m_stack.pop_back();
          fixed = true;
        }
      }
      // The first few bad arcs typically point at the problem, so
      // report them. The rest we'll just count.
      if (++m_badArcCount < 20) {
        std::string badArc;
        if (fixed) {
          badArc = folly::format("(warning: corrected bad arc #{}: "
                                 "enter '{}', exit '{}')",
                                 m_badArcCount,
                                 enteredName, exitedName).str();
        } else {
          badArc = folly::format("(error: bad arc #{}: "
                                 "enter '{}', exit '{}')",
                                 m_badArcCount,
                                 enteredName, exitedName).str();
        }
        ++stats[badArc.data()].count;
      }
    }
  }

  void popFrame(TraceIterator tIt, Stats& stats) {
    Frame callee = m_stack.back();
    m_stack.pop_back();
    Frame& caller = m_stack.back();
    char *cp = m_arcBuff;
    memcpy(cp, caller.trace->symbol, caller.len);
    cp += caller.len;
    if (caller.level >= 1) {
      std::pair<char*, int>& lvl = m_recursion[caller.level];
      memcpy(cp, lvl.first, lvl.second);
      cp += lvl.second;
    }
    memcpy(cp, HP_STACK_DELIM, HP_STACK_DELIM_LEN);
    cp += HP_STACK_DELIM_LEN;
    memcpy(cp, callee.trace->symbol, callee.len);
    cp += callee.len;
    if (callee.level >= 1) {
      std::pair<char*, int>& lvl = m_recursion[callee.level];
      memcpy(cp, lvl.first, lvl.second);
      cp += lvl.second;
    }
    *cp = 0;
    incStats(m_arcBuff, tIt, callee, stats);
  }

  vector<std::pair<char*, int>> m_recursion;
  vector<Frame> m_stack;
  int m_arcBuffLen;
  char *m_arcBuff;
  int m_badArcCount;
};

// Profiler which makes a log of all function enter and exit events,
// then processes that into per-function statistics. A single-frame
// stack trace is used to aggregate stats for each function when
// called from different call sites.
class TraceProfiler : public Profiler {
 public:
  explicit TraceProfiler(int flags)
    : Profiler(true)
    , m_traceBuffer(nullptr)
    , m_traceBufferSize(0)
    , m_nextTraceEntry(0)
    , m_traceBufferFilled(false)
    , m_maxTraceBuffer(0)
    , m_overflowCalls(0)
    , m_flags(flags)
  {
    if (!(m_flags & IHaveInfiniteMemory) && pthread_mutex_trylock(&s_inUse)) {
      // This profiler uses a very large amount of memory. Only allow
      // one in the process at any time.
      m_successful = false;
    } else {
      m_maxTraceBuffer = RuntimeOption::ProfilerMaxTraceBuffer;
      Extension* ext = ExtensionRegistry::get(s_hotprofiler);
      assert(ext);
      IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                       "profiler.max_trace_buffer",
                       &m_maxTraceBuffer);
    }
  }

  ~TraceProfiler() {
    if (m_successful) {
      free(m_traceBuffer);
      IniSetting::Unbind("profiler.max_trace_buffer");
      pthread_mutex_unlock(&s_inUse);
    }
  }

 private:
  // Data measued on function entry and exit
  struct TraceData {
    int64_t wall_time;
    int64_t cpu;

    // It's not plausible that we need a full 64bits to hold memory
    // stats, no matter what the collection mode. So we steal one bit
    // from the memory field to use as a flag. We want to keep this
    // data structure small since we need a huge number of them during
    // a profiled run.
    int64_t memory : 63; // Total memory, or memory allocated depending on flags
    bool is_func_exit : 1; // Is the entry for a function exit?
    int64_t peak_memory : 63; // Peak memory, or memory freed depending on flags
    uint64_t unused : 1; // Unused, to keep memory and peak_memory the same size

    void clear() {
      wall_time = cpu = memory = peak_memory = 0;
    }
    static void compileTimeAssertions() {
      static_assert(sizeof(TraceData) == (sizeof(uint64_t) * 4), "");
    }
  };

  // One entry in the log, representing a function enter or exit event
  struct TraceEntry : TraceData {
    const char *symbol; // Function name
  };

  bool isTraceSpaceAvailable() {
    // the two slots are reserved for internal use
    return m_nextTraceEntry < m_traceBufferSize - 3;
  }

  bool ensureTraceSpace() {
    bool track_realloc = false;
    if (m_traceBufferFilled) {
      m_overflowCalls++;
      return false;
    }
    int new_array_size;
    if (m_traceBufferSize == 0) {
      new_array_size = RuntimeOption::ProfilerTraceBuffer;
    } else {
      new_array_size = m_traceBufferSize *
        RuntimeOption::ProfilerTraceExpansion;
      if (m_maxTraceBuffer != 0 && new_array_size > m_maxTraceBuffer) {
        new_array_size = m_maxTraceBuffer > m_traceBufferSize ?
          m_maxTraceBuffer : m_traceBufferSize;
      }
      if (new_array_size - m_nextTraceEntry <= 5) {
        // for this operation to succeed, we need room for the entry we're
        // adding, two realloc entries, and two entries to mark the end of
        // the trace.
        m_traceBufferFilled = true;
        collectStats("(trace buffer terminated)", false,
                     m_traceBuffer[m_nextTraceEntry++]);
        return false;
      }
      track_realloc = true;
    }
    if (track_realloc) {
      collectStats("(trace buffer realloc)", false,
                   m_traceBuffer[m_nextTraceEntry++]);
    }
    {
      MemoryManager::MaskAlloc masker(MM());
      auto r = (TraceEntry*)realloc((void*)m_traceBuffer,
                                    new_array_size * sizeof(TraceEntry));

      if (!r) {
        m_traceBufferFilled = true;
        if (m_traceBuffer) {
          collectStats("(trace buffer terminated)", false,
                       m_traceBuffer[m_nextTraceEntry++]);
        }
        return false;
      }
      m_traceBufferSize = new_array_size;
      m_traceBuffer = r;
    }
    if (track_realloc) {
      collectStats("(trace buffer realloc)", true,
                   m_traceBuffer[m_nextTraceEntry++]);
    }
    return true;
  }

  virtual void beginFrame(const char *symbol) override {
    doTrace(symbol, false);
  }

  virtual void endFrame(const TypedValue *retval,
                        const char *symbol,
                        bool endMain = false) override {
    doTrace(symbol, true);
  }

  virtual void endAllFrames() override {
    if (m_traceBuffer && m_nextTraceEntry < m_traceBufferSize - 1) {
      collectStats(nullptr, true, m_finalEntry);
      m_traceBufferFilled = true;
    }
  }

  void collectStats(const char *symbol, bool isFuncExit, TraceEntry& te) {
    te.symbol = symbol;
    te.is_func_exit = isFuncExit;
    collectStats(te);
  }

  void collectStats(TraceData& te) {
    te.wall_time = cpuCycles();
    te.cpu = 0;
    if (m_flags & TrackCPU) {
      te.cpu = cpuTime(m_MHz);
    }
    if (m_flags & TrackMemory) {
      auto const& stats = MM().getStats();
      te.memory = stats.usage;
      te.peak_memory = stats.peakUsage;
    } else if (m_flags & TrackMalloc) {
      te.memory = get_allocs();
      te.peak_memory = get_frees();
    } else {
      te.memory = 0;
      te.peak_memory = 0;
    }
  }

  TraceEntry* nextTraceEntry() {
    if (!isTraceSpaceAvailable() && !ensureTraceSpace()) {
      return 0;
    }
    return &m_traceBuffer[m_nextTraceEntry++];
  }

  void doTrace(const char *symbol, bool isFuncExit) {
    TraceEntry *te = nextTraceEntry();
    if (te != nullptr) {
      collectStats(symbol, isFuncExit, *te);
    }
  }

  template<class TraceIterator, class Stats>
  void walkTrace(TraceIterator begin, TraceIterator end, TraceIterator final,
                 Stats& stats) {
    TraceWalker<TraceIterator, Stats> walker;
    walker.walk(begin, end, final, stats);
  }

  virtual void writeStats(Array &ret) override {
    TraceData my_begin;
    collectStats(my_begin);
    walkTrace(m_traceBuffer, m_traceBuffer + m_nextTraceEntry, &m_finalEntry,
              m_stats);
    if (m_overflowCalls) {
      m_stats["(trace buffer terminated)"].count += m_overflowCalls/2;
    }
    extractStats(ret, m_stats, m_flags, m_MHz);
    CountedTraceData allocStats;
    allocStats.count = 0;
    allocStats.peak_memory = allocStats.memory =
      m_nextTraceEntry * sizeof(*m_traceBuffer);
    returnVals(ret, "(trace buffer alloc)", allocStats, m_flags, m_MHz);
    if (m_flags & MeasureXhprofDisable) {
      CountedTraceData my_end;
      collectStats(my_end);
      my_end.count = 1;
      my_end.cpu -= my_begin.cpu;
      my_end.wall_time -= my_begin.wall_time;
      my_end.memory -= my_begin.memory;
      my_end.peak_memory -= my_begin.peak_memory;
      returnVals(ret, "xhprof_post_processing()", my_end, m_flags, m_MHz);
    }
  }

  virtual bool shouldSkipBuiltins() const override {
    return m_flags & NoTrackBuiltins;
  }

  TraceEntry* m_traceBuffer;
  TraceEntry m_finalEntry;
  int m_traceBufferSize;
  int m_nextTraceEntry;
  bool m_traceBufferFilled;
  int64_t m_maxTraceBuffer;
  int64_t m_overflowCalls;
  uint32_t m_flags;

  // Final stats, per-function per-callsite, with a count of how many
  // times the function was called from that callsite.
  class CountedTraceData : public TraceData {
  public:
    int64_t count;
    CountedTraceData() : count(0)  { clear(); }
  };
  typedef hphp_hash_map<std::string, CountedTraceData, string_hash> StatsMap;
  StatsMap m_stats; // outcome

  static pthread_mutex_t s_inUse;
};

pthread_mutex_t TraceProfiler::s_inUse = PTHREAD_MUTEX_INITIALIZER;

///////////////////////////////////////////////////////////////////////////////
// SampleProfiler

/**
 * Sampling based profiler.
 */
class SampleProfiler : public Profiler {
private:
  typedef std::pair<int64_t, int64_t> Timestamp;
  typedef req::vector<std::pair<Timestamp, std::string>> SampleVec;
  SampleVec m_samples; // outcome

public:
  SampleProfiler() : Profiler(true) {
    struct timeval  now;
    uint64_t truncated_us;
    uint64_t truncated_tsc;

    // Init the last_sample in tsc
    m_last_sample_tsc = cpuCycles();

    // Find the microseconds that need to be truncated
    gettimeofday(&m_last_sample_time, 0);
    now = m_last_sample_time;
    hp_trunc_time(&m_last_sample_time, SAMPLING_INTERVAL);

    // Subtract truncated time from last_sample_tsc
    truncated_us  = get_us_interval(&m_last_sample_time, &now);
    truncated_tsc = truncated_us * m_MHz;
    if (m_last_sample_tsc > truncated_tsc) {
      // just to be safe while subtracting unsigned ints
      m_last_sample_tsc -= truncated_tsc;
    }

    // Convert sampling interval to ticks
    m_sampling_interval_tsc = SAMPLING_INTERVAL * m_MHz;
  }

  virtual void beginFrameEx(const char *symbol) override {
    sample_check();
  }

  virtual void endFrameEx(const TypedValue *retvalue,
                          const char *symbol) override {
    sample_check();
  }

  virtual void writeStats(Array &ret) override {
    for (auto const& sample : m_samples) {
      auto const& time = sample.first;
      char timestr[512];
      snprintf(timestr, sizeof(timestr), "%" PRId64 ".%06" PRId64,
               time.first, time.second);

      ret.set(String(timestr), String(sample.second));
    }
  }

private:
  static const int SAMPLING_INTERVAL = 100000; // microsecs

  struct timeval m_last_sample_time;
  uint64_t m_last_sample_tsc;
  uint64_t m_sampling_interval_tsc;

  /**
   * Sample the stack. Add it to the stats_count global.
   *
   * @param  tv            current time
   * @param  entries       func stack as linked list of hprof_entry_t
   * @return void
   * @author veeve
   */
  void sample_stack() {
    char symbol[5120];
    m_stack->getStack(INT_MAX, symbol, sizeof(symbol));

    auto time = std::make_pair((int64_t)m_last_sample_time.tv_sec,
                               (int64_t)m_last_sample_time.tv_usec);
    m_samples.push_back(std::make_pair(time, symbol));
  }

  /**
   * Checks to see if it is time to sample the stack.
   * Calls hp_sample_stack() if its time.
   *
   * @param  entries        func stack as linked list of hprof_entry_t
   * @param  last_sample    time the last sample was taken
   * @param  sampling_intr  sampling interval in microsecs
   * @return void
   * @author veeve
   */
  void sample_check() {
    if (m_stack) {
      // While loop is to handle a single function taking a long time
      // and passing several sampling intervals
      while ((cpuCycles() - m_last_sample_tsc) > m_sampling_interval_tsc) {
        m_last_sample_tsc += m_sampling_interval_tsc;
        // HAS TO BE UPDATED BEFORE calling sample_stack
        incr_us_interval(&m_last_sample_time, SAMPLING_INTERVAL);
        sample_stack();
      }
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
// Memoization Opportunity Profiler
//
// Identifies potential memoization opportunities by comparing the
// serialized form of function return values. Functions which return
// the same data every time, but not the same instance, are
// candidates.
//
// For member functions, the args are serialized and kept along with
// the 'this' pointer, and used to look for constant returns given the
// same args.
//
// This profiler is very, very slow. It tries to be faster by giving
// up on functions quickly, and making a quick test to ignore them
// later. It also ignores functions which return "small"
// things. Nevertheless, you likely need to adjust
// Server.RequestTimeoutSeconds to get a full request processed.
//
// Future options:
// - Collect the data cross-request to find APC opportunities.
// - Auto-identify keys to functions which could be used to place things in APC
//   - i.e., same per-request, but not cross-request unless, say, locale is
//     considered.
//
// @TODO: this is quite ghetto right now, but is useful as is to
// others. In particular, it should provide the results via the return
// value from writeStats, not print to stderr :) Task 3396401 tracks this.

class MemoProfiler : public Profiler {
 public:
  explicit MemoProfiler(int flags) : Profiler(true) {}

  ~MemoProfiler() {
  }

 private:
  virtual void beginFrame(const char *symbol) override {
    VMRegAnchor _;
    ActRec *ar = vmfp();
    Frame f(symbol);
    if (ar->hasThis()) {
      auto& memo = m_memos[symbol];
      if (!memo.m_ignore) {
        auto args = hhvm_get_frame_args(ar, 0);
        args.append((int64_t)(ar->getThis())); // Use the pointer not the obj
        VariableSerializer vs(VariableSerializer::Type::DebuggerSerialize);
        String sdata;
        try {
          sdata = vs.serialize(args, true);
          f.m_args = sdata;
        } catch (...) {
          fprintf(stderr, "Args Serialization failure: %s\n", symbol);
        }
      }
    }
    m_stack.push_back(f);
  }

  virtual void endFrame(const TypedValue *retval,
                        const char *symbol,
                        bool endMain = false) override {
    if (m_stack.empty()) {
      fprintf(stderr, "STACK IMBALANCE empty %s\n", symbol);
      return;
    }
    auto f = m_stack.back();
    m_stack.pop_back();
    if (strcmp(f.m_symbol, symbol) != 0) {
      fprintf(stderr, "STACK IMBALANCE %s\n", symbol);
      return;
    }
    auto& memo = m_memos[symbol];
    if (memo.m_ignore) return;
    ++memo.m_count;
    memo.m_ignore = true;
    VMRegAnchor _;
    ActRec *ar = vmfp();
    // Lots of random cases to skip just to keep this simple for
    // now. There's no reason not to do more later.
    if (!g_context->m_faults.empty()) return;
    if (ar->m_func->isCPPBuiltin() || ar->resumed()) return;
    auto ret = tvAsCVarRef(retval);
    if (ret.isNull()) return;
    if (!(ret.isString() || ret.isObject() || ret.isArray())) return;
    VariableSerializer vs(VariableSerializer::Type::DebuggerSerialize);
    String sdata;
    try {
      sdata = vs.serialize(ret, true);
    } catch (...) {
      fprintf(stderr, "Serialization failure: %s\n", symbol);
      return;
    }
    if (sdata.length() < 3) return;
    if (ar->hasThis()) {
      memo.m_has_this = true;
      auto& member_memo = memo.m_member_memos[f.m_args.data()];
      ++member_memo.m_count;
      if (member_memo.m_return_value.length() == 0) { // First time
        member_memo.m_return_value = sdata;
        // Intentionally copy the raw pointer value
        member_memo.m_ret_tv = *retval;
        memo.m_ignore = false;
      } else if (member_memo.m_return_value == sdata) { // Same
        memo.m_ignore = false;
        if ((member_memo.m_ret_tv.m_data.num != retval->m_data.num) ||
            (member_memo.m_ret_tv.m_type != retval->m_type)) {
          memo.m_ret_tv_same = false;
        }
      } else {
        memo.m_member_memos.clear(); // Cleanup and ignore
      }
    } else {
      if (memo.m_return_value.length() == 0) { // First time
        memo.m_return_value = sdata;
        // Intentionally copy the raw pointer value
        memo.m_ret_tv = *retval;
        memo.m_ignore = false;
      } else if (memo.m_return_value == sdata) { // Same
        memo.m_ignore = false;
        if ((memo.m_ret_tv.m_data.num != retval->m_data.num) ||
            (memo.m_ret_tv.m_type != retval->m_type)) {
          memo.m_ret_tv_same = false;
        }
      } else {
        memo.m_return_value = ""; // Different, cleanup and ignore
      }
    }
  }

  virtual void endAllFrames() override {
    // Nothing to do for this profiler since all work is done as we go.
  }

  virtual void writeStats(Array &ret) override {
    fprintf(stderr, "writeStats start\n");
    // RetSame: the return value is the same instance every time
    // HasThis: call has a this argument
    // AllSame: all returns were the same data even though args are different
    // MemberCount: number of different arg sets (including this)
    fprintf(stderr, "Count Function MinSerLen MaxSerLen RetSame HasThis "
            "AllSame MemberCount\n");
    for (auto& me : m_memos) {
      if (me.second.m_ignore) continue;
      if (me.second.m_count == 1) continue;
      int min_ser_len = 999999999;
      int max_ser_len = 0;
      int count = 0;
      int member_count = 0;
      bool all_same = true;
      if (me.second.m_has_this) {
        bool any_multiple = false;
        auto& fr = me.second.m_member_memos.begin()->second.m_return_value;
        member_count = me.second.m_member_memos.size();
        for (auto& mme : me.second.m_member_memos) {
          if (mme.second.m_return_value != fr) all_same = false;
          count += mme.second.m_count;
          auto ser_len = mme.second.m_return_value.length();
          min_ser_len = std::min(min_ser_len, ser_len);
          max_ser_len = std::max(max_ser_len, ser_len);
          if (mme.second.m_count > 1) any_multiple = true;
        }
        if (!any_multiple && !all_same) continue;
      } else {
        min_ser_len = max_ser_len = me.second.m_return_value.length();
        count = me.second.m_count;
        all_same = me.second.m_ret_tv_same;
      }
      fprintf(stderr, "%d %s %d %d %s %s %s %d\n",
              count, me.first.data(),
              min_ser_len, max_ser_len,
              me.second.m_ret_tv_same ? " true" : "false",
              me.second.m_has_this ? " true" : "false",
              all_same ? " true" : "false",
              member_count
             );
    }
    fprintf(stderr, "writeStats end\n");
  }

  struct MemberMemoInfo {
    String m_return_value;
    TypedValue m_ret_tv;
    int m_count{0};
  };
  using MemberMemoMap = hphp_hash_map<std::string, MemberMemoInfo, string_hash>;

  struct MemoInfo {
    MemberMemoMap m_member_memos; // Keyed by serialized args
    String m_return_value;
    TypedValue m_ret_tv;
    int m_count{0};
    bool m_ignore{false};
    bool m_has_this{false};
    bool m_ret_tv_same{true};
  };
  using MemoMap = hphp_hash_map<std::string, MemoInfo, string_hash>;

  struct Frame {
    explicit Frame(const char* symbol) : m_symbol(symbol) {}
    const char* m_symbol;
    String m_args;
  };

public:
  MemoMap m_memos; // Keyed by function name
  vector<Frame> m_stack;
};

///////////////////////////////////////////////////////////////////////////////
// ProfilerFactory

bool ProfilerFactory::start(ProfilerKind kind,
                            long flags,
                            bool beginFrame /* = true */) {
  if (m_profiler != nullptr) {
    return false;
  }

  switch (kind) {
  case ProfilerKind::Hierarchical:
    m_profiler = new HierarchicalProfiler(flags);
    break;
  case ProfilerKind::Sample:
    m_profiler = new SampleProfiler();
    break;
  case ProfilerKind::Trace:
    m_profiler = new TraceProfiler(flags);
    break;
  case ProfilerKind::Memo:
    m_profiler = new MemoProfiler(flags);
    break;
  case ProfilerKind::XDebug:
    m_profiler = new XDebugProfiler();
    break;
  case ProfilerKind::External:
    if (g_system_profiler) {
      m_profiler = g_system_profiler->getHotProfiler();
    } else if (m_external_profiler) {
      m_profiler = m_external_profiler;
    } else {
      throw_invalid_argument(
        "ProfilerFactory::setExternalProfiler() not yet called");
      return false;
    }
    break;
  default:
    throw_invalid_argument("level: %d", static_cast<int>(kind));
    return false;
  }
  if (m_profiler && m_profiler->m_successful) {
    // This will be disabled automatically when the thread completes the request
    HPHP::EventHook::Enable();
    ThreadInfo::s_threadInfo->m_profiler = m_profiler;
    if (beginFrame) {
      m_profiler->beginFrame("main()");
    }
    return true;
  } else {
    delete m_profiler;
    m_profiler = nullptr;
    return false;
  }
}

Variant ProfilerFactory::stop() {
  if (m_profiler) {
    m_profiler->endAllFrames();

    Array ret;
    m_profiler->writeStats(ret);
    delete m_profiler;
    m_profiler = nullptr;
    ThreadInfo::s_threadInfo->m_profiler = nullptr;

    return ret;
  }
  return init_null();
}

bool ProfilerFactory::EnableNetworkProfiler = false;

IMPLEMENT_REQUEST_LOCAL(ProfilerFactory, s_profiler_factory);

///////////////////////////////////////////////////////////////////////////////
// main functions

void f_hotprofiler_enable(int ikind) {
  auto kind = static_cast<ProfilerKind>(ikind);
  long flags = 0;
  if (kind == ProfilerKind::Hierarchical) {
    flags = NoTrackBuiltins;
  } else if (kind == ProfilerKind::Memory) {
    kind = ProfilerKind::Hierarchical;
    flags = NoTrackBuiltins | TrackMemory;
  }
  if (RuntimeOption::EnableHotProfiler) {
    s_profiler_factory->start(kind, flags);
  }
}

Variant f_hotprofiler_disable() {
  return s_profiler_factory->stop();
}

void f_phprof_enable(int flags /* = 0 */) {
  if (RuntimeOption::EnableHotProfiler) {
    s_profiler_factory->start(ProfilerKind::Hierarchical, flags);
  }
}

Variant f_phprof_disable() {
  return s_profiler_factory->stop();
}

///////////////////////////////////////////////////////////////////////////////
// injected code

void begin_profiler_frame(Profiler *p,
                          const char *symbol) {
  p->beginFrame(symbol);
}

void end_profiler_frame(Profiler *p,
                        const TypedValue *retval,
                        const char *symbol) {
  p->endFrame(retval, symbol);
}

///////////////////////////////////////////////////////////////////////////////
}
