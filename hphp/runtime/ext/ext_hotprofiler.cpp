/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_fb.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/alloc.h"
#include "hphp/util/vdso.h"
#include "hphp/util/cycles.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/ext/ext_function.h"

#ifdef __FreeBSD__
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/cpuset.h>
#define cpu_set_t cpuset_t
#define SET_AFFINITY(pid, size, mask) \
           cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
#define GET_AFFINITY(pid, size, mask) \
           cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
#elif defined(__APPLE__)
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>

#define cpu_set_t thread_affinity_policy_data_t
#define CPU_SET(cpu_id, new_mask) \
        (*(new_mask)).affinity_tag = (cpu_id + 1)
#define CPU_ZERO(new_mask)                 \
        (*(new_mask)).affinity_tag = THREAD_AFFINITY_TAG_NULL
#define GET_AFFINITY(pid, size, mask) \
         (*(mask)).affinity_tag = THREAD_AFFINITY_TAG_NULL
#define SET_AFFINITY(pid, size, mask)       \
        thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, \
                          (int *)mask, THREAD_AFFINITY_POLICY_COUNT)
#else
#include <sched.h>
#define SET_AFFINITY(pid, size, mask) sched_setaffinity(0, size, mask)
#define GET_AFFINITY(pid, size, mask) sched_getaffinity(0, size, mask)
#endif


#include <iostream>
#include <fstream>
#include <zlib.h>

// Append the delimiter
#define HP_STACK_DELIM        "==>"
#define HP_STACK_DELIM_LEN    (sizeof(HP_STACK_DELIM) - 1)

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION(hotprofiler);
IMPLEMENT_DEFAULT_EXTENSION(xhprof);
///////////////////////////////////////////////////////////////////////////////
// helpers

/*
 * A hash function to calculate a 8-bit hash code for a function name.
 * This is based on a small modification to 'zend_inline_hash_func' by summing
 * up all bytes of the ulong returned by 'zend_inline_hash_func'.
 *
 * @param str, char *, string to be calculated hash code for.
 *
 * @author cjiang
 */
static inline uint8_t hprof_inline_hash(const char * str) {
  unsigned long h = 5381;
  uint i = 0;
  uint8_t res = 0;

  while (*str) {
    h += (h << 5);
    h ^= (unsigned long) *str++;
  }

  for (i = 0; i < sizeof(unsigned long); i++) {
    res += ((uint8_t *)&h)[i];
  }
  return res;
}

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
  // Sleep for 5 miliseconds. Comparaing with gettimeofday's  few microseconds
  // execution time, this should be enough.
  usleep(5000);
  if (gettimeofday(&end, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64_t tsc_end = cpuCycles();
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

class esyscall {
public:
  int num;

  explicit esyscall(const char *syscall_name) {
    num = -1;
    char format[strlen(syscall_name) + sizeof(" %d")];
    sprintf(format, "%s %%d", syscall_name);

    std::ifstream syscalls("/proc/esyscall");
    if (syscalls.fail()) {
      return;
    }
    char line[MAX_LINELENGTH];
    if (!syscalls.getline(line, sizeof(line))) {
      return;
    }
    // perhaps we should check the format, but we're just going to assume
    // Name Number
    while (syscalls.getline(line, sizeof(line))) {
      int number;
      if (sscanf(line, format, &number) == 1) {
        num = number;
        return;
      }
    }
  }
};

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
  static int64_t vdso_usable =
    Util::Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID);
#else
  static int64_t vdso_usable = -1;
#endif

  if (cpu_time && vdso_usable >= 0)
    return cycles / 1000;
  return (cycles + MHz/2) / MHz;
}

static esyscall vtsc_syscall("vtsc");

static inline uint64_t vtsc(int64_t MHz) {
#ifdef CLOCK_THREAD_CPUTIME_ID
  int64_t rval = Util::Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID);
  if (rval >= 0) {
    return rval;
  }
#endif
  if (vtsc_syscall.num > 0) {
    return syscall(vtsc_syscall.num);
  }
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

