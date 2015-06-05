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

/*
 * Forcibly define USE_TRACE, so we get the debug trace.h interface included
 * here. This allows mixed compilation, where some units were compiled
 * DEBUG and others compiled RELEASE, to successfully link.
 */
#ifndef USE_TRACE
#  define USE_TRACE 1
#endif
#include "hphp/util/trace.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>

#include "hphp/util/ringbuffer.h"

namespace HPHP {

TRACE_SET_MOD(tprefix);

namespace Trace {

int levels[NumModules];
__thread int tl_levels[NumModules];
__thread int indentDepth = 0;

static FILE* out;

static const char *tokNames[] = {
#define TM(x) #x,
  TRACE_MODULES
#undef TM
};

namespace {

/*
 * Dummy class to get some code to run before main().
 */
class Init {
  Module name2mod(folly::StringPiece name) {
    for (int i = 0; i < NumModules; i++) {
      if (name == tokNames[i]) {
        return (Module)i;
      }
    }
    return (Module)-1;
  }

  public:
  Init() {
    /* Parse the environment for flags. */
    const char *envName = "TRACE";
    const char *env = getenv(envName);
    const char *file = getenv("HPHP_TRACE_FILE");
    if (!file) file = "/tmp/hphp.log";
    if (env) {
      out = fopen(file, "w");
      if (!out) {
        fprintf(stderr, "could not create log file (%s); using stderr\n", file);
        out = stderr;
      }

      std::string envStr(env);
      std::vector<folly::StringPiece> pieces;
      folly::split(",", envStr, pieces);
      for (auto piece : pieces) {
        folly::StringPiece moduleName;
        int level;
        try {
          if (!folly::split(":", piece, moduleName, level)) {
            moduleName = piece;
            level = 1;
          }
        } catch (const std::exception& re) {
          std::cerr <<
            folly::format("Ignoring invalid TRACE component: {}\n", piece);
          continue;
        }

        int mod = name2mod(moduleName);
        if (mod >= 0) levels[mod] = level;

        static auto const groups = {
          Trace::minstr,
          Trace::interpOne,
          Trace::dispatchBB,
          Trace::decreftype,
        };
        for (auto g : groups) {
          if (mod == g) {
            levels[Trace::statgroups] = std::max(levels[Trace::statgroups], 1);
            break;
          }
        }
      }
    } else {
      // If TRACE env var is not set, nothing should be traced...
      // but if it does, use stderr.
      out = stderr;
    }
  }
};

Init i;

}

const char* moduleName(Module mod) {
  return tokNames[mod];
}

void flush() {
  if (!moduleEnabledRelease(Trace::traceAsync)) {
    fflush(out);
  }
}

void vtrace(const char *fmt, va_list ap) {
  static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
  static bool hphp_trace_ringbuffer = getenv("HPHP_TRACE_RINGBUFFER");
  if (hphp_trace_ringbuffer) {
    vtraceRingbuffer(fmt, ap);
  } else {
    ONTRACE(1, pthread_mutex_lock(&mtx));
    ONTRACE(1, fprintf(out, "t%#08x: ",
      int((int64_t)pthread_self() & 0xFFFFFFFF)));
    vfprintf(out, fmt, ap);
    ONTRACE(1, pthread_mutex_unlock(&mtx));
    flush();
  }
}

void trace(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vtrace(fmt, ap);
  va_end(ap);
}

void traceRelease(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vtrace(fmt, ap);
  va_end(ap);
}

void traceRingBufferRelease(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vtraceRingbuffer(fmt, ap);
  va_end(ap);
}

void trace(const std::string& s) {
  trace("%s", s.c_str());
}

void traceRelease(const std::string& s) {
  traceRelease("%s", s.c_str());
}

template<>
std::string prettyNode(const char* name, const std::string& s) {
  using std::string;
  return string("(") + string(name) + string(" ") +
    s +
    string(")");
}

} } // HPHP::Trace
