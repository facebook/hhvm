/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <sched.h>

// Append the delimiter
#define HP_STACK_DELIM        "==>"
#define HP_STACK_DELIM_LEN    (sizeof(HP_STACK_DELIM) - 1)

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(hotprofiler);
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
  ulong h = 5381;
  uint i = 0;
  uint8 res = 0;

  while (*str) {
    h += (h << 5);
    h ^= (ulong) *str++;
  }

  for (i = 0; i < sizeof(ulong); i++) {
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
inline uint64 cycle_timer() {
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
 * @return double.
 * @author cjiang
 */
static double get_cpu_frequency() {
  struct timeval start;
  struct timeval end;

  if (gettimeofday(&start, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64 tsc_start = cycle_timer();
  // Sleep for 5 miliseconds. Comparaing with gettimeofday's  few microseconds
  // execution time, this should be enough.
  usleep(5000);
  if (gettimeofday(&end, 0)) {
    perror("gettimeofday");
    return 0.0;
  }
  uint64 tsc_end = cycle_timer();
  return (tsc_end - tsc_start) * 1.0 / (get_us_interval(&start, &end));
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
  static void BindToCPU(uint32 cpu_id) {
    cpu_set_t new_mask;
    CPU_ZERO(&new_mask);
    CPU_SET(cpu_id, &new_mask);
    sched_setaffinity(0, sizeof(cpu_set_t), &new_mask);
  }

public:
  // This array is used to store cpu frequencies for all available logical
  // cpus. For now, we assume the cpu frequencies will not change for power
  // saving or other reasons. If we need to worry about that in the future, we
  // can use a periodical timer to re-calculate this arrary every once in a
  // while (for example, every 1 or 5 seconds).
  double *m_cpu_frequencies;

  // The number of logical CPUs this machine has.
  int m_cpu_num;

  MachineInfo() {
    m_cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    m_cpu_frequencies = (double*)malloc(sizeof(double) * m_cpu_num);

    cpu_set_t prev_mask;
    sched_getaffinity(0, sizeof(cpu_set_t), &prev_mask);
    for (int id = 0; id < m_cpu_num; ++id) {
      BindToCPU(id);

      // Make sure the current process gets scheduled to the target cpu. This
      // might not be necessary though.
      usleep(0);

      m_cpu_frequencies[id] = get_cpu_frequency();
    }
    sched_setaffinity(0, sizeof(cpu_set_t), &prev_mask);
  }

  ~MachineInfo() {
    free(m_cpu_frequencies);
  }
};
static MachineInfo s_machine;

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
  struct rusage   m_ru_start;    // user/sys time start

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

/**
 * Maintain profiles of a running stack.
 */
class Profiler {
public:
  Profiler() : m_stack(NULL), m_frame_free_list(NULL) {
    if (!s_rand_initialized) {
      s_rand_initialized = true;
      srand(GENERATE_SEED());
    }

    // bind to a random cpu so that we can use rdtsc instruction.
    m_cur_cpu_id = rand() % s_machine.m_cpu_num;
    sched_getaffinity(0, sizeof(cpu_set_t), &m_prev_mask);
    MachineInfo::BindToCPU(m_cur_cpu_id);

    memset(m_func_hash_counters, 0, sizeof(m_func_hash_counters));
  }

  virtual ~Profiler() {
    sched_setaffinity(0, sizeof(cpu_set_t), &m_prev_mask);

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
  void beginFrame(const char *symbol) __attribute__ ((noinline)) {
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
  void endFrame(bool endMain = false) __attribute__ ((noinline)) {
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

  void endAllFrames() {
    while (m_stack) {
      endFrame(true);
    }
  }

protected:
  uint32    m_cur_cpu_id; // cpu id current process is bound to
  Frame    *m_stack;      // top of the profile stack

private:
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

///////////////////////////////////////////////////////////////////////////////
// SimpleProfiler

/**
 * getrusage() based profiler, but simple enough to print basic information.
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
 * Above is a nice paper talking about the overhead and the inaccucy problem
 * asscociated with getrusage.
 */
class SimpleProfiler : public Profiler {
private:
  class CountMap {
  public:
    CountMap() : count(0), wall_time(0), user_time(0), system_time(0) {}

    int64 count;
    int64 wall_time;
    int64 user_time;
    int64 system_time;
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
    m_stack->m_tsc_start = cycle_timer();
    getrusage(RUSAGE_SELF, &m_stack->m_ru_start);
  }

  virtual void endFrameEx() {
    struct rusage ru_end;
    getrusage(RUSAGE_SELF, &ru_end);

    CountMap &counts = m_stats[m_stack->m_name];
    counts.count++;
    counts.wall_time +=
      (int64)((double)(cycle_timer() - m_stack->m_tsc_start)/
              s_machine.m_cpu_frequencies[m_cur_cpu_id]);
    counts.user_time +=
      get_us_interval(&m_stack->m_ru_start.ru_utime, &ru_end.ru_utime);
    counts.system_time +=
      get_us_interval(&m_stack->m_ru_start.ru_stime, &ru_end.ru_stime);
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
               ",\"ct\": %lld,\"wt\": %lld,\"ut\": %lld,\"st\": %lld",
               counts.count, counts.wall_time, counts.user_time,
               counts.system_time);
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
  enum Flag {
    TrackBuiltins = 1,
    TrackCPU      = 2,
    TrackMemory   = 4,
  };

public:
  HierarchicalProfiler(int flags) : m_flags(flags) {
  }

  virtual void beginFrameEx() {
    m_stack->m_tsc_start = cycle_timer();

    if (m_flags & TrackCPU) {
      getrusage(RUSAGE_SELF, &m_stack->m_ru_start);
    }

    if (m_flags & TrackMemory) {
      MemoryManager *mm = MemoryManager::TheMemoryManager().get();
      const MemoryUsageStats &stats = mm->getStats();
      m_stack->m_mu_start  = stats.usage;
      m_stack->m_pmu_start = stats.peakUsage;
    }
  }

  virtual void endFrameEx() {
    char symbol[512];
    m_stack->getStack(2, symbol, sizeof(symbol));
    CountMap &counts = m_stats[symbol];
    counts.count++;
    counts.wall_time +=
      (int64)((double)(cycle_timer() - m_stack->m_tsc_start)/
              s_machine.m_cpu_frequencies[m_cur_cpu_id]);

    if (m_flags & TrackCPU) {
      struct rusage ru_end;
      getrusage(RUSAGE_SELF, &ru_end);
      counts.cpu +=
        get_us_interval(&m_stack->m_ru_start.ru_utime, &ru_end.ru_utime) +
        get_us_interval(&m_stack->m_ru_start.ru_stime, &ru_end.ru_stime);
    }

    if (m_flags & TrackMemory) {
      MemoryManager *mm = MemoryManager::TheMemoryManager().get();
      const MemoryUsageStats &stats = mm->getStats();
      int64 mu_end = stats.usage;
      int64 pmu_end = stats.peakUsage;
      counts.memory += mu_end - m_stack->m_mu_start;
      counts.peak_memory += pmu_end - m_stack->m_pmu_start;
    }
  }

  virtual void writeStats(Array &ret) {
    for (StatsMap::const_iterator iter = m_stats.begin();
         iter != m_stats.end(); ++iter) {
      const CountMap &counts = iter->second;
      Array arr;
      arr.set("ct",  counts.count);
      arr.set("wt",  counts.wall_time);
      if (m_flags & TrackCPU) {
        arr.set("cpu", counts.cpu);
      }
      if (m_flags & TrackMemory) {
        arr.set("mu",  counts.memory);
        arr.set("pmu", counts.peak_memory);
      }
      ret.set(String(iter->first), arr);
    }
  }

private:
  uint32 m_flags;
};

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
    double cpu_freq = s_machine.m_cpu_frequencies[m_cur_cpu_id];

    // Init the last_sample in tsc
    m_last_sample_tsc = cycle_timer();

    // Find the microseconds that need to be truncated
    gettimeofday(&m_last_sample_time, 0);
    now = m_last_sample_time;
    hp_trunc_time(&m_last_sample_time, SAMPLING_INTERVAL);

    // Subtract truncated time from last_sample_tsc
    truncated_us  = get_us_interval(&m_last_sample_time, &now);
    truncated_tsc = (int64)((double)truncated_us * cpu_freq);
    if (m_last_sample_tsc > truncated_tsc) {
      // just to be safe while subtracting unsigned ints
      m_last_sample_tsc -= truncated_tsc;
    }

    // Convert sampling interval to ticks
    m_sampling_interval_tsc = (int64)((double)SAMPLING_INTERVAL * cpu_freq);
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
      while ((cycle_timer() - m_last_sample_tsc) > m_sampling_interval_tsc) {
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
    Sample       = 620002, // Rockfort's zip code
  };

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
  }

  void start(Level level, long flags) {
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
      default:
        throw_invalid_argument("level: %d", level);
        return;
      }
      m_profiler->beginFrame("main()");

      ThreadInfo::s_threadInfo->m_profiler = m_profiler;
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

private:
  Profiler *m_profiler;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(ProfilerFactory, s_factory);

///////////////////////////////////////////////////////////////////////////////
// main functions

void f_hotprofiler_enable(int level) {
#ifdef HOTPROFILER
  long flags = 0;
  if (level == ProfilerFactory::Hierarchical) {
    flags = HierarchicalProfiler::TrackBuiltins;
  } else if (level == ProfilerFactory::Memory) {
    level = ProfilerFactory::Hierarchical;
    flags = HierarchicalProfiler::TrackBuiltins |
      HierarchicalProfiler::TrackMemory;
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

void f_xhprof_enable(int flags/* = 0 */,
                     CArrRef args /* = null_array */) {
#ifdef HOTPROFILER
  s_factory->start(ProfilerFactory::Hierarchical, flags);
#endif
}

Variant f_xhprof_disable() {
#ifdef HOTPROFILER
  return s_factory->stop();
#else
  return null;
#endif
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

///////////////////////////////////////////////////////////////////////////////
// injected code

ProfilerInjection::ProfilerInjection(ThreadInfo *info, const char *symbol)
  : m_info(info) {
  Profiler *profiler = m_info->m_profiler;
  if (profiler) {
    profiler->beginFrame(symbol);
  }
}

ProfilerInjection::~ProfilerInjection() {
  Profiler *profiler = m_info->m_profiler;
  if (profiler) {
    profiler->endFrame();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