///////////////////////////////////////////////////////////////////////////////
// classes

/**
 * All information we collect about a frame.
 */
class Frame {
public:
  Frame          *m_parent;      // ptr to parent frame
  const char     *m_name;        // function name
  uint8_t           m_hash_code;   // hash_code for the function name
  int             m_recursion;   // recursion level for function

  uint64_t          m_tsc_start;   // start value for TSC counter
  int64_t           m_mu_start;    // memory usage
  int64_t           m_pmu_start;   // peak memory usage
  int64_t           m_vtsc_start;    // user/sys time start

  /**
   * Returns formatted function name
   *
   * @param  result_buf   ptr to result buf
   * @param  result_len   max size of result buf
   * @return total size of the function name returned in result_buf
   * @author veeve
   */
  size_t getName(char *result_buf, size_t result_len) {
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

  /**
   * Build a caller qualified name for a callee.
   *
   * For example, if A() is caller for B(), then it returns "A==>B".
   * Recursive invokations are denoted with @<n> where n is the recursion
   * depth.
   *
   * For example, "foo==>foo@1", and "foo@2==>foo@3" are examples of direct
   * recursion. And  "bar==>foo@1" is an example of an indirect recursive
   * call to foo (implying the foo() is on the call stack some levels
   * above).
   */
  size_t getStack(int level, char *result_buf, size_t result_len) {
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
};

enum Flag {
  TrackBuiltins         = 0x1,
  TrackCPU              = 0x2,
  TrackMemory           = 0x4,
  TrackVtsc             = 0x8,
  XhpTrace              = 0x10,
  MeasureXhprofDisable  = 0x20,
  Unused                = 0x40,
  TrackMalloc           = 0x80,
};

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
class Profiler {
public:
  Profiler() : m_successful(true), m_stack(nullptr),
               m_frame_free_list(nullptr) {
    if (!s_rand_initialized) {
      s_rand_initialized = true;
      srand(math_generate_seed());
    }

    // bind to a random cpu so that we can use rdtsc instruction.
    int cur_cpu_id = rand() % s_machine.m_cpu_num;
    GET_AFFINITY(0, sizeof(cpu_set_t), &m_prev_mask);
    MachineInfo::BindToCPU(cur_cpu_id);
    m_MHz = s_machine.m_cpu_frequencies[cur_cpu_id];

    memset(m_func_hash_counters, 0, sizeof(m_func_hash_counters));
  }

  virtual ~Profiler() {
    SET_AFFINITY(0, sizeof(cpu_set_t), &m_prev_mask);

    endAllFrames();
    for (Frame *p = m_frame_free_list; p;) {
      Frame *cur = p;
      p = p->m_parent;
      free(cur);
    }
  }

  /**
   * Subclass can do extra work by overriding these two virtual functions.
   */
  virtual void beginFrameEx() {} // called right before a function call
  virtual void endFrameEx() {}   // called right after a function is finished

  /**
   * Final results.
   */
  virtual void writeStats(Array &ret) {}

  /**
   * Start a new frame with the specified symbol.
   */
  virtual void beginFrame(const char *symbol) __attribute__ ((noinline)) ;

  /**
   * End top of the stack.
   */
  virtual void endFrame(const char *symbol,
                        bool endMain = false) __attribute__ ((noinline)) ;

  virtual void endAllFrames() {
    while (m_stack) {
      endFrame(nullptr, true);
    }
  }

  template<class phpret, class Name, class Counts>
  static void returnVals(phpret& ret, const Name& name, const Counts& counts,
                  int flags, int64_t MHz)
  {
    ArrayInit arr(5);
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
    ret.set(String(name), arr.create());
  }

  template<class phpret, class StatsMap>
  static bool extractStats(phpret& ret, StatsMap& stats, int flags, int64_t MHz)
  {
    for (typename StatsMap::const_iterator iter = stats.begin();
         iter != stats.end(); ++iter) {
      returnVals(ret, iter->first, iter->second, flags, MHz);
    }
    return true;
  }

  bool m_successful;

  int64_t    m_MHz; // cpu freq for either the local cpu or the saved trace
  Frame    *m_stack;      // top of the profile stack

  static bool s_rand_initialized;

  cpu_set_t m_prev_mask;               // saved cpu affinity
  Frame    *m_frame_free_list;         // freelist of Frame
  uint8_t     m_func_hash_counters[256]; // counter table by hash code;

  /**
   * Fast allocate a Frame structure. Picks one from the
   * free list if available, else does an actual allocate.
   */
  Frame *createFrame(const char *symbol) {
    Frame *p = m_frame_free_list;
    if (p) {
      m_frame_free_list = p->m_parent;
    } else {
      p = (Frame*)malloc(sizeof(Frame));
    }
    p->m_parent = m_stack;
    p->m_name = symbol;
    p->m_hash_code = hprof_inline_hash(symbol);
    m_stack = p;
    return p;
  }

  /**
   * Fast free a Frame structure. Simply returns back the Frame to a free list
   * and doesn't actually perform the free.
   */
  void releaseFrame() {
    assert(m_stack);

    Frame *p = m_stack;
    m_stack = p->m_parent;
    p->m_parent = m_frame_free_list; // we overload the m_parent field here
    m_frame_free_list = p;
  }
};
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
  beginFrameEx();
}

/**
 * End top of the stack.
 */
void Profiler::endFrame(const char *symbol, bool endMain) {
  if (m_stack) {
    // special case for main() frame that's only ended by endAllFrames()
    if (!endMain && m_stack->m_parent == nullptr) {
      return;
    }
    endFrameEx();
    m_func_hash_counters[m_stack->m_hash_code]--;
    releaseFrame();
  }
}

///////////////////////////////////////////////////////////////////////////////
// SimpleProfiler

/**
 * vtsc() based profiler, but simple enough to print basic information.
 *
 * When available, we now use the vtsc() call, which is relatively inexpensive
 * and accurate.  It's still a system call, but a cheap one.  If the call isn't
 * available, the comment below still applies.  --renglish
 *
 * COMMENT(cjiang): getrusage is very expensive and inaccurate. It is based
 * on sampling at the rate about once every 5 to 10 miliseconds. The sampling
 * error can be very significantly, especially given that we are
 * instrumenting at a very fine granularity. (every PHP function call will
 * lead to one invokation of getrusage.) Most PHP functions such as the
 * built-ins typically finish in microseconds. Thus the result we get from
 * getrusage is very likely going to be skewed. Also worth noting that
 * getrusage actually is a system call, which involves expensive swapping
 * between user-mode and kernel mode. I would suggest we remove collecting
 * CPU usage all together, as exclusive wall-time is very useful already.
 * Or at least we should make it an opt-in choice.
 *
 * See: http://ww2.cs.fsu.edu/~hines/present/timing_linux.pdf
 *
 * Above is a nice paper talking about the overhead and the inaccuracy problem
 * associated with getrusage.
 */
class SimpleProfiler : public Profiler {
private:
  class CountMap {
  public:
    CountMap() : count(0), tsc(0), vtsc(0) {}

