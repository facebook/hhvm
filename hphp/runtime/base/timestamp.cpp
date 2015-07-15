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
#include "hphp/runtime/base/timestamp.h"

#include <sys/time.h>
extern "C" {
#include <timelib.h>
}

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// creation

int64_t TimeStamp::Current() {
  return time(0);
}

double TimeStamp::CurrentSecond() {
  struct timeval tp;
  gettimeofday(&tp, nullptr);
  return (double)tp.tv_sec + (double)tp.tv_usec / 1000000;
}

const StaticString
  s_sec("sec"),
  s_usec("usec"),
  s_minuteswest("minuteswest"),
  s_dsttime("dsttime");

Array TimeStamp::CurrentTime() {
  struct timeval tp;
  gettimeofday(&tp, nullptr);

  timelib_time_offset *offset =
    timelib_get_time_zone_info(tp.tv_sec, TimeZone::Current()->get());

  auto const ret = make_map_array(
    s_sec, (int)tp.tv_sec,
    s_usec, (int)tp.tv_usec,
    s_minuteswest, (int)(-offset->offset / 60),
    s_dsttime, (int)offset->is_dst
  );
  timelib_time_offset_dtor(offset);
  return ret;
}

String TimeStamp::CurrentMicroTime() {
  struct timeval tp;
  gettimeofday(&tp, nullptr);
  char ret[100];
  snprintf(ret, 100, "%.8F %ld", (double)tp.tv_usec / 1000000, tp.tv_sec);
  return String(ret, CopyString);
}

int64_t TimeStamp::Get(bool &error, int hou, int min, int sec, int mon, int day,
                   int yea, bool gmt) {
  auto dt = req::make<DateTime>(Current());
  if (gmt) {
    dt->setTimezone(req::make<TimeZone>("UTC"));
  }
  dt->set(hou, min, sec, mon, day, yea);
  return dt->toTimeStamp(error);
}

///////////////////////////////////////////////////////////////////////////////
}
