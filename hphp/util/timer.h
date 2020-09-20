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

#pragma once

#include <cstdint>
#include <string>

#include <folly/portability/SysResource.h>
#include <folly/portability/SysTime.h>

#include "hphp/util/compatibility.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Timing execution of block of codes.
 */
struct Timer {
  enum Type {
    WallTime,
    SystemCPU,
    UserCPU,
    TotalCPU,
  };
  enum ReportType {
    Log,
    Stderr,
    Trace,
  };
  enum Who {
    Self = RUSAGE_SELF,
    Children = RUSAGE_CHILDREN,
#ifdef RUSAGE_THREAD
    Thread = RUSAGE_THREAD,
#endif
  };

public:
  explicit Timer(Type type, const char *name = nullptr, ReportType r = Log);
  ~Timer();

  static void GetRealtimeTime(timespec &sp);
  static void GetMonotonicTime(timespec &sp);
  static int64_t GetCurrentTimeMicros();
  static int64_t GetRusageMicros(Type t, Who who);
  static int64_t GetThreadCPUTimeNanos();
  const char *getName() const;
  int64_t getMicroSeconds() const;
  void report() const;

private:
  Type m_type;
  ReportType m_report;
  std::string m_name;
  int64_t m_start;

  int64_t measure() const;
};

///////////////////////////////////////////////////////////////////////////////

struct SlowTimer {
  SlowTimer(int64_t msThreshold, const char *location, const char *info);
  ~SlowTimer();

  int64_t getTime() const;

private:
  Timer m_timer;
  int64_t m_msThreshold;
  std::string m_location;
  std::string m_info;
};

///////////////////////////////////////////////////////////////////////////////

extern __thread int64_t s_extra_request_nanoseconds;

int gettime(clockid_t, struct timespec*);
int64_t gettime_ns(clockid_t);

/*
 * Computes the difference between two timespec objects in microseconds.
 */
int64_t gettime_diff_us(const timespec&, const timespec&);

///////////////////////////////////////////////////////////////////////////////
}