    int64_t count;
    int64_t tsc;
    int64_t vtsc;
  };
  typedef hphp_hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:
  SimpleProfiler() {
    echo("<div style='display:none'>");
  }

  ~SimpleProfiler() {
    echo("</div>");
    print_output();
  }

  virtual void beginFrameEx() {
    m_stack->m_tsc_start = cpuCycles();
    m_stack->m_vtsc_start = vtsc(m_MHz);
  }

  virtual void endFrameEx() {
    CountMap &counts = m_stats[m_stack->m_name];
    counts.count++;
    counts.tsc += cpuCycles() - m_stack->m_tsc_start;
    counts.vtsc += vtsc(m_MHz) - m_stack->m_vtsc_start;
  }

private:
  void print_output() {
    echo("<link rel='stylesheet' href='/css/hotprofiler.css' type='text/css'>"
          "<script language='javascript' src='/js/hotprofiler.js'></script>"
          "<p><center><h2>Hotprofiler Data</h2></center><br>"
          "<div id='hotprofiler_stats'></div>"
          "<script language='javascript'>hotprofiler_data = [");
    for (StatsMap::const_iterator iter = m_stats.begin();
         iter != m_stats.end(); ++iter) {
      echo("{\"fn\": \"");
      echo(iter->first.c_str());
      echo("\"");

      const CountMap &counts = iter->second;

      char buf[512];
      snprintf(
        buf, sizeof(buf),
        ",\"ct\": %" PRId64 ",\"wt\": %" PRId64 ",\"ut\": %" PRId64 ",\"st\": 0",
        counts.count, (int64_t)to_usec(counts.tsc, m_MHz),
        (int64_t)to_usec(counts.vtsc, m_MHz, true));
      echo(buf);

      echo("},\n");
    }
    echo("]; write_data('ut', false);</script><br><br>&nbsp;<br>");
  }
};

///////////////////////////////////////////////////////////////////////////////
// HierarchicalProfiler

class HierarchicalProfiler : public Profiler {
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
  typedef hphp_hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:

public:
  explicit HierarchicalProfiler(int flags) : m_flags(flags) {
  }

