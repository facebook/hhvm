/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_TRACE_H_
#define incl_HPHP_TRACE_H_

#include <string>
#include <vector>
#include <stdarg.h>

#include "folly/Format.h"

#include "hphp/util/assertions.h"
#include "hphp/util/portability.h"
#include "hphp/util/text-color.h"

/*
 * Runtime-selectable trace facility. A trace statement has both a module and a
 * level associated with it;  Enable tracing a module by setting the TRACE
 * environment variable to a comma-separated list of module:level pairs. E.g.:
 *
 * env TRACE=mcg:1,bcinterp:3,tmp0:1 ./hhvm/hhvm ...
 *
 * In a source file, select the compilation unit's module by calling the
 * TRACE_SET_MODE macro. E.g.,
 *
 *   TRACE_SET_MOD(mcg);
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
 * this. (Note you can set it to something like /dev/stderr or
 * /dev/stdout if you want the logs printed to your terminal).
 *
 * When printing to the terminal, some traces will know how to use
 * colorization.  You can set HPHP_TRACE_TTY to tell the tracing
 * facility to assume it should colorize even if the output file isn't
 * obviously a tty.
 */

/*
 * Trace levels can be bumped on a per-module, per-scope basis.  This
 * lets you run code that has a set of trace levels in a mode as if
 * they were all higher.
 *
 * Example:
 *
 *   {
 *     Trace::Bump bumper{Trace::mcg, 2};
 *     FTRACE(1, "asd\n");  // only fires at level >= 3
 *   }
 *   FTRACE(1, "asd\n");    // back to normal
 *
 *
 * There is also support for conditionally bumping in the bumper:
 *
 *   {
 *     Trace::Bump bumper{Trace::mcg, 2, somePredicate(foo)};
 *     // Only bumped if somePredicate(foo) returned true.
 *   }
 *
 * Note however that if you use that form, `somePredicate' will be
 * evaluated even if tracing is off.
 */

