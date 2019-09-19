/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <algorithm>
#include <array>
#include <iterator>

#include <folly/Format.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/struct-log.h"
#include "hphp/util/timer.h"
#include "hphp/util/trace.h"

TRACE_SET_MOD(jittime);

namespace HPHP { namespace jit {

namespace {

//////////////////////////////////////////////////////////////////////

std::array<Timer::Counter,Timer::kNumTimers> kCountersDefault{};
using TimerType = std::array<Timer::Counter,Timer::kNumTimers>;
RDS_LOCAL_NO_CHECK(TimerType, s_counters)(kCountersDefault);

struct TimerName { const char* str; Timer::Name name; };
const TimerName s_names[] = {
# define TIMER_NAME(name) {#name, Timer::name},
  JIT_TIMERS
# undef TIMER_NAME
};

int64_t getCPUTimeNanos() {
  return RuntimeOption::EvalJitTimer ? HPHP::Timer::GetThreadCPUTimeNanos() :
         -1;
}

int64_t getWallClockMicros() {
  return RuntimeOption::EvalJitTimer ? HPHP::Timer::GetCurrentTimeMicros() :
         -1;
}

//////////////////////////////////////////////////////////////////////

}

Timer::Timer(Name name, StructuredLogEntry* log_entry)
  : m_name(name)
  , m_finished(false)
  , m_start(getCPUTimeNanos())
  , m_start_wall(getWallClockMicros())
  , m_log_entry(log_entry)
{
}

Timer::~Timer() {
  if (!m_finished) stop();
}

int64_t Timer::stop() {
  if (!RuntimeOption::EvalJitTimer) return 0;

  assertx(!m_finished);
  auto const elapsed = getCPUTimeNanos() - m_start;
  auto const elapsed_wall_clock = getWallClockMicros() - m_start_wall;

  if (m_log_entry) {
    m_log_entry->setInt(std::string(s_names[(size_t)m_name].str) + "_micros",
                        elapsed / 1000);
  }

  auto& counter = s_counters[m_name];
  counter.total += elapsed;
  ++counter.count;
  counter.max = std::max(counter.max, elapsed);
  counter.wall_time_elapsed += elapsed_wall_clock;
  m_finished = true;
  return elapsed;
}

Timer::CounterVec Timer::Counters() {
  CounterVec ret;
  for (auto& pair : s_names) {
    ret.emplace_back(pair.str, s_counters[pair.name]);
  }
  return ret;
}

Timer::Counter Timer::CounterValue(Timer::Name name) {
  return s_counters[name];
}

void Timer::RequestInit() {
  memset(s_counters.get(), 0, sizeof(Timer::Counter[Timer::kNumTimers]));
}

void Timer::RequestExit() {
  Dump();
}

void Timer::Dump() {
  if (!Trace::moduleEnabledRelease(Trace::jittime)) return;
  Trace::traceRelease("%s", Show().c_str());
}

std::string Timer::Show() {
  auto const header = "{:<30} | {:>15} {:>15} {:>15} {:>15}\n";
  auto const row    = "{:<30} | {:>15} {:>13,}us {:>13,}ns {:>13,}ns\n";

  std::array<TimerName,kNumTimers> names_copy;
  std::copy(s_names, s_names + kNumTimers, begin(names_copy));

  if (!getenv("HHVM_JIT_TIMER_NO_SORT")) {
    auto totalSort = [] (const TimerName& a, const TimerName& b) {
      return s_counters[a.name].total > s_counters[b.name].total;
    };
    std::sort(begin(names_copy), end(names_copy), totalSort);
  }

  std::string rows;
  for (auto const& pair : names_copy) {
    auto const& counter = s_counters[pair.name];
    if (counter.total == 0 && counter.count == 0) continue;

    folly::format(
      &rows,
      row,
      pair.str,
      counter.count,
      counter.total / 1000,
      counter.mean(),
      counter.max
    );
  }

  if (rows.empty()) return rows;

  std::string ret;
  auto const url = g_context->getRequestUrl(75);
  folly::format(&ret, "\nJIT timers for {}\n", url);
  folly::format(&ret, header, "name", "count", "total", "average", "max");
  folly::format(&ret, "{:-^30}-+{:-^64}\n{}\n", "", "", rows);
  return ret;
}

} }