  virtual void beginFrameEx() {
    m_stack->m_tsc_start = cpuCycles();

    if (m_flags & TrackCPU) {
      m_stack->m_vtsc_start = vtsc(m_MHz);
    }

    if (m_flags & TrackMemory) {
      auto const& stats = MM().getStats();
      m_stack->m_mu_start  = stats.usage;
      m_stack->m_pmu_start = stats.peakUsage;
    } else if (m_flags & TrackMalloc) {
      m_stack->m_mu_start = get_allocs();
      m_stack->m_pmu_start = get_frees();
    }
  }

  virtual void endFrameEx() {
    char symbol[512];
    m_stack->getStack(2, symbol, sizeof(symbol));
    CountMap &counts = m_stats[symbol];
    counts.count++;
    counts.wall_time += cpuCycles() - m_stack->m_tsc_start;

    if (m_flags & TrackCPU) {
      counts.cpu += vtsc(m_MHz) - m_stack->m_vtsc_start;
    }

    if (m_flags & TrackMemory) {
      auto const& stats = MM().getStats();
      int64_t mu_end = stats.usage;
      int64_t pmu_end = stats.peakUsage;
      counts.memory += mu_end - m_stack->m_mu_start;
      counts.peak_memory += pmu_end - m_stack->m_pmu_start;
    } else if (m_flags & TrackMalloc) {
      counts.memory += get_allocs() - m_stack->m_mu_start;
      counts.peak_memory += get_frees() - m_stack->m_pmu_start;
    }
  }

