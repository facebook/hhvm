/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/timer.h"
#include <map>

#include "folly/Format.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"
#include "hphp/util/vdso.h"

TRACE_SET_MOD(jittime);

namespace HPHP { namespace JIT {

static __thread Timer::Counter s_counters[Timer::kNumTimers];

static const struct { const char* str; Timer::Name name; } s_names[] = {
# define TIMER_NAME(name) {#name, Timer::name},
  JIT_TIMERS
# undef TIMER_NAME
};

static int64_t getCPUTimeNanos() {
#ifdef CLOCK_THREAD_CPUTIME_ID
  auto const ns = Vdso::ClockGetTimeNS(CLOCK_THREAD_CPUTIME_ID);
  if (ns != -1) return ns;
#endif

#ifdef RUSAGE_THREAD
  return HPHP::Timer::GetRusageMicros(HPHP::Timer::TotalCPU,
                                      RUSAGE_THREAD) * 1000;
#else
  return -1;
#endif
}

Timer::Timer(Name name)
  : m_name(name)
  , m_start(getCPUTimeNanos())
  , m_finished(false)
{
}

Timer::~Timer() {
  if (m_finished) return;
  end();
}

void Timer::end() {
  assert(!m_finished);
  auto const finish = getCPUTimeNanos();
  auto const elapsed = finish - m_start;

  auto& counter = s_counters[m_name];
  counter.total += elapsed;
  ++counter.count;
  m_finished = true;
}

Timer::CounterVec Timer::Counters() {
  CounterVec ret;
  for (auto& pair : s_names) {
    ret.emplace_back(pair.str, s_counters[pair.name]);
  }
  return ret;
}

void Timer::RequestInit() {
  memset(&s_counters, 0, sizeof(s_counters));
}

void Timer::RequestExit() {
  Dump();
}

void Timer::Dump() {
  if (!Trace::moduleEnabledRelease(Trace::jittime)) return;
  Trace::traceRelease("%s", Show().c_str());
}

std::string Timer::Show() {
  auto const header = "{:<40} | {:>15} {:>15} {:>15}\n";
  auto const row    = "{:<40} | {:>15} {:>13,}us {:>13,}ns\n";

  std::string rows;
  for (auto& pair : s_names) {
    auto* name = pair.str;
    auto& counter = s_counters[pair.name];
    if (counter.total == 0 && counter.count == 0) continue;

    folly::format(&rows, row, name, counter.count, counter.total / 1000,
                  counter.mean());
  }

  if (rows.empty()) return rows;

  std::string ret;
  auto const url = g_context->getRequestUrl(75);
  folly::format(&ret, "\nJIT timers for {}\n", url);
  folly::format(&ret, header, "name", "count", "total time", "average time");
  folly::format(&ret, "{:-^40}-+{:-^48}\n{}\n", "", "", rows);
  return ret;
}

} }
