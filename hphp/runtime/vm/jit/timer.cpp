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

static __thread Timer::CounterMap* s_counters;

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

Timer::Timer(const std::string& name)
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

  auto& counter = (*s_counters)[m_name];
  counter.total += elapsed;
  ++counter.count;
  m_finished = true;
}

const Timer::CounterMap& Timer::Counters() {
  assert(s_counters);
  return *s_counters;
}

void Timer::RequestInit() {
  assert(!s_counters);
  s_counters = new CounterMap();
}

void Timer::RequestExit() {
  Dump();
  assert(s_counters);
  delete s_counters;
  s_counters = nullptr;
}

void Timer::Dump() {
  if (!Trace::moduleEnabledRelease(Trace::jittime)) return;
  Trace::traceRelease("%s", Show().c_str());
}

std::string Timer::Show() {
  std::string ret;

  if (Counters().empty()) return ret;

  auto const url = g_context->getRequestUrl(75);
  folly::format(&ret, "\nJIT timers for {}\n", url);

  auto const header = "{:<40} | {:>15} {:>15} {:>15}\n";
  auto const row    = "{:<40} | {:>15} {:>13,}us {:>13,}ns\n";
  folly::format(&ret, header, "name", "count", "total time", "average time");
  folly::format(&ret, "{:-^40}-+{:-^48}\n", "", "");

  std::map<std::string, Counter> sorted(s_counters->begin(), s_counters->end());
  for (auto const& pair : sorted) {
    auto& name = pair.first;
    auto& counter = pair.second;

    folly::format(&ret, row, name, counter.count, counter.total / 1000,
                  counter.mean());
  }

  ret += '\n';
  return ret;
}

} }