  virtual void writeStats(Array &ret) {
    extractStats(ret, m_stats, m_flags, m_MHz);
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
        fr.len = symbolLength(current->symbol);
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

  // Computes the length of the symbol without $continuation on the
  // end, to ensure that work done in continuations and the original
  // function are counted together.
  int symbolLength(const char* symbol) {
    auto len = strlen(symbol);
    if ((len > 13) && (strcmp(&symbol[len - 13], "$continuation") == 0)) {
      return len - 13;
    }
    return len;
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
    : m_traceBuffer(nullptr)
    , m_traceBufferSize(0)
    , m_nextTraceEntry(0)
    , m_traceBufferFilled(false)
    , m_maxTraceBuffer(0)
    , m_overflowCalls(0)
    , m_flags(flags)
  {
    if (pthread_mutex_trylock(&s_inUse)) {
      // This profiler uses a very large amount of memory. Only allow
      // one in the process at any time.
      m_successful = false;
    } else {
      char buf[20];
      sprintf(buf, "%d", RuntimeOption::ProfilerMaxTraceBuffer);
      IniSetting::Bind("profiler.max_trace_buffer", buf,
                       ini_on_update_long, ini_get_long,
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
    bool track_realloc = FALSE;
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
      track_realloc = TRUE;
    }
    if (track_realloc) {
      collectStats("(trace buffer realloc)", false,
                   m_traceBuffer[m_nextTraceEntry++]);
    }
    {
      DECLARE_THREAD_INFO
      MemoryManager::MaskAlloc masker(*info->m_mm);
      TraceEntry *r = (TraceEntry*)realloc((void *)m_traceBuffer,
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

  virtual void beginFrame(const char *symbol) {
    doTrace(symbol, false);
  }

  virtual void endFrame(const char *symbol, bool endMain = false) {
    doTrace(symbol, true);
  }

  virtual void endAllFrames() {
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
      te.cpu = vtsc(m_MHz);
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

  virtual void writeStats(Array &ret) {
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
  typedef hphp_hash_map<std::string, int64_t, string_hash> CountMap;
  typedef hphp_hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:
  SampleProfiler() {
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

  virtual void beginFrameEx() {
    sample_check();
  }

  virtual void endFrameEx() {
    sample_check();
  }

  virtual void writeStats(Array &ret) {
    for (StatsMap::const_iterator iter = m_stats.begin();
         iter != m_stats.end(); ++iter) {
      Array arr;
      const CountMap &counts = iter->second;
      for (CountMap::const_iterator iterCount = counts.begin();
           iterCount != counts.end(); ++iterCount) {
        arr.set(String(iterCount->first), iterCount->second);
      }
      ret.set(String(iter->first), arr);
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
    char key[512];
    snprintf(key, sizeof(key), "%" PRId64 ".%06" PRId64,
             (int64_t)m_last_sample_time.tv_sec,
             (int64_t)m_last_sample_time.tv_usec);

    char symbol[5120];
    m_stack->getStack(INT_MAX, symbol, sizeof(symbol));
    m_stats[key][symbol] = 1;
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
  explicit MemoProfiler(int flags)
    : m_flags(flags)
  {
  }

  ~MemoProfiler() {
  }

 private:
  virtual void beginFrame(const char *symbol) {
    JIT::VMRegAnchor _;
    ActRec *ar = g_vmContext->getFP();
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

  virtual void endFrame(const char *symbol, bool endMain = false) {
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
    JIT::VMRegAnchor _;
    ActRec *ar = g_vmContext->getFP();
    // Lots of random cases to skip just to keep this simple for
    // now. There's no reason not to do more later.
    if (!g_vmContext->m_faults.empty()) return;
    if (ar->m_func->isCPPBuiltin() || ar->m_func->isGenerator()) return;
    auto ret_tv = g_vmContext->m_stack.topTV();
    auto ret = tvAsCVarRef(ret_tv);
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
        member_memo.m_ret_tv = *ret_tv;
        memo.m_ignore = false;
      } else if (member_memo.m_return_value == sdata) { // Same
        memo.m_ignore = false;
        if ((member_memo.m_ret_tv.m_data.num != ret_tv->m_data.num) ||
            (member_memo.m_ret_tv.m_type != ret_tv->m_type)) {
          memo.m_ret_tv_same = false;
        }
      } else {
        memo.m_member_memos.clear(); // Cleanup and ignore
      }
    } else {
      if (memo.m_return_value.length() == 0) { // First time
        memo.m_return_value = sdata;
        // Intentionally copy the raw pointer value
        memo.m_ret_tv = *ret_tv;
        memo.m_ignore = false;
      } else if (memo.m_return_value == sdata) { // Same
        memo.m_ignore = false;
        if ((memo.m_ret_tv.m_data.num != ret_tv->m_data.num) ||
            (memo.m_ret_tv.m_type != ret_tv->m_type)) {
          memo.m_ret_tv_same = false;
        }
      } else {
        memo.m_return_value = ""; // Different, cleanup and ignore
      }
    }
  }

  virtual void endAllFrames() {
    // Nothing to do for this profiler since all work is done as we go.
  }

  virtual void writeStats(Array &ret) {
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

  class MemberMemoInfo {
   public:
    MemberMemoInfo()
        : m_count(0)
      {}

    String m_return_value;
    TypedValue m_ret_tv;
    int m_count;
  };
  typedef hphp_hash_map<std::string, MemberMemoInfo, string_hash>
    MemberMemoMap;

  class MemoInfo {
   public:
    MemoInfo()
        : m_count(0)
        , m_ignore(false)
        , m_has_this(false)
        , m_ret_tv_same(true)
      {}

    MemberMemoMap m_member_memos; // Keyed by serialized args
    String m_return_value;
    TypedValue m_ret_tv;
    int m_count;
    bool m_ignore;
    bool m_has_this;
    bool m_ret_tv_same;
  };
  typedef hphp_hash_map<std::string, MemoInfo, string_hash> MemoMap;
  MemoMap m_memos; // Keyed by function name

  class Frame {
   public:
    explicit Frame(const char* symbol)
        : m_symbol(symbol)
      {}

    const char* m_symbol;
    String m_args;
  };
  vector<Frame> m_stack;

  uint32_t m_flags;
};

///////////////////////////////////////////////////////////////////////////////

class ProfilerFactory : public RequestEventHandler {
public:
  enum Level {
    Simple       = 1,
    Hierarchical = 2,
    Memory       = 3,
    Trace        = 4,
    Memo         = 5,
    Sample       = 620002, // Rockfort's zip code
  };

  static bool EnableNetworkProfiler;

public:
  ProfilerFactory() : m_profiler(nullptr) {
  }

  ~ProfilerFactory() {
    stop();
  }

  Profiler *getProfiler() {
    return m_profiler;
  }

  virtual void requestInit() {
  }

  virtual void requestShutdown() {
    stop();
    m_artificialFrameNames.reset();
  }

  void start(Level level, long flags) {
    if (!RuntimeOption::EnableHotProfiler) {
      return;
    }
    // This will be disabled automatically when the thread completes the request
    HPHP::EventHook::Enable();
    if (m_profiler == nullptr) {
      switch (level) {
      case Simple:
        m_profiler = new SimpleProfiler();
        break;
      case Hierarchical:
        m_profiler = new HierarchicalProfiler(flags);
        break;
      case Sample:
        m_profiler = new SampleProfiler();
        break;
      case Trace:
        m_profiler = new TraceProfiler(flags);
        break;
      case Memo:
        m_profiler = new MemoProfiler(flags);
        break;
      default:
        throw_invalid_argument("level: %d", level);
        return;
      }
      if (m_profiler->m_successful) {
        m_profiler->beginFrame("main()");
        ThreadInfo::s_threadInfo->m_profiler = m_profiler;
      } else {
        delete m_profiler;
        m_profiler = nullptr;
      }
    }
  }

  Variant stop() {
    if (m_profiler) {
      m_profiler->endAllFrames();

      Array ret;
      m_profiler->writeStats(ret);
      delete m_profiler;
      m_profiler = nullptr;
      ThreadInfo::s_threadInfo->m_profiler = nullptr;

      return ret;
    }
    return uninit_null();
  }

  /**
   * The whole purpose to make sure "const char *" is safe to take on these
   * strings.
   */
  void cacheString(const String& name) {
    m_artificialFrameNames.append(name);
  }

private:
  Profiler *m_profiler;
  Array m_artificialFrameNames;
};

bool ProfilerFactory::EnableNetworkProfiler = false;

#ifdef HOTPROFILER
IMPLEMENT_STATIC_REQUEST_LOCAL(ProfilerFactory, s_factory);
#endif

///////////////////////////////////////////////////////////////////////////////
// main functions

void f_hotprofiler_enable(int level) {
#ifdef HOTPROFILER
  long flags = 0;
  if (level == ProfilerFactory::Hierarchical) {
    flags = TrackBuiltins;
  } else if (level == ProfilerFactory::Memory) {
    level = ProfilerFactory::Hierarchical;
    flags = TrackBuiltins | TrackMemory;
  }
  s_factory->start((ProfilerFactory::Level)level, flags);
#endif
}

Variant f_hotprofiler_disable() {
#ifdef HOTPROFILER
  return s_factory->stop();
#else
  return uninit_null();
#endif
}

void f_phprof_enable(int flags /* = 0 */) {
#ifdef HOTPROFILER
  s_factory->start(ProfilerFactory::Hierarchical, flags);
#endif
}

Variant f_phprof_disable() {
#ifdef HOTPROFILER
  return s_factory->stop();
#else
  return uninit_null();
#endif
}

void f_fb_setprofile(CVarRef callback) {
#ifdef HOTPROFILER
  if (ThreadInfo::s_threadInfo->m_profiler != nullptr) {
    // phpprof is enabled, don't let PHP code override it
    return;
  }
#endif
  g_vmContext->m_setprofileCallback = callback;
  if (callback.isNull()) {
    HPHP::EventHook::Disable();
  } else {
    HPHP::EventHook::Enable();
  }
}

void f_xhprof_frame_begin(const String& name) {
#ifdef HOTPROFILER
  Profiler *prof = ThreadInfo::s_threadInfo->m_profiler;
  if (prof) {
    s_factory->cacheString(name);
    prof->beginFrame(name.data());
  }
#endif
}

void f_xhprof_frame_end() {
#ifdef HOTPROFILER
  Profiler *prof = ThreadInfo::s_threadInfo->m_profiler;
  if (prof) {
    prof->endFrame(nullptr);
  }
#endif
}

void f_xhprof_enable(int flags/* = 0 */,
                     CArrRef args /* = null_array */) {
#ifdef HOTPROFILER
#ifdef CLOCK_THREAD_CPUTIME_ID
  bool missingClockGetTimeNS =
    Util::Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID) == -1;
#else
  bool missingClockGetTimeNS = true;
#endif
  if (vtsc_syscall.num <= 0 && missingClockGetTimeNS) {
    flags &= ~TrackVtsc;
  }
  if (flags & TrackVtsc) {
    flags |= TrackCPU;
  }
  if (flags & XhpTrace) {
    s_factory->start(ProfilerFactory::Trace, flags);
  } else {
    s_factory->start(ProfilerFactory::Hierarchical, flags);
  }
#endif
}

Variant f_xhprof_disable() {
#ifdef HOTPROFILER
  return s_factory->stop();
#else
  return uninit_null();
#endif
}

void f_xhprof_network_enable() {
  ServerStats::StartNetworkProfile();
}

Variant f_xhprof_network_disable() {
  return ServerStats::EndNetworkProfile();
}

void f_xhprof_sample_enable() {
#ifdef HOTPROFILER
  s_factory->start(ProfilerFactory::Sample, 0);
#endif
}

Variant f_xhprof_sample_disable() {
#ifdef HOTPROFILER
  return s_factory->stop();
#else
  return uninit_null();
#endif
}

///////////////////////////////////////////////////////////////////////////////
// constants
const int64_t k_XHPROF_FLAGS_NO_BUILTINS = TrackBuiltins;
const int64_t k_XHPROF_FLAGS_CPU = TrackCPU;
const int64_t k_XHPROF_FLAGS_MEMORY = TrackMemory;
const int64_t k_XHPROF_FLAGS_VTSC = TrackVtsc;
const int64_t k_XHPROF_FLAGS_TRACE = XhpTrace;
const int64_t k_XHPROF_FLAGS_MEASURE_XHPROF_DISABLE = MeasureXhprofDisable;
const int64_t k_XHPROF_FLAGS_MALLOC = TrackMalloc;

///////////////////////////////////////////////////////////////////////////////
// injected code

void begin_profiler_frame(Profiler *p, const char *symbol) {
  p->beginFrame(symbol);
}

void end_profiler_frame(Profiler *p, const char *symbol) {
  p->endFrame(symbol);
}

///////////////////////////////////////////////////////////////////////////////
}
