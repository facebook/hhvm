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

#include "hphp/util/timer.h"
#include "hphp/util/logger.h"
#include "hphp/util/trace.h"

#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define PRINT_MSG(...)                          \
  switch (m_report) {                           \
    case Log:                                   \
      Logger::Info(__VA_ARGS__);                \
      break;                                    \
    case Stderr:                                \
      fprintf(stderr, __VA_ARGS__);             \
      break;                                    \
    case Trace:                                 \
      Trace::traceRelease(__VA_ARGS__);         \
      break;                                    \
    default: not_reached();                     \
  }

Timer::Timer(Type type, const char *name /* = NULL */, ReportType r)
  : m_type(type), m_report(r) {
  if (name) {
    m_name = name;
    PRINT_MSG("%s...", name);
  }
  m_start = measure();
}

Timer::~Timer() {
  if (!m_name.empty()) {
    report();
  }
}

int64_t Timer::getMicroSeconds() const {
  return measure() - m_start;
}

void Timer::report() const {
  int64_t ms = getMicroSeconds();
  int seconds = ms / 1000000;
  PRINT_MSG("%s took %d'%02d\" (%" PRId64 " us) %s", m_name.c_str(),
            seconds / 60, seconds % 60, ms, getName());
}

void Timer::GetMonotonicTime(timespec &ts) {
#ifndef __APPLE__
  gettime(CLOCK_MONOTONIC, &ts);
#else
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif
}

void Timer::GetRealtimeTime(timespec &ts) {
#ifndef __APPLE__
  clock_gettime(CLOCK_REALTIME, &ts);
#else
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts.tv_sec = mts.tv_sec;
  ts.tv_nsec = mts.tv_nsec;
#endif
}

static int64_t to_usec(const timeval& tv) {
  return (int64_t(tv.tv_sec) * 1000000) + tv.tv_usec;
}

int64_t Timer::GetCurrentTimeMicros() {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return to_usec(tv);
}

int64_t Timer::measure() const {
  if (m_type == WallTime) {
    return GetCurrentTimeMicros();
  }

  struct rusage ru;
  memset(&ru, 0, sizeof(ru));
  getrusage(RUSAGE_SELF, &ru);

  switch (m_type) {
  case SystemCPU: return to_usec(ru.ru_stime);
  case UserCPU:   return to_usec(ru.ru_utime);
  case TotalCPU:  return to_usec(ru.ru_stime) + to_usec(ru.ru_utime);
  default: assert(false);
  }
  return 0;
}

const char *Timer::getName() const {
  switch (m_type) {
  case WallTime:  return "wall time";
  case SystemCPU: return "system cpu";
  case UserCPU:   return "user cpu";
  case TotalCPU:  return "total cpu";
  default: assert(false);
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

SlowTimer::SlowTimer(int64_t msThreshold, const char *location, const char *info)
  : m_timer(Timer::WallTime), m_msThreshold(msThreshold) {
  if (location) m_location = location;
  if (info) m_info = info;
}

SlowTimer::~SlowTimer() {
  int64_t msec = getTime();
  if (msec >= m_msThreshold && m_msThreshold != 0) {
    Logger::Error("SlowTimer [%" PRId64 "ms] at %s: %s",
                  msec, m_location.c_str(), m_info.c_str());
  }
}

int64_t SlowTimer::getTime() const {
  return m_timer.getMicroSeconds() / 1000;
}

///////////////////////////////////////////////////////////////////////////////
}
