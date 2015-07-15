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

#ifndef incl_HPHP_EXT_HOTPROFILER_H_
#define incl_HPHP_EXT_HOTPROFILER_H_

#include "hphp/runtime/base/request-local.h"

#ifdef __FreeBSD__
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

#elif (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
#include <windows.h>
typedef DWORD_PTR cpu_set_t;

#define CPU_SET(cpu_id, new_mask) (*(new_mask)) = (cpu_id + 1)
#define CPU_ZERO(new_mask) (*(new_mask)) = 0
#define SET_AFFINITY(pid, size, mask) \
         SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)mask)
#define GET_AFFINITY(pid, size, mask) DWORD_PTR s_mask; \
         GetProcessAffinityMask(GetCurrentProcess(), mask, &s_mask)

#else
#include <sched.h>
#define SET_AFFINITY(pid, size, mask) sched_setaffinity(0, size, mask)
#define GET_AFFINITY(pid, size, mask) sched_getaffinity(0, size, mask)
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// classes

/**
 * Information every Profiler collects about a frame.
 * Subclasses of Profiler may subclass this as well,
 * in case they need additional information,
 * and then override allocateFrame().
 */
class Frame {
public:
  Frame          *m_parent;        // pointer to parent frame
  const char     *m_name;          // function name
  uint8_t         m_hash_code;     // hash_code for the function name
  int             m_recursion;     // recursion level for function

  virtual ~Frame() {
  }

  /**
   * Returns formatted function name
   *
   * @param  result_buf   ptr to result buf
   * @param  result_len   max size of result buf
   * @return total size of the function name returned in result_buf
   * @author veeve
   */
  size_t getName(char *result_buf, size_t result_len);

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
  size_t getStack(int level, char *result_buf, size_t result_len);
};

enum Flag {
  NoTrackBuiltins       = 0x1,
  TrackCPU              = 0x2,
  TrackMemory           = 0x4,
  TrackVtsc             = 0x8,
  XhpTrace              = 0x10,
  MeasureXhprofDisable  = 0x20,
  Unused                = 0x40,
  TrackMalloc           = 0x80,

  // Allows profiling of multiple threads at the same time with TraceProfiler.
  // Requires a lot of memory.
  IHaveInfiniteMemory   = 0x100,

  // A very slow profiler of function arguments and results
  // to look for memoization opportunities.
  Memo                  = 0x200,

  // A hot profiler supplied by a call to HotProfiler::setExternalProfiler.
  External              = 0x400,
};

/**
 * Maintain profiles of a running stack.
 */
struct Profiler {
  explicit Profiler(bool needCPUAffinity);
  virtual ~Profiler();

  /**
   * Subclass can do extra work by overriding these two virtual functions.
   */

  /**
   * Called right before a function call.
   */
  virtual void beginFrameEx(const char *symbol);

  /**
   * Called right after a function is finished.
   */
  virtual void endFrameEx(const TypedValue *retval, const char *symbol);

  /**
   * Final results.
   */
  virtual void writeStats(Array &ret);

  /**
   * Start a new frame with the specified symbol.
   */
  virtual void beginFrame(const char *symbol) __attribute__ ((__noinline__)) ;

  /**
   * End top of the stack.
   */
  virtual void endFrame(const TypedValue *retval,
                        const char *symbol,
                        bool endMain = false) __attribute__ ((__noinline__)) ;

  virtual void endAllFrames();

  virtual bool shouldSkipBuiltins() const { return false; }

  template<class phpret, class Name, class Counts>
  static void returnVals(phpret& ret, const Name& name, const Counts& counts,
                  int flags, int64_t MHz);

  template<class phpret, class StatsMap>
  static bool extractStats(phpret& ret, StatsMap& stats, int flags,
                           int64_t MHz);

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
    uint32_t i = 0;
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
   * Fast allocate a Frame structure. Picks one from the
   * free list if available, else calls allocateFrame
   * for an actual allocate.
   */
  Frame *createFrame(const char *symbol) {
    Frame *p = m_frame_free_list;
    if (p) {
      m_frame_free_list = p->m_parent;
    } else {
      p = allocateFrame ();
    }
    p->m_parent = m_stack;
    p->m_name = symbol;
    p->m_hash_code = hprof_inline_hash(symbol);
    m_stack = p;
    return p;
  }

  /**
   * Allocate a Frame structure suitable for this class' needs.
   */
  virtual Frame *allocateFrame() {
    return new Frame();
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

public:
  bool m_successful;

  int64_t    m_MHz; // cpu freq for either the local cpu or the saved trace
  Frame    *m_stack;      // top of the profile stack

  static bool s_rand_initialized;

  cpu_set_t m_prev_mask;               // saved cpu affinity
  Frame    *m_frame_free_list;         // freelist of Frame
  uint8_t     m_func_hash_counters[256]; // counter table by hash code;
private:
  bool m_has_affinity;
};

///////////////////////////////////////////////////////////////////////////////
enum class ProfilerKind {
  Hierarchical = 1,
  Memory       = 2,
  Trace        = 3,
  Memo         = 4,
  XDebug       = 5,
  External     = 6,
  Sample       = 620002, // Rockfort's zip code
};

struct ProfilerFactory final : RequestEventHandler {
  static bool EnableNetworkProfiler;

  ProfilerFactory() : m_profiler(nullptr), m_external_profiler(nullptr) {
  }

  ~ProfilerFactory() {
    stop();
  }

  Profiler *getProfiler() {
    return m_profiler;
  }

  void requestInit() override {}
  void requestShutdown() override {
    stop();
    m_artificialFrameNames.reset();
  }

  /**
   * Attempts to start profiling in the current thread. Returns true on success
   * or false on failure. If beginFrame is true, Profiler::beginFrame is called
   * with "main()" as the symbol name.
   */
  bool start(ProfilerKind kind, long flags, bool beginFrame = true);

  /**
   * Will stop profiling if currently profiling, regardless of how it was
   * started.  The Variant returned contains profile information.
   * Some consumers of the return value may json_encode and then var_dump
   * the returned value, and may choose to skip that step if the return value
   * is a null Variant.
   */
  Variant stop();

  /**
   * The whole purpose to make sure "const char *" is safe to take on these
   * strings.
   */
  void cacheString(const String& name) {
    m_artificialFrameNames.append(name);
  }

  /**
   * Registers a Profiler to use when ProfilerKind::External is used.
   */
  void setExternalProfiler(Profiler *p) {
    m_external_profiler = p;
  }
  Profiler *getExternalProfiler() {
    return m_external_profiler;
  }

private:
  Profiler *m_profiler;
  Profiler *m_external_profiler;
  Array m_artificialFrameNames;
};

DECLARE_EXTERN_REQUEST_LOCAL(ProfilerFactory, s_profiler_factory);

}

#endif // incl_HPHP_EXT_HOTPROFILER_H_
