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

#ifndef incl_TRACE_H_
#define incl_TRACE_H_

#include <string>
#include <vector>
#include <stdarg.h>

/*
 * Runtime-selectable trace facility. A trace statement has both a module and a
 * level associated with it;  Enable tracing a module by setting the TRACE
 * environment variable to a comma-separated list of module:level pairs. E.g.:
 *
 * env TRACE=tx64:1,bcinterp:3,tmp0:1 ./hhvm/hhvm ...
 *
 * In a source file, select the compilation unit's module by calling the
 * TRACE_SET_MODE macro. E.g.,
 *
 *   TRACE_SET_MOD(tx64);
 *
 *   ...
 *   TRACE(0, "See this for any trace-enabled build: %d\n", foo);
 *   TRACE(1, "Trace-level must be 1 or higher for this one\n");
 *
 * While the levels are arbitrary integers, code so far is following a
 * rough convention of 1-5, where 1 is minimal and 5 is quite verbose.
 *
 * Normally trace information is printed to /tmp/hphp.log. You can
 * override the environment variable HPHP_TRACE_FILE to change
 * this. (Note you can set it to something like /dev/stderr if you
 * want the logs printed to your terminal).
 */

namespace HPHP {
namespace Trace {

#define TRACE_MODULES \
      TM(tprefix)     /* Meta: prefix with string */          \
      TM(ringbuffer)  /* Meta: trace to ram */                \
      TM(traceAsync)  /* Meta: lazy writes to disk */ \
      TM(trans)       \
      TM(tx64)        \
      TM(tx64stats)   \
      TM(tunwind)     \
      TM(txlease)     \
      TM(fixup)       \
      TM(tcspace)     \
      TM(targetcache) \
      TM(tcdump)      \
      TM(treadmill)   \
      TM(regalloc)    \
      TM(bcinterp)    \
      TM(refcount)    \
      TM(asmx64)      \
      TM(runtime)     \
      TM(debuginfo)   \
      TM(stats)       \
      TM(emitter)     \
      TM(hhbc)        \
      TM(stat)        \
      TM(fr)          \
      TM(intercept)   \
      TM(txdeps)      \
      TM(typeProfile) \
      TM(hhir)        \
      TM(gc)          \
      TM(unlikely)    \
      TM(jcc)         \
      TM(instancebits)\
      TM(hhas)        \
      /* Stress categories, to exercise rare paths */ \
      TM(stress_txInterpPct)    \
      TM(stress_txInterpSeed)   \
      /* Jit bisection interval */ \
      TM(txOpBisectLow) \
      TM(txOpBisectHigh) \
      /* Temporary catetories, to save compilation time */ \
      TM(tmp0)  TM(tmp1)  TM(tmp2)  TM(tmp3)               \
      TM(tmp4)  TM(tmp5)  TM(tmp6)  TM(tmp7)               \
      TM(tmp8)  TM(tmp9)  TM(tmp10) TM(tmp11)              \
      TM(tmp12) TM(tmp13) TM(tmp14) TM(tmp15)

enum Module {
#define TM(x) \
  x,
  TRACE_MODULES
#undef TM
  NumModules
};


/*
 * S-expression style structured pretty-printing. Implement
 * std::string pretty() const { }, with the convention that
 * nested structures are notated as lisp-style trees:
 *
 *    (<typename> field0 field1)
 *
 * E.g.:
 *    (Location Stack 1)
 *    (RuntimeType (Location Stack 1) (Home (Location Local 1)))
 *
 * The repetitve prettyNode() templates are intended to aid
 * implementing pretty().
 */

template<typename P1>
std::string prettyNode(const char* name, const std::vector<P1>& vec) {
  using std::string;
  std::string retval = string("(") + string(name) + string(" ");
  for(size_t i = 0; i < vec.size(); i++) {
    retval += vec[i].pretty();
    if (i != vec.size() - 1) {
      retval += string(" ");
    }
  }
  return retval + string(")");
}

template<typename P1>
std::string prettyNode(const char* name, const P1& p1) {
  using std::string;
  return string("(") + string(name) + string(" ") +
    p1.pretty() +
    string(")");
}

template<> std::string prettyNode(const char* name, const std::string& s);

template<typename P1, typename P2>
std::string prettyNode(const char* name, const P1& p1, const P2& p2) {
  using std::string;
  return string("(") + string(name) + string(" ") +
    p1.pretty() + string(" ") + p2.pretty() +
    string(")");
}

extern void traceRelease(const char*, ...);
extern int levels[NumModules];
extern const char* moduleName(Module mod);
static inline bool moduleEnabledRelease(Module tm, int level = 1) {
  return levels[tm] >= level;
}

#if (defined(DEBUG) || defined(USE_TRACE)) /* { */
#  ifndef USE_TRACE
#    define USE_TRACE 1
#  endif
static inline bool moduleEnabled(Module tm, int level = 1) {
  return moduleEnabledRelease(tm, level);
}

static inline int moduleLevel(Module tm) { return levels[tm]; }

#define HPHP_TRACE

static const bool enabled = true;

#define ONTRACE_MOD(module, n, x) do {    \
  if (HPHP::Trace::moduleEnabled(module, n)) {  \
    x;                                          \
  } } while(0)

#define ONTRACE(n, x) ONTRACE_MOD(TRACEMOD, n, x)

#define TRACE(n, ...) ONTRACE(n, HPHP::Trace::trace(__VA_ARGS__))
#define TRACE_MOD(mod, level, ...) \
  ONTRACE_MOD(mod, level, HPHP::Trace::trace(__VA_ARGS__))
#define TRACE_SET_MOD(name)  \
  static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::name;

extern void trace(const char *, ...)
  __attribute__((format(printf,1,2)));
extern void trace(const std::string&);

template<typename Pretty>
static inline void trace(Pretty p) { trace(p.pretty() + std::string("\n")); }

extern void vtrace(const char *fmt, va_list args);
extern void dumpRingbuffer();
#else /* } (defined(DEBUG) || defined(USE_TRACE)) { */
/*
 * Compile everything out of release builds. gcc is smart enough to
 * kill code hiding behind if (false) { ... }.
 */
#define ONTRACE(...)   do { } while(0)
#define TRACE(...)     do { } while(0)
#define TRACE_MOD(...) do { } while(0)
#define TRACE_SET_MOD(...) /* nil */
static const bool enabled = false;

static inline void trace(const char*, ...)      { }
static inline void trace(const std::string&)    { }
static inline void vtrace(const char*, va_list) { }
static inline bool moduleEnabled(Module t, int level = 1) { return false; }
static inline int moduleLevel(Module tm) { return 0; }
#endif /* } (defined(DEBUG) || defined(USE_TRACE)) */

} } // HPHP::Trace
#endif /* incl_TRACE_H_ */

