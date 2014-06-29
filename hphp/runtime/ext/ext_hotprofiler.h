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

#include "hphp/runtime/ext/ext_fb.h"

namespace HPHP {

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
  TrackBuiltins         = 0x1,
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
};

/**
 * Maintain profiles of a running stack.
 */
class Profiler {
public:
  Profiler();
  virtual ~Profiler();

  /**
   * Subclass can do extra work by overriding these two virtual functions.
   */
  virtual void beginFrameEx(); // called right before a function call
  virtual void endFrameEx();   // called right after a function is finished

  /**
   * Final results.
   */
  virtual void writeStats(Array &ret);

  /**
   * Start a new frame with the specified symbol.
   */
  virtual void beginFrame(const char *symbol) __attribute__ ((noinline)) ;

  /**
   * End top of the stack.
   */
  virtual void endFrame(const char *symbol,
                        bool endMain = false) __attribute__ ((noinline)) ;

  virtual void endAllFrames();

  template<class phpret, class Name, class Counts>
  static void returnVals(phpret& ret, const Name& name, const Counts& counts,
                  int flags, int64_t MHz);

  template<class phpret, class StatsMap>
  static bool extractStats(phpret& ret, StatsMap& stats, int flags,
                           int64_t MHz);

  bool m_successful;

  int64_t    m_MHz; // cpu freq for either the local cpu or the saved trace
  Frame    *m_stack;      // top of the profile stack

  static bool s_rand_initialized;

  cpu_set_t m_prev_mask;               // saved cpu affinity
  Frame    *m_frame_free_list;         // freelist of Frame
  uint8_t     m_func_hash_counters[256]; // counter table by hash code;

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

}

#endif // incl_HPHP_EXT_HOTPROFILER_H_
