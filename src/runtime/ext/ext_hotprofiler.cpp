/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_fb.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/zend/zend_math.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/ini_setting.h>
#include <util/alloc.h>

#ifdef __FreeBSD__
# include <sys/resource.h>
# include <sys/cpuset.h>
# define cpu_set_t cpuset_t
# define SET_AFFINITY(pid, size, mask) \
           cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
# define GET_AFFINITY(pid, size, mask) \
           cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
#elif __APPLE__
# include <mach/mach_init.h>
# include <mach/thread_policy.h>
# include <mach/thread_act.h>

# define cpu_set_t thread_affinity_policy_data_t
# define CPU_SET(cpu_id, new_mask) \
        (*(new_mask)).affinity_tag = (cpu_id + 1)
# define CPU_ZERO(new_mask)                 \
        (*(new_mask)).affinity_tag = THREAD_AFFINITY_TAG_NULL
# define GET_AFFINITY(pid, size, mask) \
         (*(mask)).affinity_tag = THREAD_AFFINITY_TAG_NULL
# define SET_AFFINITY(pid, size, mask)       \
        thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, \
                          (int *)mask, THREAD_AFFINITY_POLICY_COUNT)
#else
# include <sched.h>
# define SET_AFFINITY(pid, size, mask) sched_setaffinity(0, size, mask)
# define GET_AFFINITY(pid, size, mask) sched_getaffinity(0, size, mask)
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
static inline uint8 hprof_inline_hash(const char * str) {
  unsigned long h = 5381;
  uint i = 0;
  uint8 res = 0;

  while (*str) {
    h += (h << 5);
    h ^= (unsigned long) *str++;
  }

  for (i = 0; i < sizeof(unsigned long); i++) {
    res += ((uint8 *)&h)[i];
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
static void incr_us_interval(struct timeval *start, uint64 incr) {
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
static void hp_trunc_time(struct timeval *tv, uint64 intr) {
  uint64 time_in_micro;

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
 * Get time stamp counter (TSC) value via 'rdtsc' instruction.
 *
 * @return 64 bit unsigned integer
 * @author cjiang
 */
inline uint64 tsc() {
  uint32 __a,__d;
  uint64 val;
  asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
  (val) = ((uint64)__a) | (((uint64)__d)<<32);
  return val;
}

/**
 * This is a microbenchmark to get cpu frequency the process is running on. The
 * returned value is used to convert TSC counter values to microseconds.
 *
 * @return int64.
 * @author cjiang
 */
static int64 get_cpu_frequency() {
  struct timeval start;
  struct timeval end;

  if (gettimeofday(&start, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64 tsc_start = tsc();
  // Sleep for 5 miliseconds. Comparaing with gettimeofday's  few microseconds
  // execution time, this should be enough.
  usleep(5000);
  if (gettimeofday(&end, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64 tsc_end = tsc();
  return nearbyint((tsc_end - tsc_start) * 1.0
                                   / (get_us_interval(&start, &end)));
}

#define MAX_LINELENGTH 1024

static int64* get_cpu_frequency_from_file(const char *file, int ncpus)
{
  std::ifstream cpuinfo(file);
  if (cpuinfo.fail()) {
    return NULL;
  }
  char line[MAX_LINELENGTH];
  int64* freqs = new int64[ncpus];
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
      return NULL;
    }
  }
  return freqs;
}

class esyscall {
public:
  int num;

  esyscall(const char *syscall_name)
  {
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
  static void BindToCPU(uint32 cpu_id) {
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    CPU_SET(cpu_id, &new_mask);
    SET_AFFINITY(0, sizeof(cpu_set_t), &new_mask);
  }

public:
  // The number of logical CPUs this machine has.
  int m_cpu_num;
  // Store the cpu frequency.  Get it from /proc/cpuinfo if we can.
  int64* m_cpu_frequencies;

  MachineInfo() {
    m_cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    m_cpu_frequencies = get_cpu_frequency_from_file("/proc/cpuinfo", m_cpu_num);

    if (m_cpu_frequencies)
      return;

    m_cpu_frequencies = new int64[m_cpu_num];
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

static inline uint64
tv_to_cycles(const struct timeval& tv, int64 MHz)
{
  return (((uint64)tv.tv_sec * 1000000) + tv.tv_usec) * MHz;
}

static inline uint64
to_usec(int64 cycles, int64 MHz)
{
  return (cycles + MHz/2) / MHz;
}

static esyscall vtsc_syscall("vtsc");

static inline uint64 vtsc(int64 MHz) {
  if (vtsc_syscall.num > 0) {
    return syscall(vtsc_syscall.num);
  }
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  return
    tv_to_cycles(usage.ru_utime, MHz) + tv_to_cycles(usage.ru_stime, MHz);
}

#ifndef NO_JEMALLOC

#define JEMALLOC_STAT_MIB_LEN 2
size_t mallctl_mib_len = JEMALLOC_STAT_MIB_LEN;
size_t *mallctl_alloc_mib;
size_t *mallctl_free_mib;
bool mallctl_init = false;

bool
mallctl_mib_init()
{
  if (mallctl_init) {
    return false;
  }
  mallctl_init = true;
  mallctl_alloc_mib = new size_t[mallctl_mib_len];
  if (mallctlnametomib("thread.allocated",
                       mallctl_alloc_mib, &mallctl_mib_len))
  {
    return false;
  }
  mallctl_free_mib = new size_t[mallctl_mib_len];
  if (mallctlnametomib("thread.deallocated",
                       mallctl_free_mib, &mallctl_mib_len))
  {
    return false;
  }
  return true;
}
#endif

uint64
get_allocs()
{
#ifndef NO_JEMALLOC
  if (mallctl) {
    if (!mallctl_alloc_mib) {
      if (!mallctl_mib_init()) {
        return 0;
      }
    }
    uint64 stat;
    size_t size = sizeof(stat);
    if (mallctlbymib(mallctl_alloc_mib, mallctl_mib_len, &stat, &size, NULL, 0))
    {
      return 0;
    }
    return stat;
  }
#endif
#ifndef NO_TCMALLOC
  if (MallocExtensionInstance) {
    size_t stat;
    MallocExtensionInstance()->GetNumericProperty(
           "generic.thread_bytes_allocated", &stat);
    return stat;
  }
#endif
  return 0;
}

uint64
get_frees()
{
#ifndef NO_JEMALLOC
  if (mallctl) {
    if (!mallctl_free_mib) {
      if (!mallctl_mib_init()) {
        return 0;
      }
    }
    uint64 stat;
    size_t size = sizeof(stat);
    if (mallctlbymib(mallctl_free_mib, mallctl_mib_len, &stat, &size, NULL, 0))
    {
      return 0;
    }
    return stat;
  }
#endif
#ifndef NO_TCMALLOC
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
  uint8           m_hash_code;   // hash_code for the function name
  int             m_recursion;   // recursion level for function

  uint64          m_tsc_start;   // start value for TSC counter
  int64           m_mu_start;    // memory usage
  int64           m_pmu_start;   // peak memory usage
  int64           m_vtsc_start;    // user/sys time start

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
  Trace                 = 0x10,
  MeasureXhprofDisable  = 0x20,
  GetTrace              = 0x40,
  TrackMalloc           = 0x80,
};

/**
 * Maintain profiles of a running stack.
 */
class Profiler {
public:
  Profiler() : m_successful(true), m_stack(NULL), m_frame_free_list(NULL) {
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
  virtual void endFrame(bool endMain = false) __attribute__ ((noinline)) ;

  virtual void endAllFrames() {
    while (m_stack) {
      endFrame(true);
    }
  }

  template<class phpret, class Name, class Counts>
  static void returnVals(phpret& ret, const Name& name, const Counts& counts,
                  int flags, int64 MHz)
  {
    Array arr;
    arr.set("ct",  counts.count);
    arr.set("wt",  to_usec(counts.wall_time, MHz));
    if (flags & TrackCPU) {
      arr.set("cpu", to_usec(counts.cpu, MHz));
    }
    if (flags & TrackMemory) {
      arr.set("mu",  counts.memory);
      arr.set("pmu", counts.peak_memory);
    } else if (flags & TrackMalloc) {
      arr.set("alloc", counts.memory);
      arr.set("free", counts.peak_memory);
    }
    ret.set(String(name), arr);
  }

  template<class phpret, class StatsMap>
  static bool extractStats(phpret& ret, StatsMap& stats, int flags, int64 MHz)
  {
    for (typename StatsMap::const_iterator iter = stats.begin();
         iter != stats.end(); ++iter) {
      returnVals(ret, iter->first, iter->second, flags, MHz);
    }
    return true;
  }

  bool m_successful;

  int64    m_MHz; // cpu freq for either the local cpu or the saved trace
  Frame    *m_stack;      // top of the profile stack

  static bool s_rand_initialized;

  cpu_set_t m_prev_mask;               // saved cpu affinity
  Frame    *m_frame_free_list;         // freelist of Frame
  uint8     m_func_hash_counters[256]; // counter table by hash code;

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
    ASSERT(m_stack);

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
void Profiler::endFrame(bool endMain) {
  if (m_stack) {
    // special case for main() frame that's only ended by endAllFrames()
    if (!endMain && m_stack->m_parent == NULL) {
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

    int64 count;
    int64 tsc;
    int64 vtsc;
  };
  typedef __gnu_cxx::hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:
  SimpleProfiler() {
    print("<div style='display:none'>");
  }

  ~SimpleProfiler() {
    print("</div>");
    print_output();
  }

  virtual void beginFrameEx() {
    m_stack->m_tsc_start = tsc();
    m_stack->m_vtsc_start = vtsc(m_MHz);
  }

  virtual void endFrameEx() {
    CountMap &counts = m_stats[m_stack->m_name];
    counts.count++;
    counts.tsc += tsc() - m_stack->m_tsc_start;
    counts.vtsc += vtsc(m_MHz) - m_stack->m_vtsc_start;
  }

private:
  void print_output() {
    print("<link rel='stylesheet' href='/css/hotprofiler.css' type='text/css'>"
          "<script language='javascript' src='/js/hotprofiler.js'></script>"
          "<p><center><h2>Hotprofiler Data</h2></center><br>"
          "<div id='hotprofiler_stats'></div>"
          "<script language='javascript'>hotprofiler_data = [");
    for (StatsMap::const_iterator iter = m_stats.begin();
         iter != m_stats.end(); ++iter) {
      print("{\"fn\": \"");
      print(iter->first.c_str());
      print("\"");

      const CountMap &counts = iter->second;

      char buf[512];
      snprintf(buf, sizeof(buf),
               ",\"ct\": %lld,\"wt\": %lld,\"ut\": %lld,\"st\": 0",
               counts.count, to_usec(counts.tsc, m_MHz),
               to_usec(counts.vtsc, m_MHz));
      print(buf);

      print("},\n");
    }
    print("]; write_data('ut', false);</script><br><br>&nbsp;<br>");
  }
};

///////////////////////////////////////////////////////////////////////////////
// HierarchicalProfiler

class HierarchicalProfiler : public Profiler {
private:
  class CountMap {
  public:
    CountMap() : count(0), wall_time(0), cpu(0), memory(0), peak_memory(0) {}

    int64 count;
    int64 wall_time;
    int64 cpu;
    int64 memory;
    int64 peak_memory;
  };
  typedef __gnu_cxx::hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:

public:
  HierarchicalProfiler(int flags) : m_flags(flags) {
  }

  virtual void beginFrameEx() {
    m_stack->m_tsc_start = tsc();

    if (m_flags & TrackCPU) {
      m_stack->m_vtsc_start = vtsc(m_MHz);
    }

    if (m_flags & TrackMemory) {
      MemoryManager *mm = MemoryManager::TheMemoryManager().getNoCheck();
      const MemoryUsageStats &stats = mm->getStats(true);
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
    counts.wall_time += tsc() - m_stack->m_tsc_start;

    if (m_flags & TrackCPU) {
      counts.cpu += vtsc(m_MHz) - m_stack->m_vtsc_start;
    }

    if (m_flags & TrackMemory) {
      MemoryManager *mm = MemoryManager::TheMemoryManager().getNoCheck();
      const MemoryUsageStats &stats = mm->getStats(true);
      int64 mu_end = stats.usage;
      int64 pmu_end = stats.peakUsage;
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
  uint32 m_flags;
};

using namespace std;

template <class TraceIt, class Stats>
class walkTraceClass {
public:
  struct Frame {
    TraceIt trace;
    int level;
    int len;
  };
  typedef vector<std::pair<char *, int> >Recursion;
  vector<std::pair<char *, int> >recursion;
  vector<Frame> stack;

  walkTraceClass() : arc_buff_len(200), arc_buff((char*)malloc(200)) {};

  ~walkTraceClass() {
    free((void*)arc_buff);
    if (recursion.size() > 1) {
      Recursion::iterator r_it = recursion.begin();
      while (++r_it != recursion.end()) {
        delete[] r_it->first;
      }
    }
  }

  int arc_buff_len;
  char *arc_buff;

  void checkArcBuff(int len) {
    len = 2*len + HP_STACK_DELIM_LEN + 2;
    if (len >= arc_buff_len) {
      arc_buff_len *= 2;
      arc_buff = (char *)realloc(arc_buff, arc_buff_len);
      if (arc_buff == NULL) {
        throw bad_alloc();
      }
    }
  }

  void incStats(const char *arc, TraceIt tr, const Frame& fr, Stats& stats)
  {
    typename Stats::mapped_type& st = stats[arc];
    ++st.count;
    st.wall_time += tr->wall_time - fr.trace->wall_time;
    st.cpu += tr->cpu - fr.trace->cpu;
    st.memory += tr->memory - fr.trace->memory;
    st.peak_memory += tr->peak_memory - fr.trace->peak_memory;
  }

  void popFrame(TraceIt tIt, std::vector<Frame>& stack, Stats& stats)
  {
    Frame callee = stack.back();
    stack.pop_back();
    const char* arc;
    Frame& caller = stack.back();
    char *cp = arc_buff;
    memcpy(cp, caller.trace->symbol.ptr, caller.len);
    cp += caller.len;
    if (caller.level >= 1) {
      pair<char *, int>& lvl = recursion[caller.level];
      memcpy(cp, lvl.first, lvl.second);
      cp += lvl.second;
    }
    memcpy(cp, HP_STACK_DELIM, HP_STACK_DELIM_LEN);
    cp += HP_STACK_DELIM_LEN;
    memcpy(cp, callee.trace->symbol.ptr, callee.len);
    cp += callee.len;
    if (callee.level >= 1) {
      pair<char *, int>& lvl = recursion[callee.level];
      memcpy(cp, lvl.first, lvl.second);
      cp += lvl.second;
    }
    *cp = 0;
    arc = arc_buff;
    incStats(arc, tIt, callee, stats);
  }

  void walk(TraceIt begin, TraceIt end, Stats& stats,
            map<const char *, unsigned> &functionLevel)
  {
    if (begin == end) {
      return;
    }
    recursion.push_back(make_pair((char *)NULL, 0));
    while (begin != end && !begin->symbol.ptr) {
      ++begin;
    }
    while (begin != end) {
      if (begin->symbol.ptr) {
        unsigned level = ++functionLevel[begin->symbol.ptr];
        if (level >= recursion.size()) {
          char *level_string = new char[8];
          sprintf(level_string, "@%u", level);
          recursion.push_back(make_pair(level_string, strlen(level_string)));
        }
        Frame fr;
        fr.trace = begin;
        fr.level = level - 1;
        fr.len = strlen(begin->symbol.ptr);
        checkArcBuff(fr.len);
        stack.push_back(fr);
      } else if (stack.size() > 1) {
        --functionLevel[stack.back().trace->symbol.ptr];
        popFrame(begin, stack, stats);
      }
      ++begin;
    }
    --begin;
    while (stack.size() > 1) {
      popFrame(begin, stack, stats);
    }
    if (!stack.empty()) {
      incStats(stack.back().trace->symbol.ptr, begin, stack.back(), stats);
    }
  }
};

template<class TraceIt, class Stats, class Fmap>
static void
walkTrace(TraceIt begin, TraceIt end,
          Stats& stats,
          Fmap &map)
{
  walkTraceClass<TraceIt, Stats>walker;
  walker.walk(begin, end, stats, map);
}

struct TraceData {
  int64 wall_time;
  int64 cpu;
  int64 memory;
  int64 peak_memory;

  void clear() {
    wall_time = cpu = memory = peak_memory = 0;
  }
  void set(int64 w, int64 c, int64 m, int64 p) {
    wall_time = w;
    cpu = c;
    memory = m;
    peak_memory = p;
  }
};

struct TraceEntry : TraceData {
  union {
    int64 index;
    const char *ptr;
  }  symbol;

  void rebase(const TraceData& base_vals) {
    wall_time -= base_vals.wall_time;
    cpu -= base_vals.cpu;
    memory -= base_vals.memory;
    peak_memory -= base_vals.peak_memory;
  }
};

///////////////////////////////////////////////////////////////////////////////
// TraceProfiler

static char *xhprof_trace_header = "xhprof trace v0\n%d bytes\n";
static char *xhprof_trace_speed = "%d MHz\n";

class TraceProfiler : public Profiler {
public:

  static TraceEntry *s_trace;
  static int s_n_backing;
  int nTrace;
  bool full;
  int64 maxTraceBuffer;
  int64 overflowCalls;

  bool trace_space_available() {
    // the two slots are reserved for internal use
    return nTrace < s_n_backing - 3;
  }

  bool trace_get_space() {
    bool track_realloc = FALSE;
    if (full) {
      overflowCalls++;
      return false;
    }
    int new_array_size;
    if (s_n_backing == 0) {
      new_array_size = RuntimeOption::ProfilerTraceBuffer;
    } else {
      new_array_size = s_n_backing * RuntimeOption::ProfilerTraceExpansion;
      if (maxTraceBuffer != 0 && new_array_size > maxTraceBuffer) {
        new_array_size = maxTraceBuffer > s_n_backing ?
          maxTraceBuffer : s_n_backing;
      }
      if (new_array_size - nTrace <= 5) {
        // for this operation to succeed, we need room for the entry we're
        // adding, two realloc entries, and two entries to mark the end of
        // the trace.
        full = true;
        collectStats("(trace buffer terminated)", s_trace[nTrace++]);
        return false;
      }
      track_realloc = TRUE;
    }
    {
      DECLARE_THREAD_INFO
      MemoryManager::MaskAlloc masker(info->m_mm);
      TraceEntry *r = (TraceEntry*)realloc((void *)s_trace,
                                           new_array_size * sizeof(TraceEntry));

      if (!r) {
        full = true;
        if (s_trace) {
          collectStats("(trace buffer terminated)", s_trace[nTrace++]);
        }
        return false;
      }
      s_n_backing = new_array_size;
      s_trace = r;
    }
    if (track_realloc) {
      collectStats("(trace buffer realloc)", s_trace[nTrace++]);
      collectStats(NULL, s_trace[nTrace++]);
    }
    return true;
  }

  static pthread_mutex_t s_in_use;

  class CountMap : public TraceData {
  public:
    int64 count;
    CountMap() : count(0)  { clear(); }
  };
  typedef __gnu_cxx::hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

  TraceProfiler(int flags) : Profiler(), nTrace(0),
                             full(false), m_flags(flags) {
    if (pthread_mutex_trylock(&s_in_use)) {
      m_successful = false;
    } else {
      char buf[20];
      sprintf(buf, "%d", RuntimeOption::ProfilerMaxTraceBuffer);
      IniSetting::Bind("profiler.max_trace_buffer", buf,
                       ini_on_update_long, &maxTraceBuffer);
    }
    overflowCalls = 0;
    nTrace = 0;
  }

  ~TraceProfiler() {
    if (m_successful) {
      free(s_trace);
      nTrace = 0;
      s_trace = NULL;
      s_n_backing = 0;
      pthread_mutex_unlock(&s_in_use);
      IniSetting::Unbind("profiler.max_trace_buffer");
    }
  }

  virtual void beginFrame(const char *symbol) {
    doTrace(symbol);
  }

  virtual void endFrame(bool endMain = false) {
    doTrace(NULL);
  }

  virtual void endAllFrames() {
    Profiler::endAllFrames();
    if (s_trace && nTrace < s_n_backing - 1) {
      collectStats(NULL, s_trace[nTrace++]);
      full = true;
    }
  }

  void collectStats(const char *symbol, TraceEntry& te) {
    te.symbol.ptr = symbol;
    collectStats((TraceData&)te);
  }

  void collectStats(TraceData& te) {
    te.wall_time = tsc();
    te.cpu = 0;
    if (m_flags & TrackCPU) {
      te.cpu = vtsc(m_MHz);
    }
    if (m_flags & TrackMemory) {
      MemoryManager *mm = MemoryManager::TheMemoryManager().getNoCheck();
      const MemoryUsageStats &stats = mm->getStats(true);
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

  TraceEntry* next_trace() {
    if (!trace_space_available() && !trace_get_space()) {
      return 0;
    }
    return &s_trace[nTrace++];
  }

  void doTrace(const char *symbol) {
    TraceEntry *te = next_trace();
    if (te != NULL) {
      collectStats(symbol, *te);
    }
  }

  virtual void writeStats(Array &ret) {
    map<const char *, unsigned>fmap;
    TraceData my_begin;
    collectStats(my_begin);
    walkTrace(s_trace, s_trace + nTrace, m_stats, fmap);
    if (overflowCalls) {
      m_stats["(trace buffer terminated)"].count += overflowCalls/2;
    }
    extractStats(ret, m_stats, m_flags, m_MHz);
    if (m_flags & GetTrace) {
      String traceData;
      packTraceData(fmap, traceData, m_MHz);
      ret.set("(compressed_trace)", traceData);
      fprintf(stderr, "%d bytes\n", traceData.size());
    }
    CountMap trace_buffer;
    trace_buffer.count = 0;
    trace_buffer.peak_memory = trace_buffer.memory = nTrace * sizeof(*s_trace);
    returnVals(ret, "(trace buffer alloc)", trace_buffer, m_flags, m_MHz);
    if (m_flags & MeasureXhprofDisable) {
      CountMap my_end;
      collectStats((TraceData&)my_end);
      my_end.count = 1;
      my_end.cpu -= my_begin.cpu;
      my_end.wall_time -= my_begin.wall_time;
      my_end.memory -= my_begin.memory;
      my_end.peak_memory -= my_begin.peak_memory;
      returnVals(ret, "xhprof_post_processing()", my_end, m_flags, m_MHz);
    }
  }

  void extendBuffer(z_stream &strm, char *&buff, int &size) {
    int old_size = size;
    size *= 1.1;
    buff = (char *)realloc(buff, size);
    strm.next_out = (Bytef*)(buff + old_size);
    strm.avail_out = size - old_size;
  }

  void deflater(z_stream &strm, int mode, char *&buff, int &size) {
    while (deflate(&strm, Z_FULL_FLUSH),
           strm.avail_out == 0)
    {
      extendBuffer(strm, buff, size);
    }
  }

  template<class fm>
  void packTraceData(fm &fmap, String& traceData, int MHz) {
    typename fm::iterator fmIt;
    int symbol_len = 0;

    char speed_buff[50];
    sprintf(speed_buff, xhprof_trace_speed, MHz);
    int speed_len = strlen(speed_buff);

    // size the buffer for holding function names
    for (fmIt = fmap.begin(); fmIt != fmap.end(); ++fmIt) {
       // record the length of each name
       fmIt->second = strlen(fmIt->first) + 1;
       symbol_len += fmIt->second;
    }
    ++symbol_len;
    char namebuff[symbol_len];
    char *cp = namebuff;
    int index = 0;

    // copy function names, and give each function an index number
    for (fmIt = fmap.begin(); fmIt != fmap.end(); ++fmIt) {
       int len = fmIt->second;
       memcpy((void *)cp, (void *)fmIt->first, len);
       cp += len;
       // this index will go in the trace
       fmIt->second = index++;
    }
    *cp = '\0';    // an extra null
    // map null strings to -1
    fmap[NULL] = (typename fm::mapped_type)-1;

    TraceEntry *te = s_trace;
    TraceEntry *end = s_trace + nTrace;

    while (te != end) {
      te->symbol.index = fmap[te->symbol.ptr];
      te->rebase(*s_trace);
      ++te;
    }

    // compress, starting with the list of functions
    z_stream strm;
    int ret;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, 3);
    if (ret != Z_OK) {
      return;
    }

    int trace_size = nTrace * sizeof(TraceEntry);

    int dp_size = symbol_len/3 + trace_size/5;
    char *dp_buff = (char*) malloc(dp_size);

    sprintf(dp_buff, xhprof_trace_header, speed_len + symbol_len + trace_size);
    strm.next_out = (Bytef*)strchr(dp_buff, 0);
    strm.avail_out = dp_size - ((char *)strm.next_out - dp_buff);

    strm.next_in = (Bytef*)speed_buff;
    strm.avail_in = speed_len;
    deflater(strm, Z_NO_FLUSH, dp_buff, dp_size);

    strm.next_in = (Bytef*)namebuff;
    strm.avail_in = symbol_len;
    deflater(strm, Z_FULL_FLUSH, dp_buff, dp_size);

    strm.next_in = (Bytef*)s_trace;
    strm.avail_in = nTrace * sizeof(TraceEntry);
    deflater(strm, Z_NO_FLUSH, dp_buff, dp_size);

    while ((ret = deflate(&strm, Z_FINISH)) != Z_STREAM_END)
    {
      extendBuffer(strm, dp_buff, dp_size);
    }
    traceData = String(dp_buff, dp_size - strm.avail_out, AttachString);
    nTrace = 0;
  }

  static String
  unpackTraceData(CStrRef packedTrace) {
    const char *input = packedTrace.c_str();
    int64 input_length = packedTrace.size();

    int output_length;
    char *output = 0;

    if (sscanf(input, xhprof_trace_header, &output_length) != 1) {
      return String("", 0, AttachLiteral);
    }
    const char *zipped_begin;
    if (!(zipped_begin = strchr(input, '\n'))
        || !(zipped_begin = strchr(zipped_begin + 1, '\n'))) {
      goto error2_out;
    }
    ++zipped_begin;

    int ret;
    z_stream strm;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) {
      goto error2_out;
    }

    output = (char *)malloc(output_length);

    strm.avail_in = input_length - (zipped_begin - input);
    strm.next_in = (Bytef*)zipped_begin;
    strm.avail_out = output_length;
    strm.next_out = (Bytef*)output;
    switch (inflate(&strm, Z_NO_FLUSH)) {
    case Z_NEED_DICT:
    case Z_DATA_ERROR:
    case Z_MEM_ERROR:
      goto error_out;
    }
    if (strm.avail_out) {
      goto error_out;
    }
    inflateEnd(&strm);

    return String(output, output_length, AttachString);
  error_out:
    free(output);
    inflateEnd(&strm);
  error2_out:
    return String("", 0, AttachLiteral);
  }


private:
  uint32 m_flags;
};

TraceEntry *TraceProfiler::s_trace = NULL;
int TraceProfiler::s_n_backing = 0;
pthread_mutex_t TraceProfiler::s_in_use = PTHREAD_MUTEX_INITIALIZER;

///////////////////////////////////////////////////////////////////////////////
// SampleProfiler

/**
 * Sampling based profiler.
 */
class SampleProfiler : public Profiler {
private:
  typedef __gnu_cxx::hash_map<std::string, int64, string_hash> CountMap;
  typedef __gnu_cxx::hash_map<std::string, CountMap, string_hash> StatsMap;
  StatsMap m_stats; // outcome

public:
  SampleProfiler() {
    struct timeval  now;
    uint64 truncated_us;
    uint64 truncated_tsc;

    // Init the last_sample in tsc
    m_last_sample_tsc = tsc();

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
  uint64 m_last_sample_tsc;
  uint64 m_sampling_interval_tsc;

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
    snprintf(key, sizeof(key), "%ld.%06ld",
             m_last_sample_time.tv_sec, m_last_sample_time.tv_usec);

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
      while ((tsc() - m_last_sample_tsc) > m_sampling_interval_tsc) {
        m_last_sample_tsc += m_sampling_interval_tsc;
        // HAS TO BE UPDATED BEFORE calling sample_stack
        incr_us_interval(&m_last_sample_time, SAMPLING_INTERVAL);
        sample_stack();
      }
    }
  }
};

///////////////////////////////////////////////////////////////////////////////

class ProfilerFactory : public RequestEventHandler {
public:
  enum Level {
    Simple       = 1,
    Hierarchical = 2,
    Memory       = 3,
    Trace        = 4,
    Sample       = 620002, // Rockfort's zip code
  };

  static bool EnableNetworkProfiler;

public:
  ProfilerFactory() : m_profiler(NULL) {
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
    if (m_profiler == NULL) {
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
      default:
        throw_invalid_argument("level: %d", level);
        return;
      }
      if (m_profiler->m_successful) {
        m_profiler->beginFrame("main()");
        ThreadInfo::s_threadInfo->m_profiler = m_profiler;
      } else {
        delete m_profiler;
        m_profiler = NULL;
      }
    }
  }

  Variant stop() {
    if (m_profiler) {
      m_profiler->endAllFrames();

      Array ret;
      m_profiler->writeStats(ret);
      delete m_profiler;
      m_profiler = NULL;
      ThreadInfo::s_threadInfo->m_profiler = NULL;

      return ret;
    }
    return null;
  }

  /**
   * The whole purpose to make sure "const char *" is safe to take on these
   * strings.
   */
  void cacheString(CStrRef name) {
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
  return null;
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
  return null;
#endif
}

void f_xhprof_frame_begin(CStrRef name) {
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
    prof->endFrame();
  }
#endif
}

void f_xhprof_enable(int flags/* = 0 */,
                     CArrRef args /* = null_array */) {
#ifdef HOTPROFILER
  if (vtsc_syscall.num <= 0) {
    flags &= ~TrackVtsc; }
  if (flags & TrackVtsc) {
    flags |= TrackCPU;
  }
  if (flags & Trace) {
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
  return null;
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
  return null;
#endif
}

Variant f_xhprof_run_trace(CStrRef packedTrace, int flags) {
#ifdef HOTPROFILER

  String traceData = TraceProfiler::unpackTraceData(packedTrace);

  // we really don't want to copy this
  char *syms = const_cast<char*>(traceData.c_str());

  int MHz;
  if (sscanf(syms, xhprof_trace_speed, &MHz) != 1) {
    return null;
  }

  vector<char *>symbols;
  char *sym = strchr(syms, '\n') + 1;
  char *esym = strchr(sym, '\0');
  while (sym != esym) {
    symbols.push_back(sym);
    sym = esym + 1;
    esym = strchr(sym, '\0');
  }

  TraceEntry *begin = (TraceEntry*)(esym + 1);
  TraceEntry *end = (TraceEntry*)(syms + traceData.size());

  for (TraceEntry *toup = begin; toup != end; ++toup) {
    int symIndex = toup->symbol.index;
    if (symIndex >= 0 && (uint64)symIndex < symbols.size()) {
      toup->symbol.ptr = symbols[toup->symbol.index];
    } else if (symIndex == -1) {
      toup->symbol.ptr = NULL;
    } else {
      // corrupt trace
      return null;
    }
  }

  map<const char *, unsigned>fmap;
  Array result;
  TraceProfiler::StatsMap stats;
  walkTrace(&*begin, &*end, stats, fmap);
  Profiler::extractStats(result, stats, flags, MHz);
  return result;
#else
  return null;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// constants
const int64 k_XHPROF_FLAGS_NO_BUILTINS = TrackBuiltins;
const int64 k_XHPROF_FLAGS_CPU = TrackCPU;
const int64 k_XHPROF_FLAGS_MEMORY = TrackMemory;
const int64 k_XHPROF_FLAGS_VTSC = TrackVtsc;
const int64 k_XHPROF_FLAGS_TRACE = Trace;
const int64 k_XHPROF_FLAGS_MEASURE_XHPROF_DISABLE = MeasureXhprofDisable;
const int64 k_XHPROF_FLAGS_GET_TRACE = GetTrace;
const int64 k_XHPROF_FLAGS_MALLOC = TrackMalloc;

///////////////////////////////////////////////////////////////////////////////
// injected code

void begin_profiler_frame(Profiler *p, const char *symbol) {
  p->beginFrame(symbol);
}

void end_profiler_frame(Profiler *p) {
  p->endFrame();
}

///////////////////////////////////////////////////////////////////////////////
}