namespace HPHP {
namespace Trace {

#define TRACE_MODULES \
      TM(tprefix)     /* Meta: prefix with string */  \
      TM(traceAsync)  /* Meta: lazy writes to disk */ \
      TM(asmx64)        \
      TM(atomicvector)  \
      TM(bcinterp)      \
      TM(datablock)     \
      TM(debugger)      \
      TM(debuggerflow)  \
      TM(debuginfo)     \
      TM(dispatchBB)    \
      TM(emitter)       \
      TM(fixup)         \
      TM(fr)            \
      TM(gc)            \
      TM(heap)          \
      TM(hhas)          \
      TM(hhbbc)         \
      TM(hhbbc_dce)     \
      TM(hhbbc_dump)    \
      TM(hhbbc_emit)    \
      TM(hhbbc_index)   \
      TM(hhbbc_time)    \
      TM(hhbc)          \
      TM(hhir)          \
      TM(hhirTracelets) \
      TM(hhir_dce)      \
      TM(hhir_refcount) \
      TM(inlining)      \
      TM(instancebits)  \
      TM(intercept)     \
      TM(interpOne)     \
      TM(jittime)       \
      TM(mcg)           \
      TM(mcgstats)      \
      TM(minstr)        \
      TM(pgo)           \
      TM(printir)       \
      TM(rat)           \
      TM(refcount)      \
      TM(regalloc)      \
      TM(region)        \
      TM(ringbuffer)    \
      TM(runtime)       \
      TM(servicereq)    \
      TM(smartalloc)    \
      TM(stat)          \
      TM(statgroups)    \
      TM(stats)         \
      TM(targetcache)   \
      TM(tcspace)       \
      TM(trans)         \
      TM(treadmill)     \
      TM(txdeps)        \
      TM(txlease)       \
      TM(typeProfile)   \
      TM(unwind)        \
      TM(ustubs)        \
      TM(xenon)         \
      TM(xls)           \
      /* Stress categories, to exercise rare paths */ \
      TM(stress_txInterpPct)  \
      TM(stress_txInterpSeed) \
      /* Jit bisection interval */ \
      TM(txOpBisectLow)   \
      TM(txOpBisectHigh)  \
      /* Temporary categories, to save compilation time */ \
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

//////////////////////////////////////////////////////////////////////

/*
 * S-expression style structured pretty-printing. Implement
 * std::string pretty() const { }, with the convention that
 * nested structures are notated as lisp-style trees:
 *
 *    (<typename> field0 field1)
 *
 * E.g.:
 *    (Location Stack 1)
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

void traceRelease(const char*, ...) ATTRIBUTE_PRINTF(1,2);
void traceRelease(const std::string& s);

template<typename... Args>
void ftraceRelease(Args&&... args) {
  traceRelease("%s", folly::format(std::forward<Args>(args)...).str().c_str());
}

// Trace to the global ring buffer in all builds, and also trace normally
// via the standard TRACE(n, ...) macro.
#define TRACE_RB(n, ...)                            \
  HPHP::Trace::traceRingBufferRelease(__VA_ARGS__); \
  TRACE(n, __VA_ARGS__);
void traceRingBufferRelease(const char* fmt, ...) ATTRIBUTE_PRINTF(1,2);

extern int levels[NumModules];
extern __thread int tl_levels[NumModules];
const char* moduleName(Module mod);
inline bool moduleEnabledRelease(Module tm, int level = 1) {
  return levels[tm] + tl_levels[tm] >= level;
}

// Trace::Bump that is on for release tracing.
struct BumpRelease {
  BumpRelease(Module mod, int adjust, bool condition = true)
    : m_live(condition)
    , m_mod(mod)
    , m_adjust(adjust)
  {
    if (m_live) tl_levels[m_mod] -= m_adjust;
  }

  BumpRelease(BumpRelease&& o) noexcept
    : m_live(o.m_live)
    , m_mod(o.m_mod)
    , m_adjust(o.m_adjust)
  {
    o.m_live = false;
  }

  ~BumpRelease() {
    if (m_live) tl_levels[m_mod] += m_adjust;
  }

  BumpRelease(const BumpRelease&) = delete;
  BumpRelease& operator=(const BumpRelease&) = delete;

private:
  bool m_live;
  Module m_mod;
  int m_adjust;
};

//////////////////////////////////////////////////////////////////////

#if (defined(DEBUG) || defined(USE_TRACE)) /* { */
#  ifndef USE_TRACE
#    define USE_TRACE 1
#  endif

//////////////////////////////////////////////////////////////////////
/*
 * Implementation of for when tracing is enabled.
 */

inline bool moduleEnabled(Module tm, int level = 1) {
  return moduleEnabledRelease(tm, level);
}

inline int moduleLevel(Module tm) { return levels[tm]; }

#define HPHP_TRACE

const bool enabled = true;

#define ONTRACE_MOD(module, n, x) do {    \
  if (HPHP::Trace::moduleEnabled(module, n)) {  \
    x;                                          \
  } } while(0)

#define ONTRACE(n, x) ONTRACE_MOD(TRACEMOD, n, x)

#define TRACE(n, ...) ONTRACE(n, HPHP::Trace::trace(__VA_ARGS__))
#define FTRACE(n, ...)                                        \
  ONTRACE(n, HPHP::Trace::trace("%s",                         \
             folly::format(__VA_ARGS__).str().c_str()))
#define TRACE_MOD(mod, level, ...) \
  ONTRACE_MOD(mod, level, HPHP::Trace::trace(__VA_ARGS__))
#define FTRACE_MOD(mod, level, ...)                     \
  ONTRACE_MOD(mod, level, HPHP::Trace::trace("%s",      \
             folly::format(__VA_ARGS__).str().c_str()))
#define TRACE_SET_MOD(name)  \
  UNUSED static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::name;

/*
 * The Indent struct and ITRACE are used for tracing with nested
 * indentation. Create an Indent object on the stack to increase the nesting
 * level, then use ITRACE just as you would use FTRACE.
 */
extern __thread int indentDepth;
struct Indent {
  explicit Indent(int n = 2) : n(n) { indentDepth += n; }
  ~Indent()                         { indentDepth -= n; }

  int n;
};

// See doc comment above for usage.
using Bump = BumpRelease;

inline std::string indent() {
  return std::string(indentDepth, ' ');
}

template<typename... Args>
inline void itraceImpl(const char* fmtRaw, Args&&... args) {
  auto const fmt = indent() + fmtRaw;
  Trace::ftraceRelease(fmt, std::forward<Args>(args)...);
}
#define ITRACE(level, ...) ONTRACE((level), Trace::itraceImpl(__VA_ARGS__));
#define ITRACE_MOD(mod, level, ...)                             \
  ONTRACE_MOD(mod, level, Trace::itraceImpl(__VA_ARGS__));

void trace(const char *, ...) ATTRIBUTE_PRINTF(1,2);
void trace(const std::string&);

template<typename Pretty>
inline void trace(Pretty p) { trace(p.pretty() + std::string("\n")); }

void vtrace(const char *fmt, va_list args) ATTRIBUTE_PRINTF(1,0);
void dumpRingbuffer();

//////////////////////////////////////////////////////////////////////

#else /* } (defined(DEBUG) || defined(USE_TRACE)) { */

//////////////////////////////////////////////////////////////////////
/*
 * Implementation for when tracing is disabled.
 */

#define ONTRACE(...)    do { } while (0)
#define TRACE(...)      do { } while (0)
#define FTRACE(...)     do { } while (0)
#define TRACE_MOD(...)  do { } while (0)
#define FTRACE_MOD(...) do { } while (0)
#define TRACE_SET_MOD(name) \
  DEBUG_ONLY static const HPHP::Trace::Module TRACEMOD = HPHP::Trace::name;

#define ITRACE(...)     do { } while (0)
struct Indent {
  Indent() {
    always_assert(true && "If this struct is completely empty we get unused "
                  "variable warnings in code that uses it.");
  }
};
inline std::string indent() { return std::string(); }

struct Bump {
  Bump(Module mod, int adjust, bool condition = true) {
    always_assert(true && "If this struct is completely empty we get unused "
                  "variable warnings in code that uses it.");
  }
};

const bool enabled = false;

inline void trace(const char*, ...)      { }
inline void trace(const std::string&)    { }
inline void vtrace(const char*, va_list) { }
inline bool moduleEnabled(Module t, int level = 1) { return false; }
inline int moduleLevel(Module tm) { return 0; }

//////////////////////////////////////////////////////////////////////

#endif /* } (defined(DEBUG) || defined(USE_TRACE)) */

} // Trace

// Optional color utility for trace dumps; when output is a tty or
// when we've been told to assume it is.
inline const char* color(const char* color) {
  static auto const shouldColorize = []() -> bool {
    auto const traceEnv  = getenv("HPHP_TRACE_FILE");
    auto const assumeTTY = getenv("HPHP_TRACE_TTY");
    if (assumeTTY) return true;
    if (!traceEnv) return false;
    return
      !strcmp(traceEnv, "/dev/stdout") ? isatty(1) :
      !strcmp(traceEnv, "/dev/stderr") ? isatty(2) :
      false;
  }();
  return shouldColorize ? color : "";
}

inline std::string color(const char* fg, const char* bg) {
  auto const s = add_bgcolor(fg, bg);
  return color(s.c_str());
}

//////////////////////////////////////////////////////////////////////

FOLLY_CREATE_HAS_MEMBER_FN_TRAITS(has_toString, toString);

} // HPHP

namespace folly {
template<typename Val>
struct FormatValue<Val,
                   typename std::enable_if<
                     HPHP::has_toString<Val, std::string() const>::value,
                     void
                   >::type> {
  explicit FormatValue(const Val& val) : m_val(val) {}

  template<typename Callback> void format(FormatArg& arg, Callback& cb) const {
    format_value::formatString(m_val.toString(), arg, cb);
  }

 private:
  const Val& m_val;
};
}

#endif /* incl_HPHP_TRACE_H_ */
